#include "rp/RaftComponent.h"
#include <algorithm>
#include <cmath>

namespace rp {
    bool RaftComponent::PostBuild() const {
        return state == 5;
    }

    bool RaftComponent::Building() const {
        return state == 1 || PostBuild();
    }

    float RaftComponent::GetRemainingTime() const {
        return static_cast<float>(std::max(0.0, remaining - remainingFraction));
    }

    void RaftComponent::SetRemainingTime(int seconds) {
        remaining = std::max(0, seconds);
        remainingFraction = 0;
    }

    void RaftComponent::SetState(int value) {
        // Native SetState promotes state 1 with zero remaining time to state 5.
        state = (value == 1 && state == 1 && GetRemainingTime() == 0) ? 5 : value;
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
} // namespace rp
