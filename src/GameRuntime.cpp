#include <cmath>
#include "rp/RaftPirates.h"
#include "rp/PortView.h"
#include "rp/Transition.h"
#include "rp/LoginScreen.h"
#include "sys/platform/Services.h"
#include "rp/SplashScreen.h"
#include "rp/TitleScreen.h"
#include "sys/EngineAndroid.h"
#include "sys/EngineConfig.h"
#include "sys/res/Archive.h"
#include <mutex>
#include <chrono>
#include <fstream>
#include <algorithm>
#include <set>
using sys::platform::Services;

namespace {
    using Clock = std::chrono::steady_clock;

    struct Runtime {
        std::recursive_mutex mutex;
        rp::RaftPirates game;
        std::unique_ptr<rp::LoginScreen> login;
        std::future<sys::platform::AuthResult> auth;
        rp::LoginScreen::Action entry = rp::LoginScreen::Action::None;
        Clock::time_point connectStarted{};
        bool endingLoading = false;
        sys::EngineAndroid engine;
        std::unique_ptr<rp::PortView> view;
        std::unique_ptr<rp::Transition> transition;
        int transitionMode = 0;
        std::unique_ptr<rp::SplashScreen> splash;
        std::unique_ptr<rp::TitleScreen> title;
        std::unique_ptr<rp::LoadingManager> startupResources;
        bool startupFinished = false;
        std::filesystem::path archivePath, assets, savedPath;
        std::string savedSession, pendingAdminNotice;
        bool resumeAttempt = false, titlePresented = false, oceanReceived = false;
        Clock::time_point outroStarted{};
        sys::gfx::Image lastImage;
        int width = 640, height = 400, port = 9933;
        bool initialized = false, paused = false, connected = false, connecting = false;
        bool admin = false;
        std::string adminEntry;
        bool rewardPopupPending = false;
        std::string host, user, session, status, music, authEndpoint;
        bool loginSent = false;
        bool titleTouchArmed = false;
        int lastGears = 0;
        bool lastDiveFinished = false, audioFailed = false;
        float time = 0;
        Clock::time_point last = Clock::now(), lastFrame{}, lastPing{};
    };

    Runtime rt;

    template <class F> void guarded(Services& platform, F f) noexcept {
        std::lock_guard<std::recursive_mutex> l(rt.mutex);
        try {
            f();
        } catch (const std::exception& ex) {
            rt.status = ex.what();
            platform.log(rt.status);
        } catch (...) {
            rt.status = "Native operation failed";
            platform.log(rt.status);
        }
    }

    template <class F> void callAudio(Services& platform, F callback) {
        if (rt.audioFailed)
            return;
        try {
            callback();
        } catch (const std::exception& ex) {
            rt.audioFailed = true;
            platform.log(std::string("Audio unavailable: ") + ex.what());
        }
    }

    void music(Services& platform, const std::string& path) {
        const auto desired = rt.view && !rt.view->soundEnabled() ? std::string{} : path;
        if (rt.music == desired)
            return;
        callAudio(platform, [&] { platform.music(desired); });
        rt.music = desired;
    }

    void effect(Services& platform, const char* path) {
        if (!rt.view || rt.view->soundEnabled())
            callAudio(platform, [&] { platform.effect(path); });
    }

    void prompt(Services& platform, const char* title, const std::string& value) {
        const bool password = rt.entry == rp::LoginScreen::Action::Password ||
                              rt.entry == rp::LoginScreen::Action::ConfirmPassword;
        rt.engine.ShowKeyboardEntryEx(platform, title, value, password, 6, password ? 14 : 75,
                                      false);
    }

    std::string endpoint(std::string s) {
        if (s.empty() || s.find_first_of("?#@\\ \t\r\n") != std::string::npos)
            return {};
        if (s.find("://") == std::string::npos)
            s = "https://" + s;
        const bool https = s.rfind("https://", 0) == 0;
        if (!https && s.rfind("http://", 0) != 0)
            return {};
        const auto start = https ? 8u : 7u;
        if (s.size() <= start || s.find('/', start) == start)
            return {};
        if (s.size() >= 9 && s.substr(s.size() - 9) == "/auth.php")
            return s;
        if (s.back() != '/')
            s += '/';
        return s + "auth.php";
    }

    void forgetSession() {
        std::error_code ec;
        if (!rt.savedPath.empty())
            std::filesystem::remove(rt.savedPath, ec);
        rt.savedSession.clear();
    }

    void saveSession(Services& platform) {
        if (rt.savedPath.empty() || rt.savedSession.empty() || rt.user.empty() || rt.host.empty())
            return;
        if (rt.user.find_first_of("\r\n") != std::string::npos ||
            rt.host.find_first_of("\r\n") != std::string::npos ||
            rt.savedSession.find_first_of("\r\n") != std::string::npos)
            return;
        try {
            auto tmp = rt.savedPath;
            tmp += ".tmp";
            std::ofstream file(tmp, std::ios::trunc);
            if (!file)
                return;
            std::filesystem::permissions(
                tmp, std::filesystem::perms::owner_read | std::filesystem::perms::owner_write,
                std::filesystem::perm_options::replace);
            file << "RPSESSION1\n"
                 << rt.authEndpoint << '\n'
                 << rt.user << '\n'
                 << rt.host << '\n'
                 << rt.savedSession << '\n';
            file.close();
            if (file)
                std::filesystem::rename(tmp, rt.savedPath);
        } catch (...) {
            platform.log("Could not save login session");
        }
    }

