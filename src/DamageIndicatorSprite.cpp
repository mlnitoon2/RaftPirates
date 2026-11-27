#include "rp/DamageIndicatorSprite.h"
#include <cmath>
#include <algorithm>

namespace rp {
    DamageIndicatorSprite::DamageIndicatorSprite(bool o, int r, int a)
        : own(o), raft(r), amount(a) {}

    DamageIndicatorSprite::~DamageIndicatorSprite() = default;

    void DamageIndicatorSprite::tick(float dt) {
        if (std::isfinite(dt) && dt > 0)
            remaining = std::max(0.f, remaining - dt);
    }
} // namespace rp
