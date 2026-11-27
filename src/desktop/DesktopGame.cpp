#include "sys/platform/NativeHost.h"
#include "sys/platform/Services.h"
#include <SDL.h>
#include <algorithm>
#include <iostream>
#include <optional>
#include <stdexcept>

namespace {
    using namespace sys::platform;
    namespace game = sys::platform::game;
    namespace fs = std::filesystem;

    class DesktopServices final : public Services {
      public:
        explicit DesktopServices(const fs::path& root)
            : root_(fs::absolute(root)), host_("Raft Pirates", 640, 400) {
            if (!fs::is_directory(root_ / "menus") ||
                !fs::is_regular_file(root_ / "xml_resources/properties.xml"))
                throw std::runtime_error("Expected extracted APK assets directory");
        }

        fs::path assets(const fs::path&) override {
            return root_;
        }

        // This preview deliberately does not resume a live account session.
        fs::path saveDirectory() override {
            return {};
        }

        void present(const sys::gfx::Image& image, int, int) override {
            host_.present(image);
        }

        void music(const std::string&) override {}

        void pauseMusic(bool) override {}

        void effect(const std::string&) override {}

        void showPopup(const std::string& message) override {
            SDL_ShowSimpleMessageBox(SDL_MESSAGEBOX_INFORMATION, "Raft Pirates", message.c_str(),
                                     nullptr);
        }

        void openWebPage(const std::string& url) override {
            if (url.rfind("https://", 0) != 0 && url.rfind("http://", 0) != 0)
                throw std::invalid_argument("Unsupported URL");
            if (SDL_OpenURL(url.c_str()) != 0)
                throw std::runtime_error("Unable to open browser");
        }

        bool loggedIn() override {
            return false;
        }

        bool transportConnected() override {
            return false;
        }

        void keyboard(const std::string& title, const std::string& value, bool password) override {
            entry_ = Entry{title, value, password};
            host_.textInput(true);
            updateTitle();
        }

        std::future<AuthResult> authenticate(std::string, std::string, std::string, bool, int,
                                             std::string) override {
            std::promise<AuthResult> promise;
            AuthResult result;
            result.error = "Desktop SmartFox networking is not implemented yet.";
            promise.set_value(std::move(result));
            return promise.get_future();
        }

        void connect(const std::string&, int) override {
            throw std::runtime_error("Desktop SmartFox networking unavailable");
        }

        void login(const std::string&, const std::string&, const std::string&,
                   const sfs::SFSObjectWrapper&) override {
            throw std::runtime_error("Desktop SmartFox networking unavailable");
        }

        void send(const sfs::Message&) override {
            throw std::runtime_error("Desktop SmartFox networking unavailable");
        }

        void disconnect() override {}

        void keepAlive() override {}

        void log(const std::string& message) override {
            std::cerr << message << '\n';
        }

        bool poll() {
            for (const auto& event : host_.poll()) {
                if (event.type == HostEventType::Quit)
                    return false;
                if (event.type == HostEventType::Suspend) {
                    game::pause(*this, true);
                    pointer_.reset();
                    continue;
                }
                if (event.type == HostEventType::Resume) {
                    game::pause(*this, false);
                    continue;
                }
                if (entry_) {
                    if (event.type == HostEventType::TextInput &&
                        entry_->value.size() + event.text.size() <= 256) {
                        entry_->value += event.text;
                        updateTitle();
                    } else if (event.type == HostEventType::KeyDown) {
                        if (event.code == SDLK_ESCAPE || event.code == SDLK_RETURN ||
                            event.code == SDLK_KP_ENTER) {
                            finishEntry(event.code == SDLK_ESCAPE);
                        } else if (event.code == SDLK_BACKSPACE && !entry_->value.empty()) {
                            auto& text = entry_->value;
                            auto i = text.size() - 1;
                            while (i > 0 && (static_cast<unsigned char>(text[i]) & 0xc0) == 0x80)
                                --i;
                            text.erase(i);
                            updateTitle();
                        }
                    }
                    continue;
                }
                if (event.type == HostEventType::KeyDown && event.code == SDLK_ESCAPE)
                    return false;
                if (event.type == HostEventType::PointerDown && !pointer_) {
                    pointer_ = event.id;
                    game::touchDown(*this, event.x, event.y);
                } else if (pointer_ && *pointer_ == event.id) {
                    if (event.type == HostEventType::PointerMove)
                        game::touchMove(*this, event.x, event.y);
                    else if (event.type == HostEventType::PointerUp) {
                        pointer_.reset();
                        game::touchUp(*this, event.x, event.y);
                    }
                }
            }
            return true;
        }

        void delay() {
            host_.delay(10);
        }

      private:
        struct Entry {
            std::string title, value;
            bool password;
        };

        fs::path root_;
        NativeHost host_;
        std::optional<Entry> entry_;
        std::optional<int64_t> pointer_;

        void updateTitle() {
            const auto& e = *entry_;
            auto text = e.password ? std::string(e.value.size(), '*') : e.value;
            host_.setTitle(e.title + ": " + text + " [Enter to confirm / Esc to cancel]");
        }

        void finishEntry(bool cancelled) {
            auto value = cancelled ? std::string{} : std::move(entry_->value);
            entry_.reset();
            host_.textInput(false);
            host_.setTitle("Raft Pirates");
            game::keyboardResult(*this, std::move(value), cancelled);
        }
    };
} // namespace

int main(int argc, char** argv) {
    try {
        if (argc != 2) {
            std::cerr << "Usage: raftpirates_desktop ASSET_ROOT\n";
            return 1;
        }
        DesktopServices platform(argv[1]);
        game::init(platform, argv[1], 640, 400);
        if (!game::initialized())
            return 2;
        while (platform.poll()) {
            game::tick(platform);
            platform.delay();
        }
        game::kill(platform);
        return 0;
    } catch (const std::exception& ex) {
        std::cerr << ex.what() << '\n';
        return 2;
    }
}
