#pragma once
#include "sys/menu_redux/MenuRuntime.h"
#include "sys/localization/LocalizationManager.h"
#include <memory>

namespace rp {
    class LoginScreen {
      public:
        enum class Action {
            None,
            Email,
            Password,
            ConfirmPassword,
            Submit,
            Register,
            Facebook,
            Reset
        };
        explicit LoginScreen(const std::filesystem::path& root);
        void tick(float dt);
        void touchDown(float x, float y);
        void touchDrag(float x, float y);
        void cancelInput();
        Action touchUp(float x, float y);
        void setInput(Action, std::string);

        const std::string& email() const {
            return email_;
        }

        const std::string& password() const {
            return password_;
        }

        const std::string& confirmation() const {
            return confirmation_;
        }

        void clearPassword();
        void returnToMenu();
        void registrationComplete();
        void busy(bool);
        void finish();

        bool finished() const {
            return finished_;
        }

        void error(std::string);
        sys::gfx::Image render(const sys::gfx::Image* background = nullptr);
        sys::menu_redux::Rect bounds(const std::string&) const;

        std::string screen() const {
            return screen_;
        }

      private:
        using Menu = sys::menu_redux::MenuRuntime;
        std::filesystem::path root_;
        sys::localization::LocalizationManager strings_;
        Menu loading_;
        std::unique_ptr<Menu> popup_;
        std::string screen_, next_, email_, password_, confirmation_, message_, previous_;
        Action action_ = Action::None;
        bool busy_ = false, finishing_ = false, finished_ = false;
        float elapsed_ = 0;
        void show(const std::string&);
        void apply();
        void refresh();
        bool event(Menu&, unsigned, const std::string&);
    };
} // namespace rp
