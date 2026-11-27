#pragma once
#include "rp/BattlePlacementScreen.h"
#include "rp/DamageIndicatorSprite.h"
#include "rp/ExplosionFXSprite.h"
#include "rp/BattleArrowSprite.h"
#include "rp/AccessorySprite.h"
#include "sys/gfx/ResourceSpriteFont.h"
#include "sys/menu_redux/MenuRuntime.h"
#include "sys/localization/LocalizationManager.h"

namespace rp {
    class GfxBattleScreen {
      public:
        explicit GfxBattleScreen(std::filesystem::path root);
        ~GfxBattleScreen();
        sys::gfx::Image Render(const Game& game, float time);
        std::optional<sfs::Message> Select(Game& game, float x, float y);
        bool AddAccessory(bool own, int parent, int id, std::unique_ptr<AccessorySprite>);
        void SetObjectMute(bool own, const GridObject& object, bool mute, int parent = 0);
        void SetObjectVisible(bool own, const GridObject& object, float opacity, int parent = 0);
        void SetPlayerRaft(bool own, const Raft& raft, bool animate);
        void SetPlayerRaftsVisible(bool own, const Player& player, float opacity);

      private:
        void prepare(const Game& game);
        void updateBattleDamage(const Game& game, float time);
        void tick(float dt);
        void SwoopIn();
        void SwoopOut();
        void submitSprite(sys::gfx::GfxManager& gfx, const std::string& name, float x, float y,
                          float w, float h) const;
        void submitText(sys::gfx::GfxManager& gfx, const std::string& value, float cx, float y,
                        float scale, float width) const;
        void renderFleet(sys::gfx::Image& out, OceanView& view, const Player& player, int left,
                         int selected, float time) const;
        void meter(sys::gfx::GfxManager& gfx, const Player& player, int x, bool own) const;
        int pick(const Player& player, const OceanView& view, float x, float y, int left) const;
        void loadMenu(const Game& game);
        std::filesystem::path root_;
        sys::localization::LocalizationManager strings_;
        std::unique_ptr<sys::menu_redux::MenuRuntime> menu_;
        std::string menuName_;
        bool positionChosen_ = false;
        OceanView ownView_, enemyView_;
        BattlePlacementScreen placement_;
        sys::gfx::ResourceSpriteSheet sheet_;
        std::shared_ptr<const sys::gfx::Image> atlas_;
        sys::gfx::ResourceSpriteFont font_;
        BattleModeState::Phase lastPhase_ = BattleModeState::Phase::Idle;
        bool lastAttacker_ = false, retreatConfirm_ = false;
        int dice_ = 1;
        int focusedOwn_ = 0, focusedEnemy_ = 0;
        std::map<int, int> ownBP_, enemyBP_;

        std::vector<DamageIndicatorSprite> damage_;
        std::map<std::pair<bool,int>,ExplosionFXSprite> explosions_;
        BattleArrowSprite arrow_;
        std::optional<sys::res::ResourceAEAnim> explosion_;
        sys::gfx::AnimationRenderer effects_;
        float lastTime_ = -1, transition_ = 0;
        bool exiting_ = false;
    };
} // namespace rp