    bool loadSession() {
        if (rt.savedPath.empty() || rt.authEndpoint.empty())
            return false;
        std::error_code ec;
        auto size = std::filesystem::file_size(rt.savedPath, ec);
        if (ec || size > 8192)
            return false;
        std::ifstream file(rt.savedPath);
        std::string magic, address, user, host, session;
        if (!std::getline(file, magic) || !std::getline(file, address) ||
            !std::getline(file, user) || !std::getline(file, host) ||
            !std::getline(file, session) || magic != "RPSESSION1" || address != rt.authEndpoint ||
            user.empty() || host.empty() || session.empty() ||
            host.find_first_of("/\\@?# \t\r\n") != std::string::npos)
            return false;
        rt.user = std::move(user);
        rt.host = std::move(host);
        rt.session = std::move(session);
        rt.savedSession = rt.session;
        return true;
    }

    void send(Services& platform, const sfs::Message& msg) {
        try {
            if (!rt.connected || !rt.game.ready || !platform.transportConnected() ||
                !platform.loggedIn())
                throw std::runtime_error("Active server login is required");
            platform.send(msg);
            if (msg.payload.getString("c") == "set_auto_safe_enabled")
                if (auto args = msg.payload.getSFSObj("p"))
                    rt.game.autoSafe = args->getBool("is_enabled", rt.game.autoSafe);
        } catch (...) {
            rt.game.requestSendFailed(msg.payload.getString("c"));
            if (rt.view && msg.payload.getString("c") == "purchase_item")
                rt.view->purchaseCompleted(rt.game, false, 0);
            throw;
        }
        rt.status = "SENT " + msg.payload.getString("c");
    }

    void lost() {
        rt.connected = false;
        rt.connecting = false;
        rt.game.DoLostConnection(true);
        rt.loginSent = false;
        rt.transition.reset();
        rt.transitionMode = 0;
        rt.view.reset();
        rt.lastImage = {};
        rt.user.clear();
        rt.host.clear();
        rt.status = "DISCONNECTED";
        rt.session.clear();
        rt.endingLoading = false;
        rt.savedSession.clear();
        rt.pendingAdminNotice.clear();
        rt.adminEntry.clear();
        rt.rewardPopupPending = false;
        rt.oceanReceived = false;
    }

    void loginError(Services& platform, const std::string& message) {
        bool connected = rt.connected || rt.connecting;
        if (rt.resumeAttempt)
            forgetSession();
        rt.resumeAttempt = false;
        lost();
        if (connected) {
            try {
                platform.disconnect();
            } catch (...) {
            }
        }
        if (!rt.initialized)
            return;
        // The title is reached only after authentication. A lost connection there
        // must return to login, not leave an apparently playable title screen.
        if (rt.splash)
            return;
        rt.title.reset();
        rt.titlePresented = rt.titleTouchArmed = false;
        if (!rt.login) {
            if (rt.assets.empty())
                return;
            rt.login = std::make_unique<rp::LoginScreen>(rt.assets);
        }
        if (message.empty()) {
            // A new login screen discards stale loading, input and modal state.
            rt.login = std::make_unique<rp::LoginScreen>(rt.assets);
            rt.login->returnToMenu();
            rt.entry = rp::LoginScreen::Action::None;
            rt.status.clear();
        } else {
            rt.login->clearPassword();
            rt.login->error(message);
        }
    }

    void acceptGameSession(Services& platform) {
        if (!rt.connecting || !rt.loginSent || !rt.login || rt.title || rt.splash ||
            !platform.transportConnected() || !platform.loggedIn())
            return;
        rt.connected = true;
        rt.connecting = false;
        rt.status = "LOADING PLAYER";
        rt.lastPing = rt.connectStarted = Clock::now();
        try {
            rt.game.gotMsgOnLogin();
            if (rt.game.ready)
                platform.log("Player snapshot accepted");
        } catch (const std::exception& ex) {
            platform.log(std::string("Initial world rejected: ") + ex.what());
            loginError(platform, "Unable to load the game. Please try again.");
        }
    }

    void pollAuth(Services& platform) {
        if (!rt.auth.valid() ||
            rt.auth.wait_for(std::chrono::seconds(0)) != std::future_status::ready)
            return;
        sys::platform::AuthResult result;
        try {
            result = rt.auth.get();
        } catch (...) {
            if (rt.login && rt.initialized)
                loginError(platform, "Authentication request failed.");
            return;
        }
        if (!rt.login || !rt.initialized)
            return;
        if (!result.error.empty()) {
            platform.log("Authentication: " + result.error);
            loginError(platform, "Login failed. Please try again.");
            return;
        }
        if (result.registered) {
            rt.login->registrationComplete();
            return;
        }
        rt.host = std::move(result.host);
        rt.user = std::move(result.id);
        rt.session = std::move(result.session);
        rt.savedSession = rt.session;
        rt.resumeAttempt = false;
        rt.connecting = true;
        rt.loginSent = false;
        rt.game.gotMsgAuthComplete(rt.host);
        rt.connectStarted = Clock::now();
        try {
            platform.connect(rt.host, rt.port);
        } catch (...) {
            loginError(platform, "Unable to connect to game server.");
        }
    }
} // namespace

