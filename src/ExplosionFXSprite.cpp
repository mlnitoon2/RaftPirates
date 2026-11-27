#include "rp/ExplosionFXSprite.h"
#include <algorithm>
#include <cmath>

namespace rp {
    ExplosionFXSprite::ExplosionFXSprite(bool raft, bool boss) : raft_(raft), boss_(boss) {
        Init();
    }

    ExplosionFXSprite::~ExplosionFXSprite() = default;

    void ExplosionFXSprite::Init() {
        elapsed_ = 0;
        active_ = true;
    }

    void ExplosionFXSprite::RemoveFromTick() {
        active_ = false;
    }

    void ExplosionFXSprite::Update() {} // Native return-only hook.

    bool ExplosionFXSprite::Sparatic() const {
        return raft_ || boss_;
    }

    float ExplosionFXSprite::GetTime() const {
        return raft_ ? 1.5f : .5f;
    }

    void ExplosionFXSprite::tick(float dt) {
        if (!active_ || !std::isfinite(dt) || dt < 0)
            return;
        elapsed_ = std::min(GetTime(), elapsed_ + dt);
        if (elapsed_ >= GetTime())
            RemoveFromTick();
    }
} // namespace rp
