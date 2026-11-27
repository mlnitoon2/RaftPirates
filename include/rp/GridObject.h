#pragma once
#include <map>
#include <string>
#include <vector>
#include <utility>
#include <memory>

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
        size_t Pieces(const GridObjectInfo* definition) const;
        std::vector<Cell> occupiedCells(const GridObjectInfo* definition) const;
        std::pair<float, float> Middleish(const GridObjectInfo* definition) const;
        bool Overlap(const GridObject& other, const GridObjectInfo* definition,
                     const GridObjectInfo* otherDefinition) const;
        bool Beside(const GridObject& other, const GridObjectInfo* definition,
                    const GridObjectInfo* otherDefinition) const;
        int GetGridObjectID() const;
        virtual int GetUserObjectID() const;
        virtual int GetBattlePoints() const;
        virtual void SetState(int value);
        virtual bool TempTick(float dt);
        bool Destroyed() const;
        void SetBattlePoints(int value, int maxPoints);
        void Repair(int value, int maxPoints);
        virtual int GetTotalBattlePoints(int maxPoints) const;
        float GetHealth(int maxPoints) const;
        const GridObjectInfo* getResourceData(const std::map<int, GridObjectInfo>& catalogue) const;
        std::string getName(const std::map<int, GridObjectInfo>& catalogue) const;
        int getType(const std::map<int, GridObjectInfo>& catalogue) const;
        bool Contains(Cell pos, const GridObjectInfo* definition) const;
        virtual std::vector<std::string>
        getAvailableActions(const GridObjectInfo* definition) const;
        virtual std::unique_ptr<GridObject> Duplicate() const;
        std::pair<float, float> GetWorldPosition(Cell raftPosition, bool component) const;
    };
} // namespace rp
