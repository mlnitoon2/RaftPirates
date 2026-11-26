#include "rp/PortView.h"
#include "rp/RPMenuHandler.h"
#include "rp/BuildModeState.h"
#include "sys/gfx/AssetImage.h"
#include <algorithm>
#include <cmath>
#include <cctype>
#include <sstream>

namespace rp {
    using namespace sys::gfx;
    using namespace sys::res;

    namespace {
        // Original implementation of a small debug-only 5x7 pixel alphabet.
        const char* glyph(char ch) {
            switch (std::toupper(static_cast<unsigned char>(ch))) {
            case 'A':
                return "0E11111F111111";
            case 'B':
                return "1E11111E11111E";
            case 'C':
                return "0F10101010100F";
            case 'D':
                return "1E11111111111E";
            case 'E':
                return "1F10101E10101F";
            case 'F':
                return "1F10101E101010";
            case 'G':
                return "0F10101711110F";
            case 'H':
                return "1111111F111111";
            case 'I':
                return "1F04040404041F";
            case 'J':
                return "0702020202120C";
            case 'K':
                return "11121418141211";
            case 'L':
                return "1010101010101F";
            case 'M':
                return "111B1515111111";
            case 'N':
                return "11191915131311";
            case 'O':
                return "0E11111111110E";
            case 'P':
                return "1E11111E101010";
            case 'Q':
                return "0E11111115120D";
            case 'R':
                return "1E11111E141211";
            case 'S':
                return "0F10100E01011E";
            case 'T':
                return "1F040404040404";
            case 'U':
                return "1111111111110E";
            case 'V':
                return "11111111110A04";
            case 'W':
                return "11111115151B11";
            case 'X':
                return "11110A040A1111";
            case 'Y':
                return "11110A04040404";
            case 'Z':
                return "1F01020408101F";
            case '0':
                return "0E11131519110E";
            case '1':
                return "040C040404040E";
            case '2':
                return "0E11010204081F";
            case '3':
                return "1E01010601011E";
            case '4':
                return "02060A121F0202";
            case '5':
                return "1F10101E01011E";
            case '6':
                return "0E10101E11110E";
            case '7':
                return "1F010204080808";
            case '8':
                return "0E11110E11110E";
            case '9':
                return "0E11110F01010E";
            case '-':
                return "0000001F000000";
            case ':':
                return "00040400040400";
            case '.':
                return "00000000000C0C";
            case '/':
                return "01010204081010";
            case ' ':
                return "00000000000000";
            default:
                return "0E110102040004";
            }
        }

        void rect(Image& i, int x, int y, int w, int h, Color c) {
            for (int yy = std::max(0, y); yy < std::min(i.height, y + h); ++yy)
                for (int xx = std::max(0, x); xx < std::min(i.width, x + w); ++xx)
                    i.at(xx, yy) = c;
        }

        int hex(char c) {
            return c >= 'A' ? c - 'A' + 10 : c - '0';
        }

        void text(Image& i, int x, int y, const std::string& s, int scale = 2) {
            for (char c : s) {
                if (x + 6 * scale >= i.width)
                    break;
                auto g = glyph(c);
                for (int row = 0; row < 7; ++row) {
                    int b = hex(g[row * 2]) * 16 + hex(g[row * 2 + 1]);
                    for (int col = 0; col < 5; ++col)
                        if (b & (1 << (4 - col)))
                            rect(i, x + col * scale, y + row * scale, scale, scale,
                                 {238, 245, 250, 255});
                }
                x += 6 * scale;
            }
        }

        void button(Image& i, int n, const std::string& s, bool active = false) {
            rect(i, n * 106 + 2, 350, 103, 46,
                 active ? Color{51, 115, 112, 255} : Color{29, 57, 74, 255});
            text(i, n * 106 + 8, 366, s, 1);
        }
    } // namespace

