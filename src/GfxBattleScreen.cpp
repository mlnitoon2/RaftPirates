#include "rp/GfxBattleScreen.h"
#include "sys/gfx/AssetImage.h"
#include <algorithm>
#include <cmath>

namespace rp {
    using namespace sys::gfx;
    using Phase = BattleModeState::Phase;

    GfxBattleScreen::GfxBattleScreen(std::filesystem::path root)
        : root_(root), ownView_(root), enemyView_(root), effects_(root) {
        strings_.loadFile(root / "text/en.utf8");
        sheet_ = ResourceSpriteSheet::Create(root / "xml_resources/rp_menu.xml");
        atlas_ = std::make_shared<Image>(loadAssetImage(root / sheet_.imagePath));
        font_ = ResourceSpriteFont::Create(root, "rp_font_title");
        try {
            explosion_ = sys::res::ResourceAEAnim::Create(root / "xml_bin/anim_explosion.bin",
                                                          sys::res::AEFormat::RaftPirates2013);
        } catch (const std::exception&) {
        }
    }

    GfxBattleScreen::~GfxBattleScreen() = default;

    void GfxBattleScreen::tick(float dt) {
        if (!std::isfinite(dt) || dt < 0)
            return;
        transition_ = std::clamp(transition_ + (exiting_ ? -1.f : 1.f) * dt * 4, 0.f, 1.f);
        arrow_.tick(dt);
        for(auto it=explosions_.begin();it!=explosions_.end();){
            it->second.tick(dt);
            if(!it->second.active())it=explosions_.erase(it);else ++it;
        }
        for (auto& hit : damage_)
            hit.tick(dt);
        damage_.erase(std::remove_if(damage_.begin(), damage_.end(),
                                     [](const auto& hit) { return hit.remaining <= 0; }),
                      damage_.end());
    }

    void GfxBattleScreen::SwoopIn() {
        transition_ = 0;
        exiting_ = false;
    }

    void GfxBattleScreen::SwoopOut() {
        exiting_ = true;
    }

    void GfxBattleScreen::updateBattleDamage(const Game& game, float time) {
        tick(lastTime_ >= 0 ? std::clamp(time - lastTime_, 0.f, .25f) : 0);
        lastTime_ = time;
        auto update = [&](const Player& player, bool own, auto& previous) {
            std::map<int, int> next;
            for (const auto& raft : player.rafts) {
                int bp = raft.GetBattlePoints();
                next[raft.id] = bp;
                auto it = previous.find(raft.id);
                if (it != previous.end() && bp < it->second){
                    damage_.emplace_back(own,raft.id,it->second-bp);
                    explosions_.insert_or_assign({own,raft.id},ExplosionFXSprite(true));
                }
            }
            previous = std::move(next);
        };
        update(game, true, ownBP_);
        if (game.onlineBattle.hasOpponent())
            update(game.onlineBattle.opponent(), false, enemyBP_);
    }

    void GfxBattleScreen::prepare(const Game& game) {
        const auto& net = game.onlineBattle;
        if (lastPhase_ == Phase::Idle || lastAttacker_ != net.attacker() ||
            net.phase() == Phase::Starting) {
            placement_.Reset(net.attacker());
            positionChosen_ = false;
            dice_ = 1;
            retreatConfirm_ = false;
        }
        if (lastPhase_ != net.phase()) {
            retreatConfirm_ = false;
            if (lastPhase_ == Phase::Idle || net.phase() == Phase::Starting) {
                ownBP_.clear();
                enemyBP_.clear();
                damage_.clear();
                explosions_.clear();
                SwoopIn();
            }
            if (net.phase() == Phase::Over)
                SwoopOut();
        }
        lastPhase_ = net.phase();
        lastAttacker_ = net.attacker();
        if (net.hasOpponent())
            placement_.Update(game, net.opponent());
    }

    void GfxBattleScreen::submitSprite(GfxManager& gfx, const std::string& name, float x, float y,
                                       float w, float h) const {
        auto c = sheet_.findSpriteByName(name);
        if (!c || !c->ow || !c->oh || w <= 0 || h <= 0)
            return;
        GfxSprite s;
        s.image = atlas_;
        s.cell = *c;
        s.SetPos(x, y);
        s.SetScale(w / c->ow, h / c->oh);
        s.SetPriority(1);
        gfx.submit(s);
    }

