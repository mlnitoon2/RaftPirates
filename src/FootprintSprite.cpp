#include "rp/FootprintSprite.h"
#include <cmath>

namespace rp {
    using namespace sys::gfx;
    FootprintSprite::SpriteFootprint::~SpriteFootprint() = default;

    FootprintSprite::FootprintSprite(const OceanView& view, std::shared_ptr<const Image> image,
                                     const sys::res::SpriteCell& cell)
        : view_(view), image_(std::move(image)), cell_(cell) {
        Init();
    }

    FootprintSprite::~FootprintSprite() = default;

    void FootprintSprite::Init() {
        sprites_.clear();
    }

    void FootprintSprite::SetColour(uint8_t r, uint8_t g, uint8_t b, uint8_t a) {
        color_ = {r, g, b, a};
        for (auto& p : sprites_)
            p.sprite.SetColor(r, g, b, a);
    }

    void FootprintSprite::SetPriority(unsigned i, float p) {
        sprites_.at(i).sprite.SetPriority(p);
    }

    void FootprintSprite::SetPriorities() {
        for (unsigned i = 0; i < sprites_.size(); ++i)
            SetPriority(i, view_.Priority() + sprites_[i].sprite.pos.y);
    }

    void FootprintSprite::Update(Cell position, const std::vector<Cell>& footprint,
                                 const Raft* parent) {
        Init();
        if (!image_ || cell_.ow <= 0 || cell_.oh <= 0)
            return;
        const float step = parent ? 1.f / 6 : 1.f;
        const float width = view_.Unit() * 2 * view_.Zoom() * step;
        if (!std::isfinite(width) || width <= 0)
            return;
        for (auto c : footprint) {
            auto center = view_.WorldToScreen(
                {(parent ? parent->x : 0) + (float(position.x) + c.x + .5f) * step,
                 (parent ? parent->y : 0) + (float(position.y) + c.y + .5f) * step});
            if (!std::isfinite(center.x) || !std::isfinite(center.y))
                continue;
            SpriteFootprint p;
            p.sprite.image = image_;
            p.sprite.cell = cell_;
            p.sprite.SetScale(width / cell_.ow, width * .5f / cell_.oh);
            p.sprite.SetPos(center.x - width * .5f, center.y - width * .25f);
            p.sprite.SetColor(color_.r, color_.g, color_.b, color_.a);
            sprites_.push_back(std::move(p));
        }
        SetPriorities();
    }

    void FootprintSprite::Render(Image& target) const {
        GfxManager mgr;
        for (const auto& p : sprites_)
            mgr.submit(p.sprite);
        mgr.RenderScreen(target);
    }
} // namespace rp
