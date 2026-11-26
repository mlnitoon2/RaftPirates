#pragma once
#include "rp/Game.h"

namespace rp {
    class DiscoveryModeState {
      public:
        DiscoveryModeState(const Game& game, const std::filesystem::path& root);
        ~DiscoveryModeState();
        void tick(float dt);
        void Pause();
        void LeaveDiscovery(Game& game);
        void LeaveDiscoveryReally(Game& game);
        bool leaving() const;
        const std::string& title() const;
        void render(sys::gfx::Image& out);

      private:
        sys::gfx::AnimationRenderer renderer_;
        std::shared_ptr<sys::res::ResourceAEAnim> resource_;
        std::string title_;
        float time_ = 0;
        bool paused_ = false, leaving_ = false;
    };
} // namespace rp
