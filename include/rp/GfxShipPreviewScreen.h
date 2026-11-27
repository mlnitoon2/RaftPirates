#pragma once
#include "rp/Game.h"
#include "rp/RaftInfo.h"
#include <list>

namespace rp {
    class GfxShipPreviewScreen {
      public:
        GfxShipPreviewScreen(const std::string& assets, std::list<RaftInfo> rafts);
        ~GfxShipPreviewScreen();
        void aTickMethodThatsNotCalledTick(float dt);
        sys::gfx::Image render(const std::map<int, GridObjectInfo>& catalogue);

        OceanView& view() {
            return view_;
        }

      private:
        OceanView view_;
        Player player_;
    };
} // namespace rp
