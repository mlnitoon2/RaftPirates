#pragma once
#include "sys/menu_redux/MenuRuntime.h"

namespace rp {
    class SplashScreen {
      public:
        explicit SplashScreen(const std::filesystem::path& assets);
        void tick(float dt);
        void skip();

        bool finished() const {
            return finished_;
        }

        sys::gfx::Image render();

      private:
        sys::menu_redux::MenuRuntime menu_;
        float elapsed_ = 0;
        bool finished_ = false;
    };
} // namespace rp
