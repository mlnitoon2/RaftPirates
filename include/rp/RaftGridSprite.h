#pragma once
#include "rp/OverlayAccessory.h"

namespace rp {
    class RaftGridSprite : public OverlayAccessory {
      public:
        RaftGridSprite(const OceanView*, GridSprite*);
        ~RaftGridSprite() override;
        void Init() override;
        void SetPriorities() override;
        void SetPriority(unsigned, float) override;
        void Update() override;

      private:
    };
} // namespace rp
