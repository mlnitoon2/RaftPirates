#include "rp/Hud.h"
#include <algorithm>
#include <iomanip>
#include <sstream>
#include <stdexcept>

namespace rp {
    using namespace sys::menu_redux;

    namespace {
        std::string timeString(int seconds) {
            seconds = std::max(0, seconds);
            std::ostringstream text;
            text << seconds / 60 << ':' << std::setw(2) << std::setfill('0') << seconds % 60;
            return text.str();
        }
    } // namespace

    Hud::Hud(const std::filesystem::path& assets) : menu_(assets) {
        strings_.loadFile(assets / "text/en.utf8");
        menu_.nativeEvent = [this](auto&, unsigned id, const std::string& name) {
            return event(id, name);
        };
        menu_.load(assets / "menus/hud.xml");
        normal_ = menu_.find(1, "NormalHud", true);
        if (!normal_)
            throw std::runtime_error("Original NormalHud not found");
        menu_.resize(640, 400);
        menu_.initialize();
        for (const auto& name :
             {"TutorialArrow", "ObjectSelectionOptions", "InstructionText", "DivingHud"}) {
            if (auto id = menu_.find(1, name, true))
                menu_.node(id).set("visible", false);
        }
        for (const auto& name : {"CheatMenu", "LowWater"})
            show(name, false);
        // Recovered script offsets, adapted to keep controls within the 640x400 research viewport.
        auto offset = [&](const char* name, double x, double y) {
            auto& n = menu_.node(menu_.find(normal_, name, true));
            n.set("xOffset", x);
            n.set("yOffset", y);
        };
        offset("QuestMenu", 34, 25);
        offset("MainMenu", 34, -25);
        offset("PirateMap", -3, -3);
        offset("SafeFlag", -130, 2);
        offset("BadZoneFlag", -130, 2);
        offset("EventNotification", 130, 27);
        menu_.solve();
    }

    unsigned Hud::part(const std::string& owner, const std::string& component) const {
        auto id = menu_.find(normal_, owner, true);
        return id ? menu_.find(id, component) : 0;
    }

    void Hud::show(const std::string& name, bool visible) {
        if (auto id = menu_.find(normal_, name, true))
            menu_.node(id).set("visible", visible);
    }

    bool Hud::event(unsigned id, const std::string& name) {
        auto& n = menu_.node(id);
        auto ownerName = menu_.node(menu_.owner(id)).name;
        if (name == "onInit" && n.type == "spriteSheetComponent") {
            std::string sprite;
            if (ownerName == "QuestMenu")
                sprite = n.name == "IsNew" ? "notification_new" : "btn_challenges";
            else if (ownerName == "SafeFlag")
                sprite = "notification_safe_timer";
            else if (ownerName == "BadZoneFlag" || ownerName == "LowWater")
                sprite = "notification_low_water";
            else if (ownerName == "EventNotification")
                sprite = n.name == "ViewToggleSprite" ? "btn_toggle" : "notification_bird";
            else if (ownerName == "MainMenu")
                sprite = "btn_mainmenu";
            else if (ownerName == "PirateMap")
                sprite = "btn_quicktravel";
            else if (ownerName == "DaringMeter") {
                if (n.name == "Sprite")
                    sprite = "menu_daring_bg";
                else if (n.name == "Fill")
                    sprite = "menu_daring_fill";
                else if (n.name == "OpenChest")
                    sprite = "menu_daring_open";
                else if (n.name == "ClosedChest")
                    sprite = "menu_daring_closed";
                else if (n.name == "Lock")
                    sprite = "menu_daring_lock";
            }
            if (!sprite.empty()) {
                n.set("spriteName", sprite);
                n.set("sheetName", std::string("xml_resources/rp_menu.xml"));
            }
            n.set("visible", !sprite.empty() && n.name != "IsNew" && n.name != "OpenChest" &&
                                 n.name != "ClosedChest" && n.name != "Lock" &&
                                 !(ownerName == "EventNotification" && n.name == "Sprite"));
        } else if (name == "onInit" && n.type == "textComponent" &&
                   (ownerName == "DaringMeter" || ownerName == "SafeFlag")) {
            n.set("font", std::string("rp_font_title"));
            n.set("size", n.name == "TextDisplayLevel" ? .5 : .7);
            n.set("alignment", 1.0);
            n.set("multiline", false);
            n.set("visible", ownerName == "DaringMeter");
        } else if (name == "onInit" && n.type == "aeComponent") {
            n.set("visible", false);
        } else if (name == "onTouchUp" && n.type == "touchComponent") {
            if (ownerName == "QuestMenu")
                action_ = Action::Quest;
            else if (ownerName == "SafeFlag" || ownerName == "BadZoneFlag")
                action_ = Action::Safety;
            else if (ownerName == "DaringMeter")
                action_ = Action::Daring;
            else if (ownerName == "EventNotification")
                action_ = Action::ToggleView;
            else if (ownerName == "MainMenu")
                action_ = Action::Shop;
            else if (ownerName == "PirateMap")
                action_ = Action::Map;
        }
        // Original Lua bodies depend on unrecovered game bindings. Do not run partially.
        return true;
    }

    void Hud::update(const Game& game) {
        show("SafeFlag", game.safe && !game.badZone);
        show("BadZoneFlag", game.badZone);
        auto set = [&](const char* owner, const char* component, const char* key, Property value) {
            if (auto id = part(owner, component))
                menu_.node(id).set(key, std::move(value));
        };
        set("QuestMenu", "IsNew", "visible", game.questCount > 0);
        const bool locked = game.daringLocked();
        const bool ready = game.canAdvanceDaring();
        set("DaringMeter", "Fill", "maskWidth", 100.0 * game.daringProgress());
        set("DaringMeter", "TextDisplay", "text", timeString(game.daringRemaining));
        set("DaringMeter", "TextDisplay", "alpha", locked ? 0.0 : 1.0);
        set("DaringMeter", "TextDisplayLevel", "text",
            strings_.getText("TXT_LEVEL") + std::to_string(game.daring));
        for (const auto* component : {"Sprite", "TextDisplayLevel", "OpenChest", "ClosedChest"})
            for (const auto* channel : {"red", "green", "blue"})
                set("DaringMeter", component, channel, locked ? .5 : 1.0);
        set("DaringMeter", "OpenChest", "visible", ready);
        set("DaringMeter", "ClosedChest", "visible", !ready);
        set("DaringMeter", "Lock", "visible", locked);
        menu_.solve();
    }

    void Hud::render(sys::gfx::Image& image) {
        sys::gfx::GfxManager renderer;
        menu_.submit(renderer);
        renderer.RenderScreen(image);
    }

    Hud::Action Hud::touch(float x, float y) {
        action_ = Action::None;
        menu_.touchDown(x, y);
        menu_.touchUp(x, y);
        return action_;
    }

    Rect Hud::bounds(const std::string& name) const {
        auto id = menu_.find(normal_, name, true);
        return id ? menu_.node(id).bounds : Rect{};
    }
} // namespace rp
