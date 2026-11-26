#include "rp/BuildModeState.h"
#include <algorithm>

namespace rp {
    BuildModeState::BuildModeState(Game& game) : game_(game) {}

    BuildModeState::~BuildModeState() = default;

    void BuildModeState::selectObject(int parent, int userObjectID) {
        parent_ = parent;
        id_ = userObjectID;
    }

    void BuildModeState::deselectObject() {
        parent_ = id_ = 0;
    }

    const GridObject* BuildModeState::selected() const {
        if (!id_)
            return nullptr;
        if (!parent_)
            return game_.GetRaftByID(id_);
        auto raft = game_.GetRaftByID(parent_);
        if (!raft)
            return nullptr;
        auto component = std::find_if(raft->components.begin(), raft->components.end(),
                                      [&](const auto& c) { return c.id == id_; });
        return component == raft->components.end() ? nullptr : &*component;
    }

    std::optional<sfs::Message> BuildModeState::repairSelected(bool all) const {
        auto object = selected();
        if (!game_.ready || !object)
            return {};
        auto info = object->getResourceData(game_.objects);
        if (!info || (!all && object->bp >= info->bp))
            return {};
        if (auto c = dynamic_cast<const RaftComponent*>(object); c && c->Building())
            return {};
        sfs::SFSObjectWrapper p;
        p.put("object_id", int32_t(id_));
        p.put("object_type", int32_t(parent_ ? 1 : 0));
        if (parent_)
            p.put("parent_id", int32_t(parent_));
        p.put("repair_all", int32_t(all));
        return Game::request("gs_build_repair", p);
    }

    std::optional<sfs::Message> BuildModeState::UseClockSelected() const {
        auto object = dynamic_cast<const RaftComponent*>(selected());
        if (!game_.ready || !object || !object->Building() || object->GetRemainingTime() <= 0)
            return {};
        sfs::SFSObjectWrapper p;
        p.put("object_id", int32_t(id_));
        p.put("object_type", int32_t(1));
        p.put("parent_id", int32_t(parent_));
        return Game::request("gs_use_clock", p);
    }
} // namespace rp
