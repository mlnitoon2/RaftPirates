#pragma once
#include "rp/Game.h"
#include "sys/menu_redux/MenuRuntime.h"
#include "sys/localization/LocalizationManager.h"

namespace rp {
    // Original XML layout, with a native adapter for recovered normal-HUD scripts.
    class Hud {
      public:
        enum class Action {
            None,
            Quest,
            Safety,
            Daring,
            ToggleView,
            Shop,
            Map,
            Move,
            Destroy,
            Repair,
            Salvage,
            Clock,
            Build,
            Cancel
        };
        void setSelectedActions(const std::vector<std::string>& actions, int clocks);
        explicit Hud(const std::filesystem::path& assets);
        void update(const Game& game);
        void render(sys::gfx::Image& image);
        Action touch(float x, float y);
        sys::menu_redux::Rect bounds(const std::string& name) const;

      private:
        sys::menu_redux::MenuRuntime menu_;
        sys::localization::LocalizationManager strings_;
        unsigned normal_ = 0;
        Action action_ = Action::None;
        void show(const std::string& name, bool visible);
        unsigned part(const std::string& owner, const std::string& component) const;
        bool event(unsigned id, const std::string& name);
    };
} // namespace rp
