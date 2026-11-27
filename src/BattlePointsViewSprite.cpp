#include "rp/BattlePointsViewSprite.h"
#include "rp/GridSprite.h"
#include <algorithm>

namespace rp {
    BattlePointsViewSprite::BattlePointsViewSprite(const OceanView* ocean, GridSprite* parent,
                                                   std::string prefix, int overridePoints)
        : AccessorySprite(ocean, parent, 6), prefix_(std::move(prefix)), override_(overridePoints) {
        if (ocean)
            try {
                auto font = std::make_shared<sys::gfx::ResourceSpriteFont>(
                    sys::gfx::ResourceSpriteFont::Create(ocean->assetRoot(), "rp_font_title"));
                text_.init(font, "", {});
            } catch (const std::exception&) {
            }
    }

    BattlePointsViewSprite::~BattlePointsViewSprite() = default;

    // Native ignores externally supplied color; tick controls label opacity.
    void BattlePointsViewSprite::SetColour(uint8_t, uint8_t, uint8_t, uint8_t) {}

    void BattlePointsViewSprite::tick(float) {
        if (!ocean_ || !parent_)
            return;
        const int points = override_ == -1 ? parent_->battlePoints() : override_;
        if (value_.empty() || last_ != points) {
            last_ = points;
            value_ = prefix_ + std::to_string(points);
            text_.changeText(value_);
        }
        const float zoom = ocean_->Zoom();
        const float opacity = std::clamp((zoom - .3f) / .1f, 0.f, 1.f);
        text_.SetAlpha(static_cast<uint8_t>(255 * opacity));
        text_.SetVisible(opacity > 0 && parent_->GetVisible());
        if (zoom > 0) {
            const float scale = std::min(8.f, 1.f / zoom);
            text_.SetScale(scale, scale);
        }
        if (parent_->Pieces()) {
            const auto& piece = (*parent_)[0].appearance;
            const auto p = ocean_->WorldToScreen(parent_->worldPosition());
            auto image = text_.renderedTexture();
            text_.SetPos(p.x - (image ? image->width * text_.scale.x * .5f : 0.f), p.y - 18.f);
        }
    }

    int BattlePointsViewSprite::submit(sys::gfx::GfxManager& manager,
                                       sys::gfx::AnimationRenderer&) const {
        if (!text_.visible || !parent_ || !parent_->GetVisible())
            return 0;
        auto image = text_.renderedTexture();
        if (!image || image->width <= 0 || image->height <= 0)
            return 0;
        sys::gfx::GfxSprite s;
        s.init(image);
        s.SetPos(text_.pos.x, text_.pos.y);
        s.SetScale(text_.scale.x, text_.scale.y);
        s.SetAlpha(text_.Alpha());
        s.SetPriority(parent_->Pieces() ? parent_->GetPriority(0) + .07f : 0);
        manager.submit(s);
        return 1;
    }
} // namespace rp