    PortView::PortView(std::filesystem::path root)
        : root_(std::move(root)), ocean_(root_), touchControls_(&ocean_), loader_(root_) {
        if (loader_.resourceExists("xml_bin/game_manifest.bin", "")) {
            loader_.addManifest("xml_bin/game_manifest.bin");
            loader_.start();
        }
        if (std::filesystem::exists(root_ / "menus/hud.xml")) {
            try {
                hud_ = std::make_unique<Hud>(root_);
            } catch (
                const std::exception&) { /* Keep debug interface if local assets are incomplete. */
            }
        }
    }

    bool PortView::hudEnabled(const Game& game) const {
        return hud_ && game.ready && panel_ == Panel::None && !map;
    }

    void PortView::loadMap(const Game& g) {
        auto i = g.zoneMaps.find(g.zone);
        mapView_.setMapFile(root_, i == g.zoneMaps.end() ? std::string{} : i->second);
    }

    void PortView::tick(float dt) {
        loader_.update(dt);
        touchControls_.tick(dt);
        mapView_.tick(dt);
        if (battle_)
            battle_->tick(dt);
        if (discovery_)
            discovery_->tick(dt);
        if (diving_)
            diving_->tick(dt);
    }

    void PortView::touchDown(float x, float y) {
        oceanGesture_ = !diving_ && !battle_ && !discovery_ && !map && panel_ == Panel::None &&
                        y > 75 && y < 290;
        if (diving_)
            diving_->touchDown({x, y});
        else if (oceanGesture_)
            touchControls_.touchDown({x, y});
    }

    void PortView::touchDrag(float x, float y) {
        if (diving_)
            diving_->touchDrag({x, y});
        else if (oceanGesture_)
            touchControls_.touchDrag({x, y});
    }

