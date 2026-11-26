#include "rp/BattleModeState.h"
#include "rp/Game.h"
#include <algorithm>
#include <cmath>
#include <stdexcept>
#include <limits>

namespace rp {
    BattleModeState::BattleModeState(Game& g) : game_(&g) {
        if (!g.localPlay)
            throw std::logic_error("Practice is offline only");
    }

    BattleModeState::~BattleModeState() = default;

    const char* BattleModeState::GetStateName() const {
        return "BattleModeState";
    }

    std::optional<sfs::Message> BattleModeState::gotMsgAttackRequest(int dice) {
        return attacker_ ? attack(dice) : defend(dice);
    }

    bool BattleModeState::active() const {
        return active_;
    }

    bool BattleModeState::won() const {
        return won_;
    }

    bool BattleModeState::lost() const {
        return lost_;
    }

    int BattleModeState::enemyBP() const {
        return enemy_;
    }

    int BattleModeState::enemyMaxBP() const {
        return maxEnemy_;
    }

    int BattleModeState::playerHullBP() const {
        auto r = game_ ? game_->GetRaftByID(raftId_) : nullptr;
        return r ? r->bp : 0;
    }

    float BattleModeState::remaining() const {
        return wait_;
    }

    void BattleModeState::ToPositioningView() {
        active_ = false;
        paused_ = false;
    }

    void BattleModeState::RaftSelect(int id) {
        if (game_ && !active_ && game_->GetRaftByID(id))
            raftId_ = id;
    }

    bool BattleModeState::SendRaftSelection() {
        auto r = game_ ? game_->GetRaftByID(raftId_) : nullptr;
        return r && !r->Destroyed() && r->bp > 0;
    }

    void BattleModeState::ToBattleView() {
        if (!won_ && !lost_ && SendRaftSelection())
            active_ = true;
    }

    void BattleModeState::StartWaiting(float seconds) {
        wait_ = std::max(0.f, seconds);
    }

    void BattleModeState::StopWaiting() {
        wait_ = 0;
    }

    void BattleModeState::Pause() {
        paused_ = !paused_;
    }

    void BattleModeState::tick(float dt) {
        if (active_ && !paused_ && std::isfinite(dt) && dt > 0)
            wait_ = std::max(0.f, wait_ - dt);
    }

    bool BattleModeState::gotMsgAttackRequest() {
        if (!active_ || paused_ || wait_ > 0 || won_ || lost_)
            return false;
        auto r = game_ ? game_->GetRaftByID(raftId_) : nullptr;
        if (!r || r->Destroyed()) {
            lost_ = true;
            active_ = false;
            return false;
        }
        int attack = 0;
        for (const auto& c : r->components) {
            const auto* info = c.getResourceData(game_->objects);
            if (info && info->name != "DivingUnit" && !c.Destroyed())
                attack += std::min(1000, c.GetBattlePoints()) / 4;
        }
        if (!attack)
            return false;
        enemy_ = std::max(0, enemy_ - attack);
        StartWaiting(.65f);
        game_->status = "Practice attack";
        if (!enemy_) {
            won_ = true;
            active_ = false;
            return true;
        }
        auto target = std::make_shared<sfs::SFSObjectWrapper>();
        target->put("user_raft_id", int32_t(r->id));
        target->put("current_bp", int32_t(std::max(0, r->bp - 90)));
        target->put("state", int32_t(r->bp <= 90 ? 2 : r->state));
        auto result = std::make_shared<sfs::SFSObjectWrapper>();
        result->put("target_raft", target);
        sfs::SFSArrayWrapper entries;
        entries.add(result);
        r->HandleBattleOutcome(&entries, game_->objects);
        if (r->Destroyed()) {
            lost_ = true;
            active_ = false;
        }
        return true;
    }

    int BattleModeState::endBattleAndPlunder() {
        if (!game_ || !won_ || paid_)
            return 0;
        paid_ = true;
        game_->giveMaterial(4, 150);
        return 150;
    }

    void BattleModeState::gotMsgExitBattleModeRequest() {
        active_ = false;
        StopWaiting();
    }

    using O = sfs::SFSObjectWrapper;

    void BattleModeState::reset() {
        *this = BattleModeState{};
    }

    void BattleModeState::sendFailed() {
        if (pending_ == "gs_battle_plunder")
            plunderSent_ = false;
        pending_.clear();
        message_ = "Request failed";
    }

    std::optional<sfs::Message> BattleModeState::action(const std::string& c, O p, bool withSync) {
        if (waiting())
            return {};
        if (withSync)
            p.put("sync_counter", int32_t(sync_));
        pending_ = c;
        return Game::request(c, p);
    }

