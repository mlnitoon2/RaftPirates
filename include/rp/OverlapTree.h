#pragma once
#include "rp/GridObject.h"

namespace rp {
    // A portable tree of occupied cells; rectangle uses [minimum,maximum).
    class OverlapTree {
      public:
        OverlapTree() = default;
        OverlapTree(const OverlapTree&);
        ~OverlapTree();
        static void GenerateOverlapTree(OverlapTree&, const GridObject*, const GridObjectInfo*);
        bool Within(Cell minimum, Cell maximum) const;

        const std::vector<Cell>& cells() const {
            return cells_;
        }

      private:
        std::vector<Cell> cells_;
    };
} // namespace rp
