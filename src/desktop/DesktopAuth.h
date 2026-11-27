#pragma once
#include "sys/platform/Services.h"

namespace rp::desktop {
    std::future<sys::platform::AuthResult> authenticate(
        std::string endpoint, std::string email, std::string password, bool registration,
        int gameId, std::string version, std::string caFile, bool allowHttp);
}