    Image PortView::render(const Game& g, float time, const std::string& status) {
        Image out(640, 400, {36, 133, 168, 255});
        if (battle_ && !g.localPlay)
            battle_.reset();
        if (battle_) {
            out = ocean_.render(g, time, false);
            rect(out, 12, 12, 616, 62, {15, 38, 54, 255});
            text(out, 24, 20, "PRACTICE BATTLE", 2);
            text(out, 24, 45,
                 "ENEMY BP " + std::to_string(battle_->enemyBP()) + "  HULL BP " +
                     std::to_string(battle_->playerHullBP()),
                 1);
            rect(out, 320, 44, 280, 12, {70, 38, 38, 255});
            rect(out, 320, 44, 280 * battle_->enemyBP() / battle_->enemyMaxBP(), 12,
                 {210, 90, 65, 255});
            rect(out, 0, 342, 640, 58, {15, 38, 54, 255});
            text(out, 24, 365,
                 battle_->won()             ? "VICTORY - COLLECT GOLD"
                 : battle_->lost()          ? "DEFEAT"
                 : battle_->remaining() > 0 ? "RELOADING"
                                            : "ATTACK",
                 2);
            text(out, 500, 365, "LEAVE", 2);
            return out;
        }
        if (!g.discovery)
            discovery_.reset();
        if (g.discovery && !discovery_)
            discovery_ = std::make_unique<DiscoveryModeState>(g, root_);
        if (discovery_) {
            discovery_->render(out);
            rect(out, 0, 0, 640, 36, {15, 38, 54, 255});
            text(out, 12, 12, discovery_->title(), 2);
            rect(out, 0, 342, 640, 58, {15, 38, 54, 255});
            text(out, 16, 366, "DISCOVERY - TAP TO RETURN", 2);
            return out;
        }
        if (g.diveState.state != 3)
            diveSeen_ = false;
        if (diveStarted_ && g.diveState.state == 3 && !diveSeen_) {
            diveSeen_ = true;
            diveStarted_ = false;
            try {
                diving_ = std::make_unique<diving_game::DivingGame>(
                    root_, unsigned(g.diveState.depth + 1));
                diving_->setDuration(float(std::max(8, g.diveState.totalSeconds)));
            } catch (const std::exception&) {
                diving_.reset();
            }
        }
        if (diving_) {
            diving_->render(out);
            text(out, 12, 15, "GEARS " + std::to_string(diving_->gears()), 2);
            text(out, 450, 15, "TIME " + std::to_string(int(std::ceil(diving_->remaining()))), 2);
            text(out, 15, 365, diving_->finished() ? "TAP TO RETURN" : "TAP / HOLD TO MOVE", 1);
            text(out, 470, 365, "CENTER: EXIT", 1);
            return out;
        }

        if (!map) {
            mapView_.hideMap();
            out = ocean_.render(g, time, grid);
        } else {
            try {
                loadMap(g);
                if (mapView_.data()) {
                    auto& m = *mapView_.data();
                    for (auto& l : m.layers)
                        for (int y = 0; y < l.height; ++y)
                            for (int x = 0; x < l.width; ++x) {
                                auto raw = l.at(x, y);
                                auto gid = raw & 0x1fffffffu;
                                if (!gid)
                                    continue;
                                const TmxTileset* set = nullptr;
                                for (auto& ts : m.tilesets)
                                    if (ts.firstGid <= gid && (!set || ts.firstGid > set->firstGid))
                                        set = &ts;
                                if (!set || set->tileWidth <= 0 || set->tileHeight <= 0)
                                    continue;
                                auto found = tiles_.find(set->image);
                                if (found == tiles_.end())
                                    found = tiles_
                                                .emplace(set->image,
                                                         std::make_shared<Image>(
                                                             loadAssetImage(root_ / set->image)))
                                                .first;
                                auto im = found->second;
                                int cols = im->width / set->tileWidth;
                                if (cols <= 0)
                                    continue;
                                auto local = gid - set->firstGid;
                                int sx = int(local % cols) * set->tileWidth,
                                    sy = int(local / cols) * set->tileHeight;
                                int dx = 48 + x * 544 / m.width, dy = 36 + y * 306 / m.height,
                                    dx1 = 48 + (x + 1) * 544 / m.width,
                                    dy1 = 36 + (y + 1) * 306 / m.height;
                                for (int yy = dy; yy < dy1; ++yy)
                                    for (int xx = dx; xx < dx1; ++xx) {
                                        int u = (xx - dx) * set->tileWidth / (dx1 - dx),
                                            v = (yy - dy) * set->tileHeight / (dy1 - dy);
                                        if (raw & 0x20000000u)
                                            std::swap(u, v);
                                        if (raw & 0x80000000u)
                                            u = set->tileWidth - 1 - u;
                                        if (raw & 0x40000000u)
                                            v = set->tileHeight - 1 - v;
                                        if (sx + u >= im->width || sy + v >= im->height)
                                            continue;
                                        auto p = im->at(sx + u, sy + v);
                                        auto& q = out.at(xx, yy);
                                        float a = p.a / 255.f;
                                        q = {uint8_t(p.r * a + q.r * (1 - a)),
                                             uint8_t(p.g * a + q.g * (1 - a)),
                                             uint8_t(p.b * a + q.b * (1 - a)), 255};
                                    }
                            }
                    mapView_.showMap();
                    mapView_.movementAllowed(!g.pendingMap.has_value());
                    mapView_.setPlayerVisible(true);
                    mapView_.setPlayerPosition(
                        {(std::clamp(g.mapX, -100, 100) + 100) * m.width * m.tileWidth / 200.f,
                         (std::clamp(g.mapY, -100, 100) + 100) * m.height * m.tileHeight / 200.f},
                        true);
                    if (mapView_.playerVisible()) {
                        auto marker = mapView_.playerPosition();
                        int mx = 48 + int(marker.x * 544 / (m.width * m.tileWidth)),
                            my = 36 + int(marker.y * 306 / (m.height * m.tileHeight));
                        rect(out, mx - 4, my - 4, 9, 9, {255, 222, 85, 255});
                    }
                }
            } catch (const std::exception&) {
                text(out, 50, 80, "MAP UNAVAILABLE");
            }
        }
        if (hudEnabled(g)) {
            hud_->update(g);
            hud_->render(out);
            // Research-client controls until ObjectSelectPopup is restored.
            rect(out, 210, 356, 220, 42, {29, 57, 74, 255});
            text(out, 222, 370, g.diveSeconds > 0 ? "DIVING" : "DIVE", 1);
            text(out, 334, 370, "COLLECT", 1);
            if (g.localPlay && !g.safe && g.mainRaft() && !g.mainRaft()->Destroyed()) {
                rect(out, 450, 300, 175, 34, {29, 57, 74, 255});
                text(out, 460, 312, "PRACTICE BATTLE", 1);
            }
            if (grid)
                text(out, 150, 65,
                     moving_              ? "MOVE: TAP DESTINATION"
                     : selectedBlueprint_ ? "TAP TO BUILD"
                                          : "TAP COMPONENT",
                     1);
            return out;
        }
        rect(out, 0, 0, 640, 32, {15, 38, 54, 255});
        text(out, 6, 4, "HYDRALEGACY RP PORT - " + status, 1);
        text(out, 6, 18,
             "ZONE " + std::to_string(g.zone) + " DARING " + std::to_string(g.daring) + " DIVE " +
                 std::to_string(g.diveSeconds) + "S",
             1);
        rect(out, 0, 342, 640, 58, {15, 38, 54, 255});

        button(out, 1, "GRID", grid);
        button(out, 2, map ? "OCEAN" : "MAP", map);
        button(out, 3, "DIVE");
        button(out, 4, "COLLECT");
        button(out, 5, "MENU", panel_ != Panel::None);
        if (panel_ != Panel::None) {
            renderPanel(out, g);
            return out;
        }
        if (grid && !map && !g.blueprints.empty()) {
            auto b = g.blueprints.find(selectedBlueprint_);
            rect(out, 0, 32, 640, 16, {15, 38, 54, 255});
            std::string message = moving_                   ? "MOVE: TAP DESTINATION"
                                  : b != g.blueprints.end() ? "PLACE: " + b->second.name()
                                                            : "TAP COMPONENT TO SELECT";
            text(out, 8, 35, message, 1);
        }
        return out;
    }

