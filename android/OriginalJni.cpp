#include "rp/RaftPirates.h"
#include "rp/PortView.h"
#include "rp/LoginScreen.h"
#include "Auth.h"
#include "rp/SplashScreen.h"
#include "rp/TitleScreen.h"
#include "sys/EngineBase.h"
#include "sys/res/Archive.h"
#include "sys/android/Jni.h"
#include "sys/android/Presenter.h"
#include "sfs/JavaClient.h"
#include <mutex>
#include <chrono>
#include <fstream>
#include <algorithm>
#include <set>
using namespace sys::android;

namespace {
    using Clock = std::chrono::steady_clock;

    struct Runtime {
        std::recursive_mutex mutex;
        rp::RaftPirates game;
        std::unique_ptr<rp::LoginScreen> login;
        std::future<rp::auth::Result> auth;
        rp::LoginScreen::Action entry = rp::LoginScreen::Action::None;
        Clock::time_point connectStarted{};
        bool endingLoading = false;
        sys::EngineBase engine;
        Presenter presenter;
        std::unique_ptr<rp::PortView> view;
        std::unique_ptr<rp::SplashScreen> splash;
        std::unique_ptr<rp::TitleScreen> title;
        std::filesystem::path archivePath, assets;
        sys::gfx::Image lastImage;
        int width = 640, height = 400, port = 9933;
        bool initialized = false, paused = false, connected = false, connecting = false;
        std::string host, user, session, status, music;
        int lastGears = 0;
        bool lastDiveFinished = false, audioFailed = false;
        float time = 0;
        Clock::time_point last = Clock::now(), lastFrame{}, lastPing{};
    };

    Runtime rt;

    void unsupported(const char* name) {
        static std::mutex m;
        static std::set<std::string> seen;
        std::lock_guard<std::mutex> l(m);
        if (seen.insert(name).second)
            log(std::string("Unimplemented original callback: ") + name);
    }

    template <class F> void guarded(JNIEnv* e, F f) noexcept {
        std::lock_guard<std::recursive_mutex> l(rt.mutex);
        try {
            f();
        } catch (const std::exception& ex) {
            if (e->ExceptionCheck())
                e->ExceptionClear();
            rt.status = ex.what();
            log(rt.status);
        } catch (...) {
            if (e->ExceptionCheck())
                e->ExceptionClear();
            rt.status = "Native operation failed";
            log(rt.status);
        }
    }

    // Reuse the original HydraGame Java sound backend.
    template <class F> void audio(JNIEnv* e, F callback) {
        if (rt.audioFailed)
            return;
        try {
            LocalFrame frame(e);
            callback();
            check(e, "audio");
        } catch (const std::exception& ex) {
            if (e->ExceptionCheck())
                e->ExceptionClear();
            rt.audioFailed = true;
            log(std::string("Audio unavailable: ") + ex.what());
        }
    }

    void music(JNIEnv* e, jobject obj, const std::string& path) {
        if (rt.music == path)
            return;
        audio(e, [&] {
            if (path.empty())
                e->CallVoidMethod(obj, method(e, obj, "stopMusic", "()V"));
            else {
                auto name = e->NewStringUTF(path.c_str());
                check(e, "music path");
                e->CallVoidMethod(obj, method(e, obj, "playMusic", "(Ljava/lang/String;)V"), name);
            }
        });
        rt.music = path;
    }

    void effect(JNIEnv* e, jobject obj, const char* path) {
        audio(e, [&] {
            auto name = e->NewStringUTF(path);
            check(e, "sound path");
            e->CallVoidMethod(obj, method(e, obj, "playSound", "(Ljava/lang/String;F)V"), name,
                              jfloat(1));
        });
    }