    void GfxBattleScreen::submitText(GfxManager& gfx, const std::string& value, float cx, float y,
                                     float scale, float width) const {
        auto layout = font_.layout(value, scale, width, false, true, 0);
        font_.submit(gfx, layout, cx - layout.width / 2, y, {255, 244, 194, 255}, 5);
    }

    void GfxBattleScreen::renderFleet(Image& out, OceanView& view, const Player& player, int left,
                                      int selected, float time) const {
        if (player.rafts.empty())
            return;
        float minX = 1e9f, maxX = -1e9f, minY = 1e9f, maxY = -1e9f;
        for (const auto& r : player.rafts) {
            float x = float(r.x) - float(r.y), y = float(r.x) + float(r.y);
            minX = std::min(minX, x);
            maxX = std::max(maxX, x);
            minY = std::min(minY, y);
            maxY = std::max(maxY, y);
        }
        float zoom = std::clamp(
            std::min(290.f / (192 * (maxX - minX + 2)), 225.f / (96 * (maxY - minY + 3))), .015f,
            .72f);
        view.SetZoom(zoom);
        view.SetOffset({(minX + maxX + minY + maxY) / 4, (minY + maxY - minX - maxX) / 4}, true);
        view.SetPosition({320 + (left == 0 ? -100.f : 100.f) * (1 - transition_), 170});
        auto frame = view.render(player, time, false);
        for (int y = 60; y < 310; ++y)
            for (int x = 0; x < 320; ++x)
                out.at(left + x, y) = frame.at(160 + x, y);
        if (auto raft = player.GetRaftByID(selected)) {
            auto p = view.WorldToScreen({float(raft->x) + .5f, float(raft->y) + .5f});
            GfxManager gfx;
            submitSprite(gfx, "rp_battle_arrow", left + p.x - 160 - 15, p.y + arrow_.yOffset()-34, 30, 34);
            gfx.RenderScreen(out);
        }
    }

    int GfxBattleScreen::pick(const Player& player, const OceanView& view, float x, float y,
                              int left) const {
        auto world = view.ScreenToWorld({x - left + 160, y});
        auto r = player.GetRaftAtPosition({int(std::floor(world.x)), int(std::floor(world.y))});
        return r && !r->Destroyed() && r->bp > 0 ? r->id : 0;
    }

    void GfxBattleScreen::meter(GfxManager& gfx, const Player& player, int x, bool own) const {
        long long max = 0;
        for (const auto& raft : player.rafts)
            max += raft.GetTotalBattlePoints(player.objects);
        auto hp = player.currentBP();
        float f = max > 0 ? std::clamp(float(hp) / float(max), 0.f, 1.f) : 0.f;
        submitSprite(gfx, "battle_healthbar_base", x, 30, 250, 25);
        auto c = sheet_.findSpriteByName(own ? "battle_healthbar_green" : "battle_healthbar_red");
        if (c && c->ow && c->oh && f > 0) {
            GfxSprite s;
            s.image = atlas_;
            s.cell = *c;
            s.SetPos(x, 30);
            s.SetScale(250.f / c->ow, 25.f / c->oh);
            s.SetMask(0, 0, c->ow * f, c->oh, true);
            s.SetPriority(2);
            gfx.submit(s);
        }
        submitText(gfx, player.playerName, x + 125, 3, .65f, 250);
        submitText(gfx, std::to_string(hp) + " BP", x + 125, 32, .55f, 240);
    }

