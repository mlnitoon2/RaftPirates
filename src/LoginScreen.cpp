#include "rp/LoginScreen.h"
#include <regex>
#include <algorithm>

namespace rp {
    using sys::menu_redux::MenuRuntime;

    LoginScreen::LoginScreen(const std::filesystem::path& root) : root_(root), loading_(root) {
        strings_.loadFile(root / "text/en.utf8");
        loading_.nativeEvent = [this](auto& m, unsigned id, const auto& name) {
            auto& n = m.node(id);
            if (name == "onInit" && n.name == "Anim") {
                n.set("animationName", std::string("xml_bin/anim_loading_curtains.bin"));
                n.set("animation", std::string("intro"));
                // The 480x320 source composition fills the 640x400 viewport.
                n.set("size", 640.0 / 480.0);
                n.set("absoluteX", 0.0);
                n.set("absoluteY", (400.0 - 320.0 * (640.0 / 480.0)) / 2.0);
                return true;
            }
            if (name == "onAnimationFinished" && n.name == "Anim") {
                if (finishing_)
                    finished_ = true;
                else if (!popup_ && !busy_) {
                    n.set("animation", std::string("idle 2"));
                    n.set("loop", true);
                    next_ = "login_menu";
                }
                return true;
            }
            return true;
        };
        loading_.load(root / "menus/loading_intro.xml");
        loading_.resize(640, 400);
        loading_.initialize();
    }

    void LoginScreen::show(const std::string& name) {
        screen_ = name;
        popup_ = std::make_unique<Menu>(root_);
        popup_->nativeEvent = [this](auto& m, unsigned id, const auto& e) {
            return event(m, id, e);
        };
        popup_->load(root_ / "menus" / (name + ".xml"));
        popup_->resize(640, 400);
        popup_->initialize();
        refresh();
    }

    void LoginScreen::apply() {
        if (!next_.empty()) {
            auto n = std::move(next_);
            next_.clear();
            show(n);
        }
    }

