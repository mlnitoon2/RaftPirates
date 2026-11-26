#pragma once
#include "rp/GridObject.h"
#include <vector>
#include <cstdint>

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

      private:
        struct Entry {
            std::vector<Cell> footprint;
            Cell origin;
        };

        std::map<int, Entry> entries_;
        std::map<uint64_t, int> occupied_;
    };
} // namespace rp
