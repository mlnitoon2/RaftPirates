#include "rp/LoginScreen.h"
#include <cstring>
#include <iostream>
#include <stdexcept>
#define CHECK(x)                                                                                   \
    do {                                                                                           \
        if (!(x))                                                                                  \
            throw std::runtime_error(#x);                                                          \
    } while (false)

int main(int argc, char** argv) {
    if (argc != 2)
        return 2;
    try {
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
