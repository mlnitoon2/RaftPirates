#pragma once
#include "rp/GridObject.h"
#include <vector>
#include <cstdint>
#include <set>

namespace rp {
    // Cell occupancy adapter, not the original OverlapTree ABI. Owns footprints, not objects.
    class SparseGrid {
      public:
        SparseGrid();
        ~SparseGrid();
        bool CanAddObject(const std::vector<Cell>& footprint, Cell origin) const;
        bool addObject(int id, const std::vector<Cell>& footprint, Cell origin);
        bool removeObject(int id);
        bool moveObject(int id, Cell origin);
        bool hasObject(int id) const;
        int getObjectAt(Cell pos) const;
        const std::vector<Cell>* getObjectByID(int id) const;
        static uint64_t GenerateKey(Cell pos);
        // Non-owning registration; caller must unregister before destroying an object.
        void AddToTempTick(GridObject*);
        void RemoveFromTempTick(GridObject*);
        bool TempTick(float dt);
        bool IsAncestor(const GridObject* object) const;
        void AddedToOcean();

        bool attachedToOcean() const {
            return attached_;
        }

      private:
        struct Entry {
            std::vector<Cell> footprint;
            Cell origin;
        };

        std::map<int, Entry> entries_;
        std::map<uint64_t, int> occupied_;
        std::set<GridObject*> tempTick_;
        bool attached_ = false;
    };
} // namespace rp
