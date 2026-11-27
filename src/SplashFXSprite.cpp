#include "rp/SplashFXSprite.h"
#include "sys/gfx/AssetImage.h"
#include "sys/localization/LocalizationManager.h"
#include <algorithm>
#include <cmath>
#include <stdexcept>

namespace rp {
    using namespace sys::gfx;

    SplashFXSprite::SplashFXSprite(const OceanView* o, GridSprite* p, float seconds)
        : AccessorySprite(o, p, 11),
          burstCount_(std::isfinite(seconds) ?
                          unsigned(std::clamp(seconds, 0.f, 60.f) * 24.f) : 0) {}

    SplashFXSprite::~SplashFXSprite() = default;

    void SplashFXSprite::Init() {
        splashes_.clear();
        remaining_ = burstCount_;
        countdown_ = 0;
        finished_ = true;
        if (!ocean_ || !parent_ || !remaining_)
            return;
        try {
            resource_ = std::make_shared<sys::res::ResourceAEAnim>(sys::res::ResourceAEAnim::Create(
                ocean_->assetRoot() / "xml_bin/anim_water_splash.bin", sys::res::AEFormat::Rev1_5));
            if (!resource_->find("splash"))
                return;
            spawnBurst();
            finished_ = splashes_.empty();
        } catch (const std::exception&) {
            splashes_.clear();
            resource_.reset();
            remaining_ = 0;
        }
    }

    void SplashFXSprite::spawnBurst() {
        if (!remaining_ || !resource_)
            return;
        // ARM32 Init schedules the next burst before decrementing the count.
        countdown_ = 1.5f / float(remaining_);
        --remaining_;
        // Native iterates raft cells. This view still uses reconstructed visual pieces.
        for (unsigned i = 0; i < parent_->Pieces(); ++i) {
            auto a = std::make_unique<AEAnim>(resource_);
            a->setLoop(false);
            a->setAnimation("splash");
            splashes_.push_back({std::move(a), parent_->GetWorldPosition(int(i))});
        }
    }

    // Original Update and virtual RemoveFromTick are return-only; tick calls base on completion.
    void SplashFXSprite::Update() {}

    void SplashFXSprite::RemoveFromTick() {}

    void SplashFXSprite::tick(float dt) {
        if (finished_ || !std::isfinite(dt) || dt < 0)
            return;
        for (auto& s : splashes_)
            s.animation->tick(dt);
        splashes_.erase(std::remove_if(splashes_.begin(), splashes_.end(),
                                       [](const auto& s) { return !s.animation->playing(); }),
                        splashes_.end());
        if (remaining_ && countdown_ > 0) {
            countdown_ -= dt;
            if (countdown_ <= 0)
                spawnBurst();
        }
        if (splashes_.empty() && !remaining_) {
            finished_ = true;
            AccessorySprite::RemoveFromTick();
        }
    }

    int SplashFXSprite::submit(GfxManager& m, AnimationRenderer& r) const {
        if (finished_ || !ocean_ || !parent_ || !parent_->GetVisible())
            return 0;
        int n = 0;
        const float s = ocean_->Zoom();
        for (const auto& a : splashes_) {
            auto p = ocean_->WorldToScreen(a.world);
            n += r.submit(m, *a.animation,
                          Matrix::translation(p.x - a.animation->width() * s * .5f,
                                              p.y - a.animation->height() * s * .5f) *
                              Matrix::scale(s, s));
        }
        return n;
    }
} // namespace rp