    void prompt(JNIEnv* e, jobject obj, const char* title, const std::string& value) {
        LocalFrame f(e);
        auto a = e->NewStringUTF(title), b = e->NewStringUTF(value.c_str());
        if (rt.entry == rp::LoginScreen::Action::Password ||
            rt.entry == rp::LoginScreen::Action::ConfirmPassword) {
            auto invalid = e->NewStringUTF("");
            e->CallVoidMethod(
                obj,
                method(e, obj, "showPasswordEntry",
                       "(Ljava/lang/String;Ljava/lang/String;Ljava/lang/String;ZIIZ)V"),
                a, b, invalid, JNI_TRUE, jint(6), jint(14), JNI_FALSE);
            check(e, "showPasswordEntry");
        } else {
            e->CallVoidMethod(
                obj,
                method(e, obj, "showKeyboardEntry", "(Ljava/lang/String;Ljava/lang/String;ZIIZ)V"),
                a, b, JNI_TRUE, jint(6), jint(75), JNI_FALSE);
            check(e, "showKeyboardEntry");
        }
    }

    std::filesystem::path cacheRoot(JNIEnv* e, jobject obj, const std::filesystem::path& apk) {
        LocalFrame f(e);
        auto s = static_cast<jstring>(
            e->CallObjectMethod(obj, method(e, obj, "GetCachePath", "()Ljava/lang/String;")));
        check(e, "GetCachePath");
        auto base = string(e, s);
        if (base.empty())
            throw std::runtime_error("No app cache directory");
        auto name = std::to_string(std::filesystem::file_size(apk)) + "-" +
                    std::to_string(static_cast<long long>(
                        std::filesystem::last_write_time(apk).time_since_epoch().count()));
        auto root = std::filesystem::path(base) / "hydra-legacy-v03" / name;
        if (!std::filesystem::exists(root / ".complete")) {
            sys::res::Archive a(apk);
            auto n = a.extractAssets(root);
            std::ofstream marker(root / ".complete");
            marker << n;
            marker.close();
            if (!marker)
                throw std::runtime_error("Could not mark extracted assets");
            log("APK assets extracted: " + std::to_string(n));
        }
        return root;
    }

    void send(JNIEnv* e, const sfs::Message& msg) {
        try {
            if (!rt.connected || !rt.game.ready)
                throw std::runtime_error("Login is required");
            sfs::JavaClient::send(e, msg);
        } catch (...) {
            rt.game.onlineBattle.sendFailed();
            rt.game.sonarPending = false;
            rt.game.pending.reset();
            rt.game.pendingMove.reset();
            rt.game.pendingMap.reset();
            rt.game.pendingZone.reset();
            rt.game.pendingDiscovery.reset();
            throw;
        }
        rt.status = "SENT " + msg.payload.getString("c");
    }

    void lost() {
        rt.connected = false;
        rt.connecting = false;
        rt.game.DoLostConnection(false);
        rt.status = "DISCONNECTED";
        rt.session.clear();
        rt.endingLoading = false;
    }

    void loginError(JNIEnv* e, const std::string& message) {
        bool connected = rt.connected || rt.connecting;
        lost();
        if (connected) {
            try {
                sfs::JavaClient::disconnect(e);
            } catch (...) {
            }
        }
        if (!rt.initialized)
            return;
        rt.title.reset();
        if (!rt.login) {
            if (rt.assets.empty())
                return;
            rt.login = std::make_unique<rp::LoginScreen>(rt.assets);
        }
        rt.login->clearPassword();
        rt.login->error(message);
    }

    void pollAuth(JNIEnv* e) {
        if (!rt.auth.valid() ||
            rt.auth.wait_for(std::chrono::seconds(0)) != std::future_status::ready)
            return;
        auto result = rt.auth.get();
        if (!rt.login || !rt.initialized)
            return;
        if (!result.error.empty()) {
            loginError(e, result.error);
            return;
        }
        if (result.registered) {
            rt.login->registrationComplete();
            return;
        }
        rt.game.initialize();
        rt.host = std::move(result.host);
        rt.user = std::move(result.id);
        rt.session = std::move(result.session);
        rt.connecting = true;
        rt.game.DoConnect();
        rt.connectStarted = Clock::now();
        try {
            sfs::JavaClient::connect(e, rt.host, rt.port);
        } catch (...) {
            loginError(e, "Unable to connect to game server.");
        }
    }
} // namespace

#define RP(name) Java_com_sixwaves_raftpirates_MyLib_##name
#define HY(name) Java_com_bigbluebubble_hydra_HydraGame_##name
#define SFS(name) Java_com_bigbluebubble_smartfox_ClientServices_##name

