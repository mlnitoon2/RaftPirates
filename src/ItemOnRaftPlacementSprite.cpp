#include "rp/ItemOnRaftPlacementSprite.h"
namespace rp {
    ItemOnRaftPlacementSprite::ItemOnRaftPlacementSprite(const Game& g,const OceanView& v,
        std::shared_ptr<const sys::gfx::Image> image,const sys::res::SpriteCell& ok,const sys::res::SpriteCell& bad)
        :ItemPlacementSprite(g,v,std::move(image),ok,bad){}
    ItemOnRaftPlacementSprite::~ItemOnRaftPlacementSprite()=default;
    void ItemOnRaftPlacementSprite::SetPriority(unsigned piece,float value){
        ItemPlacementSprite::SetPriority(piece,value);
    }
}
