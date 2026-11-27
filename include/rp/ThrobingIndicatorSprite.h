#pragma once
#include <cmath>

namespace rp {
    class ThrobingIndicatorSprite {
      public:
        ThrobingIndicatorSprite();
        ~ThrobingIndicatorSprite();
        void tick(float dt);

        float offset() const {
            return -75.f - 30.f * std::sin(3.141592653589793f * phase_);
        }

      private:
        float phase_ = 1;
    };
} // namespace rp
