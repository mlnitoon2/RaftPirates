#include "rp/RPMenuHandler.h"
#include "rp/SparseGrid.h"
#include "rp/LocalSession.h"
#include "rp/BuildModeState.h"
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
        rp::RPMenuHandler menu(g);
        CHECK(!menu.canPurchase(2));
        CHECK(menu.canPurchase(1));
        CHECK(g.canBuildRaft({1, 0}) && !g.canBuildRaft({0, 0}) && !g.canBuildRaft({3, 3}));
        session.dispatch(g, g.build(5, 0, 1, 0));
        CHECK(g.rafts.size() == 2 && g.RaftsConnectedToRaft(1).size() == 2);
        CHECK(g.getResourceFiles().count("gfx/rafts/raft_wood.png"));
        session.dispatch(g, rp::Game::request("gs_quest"));
        CHECK(g.quests.size() == 1 && g.quests.at(1).unread);
        auto read = g.openQuest(1);
        CHECK(read.payload.getString("c") == "gs_quest_read" &&
              read.payload.getSFSObj("p")->getInt("quest_id") == 1);
        session.dispatch(g, read);
        CHECK(!g.quests.at(1).unread);
        sfs::SFSObjectWrapper update;
        update.put("quest_logid", int32_t(1));
        update.put("progress", int32_t(80));
        g.extension("quest_progress", update);
        CHECK(g.quests.at(1).progress == 80);
        rp::SparseGrid sparse;
        CHECK(sparse.addObject(1, {{0, 0}, {1, 0}}, {0, 0}));
        CHECK(sparse.addObject(2, {{0, 0}}, {3, 0}));
        CHECK(!sparse.moveObject(1, {2, 0}));
        CHECK(sparse.getObjectAt({0, 0}) == 1 && sparse.getObjectAt({3, 0}) == 2);
        CHECK(sparse.moveObject(1, {-2, -3}));
        CHECK(sparse.getObjectAt({-1, -3}) == 1 && !sparse.getObjectAt({0, 0}));
        CHECK(sparse.removeObject(1) && !sparse.hasObject(1));
        int gold = g.materials[4];
        session.dispatch(g, g.buyStoreItem(1));
        CHECK(g.materials[4] == gold - 100 && g.materials[2] == 1100);
        session.dispatch(g, g.build(4, 1, 0, 0));
        CHECK(g.rafts.front().components.size() == 3);
        auto id = g.rafts.front().components.back().id;
        auto& part = g.rafts.front().components.back();
        part.Repair(300, 1000);
        rp::BuildModeState build(g);
        build.selectObject(1, id);
        auto repair = build.repairSelected(false);
        CHECK(repair && repair->payload.getString("c") == "gs_build_repair");
        session.dispatch(g, *repair);
        CHECK(part.bp == 1000);
        part.SetRemainingTime(10);
        part.SetState(1);
        auto clock = build.UseClockSelected();
        CHECK(clock && clock->payload.getString("c") == "gs_use_clock");
        session.dispatch(g, *clock);
        CHECK(!part.Building() && part.GetRemainingTime() == 0);
        build.deselectObject();
        CHECK(!build.UseClockSelected());
        session.dispatch(g, g.moveComponent(1, id, 1, 3, 3));
        CHECK(g.rafts.front().components.back().x == 3);
        session.dispatch(g, g.remove(1, id));
        CHECK(g.rafts.front().components.size() == 2);
        session.dispatch(g, g.travel(2));
        CHECK(g.zone == 2);
        session.dispatch(g, g.moveMap(25, 30, false));
        CHECK(g.mapX == 25 && g.mapY == 30);
        if (argc == 2) {
            rp::MapView map;
            map.setMapFile(argv[1], g.zoneMaps.at(g.zone));
            CHECK(map.data());
            bool land = false, water = false;
            for (int y = 0; y < map.data()->height; ++y)
                for (int x = 0; x < map.data()->width; ++x) {
                    float wx = (x + .5f) * map.data()->tileWidth,
                          wy = (y + .5f) * map.data()->tileHeight;
                    if (map.isPosOnLand(wx, wy)) {
                        land = true;
                        CHECK(!map.isValidIconLocation(wx, wy));
                    } else if (map.isValidIconLocation(wx, wy))
                        water = true;
                }
            CHECK(land && water);
            CHECK(!map.isValidIconLocation(-1, 0));
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
