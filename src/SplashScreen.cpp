#include "rp/SplashScreen.h"
#include <cmath>

namespace rp {
    using sys::menu_redux::MenuRuntime;

    SplashScreen::SplashScreen(const std::filesystem::path& assets) : menu_(assets) {
        menu_.nativeEvent = [this](MenuRuntime& menu, unsigned id, const std::string& event) {
            auto& node = menu.node(id);
            if (event == "onInit") {
                if (node.name == "splash")
                    node.set("splashFinished", 0.0);
                else if (node.name == "blackout") {
                    // The supplied APK lacks gfx/black_bg; render() clears to black instead.
                    node.set("visible", false);
                    node.set("width", 640.0);
                    node.set("height", 400.0);
                } else if (node.name == "bbb_logo") {
                    node.set("animationName", std::string("xml_bin/bbb_splash.bin"));
                    node.set("width", 480.0);
                    node.set("height", 320.0);
                    // Center the 480x320 composition in the 640x400 research viewport.
                    node.set("xOffset", -240.0);
                    node.set("yOffset", -160.0);
                }
                return true;
            }
            if (event == "onAnimationFinished" && node.name == "bbb_logo") {
                menu.node(menu.owner(id)).set("splashFinished", 1.0);
                finished_ = true;
                return true;
            }
            if (event == "onTouchUp") {
                skip();
                return true;
            }
            return false;
        };
        menu_.load(assets / "menus/bbb_splash_menu.xml");
        menu_.resize(640, 400);
        menu_.initialize();
    }

    void SplashScreen::tick(float dt) {
        if (finished_ || !std::isfinite(dt) || dt <= 0)
            return;
        elapsed_ += dt;
        menu_.tick(dt);
        // A corrupt or missing animation cannot trap the player in the splash.
        if (elapsed_ >= 4.0f)
            finished_ = true;
    }

    void SplashScreen::skip() {
        finished_ = true;
    }

    sys::gfx::Image SplashScreen::render() {
        sys::gfx::Image image(640, 400, {0, 0, 0, 255});
        sys::gfx::GfxManager gfx;
        menu_.submit(gfx);
        gfx.RenderScreen(image);
        return image;
    }
} // namespace rp
