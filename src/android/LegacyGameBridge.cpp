#include "sys/platform/Services.h"
#include "sys/android/Auth.h"
#include "sys/android/Jni.h"
#include "sys/android/Presenter.h"
#include "sys/res/Archive.h"
#include "sfs/JavaClient.h"
#include <algorithm>
#include <fstream>
#include <mutex>
#include <set>

namespace {
    using namespace sys::android;
    namespace game = sys::platform::game;
    Presenter presenter;
    std::mutex activityMutex;
    jobject activity = nullptr;

    bool setActivity(JNIEnv* e, jobject obj) {
        std::lock_guard<std::mutex> lock(activityMutex);
        if (activity && obj && e->IsSameObject(activity, obj))
            return false;
        auto old = activity;
        activity = obj ? e->NewGlobalRef(obj) : nullptr;
        check(e, "Activity reference");
        if (old)
            e->DeleteGlobalRef(old);
        return true;
    }

    bool currentActivity(JNIEnv* e, jobject obj) {
        std::lock_guard<std::mutex> lock(activityMutex);
        return activity && e->IsSameObject(activity, obj);
    }

    jobject activityRef(JNIEnv* e, jobject fallback) {
        std::lock_guard<std::mutex> lock(activityMutex);
        return activity ? e->NewLocalRef(activity) : e->NewLocalRef(fallback);
    }

    class AndroidServices final : public sys::platform::Services {
        JNIEnv* e_;
        jobject obj_;

        std::string path(const char* name) {
            LocalFrame frame(e_);
            auto value = static_cast<jstring>(
                e_->CallObjectMethod(obj_, method(e_, obj_, name, "()Ljava/lang/String;")));
            check(e_, name);
            return string(e_, value);
        }

      public:
        AndroidServices(JNIEnv* e, jobject obj) : e_(e), obj_(activityRef(e, obj)) {}

        ~AndroidServices() override {
            if (obj_)
                e_->DeleteLocalRef(obj_);
        }

        std::filesystem::path assets(const std::filesystem::path& apk) override {
            auto base = path("GetCachePath");
            if (base.empty())
                throw std::runtime_error("No app cache directory");
            auto name = std::to_string(std::filesystem::file_size(apk)) + "-" +
                        std::to_string(static_cast<long long>(
                            std::filesystem::last_write_time(apk).time_since_epoch().count()));
            auto root = std::filesystem::path(base) / "hydra-legacy-v03" / name;
            if (!std::filesystem::exists(root / ".complete")) {
                auto n = sys::res::Archive(apk).extractAssets(root);
                std::ofstream marker(root / ".complete");
                marker << n;
                marker.close();
                if (!marker)
                    throw std::runtime_error("Could not mark extracted assets");
                log("APK assets extracted: " + std::to_string(n));
            }
            return root;
        }

        std::filesystem::path saveDirectory() override {
            return path("GetFilePath");
        }

        void present(const sys::gfx::Image& im, int w, int h) override {
            presenter.present(im, w, h);
        }

        void music(const std::string& value) override {
            LocalFrame frame(e_);
            if (value.empty())
                e_->CallVoidMethod(obj_, method(e_, obj_, "stopMusic", "()V"));
            else
                e_->CallVoidMethod(obj_, method(e_, obj_, "playMusic", "(Ljava/lang/String;)V"),
                                   e_->NewStringUTF(value.c_str()));
            check(e_, "music");
        }

        void pauseMusic(bool value) override {
            e_->CallVoidMethod(obj_, method(e_, obj_, value ? "pauseMusic" : "resumeMusic", "()V"));
            check(e_, "pause music");
        }

        void effect(const std::string& value) override {
            LocalFrame frame(e_);
            e_->CallVoidMethod(obj_, method(e_, obj_, "playSound", "(Ljava/lang/String;F)V"),
                               e_->NewStringUTF(value.c_str()), jfloat(1));
            check(e_, "sound");
        }

        void showPopup(const std::string& message) override {
            LocalFrame frame(e_);
            e_->CallVoidMethod(obj_, method(e_, obj_, "showPopup", "(Ljava/lang/String;Z)V"),
                               e_->NewStringUTF(message.c_str()), JNI_FALSE);
            check(e_, "showPopup");
        }

        void openWebPage(const std::string& url) override {
            LocalFrame frame(e_);
            e_->CallVoidMethod(obj_, method(e_, obj_, "launchWebPage", "(Ljava/lang/String;)V"),
                               e_->NewStringUTF(url.c_str()));
            check(e_, "launchWebPage");
        }