bool isCheatEnabled() {
    std::lock_guard<std::recursive_mutex> lock(rt.mutex);
    return rt.admin;
}

namespace sys::platform::game {
    void init(Services& platform, const std::filesystem::path& apk, int w, int h) {
        guarded(platform, [&] {
            rt.width = std::max(1, int(w));
            rt.height = std::max(1, int(h));
            if (apk.empty())
                throw std::runtime_error("No APK path");
            if (rt.initialized && rt.archivePath == apk)
                return;
            if (rt.connected || rt.connecting) {
                lost();
                try {
                    platform.disconnect();
                } catch (...) {
                }
            }
            rt.initialized = false;
            rt.paused = false;
            rt.splash.reset();
            rt.title.reset();
            rt.login.reset();
            rt.transition.reset();
            rt.transitionMode = 0;
            rt.view.reset();
            rt.auth = {};
            rt.loginSent = false;
            rt.host.clear();
            rt.session.clear();
            rt.authEndpoint.clear();
            rt.savedSession.clear();
            rt.pendingAdminNotice.clear();
            rt.rewardPopupPending = false;
            rt.resumeAttempt = false;
            rt.titlePresented = false;
            rt.titleTouchArmed = false;
            rt.oceanReceived = false;
            rt.time = 0;
            rt.archivePath = apk;
            rt.audioFailed = false;
            rt.music.clear();
            rt.lastGears = 0;
            rt.lastDiveFinished = false;
            auto root = platform.assets(apk);
            rt.savedPath.clear();
            try {
                auto dir = platform.saveDirectory();
                sys::File::setSaveDir(dir.string());
                if (!dir.empty())
                    rt.savedPath = dir / "raftpirates-session.dat";
            } catch (...) {
                platform.log("Persistent login storage unavailable");
            }
            rt.assets = root;
            sys::EngineConfig properties;
            try {
                if (!properties.LoadProperties((root / "xml_resources/properties.xml").string()))
                    platform.log("Could not load properties.xml");
            } catch (const std::exception& ex) {
                platform.log(std::string("Could not load properties.xml: ") + ex.what());
            }
            rt.authEndpoint = endpoint(properties.GetProperty("SERVER_IP"));
            rt.admin = properties.GetPropertyAsBool("admin", false);
            rt.game.initialize();
            rt.engine.InitEngine(0);
            rt.engine.SetupOpenGl(rt.width, rt.height, 640, 400);
            rt.user.clear();
            rt.entry = rp::LoginScreen::Action::None;
            rt.endingLoading = false;
            // Original loading_intro.xml authenticates before title.xml is presented.
            rt.startupFinished = false;
            rt.startupResources = std::make_unique<rp::LoadingManager>(root);
            if (rt.startupResources->assetExists("xml_bin/game_manifest.bin")) {
                rt.startupResources->addManifest("xml_bin/game_manifest.bin");
                rt.startupResources->start();
            }
            try {
                rt.splash = std::make_unique<rp::SplashScreen>(root);
            } catch (const std::exception& ex) {
                platform.log(std::string("Splash unavailable: ") + ex.what());
            }
            rt.lastImage = {};
            rt.lastFrame = {};
            rt.initialized = true;
            rt.status.clear();
            rt.last = Clock::now();
            platform.log("Raft Pirates startup: loading initialized; property endpoint " +
                         std::string(rt.authEndpoint.empty() ? "missing" : "configured"));
        });
    }

    bool initialized() {
        std::lock_guard<std::recursive_mutex> l(rt.mutex);
        return rt.initialized;
    }

    void pause(Services& platform, bool value) {
        guarded(platform, [&] {
            rt.paused = value;
            rt.titleTouchArmed = false;
            if (rt.title)
                rt.title->cancelTouch();
            if (rt.login)
                rt.login->cancelInput();
            if (rt.view)
                rt.view->cancelInput();
            rt.game.Pause(value);
            if (!value)
                rt.engine.Resume();
            if (!rt.music.empty())
                callAudio(platform, [&] { platform.pauseMusic(value); });
            rt.last = Clock::now();
        });
    }

    void kill(Services& platform) {
        guarded(platform, [&] {
            if (rt.connected || rt.connecting) {
                try {
                    platform.disconnect();
                } catch (...) {
                }
            }
            music(platform, "");
            lost();
            rt.splash.reset();
            rt.title.reset();
            rt.login.reset();
            rt.auth = {};
            rt.transition.reset();
            rt.transitionMode = 0;
            rt.view.reset();
            rt.lastImage = {};
            rt.initialized = false;
            rt.startupResources.reset();
            rt.startupFinished = false;
            rt.engine.ShutDown();
            rt.admin = false;
            rt.entry = rp::LoginScreen::Action::None;
        });
    }

