#include "rp/TitleScreen.h"
#include <algorithm>

namespace rp {
    TitleScreen::TitleScreen(const std::filesystem::path& assets, std::string bbbId) : menu_(assets), bbbId_(std::move(bbbId)) {
        strings_.loadFile(assets / "text/en.utf8");
        menu_.nativeEvent = [this](auto&, unsigned id, const std::string& name) {
            return event(id, name);
        };
        menu_.load(assets / "menus/title.xml");
        menu_.resize(640, 400);
        menu_.initialize();
    }

    bool TitleScreen::event(unsigned id, const std::string& name) {
        auto& n = menu_.node(id);
        auto owner = menu_.owner(id);
        auto parentName = menu_.node(owner).name;
        if (name == "onInit") {
            if (parentName == "bg" && n.name == "Sprite") {
                n.set("spriteName", std::string("gfx/menu/menu_overlay"));
                n.set("width", 640.0);
                n.set("height", 400.0);
            } else if (parentName == "TitleImg" && n.name == "Sprite") {
                n.set("spriteName", std::string("gfx/menu/title_raft_pirates"));
                menu_.node(owner).set("yOffset", -25.0);
            } else if (parentName == "version" && n.name == "Text") {
                n.set("font", std::string("rp_font_title"));
                n.set("size", .5);
                n.set("alignment", 1.0);
                n.set("text", std::string("1.0.4   (BBB ID: " + bbbId_ + ")"));
                n.set("red", .988);
                n.set("green", .991);
                n.set("blue", .129);
            } else if (parentName == "Start") {
                if (n.name == "UpSprite" || n.name == "DownSprite") {
                    bool up = n.name == "UpSprite";
                    n.set("sheetName", std::string("xml_resources/rp_menu.xml"));
                    n.set("spriteName", std::string(up ? "btn_set_sail" : "btn_set_sail_over"));
                    n.set("visible", up);
                } else if (n.name == "Text") {
                    n.set("font", std::string("rp_font_title"));
                    n.set("size", .7);
                    n.set("alignment", 1.0);
                    n.set("multiline", false);
                    n.set("red", .988);
                    n.set("green", .991);
                    n.set("blue", .129);
                    auto label = strings_.getText("OPENING_SCREEN");
                    n.set("text", label == "OPENING_SCREEN" ? std::string("Set Sail!") : label);
                    n.set("autoScale", true);
                    n.set("width", 130.0);
                }
            }
            return true;
        }
        if (parentName == "Start" && n.name == "Touch") {
            if (name == "onTouchDown" || name == "onTouchUp" || name == "onTouchRelease") {
                bool down = name == "onTouchDown";
                if (auto up = menu_.find(owner, "UpSprite"))
                    menu_.node(up).set("visible", !down);
                if (auto over = menu_.find(owner, "DownSprite"))
                    menu_.node(over).set("visible", down);
                if (name == "onTouchUp")
                    started_ = true;
                return true;
            }
        }
        return false;
    }

    void TitleScreen::touchDown(float x, float y) {
        menu_.touchDown(x, y);
    }

    void TitleScreen::touchUp(float x, float y) {
        menu_.touchUp(x, y);
    }

    void TitleScreen::touchDrag(float x, float y) {
        menu_.touchDrag(x, y);
    }

    sys::gfx::Image TitleScreen::render() {
        sys::gfx::Image image(640, 400, {36, 133, 168, 255});
        sys::gfx::GfxManager gfx;
        menu_.submit(gfx);
        gfx.RenderScreen(image);
        return image;
    }
} // namespace rp
