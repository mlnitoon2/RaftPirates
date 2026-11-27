#include "rp/DiveMeterSprite.h"
#include "sys/gfx/AssetImage.h"
#include "sys/localization/LocalizationManager.h"
#include <algorithm>
#include <cmath>
#include <stdexcept>

namespace rp {
    using namespace sys::gfx;

    DiveMeterSprite::DiveMeterSprite(const OceanView* o, GridSprite* p)
        : OverlayAccessory(o, p, 2) {
        loadSheet("xml_resources/rp_menu.xml");
        if (appendImage("gfx/HUD/dive_game_meter_frame.png", {0, -64}, 1, .03f))
            frame_ = int(overlays_.size()) - 1;
        if (appendImage("gfx/HUD/dive_game_meter.png", {0, -64}, 1, .04f))
            fill_ = int(overlays_.size()) - 1;
        if (appendCell("dive_game_button", {0, -10}, .5f, .05f))
            button_ = int(overlays_.size()) - 1;
        if (o)
            try {
                font_ = std::make_shared<ResourceSpriteFont>(
                    ResourceSpriteFont::Create(o->assetRoot(), "rp_font_title"));
            } catch (const std::exception&) {
            }
        layout();
        OverlayAccessory::SetPriorities();
    }

    DiveMeterSprite::~DiveMeterSprite() = default;

    // Native setters return immediately; tick reads authoritative dive progress.
    void DiveMeterSprite::SetPriorities() {}

    void DiveMeterSprite::SetTime(int) {}

    void DiveMeterSprite::SetDive(const DivingUnit& d) {
        dive_ = d;
        tick(0);
    }

    bool DiveMeterSprite::Select(Gfx* gfx) {
        if (!gfx || !visible_ || !dive_.Diving() || dive_.remaining <= 0 || button_ < 0 ||
            gfx != &overlays_[button_].sprite)
            return false;
        if (selected_)
            selected_();
        return true;
    }

    void DiveMeterSprite::tick(float dt) {
        if (!std::isfinite(dt) || dt < 0)
            return;
        phase_ = std::fmod(phase_ + dt, 1.5f);
        layout();
        OverlayAccessory::SetPriorities();
        if (fill_ >= 0) {
            auto& s = overlays_[fill_].sprite;
            const float h = float(s.cell.oh);
            const float f = dive_.GetDiveTime();
            s.SetMask(0, h * (1 - f), float(s.cell.ow), h * f, true);
        }
        if (button_ >= 0) {
            auto& b = overlays_[button_];
            const float s = b.size * scale_ * (1.f + .1f * std::sin(phase_ * (6.2831853f / 1.5f)));
            b.sprite.SetScale(s, s);
            b.sprite.SetVisible(visible_ && dive_.Diving() && dive_.remaining > 0);
        }
    }

    int DiveMeterSprite::submit(GfxManager& m, AnimationRenderer& r) const {
        if (!visible_ || !dive_.Diving())
            return 0;
        int n = OverlayAccessory::submit(m, r);
        if (font_ && parent_ && ocean_) {
            auto p = ocean_->WorldToScreen(parent_->worldPosition());
            auto l = font_->layout(GetTimeString(int(std::ceil(std::max(0.f, dive_.remaining)))),
                                   .65f * scale_);
            font_->submit(m, l, p.x - l.width * .5f, p.y - 62.5f * scale_ - l.height * .5f, {},
                          basePriority() + .06f);
        }
        return n;
    }
} // namespace rp
