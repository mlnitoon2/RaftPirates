#pragma once
#include "sfs/SFSObjectWrapper.h"
#include <map>
#include <string>
#include <vector>

namespace rp {
    // Current diving-unit view, without claiming the original C++ inheritance/ABI.
    struct DivingUnit {
        int parent = 0, object = 0, state = 0, lootLevel = 0, depth = 0, maxDepth = 0, richZone = 0,
            totalSeconds = 0;
        float remaining = 0;
        int load = 0;
        std::map<int, int> bonus, collected;
        bool Diving() const;
        float GetDiveTime() const;
        int GetLoadValue() const;
        int GetMaxLoad() const;
        int numAvailableBonusMaterials() const;
        int numCollectedBonusMaterials() const;
        void SetDiveLevel(int current, int maximum);
        bool OkayToDive() const;
        void SetState(int value);
        void SetLoadValue(int value);
        void initWithSFSObject(const sfs::SFSObjectWrapper& data);
        bool TempTick(float dt);
        void StartDive(float seconds, float left);
        void FinishDive();
        std::vector<std::string> getAvailableActions() const;
    };

    using DiveState = DivingUnit;
} // namespace rp
