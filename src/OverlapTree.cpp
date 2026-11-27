#include "rp/OverlapTree.h"
#include <algorithm>
#include <stdexcept>
namespace rp {
    OverlapTree::OverlapTree(const OverlapTree&)=default;
    OverlapTree::~OverlapTree()=default;
    void OverlapTree::GenerateOverlapTree(OverlapTree& out,const GridObject* object,const GridObjectInfo* def){
        if(!object)throw std::invalid_argument("Missing grid object");
        out.cells_=object->occupiedCells(def);
    }
    bool OverlapTree::Within(Cell lo,Cell hi)const{
        if(cells_.empty()||lo.x>=hi.x||lo.y>=hi.y)return false;
        return std::all_of(cells_.begin(),cells_.end(),[&](Cell p){
            return p.x>=lo.x&&p.y>=lo.y&&p.x<hi.x&&p.y<hi.y;
        });
    }
}
