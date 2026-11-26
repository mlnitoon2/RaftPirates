#include "rp/PortView.h"
#include "rp/SplashScreen.h"
#include "rp/TitleScreen.h"
#include "rp/LoginScreen.h"
#include <iostream>

int main(int argc, char** argv) {
    try {
        if (argc < 3 || argc > 4) {
            std::cerr << "Usage: raftpirates_preview ASSET_ROOT OUTPUT.png "
                         "[--splash|--title|--login|--email|--registration]\n";
            return 1;
        }
        if (argc == 4 && std::string(argv[3]) == "--splash") {
            rp::SplashScreen splash(argv[1]);
            splash.tick(2.0f);
            splash.render().savePNG(argv[2]);
            return 0;
        }
        if (argc == 4 && std::string(argv[3]) == "--title") {
            rp::TitleScreen title(argv[1]);
            title.render().savePNG(argv[2]);
            return 0;
        }
        if (argc == 4 && (std::string(argv[3]) == "--login" || std::string(argv[3]) == "--email" ||
                          std::string(argv[3]) == "--registration")) {
            rp::LoginScreen login(argv[1]);
            for (int i = 0; i < 100; ++i)
                login.tick(.05f);
            auto click = [&](const char* name) {
                auto b = login.bounds(name);
                login.touchDown(b.x + b.w / 2, b.y + b.h / 2);
                login.touchUp(b.x + b.w / 2, b.y + b.h / 2);
            };
            if (std::string(argv[3]) != "--login")
                click("EmailButton");
            if (std::string(argv[3]) == "--registration")
                click("RegisterButton");
            std::cout << login.screen() << "\n";
            login.render().savePNG(argv[2]);
            return 0;
        }
        rp::TitleScreen title(argv[1]);
        title.render().savePNG(argv[2]);
        return 0;
    } catch (const std::exception& e) {
        std::cerr << e.what() << '\n';
        return 2;
    }
}
