#pragma once
#include "rp/RaftComponent.h"
#include <map>
#include <vector>

namespace rp {
    struct GridObject;

    struct Raft {
        int id = 0, type = 0, x = 0, y = 0, bp = 0, state = 0;
        std::vector<Component> components;
        int bonusBattlePoints = 0;
        void initWithSFSObject(const sfs::SFSObjectWrapper& data);
        int GetBattlePoints() const;
        int GetTotalBattlePoints(const std::map<int, GridObject>& objects) const;
        float GetHealth(const std::map<int, GridObject>& objects) const;

        bool Destroyed() const;
        int GridDimensionsX() const;
        int GridDimensionsY() const;
        void SetState(int value);
    };

} // namespace rp
