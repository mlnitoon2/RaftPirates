#include "rp/Game.h"
#include <iostream>
#include <stdexcept>
#include <zlib.h>
#define CHECK(x)                                                                                   \
    do {                                                                                           \
        if (!(x))                                                                                  \
            throw std::runtime_error(#x);                                                          \
    } while (0)
using O = sfs::SFSObjectWrapper;
using A = sfs::SFSArrayWrapper;

int main() {
    try {
        rp::RaftComponent construction;
        construction.bp = 100;
        construction.state = 1;
        construction.SetRemainingTime(3);
        CHECK(construction.Building() && !construction.PostBuild());
        CHECK(construction.GetBattlePoints() == 0 && construction.GetTotalBattlePoints(500) == 0);
        CHECK(!construction.TempTick(.5f) && construction.GetRemainingTime() == 2.5f);
        CHECK(construction.TempTick(2.5f) && construction.state == 1);
        construction.state = 5;
        CHECK(construction.Building() && construction.PostBuild());
        construction.state = 0;
        CHECK(!construction.Building() && construction.GetBattlePoints() == 100);
        CHECK(construction.GetTotalBattlePoints(500) == 500);
        rp::Game g;
        auto h = sfs::decodePacket(sfs::encodePacket(g.handshake()));
        CHECK(h.action == 0 && h.payload.getBool("bin"));
        O o;
        o.put("tk", "test");
        g.accept({0, 0, o});
        CHECK(g.token == "test");
        auto l = g.login("10", "testhash");
        CHECK(l.payload.getString("zn") == "RaftPirates" &&
              l.payload.getSFSObj("p")->getLong("last_updated") == 0);
        O obj;
        obj.put("name", "DivingUnit");
        obj.put("object_id", int32_t(3));
        obj.put("data_string", "1;xml_bin/anim_core_dive_unit.bin 0 0 0,0;");
        auto d = rp::readGridObject(obj);
        CHECK(d.type == 1 && d.cells.size() == 1);
        O rt;
        rt.put("id", int32_t(-42));
        rt.put("str", "abc");
        rt.put("bool", true);
        rt.put("data", sfs::Bytes{1, 2, 3});
        auto round = sfs::decodeObject(sfs::encodeObject(rt));
        CHECK(round.getInt("id") == -42 && round.getString("str") == "abc" &&
              round.typed<sfs::Bytes>("data").size() == 3);
        auto large = sfs::Bytes(70000, 1);
        rt.put("data", large);
        auto pkt = sfs::encodePacket({1, 12, rt});
        CHECK(pkt[0] == 0x88 &&
              sfs::decodePacket(pkt).payload.typed<sfs::Bytes>("data").size() == 70000);
        auto raw = sfs::encodeObject(rt);
        bool rejected = false;
        try {
            sfs::decodePacket({0x80, 0, 7, 18});
        } catch (...) {
            rejected = true;
        }
        CHECK(rejected);
        // Compressed frame, independently generated via zlib.
        O env;
        env.put("c", int8_t(1));
        env.put("a", int16_t(12));
        env.put("p", std::make_shared<O>(rt));
        auto bytes = sfs::encodeObject(env);
        sfs::Bytes zip(compressBound(bytes.size()));
        uLongf n = zip.size();
        CHECK(compress(zip.data(), &n, bytes.data(), bytes.size()) == Z_OK);
        zip.resize(n);
        sfs::Bytes frame{0xa0, uint8_t(n >> 8), uint8_t(n)};
        frame.insert(frame.end(), zip.begin(), zip.end());
        CHECK(sfs::decodePacket(frame).payload.getInt("id") == -42);
        g.demo();
        auto req = g.build(4, 1, 0, 0);
        CHECK(req.payload.getString("c") == "gs_build_add" &&
              req.payload.getSFSObj("p")->getInt("parent_id") == 1);
        O reply;
        auto component = std::make_shared<O>();
        component->put("user_raft_component_id", int32_t(1000));
        component->put("current_bp", int32_t(500));
        reply.put("user_raft_component", component);
        g.extension("gs_build_add", reply);
        CHECK(!g.pending && g.rafts[0].components.size() == 3);
        rejected = false;
        try {
            g.build(4, 1, 0, 0);
        } catch (...) {
            rejected = true;
        }
        CHECK(rejected);
        CHECK(g.dive("dive_start").payload.getSFSObj("p")->getInt("object_id") == 2);
        O rem;
        rem.put("object_type", int32_t(1));
        rem.put("parent_id", int32_t(1));
        rem.put("object_id", int32_t(1000));
        g.extension("gs_build_remove", rem);
        CHECK(g.rafts[0].components.size() == 2);
        O materials;
        auto ar = std::make_shared<A>();
        auto x = std::make_shared<O>();
        x->put("id", int32_t(4));
        x->put("amt", int32_t(5));
        ar->add(x);
        materials.put("materials_changed", ar);
        g.extension("gs_materials_changed", materials);
        CHECK(g.materials[4] == 100004);
        auto move = g.moveMap(200, -200, false);
        auto mp = move.payload.getSFSObj("p");
        CHECK(mp->getInt("map_x") == 95 && mp->getInt("map_y") == -95 && !mp->getBool("safe") &&
              mp->getInt("enemy_id") == 0);
        O ocean;
        ocean.put("safe_water", false);
        g.extension("gs_ocean", ocean);
        CHECK(g.mapX == 95 && g.mapY == -95 && !g.safe && !g.pendingMap);
        g.discoveries[1] = "Test";
        auto discovery = g.discover(1);
        CHECK(discovery.payload.getString("c") == "discovery_goto");
        O result;
        result.put("success", int32_t(1));
        result.put("discoveryid", int32_t(1));
        g.extension("discovery_goto_result", result);
        CHECK(g.discovery == 1 && !g.pendingDiscovery);
        O dive;
        dive.put("time_remaining_secs", int32_t(3));
        g.extension("dive_info", dive);
        g.tick(.5);
        g.tick(.5);
        CHECK(g.diveSeconds == 2);
        obj.put("data_string", "1;first.bin 0 0 0,0 1,0;second.bin 8 9 1,0 2,0;");
        auto multi = rp::readGridObject(obj);
        CHECK(multi.pieces.size() == 2 && multi.cells.size() == 3);
        O shop;
        auto items = std::make_shared<A>();
        auto item = std::make_shared<O>();
        item->put("storeitem_id", int32_t(12));
        item->put("item_name", "random_pack_sm");
        item->put("group_name", "misc");
        item->put("currency_id", int32_t(4));
        item->put("price", int32_t(100));
        items->add(item);
        shop.put("store_item_data", items);
        g.extension("lib_store_item_data", shop);
        CHECK(g.storeItems.at(12).price == 100 &&
              g.buyStoreItem(12).payload.getSFSObj("p")->getString("itemname") == "random_pack_sm");
        O state;
        state.put("level", int32_t(5));
        state.put("timeremaining", int32_t(30));
        state.put("totaltime", int32_t(60));
        state.put("paused", int32_t(1));
        g.extension("daring_state", state);
        CHECK(g.daring == 5 && g.daringRemaining == 30 && g.daringPaused);
        g.tick(10);
        CHECK(g.daringRemaining == 30);
        O resume;
        resume.put("paused", int32_t(0));
        g.extension("daring_state", resume);
        g.tick(.5f);
        g.tick(.5f);
        CHECK(g.daringRemaining == 29);
        g.tick(28.5f);
        CHECK(g.daringRemaining == 1);
        g.tick(1);
        CHECK(g.daringRemaining == 0);
        CHECK(g.daring == 5);
        O fail;
        fail.put("success", int32_t(0));
        fail.put("level", int32_t(6));
        g.extension("daring_advanceresult", fail);
        CHECK(g.daring == 5);
        dive.put("depth", int32_t(2));
        dive.put("max_depth", int32_t(10));
        dive.put("state", int32_t(3));
        g.extension("dive_info", dive);
        CHECK(g.diveState.depth == 2 && g.diveState.maxDepth == 10 && g.diveState.state == 3);
        rp::Raft health;
        health.type = 1;
        health.bp = 50;
        health.bonusBattlePoints = 3;
        health.components.push_back({1, 2, 0, 0, 25, 0, 0});
        health.components.push_back({2, 2, 1, 0, 25, 1, 5});
        std::map<int, rp::GridObject> definitions;
        definitions[1].bp = 100;
        definitions[2].bp = 50;
        CHECK(health.GetBattlePoints() == 78 && health.GetTotalBattlePoints(definitions) == 150);
        CHECK(std::abs(health.GetHealth(definitions) - .5f) < .001f);
        health.state = 2;
        CHECK(health.Destroyed());
        std::cout << "Raft model and SFS framing tests passed\n";
        return 0;
    } catch (const std::exception& e) {
        std::cerr << e.what() << '\n';
        return 1;
    }
}
