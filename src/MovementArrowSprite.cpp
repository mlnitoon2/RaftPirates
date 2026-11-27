#include "rp/MovementArrowSprite.h"
#include "rp/GridSprite.h"
#include <cmath>

namespace rp {
    MovementArrowSprite::MovementArrowSprite(const OceanView* ocean, GridSprite* parent, int piece)
        : AccessorySprite(ocean, parent, 16), piece_(piece) {}
    MovementArrowSprite::~MovementArrowSprite() = default;
    void MovementArrowSprite::tick(float) {
        if (!ocean_ || !parent_ || piece_ < 0 || static_cast<size_t>(piece_) >= parent_->Pieces()) return;
        const float zoom = ocean_->Zoom();
        if (!std::isfinite(zoom) || zoom <= 0) return;
        const auto& piece = (*parent_)[static_cast<unsigned>(piece_)].appearance;
        const float scale = 1.f / zoom;
        arrow_.SetScale(scale, scale);
        arrow_.SetPos(piece.pos.x + .5f*piece.width()*piece.scale.x - .5f*arrow_.width()*scale,
                      piece.pos.y + .5f*piece.height()*piece.scale.y - 34.f*scale);
    }
}