extern "C" JNIEXPORT void JNICALL RP(init)(JNIEnv* e, jobject obj, jstring path, jint w, jint h,
                                           jfloat, jfloat) {
    guarded(e, [&] {
        rt.width = std::max(1, int(w));
        rt.height = std::max(1, int(h));
        auto apk = std::filesystem::path(string(e, path));
        if (apk.empty())
            throw std::runtime_error("No APK path");
        if (rt.view && rt.archivePath == apk) {
            rt.initialized = true;
            return;
        }
        rt.archivePath = apk;
        rt.audioFailed = false;
        rt.music.clear();
        rt.lastGears = 0;
        rt.lastDiveFinished = false;
        auto root = cacheRoot(e, obj, apk);
        rt.assets = root;
        rt.game.initialize();
        rt.user.clear();
        rt.entry = rp::LoginScreen::Action::None;
        rt.endingLoading = false;
        rt.view = std::make_unique<rp::PortView>(root);
        try {
            rt.splash = std::make_unique<rp::SplashScreen>(root);
        } catch (const std::exception& ex) {
            log(std::string("Splash unavailable: ") + ex.what());
        }
        try {
            rt.title = std::make_unique<rp::TitleScreen>(root);
            rt.login = std::make_unique<rp::LoginScreen>(root);
        } catch (const std::exception& ex) {
            log(std::string("Title unavailable: ") + ex.what());
        }
        rt.lastImage = {};
        rt.lastFrame = {};
        rt.initialized = true;
        rt.status.clear();
        rt.last = Clock::now();
    });
}

extern "C" JNIEXPORT void JNICALL RP(initGL)(JNIEnv* e, jobject) {
    guarded(e, [&] { rt.presenter.contextCreated(); });
}

extern "C" JNIEXPORT jboolean JNICALL RP(isJniInititalized)(JNIEnv*, jobject) {
    std::lock_guard<std::recursive_mutex> l(rt.mutex);
    return rt.initialized ? JNI_TRUE : JNI_FALSE;
}

extern "C" JNIEXPORT void JNICALL RP(pause)(JNIEnv* e, jobject obj, jboolean value) {
    guarded(e, [&] {
        rt.paused = value;
        rt.game.Pause(value);
        if (!rt.music.empty())
            audio(e, [&] {
                e->CallVoidMethod(obj, method(e, obj, value ? "pauseMusic" : "resumeMusic", "()V"));
            });
        rt.last = Clock::now();
    });
}

extern "C" JNIEXPORT void JNICALL RP(kill)(JNIEnv* e, jobject obj) {
    guarded(e, [&] {
        if (rt.connected || rt.connecting) {
            try {
                sfs::JavaClient::disconnect(e);
            } catch (...) {
            }
        }
        music(e, obj, "");
        lost();
        rt.splash.reset();
        rt.title.reset();
        rt.login.reset();
        rt.auth = {};
        rt.view.reset();
        rt.lastImage = {};
        rt.initialized = false;
        rt.entry = rp::LoginScreen::Action::None;
    });
}

