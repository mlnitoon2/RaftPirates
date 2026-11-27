#pragma once
#include "rp/PlacementSprite.h"

namespace rp {
    // Live construction preview using the authoritative Game placement checks.
    class ItemPlacementSprite : public PlacementSprite {
      public:
        ItemPlacementSprite(const Game& game,const OceanView& view,
                            std::shared_ptr<const sys::gfx::Image> image,
                            const sys::res::SpriteCell& valid,const sys::res::SpriteCell& invalid);
        ~ItemPlacementSprite() override;
        void SetPriorities() override;
        virtual void SetPriority(unsigned piece,float value);
        bool OkayToPlace()const override;
        void Update() override;
        void Render(sys::gfx::Image& target)const;
      private:
        std::shared_ptr<const sys::gfx::Image> image_;
        sys::res::SpriteCell valid_,invalid_;
        std::unique_ptr<FootprintSprite> footprint_;
    };
}
