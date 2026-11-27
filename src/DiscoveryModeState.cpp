#include "rp/DiscoveryModeState.h"
#include <algorithm>
#include <cmath>

namespace rp {
    DiscoveryModeState::DiscoveryModeState(const Game& game, const std::filesystem::path& root)
        : renderer_(root) {
        auto i = game.discoveries.find(game.discovery);
        title_ = i == game.discoveries.end() ? "Discovery" : i->second;
        if (title_.find("Barnacle Bill") != std::string::npos) {
            try {
                resource_ =
                    std::make_shared<sys::res::ResourceAEAnim>(sys::res::ResourceAEAnim::Create(
                        root / "xml_bin/anim_location_barnacle_bill.bin"));
            } catch (const std::exception&) {
            }
        }
    }

    DiscoveryModeState::~DiscoveryModeState() = default;

    void DiscoveryModeState::tick(float dt) {
        if (!paused_ && std::isfinite(dt) && dt > 0)
            time_ += dt;
    }

    void DiscoveryModeState::Pause() {
        paused_ = !paused_;
    }

    void DiscoveryModeState::LeaveDiscovery(Game& game) {
        leaving_ = true;
        LeaveDiscoveryReally(game);
    }

    void DiscoveryModeState::LeaveDiscoveryReally(Game& game) {
        game.discovery = 0;
    }

    void DiscoveryModeState::GotMsgTransitionDone(Game& game) {
        if (leaving_)
            LeaveDiscoveryReally(game);
    }

    void DiscoveryModeState::HandleExtensionResponse(Game& game, const std::string& command,
                                                     const sfs::SFSObjectWrapper& data) {
        game.extension(command, data);
        if (!game.discovery)
            leaving_ = true;
    }

    bool DiscoveryModeState::leaving() const {
        return leaving_;
    }

    const std::string& DiscoveryModeState::title() const {
        return title_;
    }

    void DiscoveryModeState::render(sys::gfx::Image& out) {
        if (!resource_ || resource_->comps.empty())
            return;
        const auto* selected = &resource_->comps.front();
        for (const auto& entry : resource_->comps)
            if (entry.name == "idle" || entry.name == "Idle") {
                selected = &entry;
                break;
            }
        const auto& c = *selected;
        sys::gfx::GfxManager mgr;
        float sc = std::min(1.f, 340.f / std::max(1, c.width));
        auto m = sys::gfx::Matrix::translation(320 - c.width * sc / 2, 180 - c.height * sc / 2) *
                 sys::gfx::Matrix::scale(sc, sc);
        renderer_.submit(mgr, *resource_, c, c.duration() > 0 ? std::fmod(time_, c.duration()) : 0,
                         m);
        mgr.RenderScreen(out);
    }
} // namespace rp
