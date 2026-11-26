#pragma once
#include "rp/Raft.h"

namespace rp {
    class Game;

    // Local practice encounter, not the original network battle protocol.
    class BattleModeState {
      public:
        explicit BattleModeState(Game& game);
        ~BattleModeState();
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

      private:
        Game& game_;
        int raftId_ = 0, enemy_ = 1800, maxEnemy_ = 1800;
        float wait_ = 0;
        bool active_ = false, won_ = false, lost_ = false, paid_ = false, paused_ = false;
    };
} // namespace rp
