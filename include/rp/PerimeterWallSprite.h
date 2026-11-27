#pragma once
#include "rp/OverlayAccessory.h"

namespace rp {
    class PerimeterWallSprite : public OverlayAccessory {
      public:
        PerimeterWallSprite(const OceanView*, GridSprite*);
        ~PerimeterWallSprite() override;
        void AddBalloons();
        void Init() override;
        void RemoveFromTick() override;
        void SetPriorities() override;
        void SetPriority(unsigned, float) override;
        void SetVisible(bool) override;
        void Update() override;
        void tick(float) override;
        int submit(sys::gfx::GfxManager&, sys::gfx::AnimationRenderer&) const override;

        // Native destructor releases a string before delegating to SpritePiece.
        struct SpriteBalloon : GridSprite::SpritePiece {
            virtual ~SpriteBalloon();
            std::string animation;
        };

      private:
        std::shared_ptr<const sys::res::ResourceAEAnim> balloonResource_;
        std::vector<std::unique_ptr<sys::gfx::AEAnim>> balloons_;
        float elapsed_ = 0;
        bool wasBuilding_ = false, celebrating_ = false;
        bool building() const;
    };
} // namespace rp
