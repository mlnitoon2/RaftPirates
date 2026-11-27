#pragma once
#include "rp/GridObject.h"

namespace rp {
    class PerimeterTree {
      public:
        struct Edge {
            Cell cell;
            Cell outward;
        };

        PerimeterTree() = default;
        PerimeterTree(const PerimeterTree&);
        ~PerimeterTree();
        static void GeneratePerimeterTree(PerimeterTree&, const GridObject*, const GridObjectInfo*);

        const std::vector<Edge>& edges() const {
            return edges_;
        }

      private:
        std::vector<Edge> edges_;
    };
} // namespace rp
