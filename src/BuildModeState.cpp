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
        if (!game_.ready || !object || !object->Building() || object->GetRemainingTime() <= 0 ||
            !game_.hasMaterial(10, 1))
            return {};
        sfs::SFSObjectWrapper p;
        p.put("object_id", int32_t(id_));
        p.put("object_type", int32_t(1));
        p.put("parent_id", int32_t(parent_));
        return Game::request("gs_use_clock", p);
    }

    bool BuildModeState::isReadyForBattle() const {
        return game_.ready && !game_.safe && !game_.badZone && !game_.RaftCleanupRequired() &&
               game_.mainRaft() && !game_.mainRaft()->Destroyed() && waiting_.empty() &&
               !game_.pending && !game_.pendingMove;
    }

    void BuildModeState::checkFocusRaft() {
        if (!selected())
            deselectObject();
    }

    void BuildModeState::StartWaiting(const std::string& reason) {
        waiting_ = reason;
    }

    void BuildModeState::FinishWaiting(const std::string& reason) {
        if (waiting_ == reason)
            waiting_.clear();
    }

    void BuildModeState::gotMsgBuildCancel() {
        deselectObject();
        waiting_.clear();
    }

    void BuildModeState::gotMsgDeselectObject() {
        deselectObject();
    }

    std::optional<sfs::Message> BuildModeState::gotMsgDestroyObjectRequest() const {
        if (!game_.ready || !selected() || !waiting_.empty())
            return {};
        if (!parent_ && CheckForHoles(id_))
            return {};
        return game_.remove(parent_, id_);
    }

    std::optional<sfs::Message> BuildModeState::gotMsgBuildConfirm(int blueprint, int parent,
                                                                   Cell pos) {
        if (!waiting_.empty() || !gotMsgStartBuildRequest(blueprint))
            return {};
        auto b = game_.blueprints.find(blueprint);
        if (b == game_.blueprints.end() || !game_.owned.count(blueprint))
            return {};
        auto object = game_.objects.find(b->second.object);
        if (object == game_.objects.end())
            return {};
        if (parent ? !game_.canPlaceComponent(object->first, parent, pos.x, pos.y)
                   : !game_.canBuildRaft(pos))
            return {};
        return game_.build(object->first, parent, pos.x, pos.y);
    }

    std::optional<sfs::Message> BuildModeState::gotMsgMoveObjectRequest(int newParent, Cell pos) {
        if (!game_.ready || !parent_ || !selected() || game_.pendingMove || !waiting_.empty())
            return {};
        if (!game_.canPlaceComponent(selected()->type, newParent, pos.x, pos.y, parent_, id_))
            return {};
        return game_.moveComponent(parent_, id_, newParent, pos.x, pos.y);
    }

    std::vector<std::pair<int, int>> BuildModeState::wrecked(bool components) const {
        std::vector<std::pair<int, int>> out;
        for (const auto& r : game_.rafts) {
            if (components) {
                for (const auto& c : r.components)
                    if (c.Destroyed())
                        out.emplace_back(r.id, c.id);
            } else if (r.Destroyed())
                out.emplace_back(0, r.id);
        }
        return out;
    }

    int BuildModeState::gotMsgRequestNumWreckedRafts() const {
        return int(wrecked(false).size());
    }

    int BuildModeState::gotMsgRequestNumWreckedRaftComponents() const {
        return int(wrecked(true).size());
    }

    void BuildModeState::initWreckedRaftCheck() {
        wreckedRaft_ = 0;
        selectWreckedRaft();
    }

    void BuildModeState::initWreckedRaftComponentCheck() {
        wreckedComponent_ = 0;
        selectWreckedRaftComponent();
    }

    void BuildModeState::selectWreckedRaft() {
        auto entries = wrecked(false);
        if (entries.empty()) {
            deselectObject();
            return;
        }
        wreckedRaft_ %= entries.size();
        selectObject(0, entries[wreckedRaft_].second);
    }

    void BuildModeState::selectWreckedRaftComponent() {
        auto entries = wrecked(true);
        if (entries.empty()) {
            deselectObject();
            return;
        }
        wreckedComponent_ %= entries.size();
        auto p = entries[wreckedComponent_];
        selectObject(p.first, p.second);
    }

    void BuildModeState::gotMsgNextWreckedRaft() {
        ++wreckedRaft_;
        selectWreckedRaft();
    }

    void BuildModeState::gotMsgPreviousWreckedRaft() {
        auto n = wrecked(false).size();
        if (n)
            wreckedRaft_ = (wreckedRaft_ % n + n - 1) % n;
        selectWreckedRaft();
    }

    void BuildModeState::gotMsgNextWreckedRaftComponent() {
        ++wreckedComponent_;
        selectWreckedRaftComponent();
    }

    void BuildModeState::gotMsgPreviousWreckedRaftComponent() {
        auto n = wrecked(true).size();
        if (n)
            wreckedComponent_ = (wreckedComponent_ % n + n - 1) % n;
        selectWreckedRaftComponent();
    }

    std::vector<std::string> BuildModeState::SendMsgSelectedObjectAvailableActions() const {
        const auto* object = selected();
        if (!game_.ready || !object || !waiting_.empty() || game_.pending || game_.pendingMove)
            return {};
        auto actions = object->getAvailableActions(object->getResourceData(game_.objects));
        if (!parent_ && (game_.mainRaft() == object || CheckForHoles(id_))) {
            actions.erase(std::remove(actions.begin(), actions.end(), "destroy"), actions.end());
            actions.erase(std::remove(actions.begin(), actions.end(), "salvage"), actions.end());
        }
        return actions;
    }

    std::optional<sfs::Message> BuildModeState::gotMsgRepairSelectedObject() const {
        return repairSelected(false);
    }

    std::optional<sfs::Message> BuildModeState::gotMsgRepairSelectedObjectAll() const {
        return repairSelected(true);
    }

    std::optional<sfs::Message> BuildModeState::gotMsgUseClockOnSelectedObject() const {
        return UseClockSelected();
    }

    std::optional<sfs::Message> BuildModeState::gotMsgSalvageSelectedObject() const {
        return salvageSelected();
    }

    std::optional<sfs::Message> BuildModeState::salvageSelected() const {
        if (!selected() || !selected()->Destroyed())
            return {};
        return gotMsgDestroyObjectRequest();
    }

    void BuildModeState::gotMsgGridObjectSelected(int parent, int id) {
        selectObject(parent, id);
        checkFocusRaft();
    }

    bool BuildModeState::gotMsgStartBuildRequest(int blueprint) const {
        auto b = game_.blueprints.find(blueprint);
        if (!game_.ready || game_.pending || game_.pendingMove || b == game_.blueprints.end() ||
            !game_.owned.count(blueprint))
            return false;
        if (!game_.objects.count(b->second.object) || b->second.deprecated)
            return false;
        for (auto [id, count] : b->second.recipe)
            if (!game_.hasMaterial(id, count))
                return false;
        return true;
    }

    bool BuildModeState::CheckForHoles(int removedRaft) const {
        const auto* core = game_.mainRaft();
        if (!core || core->id == removedRaft)
            return true;
        std::set<int> seen{core->id};
        std::vector<int> queue{core->id};
        for (size_t i = 0; i < queue.size(); ++i) {
            const auto* current = game_.GetRaftByID(queue[i]);
            for (const auto& raft : game_.rafts)
                if (raft.id != removedRaft &&
                    std::abs(int64_t(current->x) - raft.x) +
                            std::abs(int64_t(current->y) - raft.y) ==
                        1 &&
                    seen.insert(raft.id).second)
                    queue.push_back(raft.id);
        }
        for (const auto& raft : game_.rafts)
            if (raft.id != removedRaft && !seen.count(raft.id))
                return true;
        return false;
    }

    const char* BuildModeState::GetStateName() const {
        return "BuildModeState";
    }

} // namespace rp
