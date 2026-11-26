#pragma once
#include "rp/Game.h"
#include "rp/Hud.h"
#include "rp/DivingGame.h"
#include "rp/MapView.h"
#include "rp/LoadingManager.h"
#include "rp/BattleModeState.h"
#include "rp/DiscoveryModeState.h"
#include "rp/TouchControls.h"

namespace rp {
    // Temporary native controls for the original APK's GLSurfaceView. Not original Menu Redux UI.
    class PortView {
      public:
        explicit PortView(std::filesystem::path root);
        sys::gfx::Image render(const Game&, float time, const std::string& status);
        std::optional<sfs::Message> touch(Game&, float x, float y);
        void tick(float dt);
        void touchDown(float x, float y);
        void touchDrag(float x, float y);

        bool isDiving() const {
            return bool(diving_);
        }

        bool diveFinished() const {
            return diving_ && diving_->finished();
        }

        int diveGears() const {
            return diving_ ? diving_->gears() : 0;
        }

        bool grid = false, map = false;

      private:
        std::filesystem::path root_;
        OceanView ocean_;
        TouchControls touchControls_;
        bool oceanGesture_ = false;
        LoadingManager loader_;
        std::unique_ptr<Hud> hud_;
        std::unique_ptr<diving_game::DivingGame> diving_;
        std::unique_ptr<BattleModeState> battle_;
        std::unique_ptr<DiscoveryModeState> discovery_;
        bool diveStarted_ = false;
        bool diveSeen_ = false;
        bool hudEnabled(const Game& game) const;
        MapView mapView_;
        int selectedQuest_ = 0;
        std::map<std::string, std::shared_ptr<sys::gfx::Image>> tiles_;
        std::map<std::string, std::pair<std::shared_ptr<sys::res::ResourceSpriteSheet>,
                                        std::shared_ptr<sys::gfx::Image>>>
            shopAtlases_;
        bool renderItemIcon(sys::gfx::Image&, const std::string&, const std::string&, int, int);
        enum class Panel { None, Shop, Inventory, Travel, Build, Quest, Safety, Daring };
        Panel panel_ = Panel::None;
        size_t page_ = 0;
        int selectedBlueprint_ = 0;
        int selectedParent_ = 0, selectedComponent_ = 0;
        bool moving_ = false;
        std::optional<sfs::Message> panelTouch(Game&, float x, float y);
        void renderPanel(sys::gfx::Image&, const Game&);
        void loadMap(const Game&);
    };
} // namespace rp
