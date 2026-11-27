#include "rp/WaterBucketSprite.h"

namespace rp {
    WaterBucketSprite::WaterBucketSprite() = default;
    WaterBucketSprite::~WaterBucketSprite() = default;

    void WaterBucketSprite::tick(float dt) {
        if (std::isfinite(dt) && dt > 0) {
            phase_ = std::fmod(phase_ - dt, 1.f);
            if (phase_ < 0)
                phase_ += 1;
        }
    }
} // namespace rp