extern "C" JNIEXPORT void JNICALL RP(tick)(JNIEnv* e, jobject obj) {
    guarded(e, [&] {
        if (!rt.view || rt.paused)
            return;
        auto now = Clock::now();
        float dt = std::clamp(std::chrono::duration<float>(now - rt.last).count(), 0.f, .25f);
        rt.last = now;
        rt.time += dt;
        rt.engine.Tick(dt);
        rt.game.tick(dt);
        rt.view->tick(dt);
        if (rt.splash) {
            rt.splash->tick(dt);
            if (!rt.splash->finished()) {
                rt.presenter.present(rt.splash->render(), rt.width, rt.height);
                return;
            }
            rt.splash.reset();
            rt.lastImage = {};
        }
        if (rt.title) {
            music(e, obj, "audio/music/theme.mp3");
            rt.presenter.present(rt.title->render(), rt.width, rt.height);
            return;
        }
        pollAuth(e);
        if ((rt.connecting || (rt.connected && !rt.game.ready)) &&
            now - rt.connectStarted > std::chrono::seconds(30))
            loginError(e, "The game server did not respond.");
        if (rt.login) {
            if (rt.connected && rt.game.ready && !rt.endingLoading) {
                rt.endingLoading = true;
                rt.login->finish();
            }
            rt.login->tick(dt);
            if (!rt.login->finished()) {
                rt.presenter.present(rt.login->render(), rt.width, rt.height);
                return;
            }
            rt.login.reset();
            rt.lastImage = {};
        }
        if (!rt.game.ready)
            return;
        if (rt.game.checkKeepAlive(std::chrono::duration<float>(now - rt.lastPing).count())) {
            rt.lastPing = now;
            sfs::JavaClient::send(e, rp::Game::request("keep_alive"));
        }
        if (rt.lastImage.pixels.empty() || now - rt.lastFrame >= std::chrono::milliseconds(33)) {
            rt.lastFrame = now;
            rt.lastImage = rt.view->render(rt.game, rt.time, rt.status);
        }
        music(e, obj,
              rt.view->isDiving() ? "audio/music/underwater.mp3"
              : rt.game.ready     ? "audio/music/ambient.mp3"
                                  : "audio/music/theme.mp3");
        int gears = rt.view->diveGears();
        bool finished = rt.view->diveFinished();
        if (gears > rt.lastGears)
            effect(e, obj, "audio/sfx/diving_game_collect.wav");
        if (finished && !rt.lastDiveFinished)
            effect(e, obj, "audio/sfx/diving_game_end.wav");
        rt.lastGears = gears;
        rt.lastDiveFinished = finished;
        rt.presenter.present(rt.lastImage, rt.width, rt.height);
    });
}

extern "C" JNIEXPORT void JNICALL RP(touchDown)(JNIEnv* e, jobject, jfloat x, jfloat y) {
    guarded(e, [&] {
        if (rt.splash)
            return;
        if (rt.title) {
            rt.title->touchDown(x * 640 / rt.width, y * 400 / rt.height);
            return;
        }
        if (rt.login) {
            rt.login->touchDown(x * 640 / rt.width, y * 400 / rt.height);
            return;
        }
        if (rt.view && rt.game.ready)
            rt.view->touchDown(x * 640 / rt.width, y * 400 / rt.height);
        sys::msg::MsgTouchDown m;
        m.x = x * 640 / rt.width;
        m.y = y * 400 / rt.height;
        rt.engine.engineReceiver().SendGeneric(m);
    });
}

extern "C" JNIEXPORT void JNICALL RP(touchUp)(JNIEnv* e, jobject obj, jfloat x, jfloat y) {
    guarded(e, [&] {
        if (!rt.view)
            return;
        if (rt.splash) {
            rt.splash->skip();
            rt.splash.reset();
            rt.lastImage = {};
            return;
        }
        float px = x * 640 / rt.width, py = y * 400 / rt.height;
        if (rt.title) {
            rt.title->touchUp(px, py);
            if (rt.title->started()) {
                effect(e, obj, "audio/sfx/menu_button_down.wav");
                rt.title.reset();
                rt.lastImage = {};
            }
            return;
        }
        if (rt.login) {
            using Action = rp::LoginScreen::Action;
            auto action = rt.login->touchUp(px, py);
            if (action == Action::Email || action == Action::Password ||
                action == Action::ConfirmPassword) {
                rt.entry = action;
                if (action == Action::Email)
                    prompt(e, obj, "Email", rt.login->email());
                else
                    prompt(e, obj, action == Action::Password ? "Password" : "Confirm Password",
                           action == Action::Password ? rt.login->password()
                                                      : rt.login->confirmation());
            } else if (action == Action::Submit) {
                if (rt.login->email().empty() || rt.login->password().empty()) {
                    rt.login->error("Enter your email and password.");
                } else if (rp::auth::endpoint().empty()) {
                    rt.login->error("Authentication server is not configured.");
                } else {
                    rt.auth = rp::auth::request(e, rt.login->email(), rt.login->password());
                    rt.login->clearPassword();
                    rt.login->busy(true);
                }
            } else if (action == Action::Register) {
                if (rt.login->email().empty() || rt.login->password().empty())
                    rt.login->error("Enter your email and password.");
                else if (rt.login->password() != rt.login->confirmation())
                    rt.login->error("Passwords do not match.");
                else if (rp::auth::endpoint().empty())
                    rt.login->error("Authentication server is not configured.");
                else {
                    rt.auth = rp::auth::request(e, rt.login->email(), rt.login->password(), true);
                    rt.login->clearPassword();
                    rt.login->busy(true);
                }
            } else if (action == Action::Facebook)
                rt.login->error("Facebook login is not available.");
            else if (action == Action::Reset)
                rt.login->error("Password reset is not available yet.");
            return;
        }
        if (!rt.game.ready)
            return;
        sys::msg::MsgTouchUp m;
        m.x = px;
        m.y = py;
        rt.engine.engineReceiver().SendGeneric(m);
        auto result = rt.view->touch(rt.game, px, py);
        if (result) {
            if (!rt.view->isDiving())
                effect(e, obj, "audio/sfx/menu_button_down.wav");
            send(e, *result);
        }
    });
}

