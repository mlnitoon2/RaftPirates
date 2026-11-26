#include "rp/TouchControls.h"
#include <algorithm>
#include <cmath>
#include <stdexcept>

namespace rp {
    namespace {
        using sys::gfx::Vec2;

        bool valid(Vec2 p) {
            return std::isfinite(p.x) && std::isfinite(p.y);
        }
    } // namespace

    TouchControls::TouchControls(OceanView* view) : view_(view) {
        if (!view_)
            throw std::invalid_argument("Missing ocean view");
    }

    void TouchControls::SetZoomLimits(float low, float high, bool clampNow) {
        if (!std::isfinite(low) || !std::isfinite(high) || low <= 0 || high < low)
            throw std::invalid_argument("Invalid zoom limits");
        minZoom_ = low;
        maxZoom_ = high;
        if (clampNow)
            view_->SetZoom(std::clamp(view_->Zoom(), low, high));
    }

    void TouchControls::SetLockedView(bool locked) {
        LockView(locked);
        LockZoom(locked);
    }

    void TouchControls::LockView(bool locked) {
        locked_ = locked;
        if (locked) {
            pressed_ = false;
            panDuration_ = 0;
        }
    }

    void TouchControls::LockZoom(bool locked) {
        zoomLocked_ = locked;
    }

    void TouchControls::PanTo(Vec2 world, float duration, float zoom) {
        if (locked_ || !valid(world) || !std::isfinite(duration) || !std::isfinite(zoom))
            return;
        from_ = view_->Offset();
        to_ = world;
        startZoom_ = view_->Zoom();
        panZoom_ = zoomLocked_ ? startZoom_ : std::clamp(zoom, minZoom_, maxZoom_);
        panDuration_ = std::max(0.f, duration);
        panTime_ = 0;
        if (!panDuration_) {
            view_->SetOffset(to_, true);
            view_->SetZoom(panZoom_);
        }
    }

    void TouchControls::onZoom(float factor, Vec2 screen) {
        if (zoomLocked_ || factor <= 0 || !std::isfinite(factor) || !valid(screen))
            return;
        auto before = view_->ScreenToWorld(screen);
        view_->SetZoom(std::clamp(view_->Zoom() * factor, minZoom_, maxZoom_));
        auto after = view_->ScreenToWorld(screen), o = view_->Offset();
        if (!locked_)
            view_->SetOffset({o.x + before.x - after.x, o.y + before.y - after.y}, true);
    }

    void TouchControls::Pick(Vec2 point, InteractionMode mode) {
        if (valid(point) && mode == InteractionMode::Select)
            gotMsgOceanViewPickResult(view_->ScreenToWorld(point));
    }

    void TouchControls::gotMsgOceanViewPickResult(Vec2 world) {
        if (valid(world))
            picked_ = world;
    }

    void TouchControls::touchDown(const Vec2& p) {
        pressed_ = !locked_ && valid(p);
        dragged_ = false;
        start_ = last_ = p;
        panDuration_ = 0;
        picked_.reset();
    }

    void TouchControls::touchDrag(const Vec2& p) {
        if (!pressed_ || locked_ || !valid(p))
            return;
        if (!dragged_) {
            float dx = p.x - start_.x, dy = p.y - start_.y;
            if (dx * dx + dy * dy < 25)
                return;
            dragged_ = true;
        }
        auto a = view_->ScreenToWorld(last_), b = view_->ScreenToWorld(p), o = view_->Offset();
        view_->SetOffset({o.x + a.x - b.x, o.y + a.y - b.y}, true);
        last_ = p;
    }

    void TouchControls::touchUp(const Vec2& p) {
        if (!pressed_)
            return;
        touchDrag(p);
        pressed_ = false;
        if (!dragged_)
            Pick(p, InteractionMode::Select);
    }

    void TouchControls::touchCancel(const Vec2&) {
        pressed_ = false;
        dragged_ = false;
    }

    void TouchControls::tick(float dt) {
        if (panDuration_ <= 0 || dt <= 0 || !std::isfinite(dt))
            return;
        panTime_ += dt;
        float t = std::min(1.f, panTime_ / panDuration_);
        t = t * t * (3 - 2 * t);
        view_->SetOffset({from_.x + (to_.x - from_.x) * t, from_.y + (to_.y - from_.y) * t}, true);
        if (!zoomLocked_)
            view_->SetZoom(startZoom_ + (panZoom_ - startZoom_) * t);
        if (panTime_ >= panDuration_)
            panDuration_ = 0;
    }

    bool TouchControls::consumeDrag() {
        bool result = dragged_;
        dragged_ = false;
        return result;
    }
} // namespace rp
