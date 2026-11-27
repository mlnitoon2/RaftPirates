#include "rp/UnitHealthBarSprite.h"
#include "rp/Game.h"
#include <cmath>
#include <algorithm>

namespace rp {
    using namespace sys::gfx;

    UnitHealthBarSprite::UnitHealthBarSprite(const OceanView& view,
                                             std::shared_ptr<const Image> frame,
                                             std::shared_ptr<const Image> fill)
        : view_(view), frame_(std::move(frame)), fill_(std::move(fill)) {
        Init();
    }

    UnitHealthBarSprite::~UnitHealthBarSprite() = default;

    void UnitHealthBarSprite::Init() {
        health_ = 1;
        visible_ = true;
    }

    void UnitHealthBarSprite::SetColour(uint8_t r, uint8_t g, uint8_t b, uint8_t a) {
        colour_ = {r, g, b, a};
    }

    void UnitHealthBarSprite::SetVisible(bool value) {
        visible_ = value;
    }

    void UnitHealthBarSprite::Update(Vec2 world, float health) {
        world_ = world;
        health_ = std::isfinite(health) ? std::clamp(health, 0.f, 1.f) : 0;
    }

    void UnitHealthBarSprite::tick(float) {} // ARM32 0x62f8b4 returns directly.

    void UnitHealthBarSprite::Render(Image& dst) const {
        if (!visible_ || health_ >= 1 || !frame_ || !fill_ || frame_->width <= 0 ||
            fill_->width <= 0)
            return;
        const auto p = view_.WorldToScreen(world_);
        if (!std::isfinite(p.x) || !std::isfinite(p.y))
            return;
        const float s = .5f;
        if (p.x < 0 || p.x >= dst.width || p.y < 0 || p.y >= dst.height)
            return;
        const float x = std::clamp(p.x - frame_->width * s / 2, 0.f,
                                   std::max(0.f, dst.width - frame_->width * s));
        const float y = std::clamp(p.y - 24, 0.f, std::max(0.f, dst.height - frame_->height * s));
        GfxSprite frame;
        frame.init(frame_);
        frame.SetPos(x, y);
        frame.SetScale(s, s);
        frame.SetColor(colour_.r, colour_.g, colour_.b, colour_.a);
        drawSprite(dst, frame);
        const int w = int(fill_->width * health_);
        if (w <= 0)
            return;
        GfxSprite fill;
        fill.image = fill_;
        fill.cell = {"health", 0, 0, w, fill_->height, 0, 0, w, fill_->height, false};
        fill.SetPos(x + (frame_->width - fill_->width) * s / 2,
                    y + (frame_->height - fill_->height) * s / 2);
        fill.SetScale(s, s);
        fill.SetColor(colour_.r, colour_.g, colour_.b, colour_.a);
        drawSprite(dst, fill);
    }
} // namespace rp