    std::optional<sfs::Message> PortView::touch(Game& g, float x, float y) {
        if (oceanGesture_) {
            oceanGesture_ = false;
            touchControls_.touchUp({x, y});
            if (touchControls_.consumeDrag())
                return {};
        }
        if (x < 0 || x >= 640 || y < 0 || y >= 400)
            return {};
        if (battle_) {
            if (y >= 342) {
                if (x >= 470) {
                    battle_->gotMsgExitBattleModeRequest();
                    battle_.reset();
                } else if (battle_->won()) {
                    battle_->endBattleAndPlunder();
                    battle_.reset();
                } else if (battle_->lost()) {
                    battle_.reset();
                } else
                    battle_->gotMsgAttackRequest();
            }
            return {};
        }
        if (discovery_) {
            if (y >= 342) {
                discovery_->LeaveDiscovery(g);
                discovery_.reset();
            }
            return {};
        }
        if (diving_) {
            diving_->touchUp({x, y});
            if (diving_->confirmed()) {
                diving_.reset();
                return g.dive("dive_collect");
            }
            return {};
        }
        if (hudEnabled(g)) {
            if (g.localPlay && x >= 450 && y >= 300 && y < 334 && g.mainRaft() &&
                !g.mainRaft()->Destroyed() && !g.safe) {
                battle_ = std::make_unique<BattleModeState>(g);
                battle_->RaftSelect(g.mainRaft()->id);
                battle_->ToBattleView();
                return {};
            }
            hud_->update(g);
            auto action = hud_->touch(x, y);
            switch (action) {
            case Hud::Action::Quest:
                panel_ = Panel::Quest;
                selectedQuest_ = 0;
                page_ = 0;
                return Game::request("gs_quest");
            case Hud::Action::Safety:
                panel_ = Panel::Safety;
                return {};
            case Hud::Action::Daring: {
                if (g.canAdvanceDaring())
                    return Game::request("daring_advance");
                panel_ = Panel::Daring;
                return {};
            }
            case Hud::Action::ToggleView:
                grid = !grid;
                return {};
            case Hud::Action::Shop:
                panel_ = Panel::Shop;
                page_ = 0;
                return {};
            case Hud::Action::Map:
                map = true;
                return {};
            case Hud::Action::None:
                break;
            }
            if (y >= 356 && x >= 210 && x < 430) {
                if (x < 320)
                    diveStarted_ = true;
                return g.dive(x < 320 ? "dive_start" : "dive_collect");
            }
            if (y >= 350)
                return {};
        }
        if (panel_ != Panel::None && y < 350)
            return panelTouch(g, x, y);
        if (y >= 350) {
            int n = int(x) / 106;
            switch (n) {
            case 0:
                return {};
            case 1:
                panel_ = Panel::None;
                grid = !grid;
                map = false;
                return {};
            case 2:
                panel_ = Panel::None;
                map = !map;
                return {};
            case 3:
                diveStarted_ = true;
                return g.dive("dive_start");
            case 4:
                return g.dive("dive_collect");
            case 5:
                if (!g.ready) return {};
                panel_ = panel_ == Panel::None ? Panel::Build : Panel::None;
                page_ = 0;
                return {};
            default:
                return {};
            }
        }
        if (map) {
            loadMap(g);
            if (!mapView_.data() || x < 48 || x >= 592 || y < 36 || y >= 342)
                return {};
            float u = (x - 48) / 544.f, v = (y - 36) / 306.f;
            float wx = u * mapView_.data()->width * mapView_.data()->tileWidth;
            float wy = v * mapView_.data()->height * mapView_.data()->tileHeight;
            mapView_.showMap();
            mapView_.movementAllowed(!g.pendingMap.has_value());
            mapView_.touchDown({wx, wy});
            mapView_.touchUp({wx, wy});
            if (!mapView_.moveShip()) {
                g.status = g.pendingMap ? "Waiting for movement" : "Cannot sail onto land";
                return {};
            }
            return g.moveMap(int(u * 200 - 100), int(v * 200 - 100),
                             mapView_.isPosInSafeWater(wx, wy));
        }
        if (grid && !g.rafts.empty()) {
            auto chosen = g.blueprints.find(selectedBlueprint_);
            if (chosen != g.blueprints.end()) {
                auto object = g.objects.find(chosen->second.object);
                if (object != g.objects.end() && object->second.type == 0) {
                    auto world = ocean_.ScreenToWorld({x, y});
                    // Tile centre is half a raft down its projected diamond.
                    Cell pos{int(std::floor(world.x)), int(std::floor(world.y))};
                    if (g.canBuildRaft(pos))
                        return BuildModeState(g).gotMsgBuildConfirm(chosen->first, 0, pos);
                    g.status = "Select a free edge next to your raft";
                    return {};
                }
            }
            for (auto it = g.rafts.rbegin(); it != g.rafts.rend(); ++it) {
                auto& r = *it;
                auto world = ocean_.ScreenToWorld({x, y});
                int cx = int(std::floor((world.x - r.x) * 6)),
                    cy = int(std::floor((world.y - r.y) * 6));
                if (cx < 0 || cx >= 6 || cy < 0 || cy >= 6)
                    continue;
                if (moving_) {
                    BuildModeState build(g);
                    build.selectObject(selectedParent_, selectedComponent_);
                    auto request = build.gotMsgMoveObjectRequest(r.id, {cx, cy});
                    moving_ = false;
                    return request;
                }
                auto bp = g.blueprints.find(selectedBlueprint_);
                if (bp != g.blueprints.end())
                    return BuildModeState(g).gotMsgBuildConfirm(bp->first, r.id, {cx, cy});
                for (auto& component : r.components) {
                    auto definition = g.objects.find(component.type);
                    if (definition == g.objects.end())
                        continue;
                    for (auto cell : definition->second.cells)
                        if (component.x + cell.x == cx && component.y + cell.y == cy) {
                            selectedParent_ = r.id;
                            selectedComponent_ = component.id;
                            panel_ = Panel::Build;
                            page_ = 0;
                            return {};
                        }
                }
                return {};
            }
        }
        return {};
    }