        std::string deviceInfo(sys::platform::DeviceField field) override {
            using F = sys::platform::DeviceField;
            switch (field) {
            case F::Version: return path("getVersion");
            case F::AndroidID: return path("getAndroidID");
            case F::OpenUDID: return path("getOpenUDID");
            case F::MAC: return path("getMACAddress");
            case F::OS:
            case F::Type: {
#ifdef __ANDROID__
                LocalFrame frame(e_);
                const bool os = field == F::OS;
                auto cls = e_->FindClass(os ? "android/os/Build$VERSION" : "android/os/Build");
                check(e_, "Android Build class");
                auto id = e_->GetStaticFieldID(cls, os ? "RELEASE" : "MODEL", "Ljava/lang/String;");
                check(e_, "Android Build field");
                auto value = static_cast<jstring>(e_->GetStaticObjectField(cls, id));
                check(e_, "Android Build value");
                return string(e_, value);
#else
                return {};
#endif
            }
            }
            return {};
        }

        float screenDPI() override {
            auto value = e_->CallFloatMethod(obj_, method(e_, obj_, "getScreenDPI", "()F"));
            check(e_, "getScreenDPI");
            return value;
        }

        bool internetConnected() override {
            auto value = e_->CallBooleanMethod(obj_, method(e_, obj_, "checkInternetConnection", "()Z"));
            check(e_, "checkInternetConnection");
            return value == JNI_TRUE;
        }

        void popup(const std::string& text, bool buttons, bool blocking) override {
            LocalFrame frame(e_);
            auto value = e_->NewStringUTF(text.c_str());
            if (blocking)
                e_->CallVoidMethod(obj_, method(e_, obj_, "showPopupBlocking", "(Ljava/lang/String;ZZ)V"),
                                   value, jboolean(buttons), JNI_TRUE);
            else
                e_->CallVoidMethod(obj_, method(e_, obj_, "showPopup", "(Ljava/lang/String;Z)V"),
                                   value, jboolean(buttons));
            check(e_, "popup");
        }

        bool sendEmail(const std::string& address, const std::string& subject,
                       const std::string& body) override {
            LocalFrame frame(e_);
            e_->CallVoidMethod(obj_, method(e_, obj_, "sendEmail",
                "(Ljava/lang/String;Ljava/lang/String;Ljava/lang/String;)V"),
                e_->NewStringUTF(address.c_str()), e_->NewStringUTF(subject.c_str()),
                e_->NewStringUTF(body.c_str()));
            check(e_, "sendEmail");
            return true;
        }

        bool followTwitter(const std::string& account, const std::string& text) override {
            LocalFrame frame(e_);
            e_->CallVoidMethod(obj_, method(e_, obj_, "followUsOnTwitter",
                "(Ljava/lang/String;Ljava/lang/String;)V"),
                e_->NewStringUTF(account.c_str()), e_->NewStringUTF(text.c_str()));
            check(e_, "followUsOnTwitter");
            return true;
        }

        bool googleCheckout(const std::string& item, int quantity) override {
            LocalFrame frame(e_);
            e_->CallVoidMethod(obj_, method(e_, obj_, "initGoogleCheckout", "(Ljava/lang/String;I)V"),
                               e_->NewStringUTF(item.c_str()), jint(quantity));
            check(e_, "initGoogleCheckout");
            return true;
        }

        bool loggedIn() override {
            return sfs::JavaClient::loggedIn(e_);
        }

        bool transportConnected() override {
            return sfs::JavaClient::transportConnected(e_);
        }

        void keyboard(const std::string& title, const std::string& value, bool password) override {
            keyboardEntry(title, value, password, 6, password ? 14 : 75, false);
        }

        void keyboardEntry(const std::string& title, const std::string& value, bool password,
                           int minimum, int maximum, bool multiline) override {
            LocalFrame frame(e_);
            auto a = e_->NewStringUTF(title.c_str()), b = e_->NewStringUTF(value.c_str());
            if (password)
                e_->CallVoidMethod(
                    obj_,
                    method(e_, obj_, "showPasswordEntry",
                           "(Ljava/lang/String;Ljava/lang/String;Ljava/lang/String;ZIIZ)V"),
                    a, b, e_->NewStringUTF(""), JNI_TRUE, jint(minimum), jint(maximum), jboolean(multiline));
            else
                e_->CallVoidMethod(obj_,
                                   method(e_, obj_, "showKeyboardEntry",
                                          "(Ljava/lang/String;Ljava/lang/String;ZIIZ)V"),
                                   a, b, JNI_TRUE, jint(minimum), jint(maximum), jboolean(multiline));
            check(e_, "keyboard entry");
        }