    Image GfxBattleScreen::Render(const Game& game, float time) {
        prepare(game);
        updateBattleDamage(game, time);
        const auto& net = game.onlineBattle;
        ownView_.SetBossOcean(net.isBoss());
        enemyView_.SetBossOcean(net.isBoss());
        Image out(640, 400, {36, 133, 168, 255});
        auto ids = placement_.gotMsgRequestBattlePlacementState();
        bool moving = net.phase() == Phase::Positioning && net.attacker() && !net.positioned() &&
                      net.hasOpponent();
        if (moving) {
            Player combined = net.opponent();
            combined.objects.insert(game.objects.begin(), game.objects.end());
            auto offset = placement_.GetWorldPosition(0);
            for (auto raft : game.rafts) {
                raft.x += offset.x;
                raft.y += offset.y;
                combined.rafts.push_back(std::move(raft));
            }
            auto anchor = net.opponent().mainRaft();
            if (!anchor && !net.opponent().rafts.empty())
                anchor = &net.opponent().rafts.front();
            ownView_.SetZoom(.27f);
            ownView_.SetPosition({320, 160});
            ownView_.SetOffset(anchor ? Vec2{float(anchor->x), float(anchor->y)} : Vec2{}, true);
            auto scene = ownView_.render(combined, time, true);
            for (int y = 60; y < 310; ++y)
                for (int x = 0; x < 640; ++x)
                    out.at(x, y) = scene.at(x, y);
            GfxManager marker;
            for (const auto& raft : game.rafts) {
                auto p = ownView_.WorldToScreen(
                    {float(raft.x + offset.x) + .5f, float(raft.y + offset.y) + .5f});
                submitSprite(marker, "rp_battle_arrow", p.x - 15, p.y + arrow_.yOffset()-34, 30, 34);
            }
            marker.RenderScreen(out);
        } else {
            renderFleet(out, ownView_, game, 0,
                        net.phase() == Phase::Fighting ? net.ownRaftId() : ids.first, time);
            if (net.hasOpponent())
                renderFleet(out, enemyView_, net.opponent(), 320,
                            net.phase() == Phase::Fighting ? net.enemyRaftId() : ids.second, time);
        }
        GfxManager gfx;
        meter(gfx, game, 12, true);
        if (net.hasOpponent())
            meter(gfx, net.opponent(), 378, false);
        if (!moving)
            submitSprite(gfx, "battle_divider", 312, 58, 16, 251);
        submitSprite(gfx, "battle_vs", 295, 9, 50, 50);
        if (net.isBoss())
            submitSprite(gfx, "battle_boss_token", 596, 61, 34, 34);
        if (!moving && net.hasOpponent())
            for (const auto& hit : damage_) {
                const auto& player = hit.own ? static_cast<const Player&>(game) : net.opponent();
                auto raft = player.GetRaftByID(hit.raft);
                if (!raft)
                    continue;
                auto& view = hit.own ? ownView_ : enemyView_;
                auto point = view.WorldToScreen({float(raft->x) + .5f, float(raft->y) + .5f});
                point.x += hit.own ? -160 : 160;
                auto fx=explosions_.find({hit.own,hit.raft});
                if (explosion_ && !explosion_->comps.empty() && fx!=explosions_.end()) {
                    const auto& comp = explosion_->comps.front();
                    float elapsed = fx->second.elapsed();
                    if(fx->second.Sparatic()&&comp.duration()>0)elapsed=std::fmod(elapsed,comp.duration());
                    if (comp.duration() > 0 && elapsed < comp.duration())
                        effects_.submit(gfx, *explosion_, comp, elapsed,
                                        Matrix::translation(point.x - comp.width * .35f,
                                                            point.y - comp.height * .7f) *
                                            Matrix::scale(.7f, .7f),
                                        3);
                }
                submitText(gfx, "-" + std::to_string(hit.amount), point.x,
                           point.y + hit.yOffset(), .85f, 130);
            }
        gfx.RenderScreen(out);
        loadMenu(game);
        if (menu_) {
            GfxManager ui;
            menu_->submit(ui);
            ui.RenderScreen(out);
        }
        return out;
    }

