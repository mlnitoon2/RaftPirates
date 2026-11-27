#include "rp/ScrollingLayer.h"
#include "rp/DamageIndicatorSprite.h"
#include "rp/OverlapTree.h"
#include "rp/PerimeterTree.h"
#include "rp/GridObjectDataRes.h"
#include "rp/LoaderImage.h"
#include "rp/LoaderMenu.h"
#include "rp/LoaderManifest.h"
#include "rp/Game.h"
#include "rp/GridSprite.h"
#include "rp/LoadingManager.h"
#include "rp/MapView.h"
#include "rp/RPMenuHandler.h"
#include <chrono>
#include <filesystem>
#include <iostream>
#include <stdexcept>
#define CHECK(x)                                                                                   \
    do {                                                                                           \
        if (!(x))                                                                                  \
            throw std::runtime_error(#x);                                                          \
    } while (false)
using O = sfs::SFSObjectWrapper;
using A = sfs::SFSArrayWrapper;

O library(const char* name, const O& row) {
    O packet;
    auto a = std::make_shared<A>();
    a->add(std::make_shared<O>(row));
    packet.put(name, a);
    return packet;
}

int main(int argc, char** argv) {
    auto cache = std::filesystem::temp_directory_path() /
                 ("rp-library-" +
                  std::to_string(std::chrono::steady_clock::now().time_since_epoch().count()));
    try {
        rp::Game game;
        O mat;
        mat.put("material_id", int32_t(2));
        mat.put("name", "Wood");
        mat.put("gold_cost", 2.25);
        game.extension("lib_mat_data", library("material_data", mat));
        CHECK(game.materialDB.getMaterialInfo("Wood")->goldCost.value() == 2.25);
        O bp;
        bp.put("blueprint_id", int32_t(7));
        bp.put("object_id", int32_t(9));
        bp.put("name", "Wooden Raft");
        bp.put("material_1", int32_t(2));
        bp.put("material_1_amount", int32_t(5));
        auto packet = library("blueprint_data", bp);
        game.extension("lib_bp_data", packet);
        CHECK(game.blueprintDB.getBlueprintInfoFromGridObjectID(9)->id() == 7);
        CHECK(game.blueprintDB.getBlueprintInfo("Wooden Raft")->recipe.at(2) == 5);
        game.materials[2] = 2;
        CHECK(game.goldCostToBuild(7) == 7);
        CHECK(!game.goldCostToBuild(999));
        game.blueprintDB.save(cache);
        rp::BlueprintsDB restored;
        restored.load(cache);
        CHECK(restored.getAllBlueprintsAvail() == std::vector<int>{7});
        auto arr = packet.getSFSArray("blueprint_data");
        arr->add(std::make_shared<O>(bp));
        bool rejected = false;
        try {
            game.extension("lib_bp_data", packet);
        } catch (const std::runtime_error&) {
            rejected = true;
        }
        CHECK(rejected && game.blueprints.size() == 1 && game.blueprintDB.getBlueprintInfo(7));
        O go;
        go.put("object_id", int32_t(9));
        go.put("name", "WoodRaft");
        go.put("battle_points", int32_t(800));
        go.put("data_string", "0;gfx/rafts/raft_wood.png -192 0 0,0 1,0;");
        game.extension("lib_go_data", library("gridobject_data", go));
        int id = 0;
        CHECK(game.gridObjectDB.getDataString(id, "WoodRaft") == go.getString("data_string") &&
              id == 9);
        CHECK(game.gridObjectDB.getBattlePointsFromGridObject(9) == 800);
        rp::GridObjectInfo def;
        CHECK(game.gridObjectDB.getNewGridObjectData(def, "WoodRaft"));
        rp::GridObject a(1, 9, 0, 0, 800, 0), b(2, 9, 2, 0, 800, 0);
        CHECK(a.Pieces(&def) == 1 && a.occupiedCells(&def).size() == 2 && a.Beside(b, &def, &def));
        b.x = 1;
        CHECK(a.Overlap(b, &def, &def));
        CHECK(a.Middleish(&def).first == .5f);
        {
            rp::OverlapTree tree;
            rp::OverlapTree::GenerateOverlapTree(tree, &a, &def);
            CHECK(tree.Within({0, 0}, {2, 1}));
            CHECK(!tree.Within({0, 0}, {1, 1}));
            rp::PerimeterTree perimeter;
            rp::PerimeterTree::GeneratePerimeterTree(perimeter, &a, &def);
            CHECK(perimeter.edges().size() == 6);
        }
        rp::OceanView ocean(cache);
        auto multi = rp::GridObjectInfo::parse("0;one.png 0 0 0,0;two.png 30 0 1,0;", 800);
        rp::GridSprite visual(ocean, a, multi, {0, 0}, 1);
        CHECK(visual.Pieces() == 2 && visual.GetMaxOrder() == 1);
        auto accessory = std::make_unique<rp::AccessorySprite>(&ocean, &visual, 12);
        CHECK(accessory->GetTimeString(3661) == "01:01:01");
        CHECK(visual.AddAccessorySprite(std::move(accessory)));
        CHECK(visual.HasAccessorySprite(12) && visual.GetAccessorySprite(12)->ticking());
        CHECK(visual.RemoveAccessorySprite(12) && !visual.HasAccessorySprite(12));
        visual.SetOrder(0, 4, 1);
        CHECK(visual.GetOrder(0) == 4 && visual.GetPriority(0) > visual.GetPriority(1));
        CHECK(visual.Select(&visual[0].appearance));
        visual.SetVisible(false);
        CHECK(!visual.GetVisible());
        visual.SetColour(255, 255, 255, 123);
        CHECK(visual.Alpha() == 123);
        O zone;
        zone.put("zone_id", int32_t(1));
        zone.put("name", "Sea");
        zone.put("mapfile", "test.tmx");
        game.extension("lib_zone_data", library("zone_data", zone));
        game.zonesDB.buildData();
        CHECK(game.zonesDB.getById(1)->mapfile() == "test.tmx");
        O discovery;
        discovery.put("id", int32_t(2));
        discovery.put("name", "Discovery");
        discovery.put("zoneLocation", int32_t(1));
        game.extension("lib_discovery_data", library("discovery_data", discovery));
        game.discoveryDB.buildData();
        CHECK(game.discoveryDB.getById(2)->zoneid() == 1);
        O type;
        type.put("type_id", int32_t(1));
        type.put("name", "Test quest");
        game.extension("lib_quest_type_data", library("quest_type_data", type));
        rp::RPMenuHandler menu(game);
        CHECK(menu.gotMsgQuestTypeInformationRequest(1)->getString("name") == "Test quest");
        CHECK(menu.gotMsgMaterialInformationRequest(2)->name == "Wood");
        CHECK(menu.gotMsgBlueprintDisplayRequest(7)->name() == "Wooden Raft");
        game.rafts = {
            {1, 9, 0, 0, 800, 0, {}}, {2, 9, 2, 5, 800, 0, {}}, {3, 9, -1, -10, 800, 0, {}}};
        CHECK(game.EastMostPoint().x == 2 && game.WestMostPoint().x == -1);
        rp::RaftComponent part;
        part.state = 1;
        part.SetRemainingTime(5.25f);
        CHECK(part.GetRemainingTime() == 5.25f);
        part.SetRemainingTime(5);
        part.UseClock(0);
        CHECK(part.GetRemainingTime() == 5);
        part.TempTick(.025f);
        CHECK(part.GetRemainingTime() == 5);
        part.TempTick(.025f);
        CHECK(part.GetRemainingTime() == 4);
        for (int i = 0; i < 4; ++i)
            part.TempTick(.05f);
        CHECK(part.GetRemainingTime() == 0 && part.Building());
        {
            rp::LoaderImage image("gfx/rafts/raft_core");
            rp::LoaderMenu menu("menus/test.xml");
            rp::LoaderManifest manifest("xml_bin/game_manifest.bin");
            CHECK(std::string(image.getType()) == "SPR" && image.getId() == "gfx/rafts/raft_core");
            CHECK(image.getType() == rp::LoaderImage::ID);
            CHECK(std::string(menu.getType()) == "MENU" && !menu.prepare(nullptr));
            CHECK(std::string(manifest.getType()) == "MANI" && !manifest.prepare(nullptr));
        }
        {
            rp::DamageIndicatorSprite damage(true, 1, 5);
            damage.tick(.5f);
            CHECK(damage.remaining == 1.f && damage.yOffset() == -20.f);
            damage.tick(2);
            CHECK(damage.remaining == 0);
            rp::diving_game::ScrollingLayer layer({"a", "b"}, 128, .5f, 288);
            layer.SetSpeed(100);
            layer.tick(.5f);
            CHECK(layer.tiles().front().y == -25.f);
        }
        {
            rp::LoadingManager loading(cache);
            loading.setPercentage(.375f);
            CHECK(loading.percentage() == 37);
            loading.addXml("missing.xml");
            loading.addXml("missing.xml");
            CHECK(loading.total() == 1);
            loading.start();
            CHECK(loading.running() && loading.percentage() == 0);
            loading.cancel();
            CHECK(!loading.running() && loading.total() == 1);
            loading.update(.016f);
            CHECK(loading.completed() == 0);
            loading.start();
            loading.update(.016f);
            CHECK(!loading.running() && loading.completed() == 1);
            CHECK(loading.percentage() == 100 && loading.errors().size() == 1);
            loading.reset();
            CHECK(loading.total() == 0 && loading.percentage() == 0);
        }
        if (argc == 2) {
            rp::LoadingManager loading(argv[1]);
            loading.addManifest("xml_bin/game_manifest.bin");
            loading.start();
            CHECK(loading.assetExists("xml_bin/game_manifest.bin"));
            CHECK(loading.resourceExists("xml_bin/game_manifest.bin", "MANI"));
            CHECK(!loading.resourceExists("xml_bin/game_manifest.bin", "SPR"));
            CHECK(!loading.resourceExists("xml_bin/game_manifest.bin", ""));
            while (loading.running())
                loading.update(.016f);
            CHECK(loading.completed() == 32 && loading.total() == 32 &&
                  loading.percentage() == 100);
            CHECK(loading.sprite("gfx/rafts/raft_core")->width > 0);
            CHECK(loading.sheet("xml_resources/rp_menu.xml")->findSpriteByName("btn_set_sail"));
            std::cout << "Original manifest: " << loading.completed() - 1 << " resources, "
                      << loading.errors().size() << " diagnostics\n";
            CHECK(loading.errors().empty());
            for (const auto& e : loading.errors())
                std::cout << e << '\n';
            loading.cancel();
            CHECK(!loading.running() && loading.total() == 32);
            loading.reset();
            CHECK(!loading.running() && loading.total() == 0);
            rp::MapView map;
            map.setMapFile(argv[1], "xml_resources/maps/map_big_blue.tmx");
            map.showMap();
            std::vector<rp::Cell> safe, unsafe;
            map.getSafeSquares(safe);
            map.getUnsafeSquares(unsafe);
            CHECK(!safe.empty() && !unsafe.empty());
            int x = safe.front().x * map.data()->tileWidth + map.data()->tileWidth / 2,
                y = safe.front().y * map.data()->tileHeight + map.data()->tileHeight / 2;
            map.setPlayerPosition({float(x), float(y)}, true);
            CHECK(map.playerVisible() && map.hasPlayerNearArea(x, y));
            map.touchDown({float(x), float(y)});
            map.touchUp({float(x), float(y)});
            CHECK(map.moveShip());
            map.movementAllowed(false);
            map.touchDown({float(x), float(y)});
            CHECK(!map.moveShip());
            map.addTreasureMapPoint(1, x, y, 1, 2);
            CHECK(map.hasXNearArea(x, y));
            float fx = x + 1, fy = y + 1;
            map.snapPlayerToTreasureMaps(fx, fy);
            CHECK(fx == x && fy == y);
            map.clearTreasureMapPoints();
            CHECK(!map.hasXNearArea(x, y));
            O boss;
            boss.put("user_id", int32_t(77));
            boss.put("zone", int32_t(1));
            boss.put("display_name", "Boss");
            boss.put("battle_points", int32_t(1234));
            map.updateBoss(boss, 1);
            CHECK(map.boss() && !map.isPosInSafeWater(map.boss()->x, map.boss()->y));
            O enemy;
            enemy.put("user_id", int32_t(78));
            enemy.put("display_name", "Enemy");
            enemy.put("battle_points", int32_t(42));
            map.updateEnemyShips(std::vector<O>{enemy});
            CHECK(map.enemyShips().size() == 1);
            CHECK(map.enemyAt(map.boss()->x, map.boss()->y)->id == 77);
            map.movementAllowed(true);
            CHECK(map.selectEnemy(*map.boss()));
            map.hideMap();
            CHECK(!map.playerVisible());
        }
        std::filesystem::remove(cache);
        std::cout << "Catalogue, geometry, construction and map helpers passed\n";
        return 0;
    } catch (const std::exception& e) {
        std::filesystem::remove(cache);
        std::cerr << e.what() << '\n';
        return 1;
    }
}
