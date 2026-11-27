#include "DesktopAuth.h"
#include "sys/net/HttpClient.h"
#include <nlohmann/json.hpp>
#include <stdexcept>

namespace rp::desktop {
    using Result = sys::platform::AuthResult;

    namespace {
        std::string encode(const std::string& s) {
            constexpr char hex[] = "0123456789ABCDEF";
            std::string out;
            for (unsigned char c : s) {
                if ((c >= 'a' && c <= 'z') || (c >= 'A' && c <= 'Z') ||
                    (c >= '0' && c <= '9') || c == '-' || c == '_' || c == '.' || c == '~')
                    out += char(c);
                else {
                    out += '%';
                    out += hex[c >> 4];
                    out += hex[c & 15];
                }
            }
            return out;
        }

        Result fail(const std::string& message) {
            Result r;
            r.error = message;
            return r;
        }

        std::string field(const nlohmann::json& j, const char* key) {
            auto it = j.find(key);
            if (it == j.end() || it->is_null())
                return {};
            if (it->is_string())
                return it->get<std::string>();
            if (it->is_number_integer())
                return it->dump();
            return {};
        }

        Result request(const std::string& endpoint, const std::string& email,
                       const std::string& password, bool registration, int gameId,
                       const std::string& version, const std::string& caFile, bool allowHttp) {
            if (endpoint.empty() || endpoint.find_first_of("?#@\\ \r\n\t") != std::string::npos)
                return fail("Authentication server not configured.");
            const bool local = endpoint.rfind("http://127.0.0.1/", 0) == 0 ||
                endpoint.rfind("http://127.0.0.1:", 0) == 0 ||
                endpoint.rfind("http://localhost/", 0) == 0 ||
                endpoint.rfind("http://localhost:", 0) == 0 ||
                endpoint.rfind("http://[::1]/", 0) == 0 ||
                endpoint.rfind("http://[::1]:", 0) == 0;
            if (endpoint.rfind("https://", 0) != 0 &&
                !(endpoint.rfind("http://", 0) == 0 && (local || allowHttp)))
                return fail("Authentication requires HTTPS (except local test servers).");
            auto u = encode(email), p = encode(password);
            // Retain the original Raft Pirates auth endpoint's Android compatibility tag.
            auto address = endpoint + "?u=" + u +
                (registration ? "&u2=" + u + "&p=" + p + "&p2=" + p + "&c=1"
                              : "&p=" + p + "&ue=" + u + "&pe=" + p) +
                "&t=bbb&g=" + std::to_string(gameId) + "&lang=en&client_version=" +
                encode(version) + "&platform=android";
            auto response = sys::net::httpGet(address, 8000, 1024 * 1024, caFile);
            if (response.status != 200)
                return fail("Authentication server returned HTTP " + std::to_string(response.status) + ".");
            auto j = nlohmann::json::parse(response.body);
            if (!j.is_object() || !j.value("ok", false))
                return fail(registration ? "Registration failed." : "Login failed.");
            Result result;
            if (registration) {
                result.registered = true;
                return result;
            }
            result.id = field(j, "bbbId");
            result.session = field(j, "sessId");
            result.host = field(j, "serverIp");
            if (result.id.empty() || result.session.empty() || result.host.empty() ||
                result.host.find_first_of("/\\@?# \r\n\t") != std::string::npos)
                return fail("Authentication response missing required fields.");
            return result;
        }
    }

    std::future<Result> authenticate(std::string endpoint, std::string email,
        std::string password, bool registration, int gameId, std::string version,
        std::string caFile, bool allowHttp) {
        return std::async(std::launch::async,
            [endpoint = std::move(endpoint), email = std::move(email),
             password = std::move(password), registration, gameId,
             version = std::move(version), caFile = std::move(caFile), allowHttp] {
                try {
                    return request(endpoint, email, password, registration, gameId, version, caFile, allowHttp);
                } catch (...) {
                    return fail("Could not contact the authentication server.");
                }
            });
    }
}
