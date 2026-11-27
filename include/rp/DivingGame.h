#pragma once
#include "sys/gfx/Gfx.h"
#include "rp/ScrollingLayer.h"
#include "sys/gfx/GfxText.h"
#include "sys/msg/MsgReceiver.h"
#include "sys/gfx/AnimationRenderer.h"
#include <random>

namespace rp::diving_game {
    struct DivingCollectable {
        enum class CollectableType { Gear, Mine };
        CollectableType type;
        float x, y;
        DivingCollectable(CollectableType type, float x, float y);
    };

    // Reconstructed minigame, with original graphics and approximate mechanics.
    // Results are cosmetic online; server awards materials through normal packets.
    class DivingGame {
      public:
        enum class DiverMoveDirection { Left, Right };
        explicit DivingGame(std::filesystem::path assets, unsigned seed = 1);
        ~DivingGame();
        void Reset();
        void InitLevel();
        void NextLevel();
        void NextShape();
        void AutoScaleText(sys::gfx::GfxText* text);
        void onMsgTouchState(const sys::msg::MsgTouchState& msg);
        void gotMsgUpdate(const sys::msg::MsgUpdate& msg);
        void tick(float dt);
        void moveDiver(DiverMoveDirection direction, float seconds);
        void moveTo(float x, float y, float duration);
        void setPos(float x,float y);
        void touchDown(const sys::gfx::Vec2& point);
        void touchDrag(const sys::gfx::Vec2& point);
        void touchUp(const sys::gfx::Vec2& point);
        void touchCancel(const sys::gfx::Vec2& point);
        bool wasLeftButtonTouched(const sys::gfx::Vec2& point) const;
        bool wasRightButtonTouched(const sys::gfx::Vec2& point) const;
        bool wasExitButtonTouched(const sys::gfx::Vec2& point) const;
        void CheckMenuButtonStates(const sys::gfx::Vec2& point);
        void updateButtonStates();
        float GetNextGearX();
        float GetNextMine();
        bool robotCollidesWithItem(const DivingCollectable& item) const;
        void spawnItem(DivingCollectable::CollectableType type, float x);
        void UpdateGearCount();
        void SaveYourself();
        void ConfirmDiveFinish();
        void render(sys::gfx::Image& target);
        void setDuration(float seconds);

        bool finished() const {
            return finished_;
        }

        bool confirmed() const {
            return confirmed_;
        }

        int gears() const {
            return gears_;
        }

        float remaining() const {
            return remaining_;
        }

        int level() const {
            return level_;
        }

      private:
        std::filesystem::path root_;
        std::shared_ptr<sys::res::ResourceSpriteSheet> atlas_;
        std::shared_ptr<sys::gfx::Image> image_;
        sys::gfx::AnimationRenderer renderer_;
        sys::res::ResourceAEAnim robot_;
        std::mt19937 rng_;
        sys::gfx::GfxText label_;
        struct ItemFloater : DivingCollectable {
            ItemFloater(CollectableType,float x,float y);
            ~ItemFloater();
            void tick(float dt,float speed);
        };
        std::vector<ItemFloater> items_;
        std::vector<ScrollingLayer> scrolling_;
        float x_ = 320, y_ = 270, targetX_ = 320, speed_ = 120, duration_ = 20, remaining_ = 20,
              elapsed_ = 0, spawn_ = 0;
        int level_ = 1, gears_ = 0, shape_ = 0, direction_ = 0;
        bool finished_ = false, confirmed_ = false, pressed_ = false;
        void sprite(sys::gfx::Image& target, const char* name, float x, float y, float scale = 1,
                    bool flip = false);
    };
} // namespace rp::diving_game
