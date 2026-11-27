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

        struct MapRect {
            int x, y, w, h;
        };

        MapRect mapRect(int w, int h) {
            const float scale = std::min(624.f / std::max(1, w), 384.f / std::max(1, h));
            int width = std::max(1, int(w * scale)), height = std::max(1, int(h * scale));
            return {(640 - width) / 2, (400 - height) / 2, width, height};
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

    void PortView::updateHud(const Game& game) {
        if (!hud_)
            return;
        hud_->update(game);
        std::vector<std::string> actions;
        if (placement_)
            actions = {"build", "build_cancel"};
        else if (moving_ || selectedBlueprint_)
            actions = {"build_cancel"};
        else if (selectedComponent_) {
            const GridObject* object = nullptr;
            if (!selectedParent_)
                object = game.GetRaftByID(selectedComponent_);
            else if (auto raft = game.GetRaftByID(selectedParent_)) {
                for (const auto& c : raft->components)
                    if (c.id == selectedComponent_)
                        object = &c;
            }
            if (object && !game.pending && !game.pendingMove) {
                actions = object->getAvailableActions(object->getResourceData(game.objects));
                if (!selectedParent_ && game.mainRaft() == object) {
                    actions.erase(std::remove(actions.begin(), actions.end(), "destroy"),
                                  actions.end());
                    actions.erase(std::remove(actions.begin(), actions.end(), "salvage"),
                                  actions.end());
                }
            }
        }
        hud_->setSelectedActions(actions, game.numOfMaterialPossessed(10));
    }

    void PortView::updateScroll() {
        if (!xmlMenu_ || !swipe_)
            return;
        auto offset = xmlMenu_->scrollOffset(swipe_);
        for (auto [id, origin] : scrollOrigins_)
            xmlMenu_->node(id).set(scrollVertical_ ? "yOffset" : "xOffset", origin + offset);
        xmlMenu_->solve();
    }

    void PortView::tick(float dt) {
        loader_.update(dt);
        touchControls_.tick(dt);
        mapView_.tick(dt);
        if (discovery_)
            discovery_->tick(dt);
        if (diving_)
            diving_->tick(dt);
        if (xmlMenu_) {
            xmlMenu_->tick(dt);
            updateScroll();
        }
    }

    void PortView::touchDown(float x, float y) {
        xmlCaptured_ = false;
        if (xmlMenu_ && (panel_ != Panel::None || map) && xmlMenu_->hitTest(x, y)) {
            xmlMenu_->touchDown(x, y);
            xmlCaptured_ = true;
            oceanGesture_ = false;
            return;
        }
        oceanGesture_ = !onlineBattleActive_ && !diving_ && !discovery_ && !map &&
                        panel_ == Panel::None && y > 75 && y < 290;
        if (diving_)
            diving_->touchDown({x, y});
        else if (oceanGesture_)
            touchControls_.touchDown({x, y});
    }

    void PortView::touchDrag(float x, float y) {
        if (xmlCaptured_ && xmlMenu_) {
            xmlMenu_->touchDrag(x, y);
            updateScroll();
            return;
        }
        if (diving_)
            diving_->touchDrag({x, y});
        else if (oceanGesture_)
            touchControls_.touchDrag({x, y});
    }

    Image PortView::render(const Game& g, float time, const std::string& status) {
        worldTime_ = time;
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
        if (diveStarted_ && g.diveState.state == 3 && !diveSeen_ &&
            (g.diveState.remaining <= 0 ||
             g.diveState.totalSeconds - g.diveState.remaining >=
                 std::min(2.0167f, float(g.diveState.totalSeconds) * .5f))) {
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
            out = ocean_.render(g, time, grid, &g.diveState);
        } else {
            try {
                loadMap(g);
                if (mapView_.data()) {
                    auto& m = *mapView_.data();
                    auto box = mapRect(m.width * m.tileWidth, m.height * m.tileHeight);
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
                                int dx = box.x + x * box.w / m.width,
                                    dy = box.y + y * box.h / m.height,
                                    dx1 = box.x + (x + 1) * box.w / m.width,
                                    dy1 = box.y + (y + 1) * box.h / m.height;
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
                        int mx = box.x + int(ship.x * box.w / (m.width * m.tileWidth)),
                            my = box.y + int(ship.y * box.h / (m.height * m.tileHeight));
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
                        int mx = box.x + int(marker.x * box.w / (m.width * m.tileWidth)),
                            my = box.y + int(marker.y * box.h / (m.height * m.tileHeight));
                        renderItemIcon(out, "rp_menu.xml", "rp_map_marker", mx - 13, my - 13);
                    }
                }
            } catch (const std::exception&) {
                // Keep original map overlay available.
            }
        }
        // Render placement using the original footprint texture atlas, not a substitute UI.
        if (!map && grid && placement_) {
            try {
                auto atlas = loader_.sheet("xml_resources/construction_footprints_junk.xml");
                const auto* cell = atlas->findSpriteByName("footprint_1x1");
                if (cell) {
                    auto image = loader_.sprite(atlas->imagePath);
                    const auto* parent = g.GetRaftByID(placementParent_);
                    std::vector<Cell> footprint{{0, 0}};
                    if (parent) {
                        int object = 0;
                        if (auto bp = g.blueprints.find(selectedBlueprint_);
                            bp != g.blueprints.end())
                            object = bp->second.object;
                        else if (auto source = g.GetRaftByID(selectedParent_))
                            for (const auto& c : source->components)
                                if (c.id == selectedComponent_)
                                    object = c.type;
                        if (auto def = g.objects.find(object); def != g.objects.end())
                            footprint = def->second.cells;
                    }
                    float step = parent ? 1.f / 6 : 1.f;
                    for (auto c : footprint) {
                        auto center = ocean_.WorldToScreen(
                            {(parent ? parent->x : 0) + (placement_->x + c.x + .5f) * step,
                             (parent ? parent->y : 0) + (placement_->y + c.y + .5f) * step});
                        GfxSprite sprite;
                        sprite.image = image;
                        sprite.cell = *cell;
                        float width = ocean_.Unit() * 2 * ocean_.Zoom() * step;
                        sprite.SetScale(width / cell->ow, width * .5f / cell->oh);
                        sprite.SetPos(center.x - width * .5f, center.y - width * .25f);
                        drawSprite(out, sprite);
                    }
                }
            } catch (const std::exception&) { /* No substitute for missing original assets. */
            }
        }
        if (hud_ && g.ready && !map) {
            updateHud(g);
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
        if (xmlCaptured_ && xmlMenu_) {
            xmlCaptured_ = false;
            auto old = xmlMenu_->captured(0);
            auto hit = xmlMenu_->hitTest(x, y);
            xmlMenu_->touchUp(x, y);
            updateScroll();
            if (!old || old != hit || xmlMenu_->node(old).number("suppressClick", 0))
                return {};
            return xmlTouch(g, x, y);
        }
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
            updateHud(g);
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
                return map ? std::optional<sfs::Message>(Game::request("mapaction_request"))
                           : std::nullopt;
            case Hud::Action::Cancel:
                moving_ = false;
                selectedBlueprint_ = selectedComponent_ = selectedParent_ = 0;
                placement_.reset();
                return {};
            case Hud::Action::Build: {
                if (!placement_)
                    return {};
                BuildModeState build(g);
                build.gotMsgGridObjectSelected(selectedParent_, selectedComponent_);
                auto req = moving_ ? build.gotMsgMoveObjectRequest(placementParent_, *placement_)
                                   : build.gotMsgBuildConfirm(selectedBlueprint_, placementParent_,
                                                              *placement_);
                if (req) {
                    moving_ = false;
                    selectedBlueprint_ = selectedComponent_ = selectedParent_ = 0;
                    placement_.reset();
                }
                return req;
            }
            case Hud::Action::Move:
                moving_ = true;
                selectedBlueprint_ = 0;
                placement_.reset();
                grid = true;
                return {};
            case Hud::Action::Destroy:
            case Hud::Action::Salvage:
            case Hud::Action::Repair: {
                confirmAction_ = action == Hud::Action::Repair    ? "repair"
                                 : action == Hud::Action::Salvage ? "salvage"
                                                                  : "destroy";
                panel_ = Panel::Confirm;
                openXml(g, action == Hud::Action::Repair    ? "dialog_repair"
                           : action == Hud::Action::Salvage ? "dialog_salvage"
                                                            : "remove_confirm_dialog");
                return {};
            }
            case Hud::Action::Clock: {
                BuildModeState build(g);
                build.gotMsgGridObjectSelected(selectedParent_, selectedComponent_);
                return build.gotMsgUseClockOnSelectedObject();
            }
            case Hud::Action::None:
                break;
            }
        }
        if (!map && !grid) {
            const auto unit = g.GetDiveUnit();
            if (unit.second && ocean_.Pick(g, unit.first, unit.second, {x, y}, worldTime_)) {
                if (g.diveState.Diving()) {
                    if (g.diveSeconds > 0)
                        diveStarted_ = true;
                    return g.dive(g.diveSeconds > 0 ? "dive_info" : "dive_collect");
                }
                diveStarted_ = true;
                return g.dive("dive_start");
            }
        }
        if (map) {
            loadMap(g);
            if (!mapView_.data())
                return {};
            const auto& data = *mapView_.data();
            auto box = mapRect(data.width * data.tileWidth, data.height * data.tileHeight);
            if (x < box.x || x >= box.x + box.w || y < box.y || y >= box.y + box.h)
                return {};
            float u = (x - box.x) / box.w, v = (y - box.y) / box.h;
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
                    if (g.canBuildRaft(pos)) {
                        placement_ = pos;
                        placementParent_ = 0;
                        return {};
                    }
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
                    const auto* source = g.GetRaftByID(selectedParent_);
                    if (source)
                        for (const auto& part : source->components)
                            if (part.id == selectedComponent_ &&
                                g.canPlaceComponent(part.type, r.id, cx, cy, selectedParent_,
                                                    selectedComponent_)) {
                                placement_ = Cell{cx, cy};
                                placementParent_ = r.id;
                            }
                    return {};
                }
                auto bp = g.blueprints.find(selectedBlueprint_);
                if (bp != g.blueprints.end()) {
                    if (g.canPlaceComponent(bp->second.object, r.id, cx, cy)) {
                        placement_ = Cell{cx, cy};
                        placementParent_ = r.id;
                    }
                    return {};
                }
                for (auto& component : r.components) {
                    auto definition = g.objects.find(component.type);
                    if (definition == g.objects.end())
                        continue;
                    for (auto cell : definition->second.cells)
                        if (component.x + cell.x == cx && component.y + cell.y == cy) {
                            selectedParent_ = r.id;
                            selectedComponent_ = component.id;
                            return {};
                        }
                }
                selectedParent_ = 0;
                selectedComponent_ = r.id;
                return {};
            }
        }
        selectedParent_ = selectedComponent_ = 0;
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
        swipe_ = 0;
        scrollVertical_ = false;
        scrollOrigins_.clear();
        xmlCaptured_ = false;
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
                        ids = RPMenuHandler(g).gotMsgGetAllBlueprintsRequest();
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
                        if (!g.quests.at(id).visible)
                            continue;
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
                for (auto n : {"AutoSafeWater", "PopupDialog", "TutorialArrow"})
                    show(n, false);
                show("MapActionsIcon", g.mapActionMax > 0);
                if (auto quick = xmlMenu_->find(1, "QuickTravel", true)) {
                    if (auto sprite = xmlMenu_->find(quick, "Sprite")) {
                        if (auto zone = g.zones.find(g.zone); zone != g.zones.end()) {
                            try {
                                const auto atlas = ResourceSpriteSheet::Create(
                                    root_ / "xml_resources/rp_menu.xml");
                                if (atlas.findSpriteByName(zone->second))
                                    xmlMenu_->node(sprite).set("spriteName", zone->second);
                            } catch (const std::exception&) {
                            }
                        }
                    }
                }

                for (auto n : {"PingSonar", "QuickTravel"})
                    if (auto id = m.find(1, n, true)) {
                        auto b = m.node(id).bounds;
                        m.node(id).set("xOffset",
                                       std::string(n) == "PingSonar" ? b.w / 2 + 3 : -b.w / 2 - 3);
                        m.node(id).set("yOffset", -b.h / 2 - 3);
                    }
            } else if (name == "quest_log") {
                if (auto dialog = m.find(1, "DialogPopup", true)) {
                    m.node(dialog).set("absoluteX", m.width() / 4 - m.node(dialog).bounds.w);
                    if (auto sprite = m.find(dialog, "DialogSprite"))
                        m.node(sprite).set("hFlip", 1.0);
                    if (auto children = m.find(dialog, "children")) {
                        if ((swipe_ = m.find(dialog, "Swiper"))) {
                            scrollVertical_ = true;
                            m.node(swipe_).set("direction", 1.0);
                            m.node(swipe_).set("mode", 1.0);
                            m.listenToTouches(swipe_, dialog);
                            m.setWindowSize(swipe_, m.node(dialog).bounds.h);
                            m.setScrollSize(swipe_,
                                            std::max(0.f, 48.f + float(xmlItems_.size()) * 55.f -
                                                              m.node(dialog).bounds.h));
                            for (auto& [key, item] : xmlItems_)
                                if (auto el = m.find(children, key, true))
                                    scrollOrigins_[el] =
                                        m.node(el).bounds.y - m.node(children).bounds.y;
                        }
                    }
                }
            } else if (name.rfind("tabbed_menu_", 0) == 0) {
                show("Frame", false);
                // Match MenuTab and MenuTabCurrency's original onInit/onPostSolve placement.
                double x = 40;
                for (int i = 1; i <= 4; ++i)
                    if (auto id = m.find(1, "Tab" + std::to_string(i), true)) {
                        m.node(id).set("xOffset", x);
                        if (auto bg = m.find(id, "bg"))
                            x += m.node(bg).number("width") + m.node(bg).number("leftWidth") +
                                 m.node(bg).number("rightWidth") * .5;
                    }
                x = -36;
                for (int i = 7; i >= 5; --i)
                    if (auto id = m.find(1, "Tab" + std::to_string(i), true)) {
                        m.node(id).set("xOffset", x);
                        x -= m.node(id).bounds.w + 2;
                        if (auto label = m.find(id, "Text"))
                            m.node(label).set(
                                "text", std::to_string(g.numOfMaterialPossessed(i == 5 ? 4 : 11)));
                    }
                // Original game.setClipping("TABS",0,0,displaySizeX(),displaySizeY()*.75).
                for (const auto& n : m.nodes())
                    if (n.text("layer") == "TABS")
                        m.node(n.id).set("clipScreenHeight", m.height() * .75);
                if (auto list = m.find(1, "itemList", true))
                    if ((swipe_ = m.find(list, "Swiper"))) {
                        m.node(list).set("clipChildren", true);
                        m.node(swipe_).set("direction", 0.0);
                        m.node(swipe_).set("mode", 1.0);
                        m.listenToTouches(swipe_, list);
                        m.setScrollSize(
                            swipe_,
                            std::max(0.f, float(xmlItems_.size()) * m.width() * .2f - m.width()));
                        for (auto& [key, id] : xmlItems_)
                            if (auto el = m.find(list, key, true)) {
                                auto index = std::stoi(key.substr(10));
                                scrollOrigins_[el] = std::max(0.f, -m.node(list).bounds.x) + 12 +
                                                     index * m.width() * .2;
                            }
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
                        std::string image = b.image;
                        if (!image.empty() && !b.sheet.empty() && image.size() > 0) {
                            try {
                                auto atlas = ResourceSpriteSheet::Create(
                                    root_ / "xml_resources" /
                                    std::filesystem::path(b.sheet).filename());
                                if (atlas.findSpriteByName(image + "_green"))
                                    image += "_green";
                            } catch (const std::exception&) {
                            }
                        }
                        set(element, "itemImage", "spriteName", image);
                        set(element, "itemImage", "sheetName", b.sheet);
                        set(element, "itemImage", "visible", true);
                        set(element, "itemName", "text", strings_.getText(b.name()));
                        set(element, "itemName", "visible", !g.owned.count(id));
                        set(element, "itemCost", "visible", false);
                        set(element, "currencyIcon", "visible", false);
                        set(element, "selectionOverlay", "visible", false);
                        if (!g.owned.count(id))
                            for (auto c : {"red", "green", "blue"})
                                set(element, "itemImage", c, 0.0);
                        else if (!RPMenuHandler(g).gotMsgHaveMaterialsRequest(id))
                            set(element, "itemImage", "alpha", .4);
                    } else if (name == "quest_log") {
                        const auto& q = g.quests.at(id);
                        set(element, "ProgressText", "text", std::to_string(q.progress) + "%");
                        set(element, "ProgressText", "visible", q.progress > 0 && q.progress < 100);
                        if (auto type = g.questTypeDB.getQuestTypeInfo(unsigned(q.type))) {
                            auto sheet = type->getString(
                                "sheet_id", type->getString("sprite_sheet_id", q.sheet));
                            auto image = type->getString("image_id", q.image);
                            if (!sheet.empty() && !image.empty()) {
                                set(element, "ItemImage", "spriteName", image);
                                set(element, "ItemImage", "sheetName", sheet);
                            }
                        }
                        set(element, "NewQuest", "visible", q.unread);
                    } else if (name == "tabbed_menu_maps") {
                        set(element, "itemName", "text", g.zones.at(id));
                        set(element, "itemName", "visible", true);
                    } else {
                        const auto& item = g.storeItems.at(id);
                        set(element, "itemImage", "spriteName", item.image);
                        set(element, "itemImage", "sheetName", item.sheet);
                        set(element, "itemCost", "text", std::to_string(item.price));
                        set(element, "itemName", "text",
                            strings_.getText(item.title.empty() ? item.name : item.title));
                        set(element, "itemImage", "visible", true);
                        set(element, "selectionOverlay", "visible", false);
                        set(element, "itemCost", "visible", true);
                        set(element, "currencyIcon", "spriteName",
                            std::string(item.currency == 11 ? "shop_diamond_mini"
                                                            : "shop_gold_mini"));
                        set(element, "currencyIcon", "sheetName",
                            std::string("xml_resources/rp_menu.xml"));
                    }
                }
            if (name == "purchase_confirm_dialog_tabbed" && g.storeItems.count(selectedStore_)) {
                const auto& item = g.storeItems.at(selectedStore_);
                auto text = strings_.getText("PURCHASE_CONFIRM");
                auto substitute = [&](const std::string& key, const std::string& value) {
                    auto at = text.find(key);
                    if (at != std::string::npos)
                        text.replace(at, key.size(), value);
                };
                substitute("%item%", strings_.getText(item.title.empty() ? item.name : item.title));
                substitute("%price%", std::to_string(item.price));
                substitute("%currency%", strings_.getText(item.currency == 11 ? "MATERIAL_DIAMOND"
                                                                              : "MATERIAL_GOLD"));
                if (auto label = m.find(1, "ConfirmText", true))
                    set(label, "Text", "text", text);
            }
            if (name == "quest_dialog" && g.quests.count(selectedQuest_)) {
                const auto& q = g.quests.at(selectedQuest_);
                if (auto id = m.find(1, "TitleText", true))
                    set(id, "Text", "text", strings_.getText(q.name));
                if (auto id = m.find(1, "DescriptionText", true))
                    set(id, "Text", "text", strings_.getText(q.description));
                if (auto popup = m.find(1, "PopupDialog", true)) {
                    if (auto bg = m.find(popup, "bg")) {
                        m.node(bg).set("width", m.width() * .7);
                        m.node(bg).set("height", m.height() * .75);
                        if (auto sprite = m.find(bg, "Sprite")) {
                            m.node(sprite).set("width", m.width() * .7);
                            m.node(sprite).set("height", m.height() * .75);
                        }
                    }
                }
            }
            m.solve();
            updateScroll();
        } catch (const std::exception&) {
            xmlMenu_.reset();
        }
    }

    void PortView::renderXml(Image& out, const Game& g, const std::string& name) {
        bool refresh = false;
        if (name == "quest_log" && xmlMenu_ && xmlName_ == name) {
            size_t count = 0;
            for (const auto& q : g.quests)
                if (q.second.visible) {
                    ++count;
                    if (!xmlItems_.count("quest_" + std::to_string(q.first)))
                        refresh = true;
                }
            refresh = refresh || count != xmlItems_.size();
        }
        if (!xmlMenu_ || xmlName_ != name || refresh)
            openXml(g, name);
        if (!xmlMenu_)
            return;
        if (name == "piratemap_menu") {
            if (auto icon = xmlMenu_->find(1, "MapActionsIcon", true)) {
                xmlMenu_->node(icon).set("visible", g.mapActionMax > 0);
                if (auto label = xmlMenu_->find(icon, "TextRegenLimit"))
                    xmlMenu_->node(label).set("text", std::to_string(g.mapActions));
                if (auto label = xmlMenu_->find(icon, "TextMoveTimer")) {
                    xmlMenu_->node(label).set("text", std::to_string(std::max(0, g.mapActionTime)));
                    xmlMenu_->node(label).set("visible", g.mapActions < g.mapActionMax);
                }
            }
            if (auto sonar = xmlMenu_->find(1, "PingSonar", true))
                if (auto label = xmlMenu_->find(sonar, "Text"))
                    xmlMenu_->node(label).set("text", std::to_string(g.numOfMaterialPossessed(12)));
        }
        xmlMenu_->solve();
        GfxManager renderer;
        xmlMenu_->submit(renderer);
        renderer.RenderScreen(out);
    }

    void PortView::renderPanel(Image& image, const Game& g) {
        std::string name;
        switch (panel_) {
        case Panel::Confirm:
            name = xmlName_;
            break;
        case Panel::Quest:
            name = "quest_log";
            break;
        case Panel::QuestDetail:
            name = "quest_dialog";
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
        if (panel_ == Panel::Confirm) {
            if (name == "No" || name == "Back" || name == "Close") {
                panel_ = Panel::None;
                xmlMenu_.reset();
                return {};
            }
            if (name != "Yes")
                return {};
            std::optional<sfs::Message> req;
            if (confirmAction_ == "purchase") {
                if (RPMenuHandler(g).canPurchase(selectedStore_))
                    req = g.buyStoreItem(selectedStore_);
            } else {
                BuildModeState build(g);
                build.gotMsgGridObjectSelected(selectedParent_, selectedComponent_);
                if (confirmAction_ == "repair")
                    req = build.gotMsgRepairSelectedObject();
                else if (confirmAction_ == "salvage")
                    req = build.gotMsgSalvageSelectedObject();
                else if (confirmAction_ == "destroy")
                    req = build.gotMsgDestroyObjectRequest();
            }
            panel_ = Panel::None;
            xmlMenu_.reset();
            selectedComponent_ = selectedParent_ = 0;
            return req;
        }
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
                if (!BuildModeState(g).gotMsgStartBuildRequest(item)) {
                    if (!g.owned.count(item))
                        for (auto& [id, store] : g.storeItems)
                            if (store.name == g.blueprints.at(item).name() &&
                                RPMenuHandler(g).canPurchase(id)) {
                                selectedStore_ = id;
                                confirmAction_ = "purchase";
                                panel_ = Panel::Confirm;
                                openXml(g, "purchase_confirm_dialog_tabbed");
                                return {};
                            }
                    return {};
                }
                selectedBlueprint_ = item;
                selectedComponent_ = selectedParent_ = 0;
                placement_.reset();
                grid = true;
                map = false;
                panel_ = Panel::None;
                xmlMenu_.reset();
                return {};
            }
            if (xmlName_ == "quest_log") {
                selectedQuest_ = item;
                panel_ = Panel::QuestDetail;
                openXml(g, "quest_dialog");
                return g.openQuest(item);
            }
            if (xmlName_ == "tabbed_menu_maps") {
                panel_ = Panel::None;
                xmlMenu_.reset();
                return g.travel(item);
            }
            if (RPMenuHandler(g).canPurchase(item)) {
                selectedStore_ = item;
                confirmAction_ = "purchase";
                panel_ = Panel::Confirm;
                openXml(g, "purchase_confirm_dialog_tabbed");
            }
            return {};
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
