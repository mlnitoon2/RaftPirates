#include "rp/PortView.h"
#include "rp/ItemPlacementSprite.h"
#include "rp/ItemOnRaftPlacementSprite.h"
#include "rp/RaftPirates.h"
#include "rp/RPMenuHandler.h"
#include "rp/BuildModeState.h"
#include "rp/GridSprite.h"
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
        if (diving_) {
            diving_->tick(dt);
            if(diving_->finished()&&!diveClosing_&&!diving_->moving()){
                diving_->ConfirmDiveFinish();
                diveClosing_=true;
                diving_->moveTo(850,22,1.f);
            }
        }
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
        blueprintGesture_.reset();
    }
    void PortView::touchDown(float x, float y) {
        xmlCaptured_ = false;
        blueprintGesture_.reset();
        if(xmlMenu_&&xmlName_=="tabbed_menu_blueprints"&&panel_==Panel::Shop&&y>=blueprintListTop_&&y<400&&x>=0&&x<640){
            blueprintGesture_=Vec2{x,y};oceanGesture_=false;return;
        }
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
        if(blueprintGesture_)return;
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
                    // Render the actual catalogue object in addition to the placement footprint.
                    if(auto def=g.objects.find(object);def!=g.objects.end()){
                        GridObject preview;preview.type=object;preview.x=placement_->x;preview.y=placement_->y;preview.bp=def->second.bp;
                        Vec2 world{float(preview.x),float(preview.y)};
                        if(placementParent_)if(auto parent=g.GetRaftByID(placementParent_))
                            world={parent->x+(preview.x+.5f)/6.f,parent->y+(preview.y+.5f)/6.f};
                        if(!previewRenderer_)previewRenderer_=std::make_unique<AnimationRenderer>(root_);
                        GridSprite sprite(ocean_,preview,def->second,world,ocean_.Zoom());
                        sprite.SetColour(255,255,255,180);sprite.SetTime(time);
                        GfxManager gfx;sprite.submit(gfx,previewImages_,previewBins_,*previewRenderer_,root_,!placementParent_);gfx.RenderScreen(out);
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
        if(blueprintGesture_){
            auto start=*blueprintGesture_;blueprintGesture_.reset();
            if(xmlName_!="tabbed_menu_blueprints"||!xmlMenu_)return {};
            const float dx=x-start.x;
            if(std::abs(dx)>30)blueprintPageMove(g,dx<0?1:-1);
            else if(std::abs(y-start.y)<20&&y>=blueprintListTop_&&y<400)selectBlueprintSlot(g,x);
            return {};
        }
        if(xmlMenu_&&panel_==Panel::Safety&&xmlName_=="auto_safe_water_menu"){
            if(auto back=xmlMenu_->find(1,"BackSprite")){
                if(xmlMenu_->node(back).bounds.contains(x,y)){cancelInput();panel_=Panel::None;xmlMenu_.reset();return {};}
            }
        }
        if (xmlCaptured_ && xmlMenu_) {
            xmlCaptured_ = false;
            auto old = xmlMenu_->captured(0);
            const bool inside=old&&xmlMenu_->node(old).bounds.contains(x,y);
            xmlMenu_->touchUp(x, y);
            updateScroll();
            if (!inside || !old || xmlMenu_->node(old).number("suppressClick", 0))
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

    std::vector<int> PortView::menuItemIds(const Game& g,const std::string& name)const {
        RPMenuHandler handler(g);
        if(name=="tabbed_menu_inventory"){
            auto ids=handler.gotMsgGetAvailMaterialsRequest();
            for(const auto& [id,info]:g.materialDB.rows())
                if(id>0&&!info.image.empty()&&std::find(ids.begin(),ids.end(),id)==ids.end())ids.push_back(id);
            std::sort(ids.begin(),ids.end());
            return ids;
        }
        if(name=="tabbed_menu_blueprints"){
            auto ids=handler.gotMsgGetAllBlueprintsRequest();
            auto first=std::min(page_*BlueprintPageSize,ids.size());
            auto last=std::min(first+BlueprintPageSize,ids.size());
            return std::vector<int>(ids.begin()+first,ids.begin()+last);
        }
        if(name=="tabbed_menu_maps"){std::vector<int> ids;for(const auto& z:g.zones)ids.push_back(z.first);return ids;}
        auto ids=handler.storeItemsForTab(name);
        // These original Free/Settings entries are local actions, not store records.
        if(name=="tabbed_menu_free")ids.insert(ids.begin(),{-1,-2,-3});
        return ids;
    }

    std::string PortView::blueprintIcon(const std::string& sheet,const std::string& image){
        const auto key=std::make_pair(sheet,image);
        if(auto it=blueprintIcons_.find(key);it!=blueprintIcons_.end())return it->second;
        std::string result=image;
        if(!sheet.empty()&&!image.empty())try{
            auto path=std::filesystem::path(sheet);
            if(!path.has_extension())path.replace_extension(".xml");
            if(path.parent_path().empty())path="xml_resources"/path;
            const auto name=path.generic_string();
            auto it=blueprintSheets_.find(name);
            if(it==blueprintSheets_.end())it=blueprintSheets_.emplace(name,loader_.sheet(name)).first;
            if(it->second->findSpriteByName(image+"_green"))result=image+"_green";
        }catch(const std::exception&){}
        blueprintIcons_.emplace(key,result);return result;
    }

    bool PortView::blueprintPageMove(const Game& g,int delta){
        const auto count=RPMenuHandler(g).gotMsgGetAllBlueprintsRequest().size();
        if(!count)return false;
        auto next=std::clamp<long long>(static_cast<long long>(page_)+delta,0,static_cast<long long>((count-1)/BlueprintPageSize));
        if(size_t(next)==page_)return false;
        page_=size_t(next);shopSelection_=0;openXml(g,"tabbed_menu_blueprints");return true;
    }

    void PortView::selectBlueprintSlot(const Game& g,float x){
        if(x<0||x>=640)return;
        const auto key="tabbedItem"+std::to_string(std::min(4,int(x/128)));
        auto it=xmlItems_.find(key);
        if(it==xmlItems_.end()||it->second==shopSelection_)return;
        shopSelection_=it->second;updateShopFrame(g);
    }

    void PortView::openXml(const Game& g, const std::string& name) {
        using namespace sys::menu_redux;
        if (xmlName_ != name){
            shopSelection_=0;
            page_=0;
        }
        blueprintGesture_.reset();
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
                auto ids = menuItemIds(g,name);
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
                    auto ids=menuItemIds(g,name);
                    for (size_t i = 0; i < ids.size(); ++i) {
                        auto key = "tabbedItem" + std::to_string(i);
                        xmlItems_[key] = ids[i];
                        add(*list, name=="tabbed_menu_free"&&ids[i]<0?"TabbedSocialItem":"TabbedMenuItem", key, int(i) * 128, 0, listName, "BOTTOM",
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
                    const float x=70,y=m.height()-88;
                    auto& n=m.node(icon);n.set("absoluteX",double(x));n.set("absoluteY",double(y));n.set("width",141.0);n.set("height",50.0);
                    // All four original cells share the same logical canvas and carry their own trim offsets.
                    for(auto name:{"Sprite","SpriteBarBG","SpriteBarFG","SpriteBarBallBG"})
                        if(auto id=m.find(icon,name)){auto& c=m.node(id);c.set("absoluteX",double(x));c.set("absoluteY",double(y));c.set("width",141.0);c.set("height",50.0);c.set("useOffsets",true);}
                    if(auto id=m.find(icon,"TextRegenLimit")){auto& c=m.node(id);c.set("absoluteX",double(x+111));c.set("absoluteY",double(y+22));c.set("width",29.0);c.set("height",20.0);c.set("alignment",1.0);c.set("size",.7);}
                    if(auto id=m.find(icon,"TextMoveTimer")){auto& c=m.node(id);c.set("absoluteX",double(x+25));c.set("absoluteY",double(y+3));c.set("width",96.0);c.set("height",18.0);c.set("alignment",1.0);}
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
                // The original background height depends on the measured text and button.
                m.solve();
                if(auto dialog=m.find(1,"DialogPopup",true))if(auto children=m.find(dialog,"children",true))if(auto bg=m.find(children,"bg",true)){
                    const float left=m.width()*.14f, width=m.width()*.72f;
                    float y=40;
                    auto placeText=[&](const char* name,float size,float maxH){
                        if(auto el=m.find(children,name,true))if(auto text=m.find(el,"Text")){
                            auto& n=m.node(text);n.set("absoluteX",double(left+12));n.set("absoluteY",double(y));n.set("width",double(width-24));n.set("height",double(maxH));n.set("size",double(size));n.set("alignment",1.0);n.set("autoScale",true);n.set("multiline",true);n.set("clipText",true);n.set("priority",-10.0);
                            y+=maxH+10;
                        }
                    };
                    placeText("TitleText",.85f,28);
                    placeText("SafeDescrText",.8f,77);
                    if(auto toggle=m.find(children,"Toggle",true)){
                        auto& n=m.node(toggle);const float w=std::clamp(n.bounds.w,120.f,190.f),h=std::clamp(n.bounds.h,28.f,36.f);
                        n.set("width",double(w));n.set("height",double(h));n.set("absoluteX",double((m.width()-w)*.5f));n.set("absoluteY",double(y));
                        if(auto touch=m.find(toggle,"Touch")){auto& t=m.node(touch);t.set("width",double(w));t.set("height",double(h));t.set("priority",-15.0);t.set("layer",std::string("PIRATE_MAP_HUD"));}
                        y+=h+12;
                    }
                    placeText("UnsafeDescrText",.8f,77);
                    auto& n=m.node(bg);n.set("absoluteX",double(left));n.set("absoluteY",30.0);n.set("width",double(width));n.set("height",double(std::min(m.height()-48,y-20)));
                    if(auto sprite=m.find(bg,"Sprite")){auto& s=m.node(sprite);s.set("absoluteX",double(left));s.set("absoluteY",30.0);s.set("width",double(width));s.set("height",n.number("height"));s.set("includeBorder",false);}
                    if(auto back=m.find(children,"BackSprite")){
                        auto& n=m.node(back);n.set("absoluteX",double(left-24));n.set("absoluteY",8.0);n.set("width",24.0);n.set("height",24.0);n.set("useOffsets",false);
                    }
                }
            } else if (name == "quest_log") {
                if (auto dialog = m.find(1, "DialogPopup", true)) {
                    // SlideInDialog's LEFT end position reveals one quarter of the viewport.
                    const float dialogWidth = std::max(160.f,m.node(dialog).bounds.w);
                    const float dialogHeight = std::min(m.height(),std::max(300.f,m.node(dialog).bounds.h));
                    // Keep the visible quest-book page and its buttons inside the viewport.
                    m.node(dialog).set("absoluteX", double(m.width()*.25f-dialogWidth));
                    m.node(dialog).set("width",double(dialogWidth));m.node(dialog).set("height",double(dialogHeight));
                    m.node(dialog).set("absoluteY", double(std::max(0.f,(m.height()-dialogHeight)*.5f)));
                    m.node(dialog).set("layer", std::string("HUD"));
                    if (auto sprite = m.find(dialog, "DialogSprite")){
                        auto& n=m.node(sprite);n.set("hFlip",1.0);n.set("absoluteX",double(m.width()*.25f-dialogWidth));n.set("absoluteY",double(std::max(0.f,(m.height()-dialogHeight)*.5f)));n.set("width",double(dialogWidth));n.set("height",double(dialogHeight));
                    }
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
                            m.node(el).set("positionDependency",double(children));m.node(el).set("relativeAnchorX",.5);m.node(el).set("orientationAnchorX",.5);
                            m.node(el).set("relativeAnchorY",0.0);m.node(el).set("orientationAnchorY",0.0);
                            m.node(el).set("width",64.0);m.node(el).set("height",64.0);
                            m.node(el).set("xOffset", 0.0);
                            for(auto part:{"bg","ItemImage","NewQuest","Touch"})if(auto child=m.find(el,part)){
                                auto& n=m.node(child);n.set("visible",true);n.set("positionDependency",double(el));n.set("relativeAnchorX",.5);n.set("relativeAnchorY",.5);n.set("orientationAnchorX",.5);n.set("orientationAnchorY",.5);
                                if(std::string(part)=="Touch"){n.set("width",64.0);n.set("height",64.0);n.set("enabled",true);}
                            }
                            m.node(el).set("yOffset", double(y));
                            m.node(el).set("priority", -24.0);
                            if(auto touch=m.find(el,"Touch"))m.node(touch).set("priority",-35.0);
                            if(auto bg=m.find(el,"bg")){m.node(bg).set("priority",-24.0);m.node(bg).set("width",56.0);m.node(bg).set("height",56.0);m.node(bg).set("useOffsets",false);}
                            if(auto icon=m.find(el,"ItemImage")){m.node(icon).set("priority",-26.0);m.node(icon).set("width",36.0);m.node(icon).set("height",36.0);m.node(icon).set("useOffsets",false);}
                            if(auto badge=m.find(el,"NewQuest")){m.node(badge).set("priority",-28.0);m.node(badge).set("width",20.0);m.node(badge).set("height",20.0);}
                            if(auto text=m.find(el,"ProgressText"))m.node(text).set("priority",-29.0);
                            m.node(el).set("layer", std::string("HUD"));
                            y += 68;
                        }
                        m.solve();
                        if(auto touch=m.find(dialog,"Touch")){
                            m.node(touch).set("width",double(std::min(dialogWidth,m.width()*.25f)));
                            m.node(touch).set("height",double(dialogHeight));
                            m.node(touch).set("absoluteX",0.0);m.node(touch).set("absoluteY",m.node(children).number("absoluteY"));m.node(touch).set("priority",10.0);
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
                if(name=="tabbed_menu_blueprints")blueprintListTop_=listTop;
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
                                scrollOrigins_[el] = (name=="tabbed_menu_blueprints"?0:12) + index * m.width() * .2;
                                auto& n=m.node(el);n.set("width",double(m.width()*.2f));n.set("height",double(m.height()-listTop));
                                n.set("positionDependency",double(list));n.set("relativeAnchorX",0.0);n.set("orientationAnchorX",0.0);n.set("relativeAnchorY",1.0);n.set("orientationAnchorY",1.0);
                                if(auto touch=m.find(el,"Touch")){auto& c=m.node(touch);c.set("width",n.number("width"));c.set("height",n.number("height"));c.set("priority",-80.0);c.set("layer",std::string("HUD"));}
                                if(auto touch=m.find(el,"HackTouch"))m.node(touch).set("enabled",false);
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
                            set(element, "Icon", "visible", true);
                            set(element, "Icon", "useOffsets", false);
                            set(element, "Amount", "text",
                                std::to_string(g.numOfMaterialPossessed(unsigned(id))));
                        }
                    } else if (name == "tabbed_menu_blueprints") {
                        const auto& b = g.blueprints.at(id);
                        std::string image=blueprintIcon(b.sheet,b.image);
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
                        set(element,"bg","spriteName",std::string("btn_yellow"));
                        set(element,"bg","sheetName",std::string("xml_resources/rp_menu.xml"));
                        set(element,"bg","visible",true);
                        set(element,"ItemImage","visible",true);
                        set(element,"Touch","enabled",true);
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
                        if(name=="tabbed_menu_free" && id<0){
                            set(element,"Icon","spriteName",std::string(id==-1?"rp_help":id==-2?(soundEnabled_?"rp_sound_on":"rp_sound_off"):"rp_store_logout"));
                            set(element,"Icon","sheetName",std::string("xml_resources/rp_menu.xml"));
                            set(element,"Icon","visible",true);
                            set(element,"Touch","enabled",true);
                            set(element,"itemName","text",strings_.getText(id==-1?"HELP":id==-2?"SOUND":"LOGOUT"));
                            set(element,"itemName","visible",true);
                            set(element,"itemCost","visible",false);
                            set(element,"currencyIcon","visible",false);
                            set(element,"itemImageSecondaryCentered","visible",false);
                            set(element,"selectionOverlay","visible",false);
                            continue;
                        }
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
                        // Resolve the price/icon pair as one centered group, not independent anchors.
                        if(auto cost=m.find(element,"itemCost")){
                            auto& n=m.node(cost);n.set("width",42.0);n.set("height",16.0);n.set("alignment",1.0);n.set("size",.65);
                            n.set("relativeAnchorX",.5);n.set("orientationAnchorX",1.0);n.set("relativeAnchorY",1.0);n.set("orientationAnchorY",1.0);n.set("xOffset",3.0);n.set("yOffset",-4.0);
                        }
                        if(auto icon=m.find(element,"currencyIcon")){
                            auto& n=m.node(icon);n.set("positionDependency",double(element));n.set("width",18.0);n.set("height",16.0);n.set("relativeAnchorX",.5);n.set("orientationAnchorX",0.0);n.set("relativeAnchorY",1.0);n.set("orientationAnchorY",1.0);n.set("xOffset",5.0);n.set("yOffset",-4.0);
                        }
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
                // Original quest_dialog.xml binds three optional illustration slots independently.
                auto bindImage = [&](const char* name, const std::string& sheet, const std::string& image) {
                    if(auto el=m.find(1,name,true)){
                        m.node(el).set("visible",!sheet.empty()&&!image.empty());
                        if(!sheet.empty()&&!image.empty()){
                            set(el,"Sprite","sheetName",sheet);
                            set(el,"Sprite","spriteName",image);
                            set(el,"Sprite","visible",true);
                        }
                    }
                };
                bindImage("ImageRight",q.sheet,q.image);
                bindImage("ImageLeft",q.sheetLeft,q.imageLeft);
                bindImage("ImageBottom",q.sheetBottom,q.imageBottom);
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
            if(name=="tabbed_menu_inventory"){
                // The original single row can extend beyond narrow logical viewports.
                // Fit the catalogue in rows within the material area; retain original sprites/font.
                const auto ids=menuItemIds(g,name);
                const size_t cols=std::min<size_t>(12,std::max<size_t>(1,ids.size()));
                const size_t rows=(ids.size()+cols-1)/cols;
                const float step=m.width()/float(cols);
                const float cellH=std::min(50.f,100.f/float(std::max<size_t>(1,rows)));
                if(auto bg=m.find(1,"bgInventory",true)){
                    auto& n=m.node(bg);n.set("absoluteX",0.0);n.set("absoluteY",double(m.height()-125));n.set("width",double(m.width()));n.set("height",125.0);
                    if(auto sprite=m.find(bg,"Sprite")){auto& c=m.node(sprite);c.set("absoluteX",0.0);c.set("absoluteY",double(m.height()-125));c.set("includeBorder",false);c.set("width",double(m.width()));c.set("height",125.0);}
                }
                for(size_t i=0;i<ids.size();++i){
                    auto el=m.find(1,"material_"+std::to_string(ids[i]),true);if(!el)continue;
                    const float x=(float(i%cols)+.5f)*step,y=m.height()-115+float(i/cols)*cellH;
                    auto place=[&](unsigned node,float px,float py,float w,float h){if(!node)return;auto& n=m.node(node);n.set("absoluteX",double(px));n.set("absoluteY",double(py));n.set("width",double(w));n.set("height",double(h));n.set("layer",std::string("HUD"));n.set("priority",-30.0);};
                    place(el,x-step*.5f,y,step,cellH);
                    if(auto icon=m.find(el,"Icon")){
                        auto bb=m.node(icon).bounds;
                        const float scale=std::max(0.01f,std::min((step-6)/std::max(1.f,bb.w),std::max(1.f,cellH-17)/std::max(1.f,bb.h)));
                        const float w=bb.w*scale,h=bb.h*scale;
                        place(icon,x-w*.5f,y,w,h);
                        m.node(icon).set("alpha",g.numOfMaterialPossessed(unsigned(ids[i]))?1.0:.5);
                    }
                    if(auto amount=m.find(el,"Amount")){place(amount,x-step*.5f,y+cellH-16,step,16);m.node(amount).set("alignment",1.0);m.node(amount).set("size",.6);}
                }
                unsigned group=0;
                for(auto title:{"CommonText","UnCommonText","RareText"}){
                    if(auto el=m.find(1,title,true))if(auto text=m.find(el,"Text")){
                        auto& n=m.node(text);n.set("absoluteX",double(group*m.width()/3));n.set("absoluteY",double(m.height()-17));n.set("width",double(m.width()/3));n.set("height",16.0);n.set("alignment",1.0);n.set("size",.6);n.set("priority",-50.0);n.set("clipText",true);
                    }
                    ++group;
                }
                show("Frame",false);m.solve();
            }
            if (name.rfind("tabbed_menu_", 0) == 0 && name != "tabbed_menu_inventory") {
                for (auto& [key, item] : xmlItems_) {
                    auto element = m.find(1, key, true);
                    if (!element)
                        continue;
                    auto image=m.find(element,name=="tabbed_menu_free"&&item<0?"Icon":"itemImage");
                    if (image) {
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
            auto expected=menuItemIds(g,name);
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
                if(auto fill=xmlMenu_->find(icon,"SpriteBarFG"))xmlMenu_->node(fill).set("barFraction",g.mapActionMax>0?std::clamp((17.0+109.0*double(g.mapActions)/g.mapActionMax)/141.0,0.0,1.0):0.0);
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
        if(name=="quest_log"){
            // Existing entries must update when quest_progress changes without a
            // catalogue-size change. Do not recreate the menu or reset scroll.
            for(const auto& [key,id]:xmlItems_){
                auto quest=g.quests.find(id);
                if(quest==g.quests.end())continue;
                auto entry=xmlMenu_->find(1,key,true);
                if(!entry)continue;
                if(auto text=xmlMenu_->find(entry,"ProgressText")){
                    auto& n=xmlMenu_->node(text);
                    n.set("text",std::to_string(quest->second.progress)+"%");
                    n.set("visible",quest->second.progress>0&&quest->second.progress<100);
                }
                if(auto badge=xmlMenu_->find(entry,"NewQuest"))xmlMenu_->node(badge).set("visible",quest->second.unread);
            }
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
        bool local=xmlName_=="tabbed_menu_free"&&shopSelection_<0&&shopSelection_>=-3;
        bool valid = local || (shopSelection_ > 0 && (blueprint ? g.blueprints.count(shopSelection_)
                                                      : g.storeItems.count(shopSelection_)));
        m.node(frame).set("visible", valid);
        if(valid&&(blueprint||local)){
            auto& n=m.node(frame);n.set("absoluteX",105.0);n.set("absoluteY",42.0);n.set("width",430.0);n.set("height",190.0);
            if(auto bg=m.find(frame,"bg")){auto& c=m.node(bg);c.set("absoluteX",105.0);c.set("absoluteY",42.0);c.set("width",430.0);c.set("height",190.0);c.set("includeBorder",false);}
        }
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
        if(local){
            const bool sound=shopSelection_==-2;
            title=shopSelection_==-1?"HELP":sound?"SOUND":"LOGOUT";
            sheet="xml_resources/rp_menu.xml";
            image=shopSelection_==-1?"rp_help":sound?(soundEnabled_?"rp_sound_on":"rp_sound_off"):"rp_store_logout";
            prop(frame,"itemDesc","text",strings_.getText(shopSelection_==-1?"HELP_DESC":sound?"SOUND_DESC":"LOGOUT_DESC"));
        }else if (blueprint) {
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
        if(blueprint)image=blueprintIcon(sheet,image);
        prop(frame, "itemImage", "sheetName", sheet);
        prop(frame, "itemImage", "spriteName", image);
        prop(frame, "itemImage", "visible", !image.empty());
        if (btn) {
            bool build = blueprint && g.owned.count(shopSelection_);
            if(build){prop(btn,"UpSprite","spriteName",std::string("btn_long_green"));prop(btn,"DownSprite","spriteName",std::string("btn_long_green_over"));}
            else{prop(btn,"UpSprite","spriteName",std::string("btn_long_blue"));prop(btn,"DownSprite","spriteName",std::string("btn_long_blue_over"));}
            prop(btn, "Text", "text", strings_.getText(local?(shopSelection_==-2?(soundEnabled_?"SOUND_ON":"SOUND_OFF"):shopSelection_==-1?"VIEW_HELP":"VIEW_LOGOUT"):(build ? "BUILD" : "BUY_MENU_BUTTON_PREFIX")));
            prop(btn,"Touch","enabled",!local||shopSelection_==-2);
            if(local){
                prop(btn,"Text","text",std::string(shopSelection_==-2?(soundEnabled_?"SOUND ON":"SOUND OFF"):shopSelection_==-1?"HELP":"LOGOUT"));
                prop(btn,"Text","font",std::string("rp_font_title"));
                prop(btn,"Text","visible",true);
                prop(btn,"Text","size",.8);
            }
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
            prop(frame,"itemTitle","clipText",true);
            prop(frame,"itemTitle","alignment",1.0);prop(frame,"itemTitle","autoScale",true);
            if(auto icon=m.find(frame,"itemImage")){
                auto ib=m.node(icon).bounds;float factor=std::min(1.f,50.f/std::max(1.f,std::max(ib.w,ib.h)));
                place(icon,b.x+(b.w-ib.w*factor)*.5f,b.y+24,ib.w*factor,ib.h*factor);
            }
            auto count=std::min<size_t>(4,g.blueprints.at(shopSelection_).recipe.size());
            const float step=(b.w-20)/float(std::max<size_t>(1,count));
            for(unsigned i=0;i<count;++i)if(auto material=m.find(frame,"Material"+std::to_string(i+1),true)){
                const float x=b.x+10+step*i, y=b.y+b.h-91;
                place(material,x,y,step,51);
                if(auto icon=m.find(material,"ItemImage",true)){
                    const float side=38;
                    place(icon,x+(step-side)*.5f,y,side,side);
                    for(auto child:m.node(icon).children)if(m.node(child).type=="spriteSheetComponent"){
                        auto& n=m.node(child);n.set("useOffsets",false);n.set("scaleX",1.0);n.set("scaleY",1.0);n.set("size",1.0);
                        place(child,x+(step-side)*.5f,y,side,side);
                    }
                }
                if(auto req=m.find(material,"MaterialReqs",true)){
                    place(req,x,y+38,step,14);
                    if(auto label=m.find(req,"Text")){place(label,x,y+38,step,14);auto& n=m.node(label);n.set("alignment",1.0);n.set("multiline",false);n.set("autoScale",true);n.set("size",.55);}
                }
            }
            const auto bb=m.node(btn).bounds;
            const float bw=std::clamp(bb.w,100.f,190.f),bh=std::clamp(bb.h,24.f,38.f);
            place(btn,b.x+(b.w-bw)*.5f,b.y+b.h-bh-5,bw,bh);
            if(auto touch=m.find(btn,"Touch"))place(touch,b.x+(b.w-bw)*.5f,b.y+b.h-bh-5,bw,bh);
            if(auto text=m.find(btn,"Text")){auto& n=m.node(text);n.set("relativeAnchorX",.5);n.set("orientationAnchorX",.5);n.set("xOffset",0.0);n.set("yOffset",0.0);}
            m.solve();
        }else if(local&&btn){
            const auto b=m.node(frame).bounds;
            auto place=[&](unsigned id,float x,float y,float w,float h){if(!id)return;auto& n=m.node(id);n.set("absoluteX",double(x));n.set("absoluteY",double(y));n.set("width",double(w));n.set("height",double(h));n.set("priority",-10.0);};
            place(m.find(frame,"itemTitle"),b.x+15,b.y+10,b.w-30,24);
            place(m.find(frame,"itemDesc"),b.x+20,b.y+42,b.w-110,85);
            place(m.find(frame,"itemImage"),b.x+b.w-80,b.y+45,68,68);
            place(btn,b.x+(b.w-180)*.5f,b.y+b.h-42,180,36);
            if(auto touch=m.find(btn,"Touch"))place(touch,b.x+(b.w-180)*.5f,b.y+b.h-42,180,36);
            if(auto text=m.find(btn,"Text")){place(text,b.x+(b.w-180)*.5f,b.y+b.h-37,180,26);m.node(text).set("alignment",1.0);m.node(text).set("priority",-20.0);}
            if(auto sprite=m.find(btn,"UpSprite"))place(sprite,b.x+(b.w-180)*.5f,b.y+b.h-42,180,36);
            prop(frame,"itemDesc","clipText",true);
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
        if(xmlName_.rfind("tabbed_menu_",0)==0){
            if(auto button=m.find(1,"buyBtn",true);button&&m.isVisible(button)){
                if(auto visual=m.find(button,"UpSprite");visual&&m.node(visual).bounds.contains(x,y))
                    name="buyBtn";
            }
        }
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
        if(xmlName_=="tabbed_menu_free"&&name=="option_soundtoggle"){
            soundEnabled_=!soundEnabled_;
            if(auto icon=m.find(owner,"UpSprite"))m.node(icon).set("spriteName",std::string(soundEnabled_?"btn_green":"btn_red"));
            updateShopFrame(g);return {};
        }
        if(name=="buyBtn"&&xmlName_=="tabbed_menu_free"&&shopSelection_<0){
            if(shopSelection_==-2){
                soundEnabled_=!soundEnabled_;
                for(const auto& [key,id]:xmlItems_)if(id==-2)
                    if(auto el=m.find(1,key,true))if(auto sprite=m.find(el,"Icon"))
                        m.node(sprite).set("spriteName",std::string(soundEnabled_?"rp_sound_on":"rp_sound_off"));
                updateShopFrame(g);
            }
            return {};
        }
        if (name == "buyBtn" && shopSelection_ > 0 && xmlName_.rfind("tabbed_menu_", 0) == 0) {
            int item = shopSelection_;
            if (xmlName_ == "tabbed_menu_blueprints") {
                if (BuildModeState(g).gotMsgStartBuildRequest(item)) {
                    selectedBlueprint_ = item;
                    selectedComponent_ = selectedParent_ = 0;
                    placement_.reset();
                    placementParent_=0;
                    const auto& def=g.objects.at(g.blueprints.at(item).object);
                    if(def.type==0){
                        auto edges=g.GetAvailableEdgeGridSpace();
                        if(!edges.empty())placement_=edges.front();
                    }else{
                        for(const auto& raft:g.rafts){
                            for(int cy=0;cy<6&&!placement_;++cy)for(int cx=0;cx<6&&!placement_;++cx)
                                if(g.canPlaceComponent(def.id,raft.id,cx,cy)){placement_=Cell{cx,cy};placementParent_=raft.id;}
                            if(placement_)break;
                        }
                    }
                    grid = true;
                    map = false;
                    panel_ = Panel::None;
                    xmlMenu_.reset();
                    return {};
                }
                if(g.owned.count(item)){
                    g.status="Cannot build: missing materials, invalid object or construction pending";
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
                requestedStoreItem_="sonar";
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
                if(shopSelection_!=item){shopSelection_=item;updateShopFrame(g);}
            }
            return {};
        }
        return {};
    }

    std::optional<sfs::Message> PortView::panelTouch(Game& g, float x, float y) {
        if(xmlMenu_&&xmlName_=="tabbed_menu_blueprints"&&y>=blueprintListTop_&&y<400){selectBlueprintSlot(g,x);return {};}
        if (!xmlMenu_ || xmlName_ == "piratemap_menu") {
            Image dummy(640, 400);
            renderPanel(dummy, g);
        }
        return xmlTouch(g, x, y);
    }
} // namespace rp