    namespace {
        struct PanelRow {
            int id;
            std::string label;
        };

        std::vector<PanelRow> shopRows(const Game& game) {
            std::vector<PanelRow> result;
            RPMenuHandler menu(game);
            for (int id : menu.gotMsgGetAvailStoreItemRequest()) {
                const auto& item = game.storeItems.at(id);
                result.push_back({id, item.name + " - " + std::to_string(item.price)});
            }
            return result;
        }

        std::vector<PanelRow> travelRows(const Game& game) {
            std::vector<PanelRow> result;
            for (auto& [id, name] : game.zones)
                result.push_back({id, "ZONE: " + name});
            for (const auto& [id, name] : game.discoveries)
                if (id > 0 && (game.localPlay || game.knownDiscoveries == std::to_string(id)))
                    result.push_back({-id, "DISCOVERY: " + name});
            return result;
        }

        std::vector<PanelRow> blueprintRows(const Game& game) {
            std::vector<PanelRow> result;
            RPMenuHandler menu(game);
            for (int id : menu.gotMsgGetAvailBlueprintsRequest()) {
                const auto* bp = menu.gotMsgBlueprintDisplayRequest(id);
                if (!bp)
                    continue;
                std::string label = bp->name();
                if (!menu.gotMsgHaveMaterialsRequest(id)) {
                    auto cost = game.goldCostToBuild(id);
                    label += cost ? " - NEED MATERIALS (" + std::to_string(*cost) + " GOLD)"
                                  : " - NEED MATERIALS";
                }
                result.push_back({id, std::move(label)});
            }
            return result;
        }