        std::future<sys::platform::AuthResult> authenticate(std::string address, std::string email,
                                                            std::string password, bool registration,
                                                            int gameId,
                                                            std::string version) override {
            return auth::request(e_, std::move(address), std::move(email), std::move(password),
                                 registration, gameId, std::move(version));
        }

        void connect(const std::string& host, int port) override {
            sfs::JavaClient::connect(e_, host, port);
        }

        void login(const std::string& zone, const std::string& user, const std::string& session,
                   const sfs::SFSObjectWrapper& p) override {
            sfs::JavaClient::login(e_, zone, user, session, p);
        }

        void send(const sfs::Message& message) override {
            sfs::JavaClient::send(e_, message);
        }

        void disconnect() override {
            sfs::JavaClient::disconnect(e_);
        }

        void keepAlive() override {
            sfs::JavaClient::keepAlive(e_);
        }

        void log(const std::string& message) override {
            sys::android::log(message);
        }
    };

    void unsupported(const char* name) {
        static std::mutex mutex;
        static std::set<std::string> seen;
        std::lock_guard<std::mutex> lock(mutex);
        if (seen.insert(name).second)
            sys::android::log(std::string("Unimplemented original callback: ") + name);
    }

    template <class F> void dispatch(JNIEnv* e, jobject obj, F f) noexcept {
        try {
            AndroidServices platform(e, obj);
            f(platform);
        } catch (const std::exception& ex) {
            if (e->ExceptionCheck())
                e->ExceptionClear();
            sys::android::log(ex.what());
        } catch (...) {
            if (e->ExceptionCheck())
                e->ExceptionClear();
            sys::android::log("Platform callback failed");
        }
    }
} // namespace

#define RP(name) Java_com_sixwaves_raftpirates_MyLib_##name
#define HY(name) Java_com_bigbluebubble_hydra_HydraGame_##name
#define SFS(name) Java_com_bigbluebubble_smartfox_ClientServices_##name

extern "C" JNIEXPORT void JNICALL RP(init)(JNIEnv* e, jobject obj, jstring path, jint w, jint h,
                                           jfloat, jfloat) {
    try {
        const bool newOwner = setActivity(e, obj);
        dispatch(e, obj, [&](auto& p) {
            if (newOwner && game::initialized()) {
                p.log("New Android host; resetting previous game session");
                game::kill(p);
            }
            game::init(p, string(e, path), w, h);
        });
    } catch (const std::exception& ex) {
        sys::android::log(ex.what());
    }
}

extern "C" JNIEXPORT void JNICALL RP(initGL)(JNIEnv* e, jobject obj) {
    dispatch(e, obj, [&](auto&) { presenter.contextCreated(); });
}

extern "C" JNIEXPORT jboolean JNICALL RP(isJniInititalized)(JNIEnv*, jobject) {
    return game::initialized() ? JNI_TRUE : JNI_FALSE;
}

extern "C" JNIEXPORT void JNICALL RP(pause)(JNIEnv* e, jobject obj, jboolean value) {
    if (!currentActivity(e, obj))
        return;
    dispatch(e, obj, [&](auto& p) { game::pause(p, value); });
}

extern "C" JNIEXPORT void JNICALL RP(kill)(JNIEnv* e, jobject obj) {
    if (!currentActivity(e, obj))
        return;
    dispatch(e, obj, [&](auto& p) { game::kill(p); });
    try {
        setActivity(e, nullptr);
    } catch (const std::exception& ex) {
        sys::android::log(ex.what());
    }
}

extern "C" JNIEXPORT void JNICALL RP(tick)(JNIEnv* e, jobject obj) {
    if (!currentActivity(e, obj))
        return;
    dispatch(e, obj, [&](auto& p) { game::tick(p); });
}

extern "C" JNIEXPORT void JNICALL RP(touchDown)(JNIEnv* e, jobject obj, jfloat x, jfloat y) {
    if (!currentActivity(e, obj))
        return;
    dispatch(e, obj, [&](auto& p) { game::touchDown(p, x, y); });
}

extern "C" JNIEXPORT void JNICALL RP(touchUp)(JNIEnv* e, jobject obj, jfloat x, jfloat y) {
    if (!currentActivity(e, obj))
        return;
    dispatch(e, obj, [&](auto& p) { game::touchUp(p, x, y); });
}

extern "C" JNIEXPORT void JNICALL RP(touchMove)(JNIEnv* e, jobject obj, jfloat x, jfloat y) {
    if (!currentActivity(e, obj))
        return;
    dispatch(e, obj, [&](auto& p) { game::touchMove(p, x, y); });
}

