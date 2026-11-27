#pragma once
#include "sys/gfx/Gfx.h"
namespace rp {
    class OceanView;
    class UnitHealthBarSprite {
      public:
        UnitHealthBarSprite(const OceanView& view,std::shared_ptr<const sys::gfx::Image> frame,
                            std::shared_ptr<const sys::gfx::Image> fill);
        ~UnitHealthBarSprite();
        void Init();
        void SetColour(uint8_t r,uint8_t g,uint8_t b,uint8_t a);
        void SetVisible(bool visible);
        void Update(sys::gfx::Vec2 world,float health);
        void tick(float dt);
        void Render(sys::gfx::Image&)const;
      private:
        const OceanView& view_;
        std::shared_ptr<const sys::gfx::Image> frame_,fill_;
        sys::gfx::Vec2 world_{};
        sys::gfx::Color colour_{};
        bool visible_=true;
        float health_=1;
    };
}