    bool LoginScreen::event(Menu& m, unsigned id, const std::string& e) {
        auto& n = m.node(id);
        auto owner = m.owner(id);
        auto parent = m.node(owner).name;
        if (e == "onInit") {
            // Interpret only literal component assignments in the recovered XML. Game calls are
            // explicit below.
            if (!n.isElement()) {
                const auto& body = n.events.at(e).source;
                static const std::regex str(
                    R"rx(component(?:\("([^"]+)"\)|:GetVar\("([^"]+)"\))\s*:Set(?:String)?\(\s*"([^"]*)"\s*\))rx");
                for (auto i = std::sregex_iterator(body.begin(), body.end(), str);
                     i != std::sregex_iterator(); ++i) {
                    auto key = (*i)[1].matched ? (*i)[1].str() : (*i)[2].str();
                    auto val = (*i)[3].str();
                    if (key == "text")
                        val = strings_.getText(val);
                    n.set(key, val);
                }
                static const std::regex num(
                    R"rx(component(?:\("([^"]+)"\)|:GetVar\("([^"]+)"\))\s*:Set(?:Int|Float)?\(\s*(-?[0-9]+(?:\.[0-9]+)?)\s*\))rx");
                for (auto i = std::sregex_iterator(body.begin(), body.end(), num);
                     i != std::sregex_iterator(); ++i)
                    n.set((*i)[1].matched ? (*i)[1].str() : (*i)[2].str(), std::stod((*i)[3]));
                if (n.type == "textComponent") {
                    n.set("alignment", 1.0);
                    n.set("priority", -10.0);
#ifndef HYDRA_HAS_FREETYPE
                    if (n.text("font") == "blue_highway")
                        n.set("font", std::string("rp_font_title"));
#endif
                    if (n.text("font") == "blue_highway")
                        n.set("size", .85);
                }
                if (n.type == "spriteComponent" || n.type == "spriteSheetComponent")
                    n.set("priority", 0.0);
                if (n.type == "ninesliceComponent" && parent == "bg") {
                    n.set("priority", 50.0);
                    n.set("width", 640 * .66);
                    n.set("height", screen_ == "status_popup" ? 110. : 400 * .66);
                }
            }
            return true;
        }
        if (n.name == "Touch" &&
            (e == "onTouchDown" || e == "onTouchRelease" || e == "onTouchUp")) {
            bool down = e == "onTouchDown";
            if (auto up = m.find(owner, "UpSprite"))
                m.node(up).set("visible", !down);
            if (auto over = m.find(owner, "DownSprite"))
                m.node(over).set("visible", down);
            if (e != "onTouchUp")
                return true;
            if (parent == "EmailButton")
                next_ = "login_email";
            else if (parent == "FacebookButton")
                action_ = Action::Facebook;
            else if (parent == "EmailInput")
                action_ = Action::Email;
            else if (parent == "PasswordInput")
                action_ = Action::Password;
            else if (parent == "Password2Input")
                action_ = Action::ConfirmPassword;
            else if (parent == "password_forgot")
                action_ = Action::Reset;
            else if (parent == "RegisterButton") {
                clearPassword();
                email_.clear();
                next_ = "login_registration";
            } else if (parent == "BackButton") {
                clearPassword();
                next_ = screen_ == "login_registration" ? "login_email" : "login_menu";
            } else if (parent == "SubmitButton")
                action_ = screen_ == "login_registration" ? Action::Register : Action::Submit;
            else if (parent == "OK")
                next_ = previous_.empty() ? "login_menu" : previous_;
            return true;
        }
        return true;
    }

    void LoginScreen::refresh() {
        if (!popup_)
            return;
        for (auto& n : popup_->nodes()) {
            if (n.type != "textComponent")
                continue;
            auto p = popup_->node(n.parent).name;
            auto& t = popup_->node(n.id);
            if (p == "EmailInput")
                t.set("text", email_.empty() ? strings_.getText("EMAIL_INPUT_LABEL") : email_);
            else if (p == "PasswordInput" || p == "Password2Input") {
                auto& v = p == "PasswordInput" ? password_ : confirmation_;
                t.set("text", v.empty() ? strings_.getText(p == "PasswordInput"
                                                               ? "PASSWORD_INPUT_LABEL"
                                                               : "PASSWORD_CONFIRM_INPUT_LABEL")
                                        : v);
                t.set("hideText", !v.empty());
            } else if (screen_ == "status_popup" && n.name == "Text" && p == "topElement")
                t.set("text", message_);
        }
        popup_->solve();
    }

    void LoginScreen::tick(float dt) {
        elapsed_ += dt;
        loading_.tick(dt);
        if (!popup_ && !busy_ && !finishing_ && elapsed_ > 4.f) {
            auto id = loading_.find(loading_.root(), "Anim");
            if (id) {
                loading_.node(id).set("animation", std::string("idle 2"));
                loading_.node(id).set("loop", true);
            }
            next_ = "login_menu";
        }
        if (finishing_ && elapsed_ > 4.f)
            finished_ = true;
        apply();
        if (popup_)
            popup_->tick(dt);
    }

    void LoginScreen::touchDown(float x, float y) {
        if (popup_ && !busy_)
            popup_->touchDown(x, y);
    }

    void LoginScreen::cancelInput() {
        if (popup_)
            popup_->touchCancel();
        action_ = Action::None;
    }

    void LoginScreen::touchDrag(float x, float y) {
        if (popup_ && !busy_)
            popup_->touchDrag(x, y);
    }

    LoginScreen::Action LoginScreen::touchUp(float x, float y) {
        action_ = Action::None;
        if (popup_ && !busy_)
            popup_->touchUp(x, y);
        apply();
        return action_;
    }

    void LoginScreen::setInput(Action a, std::string v) {
        if (a == Action::Email)
            email_ = std::move(v);
        else if (a == Action::Password)
            password_ = std::move(v);
        else if (a == Action::ConfirmPassword)
            confirmation_ = std::move(v);
        refresh();
    }

    void LoginScreen::clearPassword() {
        password_.clear();
        confirmation_.clear();
        refresh();
    }

    void LoginScreen::busy(bool b) {
        busy_ = b;
        if (b) {
            previous_ = screen_;
            popup_.reset();
            screen_.clear();
            auto id = loading_.find(loading_.root(), "Anim");
            if (id) {
                loading_.node(id).set("animation", std::string("idle"));
                loading_.node(id).set("loop", true);
            }
        }
    }

    void LoginScreen::registrationComplete() {
        screen_ = "login_email";
        error("Registration accepted. Please log in.");
    }

    void LoginScreen::finish() {
        next_.clear();
        finished_ = false;
        busy_ = true;
        finishing_ = true;
        elapsed_ = 0;
        popup_.reset();
        auto id = loading_.find(loading_.root(), "Anim");
        if (id) {
            loading_.node(id).set("animation", std::string("outro"));
            loading_.node(id).set("loop", false);
        }
    }

    void LoginScreen::error(std::string m) {
        next_.clear();
        busy_ = false;
        finishing_ = false;
        finished_ = false;
        if (!screen_.empty() && screen_ != "status_popup")
            previous_ = screen_;
        if (previous_.empty())
            previous_ = "login_email";
        auto id = loading_.find(loading_.root(), "Anim");
        if (id) {
            loading_.node(id).set("animation", std::string("idle 2"));
            loading_.node(id).set("loop", true);
        }
        message_ = std::move(m);
        show("status_popup");
    }

    sys::gfx::Image LoginScreen::render(const sys::gfx::Image* background) {
        auto im = finishing_ && background && background->width == 640 && background->height == 400
            ? *background : sys::gfx::Image(640, 400, {0, 0, 0, 255});
        sys::gfx::GfxManager g;
        loading_.submit(g);
        g.RenderScreen(im);
        if (popup_) {
            sys::gfx::GfxManager p;
            popup_->submit(p);
            p.RenderScreen(im);
        }
        return im;
    }

    sys::menu_redux::Rect LoginScreen::bounds(const std::string& s) const {
        if (!popup_)
            return {};
        auto id = popup_->find(popup_->root(), s);
        return id ? popup_->node(id).bounds : sys::menu_redux::Rect{};
    }
} // namespace rp
