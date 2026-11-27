#include "rp/Game.h"
#include "rp/SparseGrid.h"
#include "rp/OverlapTree.h"
#include <algorithm>
#include <limits>

namespace rp {
    namespace {
        int bounded(long long value) {
            return static_cast<int>(std::max(
                0ll, std::min(value, static_cast<long long>(std::numeric_limits<int>::max()))));
        }

        int maximum(const std::map<int, GridObjectInfo>& objects, int type) {
            auto i = objects.find(type);
            return i == objects.end() ? 0 : i->second.bp;
        }
    } // namespace

    Raft::Raft(int user, int object, int px, int py, int points, int objectState,
               std::vector<Component> parts)
        : GridObject(user, object, px, py, points, objectState), components(std::move(parts)) {}

    int Raft::GetBattlePoints() const {
        long long total = static_cast<long long>(bp) + bonusBattlePoints;
        for (const auto& c : components)
            total += c.GetBattlePoints();
        return bounded(total);
    }

    int Raft::GetTotalBattlePoints(const std::map<int, GridObjectInfo>& objects) const {
        long long total = maximum(objects, type);
        for (const auto& c : components)
            total += c.GetTotalBattlePoints(maximum(objects, c.type));
        return bounded(total);
    }

    float Raft::GetHealth(const std::map<int, GridObjectInfo>& objects) const {
        const int total = GetTotalBattlePoints(objects);
        if (!total)
            return 0;
        long long current = bp;
        for (const auto& c : components)
            current += c.GetBattlePoints();
        return static_cast<float>(current) / total;
    }

    bool Raft::Salvage() const {return Destroyed()&&id>0;}

    bool Raft::Destroyed() const {
        return state == 2;
    }

    int Raft::GridDimensionsX() const {
        return 6;
    }

    int Raft::GridDimensionsY() const {
        return 6;
    }

    void Raft::Repair(int value, int maxBP) {
        GridObject::Repair(value, maxBP);
    }

    void Raft::HandleBattleOutcome(const sfs::SFSArrayWrapper* outcomes,
                                   const std::map<int, GridObjectInfo>& objects) {
        if (!outcomes)
            return;
        for (const auto& v : outcomes->values) {
            auto entry = std::get_if<std::shared_ptr<sfs::SFSObjectWrapper>>(&v);
            if (!entry || !*entry)
                continue;
            const auto& e = **entry;
            if (auto target = e.getSFSObj("target_raft");
                target && target->getInt("user_raft_id") == id) {
                if (target->get("current_bp"))
                    Repair(target->getInt("current_bp"), maximum(objects, type));
                if (target->get("state"))
                    SetState(target->getInt("state"));
            }
            if (auto target = e.getSFSObj("target_component")) {
                if (!target->get("user_raft") || target->getInt("user_raft") != id)
                    continue;
                int component = target->getInt("user_raft_component_id");
                for (auto& c : components)
                    if (c.id == component) {
                        if (target->get("current_bp"))
                            c.Repair(target->getInt("current_bp"), maximum(objects, c.type));
                        if (target->get("state"))
                            c.SetState(target->getInt("state"));
                    }
            }
        }
    }

    void Raft::SetState(int value) {
        state = value;
    }

    std::vector<std::string> Raft::getAvailableActions(const GridObjectInfo* definition) const {
        auto actions = GridObject::getAvailableActions(definition);
        actions.erase(std::remove(actions.begin(), actions.end(), "move"), actions.end());
        return actions;
    }

    std::unique_ptr<GridObject> Raft::Duplicate() const {
        return std::make_unique<Raft>(*this);
    }

    int Raft::raftSizeX() const {
        return GridDimensionsX();
    }

    int Raft::raftSizeY() const {
        return GridDimensionsY();
    }

    bool Raft::CanAddObject(const GridObjectInfo& def, Cell pos,
                            const std::map<int, GridObjectInfo>& catalogue, int ignoreId) const {
        if (Destroyed() || def.type != 1 || def.cells.empty())
            return false;
        GridObject candidate(0,def.id,pos.x,pos.y,0,0);
        OverlapTree footprint;
        try {OverlapTree::GenerateOverlapTree(footprint,&candidate,&def);}
        catch(const std::overflow_error&){return false;}
        if(!footprint.Within({0,0},{raftSizeX(),raftSizeY()}))return false;
        SparseGrid occupancy;
        int occupancyId = 1;
        for (const auto& component : components) {
            if (component.id == ignoreId)
                continue;
            auto info = component.getResourceData(catalogue);
            if (!occupancy.addObject(occupancyId++, info ? info->cells : std::vector<Cell>{{0, 0}},
                                     {component.x, component.y}))
                return false;
        }
        return occupancy.CanAddObject(def.cells, pos);
    }

} // namespace rp
