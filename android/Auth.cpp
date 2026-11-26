#include "Auth.h"
#include "sys/android/Jni.h"
#include <thread>
#include <cctype>
#include <stdexcept>
#include <utility>

namespace rp::auth {
    using namespace sys::android;

    namespace {
        std::string encode(const std::string& s) {
            static const char hex[] = "0123456789ABCDEF";
            std::string out;
            for (unsigned char c : s) {
                if ((c >= 'a' && c <= 'z') || (c >= 'A' && c <= 'Z') || (c >= '0' && c <= '9') ||
                    c == '-' || c == '_' || c == '.' || c == '~')
                    out += char(c);
                else {
                    out += '%';
                    out += hex[c >> 4];
                    out += hex[c & 15];
                }
            }
            return out;
        }

        jclass cls(JNIEnv* e, const char* name) {
            auto c = e->FindClass(name);
            check(e, "auth class");
            if (!c)
                throw std::runtime_error("Authentication API unavailable");
            return c;
        }

        jmethodID ctor(JNIEnv* e, jclass c, const char* sig) {
            auto m = e->GetMethodID(c, "<init>", sig);
            check(e, "auth constructor");
            return m;
        }

        jstring jstr(JNIEnv* e, const std::string& value) {
            auto j = e->NewStringUTF(value.c_str());
            check(e, "auth string");
            return j;
        }

        Result fetch(JNIEnv* e, const std::string& email, const std::string& password,
                     bool registration, jclass jsonClass) {
            LocalFrame frame(e);
            auto base = endpoint();
            if (base.empty())
                return {{}, {}, {}, "Authentication server is not configured."};
            auto u = encode(email), p = encode(password);
            auto address = base + "?u=" + u +
                           (registration ? "&u2=" + u + "&p=" + p + "&p2=" + p + "&c=1"
                                         : "&p=" + p + "&ue=" + u + "&pe=" + p) +
                           "&t=bbb&g=2&lang=en&client_version=1.0.4&platform=android";
            auto urlClass = cls(e, "java/net/URL");
            auto url = e->NewObject(urlClass, ctor(e, urlClass, "(Ljava/lang/String;)V"),
                                    jstr(e, address));
            check(e, "auth URL");
            auto conn = e->CallObjectMethod(
                url, method(e, url, "openConnection", "()Ljava/net/URLConnection;"));
            check(e, "auth connection");

            struct Close {
                JNIEnv* e;
                jobject c;

                ~Close() {
                    try {
                        e->CallVoidMethod(c, method(e, c, "disconnect", "()V"));
                        if (e->ExceptionCheck())
                            e->ExceptionClear();
                    } catch (...) {
                    }
                }
            } close{e, conn};

            e->CallVoidMethod(conn, method(e, conn, "setConnectTimeout", "(I)V"), jint(8000));
            check(e, "auth timeout");
            e->CallVoidMethod(conn, method(e, conn, "setReadTimeout", "(I)V"), jint(8000));
            check(e, "auth timeout");
            e->CallVoidMethod(conn, method(e, conn, "setInstanceFollowRedirects", "(Z)V"),
                              JNI_FALSE);
            check(e, "auth redirects");
            auto code = e->CallIntMethod(conn, method(e, conn, "getResponseCode", "()I"));
            check(e, "auth response");
            if (code != 200)
                return {{},
                        {},
                        {},
                        "Authentication server returned HTTP " + std::to_string(code) + "."};
            auto stream = e->CallObjectMethod(
                conn, method(e, conn, "getInputStream", "()Ljava/io/InputStream;"));
            check(e, "auth stream");
            auto bytes = e->NewByteArray(4096);
            check(e, "auth buffer");
            auto read = method(e, stream, "read", "([B)I");
            std::string body;
            while (true) {
                auto n = e->CallIntMethod(stream, read, bytes);
                check(e, "auth read");
                if (n < 0)
                    break;
                if (n == 0 || body.size() + size_t(n) > 1024 * 1024)
                    throw std::runtime_error("Invalid authentication response");
                char data[4096];
                e->GetByteArrayRegion(bytes, 0, n, reinterpret_cast<jbyte*>(data));
                check(e, "auth bytes");
                body.append(data, n);
            }
            e->CallVoidMethod(stream, method(e, stream, "close", "()V"));
            check(e, "auth close");
            auto json =
                e->NewObject(jsonClass, ctor(e, jsonClass, "(Ljava/lang/String;)V"), jstr(e, body));
            check(e, "auth JSON");
            auto ok =
                e->CallBooleanMethod(json, method(e, json, "optBoolean", "(Ljava/lang/String;Z)Z"),
                                     jstr(e, "ok"), JNI_FALSE);
            check(e, "auth result");
            if (!ok)
                return {{},
                        {},
                        {},
                        registration ? "Registration failed."
                                     : "Login failed. Check your email and password."};
            if (registration) {
                Result r;
                r.registered = true;
                return r;
            }
            auto get = [&](const char* key) {
                auto v = static_cast<jstring>(e->CallObjectMethod(
                    json,
                    method(e, json, "optString",
                           "(Ljava/lang/String;Ljava/lang/String;)Ljava/lang/String;"),
                    jstr(e, key), jstr(e, "")));
                check(e, "auth field");
                return string(e, v);
            };
            Result result{get("bbbId"), get("sessId"), get("serverIp"), {}};
            if (result.id.empty() || result.session.empty() || result.host.empty() ||
                result.host.find_first_of("/\\ \t\r\n") != std::string::npos)
                return {{}, {}, {}, "Authentication response is missing required fields."};
            return result;
        }
    } // namespace

    std::string endpoint() {
        std::string s = RAFT_AUTH_URL;
        if (s.rfind("https://", 0) != 0 && s.rfind("http://", 0) != 0)
            return {};
        if (s.find_first_of("?#\r\n") != std::string::npos)
            return {};
        if (s.size() >= 8 && s.substr(s.size() - 8) == "auth.php")
            return s;
        if (s.back() != '/')
            s += '/';
        return s + "auth.php";
    }

    std::future<Result> request(JNIEnv* e, std::string email, std::string password,
                                bool registration) {
        JavaVM* vm = nullptr;
        if (e->GetJavaVM(&vm) != JNI_OK)
            throw std::runtime_error("Java VM unavailable");
        auto local = cls(e, "org/json/JSONObject");
        auto jsonClass = static_cast<jclass>(e->NewGlobalRef(local));
        e->DeleteLocalRef(local);
        check(e, "auth JSON class");
        std::promise<Result> promise;
        auto future = promise.get_future();
        std::thread([vm, promise = std::move(promise), email = std::move(email),
                     password = std::move(password), registration, jsonClass]() mutable {
            JNIEnv* env = nullptr;
#ifdef __ANDROID__
            auto rc = vm->AttachCurrentThread(&env, nullptr);
#else
            auto rc = vm->AttachCurrentThread(reinterpret_cast<void**>(&env), nullptr);
#endif
            if (rc != JNI_OK) {
                promise.set_value({{}, {}, {}, "Authentication worker unavailable."});
                return;
            }
            Result r;
            try {
                r = fetch(env, email, password, registration, jsonClass);
            } catch (...) {
                if (env->ExceptionCheck())
                    env->ExceptionClear();
                r.error = "Could not contact the authentication server.";
            }
            env->DeleteGlobalRef(jsonClass);
            vm->DetachCurrentThread();
            promise.set_value(std::move(r));
        }).detach();
        return future;
    }
} // namespace rp::auth
