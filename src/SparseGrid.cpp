#include "rp/SparseGrid.h"
#include <limits>
#include <cmath>

namespace rp {
    namespace {
        bool translate(Cell a, Cell b, Cell& out) {
            int64_t x = int64_t(a.x) + b.x, y = int64_t(a.y) + b.y;
            if (x < std::numeric_limits<int>::min() || x > std::numeric_limits<int>::max() ||
                y < std::numeric_limits<int>::min() || y > std::numeric_limits<int>::max())
                return false;
            out = {int(x), int(y)};
            return true;
        }
    } // namespace

    SparseGrid::SparseGrid() = default;
    SparseGrid::~SparseGrid() = default;

    void SparseGrid::AddedToOcean() { attached_ = true; }

    bool SparseGrid::IsAncestor(const GridObject* object) const {
        // This host grid has only direct membership, not the native parent chain.
        return object && hasObject(object->GetUserObjectID());
    }

    void SparseGrid::AddToTempTick(GridObject* object) {
        if (object) tempTick_.insert(object);
    }

    void SparseGrid::RemoveFromTempTick(GridObject* object) {
        tempTick_.erase(object);
    }

    bool SparseGrid::TempTick(float dt) {
        if (!std::isfinite(dt) || dt < 0) return tempTick_.empty();
        const auto snapshot = tempTick_;
        for (auto* object : snapshot)
            if (tempTick_.count(object) && object->TempTick(dt)) tempTick_.erase(object);
        return tempTick_.empty();
    }

    uint64_t SparseGrid::GenerateKey(Cell p) {
        return (uint64_t(uint32_t(p.x)) << 32) | uint32_t(p.y);
    }

    int SparseGrid::getObjectAt(Cell p) const {
        auto i = occupied_.find(GenerateKey(p));
        return i == occupied_.end() ? 0 : i->second;
    }

    bool SparseGrid::hasObject(int id) const {
        return entries_.count(id) != 0;
    }

    const std::vector<Cell>* SparseGrid::getObjectByID(int id) const {
        auto i = entries_.find(id);
        return i == entries_.end() ? nullptr : &i->second.footprint;
    }

    bool SparseGrid::CanAddObject(const std::vector<Cell>& footprint, Cell origin) const {
        if (footprint.empty())
            return false;
        for (auto p : footprint) {
            Cell cell;
            if (!translate(p, origin, cell) || getObjectAt(cell))
                return false;
        }
        return true;
    }

    bool SparseGrid::addObject(int id, const std::vector<Cell>& footprint, Cell origin) {
        if (id <= 0 || hasObject(id) || !CanAddObject(footprint, origin))
            return false;
        entries_[id] = {footprint, origin};
        for (auto p : footprint) {
            Cell cell;
            translate(p, origin, cell);
            occupied_[GenerateKey(cell)] = id;
        }
        return true;
    }

    bool SparseGrid::removeObject(int id) {
        auto i = entries_.find(id);
        if (i == entries_.end())
            return false;
        for (auto p : i->second.footprint) {
            Cell cell;
            translate(p, i->second.origin, cell);
            occupied_.erase(GenerateKey(cell));
        }
        entries_.erase(i);
        return true;
    }

    bool SparseGrid::moveObject(int id, Cell origin) {
        auto i = entries_.find(id);
        if (i == entries_.end())
            return false;
        auto entry = i->second;
        for (auto p : entry.footprint) {
            Cell cell;
            if (!translate(p, origin, cell))
                return false;
            int other = getObjectAt(cell);
            if (other && other != id)
                return false;
        }
        removeObject(id);
        return addObject(id, entry.footprint, origin);
    }
} // namespace rp
