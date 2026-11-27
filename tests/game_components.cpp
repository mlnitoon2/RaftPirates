#include "rp/BuildModeState.h"
#include "rp/GridSprite.h"
#include "rp/GridObjectHelpers.h"
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
        {
            Game dive;
            dive.ready = true;
            dive.diveState.parent = 1;
            dive.diveState.object = 2;
            dive.diveState.state = 3;
            dive.diveState.bonus[9] = 2;
            dive.diveState.collected[9] = 1;
            BuildModeState first(dive), second(dive);
            CHECK(first.SendBonusMaterials().has_value());
            CHECK(!second.SendBonusMaterials());
            dive.extension("gs_error", {});
            CHECK(second.SendBonusMaterials().has_value());
            dive.extension("gs_error", {});
            dive.diveState.collected.clear();
            CHECK(!BuildModeState(dive).SendBonusMaterials());
            dive.diveState.remaining = 1;
            CHECK(!dive.collectDive());
            dive.diveState.remaining = 0;
            CHECK(dive.collectDive());
            CHECK(!dive.collectDive());
            dive.extension("gs_materials_changed", {});
            CHECK(dive.diveCollectPending && !dive.collectDive());
            O info;
            info.put("parent_id", int32_t(1));
            info.put("object_id", int32_t(2));
            info.put("state", int32_t(3));
            info.put("time_remaining_secs", int32_t(0));
            dive.extension("dive_info", info);
            CHECK(dive.diveCollectPending);
            O result;
            result.put("diver", true);
            dive.extension("gs_materials_changed", result);
            CHECK(!dive.diveCollectPending && !dive.diveState.Diving());
            dive.pending = PendingBuild{1, 2, 3, 4};
            dive.pendingZone = 3;
            dive.sonarPending = true;
            dive.requestSendFailed("ping_sonar");
            CHECK(!dive.sonarPending && dive.pending && dive.pendingZone);
            dive.requestSendFailed("gs_build_add");
            CHECK(!dive.pending && dive.pendingZone);
            dive.requestSendFailed("zone_goto");
            CHECK(!dive.pendingZone);
        }
        {
            Game shop;
            StoreItem gold;
            gold.id = 1;
            gold.name = "gold_test";
            gold.group = "gold";
            StoreItem diamond;
            diamond.id = 2;
            diamond.name = "diamond_test";
            diamond.group = "diamond";
            StoreItem offer;
            offer.id = 3;
            offer.name = "offer_test";
            offer.group = "offerwall";
            shop.storeItems = {{1, gold}, {2, diamond}, {3, offer}};
            RPMenuHandler menu(shop);
            CHECK(menu.storeItemsForTab("tabbed_menu_diamonds") == std::vector<int>({2}));
            CHECK(menu.storeItemsForTab("tabbed_menu_gold") == std::vector<int>({1, 3}));
        }
        {
            std::string tokens = "alpha;;omega";
            CHECK(Tokenize(tokens, ";") == "alpha");
            CHECK(Tokenize(tokens, ";").empty());
            CHECK(Tokenize(tokens, ";") == "omega" && tokens.empty());
            GridObject x(1, 7, 1, 2, 0, 0), y(2, 7, 1, 3, 0, 0);
            std::map<int, GridObjectInfo> definitions;
            definitions[7].type = 1;
            definitions[7].name = "DivingUnit";
            CHECK(IsComponent(&x, definitions) && IsDivingUnit(&x, definitions));
            CHECK(!IsRaft(&x, definitions) && !IsWreckage(nullptr, definitions));
            CHECK(SortGridObjPredicate(x, y));
            definitions[7].name = "boss05";
            CHECK(IsBossWeapon(&x, definitions) && !IsBarracks(&x, definitions));
            definitions[7].name = "barracks";
            CHECK(IsBarracks(&x, definitions) && !IsBossWeapon(&x, definitions));
            GridObjectPiece shape("a.png 5 7 0,0");
            std::vector<GridObject::GridShapePiece> pieces;
            pieces.emplace_back(y, shape, 0);
            pieces.emplace_back(x, shape, 1);
            CHECK(pieces[1].Filename() == "a.png");
            CHECK(pieces[1].GetWorldSpace(0).second == 2);
            std::vector<GridObject::GridShapePiece*> order;
            performaIsoSort(pieces, order);
            CHECK(order.size() == 2 && order.front() == &pieces[1]);
        }
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
        MsgStoreDisplayStatus storeStatus;
        storeStatus.visible = true;
        menu.gotMsgStoreDisplayStatus(storeStatus);
        CHECK(menu.storeVisible());
        menu.gotMsgPopPopUpGlobal();
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
