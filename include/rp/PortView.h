#pragma once
#include "rp/Game.h"
#include "rp/Hud.h"
#include "rp/DivingGame.h"
#include "rp/MapView.h"
#include "rp/LoadingManager.h"
#include "rp/BattleModeState.h"
#include "rp/GfxBattleScreen.h"
#include "rp/DiscoveryModeState.h"
#include "rp/TouchControls.h"
#include "sys/menu_redux/MenuRuntime.h"
#include "sys/localization/LocalizationManager.h"
#include "sys/gfx/ResourceSpriteFont.h"

namespace rp {
    // World interaction and native bindings for the original Menu Redux resources.
    class PortView {
      public:
        explicit PortView(std::filesystem::path root);
        sys::gfx::Image render(const Game&, float time, const std::string& status);
        sys::gfx::Image renderTitleBackground(const Game&, float time);
        std::optional<sfs::Message> touch(Game&, float x, float y);
        void tick(float dt);
        std::optional<sfs::Message> takePendingRequest(Game&);
        std::string activeMenu() const;
        void touchDown(float x, float y);
        void touchDrag(float x, float y);
        void cancelInput();
        void purchaseCompleted(const Game&, bool success, int blueprint);
        std::string takeNotice() { auto notice = std::move(notice_); notice_.clear(); return notice; }

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

        bool soundEnabled() const {
            return soundEnabled_;
        }

      private:
        friend struct PortViewTestAccess;
        std::filesystem::path root_;
        OceanView ocean_, opponentOcean_;
        Cell attackPosition_{0, 0};
        TouchControls touchControls_;
        bool oceanGesture_ = false;
        LoadingManager loader_;
        std::unique_ptr<Hud> hud_;
        std::unique_ptr<diving_game::DivingGame> diving_;
        std::unique_ptr<GfxBattleScreen> battleScreen_;
        bool onlineBattleActive_ = false;
        std::unique_ptr<DiscoveryModeState> discovery_;
        size_t ownBattleIndex_ = 0, enemyBattleIndex_ = 0;
        int battleDice_ = 1;
        bool diveStarted_ = false;
        bool diveSeen_ = false;
        bool diveClosing_ = false;
        std::pair<int, int> divingUnit_{};
        bool hudEnabled(const Game& game) const;
        MapView mapView_;
        int selectedQuest_ = 0;
        std::string requestedStoreItem_, notice_;
        std::optional<int> mapInfoEnemy_;
        float worldTime_ = 0;
        bool scrollVertical_ = false;
        std::optional<float> lastScrollOffset_;
        std::map<std::string, std::shared_ptr<sys::gfx::Image>> tiles_;
        std::map<std::string, std::pair<std::shared_ptr<sys::res::ResourceSpriteSheet>,
                                        std::shared_ptr<sys::gfx::Image>>>
            shopAtlases_;
        bool renderItemIcon(sys::gfx::Image&, const std::string&, const std::string&, int, int);
        enum class Panel {
            None,
            Shop,
            Inventory,
            Travel,
            Build,
            Quest,
            QuestDetail,
            Safety,
            Daring,
            Confirm,
            Admin
        };
        Panel panel_ = Panel::None;
        size_t page_ = 0;
        static constexpr size_t BlueprintPageSize = 5;
        float blueprintListTop_ = 300;
        std::optional<sys::gfx::Vec2> blueprintGesture_;
        bool blueprintPageMove(const Game&, int);
        void selectBlueprintSlot(const Game&, float);
        int selectedBlueprint_ = 0;
        int selectedParent_ = 0, selectedComponent_ = 0;
        bool moving_ = false;
        std::optional<Cell> placement_;
        int placementParent_ = 0;
        int selectedStore_ = 0, shopSelection_ = 0;
        void updateShopFrame(const Game&);
        std::vector<int> menuItemIds(const Game&, const std::string&) const;
        std::string blueprintIcon(const std::string&, const std::string&);
        std::map<std::pair<std::string, std::string>, std::string> blueprintIcons_;
        std::map<std::string, std::shared_ptr<sys::res::ResourceSpriteSheet>> blueprintSheets_;
        std::map<std::string, std::shared_ptr<sys::gfx::Image>> previewImages_;
        std::map<std::string, sys::res::ResourceAEAnim> previewBins_;
        std::unique_ptr<sys::gfx::AnimationRenderer> previewRenderer_;
        bool soundEnabled_ = true;
        std::string confirmAction_;
        unsigned swipe_ = 0;
        std::map<unsigned, double> scrollOrigins_;
        bool xmlCaptured_ = false;
        bool shopButtonArmed_ = false;
        std::optional<sys::gfx::Vec2> mapTap_;
        unsigned mapControlAt(float x, float y) const;
        std::optional<int> pendingStoreItem_;
        unsigned shopButtonAt(float x, float y) const;
        void updateHud(const Game& game);
        void updateScroll();
        std::optional<sfs::Message> panelTouch(Game&, float x, float y);
        std::unique_ptr<sys::menu_redux::MenuRuntime> xmlMenu_;
        std::string xmlName_;
        std::map<std::string, sys::res::XmlNode> preloadedMenus_;
        std::map<std::string, int> xmlItems_;
        sys::localization::LocalizationManager strings_;
        void openXml(const Game&, const std::string&);
        void renderXml(sys::gfx::Image&, const Game&, const std::string&);
        std::optional<sfs::Message> xmlTouch(Game&, float, float, unsigned captured = 0);
        void renderPanel(sys::gfx::Image&, const Game&);
        void loadMap(const Game&);
    };
} // namespace rp