    std::optional<sfs::Message> BattleModeState::encounterResponse(bool attack) {
        if (phase_ != Phase::Encounter)
            return {};
        decline_ = !attack;
        O p;
        p.put("attack", attack);
        return action("gs_encounter_response", p, false);
    }

    std::optional<sfs::Message> BattleModeState::gotMsgBattleScreenRaftsSelected(int x, int y) {
        if (phase_ != Phase::Positioning || !haveOpponent_ || !attacker_ || positioned_)
            return {};
        O p;
        p.put("attack_x", int32_t(x));
        p.put("attack_y", int32_t(y));
        return action("gs_battle_position", p);
    }

    std::optional<sfs::Message> BattleModeState::SendRaftSelection(int own, int enemy,
                                                                   const Player& player) {
        if (phase_ != Phase::Positioning || !haveOpponent_ || (attacker_ && !positioned_) ||
            waiting())
            return {};
        const auto* a = player.GetRaftByID(own);
        const auto* b = opponent_->GetRaftByID(enemy);
        if (!a || !b || a->Destroyed() || b->Destroyed())
            return {};
        ownRaft_ = own;
        enemyRaft_ = enemy;
        O p;
        p.put("attack_raft", int32_t(attacker_ ? own : enemy));
        p.put("defend_raft", int32_t(attacker_ ? enemy : own));
        p.put("ai_selection", int32_t(0));
        return action("gs_battle_select_raft", p);
    }

    std::optional<sfs::Message> BattleModeState::SendClientReady(bool confirmSwap) {
        if (phase_ != Phase::Fighting || !needReady_)
            return {};
        O p;
        p.put("confirm_swap_stance", confirmSwap || swap_);
        return action("gs_battle_client_ready", p);
    }

    std::optional<sfs::Message> BattleModeState::attack(int dice) {
        if (!readyForDice() || !attacker_ || dice <= 0 || dice > 8)
            return {};
        O p;
        p.put("attack_dice", int32_t(dice));
        return action("gs_battle_attack", p);
    }

    std::optional<sfs::Message> BattleModeState::defend(int dice) {
        if (!readyForDice() || attacker_ || dice <= 0 || dice > 8)
            return {};
        O p;
        p.put("defend_dice", int32_t(dice));
        return action("gs_battle_defend", p);
    }

    std::optional<sfs::Message> BattleModeState::retreat() {
        if (phase_ != Phase::Positioning && phase_ != Phase::Fighting && phase_ != Phase::Desynced)
            return {};
        return action("gs_battle_retreat");
    }

    std::optional<sfs::Message> BattleModeState::plunder() {
        if (phase_ != Phase::Over || plunderSent_ || waiting())
            return {};
        auto result = action("gs_battle_plunder", {}, false);
        if (result)
            plunderSent_ = true;
        return result;
    }