    void GfxBattleScreen::loadMenu(const Game& game) {
        using namespace sys::menu_redux;
        const auto& net = game.onlineBattle;
        std::string name = retreatConfirm_ || net.phase() == Phase::Desynced ? "retreat_confirm"
                           : net.phase() == Phase::Encounter   ? "attack_confirm_dialog"
                           : net.phase() == Phase::Positioning ? "battle_positioning_hud"
                           : net.phase() == Phase::Over        ? "battle_over_popup"
                                                               : "battle_hud";
        if (!menu_ || menuName_ != name) {
            menuName_ = name;
            menu_ = std::make_unique<MenuRuntime>(root_);
            menu_->nativeEvent = [this](auto& m, unsigned id, const std::string& e) {
                if (e == "onInit")
                    m.applyLiteralProperties(
                        id, [this](const auto& key) { return strings_.getText(key); });
                return true;
            };
            try {
                menu_->load(root_ / "menus" / (name + ".xml"));
                menu_->resize(640, 400);
                menu_->initialize();
            } catch (const std::exception&) {
                menu_.reset();
                return;
            }
        }
        auto& m = *menu_;
        auto show = [&](const std::string& n, bool on) {
            if (auto id = m.find(1, n, true))
                m.node(id).set("visible", on);
        };
        if (name == "battle_hud") {
            // Existing world meter rendering is retained until all original meter scripts are
            // bound.
            for (auto n : {"LeftBuildPoints", "RightBuildPoints", "CancelAutoAttack",
                           "AutoAttackButton", "BattleMapButton"})
                show(n, false);
            if (auto id = m.find(1, "Dice", true)) {
                auto& dice = m.node(id);
                if (auto touch = m.find(id, "Touch")) {
                    auto& t = m.node(touch);
                    t.set("width", double(std::max(1.f, dice.bounds.w)));
                    t.set("height", double(std::max(1.f, dice.bounds.h)));
                    t.set("enabled", net.readyForDice() || !net.waiting());
                }
            }
        } else if (name == "battle_positioning_hud") {
            if (auto id = m.find(1, "InstructionText", true)) {
                if (auto text = m.find(id, "Text"))
                    m.node(text).set(
                        "text",
                        strings_.getText(net.attacker() && !net.positioned()
                                             ? "BATTLE_INSTRUCTION_PLACEMENT"
                                         : placement_.gotMsgRequestBattlePlacementState().first
                                             ? "BATTLE_INSTRUCTION_CHOOSE_DEFEND"
                                             : "BATTLE_INSTRUCTION_CHOOSE_ATTACK"));
            }
        } else if (name == "attack_confirm_dialog" || name == "retreat_confirm") {
            // Original onPostInit sizes the popup using its text and No button.
            if (auto dialog = m.find(1, "Dialog", true))
                if (auto bg = m.find(dialog, "bg", true))
                    if (auto sprite = m.find(bg, "Sprite")) {
                        auto yes = m.find(dialog, "Yes", true),
                             text = m.find(dialog, "BattleText", true);
                        if (yes && text)
                            m.node(sprite).set("height",
                                               double(std::max(1.f, m.node(yes).bounds.h +
                                                                        m.node(text).bounds.h)));
                    }
        }
        m.solve();
    }

    std::optional<sfs::Message> GfxBattleScreen::Select(Game& game, float x, float y) {
        prepare(game);
        auto& net = game.onlineBattle;
        if (net.waiting())
            return {};
        loadMenu(game);
        if (menu_) {
            auto hit = menu_->hitTest(x, y);
            if (hit) {
                auto owner = menu_->owner(hit);
                auto name = menu_->node(owner).name;
                if (menuName_ == "attack_confirm_dialog" && (name == "Yes" || name == "No"))
                    return net.encounterResponse(name == "Yes");
                if (menuName_ == "retreat_confirm" && (name == "Yes" || name == "No")) {
                    retreatConfirm_ = false;
                    menu_.reset();
                    return name == "Yes" ? net.retreat() : std::optional<sfs::Message>{};
                }
                if (name == "RetreatButton") {
                    retreatConfirm_ = true;
                    menu_.reset();
                    return {};
                }
                if (menuName_ == "battle_over_popup" && name == "Continue")
                    return net.plunder();
                if (menuName_ == "battle_hud" && name == "Dice") {
                    auto b = menu_->node(hit).bounds;
                    dice_ = std::clamp(int((x - b.x) / std::max(1.f, b.w) * 8) + 1, 1, 8);
                    return net.readyForDice() ? net.gotMsgAttackRequest(dice_)
                                              : net.SendClientReady();
                }
                return {};
            }
        }
        if (net.phase() == Phase::Positioning && y >= 60 && y < 310) {
            if (net.attacker() && !net.positioned()) {
                auto p = ownView_.ScreenToWorld({x, y});
                auto anchor = game.mainRaft();
                if (!anchor && !game.rafts.empty())
                    anchor = &game.rafts.front();
                if (anchor) {
                    p.x -= anchor->x;
                    p.y -= anchor->y;
                }
                auto cell = placement_.GetClosestPosition(p);
                auto previous = placement_.GetWorldPosition(0);
                placement_.SetPosition(cell);
                if (positionChosen_ && cell.x == previous.x && cell.y == previous.y &&
                    placement_.OkayToPlace())
                    return net.gotMsgBattleScreenRaftsSelected(cell.x, cell.y);
                positionChosen_ = true;
            } else if (net.hasOpponent()) {
                bool own = x < 320;
                auto id = pick(own ? static_cast<const Player&>(game) : net.opponent(),
                               own ? ownView_ : enemyView_, x, y, own ? 0 : 320);
                if (id && placement_.Select(own, id)) {
                    auto ids = placement_.gotMsgRequestBattlePlacementState();
                    if (ids.first && ids.second)
                        return net.SendRaftSelection(ids.first, ids.second, game);
                }
            }
        }
        return {};
    }
} // namespace rp
