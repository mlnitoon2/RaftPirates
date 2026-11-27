#pragma once
#include "rp/RaftComponent.h"
#include <map>
#include <vector>

namespace rp {
    struct GridObjectInfo;

    struct Raft : GridObject {
        Raft() = default;
        Raft(int user, int object, int px, int py, int points, int objectState,
             std::vector<Component> parts);
        std::vector<Component> components;
        int bonusBattlePoints = 0;
        void initWithSFSObject(const sfs::SFSObjectWrapper& data);
        int GetBattlePoints() const override;
        int GetTotalBattlePoints(const std::map<int, GridObjectInfo>& objects) const;
        float GetHealth(const std::map<int, GridObjectInfo>& objects) const;

        bool Destroyed() const;
        bool Salvage() const;
        int GridDimensionsX() const;
        int GridDimensionsY() const;
        void SetState(int value) override;
        void HandleBattleOutcome(const sfs::SFSArrayWrapper* outcomes,
                                 const std::map<int, GridObjectInfo>& objects);
        void Repair(int battlePoints, int maxBattlePoints);
        std::vector<std::string>
        getAvailableActions(const GridObjectInfo* definition) const override;
        std::unique_ptr<GridObject> Duplicate() const override;
        int raftSizeX() const;
        int raftSizeY() const;
        bool CanAddObject(const GridObjectInfo& definition, Cell pos,
                          const std::map<int, GridObjectInfo>& catalogue, int ignoreId = 0) const;
    };

} // namespace rp
