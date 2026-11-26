#include "rp/PortView.h"
#include "rp/SplashScreen.h"
#include "rp/TitleScreen.h"
#include <iostream>

int main(int argc, char** argv) {
    try {
        if (argc < 3 || argc > 4) {
            std::cerr << "Usage: raftpirates_preview ASSET_ROOT OUTPUT.png "
                         "[--map|--splash|--title|--menu]\n";
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
        rp::Game g;
        g.demo();
        rp::PortView v(argv[1]);
        v.grid = true;
        v.map = argc == 4 && std::string(argv[3]) == "--map";
        if (argc == 4 && std::string(argv[3]) == "--menu")
            v.touch(g, 580, 370);
        v.render(g, 0, "OFFLINE PREVIEW").savePNG(argv[2]);
        std::cout << g.summary() << '\n';
        return 0;
    } catch (const std::exception& e) {
        std::cerr << e.what() << '\n';
        return 2;
    }
}
