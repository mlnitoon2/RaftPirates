#include "rp/RaftAttackSprite.h"
#include "rp/GridSprite.h"
#include <algorithm>
#include <cmath>

namespace rp {
    RaftAttackSprite::RaftAttackSprite(const OceanView* ocean, GridSprite* parent,
                                       std::shared_ptr<const sys::res::ResourceAEAnim> resource,
                                       float delay)
        : AccessorySprite(ocean, parent, 15), resource_(std::move(resource)),
          delay_(std::isfinite(delay) ? std::max(0.f, delay) : 0.f) {}

    RaftAttackSprite::~RaftAttackSprite() = default;

    void RaftAttackSprite::Init() {
        front_.reset();
        back_.reset();
        finished_ = false;
        if (!resource_ || !parent_) {
            finished_ = true;
            return;
        }
        const std::string prefix = parent_->definitionName() == "CoreRaft" ? "core" : "raft";
        auto create = [&](const std::string& name) {
            if (!resource_->find(name))
                return std::unique_ptr<sys::gfx::AEAnim>{};
            auto anim = std::make_unique<sys::gfx::AEAnim>(resource_);
            anim->setLoop(false);
            anim->setAnimation(name);
            return anim;
        };
        front_ = create(prefix + "_attack_front");
        back_ = create(prefix + "_attack_back");
        finished_ = !front_ && !back_;
    }

    // Original Update returns immediately.
    void RaftAttackSprite::Update() {}

    void RaftAttackSprite::tick(float dt) {
        if (finished_ || !std::isfinite(dt) || dt < 0)
            return;
        const float consumed = std::min(delay_, dt);
        delay_ -= consumed;
        dt -= consumed;
        if (delay_ > 0)
            return;
        if (front_)
            front_->tick(dt);
        if (back_)
            back_->tick(dt);
        finished_ = (!front_ || !front_->playing()) && (!back_ || !back_->playing());
        if (finished_)
            RemoveFromTick();
    }

    int RaftAttackSprite::submit(sys::gfx::GfxManager& manager,
                                 sys::gfx::AnimationRenderer& renderer) const {
        if (finished_ || delay_ > 0 || !ocean_ || !parent_ || !parent_->Pieces())
            return 0;
        const auto& piece = (*parent_)[0].appearance;
        int result = 0;
        auto draw = [&](const auto& anim) {
            if (!anim || !anim->playing())
                return;
            const auto scale = ocean_->Zoom();
            auto transform =
                sys::gfx::Matrix::translation(piece.pos.x - anim->width() * scale * .5f,
                                              piece.pos.y - anim->height() * scale * .5f) *
                sys::gfx::Matrix::scale(scale, scale);
            result += renderer.submit(manager, *anim, transform);
        };
        draw(back_);
        draw(front_);
        return result;
    }
} // namespace rp
