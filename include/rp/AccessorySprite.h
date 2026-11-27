#pragma once
#include <string>
#include <cstdint>

namespace sys::gfx {
    class Gfx;
    class GfxManager;
    class AnimationRenderer;
} // namespace sys::gfx

namespace rp {
    class OceanView;
    class GridSprite;

    class AccessorySprite {
      public:
        AccessorySprite(const OceanView* ocean, GridSprite* parent, int id);
        virtual ~AccessorySprite();
        void AddToTick();
        virtual void RemoveFromTick();
        int GetID() const;
        std::string GetTimeString(int seconds) const;
        virtual void Init();
        virtual void Update();
        virtual void tick(float dt);
        virtual void SetScale(float scale);
        virtual bool ListenForAnimation();

        virtual void AnimationStarted() {}

        virtual void SetMute(bool) {}

        virtual void UpdateParticles() {}

        virtual void SetVisible(bool) {}

        virtual void SetColour(uint8_t, uint8_t, uint8_t, uint8_t) {}

        virtual void SetPriorities() {}

        virtual void SetPriority(unsigned, float) {}

        virtual bool ContainsGfx(const sys::gfx::Gfx*) const {
            return false;
        }

        virtual bool finished() const {
            return false;
        }

        virtual int submit(sys::gfx::GfxManager&, sys::gfx::AnimationRenderer&) const {
            return 0;
        }

        const GridSprite* parent() const {
            return parent_;
        }

        bool ticking() const {
            return ticking_;
        }

      protected:
        const OceanView* ocean_;
        GridSprite* parent_;

      private:
        int id_;
        bool ticking_ = false;
    };
} // namespace rp
