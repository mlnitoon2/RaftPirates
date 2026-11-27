#pragma once
#include "rp/FootprintSprite.h"

namespace rp {
    // Live construction preview using the authoritative Game placement checks.
    class ItemPlacementSprite {
      public:
        ItemPlacementSprite(const Game& game,const OceanView& view,
                            std::shared_ptr<const sys::gfx::Image> image,
                            const sys::res::SpriteCell& valid,const sys::res::SpriteCell& invalid);
        ~ItemPlacementSprite();
        void SetToObject(int object,int parent,int ignoreParent=0,int ignoreId=0);
        void SetPosition(Cell position);
        void SetPriority(unsigned piece,float value);
        bool OkayToPlace()const;
        void Update();
        void Render(sys::gfx::Image& target)const;
      private:
        const Game& game_;
        const OceanView& view_;
        std::shared_ptr<const sys::gfx::Image> image_;
        sys::res::SpriteCell valid_,invalid_;
        int object_=0,parent_=0,ignoreParent_=0,ignoreId_=0;
        Cell position_{};
        std::unique_ptr<FootprintSprite> footprint_;
    };
}
