#include "rp/Game.h"
#include "rp/GridSprite.h"
#include "sys/gfx/AssetImage.h"
#include <cmath>
#include <algorithm>

namespace rp {
    using namespace sys::gfx;
    using namespace sys::res;

    namespace {
        void line(Image& out, int x, int y, int x2, int y2, Color c) {
            int dx = std::abs(x2 - x), dy = -std::abs(y2 - y), sx = x < x2 ? 1 : -1,
                sy = y < y2 ? 1 : -1, err = dx + dy;
            while (true) {
                if (x >= 0 && y >= 0 && x < out.width && y < out.height)
                    out.at(x, y) = c;
                if (x == x2 && y == y2)
                    break;
                int e = 2 * err;
                if (e >= dy) {
                    err += dy;
                    x += sx;
                }
                if (e <= dx) {
                    err += dx;
                    y += sy;
                }
            }
        }
    } // namespace

    void OceanView::SetPosition(Vec2 p) {
        position_ = p;
    }

    void OceanView::SetWidth(float w) {
        if (w > 0 && std::isfinite(w))
            width_ = w;
    }

    void OceanView::SetHeight(float h) {
        if (h > 0 && std::isfinite(h))
            height_ = h;
    }

    void OceanView::SetPriority(float p) {
        priority_ = p;
    }

    void OceanView::SetUnitSize(float size) {
        if (size > 0 && std::isfinite(size)) {
            unit_ = size;
            CalculateHelpers();
        }
    }

    Vec2 OceanView::Position() const {
        return position_;
    }

    float OceanView::Width() const {
        return width_;
    }

    float OceanView::Height() const {
        return height_;
    }

    float OceanView::Priority() const {
        return priority_;
    }

    float OceanView::Unit() const {
        return unit_;
    }

    Vec2 OceanView::AxisX() const {
        return axisX_;
    }

    Vec2 OceanView::AxisY() const {
        return axisY_;
    }

    Vec2 OceanView::ConvertToViewSpecial(Vec2 w) const {
        return {192 * (w.x - w.y), 96 * (w.x + w.y)};
    }

    Vec2 OceanView::ConvertToView(Vec2 w) const {
        return {w.x * axisX_.x + w.y * axisY_.x, w.x * axisX_.y + w.y * axisY_.y};
    }

    Vec2 OceanView::ConvertToWorld(Vec2 v) const {
        return {(v.x / unit_ + 2 * v.y / unit_) / 2, (2 * v.y / unit_ - v.x / unit_) / 2};
    }

    Vec2 OceanView::WorldToScreen(Vec2 w) const {
        auto v = ConvertToView({w.x - offset_.x, w.y - offset_.y});
        return {position_.x + v.x * zoom_, position_.y + v.y * zoom_};
    }

    Vec2 OceanView::ScreenToWorld(Vec2 s) const {
        auto w = ConvertToWorld({(s.x - position_.x) / zoom_, (s.y - position_.y) / zoom_});
        return {w.x + offset_.x, w.y + offset_.y};
    }

    void OceanView::SetZoom(float z) {
        if (z > 0 && std::isfinite(z))
            zoom_ = z;
    }

    float OceanView::Zoom() const {
        return zoom_;
    }

    void OceanView::SetOffset(Vec2 o, bool) {
        offset_ = o;
    }

    Vec2 OceanView::Offset() const {
        return offset_;
    }

    void OceanView::CalculateHelpers() {
        axisX_ = {unit_, unit_ / 2};
        axisY_ = {-unit_, unit_ / 2};
    }

    void OceanView::Update() {
        CalculateHelpers();
    }

