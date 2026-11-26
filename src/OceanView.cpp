#include "rp/Game.h"
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

    Image OceanView::render(const Player& game, float time, bool grid) {
        Image out(640, 400, {36, 133, 168, 255});
        GfxManager mgr;
        auto load = [&](const std::string& p) {
            auto it = images_.find(p);
            if (it == images_.end())
                it = images_.emplace(p, std::make_shared<Image>(loadAssetImage(root_ / p))).first;
            return it->second;
        };
        auto base = [this](const Raft& r) { return WorldToScreen({float(r.x), float(r.y)}); };
        for (auto& r : game.rafts) {
            auto b = base(r);
            auto it = game.objects.find(r.type);
            if (it == game.objects.end())
                continue;
            try {
                auto im = load(it->second.asset);
                GfxSprite s;
                s.image = im;
                s.cell = {"raft", 0, 0, im->width, im->height, 0, 0, im->width, im->height, false};
                s.SetPos(b.x + it->second.offsetX * zoom_, b.y + it->second.offsetY * zoom_);
                s.SetScale(zoom_, zoom_);
                s.SetPriority((r.x + r.y) * 100.f);
                mgr.submit(s);
            } catch (const std::exception&) {
            }
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
                float x = b.x + (c.x - c.y) * 32 * zoom_, y = b.y + (c.x + c.y + 1) * 16 * zoom_;
                try {
                    const auto& path = it->second.asset;
                    auto a = bins_.find(path);
                    if (a == bins_.end())
                        a = bins_
                                .emplace(path, ResourceAEAnim::Create(root_ / path,
                                                                      AEFormat::RaftPirates2013))
                                .first;
                    auto& bin = a->second;
                    const AEComp* comp = nullptr;
                    for (auto& v : bin.comps)
                        if (v.name == "idle" || v.name == "Idle" ||
                            v.name.find("idle") != std::string::npos) {
                            comp = &v;
                            break;
                        }
                    if (!comp && !bin.comps.empty())
                        comp = &bin.comps.front();
                    if (comp && comp->width > 0 && comp->height > 0) {
                        float t = comp->duration() > 0 ? std::fmod(time, comp->duration()) : 0;
                        float scale = .5f * zoom_;
                        auto m =
                            Matrix::translation(x - comp->width * scale / 2,
                                                y - comp->height * scale * .5f - 12.f * zoom_) *
                            Matrix::scale(scale, scale);
                        if (animations_.submit(mgr, bin, *comp, t, m, y) > 0)
                            continue;
                    }
                } catch (const std::exception&) {
                }
                line(out, int(x - 10), int(y - 10), int(x + 10), int(y + 10), {250, 192, 70, 255});
                line(out, int(x - 10), int(y + 10), int(x + 10), int(y - 10), {250, 192, 70, 255});
            }
        }
        mgr.RenderScreen(out);
        return out;
    }
} // namespace rp