extern "C" JNIEXPORT void JNICALL RP(touchMove)(JNIEnv* e, jobject, jfloat x, jfloat y) {
    guarded(e, [&] {
        if (rt.splash)
            return;
        if (rt.title) {
            rt.title->touchDrag(x * 640 / rt.width, y * 400 / rt.height);
            return;
        }
        if (rt.login) {
            rt.login->touchDrag(x * 640 / rt.width, y * 400 / rt.height);
            return;
        }
        if (rt.view && rt.game.ready)
            rt.view->touchDrag(x * 640 / rt.width, y * 400 / rt.height);
        sys::msg::MsgTouchDrag m;
        m.x = x * 640 / rt.width;
        m.y = y * 400 / rt.height;
        rt.engine.engineReceiver().SendGeneric(m);
    });
}

extern "C" JNIEXPORT void JNICALL RP(touchState)(JNIEnv* e, jobject, jint n, jfloatArray xs,
                                                 jfloatArray ys) {
    guarded(e, [&] {
        if (n <= 0 || !xs || !ys) {
            rt.engine.engineReceiver().SendGeneric(sys::msg::MsgTouchState{});
            return;
        }
        n = std::min({n, e->GetArrayLength(xs), e->GetArrayLength(ys), jint(16)});
        std::vector<jfloat> x(n), y(n);
        e->GetFloatArrayRegion(xs, 0, n, x.data());
        e->GetFloatArrayRegion(ys, 0, n, y.data());
        check(e, "touchState arrays");
        sys::msg::MsgTouchState m;
        for (int i = 0; i < n; ++i)
            if (x[i] >= 0 && y[i] >= 0)
                m.AddPoint(int(x[i] * 640 / rt.width), int(y[i] * 400 / rt.height), i);
        rt.engine.engineReceiver().SendGeneric(m);
    });
}

extern "C" JNIEXPORT jstring JNICALL RP(getAuthURL)(JNIEnv* e, jobject) {
    return e->NewStringUTF(rp::auth::endpoint().c_str());
}

extern "C" JNIEXPORT jstring JNICALL RP(getUserID)(JNIEnv* e, jobject) {
    std::lock_guard<std::recursive_mutex> l(rt.mutex);
    return e->NewStringUTF(rt.user.c_str());
}

extern "C" JNIEXPORT void JNICALL HY(sendMsgMusicFinished)(JNIEnv* e, jobject) {
    guarded(e, [&] { rt.engine.engineReceiver().SendGeneric(sys::msg::MsgMusicFinished{}); });
}

extern "C" JNIEXPORT void JNICALL HY(sendMsgSoundFinished)(JNIEnv* e, jobject, jint id) {
    guarded(e, [&] { rt.engine.engineReceiver().SendGeneric(sys::msg::MsgSoundFinished(id)); });
}

