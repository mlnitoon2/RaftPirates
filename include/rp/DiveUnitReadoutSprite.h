#pragma once
#include "sys/gfx/Gfx.h"
#include <array>

namespace rp {
    class DiveUnitReadoutSprite {
      public:
        // Native default construction clears the graphics reference, flags and float slots.
        struct ReadoutLight {
            ReadoutLight();
            virtual ~ReadoutLight();
            std::shared_ptr<sys::gfx::Gfx> graphics;
            std::array<float, 4> values{};
            unsigned flags = 0;
            bool active = false;
        };
    };
}
