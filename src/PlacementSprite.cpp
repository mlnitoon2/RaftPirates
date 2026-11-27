#include "rp/PlacementSprite.h"
#include <cmath>
#include <limits>
namespace rp {
    PlacementSprite::PlacementSprite(const Game& g,const OceanView& v):game_(g),view_(v){}
    PlacementSprite::~PlacementSprite()=default;
    bool PlacementSprite::OkayToPlace()const{return false;}
    bool PlacementSprite::Select(const sys::gfx::Gfx*){return false;}
    std::optional<Cell> PlacementSprite::GetClosestPosition(sys::gfx::Vec2 world,const std::vector<Cell>& candidates){
        if(!std::isfinite(world.x)||!std::isfinite(world.y))return {};
        std::optional<Cell> best;
        double distance=std::numeric_limits<double>::infinity();
        for(auto p:candidates){
            double x=double(p.x)+.5-world.x,y=double(p.y)+.5-world.y;
            double d=x*x+y*y;
            if(d<distance){best=p;distance=d;}
        }
        return best;
    }
    void PlacementSprite::SetPosition(Cell p){position_=p;Update();}
    void PlacementSprite::SetToObject(int object,int parent,int ignoreParent,int ignoreId){
        object_=object;parent_=parent;ignoreParent_=ignoreParent;ignoreId_=ignoreId;Update();
    }
    void PlacementSprite::SetPriorities(){}
    void PlacementSprite::Update(){}
}
