#include "rp/MapView.h"
#include <algorithm>
#include <cmath>
#include <stdexcept>

namespace rp {
    MapView::MapView() = default;
    MapView::~MapView() = default;

    void MapView::setPlayerVisible(bool visible) {
        playerVisible_ = visible;
    }

    void MapView::setTargetVisible(bool visible) {
        targetVisible_ = visible;
    }

    bool MapView::movementAllowed() const {
        return canMove_;
    }

    void MapView::movementAllowed(bool allowed) {
        canMove_ = allowed;
        if (!allowed)
            DeleteShipPreview();
    }

    void MapView::showMap() {
        visible_ = true;
    }

    void MapView::hideMap() {
        visible_ = false;
        DeleteShipPreview();
    }

    void MapView::setPlayerPosition(sys::gfx::Vec2 point, bool immediate) {
        if (!std::isfinite(point.x) || !std::isfinite(point.y))
            return;
        destination_ = point;
        if (immediate)
            player_ = point;
    }

    void MapView::tick(float dt) {
        if (!std::isfinite(dt) || dt <= 0)
            return;
        // Portable interpolation; original animation timing remains unverified.
        float t = std::min(1.f, dt * 8.f);
        player_.x += (destination_.x - player_.x) * t;
        player_.y += (destination_.y - player_.y) * t;
    }

    void MapView::touchDown(const sys::gfx::Vec2& point) {
        dragging_ = visible_ && canMove_;
        if (dragging_)
            touchDrag(point);
    }

    void MapView::touchDrag(const sys::gfx::Vec2& point) {
        if (!dragging_)
            return;
        target_ = point;
        setTargetVisible(isValidIconLocation(point.x, point.y));
    }

    void MapView::touchUp(const sys::gfx::Vec2& point) {
        if (dragging_)
            touchDrag(point);
        dragging_ = false;
    }

    void MapView::touchCancel(const sys::gfx::Vec2&) {
        DeleteShipPreview();
    }

    void MapView::DeleteShipPreview() {
        dragging_ = false;
        setTargetVisible(false);
    }

    bool MapView::moveShip() {
        // Only indicates a valid requested destination. The server owns the player position.
        if (!canMove_ || !visible_ || !targetVisible_ || !isValidIconLocation(target_.x, target_.y))
            return false;
        DeleteShipPreview();
        return true;
    }

    void MapView::getSafeSquares(std::vector<Cell>& result) const {
        result.clear();
        if (!map_)
            return;
        for (int y = 0; y < map_->height; ++y)
            for (int x = 0; x < map_->width; ++x) {
                float wx = (x + .5f) * map_->tileWidth, wy = (y + .5f) * map_->tileHeight;
                if (!isPosOnLand(wx, wy) && isPosInSafeWater(wx, wy))
                    result.push_back({x, y});
            }
    }

    void MapView::getUnsafeSquares(std::vector<Cell>& result) const {
        result.clear();
        if (!map_)
            return;
        for (int y = 0; y < map_->height; ++y)
            for (int x = 0; x < map_->width; ++x) {
                float wx = (x + .5f) * map_->tileWidth, wy = (y + .5f) * map_->tileHeight;
                if (!isPosOnLand(wx, wy) && !isPosInSafeWater(wx, wy))
                    result.push_back({x, y});
            }
    }

    bool MapView::hasPlayerNearArea(int x, int y) const {
        if (!map_)
            return false;
        float dx = player_.x - x, dy = player_.y - y;
        // Radius is a reconstruction choice. The native method compares squared distance.
        float radius = float(map_->tileWidth);
        return dx * dx + dy * dy < radius * radius;
    }

    void MapView::clearTreasureMapPoints() {
        treasure_.clear();
    }

    void MapView::addTreasureMapPoint(int id, int x, int y, int moves, int maxMoves) {
        if (id <= 0 || !isValidIconLocation(float(x), float(y)))
            return;
        auto it = std::find_if(treasure_.begin(), treasure_.end(),
                               [&](const TreasurePoint& p) { return p.id == id; });
        TreasurePoint point{id, x, y, moves, maxMoves};
        if (it == treasure_.end())
            treasure_.push_back(point);
        else
            *it = point;
    }

    bool MapView::hasXNearArea(int x, int y) const {
        float a = float(x), b = float(y);
        snapPlayerToTreasureMaps(a, b);
        if (a != x || b != y)
            return true;
        return std::any_of(treasure_.begin(), treasure_.end(),
                           [&](const TreasurePoint& p) { return p.x == x && p.y == y; });
    }

    void MapView::snapPlayerToTreasureMaps(float& x, float& y) const {
        if (!map_)
            return;
        float nearest = float(map_->tileWidth) * map_->tileWidth;
        const TreasurePoint* best = nullptr;
        for (const auto& p : treasure_) {
            float dx = p.x - x, dy = p.y - y, d = dx * dx + dy * dy;
            if (d < nearest) {
                nearest = d;
                best = &p;
            }
        }
        if (best) {
            x = float(best->x);
            y = float(best->y);
        }
    }

    void MapView::setMapFile(const std::filesystem::path& root, const std::string& name) {
        if (name.empty()) {
            map_.reset();
            path_.clear();
            clearTreasureMapPoints();
            DeleteShipPreview();
            return;
        }
        std::filesystem::path p(name);
        if (p.is_absolute() || p.has_root_name() || p.extension() != ".tmx")
            throw std::runtime_error("Invalid map resource path");
        for (const auto& part : p)
            if (part == "..")
                throw std::runtime_error("Invalid map resource path");
        auto full = (root / p).lexically_normal().string();
        if (path_ == full && map_)
            return;
        auto next = sys::res::TmxMap::Load(full);
        map_ = std::move(next);
        path_ = full;
        clearTreasureMapPoints();
        DeleteShipPreview();
    }

    bool MapView::isPosOnLand(float x, float y) const {
        if (!map_ || !std::isfinite(x) || !std::isfinite(y))
            return false;
        for (auto name : {"Islands", "Islands 2"})
            if (map_->tileAtWorld(name, x, y).value_or(0) != 0)
                return true;
        return false;
    }

    bool MapView::isPosInSafeWater(float x, float y) const {
        return map_ && std::isfinite(x) && std::isfinite(y) &&
               map_->tileAtWorld("Safe Water", x, y).value_or(0) != 0;
    }

    bool MapView::isValidIconLocation(float x, float y) const {
        return map_ && std::isfinite(x) && std::isfinite(y) && x >= 0 && y >= 0 &&
               x < map_->width * map_->tileWidth && y < map_->height * map_->tileHeight &&
               !isPosOnLand(x, y);
    }

    void MapView::limitPlayerPos(float& x, float& y) const {
        if (map_) {
            float w = map_->width * map_->tileWidth, h = map_->height * map_->tileHeight;
            x = std::clamp(x, w * .1f, w * .9f);
            y = std::clamp(y, h * .1f, h * .9f);
        }
    }
} // namespace rp
