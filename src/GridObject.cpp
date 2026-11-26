#include "rp/Player.h"
#include <algorithm>
#include <cstdint>

namespace rp {
    GridObject::GridObject(int user, int object, int px, int py, int points, int objectState)
        : id(user), type(object), x(px), y(py), bp(points), state(objectState) {}

    GridObject::~GridObject() = default;

    Cell GridObject::GetGridPosition() const {
        return {x, y};
    }

    int GridObject::GetGridObjectID() const {
        return type;
    }

    int GridObject::GetUserObjectID() const {
        return id;
    }

    int GridObject::GetBattlePoints() const {
        return bp;
    }

    void GridObject::SetState(int value) {
        state = value;
    }

    bool GridObject::Destroyed() const {
        return state == 2;
    }

    void GridObject::SetBattlePoints(int value, int maxPoints) {
        bp = std::clamp(value, 0, std::max(0, maxPoints));
    }

    void GridObject::Repair(int value, int maxPoints) {
        SetBattlePoints(value, maxPoints);
    }

    int GridObject::GetTotalBattlePoints(int maxPoints) const {
        return maxPoints;
    }

    float GridObject::GetHealth(int maxPoints) const {
        return maxPoints > 0 ? float(bp) / maxPoints : 0.f;
    }

    const GridObjectInfo*
    GridObject::getResourceData(const std::map<int, GridObjectInfo>& catalogue) const {
        auto i = catalogue.find(type);
        return i == catalogue.end() ? nullptr : &i->second;
    }

    std::string GridObject::getName(const std::map<int, GridObjectInfo>& catalogue) const {
        auto p = getResourceData(catalogue);
        return p ? p->name : std::string{};
    }

    int GridObject::getType(const std::map<int, GridObjectInfo>& catalogue) const {
        auto p = getResourceData(catalogue);
        return p ? p->type : -1;
    }

    bool GridObject::Contains(Cell pos, const GridObjectInfo* definition) const {
        if (!definition)
            return pos.x == x && pos.y == y;
        for (auto c : definition->cells)
            if (int64_t(x) + c.x == pos.x && int64_t(y) + c.y == pos.y)
                return true;
        return false;
    }
} // namespace rp
