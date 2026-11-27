#pragma once
#include "rp/AccessorySprite.h"
#include "sys/gfx/GfxText.h"

namespace rp {
    class BattlePointsViewSprite final : public AccessorySprite {
      public:
        BattlePointsViewSprite(const OceanView*, GridSprite*, std::string, int overridePoints);
        ~BattlePointsViewSprite() override;
        void SetColour(uint8_t, uint8_t, uint8_t, uint8_t);
        void tick(float dt) override;
        int submit(sys::gfx::GfxManager&, sys::gfx::AnimationRenderer&) const override;

        sys::gfx::GfxText& graphics() {
            return text_;
        }

        const std::string& text() const {
            return value_;
        }

      private:
        std::string prefix_, value_;
        int override_, last_ = -1;
        sys::gfx::GfxText text_;
    };
} // namespace rp
