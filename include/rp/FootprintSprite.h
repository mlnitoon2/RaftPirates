#pragma once
#include "rp/Game.h"

namespace rp {
    class FootprintSprite {
      public:
        struct SpriteFootprint {
            sys::gfx::GfxSprite sprite;
            ~SpriteFootprint();
        };

        FootprintSprite(const OceanView& view, std::shared_ptr<const sys::gfx::Image> image,
                        const sys::res::SpriteCell& cell);
        ~FootprintSprite();
        void Init();
        void SetColour(uint8_t r, uint8_t g, uint8_t b, uint8_t a);
        void SetPriorities();
        void SetPriority(unsigned piece, float priority);
        void Update(Cell position, const std::vector<Cell>& footprint, const Raft* parent);
        void Render(sys::gfx::Image& target) const;

      private:
        const OceanView& view_;
        std::shared_ptr<const sys::gfx::Image> image_;
        sys::res::SpriteCell cell_;
        sys::gfx::Color color_{};
        std::vector<SpriteFootprint> sprites_;
    };
} // namespace rp
