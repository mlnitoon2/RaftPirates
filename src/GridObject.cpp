#include "rp/Player.h"
#include <algorithm>
#include <cstdint>
#include <stdexcept>
#include <cstdlib>

namespace rp {
    GridObject::GridObject(int user, int object, int px, int py, int points, int objectState)
        : id(user), type(object), x(px), y(py), bp(points), state(objectState) {}

    GridObject::~GridObject() = default;

    Cell GridObject::GetGridPosition() const {
        return {x, y};
    }

    size_t GridObject::Pieces(const GridObjectInfo* def) const {
        return def ? def->pieces.size() : 0;
    }

    std::vector<Cell> GridObject::occupiedCells(const GridObjectInfo* def) const {
        std::vector<Cell> result;
        if (!def || def->cells.empty())
            return {{x, y}};
        for (auto cell : def->cells) {
            auto px = int64_t(x) + cell.x, py = int64_t(y) + cell.y;
            if (px < INT32_MIN || px > INT32_MAX || py < INT32_MIN || py > INT32_MAX)
                throw std::overflow_error("Grid footprint outside integer range");
            result.push_back({int(px), int(py)});
        }
        return result;
    }

    std::pair<float, float> GridObject::Middleish(const GridObjectInfo* def) const {
        auto cells = occupiedCells(def);
        double sx = 0, sy = 0;
        for (auto c : cells) {
            sx += c.x;
            sy += c.y;
        }
        return {float(sx / cells.size()), float(sy / cells.size())};
    }

    bool GridObject::Overlap(const GridObject& other, const GridObjectInfo* def,
                             const GridObjectInfo* otherDef) const {
        for (auto c : occupiedCells(def))
            if (other.Contains(c, otherDef))
                return true;
        return false;
    }

    bool GridObject::Beside(const GridObject& other, const GridObjectInfo* def,
                            const GridObjectInfo* otherDef) const {
        if (Overlap(other, def, otherDef))
            return false;
        for (auto a : occupiedCells(def))
            for (auto b : other.occupiedCells(otherDef))
                if (std::abs(int64_t(a.x) - b.x) + std::abs(int64_t(a.y) - b.y) == 1)
                    return true;
        return false;
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

    bool GridObject::TempTick(float) { return true; } // ARM 0x3dbf28 returns 1.

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
        if (!definition || definition->cells.empty())
            return pos.x == x && pos.y == y;
        for (auto c : definition->cells)
            if (int64_t(x) + c.x == pos.x && int64_t(y) + c.y == pos.y)
                return true;
        return false;
    }

    std::vector<std::string>
    GridObject::getAvailableActions(const GridObjectInfo* definition) const {
        if (!definition)
            return {};
        if (Destroyed())
            return {"repair", "salvage"};
        std::vector<std::string> result;
        if (bp < definition->bp)
            result.push_back("repair");
        result.push_back("move");
        result.push_back("destroy");
        return result;
    }

    std::unique_ptr<GridObject> GridObject::Duplicate() const {
        return std::make_unique<GridObject>(*this);
    }

    std::pair<float, float> GridObject::GetWorldPosition(Cell raftPosition, bool component) const {
        return component ? std::make_pair(float(raftPosition.x) + x / 6.f,
                                          float(raftPosition.y) + y / 6.f)
                         : std::make_pair(float(x), float(y));
    }

} // namespace rp
