#pragma once
#include "rp/OverlayAccessory.h"
#include "rp/Messages.h"
#include <array>

namespace rp {
    class DiveUnitReadoutSprite : public OverlayAccessory {
      public:
        DiveUnitReadoutSprite(const OceanView*,GridSprite*);
        ~DiveUnitReadoutSprite() override;
        void SetPriorities() override;
        void SetVisible(bool) override;
        void gotMsgDiveDepthChange(const MsgDiveDepthChange&);
        void tick(float) override;
        // Native default construction clears the graphics reference, flags and float slots.
        struct ReadoutLight {
            ReadoutLight();
            virtual ~ReadoutLight();
            std::shared_ptr<sys::gfx::Gfx> graphics;
            std::array<float, 4> values{};
            unsigned flags = 0;
            bool active = false;
        };
      private:
        int depth_=-1, maximum_=-1;
        float elapsed_=0;
        void refresh();
    };
}
