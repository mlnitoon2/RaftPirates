#pragma once
#include "sys/res/TmxMap.h"
#include "rp/GridObject.h"
#include "sys/gfx/Gfx.h"

namespace rp {
    class MapView {
      public:
        MapView();
        ~MapView();
        void setMapFile(const std::filesystem::path& root, const std::string& name);
        bool isPosOnLand(float x, float y) const;
        bool isPosInSafeWater(float x, float y) const;
        bool isValidIconLocation(float x, float y) const;
        void limitPlayerPos(float& x, float& y) const;
        void setPlayerVisible(bool visible);
        void setTargetVisible(bool visible);

        bool playerVisible() const {
            return visible_ && playerVisible_;
        }

        bool targetVisible() const {
            return visible_ && targetVisible_;
        }

        void setPlayerPosition(sys::gfx::Vec2 position, bool immediate);

        sys::gfx::Vec2 playerPosition() const {
            return player_;
        }

        sys::gfx::Vec2 targetPosition() const {
            return target_;
        }

        bool movementAllowed() const;
        void movementAllowed(bool allowed);
        void showMap();
        void hideMap();

        bool visible() const {
            return visible_;
        }

        void touchDown(const sys::gfx::Vec2& point);
        void touchDrag(const sys::gfx::Vec2& point);
        void touchUp(const sys::gfx::Vec2& point);
        void touchCancel(const sys::gfx::Vec2& point);
        void tick(float dt);
        void getSafeSquares(std::vector<Cell>& result) const;
        void getUnsafeSquares(std::vector<Cell>& result) const;
        bool hasPlayerNearArea(int x, int y) const;
        void clearTreasureMapPoints();
        void addTreasureMapPoint(int id, int x, int y, int moves, int maxMoves);
        bool hasXNearArea(int x, int y) const;
        void snapPlayerToTreasureMaps(float& x, float& y) const;
        void DeleteShipPreview();
        bool moveShip();

        const std::optional<sys::res::TmxMap>& data() const {
            return map_;
        }

      private:
        std::optional<sys::res::TmxMap> map_;
        std::string path_;

        struct TreasurePoint {
            int id, x, y, moves, maxMoves;
        };

        std::vector<TreasurePoint> treasure_;
        sys::gfx::Vec2 player_{}, destination_{}, target_{};
        bool visible_ = false, playerVisible_ = true, targetVisible_ = false;
        bool canMove_ = true, dragging_ = false;
    };
} // namespace rp
