#pragma once
#include "sfs/SFSObjectWrapper.h"

namespace rp {
    struct RaftComponent {
        int id = 0, type = 0, x = 0, y = 0, bp = 0, state = 0, remaining = 0;
        double remainingFraction = 0;
        void initWithSFSObject(const sfs::SFSObjectWrapper& data);
        bool PostBuild() const;
        bool Building() const;
        float GetRemainingTime() const;
        void SetRemainingTime(int seconds);
        void SetState(int value);
        int GetBattlePoints() const;
        int GetTotalBattlePoints(int staticBattlePoints) const;
        // True when no further construction countdown is needed. Server owns state changes.
        bool TempTick(float dt);
    };

    using Component = RaftComponent;
} // namespace rp
