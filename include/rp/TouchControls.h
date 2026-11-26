#pragma once
#include "rp/Game.h"

namespace rp {
    class TouchControls {
      public:
        enum class InteractionMode { Select, Pan };
        explicit TouchControls(OceanView* view);
        void SetZoomLimits(float low, float high, bool clampNow);
        void SetLockedView(bool locked);
        void LockView(bool locked);
        void LockZoom(bool locked);
        void PanTo(sys::gfx::Vec2 world, float duration, float zoom);
        void onZoom(float factor, sys::gfx::Vec2 screen);
        void Pick(sys::gfx::Vec2 point, InteractionMode mode);
        void gotMsgOceanViewPickResult(sys::gfx::Vec2 world);
        void touchDown(const sys::gfx::Vec2& point);
        void touchDrag(const sys::gfx::Vec2& point);
        void touchUp(const sys::gfx::Vec2& point);
        void touchCancel(const sys::gfx::Vec2& point);
        void tick(float dt);
        bool consumeDrag();

        std::optional<sys::gfx::Vec2> picked() const {
            return picked_;
        }

      private:
        OceanView* view_;
        float minZoom_ = .5f, maxZoom_ = 2.f, panTime_ = 0, panDuration_ = 0, panZoom_ = 1,
              startZoom_ = 1;
        bool locked_ = false, zoomLocked_ = false, pressed_ = false, dragged_ = false;
        sys::gfx::Vec2 start_{}, last_{}, from_{}, to_{};
        std::optional<sys::gfx::Vec2> picked_;
    };
} // namespace rp
