#pragma once
#include "rp/GridSprite.h"
#include "sys/gfx/AnimationRenderer.h"

namespace rp {
    // Portable shared atlas overlay: native classes retain separate identities.
    class OverlayAccessory : public AccessorySprite {
      public:
        OverlayAccessory(const OceanView* ocean, GridSprite* parent, int id)
            : AccessorySprite(ocean, parent, id) {}

        void SetScale(float scale) override;
        void SetVisible(bool visible) override;
        void SetColour(uint8_t, uint8_t, uint8_t, uint8_t) override;
        void SetPriorities() override;
        void SetPriority(unsigned, float) override;
        bool ContainsGfx(const sys::gfx::Gfx*) const override;
        int submit(sys::gfx::GfxManager&, sys::gfx::AnimationRenderer&) const override;

      protected:
        struct Piece {
            sys::gfx::GfxSprite sprite;
            sys::gfx::Vec2 offset;
            float size = 1, priorityOffset = 0;
        };

        std::vector<Piece> overlays_;
        std::shared_ptr<sys::res::ResourceSpriteSheet> sheet_;
        std::shared_ptr<const sys::gfx::Image> atlas_;
        float scale_ = 1;
        bool visible_ = true;
        bool loadSheet(const char*);
        bool appendCell(const char*, sys::gfx::Vec2 = {}, float = 1, float = 0);
        bool appendImage(const char*, sys::gfx::Vec2 = {}, float = 1, float = 0);
        bool changeCell(unsigned, const char*);
        void layout();
        float basePriority() const;
        void pulse(float phase, bool red);
    };
} // namespace rp