    bool OceanView::Pick(const Player& game, int parent, int object, Vec2 screen, float time) {
        if (screen.x < 0 || screen.y < 0 || screen.x >= 640 || screen.y >= 400)
            return false;
        auto raft = game.GetRaftByID(parent);
        if (!raft)
            return false;
        for (const auto& c : raft->components) {
            if (c.id != object)
                continue;
            auto def = game.objects.find(c.type);
            if (def == game.objects.end())
                return false;
            GridSprite sprite(*this, c, def->second,
                              {float(raft->x) + float(c.x) / 6 + 1.f / 12,
                               float(raft->y) + float(c.y) / 6 + 1.f / 12},
                              zoom_);
            sprite.UpdateAnimations(std::max(0.f, time));
            Image mask(640, 400, {0, 0, 0, 0});
            GfxManager mgr;
            sprite.submit(mgr, images_, bins_, animations_, root_, false);
            mgr.RenderScreen(mask);
            if (mask.at(int(screen.x), int(screen.y)).a > 32)
                return true;
            auto world = ScreenToWorld(screen);
            int cx = int(std::floor((world.x - raft->x) * 6)),
                cy = int(std::floor((world.y - raft->y) * 6));
            for (auto cell : def->second.cells)
                if (cx == c.x + cell.x && cy == c.y + cell.y)
                    return true;
        }
        return false;
    }

    Image OceanView::render(const Player& game, float time, bool grid, const DiveState* dive) {
        const bool diving = dive && dive->Diving();
        if (lastDiving_ && !diving)
            returnStart_ = time;
        if (diving) {
            lastDiveParent_ = dive->parent;
            lastDiveObject_ = dive->object;
        }
        lastDiving_ = diving;
        Image out(640, 400, {36, 133, 168, 255});
        GfxManager mgr;
        auto base = [this](const Raft& r) { return WorldToScreen({float(r.x), float(r.y)}); };
        for (const auto& r : game.rafts) {
            auto it = game.objects.find(r.type);
            if (it == game.objects.end())
                continue;
            GridSprite sprite(*this, r, it->second, {float(r.x), float(r.y)}, zoom_);
            if (r.Destroyed())
                sprite.SetColour(150, 150, 150, 210);
            sprite.UpdateAnimations(std::max(0.f, time));
            sprite.submit(mgr, images_, bins_, animations_, root_, true);
        }
        mgr.RenderScreen(out);
        for (auto& r : game.rafts) {
            auto b = base(r);
            if (grid)
                for (int x = 0; x <= 6; ++x) {
                    line(out, int(b.x + x * 32 * zoom_), int(b.y + x * 16 * zoom_),
                         int(b.x + (x - 6) * 32 * zoom_), int(b.y + (x + 6) * 16 * zoom_),
                         {225, 236, 216, 255});
                    line(out, int(b.x - x * 32 * zoom_), int(b.y + x * 16 * zoom_),
                         int(b.x + (6 - x) * 32 * zoom_), int(b.y + (6 + x) * 16 * zoom_),
                         {225, 236, 216, 255});
                }
            for (auto& c : r.components) {
                auto it = game.objects.find(c.type);
                if (it == game.objects.end())
                    continue;
                GridSprite sprite(*this, c, it->second,
                                  {float(r.x) + float(c.x) / 6 + 1.f / 12,
                                   float(r.y) + float(c.y) / 6 + 1.f / 12},
                                  zoom_);
                if (c.Destroyed())
                    sprite.SetColour(150, 150, 150, 180);
                float animTime = std::max(0.f, time);
                if (diving && dive->parent == r.id && dive->object == c.id) {
                    float elapsed = std::max(0.f, float(dive->totalSeconds) - dive->remaining);
                    if (elapsed < 2.0167f) {
                        sprite.SetAnimation({"drop"});
                        animTime = elapsed;
                    } else if (dive->remaining <= 0) {
                        sprite.SetAnimation({"return"});
                        animTime = .68f;
                    } else {
                        sprite.SetAnimation({"diving"});
                        animTime = elapsed - 2.0167f;
                    }
                } else if (r.id == lastDiveParent_ && c.id == lastDiveObject_ &&
                           time - returnStart_ < .6833f && time >= returnStart_) {
                    sprite.SetAnimation({"return"});
                    animTime = time - returnStart_;
                }
                sprite.UpdateAnimations(animTime);
                sprite.submit(mgr, images_, bins_, animations_, root_, false);
            }
        }
        mgr.RenderScreen(out);
        return out;
    }
} // namespace rp