        std::vector<PanelRow> inventoryRows(const Game& game) {
            std::vector<PanelRow> result;
            for (auto& [id, amount] : game.materials) {
                auto it = game.materialNames.find(id);
                result.push_back(
                    {id, (it == game.materialNames.end() ? std::to_string(id) : it->second) +
                             " : " + std::to_string(amount)});
            }
            return result;
        }
    } // namespace

    bool PortView::renderItemIcon(Image& target, const std::string& sheet, const std::string& name,
                                  int x, int y) {
        if (sheet.empty() || name.empty())
            return false;
        // Restrict network-supplied names to the bundled atlas directory.
        auto path = std::filesystem::path(sheet);
        if (path.filename() != path || path.extension() != ".xml")
            return false;
        try {
            auto it = shopAtlases_.find(sheet);
            if (it == shopAtlases_.end()) {
                auto atlas =
                    loader_.sheet((std::filesystem::path("xml_resources") / path).generic_string());
                auto img = loader_.sprite(atlas->imagePath);
                it = shopAtlases_.emplace(sheet, std::make_pair(atlas, img)).first;
            }
            const auto* cell = it->second.first->findSpriteByName(name);
            if (!cell || cell->w <= 0 || cell->h <= 0)
                return false;
            GfxSprite sprite;
            sprite.cell = *cell;
            sprite.image = it->second.second;
            float scale = std::min(26.f / cell->w, 26.f / cell->h);
            sprite.SetScale(scale, scale);
            sprite.SetPos(float(x), float(y));
            drawSprite(target, sprite);
            return true;
        } catch (const std::exception&) {
            return false;
        }
    }

