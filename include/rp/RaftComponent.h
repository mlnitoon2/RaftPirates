#pragma once
#include "sfs/SFSObjectWrapper.h"
#include "rp/GridObject.h"

namespace rp {
    class SparseGrid;

    // Shared countdown fields of the native RaftComponent base. DivingUnit currently
    // stores a separate network view, so both views execute the same countdown code.
    struct FastClockCountdown {
        static constexpr float CLOCK_DELAY_TIME = .05f;
        static constexpr float CLOCK_FRAME_COUNT = 2.f / CLOCK_DELAY_TIME;
        static constexpr float MIN_TIME_FRACTION = 1.f;
        float step = 0, delay = 0, target = 0;
        void begin(float remaining, float targetRemaining);
        void tick(float& remaining, float dt);
        bool active() const { return delay > 0; }
    };

    struct RaftComponent : GridObject {
        RaftComponent() = default;
        RaftComponent(int user, int object, int px, int py, int points, int objectState,
                      int seconds);
        int remaining = 0;
        double remainingFraction = 0;
        FastClockCountdown clock;
        void initWithSFSObject(const sfs::SFSObjectWrapper& data);
        bool PostBuild() const;
        bool Building() const;
        float GetRemainingTime() const;
        void SetRemainingTime(float seconds);
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
