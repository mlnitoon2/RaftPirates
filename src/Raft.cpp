#include "rp/Game.h"
#include <algorithm>
#include <limits>

namespace rp {
    namespace {
        int bounded(long long value) {
            return static_cast<int>(std::max(
                0ll, std::min(value, static_cast<long long>(std::numeric_limits<int>::max()))));
        }

        int maximum(const std::map<int, GridObject>& objects, int type) {
            auto i = objects.find(type);
            return i == objects.end() ? 0 : i->second.bp;
        }
    } // namespace

    int Raft::GetBattlePoints() const {
        long long total = static_cast<long long>(bp) + bonusBattlePoints;
        for (const auto& c : components)
            total += c.GetBattlePoints();
        return bounded(total);
    }

    int Raft::GetTotalBattlePoints(const std::map<int, GridObject>& objects) const {
        long long total = maximum(objects, type);
        for (const auto& c : components)
            total += c.GetTotalBattlePoints(maximum(objects, c.type));
        return bounded(total);
    }

    float Raft::GetHealth(const std::map<int, GridObject>& objects) const {
        const int total = GetTotalBattlePoints(objects);
        if (!total)
            return 0;
        long long current = bp;
        for (const auto& c : components)
            current += c.GetBattlePoints();
        return static_cast<float>(current) / total;
    }

    bool Raft::Destroyed() const {
        return state == 2;
    }

    int Raft::GridDimensionsX() const {
        return 6;
    }

    int Raft::GridDimensionsY() const {
        return 6;
    }

    void Raft::SetState(int value) {
        state = value;
    }
} // namespace rp
