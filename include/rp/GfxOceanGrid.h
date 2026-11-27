#pragma once
#include "sys/gfx/Gfx.h"

namespace rp {
    class OceanView;

    class GfxOceanGrid {
      public:
        GfxOceanGrid(const OceanView* view, float unit);
        ~GfxOceanGrid();
        float FloatMod(float value, float modulus) const;
        void SetPosition(sys::gfx::Vec2 position);
        void SetUnitSize(float size);
        void Render(sys::gfx::Image& dst) const;

      private:
        const OceanView* view_;
        sys::gfx::Vec2 position_{};
        float unit_;
    };
} // namespace rp
