#include "rp/LocalSession.h"
#include "rp/PortView.h"
#include <iostream>
#include <stdexcept>
#define CHECK(x)                                                                                   \
    do {                                                                                           \
        if (!(x))                                                                                  \
            throw std::runtime_error(#x);                                                          \
    } while (false)

int main(int argc, char** argv) {
    try {
        rp::Game g;
        rp::LocalSession session;
        session.start(g);
        CHECK(g.ready && g.mainRaft() && g.currentBP() == 3000 && g.hasMaterial(4, 100));
        CHECK(g.GetRaftAtPosition({0, 0}) && g.GetRaftByID(1));
        session.dispatch(g, g.buyStoreItem(2));
        CHECK(g.hasBlueprint("Boom-Boom Buddha"));
        int gold = g.materials[4];
        session.dispatch(g, g.buyStoreItem(1));
        CHECK(g.materials[4] == gold - 100 && g.materials[2] == 1100);
        session.dispatch(g, g.build(4, 1, 0, 0));
        CHECK(g.rafts.front().components.size() == 3);
        auto id = g.rafts.front().components.back().id;
        session.dispatch(g, g.moveComponent(1, id, 1, 3, 3));
        CHECK(g.rafts.front().components.back().x == 3);
        session.dispatch(g, g.remove(1, id));
        CHECK(g.rafts.front().components.size() == 2);
        session.dispatch(g, g.travel(2));
        CHECK(g.zone == 2);
        session.dispatch(g, g.moveMap(25, 30, false));
        CHECK(g.mapX == 25 && g.mapY == 30);
        if (argc == 2) {
            rp::PortView view(argv[1]);
            auto req = view.touch(g, 250, 372);
            CHECK(req && req->payload.getString("c") == "dive_start");
            session.dispatch(g, *req);
            CHECK(g.diveState.state == 3);
            auto img = view.render(g, 0, "LOCAL SANDBOX");
            CHECK(img.at(0, 0).b == 60);
            view.touchDown(250, 355);
            for (int i = 0; i < 40; ++i)
                view.tick(.05f);
            view.touchDrag(380, 355);
            for (int i = 0; i < 20; ++i)
                view.tick(.05f);
            view.touch(g, 380, 355);
            view.render(g, 3, "LOCAL SANDBOX");
            view.touch(g, 320, 355);
            req = view.touch(g, 320, 355);
            CHECK(req && req->payload.getString("c") == "dive_collect");
            session.dispatch(g, *req);
        } else {
            session.dispatch(g, g.dive("dive_start"));
            session.dispatch(g, g.dive("dive_collect"));
        }
        CHECK(g.diveState.state == 0 && g.materials[2] == 1150);
        g.daringRemaining = 0;
        session.dispatch(g, rp::Game::request("daring_advance"));
        CHECK(g.daring == 2 && g.daringRemaining == 60);
        std::cout << "Local build, move, remove, purchase, travel, diving and progression passed\n";
        return 0;
    } catch (const std::exception& e) {
        std::cerr << e.what() << '\n';
        return 1;
    }
}
