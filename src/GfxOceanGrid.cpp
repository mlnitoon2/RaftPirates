#include "rp/GfxOceanGrid.h"
#include "rp/Game.h"
#include <cmath>
#include <algorithm>

namespace rp {
    using namespace sys::gfx;

    GfxOceanGrid::GfxOceanGrid(const OceanView* view, float unit) : view_(view), unit_(192) {
        SetUnitSize(unit);
    }

    GfxOceanGrid::~GfxOceanGrid() = default;

    float GfxOceanGrid::FloatMod(float value, float modulus) const {
        return std::isfinite(value) && std::isfinite(modulus) && modulus != 0
                   ? std::fmod(value, modulus)
                   : 0;
    }

    void GfxOceanGrid::SetPosition(Vec2 p) {
        position_ = p;
    }

    void GfxOceanGrid::SetUnitSize(float size) {
        if (std::isfinite(size) && size > 0)
            unit_ = size;
    }

    void GfxOceanGrid::Render(Image& dst) const {
        if (!view_)
            return;
        auto a = view_->AxisX(), b = view_->AxisY();
        const float scale = unit_ / view_->Unit() * view_->Zoom() / 6.f;
        a = {a.x * scale, a.y * scale};
        b = {b.x * scale, b.y * scale};
        auto line = [&](Vec2 p, Vec2 q) {
            if (!std::isfinite(p.x) || !std::isfinite(p.y) || !std::isfinite(q.x) ||
                !std::isfinite(q.y))
                return;
            const float distance = std::max(std::abs(q.x - p.x), std::abs(q.y - p.y));
            if (distance > 8192 || distance < 0)
                return;
            const int steps = std::max(1, int(std::ceil(distance)));
            for (int i = 0; i <= steps; ++i) {
                const float t = float(i) / steps, x = p.x + (q.x - p.x) * t,
                            y = p.y + (q.y - p.y) * t;
                if (x >= 0 && y >= 0 && x < dst.width && y < dst.height)
                    dst.at(int(x), int(y)) = {225, 236, 216, 255};
            }
        };
        for (int i = 0; i <= 6; ++i) {
            Vec2 p{position_.x + i * a.x, position_.y + i * a.y};
            line(p, {p.x + 6 * b.x, p.y + 6 * b.y});
            p = {position_.x + i * b.x, position_.y + i * b.y};
            line(p, {p.x + 6 * a.x, p.y + 6 * a.y});
        }
    }
} // namespace rp
