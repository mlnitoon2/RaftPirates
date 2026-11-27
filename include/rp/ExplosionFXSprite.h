#pragma once

namespace rp {
    class ExplosionFXSprite {
      public:
        explicit ExplosionFXSprite(bool raft, bool boss = false);
        ~ExplosionFXSprite();
        void Init();
        void RemoveFromTick();
        void Update();
        bool Sparatic() const;
        float GetTime() const;
        void tick(float dt);

        bool active() const {
            return active_;
        }

        float elapsed() const {
            return elapsed_;
        }

      private:
        bool raft_, boss_, active_ = false;
        float elapsed_ = 0;
    };
} // namespace rp
