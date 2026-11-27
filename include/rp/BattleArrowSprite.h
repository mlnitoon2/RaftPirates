#pragma once

namespace rp {
    class BattleArrowSprite {
      public:
        BattleArrowSprite();
        ~BattleArrowSprite();
        void tick(float dt);
        float yOffset() const;

      private:
        float phase_ = 1;
    };
} // namespace rp
