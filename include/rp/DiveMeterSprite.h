#pragma once
#include "rp/OverlayAccessory.h"
#include "rp/DivingUnit.h"
#include "sys/gfx/ResourceSpriteFont.h"
#include <functional>

namespace rp {
    class DiveMeterSprite : public OverlayAccessory {
      public:
        DiveMeterSprite(const OceanView*, GridSprite*);
        ~DiveMeterSprite() override;
        bool Select(sys::gfx::Gfx*);
        void SetPriorities() override;
        void SetTime(int);
        void tick(float) override;
        void SetDive(const DivingUnit&);

        void setSelected(std::function<void()> callback) {
            selected_ = std::move(callback);
        }

        int submit(sys::gfx::GfxManager&, sys::gfx::AnimationRenderer&) const override;

      private:
        DivingUnit dive_;
        float phase_ = 0;
        std::shared_ptr<sys::gfx::ResourceSpriteFont> font_;
        std::function<void()> selected_;
        int frame_ = -1, fill_ = -1, button_ = -1;
    };
} // namespace rp
