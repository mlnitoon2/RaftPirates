#pragma once
#include "rp/AccessorySprite.h"
#include "sys/gfx/AnimationRenderer.h"

namespace rp {
    class RaftAttackSprite final : public AccessorySprite {
      public:
        RaftAttackSprite(const OceanView*, GridSprite*,
                         std::shared_ptr<const sys::res::ResourceAEAnim>, float delay = 0);
        ~RaftAttackSprite() override;
        void Init() override;
        void Update() override;
        void tick(float dt) override;

        bool finished() const {
            return finished_;
        }

        int submit(sys::gfx::GfxManager&, sys::gfx::AnimationRenderer&) const;

      private:
        std::shared_ptr<const sys::res::ResourceAEAnim> resource_;
        std::unique_ptr<sys::gfx::AEAnim> front_, back_;
        float delay_ = 0;
        bool finished_ = false;
    };
} // namespace rp
