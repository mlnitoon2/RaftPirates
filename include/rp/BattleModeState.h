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
        ~BattleModeState();
        const char* GetStateName() const;
        std::optional<sfs::Message> gotMsgAttackRequest(int dice);
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
    };
} // namespace rp
