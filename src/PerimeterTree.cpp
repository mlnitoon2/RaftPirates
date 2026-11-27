#include "rp/PerimeterTree.h"
#include <set>
#include <cstdint>
#include <stdexcept>
namespace rp {
    PerimeterTree::PerimeterTree(const PerimeterTree&)=default;
    PerimeterTree::~PerimeterTree()=default;
    void PerimeterTree::GeneratePerimeterTree(PerimeterTree& out,const GridObject* object,const GridObjectInfo* def){
        if(!object)throw std::invalid_argument("Missing grid object");
        std::set<std::pair<int64_t,int64_t>> cells;
        for(auto c:object->occupiedCells(def))cells.emplace(c.x,c.y);
        std::vector<Edge> next;
        for(auto p:cells)for(auto d:{Cell{1,0},Cell{-1,0},Cell{0,1},Cell{0,-1}})
            if(!cells.count({p.first+d.x,p.second+d.y}))
                next.push_back({{int(p.first),int(p.second)},d});
        out.edges_.swap(next);
    }
}
