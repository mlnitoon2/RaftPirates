#pragma once
#include <string>
#include <vector>

namespace rp::diving_game {
    // Repeating vertical sprite strip; the native speed multiplier is retained.
    class ScrollingLayer {
      public:
        struct Tile {
            std::string name;
            float y;
        };

        ScrollingLayer(std::vector<std::string> images, float height, float multiplier,
                       float viewportHeight);
        void SetSpeed(float speed);
        void addSprite(float y);
        void tick(float dt);

        const std::vector<Tile>& tiles() const {
            return tiles_;
        }

      private:
        std::vector<std::string> images_;
        std::vector<Tile> tiles_;
        float height_, multiplier_, viewportHeight_, speed_ = 0;
        size_t next_ = 0;
    };
} // namespace rp::diving_game
