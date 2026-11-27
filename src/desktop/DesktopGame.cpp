#include "sys/platform/NativeHost.h"
#include "sys/platform/Services.h"
#include "sfs/SFSClient.h"
#include "DesktopAuth.h"
#define SDL_MAIN_HANDLED
#include <SDL.h>
#include <SDL_mixer.h>
#include <map>
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
        explicit DesktopServices(const fs::path& root, bool allowHttp)
            : root_(fs::absolute(root)), host_("Raft Pirates", 640, 400),
              allowHttp_(allowHttp), paths_(nativePaths("RaftPirates")) {
            if (!fs::is_directory(root_ / "menus") ||
                !fs::is_regular_file(root_ / "xml_resources/properties.xml"))
                throw std::runtime_error("Expected extracted APK assets directory");
            if (auto* base = SDL_GetBasePath()) {
                const auto ca = fs::path(base) / "ca-bundle.crt";
                if (fs::is_regular_file(ca))
                    caFile_ = ca.string();
                SDL_free(base);
            }
            if (SDL_InitSubSystem(SDL_INIT_AUDIO) == 0) {
                audioInitialized_ = true;
                Mix_Init(MIX_INIT_MP3);
                audioOpen_ = Mix_OpenAudio(44100, MIX_DEFAULT_FORMAT, 2, 2048) == 0;
                if (audioOpen_)
                    Mix_AllocateChannels(24);
            }
            if (!audioOpen_)
                log("Audio device unavailable; continuing without audio");
        }

        ~DesktopServices() override {
            client_.disconnect();
            if (audioOpen_) {
                Mix_HaltMusic();
                Mix_HaltChannel(-1);
                if (music_)
                    Mix_FreeMusic(music_);
                for (auto& e : effects_)
                    Mix_FreeChunk(e.second);
                Mix_CloseAudio();
            }
            if (audioInitialized_) {
                Mix_Quit();
                SDL_QuitSubSystem(SDL_INIT_AUDIO);
            }
        }

        fs::path assets(const fs::path&) override {
            return root_;
        }

        fs::path saveDirectory() override {
            return paths_.saves;
        }

        void present(const sys::gfx::Image& image, int, int) override {
            host_.present(image);
        }

        void music(const std::string& path) override {
            if (!audioOpen_)
                return;
            Mix_HaltMusic();
            if (music_) {
                Mix_FreeMusic(music_);
                music_ = nullptr;
            }
            if (path.empty())
                return;
            music_ = Mix_LoadMUS((root_ / path).string().c_str());
            if (!music_ || Mix_PlayMusic(music_, -1) < 0)
                log("Unable to play music: " + path);
        }

        void pauseMusic(bool paused) override {
            if (!audioOpen_)
                return;
            if (paused) {
                Mix_PauseMusic();
                Mix_Pause(-1);
            } else {
                Mix_ResumeMusic();
                Mix_Resume(-1);
            }
        }

        void effect(const std::string& path) override {
            if (!audioOpen_)
                return;
            auto it = effects_.find(path);
            if (it == effects_.end()) {
                auto* chunk = Mix_LoadWAV((root_ / path).string().c_str());
                if (!chunk) {
                    log("Unable to load sound: " + path);
                    return;
                }
                it = effects_.emplace(path, chunk).first;
            }
            Mix_PlayChannel(-1, it->second, 0);
        }

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
            return client_.loggedIn();
        }

        bool transportConnected() override {
            return client_.connected();
        }

        void keyboard(const std::string& title, const std::string& value, bool password) override {
            entry_ = Entry{title, value, password};
            host_.textInput(true);
            updateTitle();
        }

        std::future<AuthResult> authenticate(std::string endpoint, std::string email,
                                             std::string password, bool registration,
                                             int gameId, std::string version) override {
            return rp::desktop::authenticate(std::move(endpoint), std::move(email),
                std::move(password), registration, gameId, std::move(version),
                caFile_, allowHttp_);
        }

        void connect(const std::string& host, int port) override {
            client_.connect(host, port);
        }

        void login(const std::string& zone, const std::string& user,
                   const std::string& session, const sfs::SFSObjectWrapper& params) override {
            client_.login(zone, user, session, params);
        }

        void send(const sfs::Message& message) override { client_.send(message); }
        void disconnect() override { client_.disconnect(); }
        void keepAlive() override {
            // Raft Pirates' game-server keep-alive is an extension request,
            // separate from the SDK's generic system ping.
            if (client_.loggedIn()) {
                sfs::SFSObjectWrapper p;
                p.put("c", "keep_alive");
                p.put("r", int32_t(-1));
                p.put("p", std::make_shared<sfs::SFSObjectWrapper>());
                client_.send({1, 13, p});
            }
        }

        void networkEvents() {
            for (const auto& e : client_.poll()) {
                using E = sfs::ClientEventType;
                switch (e.type) {
                case E::Connected: game::onConnection(*this, true); break;
                case E::ConnectFailed: game::onConnection(*this, false); break;
                case E::Login: game::onLogin(*this); break;
                case E::LoginError: game::onLoginError(*this); break;
                case E::RoomJoin: game::onRoomJoin(*this); break;
                case E::Extension: game::onExtension(*this, e.command, e.data); break;
                case E::AdminMessage:
                    game::onAdminMessage(*this, e.data.getString("message", e.data.getString("m")));
                    break;
                case E::Logout: game::onLogout(*this); break;
                case E::ConnectionLost: game::onConnectionLost(*this); break;
                }
            }
        }

        void log(const std::string& message) override {
            std::cerr << message << '\n';
        }

        bool poll() {
            networkEvents();
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
                if (event.type == HostEventType::KeyDown && event.code == SDLK_F11) {
                    host_.toggleFullscreen();
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
        bool allowHttp_;
        NativePaths paths_;
        std::string caFile_;
        sfs::SFSClient client_;
        bool audioInitialized_ = false, audioOpen_ = false;
        Mix_Music* music_ = nullptr;
        std::map<std::string, Mix_Chunk*> effects_;
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
    SDL_SetMainReady();
    if (argc == 2 && std::string(argv[1]) == "--version") {
        std::cout << "Raft Pirates desktop reconstruction\n";
        return 0;
    }
    try {
        fs::path root;
        bool allowHttp = false;
        for (int i = 1; i < argc; ++i) {
            if (std::string(argv[i]) == "--allow-insecure-auth")
                allowHttp = true;
            else if (root.empty())
                root = argv[i];
            else
                throw std::invalid_argument("Usage: raftpirates_desktop [ASSET_ROOT] [--allow-insecure-auth]");
        }
        if (root.empty()) {
            auto* base = SDL_GetBasePath();
            if (!base)
                throw std::runtime_error("Cannot locate executable");
            root = fs::path(base) / "assets";
            SDL_free(base);
        }
        if (!fs::is_directory(root / "menus")) {
            SDL_ShowSimpleMessageBox(SDL_MESSAGEBOX_ERROR, "Raft Pirates",
                "Extract the original APK's assets into the assets folder next to this executable.", nullptr);
            return 1;
        }
        DesktopServices platform(root, allowHttp);
        game::init(platform, root, 640, 400);
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
        SDL_ShowSimpleMessageBox(SDL_MESSAGEBOX_ERROR, "Raft Pirates", ex.what(), nullptr);
        return 2;
    }
}
