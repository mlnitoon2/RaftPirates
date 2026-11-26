#include "rp/GfxBattleScreen.h"
#include "sys/gfx/AssetImage.h"
#include <algorithm>
#include <cmath>

namespace rp {
    using namespace sys::gfx;
    using Phase = BattleModeState::Phase;

    GfxBattleScreen::GfxBattleScreen(std::filesystem::path root)
        : ownView_(root), enemyView_(root), effects_(root) {
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
        for (auto& hit : damage_)
            hit.remaining -= dt;
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
                if (it != previous.end() && bp < it->second)
                    damage_.push_back({own, raft.id, it->second - bp, 1.2f});
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
            dice_ = 1;
            retreatConfirm_ = false;
        }
        if (lastPhase_ != net.phase()) {
            retreatConfirm_ = false;
            if (lastPhase_ == Phase::Idle || net.phase() == Phase::Starting) {
                ownBP_.clear();
                enemyBP_.clear();
                damage_.clear();
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
            submitSprite(gfx, "rp_battle_arrow", left + p.x - 160 - 15, p.y - 66, 30, 34);
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
                submitSprite(marker, "rp_battle_arrow", p.x - 15, p.y - 66, 30, 34);
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
        auto button = [&](const char* image, const std::string& label, float x, float w) {
            submitSprite(gfx, image, x, 346, w, 48);
            submitText(gfx, label, x + w / 2, 358, .63f, w - 8);
        };
        std::string instruction;
        if (net.waiting())
            instruction = "Waiting for opponent...";
        else if (retreatConfirm_) {
            instruction = "Retreat from battle?";
            button("btn_long_red", "Retreat", 150, 145);
            button("btn_long_green", "Cancel", 345, 145);
        } else if (net.phase() == Phase::Encounter) {
            instruction = "Battle encounter";
            button("btn_long_green", "Attack", 40, 230);
            button("btn_long_red", "Decline", 370, 230);
        } else if (net.phase() == Phase::Positioning) {
            bool moving = net.attacker() && !net.positioned();
            instruction = moving ? "Tap water to position your fleet"
                                 : "Select attacking and defending rafts";
            if (moving) {
                auto p = placement_.GetWorldPosition(0);
                instruction += " (" + std::to_string(p.x) + ", " + std::to_string(p.y) + ")";
                if (!placement_.OkayToPlace())
                    instruction += " - overlaps or wreckage";
            }
            button("btn_long_green", moving ? "Position" : "Select", 245, 150);
            button("btn_red", "", 576, 58);
            submitSprite(gfx, "btn_retreat", 585, 354, 40, 32);
        } else if (net.phase() == Phase::Fighting) {
            instruction = net.readyForDice()
                              ? (net.attacker() ? "Choose attack dice" : "Choose defense dice")
                              : "Ready for next turn?";
            submitSprite(gfx, "battle_slider_left", 10, 353, 26, 34);
            submitSprite(gfx, "battle_slider_mid", 36, 353, 138, 34);
            submitSprite(gfx, "battle_slider_right", 174, 353, 26, 34);
            for (int n = 0; n < 8; ++n)
                submitSprite(
                    gfx, n < dice_ ? "dice_sm_green00" + std::to_string(n + 1) : "dice_sm_blank",
                    23 + n * 20, 359, 19, 19);
            button("btn_long_green",
                   net.readyForDice() ? (net.attacker() ? "Attack" : "Defend") : "Ready", 245, 150);
            button("btn_red", "", 576, 58);
            submitSprite(gfx, "btn_retreat", 585, 354, 40, 32);
        } else if (net.phase() == Phase::Over) {
            instruction = "Battle over";
            button("btn_long_green", "Continue", 245, 150);
        } else if (net.phase() == Phase::Desynced) {
            instruction = "Battle out of sync";
            button("btn_long_red", "Retreat", 245, 150);
        } else
            instruction = "Waiting for opponent...";
        if (!moving && net.hasOpponent())
            for (const auto& hit : damage_) {
                const auto& player = hit.own ? static_cast<const Player&>(game) : net.opponent();
                auto raft = player.GetRaftByID(hit.raft);
                if (!raft)
                    continue;
                auto& view = hit.own ? ownView_ : enemyView_;
                auto point = view.WorldToScreen({float(raft->x) + .5f, float(raft->y) + .5f});
                point.x += hit.own ? -160 : 160;
                if (explosion_ && !explosion_->comps.empty()) {
                    const auto& comp = explosion_->comps.front();
                    float elapsed = 1.2f - hit.remaining;
                    if (comp.duration() > 0 && elapsed < comp.duration())
                        effects_.submit(gfx, *explosion_, comp, elapsed,
                                        Matrix::translation(point.x - comp.width * .35f,
                                                            point.y - comp.height * .7f) *
                                            Matrix::scale(.7f, .7f),
                                        3);
                }
                submitText(gfx, "-" + std::to_string(hit.amount), point.x,
                           point.y - 25 - 35 * (1.2f - hit.remaining), .85f, 130);
            }
        if (!net.message().empty() && !net.waiting())
            submitText(gfx, net.message(), 320, 289, .55f, 610);
        submitText(gfx, instruction, 320, 318, .65f, 610);
        gfx.RenderScreen(out);
        return out;
    }

    std::optional<sfs::Message> GfxBattleScreen::Select(Game& game, float x, float y) {
        prepare(game);
        auto& net = game.onlineBattle;
        if (net.waiting())
            return {};
        if (retreatConfirm_) {
            if (y >= 346) {
                retreatConfirm_ = false;
                if (x >= 150 && x < 295)
                    return net.retreat();
            }
            return {};
        }
        if (net.phase() == Phase::Encounter && y >= 346) {
            if (x < 270)
                return net.encounterResponse(true);
            if (x >= 370)
                return net.encounterResponse(false);
        }
        if (net.phase() == Phase::Positioning) {
            if (y >= 346 && x >= 576) {
                retreatConfirm_ = true;
                return {};
            }
            if (y >= 346 && x >= 245 && x < 395) {
                if (net.attacker() && !net.positioned()) {
                    auto p = placement_.GetWorldPosition(0);
                    return net.gotMsgBattleScreenRaftsSelected(p.x, p.y);
                }
                auto ids = placement_.gotMsgRequestBattlePlacementState();
                return net.SendRaftSelection(ids.first, ids.second, game);
            }
            if (y >= 60 && y < 310) {
                if (net.attacker() && !net.positioned()) {
                    auto p = ownView_.ScreenToWorld({x, y});
                    auto anchor = game.mainRaft();
                    if (!anchor && !game.rafts.empty())
                        anchor = &game.rafts.front();
                    if (anchor) {
                        p.x -= anchor->x;
                        p.y -= anchor->y;
                    }
                    placement_.SetPosition(placement_.GetClosestPosition(p));
                } else if (net.hasOpponent()) {
                    bool own = x < 320;
                    auto id = pick(own ? static_cast<const Player&>(game) : net.opponent(),
                                   own ? ownView_ : enemyView_, x, y, own ? 0 : 320);
                    if (id)
                        placement_.Select(own, id);
                }
            }
        } else if (net.phase() == Phase::Fighting && y >= 346) {
            if (x >= 576) {
                retreatConfirm_ = true;
                return {};
            }
            if (x >= 10 && x < 200) {
                dice_ = std::clamp(int((x - 20) / 20) + 1, 1, 8);
                return {};
            }
            if (x >= 245 && x < 395)
                return net.readyForDice() ? net.gotMsgAttackRequest(dice_) : net.SendClientReady();
        } else if (net.phase() == Phase::Over && y >= 346 && x >= 245 && x < 395)
            return net.plunder();
        else if (net.phase() == Phase::Desynced && y >= 346 && x >= 245 && x < 395)
            return net.retreat();
        return {};
    }
} // namespace rp
