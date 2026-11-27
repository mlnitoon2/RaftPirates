#pragma once
#include <map>
#include <string>
#include <vector>
#include <utility>
#include <memory>
#include <functional>

namespace rp {
    struct Cell {
        int x = 0, y = 0;
    };
    struct GridObjectInfo;
    class GridObjectDataRes;
    class SparseGrid;
    struct GridObjectPiece;

    // User-owned object instance, distinct from the static GridObjectInfo library.
    // Explicit catalogue lookup avoids dangling pointers after a library refresh.
    struct GridObject {
        // Snapshot of one occupied shape, independent of the catalogue lifetime.
        struct GridShapePiece {
            GridShapePiece(const GridObject& object,const GridObjectPiece& piece,
                           unsigned index,Cell parent={},bool component=false,bool transpose=false);
            const std::string& Filename()const;
            Cell GetGridSpace(unsigned index)const;
            std::pair<float,float> GetWorldSpace(unsigned index)const;
            std::pair<float,float> TextureOffset()const;
            bool precedes(const GridShapePiece& other)const;
            unsigned index()const{return index_;}
          private:
            std::string filename_;
            std::vector<Cell> cells_;
            std::pair<float,float> base_;
            int offsetX_=0,offsetY_=0;
            unsigned index_=0;
            bool component_=false;
        };
        GridObject() = default;
        GridObject(int user, int object, int px, int py, int points, int objectState);
        virtual ~GridObject();
        int id = 0, type = 0, x = 0, y = 0, bp = 0, state = 0;
        Cell GetGridPosition() const;
        virtual size_t Pieces(const GridObjectInfo* definition) const;
        virtual void AddedToOcean(SparseGrid& grid);
        bool Salvage(SparseGrid& grid);
        bool setGridObjectData(const std::string& name, const std::map<int, GridObjectInfo>& catalogue);
        // Optional observer for native object-added notification; supplied by the host.
        void setAddedObserver(std::function<void(const GridObject&)> observer) { addedObserver_ = std::move(observer); }
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
      private:
        std::shared_ptr<const GridObjectDataRes> resource_;
        std::function<void(const GridObject&)> addedObserver_;
    };
} // namespace rp