    void tick(Services& platform) {
        guarded(platform, [&] {
            if (!rt.initialized || rt.paused)
                return;
            auto now = Clock::now();
            float dt = std::clamp(std::chrono::duration<float>(now - rt.last).count(), 0.f, .25f);
            rt.last = now;
            rt.time += dt;
            rt.engine.Tick(dt);
            if (rt.view && !rt.login) {
                rt.game.tick(dt);
                rt.view->tick(dt);
                if (rt.connected)
                    if (auto request = rt.view->takePendingRequest(rt.game))
                        send(platform, *request);
                if (rt.connected && rt.game.onlineBattle.shouldSendClientReady())
                    if (auto ready = rt.game.onlineBattle.SendClientReady())
                        send(platform, *ready);
                if (!rt.title && rt.game.ready) {
                    const int mode =
                        rt.game.onlineBattle.phase() != rp::BattleModeState::Phase::Idle ? 2
                        : rt.game.discovery                                              ? 1
                                                                                         : 0;
                    if (!rt.transition) {
                        rt.transition = std::make_unique<rp::Transition>(
                            rt.assets, &rt.engine.engineReceiver());
                        rt.transitionMode = mode;
                        rt.transition->done = [](bool shown) {
                            if (shown) {
                                rt.game.GotTransitionMsgDone();
                                rt.game.GotMsgStartTransition(false);
                                rt.transition->hide();
                            } else {
                                rp::MsgTransitionDone msg;
                                msg.shown = false;
                                rt.game.onlineBattle.gotMsgTransitionDone(msg);
                            }
                        };
                    }
                    if (mode != rt.transitionMode && !rt.transition->active()) {
                        rt.transitionMode = mode;
                        rt.view->cancelInput();
                        rt.game.GotMsgStartTransition(true);
                        rt.transition->show();
                    }
                    rt.transition->tick(dt);
                }
                const auto menu = rt.view->activeMenu();
                if (menu != rt.game.currentMenu()) {
                    rt.game.GotMsgOnMenuLoadGlobal(menu);
                    sys::msg::MsgOnMenuLoadGlobal event;
                    event.name = menu;
                    rt.engine.engineReceiver().SendGeneric(event);
                }
            }
            if (rt.splash) {
                try {
                    rt.splash->tick(dt);
                    if (!rt.splash->finished()) {
                        platform.present(rt.splash->render(), rt.width, rt.height);
                        return;
                    }
                } catch (const std::exception& ex) {
                    platform.log(std::string("Splash animation failed: ") + ex.what());
                }
                rt.splash.reset();
                rt.lastImage = {};
            }
            if (!rt.startupFinished) {
                rt.startupFinished = true;
                rt.login = std::make_unique<rp::LoginScreen>(rt.assets);
                if (loadSession() && rt.game.gotMsgReconnect(true)) {
                    rt.login->busy(true);
                    rt.resumeAttempt = true;
                    rt.connecting = true;
                    rt.connectStarted = now;
                    platform.log("Resuming saved login session during startup");
                    try {
                        platform.connect(rt.host, rt.port);
                    } catch (...) {
                        loginError(platform, "Please log in again.");
                    }
                }
            }
            if (rt.startupResources && rt.startupResources->running()) {
                rt.startupResources->update(dt);
                if (!rt.startupResources->running()) {
                    for (const auto& error : rt.startupResources->errors())
                        platform.log("Resource: " + error);
                    platform.log("Initial resource manifest processed");
                }
            }
            pollAuth(platform);
            if (rt.connecting && rt.loginSent) {
                try {
                    if (platform.loggedIn()) {
                        platform.log(
                            "SmartFox SDK confirms login; recovering missing native login event");
                        sys::platform::game::onLogin(platform);
                    }
                } catch (const std::exception& ex) {
                    platform.log(std::string("SmartFox login state unavailable: ") + ex.what());
                }
            }
            if (rt.connected && (!platform.transportConnected() || !platform.loggedIn())) {
                loginError(platform, "Game server connection lost.");
                return;
            }
            if (rt.connected && rt.game.ready && rt.view && !rt.login && !rt.rewardPopupPending) {
                if (auto reward = rt.game.takeQuestRewardNotice()) {
                    if (reward->message)
                        rt.engine.engineReceiver().SendGeneric(*reward->message);
                    try {
                        rt.rewardPopupPending = true;
                        rt.engine.PopupText(platform, reward->title + "\n" + reward->text);
                    } catch (...) {
                        rt.rewardPopupPending = false;
                        platform.log("Quest reward popup unavailable");
                    }
                }
            }
            if (rt.connected && !rt.pendingAdminNotice.empty() && !rt.rewardPopupPending) {
                auto notice = std::move(rt.pendingAdminNotice);
                rt.pendingAdminNotice.clear();
                try {
                    rt.engine.PopupText(platform, notice, false, true);
                } catch (...) {
                    platform.log("Admin popup unavailable");
                }
            }
            if (rt.connected &&
                rt.game.checkKeepAlive(std::chrono::duration<float>(now - rt.lastPing).count())) {
                rt.lastPing = now;
                try {
                    platform.keepAlive();
                } catch (...) {
                    loginError(platform, "Game server connection failed.");
                    return;
                }
            }
            if ((rt.connecting || (rt.connected && (!rt.game.ready || !rt.oceanReceived))) &&
                now - rt.connectStarted > std::chrono::seconds(30)) {
                platform.log(
                    rt.oceanReceived
                        ? "World loading timed out after gs_ocean; awaiting login/player snapshot"
                        : "World loading timed out awaiting server response");
                loginError(platform, "Unable to connect. Please try again.");
            }
            if (rt.login) {
                if (rt.connected && rt.game.ready && rt.oceanReceived && !rt.endingLoading &&
                    (!rt.startupResources || !rt.startupResources->running())) {
                    try {
                        rt.view = std::make_unique<rp::PortView>(rt.assets);
                        rt.lastImage = rt.view->renderTitleBackground(rt.game, rt.time);
                    } catch (...) {
                        loginError(platform, "Unable to load the game. Please try again.");
                        return;
                    }
                    rt.endingLoading = true;
                    rt.outroStarted = now;
                    rt.game.gotMsgLoadFinished();
                    saveSession(platform);
                    try {
                        rt.game.saveStaticLibraries();
                    } catch (const std::exception& ex) {
                        platform.log(std::string("Static library cache not saved: ") + ex.what());
                    }
                    rt.savedSession.clear();
                    rt.resumeAttempt = false;
                    rt.login->finish();
                    platform.log("Player and ocean ready: starting loading outro before title");
                }
                if (!rt.endingLoading || now - rt.outroStarted < std::chrono::seconds(5)) {
                    try {
                        rt.login->tick(dt);
                        if (!rt.login->finished()) {
                            platform.present(rt.login->render(rt.endingLoading ? &rt.lastImage : nullptr),
                                             rt.width, rt.height);
                            return;
                        }
                    } catch (const std::exception& ex) {
                        platform.log(std::string("Loading menu rendering failed: ") + ex.what());
                        if (!rt.endingLoading)
                            throw;
                        // The first gameplay frame is already rendered. A broken outro must not
                        // keep the player behind the curtains indefinitely.
                    }
                }
                if (!rt.connected || !rt.game.ready || !rt.view)
                    return;
                rt.login.reset();
                rt.lastFrame = now;
                rt.title = std::make_unique<rp::TitleScreen>(rt.assets, rt.user);
                rt.titlePresented = rt.titleTouchArmed = false;
                platform.log("Loading complete: presenting title");
            }
            if (rt.title) {
                music(platform, "audio/music/theme.mp3");
                if (rt.view && (rt.lastImage.pixels.empty() ||
                    now - rt.lastFrame >= std::chrono::milliseconds(33))) {
                    rt.lastFrame = now;
                    rt.lastImage = rt.view->renderTitleBackground(rt.game, rt.time);
                }
                platform.present(rt.title->render(&rt.lastImage), rt.width, rt.height);
                rt.titlePresented = true;
                return;
            }
            if (!rt.connected || !rt.game.ready || !rt.view)
                return;
            if (auto tutorial = rt.view->takeTutorialRequest(rt.game))
                send(platform, *tutorial);
            if (rt.lastImage.pixels.empty() ||
                now - rt.lastFrame >= std::chrono::milliseconds(33)) {
                rt.lastFrame = now;
                try {
                    rt.lastImage = rt.view->render(rt.game, rt.time, rt.status);
                } catch (const std::exception& ex) {
                    platform.log(std::string("Gameplay render failed: ") + ex.what());
                    loginError(platform, "Unable to load the game. Please try again.");
                    return;
                }
            }
            music(platform, rt.view->isDiving() ? "audio/music/underwater.mp3"
                            : rt.game.onlineBattle.phase() != rp::BattleModeState::Phase::Idle
                                ? "audio/music/battle.mp3"
                            : rt.game.ready ? "audio/music/ambient.mp3"
                                            : "audio/music/theme.mp3");
            int gears = rt.view->diveGears();
            bool finished = rt.view->diveFinished();
            if (gears > rt.lastGears)
                effect(platform, "audio/sfx/diving_game_collect.wav");
            if (finished && !rt.lastDiveFinished)
                effect(platform, "audio/sfx/diving_game_end.wav");
            rt.lastGears = gears;
            rt.lastDiveFinished = finished;
            if (rt.transition && rt.transition->active()) {
                auto frame = rt.lastImage;
                rt.transition->Render(frame);
                platform.present(frame, rt.width, rt.height);
            } else
                platform.present(rt.lastImage, rt.width, rt.height);
        });
    }

