#pragma once
#include <jni.h>
#include <future>
#include <string>

namespace rp::auth {
    struct Result {
        std::string id, session, host, error;
        bool registered = false;
    };

    std::string endpoint();
    std::future<Result> request(JNIEnv*, std::string email, std::string password,
                                bool registration = false);
} // namespace rp::auth