    bool BattleModeState::HandleExtensionResponse(const std::string& c, const O& p,
                                                  Player& player) {
        if (c == "gs_encounter") {
            if (phase_ == Phase::Idle) {
                reset();
                phase_ = Phase::Encounter;
            }
            return true;
        }
        if (c == "gs_start_battle") {
            reset();
            attacker_ = p.getBool("attacker", false);
            phase_ = Phase::Starting;
            return true;
        }
        if (c == "gs_error") {
            if (phase_ == Phase::Idle)
                return false;
            message_ = p.getString("message");
            if (pending_ == "gs_battle_plunder")
                plunderSent_ = false;
            pending_.clear();
            return true;
        }
        if (phase_ == Phase::Idle)
            return false;
        if (c == "gs_opponent") {
            if (phase_ != Phase::Starting && phase_ != Phase::Positioning)
                return true;
            auto data = p.getSFSObj("player_object");
            if (!data) {
                message_ = "Missing opponent data";
                return true;
            }
            Player next;
            next.objects = player.objects;
            next.initWithSFSObject(*data);
            next.playerName = p.getString("opponent_name", next.playerName);
            opponent_ = std::make_shared<Player>(std::move(next));
            ocean_.init(0);
            ocean_.addPlayer(1, opponent_, 0, 0);
            haveOpponent_ = true;
            boss_ = p.getBool("is_boss", false);
            phase_ = Phase::Positioning;
            pending_.clear();
            return true;
        }
        if (c == "combat_warning") {
            message_ = "Combat time remaining: " +
                       std::to_string(std::max(0, p.getInt("time_remaining", 0)));
            return true;
        }
        if (c == "gs_battle_opponent_time") {
            opponentTime_ = std::max(0, p.getInt("opponent_time", 0));
            return true;
        }
        if (c == "gs_battle_retreat" || c == "gs_encounter_response") {
            if (c == "gs_encounter_response" && phase_ == Phase::Encounter) {
                if (decline_)
                    reset();
                else {
                    phase_ = Phase::Starting;
                    pending_.clear();
                }
                return true;
            }
            if (c == "gs_battle_retreat") {
                reset();
                return true;
            }
        }
        if (c == "gs_battle_over") {
            if (phase_ != Phase::Over) {
                phase_ = Phase::Over;
                lastResult_ = p;
                pending_.clear();
            }
            return true;
        }
        if (c == "gs_battle_plunder") {
            if (phase_ == Phase::Over)
                reset();
            return true;
        }
        if (c != "gs_battle_position" && c != "gs_battle_select_raft" && c != "gs_battle_attack" &&
            c != "gs_battle_defend" && c != "gs_battle_client_ready" && c != "gs_swap_stance")
            return false;
        if (phase_ == Phase::Over || phase_ == Phase::Desynced)
            return true;
        if (p.get("sync_counter") && p.getInt("sync_counter", -1) != sync_) {
            int received = p.getInt("sync_counter", -1);
            if (received >= 0 && received < sync_)
                return true;
            phase_ = Phase::Desynced;
            pending_.clear();
            message_ = "Battle out of sync";
            return true;
        }
        if (c == "gs_battle_position") {
            if (phase_ != Phase::Starting && phase_ != Phase::Positioning)
                return true;
            if (!p.getBool("attack_position_okay", false)) {
                message_ = "Position rejected";
                phase_ = Phase::Positioning;
            } else {
                if (sync_ == std::numeric_limits<int>::max()) {
                    phase_ = Phase::Desynced;
                    return true;
                }
                ++sync_;
                positioned_ = true;
                phase_ = Phase::Positioning;
                if (haveOpponent_)
                    ocean_.movePlayer(1, p.getInt("attack_x", 0), p.getInt("attack_y", 0));
            }
        } else if (c == "gs_battle_select_raft") {
            if (phase_ != Phase::Positioning)
                return true;
            if (sync_ == std::numeric_limits<int>::max()) {
                phase_ = Phase::Desynced;
                return true;
            }
            if (p.get("attack_raft") && p.get("defend_raft")) {
                ownRaft_ = p.getInt(attacker_ ? "attack_raft" : "defend_raft");
                enemyRaft_ = p.getInt(attacker_ ? "defend_raft" : "attack_raft");
            }
            if (!player.GetRaftByID(ownRaft_) || !opponent_->GetRaftByID(enemyRaft_)) {
                phase_ = Phase::Desynced;
                pending_.clear();
                message_ = "Invalid selected rafts";
                return true;
            }
            ++sync_;
            phase_ = Phase::Fighting;
            needReady_ = true;
        } else if (c == "gs_battle_attack" || c == "gs_battle_defend") {
            if (phase_ != Phase::Fighting || !p.get("sync_counter"))
                return true;
            auto results = p.getSFSArray("results");
            if (!results)
                return true; // Acknowledgement without dice results.
            auto* own = player.GetRaftByID(ownRaft_);
            auto* enemy = opponent_->GetRaftByID(enemyRaft_);
            if (!own || !enemy || own->id == enemy->id ||
                sync_ == std::numeric_limits<int>::max()) {
                phase_ = Phase::Desynced;
                pending_.clear();
                message_ = "Ambiguous battle result ownership";
                return true;
            }
            own->HandleBattleOutcome(results.get(), player.objects);
            enemy->HandleBattleOutcome(results.get(), opponent_->objects);
            lastResult_ = p;
            ++sync_;
            needReady_ = true;
            if (own->Destroyed() || enemy->Destroyed()) {
                phase_ = Phase::Positioning;
                needReady_ = false;
            }
        } else if (c == "gs_battle_client_ready") {
            if (pending_ != "gs_battle_client_ready")
                return true;
            needReady_ = false;
            swap_ = false;
        } else if (c == "gs_swap_stance") {
            if (p.get("sync_counter")) {
                attacker_ = p.getBool("attacker", !attacker_);
                swap_ = true;
                if (sync_ == std::numeric_limits<int>::max()) {
                    phase_ = Phase::Desynced;
                    return true;
                }
                ++sync_;
                needReady_ = true;
            }
        }
        if (pending_ == c || c == "gs_battle_attack" || c == "gs_battle_defend" ||
            c == "gs_swap_stance")
            pending_.clear();
        return true;
    }
} // namespace rp