    void touchDown(Services& platform, float x, float y) {
        guarded(platform, [&] {
            if (!rt.initialized || rt.paused || !std::isfinite(x) || !std::isfinite(y) ||
                rt.splash || (rt.transition && rt.transition->blocksInput()))
                return;
            if (rt.title) {
                if (!rt.titlePresented)
                    return;
                rt.titleTouchArmed = true;
                rt.title->touchDown(x * 640 / rt.width, y * 400 / rt.height);
                return;
            }
            if (rt.login) {
                rt.login->touchDown(x * 640 / rt.width, y * 400 / rt.height);
                return;
            }
            if (rt.view && rt.game.ready)
                rt.view->touchDown(x * 640 / rt.width, y * 400 / rt.height);
            const auto now = Clock::now();
            if (rt.connected &&
                rt.game.gotMsgTouchDown(std::chrono::duration<float>(now - rt.lastPing).count())) {
                rt.lastPing = now;
                platform.keepAlive();
            }
            sys::msg::MsgTouchDown m;
            m.x = x * 640 / rt.width;
            m.y = y * 400 / rt.height;
            rt.engine.engineReceiver().SendGeneric(m);
        });
    }

    void touchUp(Services& platform, float x, float y) {
        guarded(platform, [&] {
            if (!rt.initialized || rt.paused || !std::isfinite(x) || !std::isfinite(y))
                return;
            if (rt.splash) {
                rt.splash->skip();
                rt.splash.reset();
                rt.lastImage = {};
                return;
            }
            float px = x * 640 / rt.width, py = y * 400 / rt.height;
            if (rt.title) {
                if (!rt.titlePresented || !rt.titleTouchArmed || rt.paused)
                    return;
                rt.titleTouchArmed = false;
                rt.title->touchUp(px, py);
                if (rt.title->started()) {
                    if (!rt.connected || !rt.game.ready || !rt.oceanReceived || !rt.view)
                        return;
                    platform.log("Set Sail pressed; entering loaded gameplay");
                    effect(platform, "audio/sfx/menu_button_down.wav");
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
                        prompt(platform, "Email", rt.login->email());
                    else
                        prompt(platform,
                               action == Action::Password ? "Password" : "Confirm Password",
                               action == Action::Password ? rt.login->password()
                                                          : rt.login->confirmation());
                } else if (action == Action::Submit) {
                    if (rt.login->email().empty() || rt.login->password().empty()) {
                        rt.login->error("Enter your email and password.");
                    } else if (rt.authEndpoint.empty()) {
                        rt.login->error("Unable to connect. Please try again.");
                    } else {
                        rt.auth = platform.authenticate(rt.authEndpoint, rt.login->email(),
                                                        rt.login->password(), false, 2, "1.0.4");
                        rt.login->clearPassword();
                        rt.login->busy(true);
                    }
                } else if (action == Action::Register) {
                    if (rt.login->email().empty() || rt.login->password().empty())
                        rt.login->error("Enter your email and password.");
                    else if (rt.login->password() != rt.login->confirmation())
                        rt.login->error("Passwords do not match.");
                    else if (rt.authEndpoint.empty())
                        rt.login->error("Unable to connect. Please try again.");
                    else {
                        rt.auth = platform.authenticate(rt.authEndpoint, rt.login->email(),
                                                        rt.login->password(), true, 2, "1.0.4");
                        rt.login->clearPassword();
                        rt.login->busy(true);
                    }
                } else if (action == Action::Facebook)
                    rt.login->error("Facebook login is not available.");
                else if (action == Action::Reset) {
                    if (rt.authEndpoint.empty()) {
                        rt.login->error("Unable to connect. Please try again.");
                    } else {
                        auto page = rt.authEndpoint.substr(0, rt.authEndpoint.size() - 8) +
                                    "lost_password.php";
                        // The ARM32 release used https://raftpirates.6waves.com/lost_password.php.
                        // Custom servers use their configured authentication base instead.
                        if (rt.authEndpoint == "https://w1.raftpirates.6waves.com/auth.php")
                            page = "https://raftpirates.6waves.com/lost_password.php";
                        try {
                            rt.engine.GotoWebSite(platform, page);
                        } catch (const std::exception& ex) {
                            platform.log(std::string("Password recovery: ") + ex.what());
                            rt.login->error("Unable to open password recovery.");
                        }
                    }
                }
                return;
            }
            if (!rt.connected || !rt.game.ready || !rt.view ||
                (rt.transition && rt.transition->blocksInput()))
                return;
            const auto now = Clock::now();
            if (rt.connected &&
                rt.game.gotMsgTouchUp(std::chrono::duration<float>(now - rt.lastPing).count())) {
                rt.lastPing = now;
                platform.keepAlive();
            }
            sys::msg::MsgTouchUp m;
            m.x = px;
            m.y = py;
            rt.engine.engineReceiver().SendGeneric(m);
            auto result = rt.view->touch(rt.game, px, py);
            auto notice = rt.view->takeNotice();
            if (!notice.empty())
                platform.showPopup(notice);
            if (result && result->payload.getString("c") == "name_ship") {
                if (rt.game.gotMsgNameShip())
                    rt.engine.ShowKeyboardEntryEx(platform, "Name your raft",
                        rt.game.playerName, false, 1, 32, false);
                return;
            }
            if (result && result->payload.getString("c") == "cheat") {
                if (!rt.admin)
                    return;
                auto args = result->payload.getSFSObj("p");
                if (!args)
                    return;
                const auto type = args->getString("type");
                rt.game.enableDevelopmentCommands(rt.admin);
                if (type == "start_battle" || type == "player_dup") {
                    rt.adminEntry = type;
                    rt.engine.ShowKeyboardEntryEx(platform,
                        type == "start_battle" ? "Opponent name" : "Player name",
                        "", false, 0, 75, false);
                    return;
                }
                rp::MsgCheat cheat;
                cheat.type = type;
                if (auto request = rt.game.gotMsgCheat(cheat))
                    send(platform, *request);
                return;
            }
            if (result) {
                if (!rt.view->isDiving())
                    effect(platform, "audio/sfx/menu_button_down.wav");
                send(platform, *result);
            }
        });
    }

