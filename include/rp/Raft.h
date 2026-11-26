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
        int GridDimensionsX() const;
        int GridDimensionsY() const;
        void SetState(int value) override;
        void HandleBattleOutcome(const sfs::SFSArrayWrapper* outcomes,
                                 const std::map<int, GridObjectInfo>& objects);
        void Repair(int battlePoints, int maxBattlePoints);
    };

} // namespace rp
