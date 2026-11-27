#include "rp/LoginScreen.h"
#include "rp/TitleScreen.h"
#include "sys/platform/Services.h"
#include <cstring>
#include <iostream>
#include <stdexcept>
#define CHECK(x)                                                                                   \
    do {                                                                                           \
        if (!(x))                                                                                  \
            throw std::runtime_error(std::string(#x) + " at " + std::to_string(__LINE__));         \
    } while (false)

namespace {
    using namespace sys::platform;

    struct StartupPlatform : Services {
        std::filesystem::path root;
        sys::gfx::Image frame;
        unsigned connects = 0, auths = 0;

        explicit StartupPlatform(std::filesystem::path p) : root(std::move(p)) {}

        std::filesystem::path assets(const std::filesystem::path&) override {
            return root;
        }

        std::filesystem::path saveDirectory() override {
            return {};
        }

        void present(const sys::gfx::Image& image, int, int) override {
            frame = image;
        }

        void music(const std::string&) override {}

        void pauseMusic(bool) override {}

        void effect(const std::string&) override {}

        void showPopup(const std::string&) override {}

        void openWebPage(const std::string&) override {}

        bool loggedIn() override {
            return false;
        }

        bool transportConnected() override {
            return false;
        }

        void keyboard(const std::string&, const std::string&, bool) override {}

        std::future<AuthResult> authenticate(std::string, std::string, std::string, bool, int,
                                             std::string) override {
            ++auths;
            return {};
        }

        void connect(const std::string&, int) override {
            ++connects;
        }

        void login(const std::string&, const std::string&, const std::string&,
                   const sfs::SFSObjectWrapper&) override {}

        void send(const sfs::Message&) override {}

        void disconnect() override {}

        void keepAlive() override {}

        void log(const std::string&) override {}
    };

    bool sameImage(const sys::gfx::Image& a, const sys::gfx::Image& b) {
        return a.width == b.width && a.height == b.height && a.pixels.size() == b.pixels.size() &&
               std::memcmp(a.pixels.data(), b.pixels.data(),
                           a.pixels.size() * sizeof(a.pixels[0])) == 0;
    }

    void startup(const std::filesystem::path& root) {
        StartupPlatform platform(root);
        const auto expected = rp::TitleScreen(root).render();
        game::init(platform, "test.apk", 1280, 800);
        game::touchDown(platform, 640, 730);
        game::touchUp(platform, 640, 730);
        game::tick(platform);
        CHECK(!sameImage(expected, platform.frame));
        game::touchUp(platform, 640, 730);
        game::tick(platform);
        CHECK(!sameImage(expected, platform.frame));
        game::touchDown(platform, 640, 730);
        game::pause(platform, true);
        game::pause(platform, false);
        game::touchUp(platform, 640, 730);
        game::tick(platform);
        CHECK(!sameImage(expected, platform.frame));
        for (int i = 0; i < 10; ++i)
            game::tick(platform);
        CHECK(!sameImage(expected, platform.frame));
        CHECK(platform.connects == 0 && platform.auths == 0);
        game::touchDown(platform, 640, 730);
        game::touchUp(platform, 640, 730);
        game::tick(platform);
        CHECK(!sameImage(expected, platform.frame));
        game::kill(platform);
        game::init(platform, "test.apk", 1280, 800);
        game::touchUp(platform, 640, 730);
        game::tick(platform);
        CHECK(!sameImage(expected, platform.frame));
        game::kill(platform);
    }
} // namespace

int main(int argc, char** argv) {
    if (argc != 2)
        return 2;
    try {
        startup(argv[1]);
        using A = rp::LoginScreen::Action;
        rp::LoginScreen login(argv[1]);
        for (int i = 0; i < 100; ++i)
            login.tick(.05f);
        CHECK(login.screen() == "login_menu");
        auto tap = [&](const char* name) {
            auto b = login.bounds(name);
            CHECK(b.w > 0 && b.h > 0);
            auto x = b.x + b.w / 2, y = b.y + b.h / 2;
            login.touchDown(x, y);
            return login.touchUp(x, y);
        };
        tap("EmailButton");
        CHECK(login.screen() == "login_email");
        CHECK(tap("EmailInput") == A::Email);
        login.setInput(A::Email, "test@local.invalid");
        CHECK(tap("PasswordInput") == A::Password);
        login.setInput(A::Password, "alpha");
        auto a = login.render();
        login.setInput(A::Password, "bravo");
        auto b = login.render();
        CHECK(a.pixels.size() == b.pixels.size());
        CHECK(std::memcmp(a.pixels.data(), b.pixels.data(),
                          a.pixels.size() * sizeof(a.pixels[0])) == 0);
        CHECK(tap("password_forgot") == A::Reset);
        CHECK(tap("SubmitButton") == A::Submit);
        login.clearPassword();
        login.busy(true);
        login.tick(.1f);
        login.error("Login failed.");
        CHECK(login.screen() == "status_popup");
        tap("OK");
        CHECK(login.screen() == "login_email");
        tap("RegisterButton");
        CHECK(login.screen() == "login_registration");
        CHECK(tap("Password2Input") == A::ConfirmPassword);
        CHECK(tap("SubmitButton") == A::Register);
        tap("BackButton");
        CHECK(login.screen() == "login_email");
        login.busy(true);
        login.finish();
        for (int i = 0; i < 100 && !login.finished(); ++i)
            login.tick(.05f);
        CHECK(login.finished());
        std::cout
            << "Original loading, email, registration, password masking and status popup passed\n";
    } catch (const std::exception& e) {
        std::cerr << e.what() << '\n';
        return 1;
    }
}
