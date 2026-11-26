#pragma once
#include <map>
#include <string>

namespace rp {
    struct Cell {
        int x = 0, y = 0;
    };
    struct GridObjectInfo;

    // User-owned object instance, distinct from the static GridObjectInfo library.
    // Explicit catalogue lookup avoids dangling pointers after a library refresh.
    struct GridObject {
        GridObject() = default;
        GridObject(int user, int object, int px, int py, int points, int objectState);
        virtual ~GridObject();
        int id = 0, type = 0, x = 0, y = 0, bp = 0, state = 0;
        Cell GetGridPosition() const;
        int GetGridObjectID() const;
        virtual int GetUserObjectID() const;
        virtual int GetBattlePoints() const;
        virtual void SetState(int value);
        bool Destroyed() const;
        void SetBattlePoints(int value, int maxPoints);
        void Repair(int value, int maxPoints);
        virtual int GetTotalBattlePoints(int maxPoints) const;
        float GetHealth(int maxPoints) const;
        const GridObjectInfo* getResourceData(const std::map<int, GridObjectInfo>& catalogue) const;
        std::string getName(const std::map<int, GridObjectInfo>& catalogue) const;
        int getType(const std::map<int, GridObjectInfo>& catalogue) const;
        bool Contains(Cell pos, const GridObjectInfo* definition) const;
    };
} // namespace rp
