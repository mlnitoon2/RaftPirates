#pragma once
#include "sys/menu_redux/MenuRuntime.h"
#include "sys/localization/LocalizationManager.h"

namespace rp {
    class TitleScreen {
      public:
        explicit TitleScreen(const std::filesystem::path& assets);
        void touchDown(float x, float y);
        void touchUp(float x, float y);
        void touchDrag(float x, float y);

        bool started() const {
            return started_;
        }

        sys::gfx::Image render();

      private:
        sys::localization::LocalizationManager strings_;
        sys::menu_redux::MenuRuntime menu_;
        bool started_ = false;
        bool event(unsigned id, const std::string& name);
    };
} // namespace rp