    void PortView::renderPanel(Image& image, const Game& game) {
        rect(image, 14, 36, 612, 302, {15, 38, 54, 255});
        if (panel_ == Panel::Quest || panel_ == Panel::Safety || panel_ == Panel::Daring) {
            std::string title = panel_ == Panel::Quest    ? "QUESTS"
                                : panel_ == Panel::Safety ? "OCEAN SAFETY"
                                                          : "DARING";
            text(image, 25, 52, title, 2);
            if (panel_ == Panel::Quest) {
                auto ids = RPMenuHandler(game).gotMsgQuestInformationRequest();
                auto selected = game.quests.find(selectedQuest_);
                if (selected != game.quests.end()) {
                    text(image, 25, 90, selected->second.name.substr(0, 85), 1);
                    text(image, 25, 122,
                         "PROGRESS: " + std::to_string(selected->second.progress) + "%", 1);
                    const auto& description = selected->second.description;
                    for (size_t i = 0; i < description.size() && i < 450; i += 85)
                        text(image, 25, 150 + int(i / 85) * 18, description.substr(i, 85), 1);
                    text(image, 25, 265, "TAP TO RETURN TO QUESTS", 1);
                } else {
                    for (size_t i = 0; i < 6 && i + page_ * 6 < ids.size(); ++i) {
                        const auto& q = game.quests.at(ids[i + page_ * 6]);
                        text(image, 25, 90 + int(i) * 29,
                             (q.unread ? "NEW: " : "") + q.name.substr(0, 65) + " " +
                                 std::to_string(q.progress) + "%",
                             1);
                    }
                    if (ids.empty())
                        text(image, 25, 90, "NO QUESTS", 1);
                    text(image, 25, 275, "PREV", 1);
                    text(image, 560, 275, "NEXT", 1);
                }
            }
            if (panel_ == Panel::Safety)
                text(image, 25, 90,
                     game.badZone ? "LOW WATER"
                     : game.safe  ? "SAFE WATER"
                                  : "PIRATE WATER",
                     2);
            if (panel_ == Panel::Daring) {
                text(image, 25, 90, "LEVEL " + std::to_string(game.daring), 2);
                text(image, 25, 125, "REMAINING " + std::to_string(game.daringRemaining) + "S", 2);
            }
            text(image, 25, 306, "TAP TO CLOSE", 1);
            return;
        }
        const char* names[] = {"BUILD", "SHOP", "INVENTORY", "TRAVEL"};
        const Panel panels[] = {Panel::Build, Panel::Shop, Panel::Inventory, Panel::Travel};
        for (int i = 0; i < 4; ++i) {
            rect(image, 20 + 150 * i, 42, 145, 28,
                 panel_ == panels[i] ? Color{51, 115, 112, 255} : Color{29, 57, 74, 255});
            text(image, 30 + 150 * i, 52, names[i], 1);
        }
        auto rows = panel_ == Panel::Shop        ? shopRows(game)
                    : panel_ == Panel::Travel    ? travelRows(game)
                    : panel_ == Panel::Inventory ? inventoryRows(game)
                                                 : blueprintRows(game);
        for (size_t i = 0; i < 6 && page_ * 6 + i < rows.size(); ++i) {
            rect(image, 20, 76 + int(i) * 34, 600, 30, {29, 57, 74, 255});
            const auto& row = rows[page_ * 6 + i];
            bool icon = false;
            if (panel_ == Panel::Shop) {
                const auto& item = game.storeItems.at(row.id);
                icon = renderItemIcon(image, item.sheet, item.image, 26, 78 + int(i) * 34);
            } else if (panel_ == Panel::Build) {
                const auto& item = game.blueprints.at(row.id);
                icon = renderItemIcon(image, item.sheet, item.image, 26, 78 + int(i) * 34);
            }
            text(image, icon ? 60 : 28, 87 + int(i) * 34, row.label, 1);
        }
        if (rows.empty())
            text(image, 28, 90, "NO ITEMS", 1);
        text(image, 25, 290, "PREV", 1);
        text(image, 560, 290, "NEXT", 1);
        text(image, 270, 290, "PAGE " + std::to_string(page_ + 1), 1);
        if (panel_ == Panel::Build) {
            text(image, 25, 316, "SELECT PART", 1);
            if (selectedComponent_) {
                text(image, 220, 316, "MOVE", 1);
                text(image, 390, 316, "REMOVE", 1);
                text(image, 470, 316, "REPAIR", 1);
                text(image, 550, 316, "CLOCK", 1);
            }
        }
    }

