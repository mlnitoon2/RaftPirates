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

    Image OceanView::render(const Game& game, float time, bool grid) {
        Image out(640, 400, {36, 133, 168, 255});
        GfxManager mgr;
        auto load = [&](const std::string& p) {
            auto it = images_.find(p);
            if (it == images_.end())
                it = images_.emplace(p, std::make_shared<Image>(loadAssetImage(root_ / p))).first;
            return it->second;
        };
        auto base = [](const Raft& r) {
            return Vec2{320.f + (r.x - r.y) * 192.f, 86.f + (r.x + r.y) * 96.f};
        };
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
                s.SetPos(b.x + it->second.offsetX, b.y + it->second.offsetY);
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
                    line(out, int(b.x + x * 32), int(b.y + x * 16), int(b.x + (x - 6) * 32),
                         int(b.y + (x + 6) * 16), {225, 236, 216, 255});
                    line(out, int(b.x - x * 32), int(b.y + x * 16), int(b.x + (6 - x) * 32),
                         int(b.y + (6 + x) * 16), {225, 236, 216, 255});
                }
            for (auto& c : r.components) {
                auto it = game.objects.find(c.type);
                if (it == game.objects.end())
                    continue;
                float x = b.x + (c.x - c.y) * 32, y = b.y + (c.x + c.y + 1) * 16;
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
                        float scale = .5f;
                        auto m = Matrix::translation(x - comp->width * scale / 2,
                                                     y - comp->height * scale * .5f - 12.f) *
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
