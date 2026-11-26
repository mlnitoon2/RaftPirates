#include "rp/BuildModeState.h"
#include "rp/TouchControls.h"
#include "rp/RPMenuHandler.h"
#include "rp/LocalSession.h"
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
        LocalSession local;
        local.start(game);
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
        local.dispatch(game, request);
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
        local.dispatch(game, *add);
        CHECK(game.rafts.size() == 2);
        auto& c = game.rafts.front().components.front();
        c.bp = 0;
        c.state = 2;
        build.initWreckedRaftComponentCheck();
        if (c.Destroyed())
            CHECK(build.selection().second == c.id);
        OceanView ocean({});
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
