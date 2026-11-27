#pragma once
#include "sfs/SFSObjectWrapper.h"
#include "rp/GridObject.h"

namespace rp {
    class SparseGrid;
    struct RaftComponent : GridObject {
        RaftComponent() = default;
        RaftComponent(int user, int object, int px, int py, int points, int objectState,
                      int seconds);
        int remaining = 0;
        double remainingFraction = 0;
        void initWithSFSObject(const sfs::SFSObjectWrapper& data);
        bool PostBuild() const;
        bool Building() const;
        float GetRemainingTime() const;
        void SetRemainingTime(int seconds);
        void UseClock(float seconds);
        void AddedToOcean(SparseGrid& grid) override;
        void SetState(int value) override;
        int GetBattlePoints() const override;
        void Repair(int battlePoints, int maxBattlePoints);
        int GetTotalBattlePoints(int staticBattlePoints) const override;
        // True when no further construction countdown is needed. Server owns state changes.
        bool TempTick(float dt) override;
        std::vector<std::string>
        getAvailableActions(const GridObjectInfo* definition) const override;
        std::unique_ptr<GridObject> Duplicate() const override;
    };

    using Component = RaftComponent;
} // namespace rp
