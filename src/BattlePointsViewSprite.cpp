#include "rp/BattlePointsViewSprite.h"
#include "rp/GridSprite.h"
#include <algorithm>

namespace rp {
    BattlePointsViewSprite::BattlePointsViewSprite(const OceanView* ocean, GridSprite* parent,
                                                   std::string prefix, int overridePoints)
        : AccessorySprite(ocean, parent, 17), prefix_(std::move(prefix)), override_(overridePoints) {}
    BattlePointsViewSprite::~BattlePointsViewSprite() = default;
    // Native ignores externally supplied color; tick controls label opacity.
    void BattlePointsViewSprite::SetColour(uint8_t, uint8_t, uint8_t, uint8_t) {}
    void BattlePointsViewSprite::tick(float) {
        if (!ocean_ || !parent_) return;
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
            text_.SetPos(piece.pos.x, piece.pos.y - 34.f);
        }
    }
}