extern "C" JNIEXPORT void JNICALL RP(touchState)(JNIEnv* e, jobject obj, jint n, jfloatArray xs,
                                                 jfloatArray ys) {
    if (!currentActivity(e, obj))
        return;
    dispatch(e, obj, [&](auto& p) {
        n = xs && ys
                ? std::clamp(n, jint(0),
                             std::min({e->GetArrayLength(xs), e->GetArrayLength(ys), jint(16)}))
                : 0;
        std::vector<float> x(n), y(n);
        if (n) {
            e->GetFloatArrayRegion(xs, 0, n, x.data());
            e->GetFloatArrayRegion(ys, 0, n, y.data());
            check(e, "touchState");
        }
        game::touchState(p, x, y);
    });
}

extern "C" JNIEXPORT jstring JNICALL RP(getAuthURL)(JNIEnv* e, jobject) {
    return e->NewStringUTF(game::website().c_str());
}

extern "C" JNIEXPORT jstring JNICALL RP(getUserID)(JNIEnv* e, jobject) {
    return e->NewStringUTF(game::userId().c_str());
}

extern "C" JNIEXPORT void JNICALL HY(init)(JNIEnv*, jobject) {
    sys::android::log("Hydra platform bridge initialized");
}

extern "C" JNIEXPORT void JNICALL HY(sendMsgMusicFinished)(JNIEnv* e, jobject obj) {
    dispatch(e, obj, [&](auto& p) { game::musicFinished(p); });
}

extern "C" JNIEXPORT void JNICALL HY(sendMsgSoundFinished)(JNIEnv* e, jobject obj, jint id) {
    dispatch(e, obj, [&](auto& p) { game::soundFinished(p, id); });
}

extern "C" JNIEXPORT void JNICALL HY(sendMsgPopupTextResult)(JNIEnv* e, jobject obj,
                                                             jboolean value) {
    dispatch(e, obj, [&](auto& p) { game::popupResult(p, value); });
}

extern "C" JNIEXPORT void JNICALL HY(sendMsgKeyboardEntryResult)(JNIEnv* e, jobject obj,
                                                                 jstring value,
                                                                 jboolean cancelled) {
    dispatch(e, obj, [&](auto& p) { game::keyboardResult(p, string(e, value), cancelled); });
}

extern "C" JNIEXPORT void JNICALL SFS(OnConnection)(JNIEnv* e, jobject obj, jobject evt) {
    dispatch(e, obj,
             [&](auto& p) { game::onConnection(p, sfs::JavaClient::connectionSucceeded(e, evt)); });
}

extern "C" JNIEXPORT void JNICALL SFS(OnLogin)(JNIEnv* e, jobject obj, jobject) {
    dispatch(e, obj, [&](auto& p) {
        p.log("SmartFox callback: OnLogin");
        game::onLogin(p);
    });
}

extern "C" JNIEXPORT void JNICALL SFS(OnLoginError)(JNIEnv* e, jobject obj, jobject) {
    dispatch(e, obj, [&](auto& p) {
        p.log("SmartFox callback: OnLoginError");
        game::onLoginError(p);
    });
}

extern "C" JNIEXPORT void JNICALL SFS(OnRoomJoin)(JNIEnv* e, jobject obj, jobject) {
    dispatch(e, obj, [&](auto& p) {
        p.log("SmartFox callback: OnRoomJoin");
        game::onRoomJoin(p);
    });
}

extern "C" JNIEXPORT void JNICALL SFS(OnConnectionLost)(JNIEnv* e, jobject obj, jobject) {
    dispatch(e, obj, [&](auto& p) { game::onConnectionLost(p); });
}

extern "C" JNIEXPORT void JNICALL SFS(OnSocketError)(JNIEnv* e, jobject obj, jobject) {
    dispatch(e, obj, [&](auto& p) { game::onSocketError(p); });
}

extern "C" JNIEXPORT void JNICALL SFS(OnLogout)(JNIEnv* e, jobject obj, jobject) {
    dispatch(e, obj, [&](auto& p) { game::onLogout(p); });
}

extern "C" JNIEXPORT void JNICALL SFS(OnExtensionResponse)(JNIEnv* e, jobject obj, jobject evt) {
    dispatch(e, obj, [&](auto& p) {
        auto d = sfs::JavaClient::extension(e, evt);
        game::onExtension(p, d.first, d.second);
    });
}

extern "C" JNIEXPORT void JNICALL SFS(OnAdminMessage)(JNIEnv* e, jobject obj, jobject evt) {
    dispatch(e, obj,
             [&](auto& p) { game::onAdminMessage(p, sfs::JavaClient::adminMessage(e, evt)); });
}

#include "UnsupportedCallbacks.inc"