    void touchMove(Services& platform, float x, float y) {
        guarded(platform, [&] {
            if (!rt.initialized || rt.paused || !std::isfinite(x) || !std::isfinite(y) ||
                rt.splash || (rt.transition && rt.transition->blocksInput()))
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
            const auto now = Clock::now();
            if (rt.connected &&
                rt.game.gotMsgTouchDrag(std::chrono::duration<float>(now - rt.lastPing).count())) {
                rt.lastPing = now;
                platform.keepAlive();
            }
            sys::msg::MsgTouchDrag m;
            m.x = x * 640 / rt.width;
            m.y = y * 400 / rt.height;
            rt.engine.engineReceiver().SendGeneric(m);
        });
    }

    void touchState(Services& platform, const std::vector<float>& x, const std::vector<float>& y) {
        guarded(platform, [&] {
            if (!rt.initialized || rt.paused || rt.width <= 0 || rt.height <= 0 ||
                (rt.transition && rt.transition->blocksInput()))
                return;
            auto n = std::min({x.size(), y.size(), size_t(16)});
            sys::msg::MsgTouchState m;
            for (size_t i = 0; i < n; ++i)
                if (std::isfinite(x[i]) && std::isfinite(y[i]) && x[i] >= 0 && y[i] >= 0 &&
                    x[i] <= rt.width && y[i] <= rt.height)
                    m.AddPoint(int(double(x[i]) * 640 / rt.width),
                               int(double(y[i]) * 400 / rt.height), int(i));
            rt.engine.engineReceiver().SendGeneric(m);
        });
    }

