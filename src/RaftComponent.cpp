#include "rp/RaftComponent.h"
#include "rp/SparseGrid.h"
#include <algorithm>
#include <cmath>
#include <limits>

namespace rp {
    RaftComponent::RaftComponent(int user, int object, int px, int py, int points, int objectState,
                                 int seconds)
        : GridObject(user, object, px, py, points, objectState) {
        SetRemainingTime(seconds);
    }

    bool RaftComponent::PostBuild() const {
        return state == 5;
    }

    bool RaftComponent::Building() const {
        return state == 1 || PostBuild();
    }

    float RaftComponent::GetRemainingTime() const {
        return static_cast<float>(std::max(0.0, remaining - remainingFraction));
    }

    void RaftComponent::SetRemainingTime(float seconds) {
        if (!std::isfinite(seconds))
            return;
        const double value = std::clamp(double(seconds), 0.0,
                                        double(std::numeric_limits<int>::max()));
        remaining = static_cast<int>(std::ceil(value));
        remainingFraction = remaining - value;
    }

    void RaftComponent::AddedToOcean(SparseGrid& grid) {
        GridObject::AddedToOcean(grid);
        if (Building())
            grid.AddToTempTick(this);
    }

    void RaftComponent::UseClock(float seconds) {
        if (!Building() || !std::isfinite(seconds) || seconds <= 0)
            return;
        TempTick(seconds);
    }

    void RaftComponent::SetState(int value) {
        // Native SetState promotes state 1 with zero remaining time to state 5.
        state = (value == 1 && state == 1 && GetRemainingTime() == 0) ? 5 : value;
    }

    void RaftComponent::Repair(int value, int maximum) {
        GridObject::Repair(value, maximum);
    }

    int RaftComponent::GetBattlePoints() const {
        return Building() ? 0 : bp;
    }

    int RaftComponent::GetTotalBattlePoints(int staticBattlePoints) const {
        return Building() ? 0 : staticBattlePoints;
    }

    bool RaftComponent::TempTick(float dt) {
        if (!std::isfinite(dt) || dt < 0)
            return state != 1 || remaining <= 0;
        if (remaining > 0) {
            const double elapsed = remainingFraction + dt;
            if (elapsed >= remaining) {
                SetRemainingTime(0);
            } else {
                const int seconds = static_cast<int>(elapsed);
                remaining -= seconds;
                remainingFraction = elapsed - seconds;
            }
        }
        return state != 1 || remaining <= 0;
    }

    std::vector<std::string>
    RaftComponent::getAvailableActions(const GridObjectInfo* definition) const {
        if (Building())
            return GetRemainingTime() > 0 ? std::vector<std::string>{"fast_clock"}
                                          : std::vector<std::string>{"wait"};
        return GridObject::getAvailableActions(definition);
    }

    std::unique_ptr<GridObject> RaftComponent::Duplicate() const {
        return std::make_unique<RaftComponent>(*this);
    }

} // namespace rp