    std::optional<sfs::Message> PortView::panelTouch(Game& game, float x, float y) {
        if (panel_ == Panel::Quest || panel_ == Panel::Safety || panel_ == Panel::Daring) {
            if (panel_ == Panel::Quest) {
                if (y >= 290) {
                    panel_ = Panel::None;
                    selectedQuest_ = 0;
                    return {};
                }
                if (selectedQuest_) {
                    selectedQuest_ = 0;
                    return {};
                }
                auto ids = RPMenuHandler(game).gotMsgQuestInformationRequest();
                if (y >= 260) {
                    if (x < 150 && page_)
                        --page_;
                    else if (x > 490 && (page_ + 1) * 6 < ids.size())
                        ++page_;
                    return {};
                }
                if (y >= 80 && y < 254) {
                    size_t row = page_ * 6 + size_t(y - 80) / 29;
                    if (row < ids.size()) {
                        selectedQuest_ = ids[row];
                        return game.openQuest(selectedQuest_);
                    }
                }
            } else if (y >= 280)
                panel_ = Panel::None;
            return {};
        }
        if (y >= 42 && y < 70 && x >= 20 && x < 620) {
            const Panel panels[] = {Panel::Build, Panel::Shop, Panel::Inventory, Panel::Travel};
            panel_ = panels[std::min(3, int(x - 20) / 150)];
            page_ = 0;
            return {};
        }
        auto rows = panel_ == Panel::Shop        ? shopRows(game)
                    : panel_ == Panel::Travel    ? travelRows(game)
                    : panel_ == Panel::Inventory ? inventoryRows(game)
                                                 : blueprintRows(game);
        if (y >= 280 && y < 305) {
            if (x < 150 && page_ > 0)
                --page_;
            if (x > 490 && (page_ + 1) * 6 < rows.size())
                ++page_;
            return {};
        }
        if (panel_ == Panel::Build && y >= 305 && y < 338) {
            panel_ = Panel::None;
            grid = true;
            map = false;
            selectedBlueprint_ = 0;
            if (x >= 190 && x < 360 && selectedComponent_)
                moving_ = true;
            else if (x >= 460 && selectedComponent_) {
                BuildModeState build(game);
                build.selectObject(selectedParent_, selectedComponent_);
                return x >= 540 ? build.UseClockSelected() : build.repairSelected(false);
            } else if (x >= 360 && selectedComponent_) {
                BuildModeState build(game);
                build.selectObject(selectedParent_, selectedComponent_);
                auto request = build.gotMsgDestroyObjectRequest();
                selectedComponent_ = 0;
                selectedParent_ = 0;
                return request;
            } else {
                moving_ = false;
                selectedComponent_ = 0;
                selectedParent_ = 0;
            }
            return {};
        }
        if (x < 20 || x >= 620 || y < 76 || y >= 280)
            return {};
        size_t row = page_ * 6 + size_t(y - 76) / 34;
        if (row >= rows.size())
            return {};
        int id = rows[row].id;
        if (panel_ == Panel::Shop) {
            if (!RPMenuHandler(game).canPurchase(id)) {
                game.status = "Item unavailable or insufficient currency";
                return {};
            }
            return game.buyStoreItem(id);
        }
        if (panel_ == Panel::Travel) {
            panel_ = Panel::None;
            return id < 0 ? game.discover(-id) : game.travel(id);
        }
        if (panel_ == Panel::Build) {
            selectedBlueprint_ = id;
            moving_ = false;
            grid = true;
            map = false;
            panel_ = Panel::None;
        }
        return {};
    }
} // namespace rp