extern "C" JNIEXPORT void JNICALL HY(sendMsgPopupTextResult)(JNIEnv* e, jobject,
                                                             jboolean accepted) {
    guarded(e, [&] {
        sys::msg::MsgPopupTextResult m;
        m.accepted = accepted;
        rt.engine.engineReceiver().SendGeneric(m);
    });
}

extern "C" JNIEXPORT void JNICALL HY(init)(JNIEnv*, jobject) {
    log("Original HydraGame JNI bridge initialized (v0.3)");
}

extern "C" JNIEXPORT void JNICALL HY(sendMsgKeyboardEntryResult)(JNIEnv* e, jobject obj,
                                                                 jstring value,
                                                                 jboolean cancelled) {
    guarded(e, [&] {
        auto entry = rt.entry;
        rt.entry = rp::LoginScreen::Action::None;
        if (entry != rp::LoginScreen::Action::None) {
            if (rt.login && !cancelled)
                rt.login->setInput(entry, string(e, value));
            return;
        }
        sys::msg::MsgKeyboardEntryResult event;
        event.text = string(e, value);
        event.cancelled = cancelled;
        rt.engine.engineReceiver().SendGeneric(event);
    });
}

extern "C" JNIEXPORT void JNICALL SFS(OnConnection)(JNIEnv* e, jobject, jobject evt) {
    guarded(e, [&] {
        if (!rt.connecting)
            return;
        if (!sfs::JavaClient::connectionSucceeded(e, evt)) {
            loginError(e, "Connection failed.");
            return;
        }
        auto p = rt.game.login(rt.user, "").payload.getSFSObj("p");
        if (!p)
            throw std::runtime_error("Missing login params");
        rt.game.gotMsgOnConnection(true);
        rt.status = "LOGGING IN";
        try {
            sfs::JavaClient::login(e, "RaftPirates", rt.user, rt.session, *p);
        } catch (...) {
            loginError(e, "Unable to log into game server.");
            return;
        }
        rt.session.clear();
    });
}

extern "C" JNIEXPORT void JNICALL SFS(OnLogin)(JNIEnv* e, jobject, jobject) {
    guarded(e, [&] {
        if (!rt.connecting)
            return;
        rt.game.gotMsgOnLogin();
        rt.connected = true;
        rt.connecting = false;
        rt.status = "LOADING PLAYER";
        rt.lastPing = Clock::now();
    });
}

extern "C" JNIEXPORT void JNICALL SFS(OnLoginError)(JNIEnv* e, jobject, jobject) {
    guarded(e, [&] { loginError(e, "Game server rejected the login."); });
}

extern "C" JNIEXPORT void JNICALL SFS(OnRoomJoin)(JNIEnv* e, jobject, jobject) {
    guarded(e, [&] {
        rt.game.gotMsgOnRoomJoin();
        rt.status = "ROOM JOINED";
    });
}

extern "C" JNIEXPORT void JNICALL SFS(OnConnectionLost)(JNIEnv* e, jobject, jobject) {
    guarded(e, [&] {
        if (rt.connected || rt.connecting)
            loginError(e, "Disconnected from game server.");
    });
}

extern "C" JNIEXPORT void JNICALL SFS(OnSocketError)(JNIEnv* e, jobject, jobject) {
    guarded(e, [&] {
        if (rt.connected || rt.connecting)
            loginError(e, "Game server connection failed.");
    });
}

extern "C" JNIEXPORT void JNICALL SFS(OnLogout)(JNIEnv* e, jobject, jobject) {
    guarded(e, [&] {
        if (rt.connected || rt.connecting)
            loginError(e, "Disconnected from game server.");
    });
}

extern "C" JNIEXPORT void JNICALL SFS(OnExtensionResponse)(JNIEnv* e, jobject, jobject evt) {
    guarded(e, [&] {
        if (!rt.connected && !rt.connecting)
            return;
        auto decoded = sfs::JavaClient::extension(e, evt);
        rt.game.gotMsgOnExtensionResponse(decoded.first, decoded.second);
        rt.status = rt.game.status;
    });
}

// Explicit unsupported callbacks. They match the original DEX signatures but do not
// pretend to implement billing, social features, audio or unused event handling.
#include "UnsupportedCallbacks.inc"
