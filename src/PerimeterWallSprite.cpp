#include "rp/PerimeterWallSprite.h"
#include "sys/gfx/AssetImage.h"
#include "sys/localization/LocalizationManager.h"
#include <algorithm>
#include <cmath>
#include <stdexcept>

namespace rp {
    using namespace sys::gfx;

    PerimeterWallSprite::PerimeterWallSprite(const OceanView* o, GridSprite* p)
        : OverlayAccessory(o, p, 1) {}

    PerimeterWallSprite::~PerimeterWallSprite() = default;

    bool PerimeterWallSprite::building() const {
        return parent_ && parent_->constructionTime() > 0;
    }

    void PerimeterWallSprite::Init() {
        overlays_.clear();
        balloons_.clear();
        elapsed_ = 0;
        celebrating_ = false;
        wasBuilding_ = building();
        loadSheet("xml_resources/construction_footprints_junk.xml");
        appendCell("build_tarp_dark", {-16, -20}, 1, .01f);
        appendCell("build_tarp_light", {16, -20}, 1, .02f);
        appendCell("construction_timer", {0, -64}, 1, .03f);
        Update();
    }

    void PerimeterWallSprite::AddBalloons() {
        if (!ocean_ || celebrating_)
            return;
        celebrating_ = true;
        elapsed_ = 0;
        balloons_.clear();
        try {
            balloonResource_ =
                std::make_shared<sys::res::ResourceAEAnim>(sys::res::ResourceAEAnim::Create(
                    ocean_->assetRoot() / "xml_bin/anim_build_complete.bin",
                    sys::res::AEFormat::Rev1_5));
            for (const std::string color : {"red", "blue", "green", "yellow"}) {
                if (!balloonResource_->find(color + "_intro"))
                    continue;
                auto a = std::make_unique<AEAnim>(balloonResource_);
                a->setLoop(false);
                a->setAnimation(color + "_intro");
                if (balloonResource_->find(color + "_idle"))
                    a->queueAnimation(color + "_idle");
                balloons_.push_back(std::move(a));
            }
        } catch (const std::exception&) {
            balloonResource_.reset();
        }
    }

    void PerimeterWallSprite::RemoveFromTick() {
        if (!building())
            AccessorySprite::RemoveFromTick();
    }

    void PerimeterWallSprite::SetPriorities() {
        OverlayAccessory::SetPriorities();
    }

    void PerimeterWallSprite::SetPriority(unsigned i, float p) {
        OverlayAccessory::SetPriority(i, p);
    }

    void PerimeterWallSprite::SetVisible(bool v) {
        OverlayAccessory::SetVisible(v);
        for (auto& b : balloons_)
            b->SetVisible(v);
    }

    void PerimeterWallSprite::Update() {
        layout();
        SetPriorities();
    }

    void PerimeterWallSprite::tick(float dt) {
        if (!std::isfinite(dt) || dt < 0)
            return;
        const bool now = building();
        if (wasBuilding_ && !now)
            AddBalloons();
        wasBuilding_ = now;
        if (celebrating_) {
            elapsed_ += dt;
            for (auto& a : balloons_)
                a->tick(dt);
            if (elapsed_ >= 2.f) {
                balloons_.clear();
                celebrating_ = false;
                AccessorySprite::RemoveFromTick();
            }
        }
    }

    int PerimeterWallSprite::submit(GfxManager& m, AnimationRenderer& r) const {
        int n = building() ? OverlayAccessory::submit(m, r) : 0;
        if (!visible_ || !celebrating_ || !parent_ || !ocean_)
            return n;
        auto p = ocean_->WorldToScreen(parent_->worldPosition());
        for (unsigned i = 0; i < balloons_.size(); ++i) {
            const auto& a = balloons_[i];
            const float rise = std::max(0.f, elapsed_ - .5f) * 100.f * scale_;
            auto matrix = Matrix::translation(p.x + (float(i) - 1.5f) * 25 * scale_ -
                                                  a->width() * scale_ * .5f,
                                              p.y - a->height() * scale_ - rise) *
                          Matrix::scale(scale_, scale_);
            n += r.submit(m, *a, matrix);
        }
        return n;
    }
} // namespace rp

namespace rp {
    PerimeterWallSprite::SpriteBalloon::~SpriteBalloon() = default;
} // namespace rp
