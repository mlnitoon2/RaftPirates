#pragma once
#include "rp/GridSprite.h"

namespace rp {
    class PerimeterWallSprite {
      public:
        // Native destructor releases a string before delegating to SpritePiece.
        struct SpriteBalloon : GridSprite::SpritePiece {
            virtual ~SpriteBalloon();
            std::string animation;
        };
    };
}
