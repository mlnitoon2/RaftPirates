#include "rp/BattleModeState.h"
#include "rp/Game.h"
#include <algorithm>
#include <cmath>
#include <stdexcept>

namespace rp {
    BattleModeState::BattleModeState(Game& g) : game_(g) {
        if (!g.localPlay)
            throw std::logic_error("Practice is offline only");
    }

    BattleModeState::~BattleModeState() = default;

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
        auto r = game_.GetRaftByID(raftId_);
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
        if (!active_ && game_.GetRaftByID(id))
            raftId_ = id;
    }

    bool BattleModeState::SendRaftSelection() {
        auto r = game_.GetRaftByID(raftId_);
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
        auto r = game_.GetRaftByID(raftId_);
        if (!r || r->Destroyed()) {
            lost_ = true;
            active_ = false;
            return false;
        }
        int attack = 0;
        for (const auto& c : r->components) {
            const auto* info = c.getResourceData(game_.objects);
            if (info && info->name != "DivingUnit" && !c.Destroyed())
                attack += std::min(1000, c.GetBattlePoints()) / 4;
        }
        if (!attack)
            return false;
        enemy_ = std::max(0, enemy_ - attack);
        StartWaiting(.65f);
        game_.status = "Practice attack";
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
        r->HandleBattleOutcome(&entries, game_.objects);
        if (r->Destroyed()) {
            lost_ = true;
            active_ = false;
        }
        return true;
    }

    int BattleModeState::endBattleAndPlunder() {
        if (!won_ || paid_)
            return 0;
        paid_ = true;
        game_.giveMaterial(4, 150);
        return 150;
    }

    void BattleModeState::gotMsgExitBattleModeRequest() {
        active_ = false;
        StopWaiting();
    }
} // namespace rp
