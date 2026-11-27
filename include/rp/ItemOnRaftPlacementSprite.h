#pragma once
#include "rp/ItemPlacementSprite.h"
namespace rp {
    class ItemOnRaftPlacementSprite:public ItemPlacementSprite {
      public:
        ItemOnRaftPlacementSprite(const Game&,const OceanView&,std::shared_ptr<const sys::gfx::Image>,
                                  const sys::res::SpriteCell&,const sys::res::SpriteCell&);
        ~ItemOnRaftPlacementSprite() override;
        void SetPriority(unsigned piece,float value) override;
    };
}
