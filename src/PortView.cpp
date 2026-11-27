#include "rp/PortView.h"
#include "rp/RPMenuHandler.h"
#include "rp/BuildModeState.h"
#include "sys/gfx/AssetImage.h"
#include <algorithm>
#include <cmath>
#include <cctype>
#include <sstream>
#include <regex>

namespace rp {
    using namespace sys::gfx;
    using namespace sys::res;

    namespace {
        void rect(Image& i, int x, int y, int w, int h, Color c) {
            for (int yy = std::max(0, y); yy < std::min(i.height, y + h); ++yy)
                for (int xx = std::max(0, x); xx < std::min(i.width, x + w); ++xx)
                    i.at(xx, yy) = c;
        }

    } // namespace

    PortView::PortView(std::filesystem::path root)
        : root_(std::move(root)), ocean_(root_), opponentOcean_(root_), touchControls_(&ocean_),
          loader_(root_) {
        if (std::filesystem::exists(root_ / "text/en.utf8"))
            strings_.loadFile(root_ / "text/en.utf8");
        if (loader_.resourceExists("xml_bin/game_manifest.bin", "")) {
            loader_.addManifest("xml_bin/game_manifest.bin");
            loader_.start();
        }
        if (std::filesystem::exists(root_ / "menus/hud.xml")) {
            try {
                hud_ = std::make_unique<Hud>(root_);
            } catch (const std::exception&) { /* No substitute controls for missing assets. */
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
        if (discovery_)
            discovery_->tick(dt);
        if (diving_)
            diving_->tick(dt);
        if (xmlMenu_)
            xmlMenu_->tick(dt);
    }

    void PortView::touchDown(float x, float y) {
        oceanGesture_ = !onlineBattleActive_ && !diving_ && !discovery_ && !map &&
                        panel_ == Panel::None && y > 75 && y < 290;
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
        using Phase = BattleModeState::Phase;
        const auto& net = g.onlineBattle;
        onlineBattleActive_ = net.phase() != Phase::Idle;
        if (!onlineBattleActive_)
            battleScreen_.reset();
        if (onlineBattleActive_) {
            if (!battleScreen_) {
                try {
                    battleScreen_ = std::make_unique<GfxBattleScreen>(root_);
                } catch (const std::exception&) {
                }
            }
            if (battleScreen_)
                return battleScreen_->Render(g, time);
            return ocean_.render(g, time, false);
        }
        if (!g.discovery)
            discovery_.reset();
        if (g.discovery && !discovery_)
            discovery_ = std::make_unique<DiscoveryModeState>(g, root_);
        if (discovery_) {
            discovery_->render(out);
            renderXml(out, g, "piratemap_menu");
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
            return out;
        }

        if (g.hideMap)
            map = false;
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
                    mapView_.updateBoss(g.bossData, g.zone);
                    mapView_.updateEnemyShips(g.sonarShips);
                    auto drawMarker = [&](const MapView::EnemyShipPoints& ship, bool boss) {
                        int mx = 48 + int(ship.x * 544.f / (m.width * m.tileWidth)),
                            my = 36 + int(ship.y * 306.f / (m.height * m.tileHeight));
                        if (!renderItemIcon(out, "rp_menu.xml",
                                            boss ? "rp_map_boss" : "rp_battle_arrow", mx - 13,
                                            my - 13))
                            rect(out, mx - 4, my - 4, 9, 9, {240, 110, 85, 255});
                    };
                    if (mapView_.boss())
                        drawMarker(*mapView_.boss(), true);
                    for (const auto& enemy : mapView_.enemyShips())
                        drawMarker(enemy, false);
                    if (mapView_.playerVisible()) {
                        auto marker = mapView_.playerPosition();
                        int mx = 48 + int(marker.x * 544 / (m.width * m.tileWidth)),
                            my = 36 + int(marker.y * 306 / (m.height * m.tileHeight));
                        rect(out, mx - 4, my - 4, 9, 9, {255, 222, 85, 255});
                    }
                }
            } catch (const std::exception&) {
                // Keep original map overlay available.
            }
        }
        if (hud_ && g.ready && !map) {
            hud_->update(g);
            hud_->render(out);
        }
        if (panel_ != Panel::None)
            renderPanel(out, g);
        else if (map)
            renderXml(out, g, "piratemap_menu");
        else {
            xmlMenu_.reset();
            xmlName_.clear();
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
        using Phase = BattleModeState::Phase;
        auto& net = g.onlineBattle;
        if (net.phase() != Phase::Idle) {
            if (battleScreen_)
                return battleScreen_->Select(g, x, y);
            return {};
        }
        if (discovery_) {
            if (xmlMenu_ && xmlMenu_->hitTest(x, y)) {
                auto id = xmlMenu_->owner(xmlMenu_->hitTest(x, y));
                if (xmlMenu_->node(id).name == "Back") {
                    discovery_->LeaveDiscovery(g);
                    discovery_.reset();
                }
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
        if (panel_ != Panel::None)
            return panelTouch(g, x, y);
        if (map) {
            if (!xmlMenu_ || xmlName_ != "piratemap_menu")
                openXml(g, "piratemap_menu");
            if (xmlMenu_ && xmlMenu_->hitTest(x, y))
                return xmlTouch(g, x, y);
        }
        if (hudEnabled(g)) {
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
                map = !g.hideMap;
                return {};
            case Hud::Action::None:
                break;
            }
        }
        if (!map && !grid) {
            const auto unit = g.GetDiveUnit();
            auto world = ocean_.ScreenToWorld({x, y});
            for (const auto& raft : g.rafts)
                if (raft.id == unit.first)
                    for (const auto& component : raft.components)
                        if (component.id == unit.second) {
                            auto def = g.objects.find(component.type);
                            if (def == g.objects.end())
                                continue;
                            int cx = int(std::floor((world.x - raft.x) * 6)),
                                cy = int(std::floor((world.y - raft.y) * 6));
                            for (auto cell : def->second.cells)
                                if (component.x + cell.x == cx && component.y + cell.y == cy) {
                                    if (g.diveState.state == 3)
                                        return g.dive(g.diveSeconds > 0 ? "dive_info"
                                                                        : "dive_collect");
                                    diveStarted_ = true;
                                    return g.dive("dive_start");
                                }
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
            int enemyId = 0;
            bool usedSonar = false;
            if (const auto* target = mapView_.enemyAt(wx, wy)) {
                if (mapView_.selectEnemy(*target)) {
                    enemyId = target->id;
                    usedSonar = !mapView_.boss() || mapView_.boss()->id != enemyId;
                    wx = float(target->x);
                    wy = float(target->y);
                    u = wx / (mapView_.data()->width * mapView_.data()->tileWidth);
                    v = wy / (mapView_.data()->height * mapView_.data()->tileHeight);
                }
            } else
                mapView_.touchDown({wx, wy});
            if (!enemyId)
                mapView_.touchUp({wx, wy});
            if (!mapView_.moveShip()) {
                g.status = g.pendingMap ? "Waiting for movement" : "Cannot sail onto land";
                return {};
            }
            return g.moveMap(int(u * 200 - 100), int(v * 200 - 100),
                             mapView_.isPosInSafeWater(wx, wy), enemyId, usedSonar);
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

    void PortView::openXml(const Game& g, const std::string& name) {
        using namespace sys::menu_redux;
        xmlMenu_ = std::make_unique<MenuRuntime>(root_);
        xmlName_ = name;
        xmlItems_.clear();
        try {
            xmlMenu_->nativeEvent = [this](auto& m, unsigned id, const std::string& event) {
                if (event == "onInit")
                    m.applyLiteralProperties(id,
                                             [this](const auto& s) { return strings_.getText(s); });
                return true;
            };
            auto xml = sys::res::loadXML(root_ / "menus" / (name + ".xml"));
            auto find = [](auto& self, sys::res::XmlNode& n,
                           const std::string& name) -> sys::res::XmlNode* {
                if (n.attr("name") == name)
                    return &n;
                for (auto& c : n.children)
                    if (auto* p = self(self, c, name))
                        return p;
                return nullptr;
            };
            auto add = [&](sys::res::XmlNode& parent, const std::string& type,
                           const std::string& id, double x, double y, const char* relative,
                           const char* v, const char* h) {
                sys::res::XmlNode t;
                t.name = "template";
                t.attributes = {{"type", type}, {"name", id}};
                sys::res::XmlNode pos;
                pos.name = "position";
                pos.attributes = {{"relativeTo", relative}, {"vAnchor", v}, {"hAnchor", h}};
                sys::res::XmlNode ori;
                ori.name = "orientation";
                ori.attributes = {{"vAnchor", v},
                                  {"hAnchor", h},
                                  {"xOffset", std::to_string(x)},
                                  {"yOffset", std::to_string(y)},
                                  {"priority", "-10"}};
                t.children = {pos, ori};
                parent.children.push_back(std::move(t));
            };
            if (name == "tabbed_menu_inventory") {
                auto ids = RPMenuHandler(g).gotMsgGetAvailMaterialsRequest();
                size_t i = 0;
                for (int id : ids) {
                    auto key = "material_" + std::to_string(id);
                    xmlItems_[key] = id;
                    add(xml, "MaterialInstance", key,
                        (double(i) - (double(ids.size()) - 1) / 2) * 45, -50, "SCREEN", "BOTTOM",
                        "HCENTER");
                    ++i;
                }
            } else if (name.rfind("tabbed_menu_", 0) == 0) {
                if (auto* list = find(find, xml, "itemList")) {
                    std::vector<int> ids;
                    if (name == "tabbed_menu_blueprints")
                        ids = RPMenuHandler(g).gotMsgGetAvailBlueprintsRequest();
                    else if (name == "tabbed_menu_maps")
                        for (auto& [id, v] : g.zones)
                            ids.push_back(id);
                    else {
                        for (int id : RPMenuHandler(g).gotMsgGetAvailStoreItemRequest())
                            if (g.storeItems.at(id).group == name.substr(12))
                                ids.push_back(id);
                    }
                    for (size_t i = 0; i < ids.size(); ++i) {
                        auto key = "tabbedItem" + std::to_string(i);
                        xmlItems_[key] = ids[i];
                        add(*list, "TabbedMenuItem", key, 12 + int(i) * 128, 0, "itemList",
                            "BOTTOM", "LEFT");
                    }
                }
            } else if (name == "quest_log") {
                if (auto* list = find(find, xml, "children")) {
                    size_t i = 0;
                    for (int id : RPMenuHandler(g).gotMsgQuestInformationRequest()) {
                        auto key = "quest_" + std::to_string(id);
                        xmlItems_[key] = id;
                        add(*list, "QuestButton", key, -50, 48 + int(i++) * 55, "children", "TOP",
                            "RIGHT");
                    }
                }
            }
            xmlMenu_->load(xml);
            xmlMenu_->resize(640, 400);
            xmlMenu_->initialize();
            auto& m = *xmlMenu_;
            auto show = [&](const char* name, bool visible) {
                if (auto id = m.find(1, name, true))
                    m.node(id).set("visible", visible);
            };
            if (name == "piratemap_menu") {
                for (auto n : {"AutoSafeWater", "MapActionsIcon", "PopupDialog", "TutorialArrow"})
                    show(n, false);
                for (auto n : {"PingSonar", "QuickTravel"})
                    if (auto id = m.find(1, n, true)) {
                        auto b = m.node(id).bounds;
                        m.node(id).set("xOffset",
                                       std::string(n) == "PingSonar" ? b.w / 2 + 3 : -b.w / 2 - 3);
                        m.node(id).set("yOffset", -b.h / 2 - 3);
                    }
            } else if (name.rfind("tabbed_menu_", 0) == 0) {
                show("Frame", false);
                double x = 8;
                for (int i = 1; i <= 7; ++i)
                    if (auto id = m.find(1, "Tab" + std::to_string(i), true)) {
                        m.node(id).set("xOffset", x);
                        x += m.node(id).bounds.w + 16;
                    }
            }
            auto set = [&](unsigned id, const char* component, const char* key, Property value) {
                if (auto c = m.find(id, component))
                    m.node(c).set(key, std::move(value));
            };
            for (auto& [key, id] : xmlItems_)
                if (auto element = m.find(1, key, true)) {
                    if (name == "tabbed_menu_inventory") {
                        if (auto mat = g.materialDB.getMaterialInfo(unsigned(id))) {
                            set(element, "Icon", "spriteName", mat->image);
                            set(element, "Icon", "sheetName", mat->sheet);
                            set(element, "Amount", "text",
                                std::to_string(g.numOfMaterialPossessed(unsigned(id))));
                        }
                    } else if (name == "tabbed_menu_blueprints") {
                        const auto& b = g.blueprints.at(id);
                        set(element, "itemImage", "spriteName", b.image);
                        set(element, "itemImage", "sheetName", b.sheet);
                        set(element, "itemImage", "visible", true);
                        set(element, "itemName", "text", b.name());
                        set(element, "itemCost", "visible", false);
                        set(element, "currencyIcon", "visible", false);
                    } else if (name == "quest_log") {
                        const auto& q = g.quests.at(id);
                        set(element, "ProgressText", "text", std::to_string(q.progress) + "%");
                    } else if (name == "tabbed_menu_maps") {
                        set(element, "itemName", "text", g.zones.at(id));
                        set(element, "itemName", "visible", true);
                    } else {
                        const auto& item = g.storeItems.at(id);
                        set(element, "itemImage", "spriteName", item.image);
                        set(element, "itemImage", "sheetName", item.sheet);
                        set(element, "itemCost", "text", std::to_string(item.price));
                    }
                }
            m.solve();
        } catch (const std::exception&) {
            xmlMenu_.reset();
        }
    }

    void PortView::renderXml(Image& out, const Game& g, const std::string& name) {
        if (!xmlMenu_ || xmlName_ != name)
            openXml(g, name);
        if (!xmlMenu_)
            return;
        if (name == "piratemap_menu")
            if (auto sonar = xmlMenu_->find(1, "PingSonar", true))
                if (auto label = xmlMenu_->find(sonar, "Text"))
                    xmlMenu_->node(label).set("text", std::to_string(g.numOfMaterialPossessed(12)));
        xmlMenu_->solve();
        GfxManager renderer;
        xmlMenu_->submit(renderer);
        renderer.RenderScreen(out);
    }

    void PortView::renderPanel(Image& image, const Game& g) {
        std::string name;
        switch (panel_) {
        case Panel::Quest:
            name = "quest_log";
            break;
        case Panel::Safety:
            name = g.badZone ? "hud_badzone_popup" : "safe_flag_popup";
            break;
        case Panel::Daring:
            name = "daring_popup";
            break;
        case Panel::Inventory:
            name = "tabbed_menu_inventory";
            break;
        case Panel::Travel:
            name = "tabbed_menu_maps";
            break;
        case Panel::Shop:
        case Panel::Build:
            name = "tabbed_menu_blueprints";
            break;
        default:
            return;
        }
        if (xmlMenu_ && xmlName_.rfind("tabbed_menu_", 0) == 0 &&
            (panel_ == Panel::Shop || panel_ == Panel::Build || panel_ == Panel::Travel))
            name = xmlName_;
        if (xmlName_ == "sonar_popup" && panel_ == Panel::Safety)
            name = xmlName_;
        renderXml(image, g, name);
    }

    std::optional<sfs::Message> PortView::xmlTouch(Game& g, float x, float y) {
        if (!xmlMenu_)
            return {};
        auto& m = *xmlMenu_;
        auto hit = m.hitTest(x, y);
        if (!hit)
            return {};
        auto owner = m.owner(hit);
        auto name = m.node(owner).name;
        auto body = m.node(hit).events.find("onTouchUp");
        std::string script = body == m.node(hit).events.end() ? "" : body->second.source;
        if (xmlName_ == "piratemap_menu") {
            if (name == "Back") {
                map = false;
                xmlMenu_.reset();
                return {};
            }
            if (name == "QuickTravel") {
                panel_ = Panel::Travel;
                openXml(g, "tabbed_menu_maps");
                return {};
            }
            if (name == "PingSonar") {
                if (g.badZone) {
                    panel_ = Panel::Safety;
                    openXml(g, "sonar_popup");
                    return {};
                }
                if (g.numOfMaterialPossessed(12) > 0)
                    return g.pingSonar();
                panel_ = Panel::Shop;
                openXml(g, "tabbed_menu_misc");
                return {};
            }
        }
        static const std::regex push(R"rx(menu:pushPopUp\("(tabbed_menu_[a-z]+)"\))rx");
        std::smatch match;
        if (std::regex_search(script, match, push)) {
            openXml(g, match[1]);
            return {};
        }
        if (name.rfind("backing", 0) == 0 || name == "bg" || name == "OK" || name == "Back" ||
            name == "Close") {
            panel_ = Panel::None;
            xmlMenu_.reset();
            return {};
        }
        for (unsigned id = owner; id; id = m.node(id).parent) {
            auto i = xmlItems_.find(m.node(id).name);
            if (i == xmlItems_.end())
                continue;
            int item = i->second;
            if (xmlName_ == "tabbed_menu_blueprints") {
                selectedBlueprint_ = item;
                grid = true;
                map = false;
                panel_ = Panel::None;
                xmlMenu_.reset();
                return {};
            }
            if (xmlName_ == "quest_log")
                return g.openQuest(item);
            if (xmlName_ == "tabbed_menu_maps") {
                panel_ = Panel::None;
                xmlMenu_.reset();
                return g.travel(item);
            }
            return {}; // The original purchase confirmation is not yet bound.
        }
        return {};
    }

    std::optional<sfs::Message> PortView::panelTouch(Game& g, float x, float y) {
        if (!xmlMenu_ || xmlName_ == "piratemap_menu") {
            Image dummy(640, 400);
            renderPanel(dummy, g);
        }
        return xmlTouch(g, x, y);
    }
} // namespace rp
