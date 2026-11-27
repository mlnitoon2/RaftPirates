#pragma once
#include "sys/gfx/AnimationRenderer.h"

namespace rp {
    class OceanView;

    class GfxOceanGradient {
      public:
        enum class GradientZone { Safe, Open, Boss };
        GfxOceanGradient(const OceanView* view, std::filesystem::path root);
        ~GfxOceanGradient();
        void SetZone(GradientZone zone);
        void InitWaves();
        void ComputeWorldState();
        void tick(float dt);
        void Render(sys::gfx::Image& dst);

      private:
        const OceanView* view_;
        std::filesystem::path root_;
        sys::gfx::AnimationRenderer renderer_;
        std::optional<sys::res::ResourceAEAnim> waves_;
        GradientZone zone_ = GradientZone::Safe;
        float time_ = 0;
        bool loaded_ = false;
    };
} // namespace rp
