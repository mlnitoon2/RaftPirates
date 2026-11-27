#pragma once
#include "rp/AccessorySprite.h"
#include "sys/gfx/Gfx.h"

namespace rp {
    class MovementArrowSprite final : public AccessorySprite {
      public:
        MovementArrowSprite(const OceanView*, GridSprite*, int piece);
        ~MovementArrowSprite() override;
        void tick(float dt) override;

        sys::gfx::GfxSprite& graphics() {
            return arrow_;
        }

      private:
        int piece_;
        sys::gfx::GfxSprite arrow_;
    };
} // namespace rp