    std::string website() {
        // The original Java callback returns the website base, not the BBB auth.php endpoint.
        return "https://raftpirates.6waves.com/";
    }

    std::string userId() {
        std::lock_guard<std::recursive_mutex> l(rt.mutex);
        return rt.user;
    }

    void musicFinished(Services& platform) {
        guarded(platform,
                [&] { rt.engine.engineReceiver().SendGeneric(sys::msg::MsgMusicFinished{}); });
    }

    void soundFinished(Services& platform, int id) {
        guarded(platform,
                [&] { rt.engine.engineReceiver().SendGeneric(sys::msg::MsgSoundFinished(id)); });
    }

    void popupResult(Services& platform, bool accepted) {
        guarded(platform, [&] {
            sys::msg::MsgPopupTextResult m;
            m.accepted = accepted;
            rt.rewardPopupPending = false;
            rt.game.gotMsgPopupTextResult(accepted);
            rt.engine.engineReceiver().SendGeneric(m);
        });
    }

    void keyboardPreview(Services& platform, const std::string& value) {
        guarded(platform, [&] {
            if (rt.login && rt.entry != rp::LoginScreen::Action::None)
                rt.login->setInput(rt.entry, value);
        });
    }

    void keyboardResult(Services& platform, std::string value, bool cancelled) {
        guarded(platform, [&] {
            auto entry = rt.entry;
            rt.entry = rp::LoginScreen::Action::None;
            if (entry != rp::LoginScreen::Action::None) {
                rt.engine.ShowKeyboardResult(value, cancelled, false);
                if (rt.login && !cancelled)
                    rt.login->setInput(entry, value);
                return;
            }
            if (!rt.adminEntry.empty()) {
                auto type = std::move(rt.adminEntry);
                rt.adminEntry.clear();
                rt.engine.ShowKeyboardResult(value, cancelled, false);
                if (!cancelled && rt.admin && rt.connected && !value.empty()) {
                    if (type == "start_battle") {
                        rp::MsgCheatStartBattle msg;
                        msg.username = value;
                        if (auto request = rt.game.gotMsgCheatStartBattle(msg))
                            send(platform, *request);
                    } else if (type == "player_dup") {
                        rp::MsgCheatDuplicatePlayer msg;
                        msg.username = value;
                        if (auto request = rt.game.gotMsgCheatDuplicatePlayer(msg))
                            send(platform, *request);
                    }
                }
                return;
            }
            rt.engine.ShowKeyboardResult(value, cancelled);
            if (auto request = rt.game.gotMsgKeyboardEntryResult(value, cancelled))
                send(platform, *request);
        });
    }

