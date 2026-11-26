#pragma once
#include "rp/Player.h"
#include "rp/Ocean.h"
#include "sfs/SFSWire.h"
#include <optional>

namespace rp {
    class Game;

    // Original battle state name; online actions remain server-authoritative.
    class BattleModeState {
      public:
        BattleModeState() = default;
        explicit BattleModeState(Game& game);
        ~BattleModeState();
        const char* GetStateName() const;
        std::optional<sfs::Message> gotMsgAttackRequest(int dice);
        bool active() const;
        bool won() const;
        bool lost() const;
        int enemyBP() const;
        int enemyMaxBP() const;
        int playerHullBP() const;
        float remaining() const;
        void ToPositioningView();
        void RaftSelect(int id);
        bool SendRaftSelection();
        void ToBattleView();
        void StartWaiting(float seconds);
        void StopWaiting();
        void Pause();
        void tick(float dt);
        bool gotMsgAttackRequest();
        int endBattleAndPlunder();
        void gotMsgExitBattleModeRequest();

        enum class Phase { Idle, Encounter, Starting, Positioning, Fighting, Over, Desynced };

        Phase phase() const {
            return phase_;
        }

        const Player& opponent() const {
            return *opponent_;
        }

        bool hasOpponent() const {
            return haveOpponent_;
        }

        const Ocean& ocean() const {
            return ocean_;
        }

        bool isBoss() const {
            return boss_;
        }

        int ownRaftId() const {
            return ownRaft_;
        }

        int enemyRaftId() const {
            return enemyRaft_;
        }

        bool positioned() const {
            return positioned_;
        }

        bool readyForDice() const {
            return phase_ == Phase::Fighting && !waiting() && !needReady_;
        }

        bool attacker() const {
            return attacker_;
        }

        int syncCounter() const {
            return sync_;
        }

        int opponentTime() const {
            return opponentTime_;
        }

        bool waiting() const {
            return !pending_.empty();
        }

        const std::string& message() const {
            return message_;
        }

        const sfs::SFSObjectWrapper& lastResult() const {
            return lastResult_;
        }

        std::optional<sfs::Message> encounterResponse(bool attack);
        std::optional<sfs::Message> gotMsgBattleScreenRaftsSelected(int x, int y);
        std::optional<sfs::Message> SendRaftSelection(int own, int enemy, const Player& player);
        std::optional<sfs::Message> SendClientReady(bool confirmSwap = false);
        std::optional<sfs::Message> attack(int dice);
        std::optional<sfs::Message> defend(int dice);
        std::optional<sfs::Message> retreat();
        std::optional<sfs::Message> plunder();
        bool HandleExtensionResponse(const std::string& command, const sfs::SFSObjectWrapper& data,
                                     Player& player);
        void reset();
        void sendFailed();

      private:
        Phase phase_ = Phase::Idle;
        std::shared_ptr<Player> opponent_ = std::make_shared<Player>();
        Ocean ocean_;
        bool attacker_ = false, haveOpponent_ = false, plunderSent_ = false, needReady_ = false,
             decline_ = false, boss_ = false, positioned_ = false, swap_ = false;
        int ownRaft_ = 0, enemyRaft_ = 0;
        int sync_ = 0, opponentTime_ = 0;
        std::string pending_, message_;
        sfs::SFSObjectWrapper lastResult_;
        std::optional<sfs::Message> action(const std::string& command,
                                           sfs::SFSObjectWrapper data = {}, bool withSync = true);

        Game* game_ = nullptr;
        int raftId_ = 0, enemy_ = 1800, maxEnemy_ = 1800;
        float wait_ = 0;
        bool active_ = false, won_ = false, lost_ = false, paid_ = false, paused_ = false;
    };
} // namespace rp
