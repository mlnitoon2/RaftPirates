#include "rp/BuildModeState.h"
#include "rp/GridSprite.h"
#include "rp/TouchControls.h"
#include "rp/RPMenuHandler.h"
#include "PlayerFixture.h"
#include "rp/MapView.h"
#include <cmath>
#include <iostream>
#include <stdexcept>
#define CHECK(x)                                                                                   \
    do {                                                                                           \
        if (!(x))                                                                                  \
            throw std::runtime_error(#x);                                                          \
    } while (false)
using namespace rp;
using O = sfs::SFSObjectWrapper;
using A = sfs::SFSArrayWrapper;

int main() {
    try {
        Game game;
        loadPlayerFixture(game);
        game.storeItems[1] = {1, 4, 100, 1, "pack", "misc", "Pack", "rp_menu.xml", ""};
        game.objects[5] = {5, 0, 1000, "WoodRaft", "gfx/rafts/raft_wood.png", -192, 0, {{0, 0}}};
        game.blueprints[3] = {3, 5, 1, "Wooden Raft"};
        game.owned.insert(3);
        RPMenuHandler menu(game);
        menu.gotMsgSetStoreCategory("misc");
        CHECK(menu.currentStorePage(1) == std::vector<int>{1});
        menu.gotMsgSetStoreItemPage(1);
        CHECK(menu.currentStorePage(1).empty());
        menu.gotMsgSetStoreItemSelected(1);
        CHECK(menu.gotMsgStoreItemSelectedRequest()->id == 1);
        menu.gotMsgSetBlueprintSelected(3);
        CHECK(menu.gotMsgRecipeSelectedRequest() == game.blueprints.at(3).recipe);
        menu.pushPopUp("A");
        menu.gotMsgQueueMenuPopup("B");
        menu.tick(1);
        CHECK(menu.topPopUp() == "A");
        menu.popPopUp();
        menu.tick(1);
        CHECK(menu.topPopUp() == "B" && !menu.hasAnyQueuedPopups());
        menu.destroyCurMenuData();
        CHECK(menu.topPopUp().empty());
        O packet;
        auto rows = std::make_shared<A>();
        auto item = std::make_shared<O>();
        item->put("storeitem_id", int32_t(12));
        item->put("item_name", "pack");
        item->put("price", int32_t(10));
        rows->add(item);
        packet.put("store_item_data", rows);
        game.extension("lib_store_item_data", packet);
        CHECK(game.storeDB.getStoreInfo("pack")->id == 12);
        rows->add(item);
        bool rejected = false;
        try {
            game.extension("lib_store_item_data", packet);
        } catch (...) {
            rejected = true;
        }
        CHECK(rejected && game.storeItems.size() == 1 && game.storeDB.items.size() == 1);
        auto request = game.dive("dive_start");
        CHECK(request.payload.getString("c") == "dive_start");
        O dive;
        dive.put("state", int32_t(3));
        dive.put("time_remaining_secs", int32_t(20));
        dive.put("time_total_secs", int32_t(20));
        dive.put("max_load", int32_t(200));
        game.extension("dive_info", dive);
        CHECK(game.diveState.Diving() && game.diveState.GetMaxLoad() == 200);
        game.tick(5);
        CHECK(game.diveSeconds == 15 && std::abs(game.diveState.GetDiveTime() - .25f) < .001f);
        game.diveState.SetLoadValue(300);
        CHECK(game.diveState.GetLoadValue() == 200);
        game.diveState.bonus[2] = 3;
        game.diveState.collected[2] = 2;
        CHECK(game.diveState.numAvailableBonusMaterials() == 3 &&
              game.diveState.numCollectedBonusMaterials() == 2);
        BuildModeState build(game);
        build.selectObject(0, game.mainRaft()->id);
        CHECK(!build.gotMsgDestroyObjectRequest());
        build.StartWaiting("test");
        CHECK(!build.gotMsgBuildConfirm(3, 0, {1, 0}));
        build.FinishWaiting("test");
        auto spots = game.GetAvailableEdgeGridSpace();
        CHECK(!spots.empty());
        auto add = build.gotMsgBuildConfirm(3, 0, spots.front());
        CHECK(add);
        O buildResult;
        auto raft = std::make_shared<O>();
        raft->put("user_raft_id", int32_t(10));
        raft->put("current_bp", int32_t(1000));
        buildResult.put("user_raft", raft);
        game.extension("gs_build_add", buildResult);
        CHECK(game.rafts.size() == 2);
        CHECK(build.CheckForHoles(game.mainRaft()->id));
        CHECK(!build.CheckForHoles(10));
        auto& c = game.rafts.front().components.front();
        c.bp = 500;
        build.gotMsgGridObjectSelected(game.rafts.front().id, c.id);
        auto repair = build.gotMsgRepairSelectedObject();
        CHECK(repair && repair->payload.getString("c") == "gs_build_repair");
        auto actions = build.SendMsgSelectedObjectAvailableActions();
        CHECK(std::find(actions.begin(), actions.end(), "repair") != actions.end());
        c.bp = 0;
        c.state = 2;
        build.initWreckedRaftComponentCheck();
        if (c.Destroyed())
            CHECK(build.selection().second == c.id);
        OceanView ocean({});
        ocean.SetOcean(game);
        const auto firstRaft = game.mainRaft()->id;
        auto originalSprite = ocean.getGridSprite(0, firstRaft);
        CHECK(originalSprite && ocean.spriteCount() >= game.rafts.size());
        ocean.gotMsgUpdateOceanView(game);
        CHECK(ocean.getGridSprite(0, firstRaft) == originalSprite);
        ocean.SetRaftTransparent(firstRaft, .5f);
        CHECK(originalSprite->Alpha() <= 128);
        CHECK(ocean.gotMsgGfxLayerPickResult(0, firstRaft));
        ocean.RemoveGrid(firstRaft);
        CHECK(!ocean.getGridSprite(0, firstRaft) && !ocean.pickedSprite());
        ocean.SetAxiseseses({100, 20}, {-80, 60});
        auto inverse = ocean.ScreenToWorld(ocean.WorldToScreen({2, 3}));
        CHECK(std::abs(inverse.x - 2) < .001f && std::abs(inverse.y - 3) < .001f);
        ocean.SetUnitSize(192);
        TouchControls touch(&ocean);
        touch.SetZoomLimits(.5f, 2.f, true);
        touch.touchDown({100, 100});
        touch.touchDrag({130, 120});
        touch.touchUp({130, 120});
        CHECK(touch.consumeDrag());
        auto old = ocean.ScreenToWorld({200, 150});
        touch.onZoom(1.5f, {200, 150});
        auto now = ocean.ScreenToWorld({200, 150});
        CHECK(std::abs(old.x - now.x) < .001f && std::abs(old.y - now.y) < .001f);
        touch.PanTo({3, 4}, 1, 1);
        touch.tick(1);
        CHECK(ocean.Offset().x == 3 && ocean.Offset().y == 4);
        touch.SetLockedView(true);
        touch.touchDown({0, 0});
        touch.touchDrag({100, 100});
        CHECK(ocean.Offset().x == 3);
        std::cout << "Game component tests passed\n";
        return 0;
    } catch (const std::exception& e) {
        std::cerr << e.what() << "\n";
        return 1;
    }
}