    void onConnection(Services& platform, bool success) {
        guarded(platform, [&] {
            if (!rt.connecting || rt.loginSent || rt.session.empty())
                return;
            if (!success) {
                loginError(platform, "Connection failed.");
                return;
            }
            if (!rt.savedPath.empty()) {
                try {
                    rt.game.loadStaticLibraries(rt.savedPath.parent_path() /
                                                    "raftpirates-libraries.dat",
                                                rt.authEndpoint + "\n" + rt.host);
                } catch (const std::exception& ex) {
                    platform.log(std::string("Static library cache ignored: ") + ex.what());
                }
            }
            auto p = rt.game.login(rt.user, "").payload.getSFSObj("p");
            if (!p)
                throw std::runtime_error("Missing login params");
            rt.game.gotMsgOnConnection(true);
            rt.connectStarted = Clock::now();
            rt.status = "LOGGING IN";
            rt.loginSent = true;
            platform.log("SmartFox login request sent");
            try {
                platform.login("RaftPirates", rt.user, rt.session, *p);
            } catch (...) {
                loginError(platform, "Unable to log into game server.");
                return;
            }
            rt.session.clear();
        });
    }

    void onLogin(Services& platform) {
        guarded(platform, [&] {
            if ((!rt.connecting && !rt.connected) || !rt.loginSent)
                return;
            platform.log("SmartFox login accepted");
            acceptGameSession(platform);
        });
    }

    void onLoginError(Services& platform) {
        guarded(platform, [&] {
            if ((rt.connecting || rt.connected) && rt.loginSent)
                loginError(platform, "Login failed. Please try again.");
        });
    }

    void onRoomJoin(Services& platform) {
        guarded(platform, [&] {
            if (!rt.connected && !rt.connecting)
                return;
            rt.game.gotMsgOnRoomJoin2();
            rt.status = "ROOM JOINED";
        });
    }

    void onConnectionLost(Services& platform) {
        guarded(platform, [&] {
            if (rt.connected || rt.connecting)
                loginError(platform, "");
        });
    }

    void onSocketError(Services& platform) {
        guarded(platform, [&] {
            if (rt.connected || rt.connecting)
                loginError(platform, "Game server connection failed.");
        });
    }

    void onLogout(Services& platform) {
        guarded(platform, [&] {
            if (rt.connected || rt.connecting) {
                forgetSession();
                loginError(platform, "");
            }
        });
    }

    void onExtension(Services& platform, const std::string& command,
                     const sfs::SFSObjectWrapper& data) {
        guarded(platform, [&] {
            if ((!rt.connected && !rt.connecting) || !rt.loginSent ||
                !platform.transportConnected())
                return;
            platform.log("NET: " + command);
            try {
                if (command == "gs_player" && !rt.connected) {
                    auto player = data.getSFSObj("player_object");
                    if (!player || player->getInt("user_id") <= 0 ||
                        !player->getSFSArray("rafts") || !player->getSFSArray("materials") ||
                        !player->getSFSArray("blueprints"))
                        throw std::runtime_error("Incomplete initial player snapshot");
                    platform.log("initial player packet validated");
                }
                if (command == "gs_ocean" && !rt.connected && !data.getSFSObj("ocean_state"))
                    throw std::runtime_error("Incomplete initial ocean snapshot");
                rt.game.gotMsgOnExtensionResponse(command, data);
            } catch (const std::exception& ex) {
                platform.log("Rejected " + command + ": " + ex.what());
                if (rt.login && !rt.game.ready && command == "gs_player")
                    loginError(platform, "Unable to load the game. Please try again.");
                return;
            }
            rt.status = rt.game.status;
            if (command == "cheat_result") {
                if (data.getBool("result") && data.getBool("logout") && rt.admin) {
                    forgetSession();
                    loginError(platform, "");
                    return;
                }
                if (!data.getBool("result"))
                    platform.showPopup(data.getString("message", "Command failed."));
            }
            if (command == "gs_error" && rt.view)
                rt.view->purchaseCompleted(rt.game, false, 0);
            if (command == "lib_go_data") {
                for (const auto& row : rt.game.objects)
                    if (row.second.name == "Pain Sprinkler" ||
                        row.second.asset.find("pain_sprinkler") != std::string::npos) {
                        std::string cells;
                        for (const auto& c : row.second.cells)
                            cells += " (" + std::to_string(c.x) + "," + std::to_string(c.y) + ")";
                        platform.log("Pain Sprinkler object " + std::to_string(row.first) +
                                     " server footprint:" + cells);
                    }
            }
            if (command == "purchase_response") {
                const bool accepted = data.getBool("result");
                platform.log(std::string("purchase_response result=") +
                    (accepted ? "true" : "false") + " blueprint_id=" +
                    std::to_string(data.getInt("blueprint_id")));
                if (rt.view)
                    rt.view->purchaseCompleted(rt.game, accepted, data.getInt("blueprint_id"));
                if (!accepted)
                    platform.showPopup("Purchase failed.");
            }
            if (command == "gs_ocean") {
                rt.oceanReceived = true;
                if (!rt.game.ready)
                    platform.log("Initial ocean received; checking game session readiness");
            }
            if (command == "gs_player" && rt.game.ready)
                platform.log("Player snapshot accepted");
        });
    }

    void onAdminMessage(Services& platform, const std::string& message) {
        guarded(platform, [&] {
            if (!rt.connected)
                return;
            rt.game.gotMsgOnAdminMessage(message);
            rt.pendingAdminNotice = rt.game.adminMessage();
            platform.log("Received server administration message");
        });
    }
} // namespace sys::platform::game
