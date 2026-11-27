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
        menu_.reuseUnchangedLayout = true;
        menu_.resize(640, 400);
        menu_.initialize();
        for (const auto& name :
             {"TutorialArrow", "ObjectSelectionOptions", "InstructionText", "DivingHud"}) {
            if (auto id = menu_.find(1, name, true))
                menu_.node(id).set("visible", false);
        }
        for (const auto& name : {"LowWater"})
            show(name, false);
        // Apply the original onPostInit offsets using the actual atlas bounds.
        auto offset = [&](const char* name, double x, double y) {
            auto& n = menu_.node(menu_.find(normal_, name, true));
            n.set("xOffset", x);
            n.set("yOffset", y);
        };
        auto quest = bounds("QuestMenu"), main = bounds("MainMenu"),
             event = bounds("EventNotification");
        offset("QuestMenu", quest.w / 2, quest.h / 2);
        offset("MainMenu", main.w / 2 - 3, -main.h / 2 - 3);
        offset("PirateMap", -3, -3);
        auto daring = bounds("DaringMeter");
        double half = daring.w / 2;
        double safeX = (640.0 / 2 - half - quest.w) / 2 + half;
        offset("SafeFlag", -safeX, 2);
        offset("BadZoneFlag", -safeX - 40, 2);
        offset("EventNotification", (640.0 / 2 - half - main.w) / 2 + half, event.h / 2);
        menu_.solve();
        // Bind normal HUD touch rectangles to the rendered icons.
        for (auto name : {"QuestMenu", "MainMenu", "PirateMap", "EventNotification", "DaringMeter",
                          "SafeFlag", "BadZoneFlag", "CheatMenu"}) {
            auto owner = menu_.find(normal_, name, true);
            if (!owner)
                continue;
            auto visual = menu_.find(owner, "Sprite");
            auto touch = menu_.find(owner, "Touch");
            if (!visual || !touch)
                continue;
            const auto b = menu_.node(visual).bounds;
            if (b.w <= 0 || b.h <= 0)
                continue;
            auto& n = menu_.node(touch);
            n.set("absoluteX", double(b.x));
            n.set("absoluteY", double(b.y));
            n.set("width", double(b.w));
            n.set("height", double(b.h));
            n.set("layer", std::string("HUD"));
        }
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
        if (name == "onInit")
            menu_.applyLiteralProperties(id, [this](const auto& s) { return strings_.getText(s); });
        auto options = menu_.find(1, "ObjectSelectionOptions", true);
        for (auto parent = id; parent; parent = menu_.node(parent).parent) {
            if (parent != options)
                continue;
            if (name == "onTouchUp" && n.type == "touchComponent") {
                auto button = menu_.node(menu_.owner(id)).parent;
                auto action = button ? menu_.node(button).text("action") : "";
                if (action == "move")
                    action_ = Action::Move;
                else if (action == "destroy")
                    action_ = Action::Destroy;
                else if (action == "repair")
                    action_ = Action::Repair;
                else if (action == "salvage")
                    action_ = Action::Salvage;
                else if (action == "fast_clock")
                    action_ = Action::Clock;
                else if (action == "build")
                    action_ = Action::Build;
                else if (action == "build_cancel")
                    action_ = Action::Cancel;
            }
            return true;
        }
        if (name == "onInit" && n.type == "spriteSheetComponent") {
            std::string sprite;
            if (ownerName == "QuestMenu")
                sprite = n.name == "IsNew" ? "notification_new" : "btn_challenges";
            else if (ownerName == "CheatMenu")
                sprite = "THE_BIG_BLUE";
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
            else if (ownerName == "CheatMenu")
                action_ = Action::Admin;
            else if (ownerName == "MainMenu")
                action_ = Action::Shop;
            else if (ownerName == "PirateMap")
                action_ = Action::Map;
        }
        // Original Lua bodies depend on unrecovered game bindings. Do not run partially.
        return true;
    }

    void Hud::setSelectedActions(const std::vector<std::string>& actions, int clocks) {
        auto options = menu_.find(1, "ObjectSelectionOptions", true);
        if (!options)
            return;
        menu_.node(normal_).set("visible", actions.empty());
        menu_.node(options).set("visible", !actions.empty());
        if (actions.empty())
            return;
        const std::map<std::string, std::string> images{
            {"move", "btn_move"},        {"destroy", "btn_dismantle"},
            {"repair", "btn_repair"},    {"salvage", "rp_challenge_salvage"},
            {"fast_clock", "btn_clock"}, {"build", "btn_yes"},
            {"build_cancel", "btn_no"}};
        for (size_t i = 0; i < 6; ++i) {
            auto id = menu_.find(options, "Button" + std::to_string(i + 1), true);
            if (!id)
                continue;
            bool showButton = i < actions.size() && images.count(actions[i]);
            menu_.node(id).set("visible", showButton);
            if (!showButton)
                continue;
            menu_.node(id).set("action", actions[i]);
            auto button = menu_.find(id, "Button", true);
            if (!button)
                continue;
            auto set = [&](const char* child, const char* key, Property value) {
                if (auto c = menu_.find(button, child))
                    menu_.node(c).set(key, std::move(value));
            };
            std::string color = actions[i] == "build_cancel" ? "btn_red" : "btn_green";
            set("UpSprite", "spriteName", color);
            set("UpSprite", "visible", true);
            set("DownSprite", "spriteName", color + "_over");
            set("DownSprite", "visible", false);
            set("Sprite", "spriteName", images.at(actions[i]));
            set("Sprite", "sheetName", std::string("xml_resources/rp_menu.xml"));
            set("Sprite", "visible", true);
            set("Text", "text",
                actions[i] == "fast_clock" ? std::to_string(clocks) : std::string{});
            set("Touch", "enabled", actions[i] != "fast_clock" || clocks > 0);
        }
        if (auto bg = menu_.find(options, "Bg", true))
            if (auto sprite = menu_.find(bg, "Sprite"))
                menu_.node(sprite).set("visible", true);
        if (auto gears = menu_.find(options, "Gears", true))
            menu_.node(gears).set("visible", actions.size() == 1 && actions.front() == "wait");
        if (auto instruction = menu_.find(options, "InstructionText", true))
            menu_.node(instruction).set("visible", false);
        menu_.solve();
        const float total = float(actions.size()) * 54.f;
        for (size_t i = 0; i < std::min<size_t>(6, actions.size()); ++i) {
            auto id = menu_.find(options, "Button" + std::to_string(i + 1), true);
            if (!id)
                continue;
            auto& n = menu_.node(id);
            n.set("absoluteX", double((640 - total) * .5f + i * 54));
            n.set("absoluteY", 340.0);
        }
        menu_.solve();
    }

    void Hud::update(const Game& game) {
        show("CheatMenu", admin_);
        show("SafeFlag", game.safe && !game.badZone);
        show("BadZoneFlag", game.badZone);
        auto set = [&](const char* owner, const char* component, const char* key, Property value) {
            if (auto id = part(owner, component))
                menu_.node(id).set(key, std::move(value));
        };
        set("QuestMenu", "IsNew", "visible",
            game.quests.empty() ? game.questCount > 0
                                : std::any_of(game.quests.begin(), game.quests.end(),
                                              [](const auto& q) { return q.second.unread; }));
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
