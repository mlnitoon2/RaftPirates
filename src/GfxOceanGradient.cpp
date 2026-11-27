#include "rp/GfxOceanGradient.h"
#include "rp/Game.h"
#include <cmath>
#include <algorithm>

namespace rp {
    using namespace sys::gfx;

    GfxOceanGradient::GfxOceanGradient(const OceanView* view, std::filesystem::path root)
        : view_(view), root_(std::move(root)), renderer_(root_) {}

    GfxOceanGradient::~GfxOceanGradient() = default;

    void GfxOceanGradient::SetZone(GradientZone zone) {
        zone_ = zone;
    }

    void GfxOceanGradient::InitWaves() {
        if (loaded_)
            return;
        loaded_ = true;
        try {
            waves_ = sys::res::ResourceAEAnim::Create(root_ / "xml_bin/anim_water_peak.bin",
                                                      sys::res::AEFormat::RaftPirates2013);
        } catch (const std::exception&) {
        }
    }

    void GfxOceanGradient::ComputeWorldState() {
        InitWaves();
    }

    void GfxOceanGradient::tick(float dt) {
        if (std::isfinite(dt) && dt > 0)
            time_ = std::fmod(time_ + std::min(dt, 60.f), 3600.f);
    }

    void GfxOceanGradient::Render(Image& dst) {
        // Six RGB triplets from GRADIENTCOLOURS (ARM address 0xa60a74).
        // Zone ordering and interpolation direction are portable interpretations.
        static constexpr Color palette[][2] = {{{83, 183, 172, 255}, {56, 144, 134, 255}},
                                               {{10, 181, 193, 255}, {0, 124, 147, 255}},
                                               {{45, 45, 45, 255}, {149, 24, 0, 255}}};
        const auto& colors = palette[static_cast<unsigned>(zone_)];
        for (int y = 0; y < dst.height; ++y) {
            const float t = float(y) / std::max(1, dst.height - 1);
            auto blend = [&](uint8_t a, uint8_t b) {
                return uint8_t(std::lround(a + (b - a) * t));
            };
            const Color c{blend(colors[0].r, colors[1].r), blend(colors[0].g, colors[1].g),
                          blend(colors[0].b, colors[1].b), 255};
            std::fill_n(dst.pixels.begin() + size_t(y) * dst.width, dst.width, c);
        }
        ComputeWorldState();
        if (!waves_ || waves_->comps.empty() || !view_)
            return;
        const auto& comp = waves_->comps.front();
        if (comp.duration() <= 0)
            return;
        GfxManager mgr;
        // Stable, sparse wave placement; original wave density and camera rules unresolved.
        for (int i = 0; i < 5; ++i) {
            const float x = std::fmod(83.f + i * 151.f + time_ * 4, float(std::max(1, dst.width)));
            const float y = 40.f + i * 73.f;
            Gfx appearance;
            appearance.SetAlpha(100);
            renderer_.submit(mgr, *waves_, comp, std::fmod(time_ + i * .37f, comp.duration()),
                             Matrix::translation(x, y) * Matrix::scale(.45f, .45f), 0, &appearance);
        }
        mgr.RenderScreen(dst);
    }
} // namespace rp
