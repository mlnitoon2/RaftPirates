#include "rp/PortView.h"
#include "rp/ItemPlacementSprite.h"
#include "rp/ItemOnRaftPlacementSprite.h"
#include "rp/RaftPirates.h"
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
            // The map is a full-screen surface; use the same conversion for picking.
            return {0, 0, 640, 400};
        }
    } // namespace

    PortView::PortView(std::filesystem::path root)
        : root_(std::move(root)), ocean_(root_), opponentOcean_(root_), touchControls_(&ocean_),
          loader_(root_) {
        preloadedMenus_ = RaftPirates::preloadMenus(root_);
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
        if(lastScrollOffset_ && *lastScrollOffset_==offset)return;
        lastScrollOffset_=offset;
        for (auto [id, origin] : scrollOrigins_)
            xmlMenu_->node(id).set(scrollVertical_ ? "yOffset" : "xOffset", origin + offset);
        xmlMenu_->solve();
    }

    std::string PortView::activeMenu() const {
        return xmlMenu_ ? xmlName_ : std::string("hud");
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

    std::optional<sfs::Message> PortView::takePendingRequest(Game& g){
        if(!diving_||!diveClosing_||diving_->moving())return {};
        diving_.reset();diveClosing_=false;
        if(!g.diveState.Diving()||divingUnit_!=std::make_pair(g.diveState.parent,g.diveState.object))return {};
        if(!g.diveState.collected.empty())return BuildModeState(g).SendBonusMaterials();
        return g.collectDive();
    }

    void PortView::cancelInput(){
        if(xmlCaptured_&&xmlMenu_)xmlMenu_->touchCancel();
        if(oceanGesture_)touchControls_.touchCancel({});
        if(diving_)diving_->touchCancel({});
        xmlCaptured_=oceanGesture_=false;
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
        if (g.diveState.state != 3){
            diveSeen_ = false;
            if(!g.diveState.Diving()){diving_.reset();diveClosing_=false;}
        }
        if(diving_&&divingUnit_!=std::make_pair(g.diveState.parent,g.diveState.object)){
            diving_.reset();diveClosing_=false;diveSeen_=false;
        }
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
                divingUnit_={g.diveState.parent,g.diveState.object};
                diveClosing_=false;
                diving_->setPos(850,22);diving_->moveTo(210,22,1.f);
            } catch (const std::exception&) {
                diving_.reset();
            }
        }
        if (diving_) {
            out=ocean_.render(g,time,grid,&g.diveState);
            diving_->render(out);
            return out;
        }

        if (g.hideMap)
            map = false;
        if (!map) {
            mapView_.gotMsgHidePirateMap();
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
                    mapView_.gotMsgShowPirateMap();
                    mapView_.updatePlayerInfo(g);
                    auto drawMarker = [&](const MapView::EnemyShipPoints& ship, bool boss) {
                        int mx = box.x + int(ship.x * box.w / (m.width * m.tileWidth)),
                            my = box.y + int(ship.y * box.h / (m.height * m.tileHeight));
                        if (!renderItemIcon(out, "rp_menu.xml",
                                            boss ? "rp_map_boss" : "sonar_enemy", mx - 13, my - 13))
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
                    int object=0;
                    if(auto bp=g.blueprints.find(selectedBlueprint_);bp!=g.blueprints.end())
                        object=bp->second.object;
                    else if(auto source=g.GetRaftByID(selectedParent_))
                        for(const auto& c:source->components)
                            if(c.id==selectedComponent_)object=c.type;
                    const auto* red=atlas->findSpriteByName("footprint_1x1_red");
                    std::unique_ptr<ItemPlacementSprite> overlay;
                    if(placementParent_)overlay=std::make_unique<ItemOnRaftPlacementSprite>(g,ocean_,image,*cell,red?*red:*cell);
                    else overlay=std::make_unique<ItemPlacementSprite>(g,ocean_,image,*cell,red?*red:*cell);
                    overlay->SetToObject(object,placementParent_,moving_?selectedParent_:0,
                                         moving_?selectedComponent_:0);
                    overlay->SetPosition(*placement_);
                    overlay->Render(out);
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
            return xmlTouch(g, x, y, old);
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
            if (diving_->confirmed() && !diveClosing_) {
                diveClosing_=true;
                diving_->moveTo(850,22,1.f);
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
                    return g.advanceDaring();
                panel_ = Panel::Daring;
                return {};
            }
            case Hud::Action::ToggleView: {
                BuildModeState build(g);
                build.BindView(&ocean_,&touchControls_);
                build.ChangeView(ocean_.Offset(),grid?BuildModeState::ViewMode::Footprints:BuildModeState::ViewMode::Normal,true);
                MsgBuildModeToggleView msg;build.gotMsgBuildModeToggleView(msg);
                grid=build.viewMode()==BuildModeState::ViewMode::Footprints;
                return {};
            }
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
                    if(g.diveSeconds>0)return g.dive("dive_info");
                    return g.collectDive();
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
                if (mapInfoEnemy_ != target->id) {
                    mapInfoEnemy_ = target->id;
                    if (xmlMenu_ && xmlName_ == "piratemap_menu") {
                        auto& m = *xmlMenu_;
                        if (auto dialog = m.find(1, "PopupDialog", true)) {
                            m.node(dialog).set("visible", true);
                            for (const auto& entry :
                                 {std::make_pair("PIRATE_NAME", target->name),
                                  std::make_pair("PIRATE_BP",
                                                 "Strength:" +
                                                     std::to_string(target->battlePoints))})
                                if (auto element = m.find(dialog, entry.first, true))
                                    if (auto text = m.find(element, "Text")) {
                                        m.node(text).set("text", entry.second);
                                        m.node(text).set("visible", true);
                                    }
                            m.solve();
                        }
                    }
                    return {};
                }
                mapInfoEnemy_.reset();
                if (mapView_.selectEnemy(*target)) {
                    enemyId = target->id;
                    usedSonar = !mapView_.boss() || mapView_.boss()->id != enemyId;
                    wx = float(target->x);
                    wy = float(target->y);
                    u = wx / (mapView_.data()->width * mapView_.data()->tileWidth);
                    v = wy / (mapView_.data()->height * mapView_.data()->tileHeight);
                }
            } else {
                mapInfoEnemy_.reset();
                mapView_.touchDown({wx, wy});
            }
            if (!enemyId)
                mapView_.touchUp({wx, wy});
            MsgConfirmMapMovement confirmation;confirmation.confirmed=true;
            if (!mapView_.gotMsgConfirmMapMovement(confirmation)) {
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
                    const auto nearest=PlacementSprite::GetClosestPosition(world,g.GetAvailableEdgeGridSpace());
                    // Snap within one tile, not to an arbitrary remote free edge.
                    if (nearest && std::abs(world.x-nearest->x-.5f)<=1.f &&
                        std::abs(world.y-nearest->y-.5f)<=1.f && g.canBuildRaft(*nearest)) {
                        placement_ = *nearest;
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
            sprite.SetPos(float(x + 13) - scale * (cell->ox + cell->w / 2.f),
                          float(y + 13) - scale * (cell->oy + cell->h / 2.f));
            drawSprite(target, sprite);
            return true;
        } catch (const std::exception&) {
            return false;
        }
    }

    void PortView::openXml(const Game& g, const std::string& name) {
        using namespace sys::menu_redux;
        if (xmlName_ != name)
            shopSelection_ = 0;
        xmlMenu_ = std::make_unique<MenuRuntime>(root_);
        xmlMenu_->reuseUnchangedLayout=true;
        lastScrollOffset_.reset();
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
            auto cached = preloadedMenus_.find(name);
            auto xml = cached == preloadedMenus_.end()
                ? sys::res::loadXML(root_ / "menus" / (name + ".xml")) : cached->second;
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
                const char* listName = name == "tabbed_menu_misc" ? "itemListMisc" : "itemList";
                if (auto* list = find(find, xml, listName)) {
                    std::vector<int> ids;
                    if (name == "tabbed_menu_blueprints")
                        ids = RPMenuHandler(g).gotMsgGetAllBlueprintsRequest();
                    else if (name == "tabbed_menu_maps")
                        for (auto& [id, v] : g.zones)
                            ids.push_back(id);
                    else {
                        ids=RPMenuHandler(g).storeItemsForTab(name);
                    }
                    for (size_t i = 0; i < ids.size(); ++i) {
                        auto key = "tabbedItem" + std::to_string(i);
                        xmlItems_[key] = ids[i];
                        add(*list, "TabbedMenuItem", key, int(i) * 128, 0, listName, "BOTTOM",
                            "LEFT");
                        auto& orientation=list->children.back().children.back();
                        orientation.attributes["xOffset"]="screenWidth()*0.2*"+std::to_string(i);
                        orientation.attributes["priority"]="-1";
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
                        add(*list, "QuestButton", key, -16, 48 + int(i++) * 55, "children", "TOP",
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
            if (name == "daring_popup") {
                std::string description = "DARING_METER_DESC";
                bool locked = g.daringLocked();
                if (g.safe)
                    description = "DARING_METER_DESC_INSAFEWATER";
                else if (g.badZone)
                    description = "DARING_METER_DESC_BADZONE";
                else if (locked)
                    description = "DARING_METER_DESC_ZONELIMIT";
                if (auto e = m.find(1, "TutorialText", true))
                    if (auto t = m.find(e, "Text"))
                        m.node(t).set("text", strings_.getText(description));
                if (auto e = m.find(1, "MainButton", true)) {
                    m.node(e).set("gotostore", !locked);
                    if (auto t = m.find(e, "Text"))
                        m.node(t).set("text", strings_.getText(locked ? "OK" : "FILL_IT_NOW"));
                }
                m.solve();
                if (auto popup = m.find(1, "PopupDialog", true))
                    if (auto bg = m.find(popup, "bg", true))
                        if (auto s = m.find(bg, "Sprite")) {
                            auto text = m.find(1, "TutorialText", true);
                            auto button = m.find(1, "MainButton", true);
                            if (text && button)
                                m.node(s).set("height", double(m.node(text).bounds.h +
                                                               m.node(button).bounds.h + 18));
                        }
            } else if (name == "piratemap_menu") {
                for (auto n : {"PopupDialog", "TutorialArrow"})
                    show(n, false);
                show("MapActionsIcon", g.mapActionMax > 0);
                if(auto icon=m.find(1,"MapActionsIcon",true)){
                    auto& n=m.node(icon);n.set("absoluteX",70.0);n.set("absoluteY",double(m.height()-42));n.set("width",170.0);n.set("height",38.0);
                    auto place=[&](const char* name,float x,float y){if(auto id=m.find(icon,name)){auto& c=m.node(id);c.set("absoluteX",double(x));c.set("absoluteY",double(y));}};
                    place("Sprite",70,m.height()-42);place("SpriteBarBG",100,m.height()-27);place("SpriteBarFG",100,m.height()-24);
                    if(auto ball=m.find(icon,"SpriteBarBallBG")){
                        auto& n=m.node(ball);n.set("absoluteX",double(100+m.node(m.find(icon,"SpriteBarBG")).bounds.w-4));
                        n.set("absoluteY",double(m.height()-34));
                    }
                    place("TextRegenLimit",145,m.height()-27);place("TextMoveTimer",145,m.height()-45);
                    if(auto label=m.find(icon,"TextMoveCount"))m.node(label).set("visible",false);
                }
                show("Discoveries",false);show("Zones",false);
                if (auto e = m.find(1, "AutoSafeWater", true)) {
                    m.node(e).set("xOffset", -3.0);
                    if (auto icon = m.find(e, "Sprite"))
                        m.node(icon).set(
                            "spriteName",
                            std::string(g.autoSafe ? "btn_autosafe_on" : "btn_autosafe_off"));
                }
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
            } else if (name == "auto_safe_water_menu") {
                for (const auto& entry : {std::make_pair("SafeDescrText", g.autoSafe),
                                          std::make_pair("UnsafeDescrText", !g.autoSafe)})
                    if (auto id = m.find(1, entry.first, true))
                        if (auto label = m.find(id, "Text"))
                            m.node(label).set("alpha", entry.second ? 1.0 : 0.5);
                for (const auto& entry : {std::make_pair("SafeFace", g.autoSafe),
                                          std::make_pair("UnsafeFace", !g.autoSafe)})
                    if (auto id = m.find(1, entry.first))
                        m.node(id).set("alpha", entry.second ? 1.0 : 0.5);
                if (auto id = m.find(1, "Toggle", true))
                    if (auto label = m.find(id, "Text"))
                        m.node(label).set("text",
                                          strings_.getText(g.autoSafe ? "SAFE_WATER_ENABLED"
                                                                      : "SAFE_WATER_DISABLED"));
            } else if (name == "quest_log") {
                if (auto dialog = m.find(1, "DialogPopup", true)) {
                    // SlideInDialog's LEFT end position reveals one quarter of the viewport.
                    const float dialogWidth = m.node(dialog).bounds.w;
                    const float dialogHeight = m.node(dialog).bounds.h;
                    // Keep the visible quest-book page and its buttons inside the viewport.
                    m.node(dialog).set("absoluteX", 0.0);
                    m.node(dialog).set("absoluteY", double(std::max(0.f,(m.height()-dialogHeight)*.5f)));
                    m.node(dialog).set("layer", std::string("HUD"));
                    if (auto sprite = m.find(dialog, "DialogSprite"))
                        m.node(sprite).set("hFlip", 1.0);
                    if (auto children = m.find(dialog, "children")) {
                        m.node(children).set("width", double(std::min(dialogWidth,m.width()*.25f)));
                        m.node(children).set("absoluteX", 0.0);
                        m.node(children).set("absoluteY", double(std::max(0.f,(m.height()-dialogHeight)*.5f)));
                        m.node(children).set("height", double(dialogHeight));
                        m.node(children).set("clipChildren", true);
                        m.solve();
                        float y = m.height() * .12f;
                        for (int quest : RPMenuHandler(g).gotMsgQuestInformationRequest()) {
                            auto el = m.find(children, "quest_" + std::to_string(quest), true);
                            if (!el) continue;
                            m.node(el).set("xOffset", -8.0);
                            m.node(el).set("yOffset", double(y));
                            m.node(el).set("priority", -4.0);
                            if(auto touch=m.find(el,"Touch"))m.node(touch).set("priority",-5.0);
                            m.node(el).set("layer", std::string("HUD"));
                            y += m.node(el).bounds.h + 4;
                        }
                        m.solve();
                        if(auto touch=m.find(dialog,"Touch")){
                            m.node(touch).set("width",double(std::min(dialogWidth,m.width()*.25f)));
                            m.node(touch).set("height",double(dialogHeight));
                            m.node(touch).set("priority",10.0);
                        }
                        if ((swipe_ = m.find(dialog, "Swiper"))) {
                            scrollVertical_ = true;
                            m.node(swipe_).set("direction", 1.0);
                            m.node(swipe_).set("mode", 1.0);
                            m.listenToTouches(swipe_, dialog);
                            m.setWindowSize(swipe_, m.node(dialog).bounds.h);
                            m.setScrollSize(swipe_,
                                            std::max(0.f, y - m.node(dialog).bounds.h));
                            for (auto& [key, item] : xmlItems_)
                                if (auto el = m.find(children, key, true))
                                    scrollOrigins_[el] = m.node(el).number("yOffset", 48);
                        }
                    }
                }
            } else if (name.rfind("tabbed_menu_", 0) == 0) {
                // Information frame is populated when an item is selected; do not show its
                // uninitialized template.
                show("Frame", false);
                // Match MenuTab and MenuTabCurrency's original onInit/onPostSolve placement.
                m.setLayerOrder("TABS", 3);
                // The original tab scripts apply dynamic positions after the initial solve.
                // Reserve a compact strip above the scrolling list, without overlapping touch areas.
                float listTop=m.height()*.75f;
                if(auto background=m.find(1,"bgBluePrint",true))listTop=m.node(background).bounds.y;
                listTop=std::clamp(listTop,m.height()*.65f,m.height()*.8f);
                float left=8;
                for(int i=1;i<=7;++i){
                    auto id=m.find(1,"Tab"+std::to_string(i),true);if(!id)continue;
                    const float width=i<=4?60.f:124.f;
                    const float height=42.f;
                    const float x=i<=4?left:m.width()-8.f-width*(7-i+1);
                    if(i<=4)left+=width+2;
                    auto& tab=m.node(id);
                    tab.set("absoluteX",double(i>=5&&i<=6&&!m.find(1,"Tab7",true)?x+width:x));
                    tab.set("absoluteY",double(listTop-height));tab.set("width",double(width));tab.set("height",double(height));
                    tab.set("layer",std::string("TABS"));
                    if(auto bg=m.find(id,"bg")){
                        auto& n=m.node(bg);n.set("includeBorder",false);n.set("width",double(width));n.set("height",double(height));
                    }
                    if(auto icon=m.find(id,"Icon")){
                        auto& n=m.node(icon);n.set("relativeAnchorX",i<=4?.5:1.0);n.set("orientationAnchorX",i<=4?.5:1.0);n.set("xOffset",i<=4?0.0:-4.0);n.set("relativeAnchorY",.5);n.set("orientationAnchorY",.5);n.set("yOffset",0.0);
                    }
                    if(auto touch=m.find(id,"Touch")){auto& n=m.node(touch);n.set("width",double(width));n.set("height",double(height));}
                    if(auto label=m.find(id,"Text"))m.node(label).set("text",std::to_string(g.numOfMaterialPossessed(i==5?4:11)));
                }
                if (auto list =
                        m.find(1, name == "tabbed_menu_misc" ? "itemListMisc" : "itemList", true))
                    if ((swipe_ = m.find(list, "Swiper"))) {
                        m.node(list).set("absoluteX", 0.0);
                        m.node(list).set("width", double(m.width()));
                        m.node(list).set("absoluteY", double(listTop));
                        m.node(list).set("height", double(m.height()-listTop));
                        m.node(list).set("layer", std::string("HUD"));
                        m.node(list).set("clipChildren", true);
                        if (auto touch = m.find(list, "Touch")) {
                            m.node(touch).set("width", double(m.width()));
                            m.node(touch).set("height", double(m.node(list).bounds.h));
                            m.node(touch).set("layer", std::string("HUD"));
                        }
                        m.solve();
                        m.node(swipe_).set("direction", 0.0);
                        m.node(swipe_).set("mode", 1.0);
                        m.listenToTouches(swipe_, list);
                        m.setScrollSize(
                            swipe_,
                            std::max(0.f, float(xmlItems_.size()) * m.width() * .2f - m.width()));
                        for (auto& [key, id] : xmlItems_)
                            if (auto el = m.find(list, key, true)) {
                                auto index = std::stoi(key.substr(10));
                                m.node(el).set("layer", std::string("HUD"));
                                scrollOrigins_[el] = 12 + index * m.width() * .2;
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
                        set(element, "itemName", "size", .65);
                        set(element, "itemName", "visible", !g.owned.count(id));
                        set(element, "itemCost", "visible", false);
                        set(element, "currencyIcon", "visible", false);
                        set(element, "selectionOverlay", "visible", false);
                        if (!g.owned.count(id))
                            for (auto c : {"red", "green", "blue"})
                                set(element, "itemImage", c, .55);
                        else
                            for (auto c : {"red", "green", "blue"})
                                set(element, "itemImage", c, 1.0);
                        set(element, "itemImageSecondaryCentered", "visible", false);
                        if (g.owned.count(id) && !RPMenuHandler(g).gotMsgHaveMaterialsRequest(id))
                            set(element, "itemImage", "alpha", .4);
                    } else if (name == "quest_log") {
                        const auto& q = g.quests.at(id);
                        set(element, "ProgressText", "text", std::to_string(q.progress) + "%");
                        set(element, "ProgressText", "visible", q.progress > 0 && q.progress < 100);
                        if (!q.sheet.empty() && !q.image.empty()) {
                            set(element, "ItemImage", "spriteName", q.image);
                            set(element, "ItemImage", "sheetName", q.sheet);
                        }
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
                        // Original gotStoreItemInfo sets the template's Layer to HUD.
                        m.node(element).set("layer",std::string("HUD"));
                        set(element, "itemImage", "spriteName", item.image);
                        set(element, "itemImage", "sheetName", item.sheet);
                        const bool cashTab=name=="tabbed_menu_gold"||name=="tabbed_menu_diamonds";
                        const bool freeTab=name=="tabbed_menu_free";
                        const bool offer=item.group=="offerwall";
                        set(element, "itemCost", "text", offer?strings_.getText("OFFERS"):std::to_string(item.price));
                        set(element, "itemName", "text",
                            strings_.getText(item.title.empty() ? item.name : item.title));
                        set(element,"itemName","visible",false);
                        set(element,"itemImageSecondaryCentered","visible",false);
                        set(element, "itemImage", "visible", true);
                        set(element, "selectionOverlay", "visible", false);
                        set(element, "itemCost", "visible", !freeTab);
                        set(element, "currencyIcon", "visible", !cashTab&&!freeTab&&!offer);
                        if(cashTab||offer)set(element,"itemCost","xOffset",0.0);
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
            if (name.rfind("tabbed_menu_", 0) == 0 && name != "tabbed_menu_inventory") {
                for (auto& [key, item] : xmlItems_) {
                    auto element = m.find(1, key, true);
                    if (!element)
                        continue;
                    if (auto image = m.find(element, "itemImage")) {
                        const auto b = m.node(image).bounds;
                        const auto slot = m.node(element).bounds;
                        if (b.w > 0 && b.h > 0 && slot.w > 0 && slot.h > 0) {
                            double factor =
                                std::min(double(slot.w * .9 / b.w), double(slot.h * .72 / b.h));
                            m.node(image).set("size", m.node(image).number("size", 1) * factor);
                        }
                    }
                    if (!requestedStoreItem_.empty() && g.storeItems.count(item) &&
                        g.storeItems.at(item).name == requestedStoreItem_) {
                        shopSelection_ = item;
                        if (swipe_) {
                            auto origin = scrollOrigins_.find(element);
                            if (origin != scrollOrigins_.end())
                                m.setScrollOffset(
                                    swipe_, -float(std::max(0.0, origin->second - m.width() / 2)));
                        }
                    }
                }
                requestedStoreItem_.clear();
                if (!shopSelection_ && xmlName_ != "tabbed_menu_maps")
                    if (auto first = xmlItems_.find("tabbedItem0"); first != xmlItems_.end())
                        shopSelection_ = first->second;
                updateShopFrame(g);
                m.solve();
            }
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
        if(xmlMenu_&&xmlName_==name&&name.rfind("tabbed_menu_",0)==0){
            std::vector<int> expected;
            if(name=="tabbed_menu_inventory")expected=RPMenuHandler(g).gotMsgGetAvailMaterialsRequest();
            else if(name=="tabbed_menu_blueprints")expected=RPMenuHandler(g).gotMsgGetAllBlueprintsRequest();
            else if(name=="tabbed_menu_maps"){for(const auto& entry:g.zones)expected.push_back(entry.first);}
            else expected=RPMenuHandler(g).storeItemsForTab(name);
            if(expected.size()!=xmlItems_.size())refresh=true;
            for(size_t i=0;!refresh&&i<expected.size();++i){
                const auto key=name=="tabbed_menu_inventory"?"material_"+std::to_string(expected[i]):"tabbedItem"+std::to_string(i);
                auto it=xmlItems_.find(key);
                if(it==xmlItems_.end()||it->second!=expected[i])refresh=true;
            }
        }
        if (!xmlMenu_ || xmlName_ != name || refresh)
            openXml(g, name);
        if (!xmlMenu_)
            return;
        if (name == "piratemap_menu") {
            if (auto icon = xmlMenu_->find(1, "MapActionsIcon", true)) {
                xmlMenu_->node(icon).set("visible", g.mapActionMax > 0);
                if(auto fill=xmlMenu_->find(icon,"SpriteBarFG"))xmlMenu_->node(fill).set("barFraction",g.mapActionMax>0?std::clamp(double(g.mapActions)/g.mapActionMax,0.0,1.0):0.0);
                if (auto label = xmlMenu_->find(icon, "TextRegenLimit"))
                    xmlMenu_->node(label).set("text", std::to_string(g.mapActions));
                if (auto label = xmlMenu_->find(icon, "TextMoveTimer")) {
                    xmlMenu_->node(label).set("text", std::to_string(std::max(0, g.mapActionTime)));
                    xmlMenu_->node(label).set("visible", g.mapActions < g.mapActionMax);
                }
            }
            if (auto autoSafe = xmlMenu_->find(1, "AutoSafeWater", true))
                if (auto sprite = xmlMenu_->find(autoSafe, "Sprite"))
                    xmlMenu_->node(sprite).set(
                        "spriteName",
                        std::string(g.autoSafe ? "btn_autosafe_on" : "btn_autosafe_off"));
            if (auto sonar = xmlMenu_->find(1, "PingSonar", true))
                if (auto label = xmlMenu_->find(sonar, "Text"))
                    xmlMenu_->node(label).set("text", std::to_string(g.numOfMaterialPossessed(12)));
        }
        if(name.rfind("tabbed_menu_",0)==0){
            for(int tab=5;tab<=7;++tab)
                if(auto id=xmlMenu_->find(1,"Tab"+std::to_string(tab),true))
                    if(auto label=xmlMenu_->find(id,"Text"))
                        xmlMenu_->node(label).set("text",std::to_string(g.numOfMaterialPossessed(tab==5?4:11)));
            if(name=="tabbed_menu_inventory")
                for(const auto& [key,id]:xmlItems_)
                    if(auto entry=xmlMenu_->find(1,key,true))
                        if(auto amount=xmlMenu_->find(entry,"Amount"))
                            xmlMenu_->node(amount).set("text",std::to_string(g.numOfMaterialPossessed(unsigned(id))));
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
        if ((xmlName_ == "sonar_popup" || xmlName_ == "auto_safe_water_menu") &&
            panel_ == Panel::Safety)
            name = xmlName_;
        renderXml(image, g, name);
    }

    void PortView::updateShopFrame(const Game& g) {
        if (!xmlMenu_)
            return;
        auto& m = *xmlMenu_;
        auto frame = m.find(1, "Frame", true);
        if (!frame)
            return;
        bool blueprint = xmlName_ == "tabbed_menu_blueprints";
        bool valid = shopSelection_ > 0 && (blueprint ? g.blueprints.count(shopSelection_)
                                                      : g.storeItems.count(shopSelection_));
        m.node(frame).set("visible", valid);
        if (!valid)
            return;
        auto prop = [&](unsigned parent, const char* node, const char* key,
                        sys::menu_redux::Property value) {
            if (auto id = m.find(parent, node))
                m.node(id).set(key, std::move(value));
        };
        auto btn = m.find(frame, "buyBtn", true);
        std::string title, sheet, image;
        const StoreItem* store = nullptr;
        if (blueprint) {
            const auto& b = g.blueprints.at(shopSelection_);
            title = b.name();
            sheet = b.sheet;
            image = b.image;
            for (const auto& [id, item] : g.storeItems)
                if (item.name == title) {
                    store = &item;
                    break;
                }
            unsigned i = 0;
            for (const auto& [id, amount] : b.recipe) {
                if (++i > 4)
                    break;
                if (auto material = m.find(frame, "Material" + std::to_string(i), true)) {
                    m.node(material).set("visible", true);
                    if (auto icon = m.find(material, "ItemImage", true))
                        if (auto info = g.materialDB.getMaterialInfo(unsigned(id))) {
                            for (auto child : m.node(icon).children) {
                                m.node(child).set("sheetName", info->sheet);
                                m.node(child).set("spriteName", info->image);
                            }
                        }
                    if (auto req = m.find(material, "MaterialReqs", true))
                        prop(req, "Text", "text",
                             std::to_string(g.numOfMaterialPossessed(id)) + "/" +
                                 std::to_string(amount));
                }
            }
            for (; i < 4; ++i)
                if (auto el = m.find(frame, "Material" + std::to_string(i + 1), true))
                    m.node(el).set("visible", false);
            for (auto name :
                 {"lockedOverlay", "constructionJerks", "buildLimitOverlay", "buildLimitExplain"})
                prop(frame, name, "visible", false);
            for (auto name : {"Footprint", "BP"})
                if (auto el = m.find(frame, name, true))
                    m.node(el).set("visible", false);
        } else {
            store = &g.storeItems.at(shopSelection_);
            title = store->title.empty() ? store->name : store->title;
            sheet = store->sheet;
            image = store->image;
            prop(frame, "itemDesc", "text", std::string());
            for (auto name : {"CommonText", "UnCommonText", "RareText"})
                if (auto el = m.find(frame, name, true))
                    m.node(el).set("visible", false);
        }
        prop(frame, "itemTitle", "text", strings_.getText(title));
        if(blueprint && !sheet.empty() && !image.empty())try{
            auto atlas=ResourceSpriteSheet::Create(root_/"xml_resources"/std::filesystem::path(sheet).filename());
            if(atlas.findSpriteByName(image+"_green"))image+="_green";
        }catch(const std::exception&){}
        prop(frame, "itemImage", "sheetName", sheet);
        prop(frame, "itemImage", "spriteName", image);
        prop(frame, "itemImage", "visible", !image.empty());
        if (btn) {
            bool build = blueprint && g.owned.count(shopSelection_);
            if(build){prop(btn,"UpSprite","spriteName",std::string("btn_long_green"));prop(btn,"DownSprite","spriteName",std::string("btn_long_green_over"));}
            else{prop(btn,"UpSprite","spriteName",std::string("btn_long_blue"));prop(btn,"DownSprite","spriteName",std::string("btn_long_blue_over"));}
            prop(btn, "Text", "text", strings_.getText(build ? "BUILD" : "BUY_MENU_BUTTON_PREFIX"));
            prop(btn, "PriceText", "visible", !build && store);
            prop(btn, "currencySprite", "visible", !build && store);
            if (store && !build) {
                prop(btn, "PriceText", "text", std::to_string(store->price));
                prop(btn, "currencySprite", "sheetName", std::string("rp_menu"));
                prop(btn, "currencySprite", "spriteName",
                     std::string(store->currency == 11 ? "shop_diamond_mini" : "shop_gold_mini"));
            }
        }
        for (const auto& [key, id] : xmlItems_)
            if (auto el = m.find(1, key, true))
                prop(el, "selectionOverlay", "visible", id == shopSelection_);
        m.solve();
        if(blueprint && btn){
            const auto b=m.node(frame).bounds;
            auto place=[&](unsigned id,float x,float y,float w,float h){if(!id)return;auto& n=m.node(id);n.set("absoluteX",double(x));n.set("absoluteY",double(y));if(w>=0)n.set("width",double(w));if(h>=0)n.set("height",double(h));};
            place(m.find(frame,"itemTitle"),b.x+8,b.y+5,b.w-16,20);
            prop(frame,"itemTitle","alignment",1.0);prop(frame,"itemTitle","autoScale",true);
            if(auto icon=m.find(frame,"itemImage")){
                auto ib=m.node(icon).bounds;float factor=std::min(1.f,50.f/std::max(1.f,std::max(ib.w,ib.h)));
                place(icon,b.x+(b.w-ib.w*factor)*.5f,b.y+24,ib.w*factor,ib.h*factor);
            }
            auto count=std::min<size_t>(4,g.blueprints.at(shopSelection_).recipe.size());
            const float step=(b.w-20)/float(std::max<size_t>(1,count));
            for(unsigned i=0;i<count;++i)if(auto material=m.find(frame,"Material"+std::to_string(i+1),true)){
                const float x=b.x+10+step*i, y=b.y+b.h-76;
                place(material,x,y,step,36);
                if(auto icon=m.find(material,"ItemImage",true)){
                    place(icon,x+(step-24)*.5f,y,24,24);
                    if(auto sprite=m.find(icon,"Sprite"))place(sprite,x+(step-24)*.5f,y,24,24);
                }
                if(auto req=m.find(material,"MaterialReqs",true)){
                    place(req,x,y+24,step,14);
                    if(auto label=m.find(req,"Text")){place(label,x,y+24,step,14);auto& n=m.node(label);n.set("alignment",1.0);n.set("multiline",false);n.set("autoScale",true);n.set("size",.55);}
                }
            }
            const auto bb=m.node(btn).bounds;
            place(btn,b.x+(b.w-bb.w)*.5f,b.y+b.h-bb.h-5,-1,-1);
            if(auto text=m.find(btn,"Text")){auto& n=m.node(text);n.set("relativeAnchorX",.5);n.set("orientationAnchorX",.5);n.set("xOffset",0.0);n.set("yOffset",0.0);}
            m.solve();
        }
    }

    std::optional<sfs::Message> PortView::xmlTouch(Game& g, float x, float y, unsigned captured) {
        if (!xmlMenu_)
            return {};
        auto& m = *xmlMenu_;
        auto hit = captured ? captured : m.hitTest(x, y);
        if (!hit)
            return {};
        auto owner = m.owner(hit);
        auto name = m.node(owner).name;
        auto body = m.node(hit).events.find("onTouchUp");
        std::string script = body == m.node(hit).events.end() ? "" : body->second.source;
        if (panel_ == Panel::Daring && name == "MainButton") {
            if (m.node(owner).number("gotostore", 0)) {
                requestedStoreItem_ = "daring";
                panel_ = Panel::Shop;
                openXml(g, "tabbed_menu_misc");
            } else {
                panel_ = Panel::None;
                xmlMenu_.reset();
            }
            return {};
        }
        if (xmlName_ == "auto_safe_water_menu" && name == "Toggle") {
            sfs::SFSObjectWrapper packet;
            packet.put("is_enabled", !g.autoSafe);
            panel_ = Panel::None;
            xmlMenu_.reset();
            return Game::request("set_auto_safe_enabled", packet);
        }
        if (name == "buyBtn" && shopSelection_ > 0 && xmlName_.rfind("tabbed_menu_", 0) == 0) {
            int item = shopSelection_;
            if (xmlName_ == "tabbed_menu_blueprints") {
                if (BuildModeState(g).gotMsgStartBuildRequest(item)) {
                    selectedBlueprint_ = item;
                    selectedComponent_ = selectedParent_ = 0;
                    placement_.reset();
                    grid = true;
                    map = false;
                    panel_ = Panel::None;
                    xmlMenu_.reset();
                    return {};
                }
                if (!g.owned.count(item))
                    for (const auto& [id, store] : g.storeItems)
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
            if (RPMenuHandler(g).canPurchase(item)) {
                selectedStore_ = item;
                confirmAction_ = "purchase";
                panel_ = Panel::Confirm;
                openXml(g, "purchase_confirm_dialog_tabbed");
            }
            return {};
        }
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
            if (mapInfoEnemy_ &&
                (name == "DISMISS_BUTTON" || name == "TOUCH_CATCHER" || name == "BG")) {
                if (name != "BG") {
                    if (auto dialog = m.find(1, "PopupDialog", true))
                        m.node(dialog).set("visible", false);
                    // Retain the inspected marker until movement or leaving the map.
                }
                return {};
            }
            if (name == "Back") {
                map = false;
                mapInfoEnemy_.reset();
                xmlMenu_.reset();
                return {};
            }
            if (name == "AutoSafeWater") {
                panel_ = Panel::Safety;
                openXml(g, "auto_safe_water_menu");
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
        // Native tab event scripts are consumed by this host; keep their navigation explicit.
        if(xmlName_.rfind("tabbed_menu_",0)==0 && name.size()==4 && name.rfind("Tab",0)==0){
            static const char* menus[]={"tabbed_menu_blueprints","tabbed_menu_inventory","tabbed_menu_misc","tabbed_menu_free","tabbed_menu_gold","tabbed_menu_diamonds"};
            const int tab=name[3]-'1';
            if(tab>=0 && tab<6){panel_=Panel::Shop;if(xmlName_!=menus[tab])openXml(g,menus[tab]);return {};}
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
            if (xmlName_ == "quest_log") {
                selectedQuest_ = item;
                panel_ = Panel::QuestDetail;
                openXml(g, "quest_dialog");
                MsgQuestRead request;request.id=item;
                return RPMenuHandler(g).gotMsgQuestRead(g,request);
            }
            if (xmlName_ == "tabbed_menu_maps") {
                panel_ = Panel::None;
                xmlMenu_.reset();
                return g.travel(item);
            }
            if (xmlName_ != "tabbed_menu_inventory") {
                shopSelection_ = item;
                updateShopFrame(g);
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
