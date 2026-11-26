#include "rp/Hud.h"
#include "rp/PortView.h"
#include <iostream>
#include <stdexcept>
#define CHECK(x)                                                                                   \
    do {                                                                                           \
        if (!(x))                                                                                  \
            throw std::runtime_error(#x);                                                          \
    } while (false)

int main(int argc, char** argv) {
    if (argc != 2) {
        std::cerr << "Usage: raftpirates_hud_tests ASSET_ROOT\n";
        return 2;
    }
    try {
        rp::Game g;
        g.demo();
        g.daring = 4;
        g.daringTotal = 120;
        g.daringRemaining = 60;
        CHECK(g.daringLocked() && g.daringProgress() == 0 && !g.canAdvanceDaring());
        rp::Hud hud(argv[1]);
        hud.update(g);
        const auto test = [&](const char* name, rp::Hud::Action action) {
            auto bounds = hud.bounds(name);
            CHECK(bounds.w > 0 && bounds.h > 0);
            CHECK(hud.touch(bounds.x + bounds.w / 2, bounds.y + bounds.h / 2) == action);
        };
        test("QuestMenu", rp::Hud::Action::Quest);
        test("SafeFlag", rp::Hud::Action::Safety);
        test("DaringMeter", rp::Hud::Action::Daring);
        test("EventNotification", rp::Hud::Action::ToggleView);
        test("MainMenu", rp::Hud::Action::Shop);
        test("PirateMap", rp::Hud::Action::Map);
        g.safe = false;
        g.badZone = true;
        hud.update(g);
        test("BadZoneFlag", rp::Hud::Action::Safety);
        sys::gfx::Image img(640, 400);
        hud.render(img);
        CHECK(img.at(320, 30).a > 0);
        rp::PortView view(argv[1]);
        g.badZone = false;
        CHECK(!g.daringLocked() && g.daringProgress() == .5 && !g.canAdvanceDaring());
        g.daringRemaining = 0;
        CHECK(g.canAdvanceDaring());
        auto bounds = hud.bounds("DaringMeter");
        auto req = view.touch(g, bounds.x + bounds.w / 2, bounds.y + bounds.h / 2);
        CHECK(req && req->payload.getString("c") == "daring_advance");
        bounds = hud.bounds("QuestMenu");
        req = view.touch(g, bounds.x + bounds.w / 2, bounds.y + bounds.h / 2);
        CHECK(req && req->payload.getString("c") == "gs_quest");
        view.touch(g, 30, 310);
        bounds = hud.bounds("EventNotification");
        view.touch(g, bounds.x + bounds.w / 2, bounds.y + bounds.h / 2);
        CHECK(view.grid);
        bounds = hud.bounds("PirateMap");
        view.touch(g, bounds.x + bounds.w / 2, bounds.y + bounds.h / 2);
        CHECK(view.map);
        std::cout << "Original HUD asset, action, and protocol checks passed\n";
        return 0;
    } catch (const std::exception& e) {
        std::cerr << e.what() << '\n';
        return 1;
    }
}
