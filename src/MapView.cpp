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
            clearEnemyShips();
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
        clearEnemyShips();
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

    void MapView::clearEnemyShips() {
        enemies_.clear();
        boss_.reset();
    }

    void MapView::updateEnemyShips(std::vector<EnemyShipPoints> enemies) {
        enemies_.clear();
        for (const auto& p : enemies)
            if (p.id > 0 && isValidIconLocation(p.x, p.y) &&
                std::none_of(enemies_.begin(), enemies_.end(),
                             [&](const auto& e) { return e.id == p.id; }))
                enemies_.push_back(p);
    }

    void MapView::updateBoss(std::optional<EnemyShipPoints> boss) {
        boss_ = boss && boss->id > 0 && isValidIconLocation(boss->x, boss->y) ? boss : std::nullopt;
    }

    bool MapView::hasSonarIconNearArea(int x, int y) const {
        if (!map_)
            return false;
        double radius = map_->tileWidth;
        return std::any_of(enemies_.begin(), enemies_.end(), [&](const auto& e) {
            double dx = double(x) - e.x, dy = double(y) - e.y;
            return dx * dx + dy * dy < radius * radius;
        });
    }

    void MapView::updateBossGfx(EnemyShipPoints& boss) {
        // Original updateBossGfx consumes unsafe squares, rejecting player, treasure and sonar
        // proximity. The candidate order is deterministic here, not the original shuffled order.
        std::vector<Cell> candidates;
        getUnsafeSquares(candidates);
        boss.x = boss.y = -1;
        if (!map_)
            return;
        for (auto it = candidates.rbegin(); it != candidates.rend(); ++it) {
            int x = it->x * map_->tileWidth + map_->tileWidth / 2,
                y = it->y * map_->tileHeight + map_->tileHeight / 2;
            if (!isValidIconLocation(x, y) || hasPlayerNearArea(x, y) || hasXNearArea(x, y) ||
                hasSonarIconNearArea(x, y) ||
                (boss_ && boss_->id != boss.id && hasBossNearArea(x, y)))
                continue;
            boss.x = x;
            boss.y = y;
            break;
        }
    }

    void MapView::updateBoss(const std::optional<sfs::SFSObjectWrapper>& data, int zone) {
        if (!data || data->getInt("zone") != zone || data->getInt("user_id") <= 0) {
            boss_.reset();
            return;
        }
        EnemyShipPoints next;
        next.id = data->getInt("user_id");
        next.name = data->getString("display_name");
        next.battlePoints = data->getInt("battle_points");
        if (boss_ && boss_->id == next.id) {
            next.x = boss_->x;
            next.y = boss_->y;
        } else
            updateBossGfx(next);
        updateBoss(next);
    }

    void MapView::updateEnemyShips(const std::vector<sfs::SFSObjectWrapper>& records) {
        if (!map_)
            return;
        auto previous = std::move(enemies_);
        enemies_.clear();
        for (const auto& data : records) {
            EnemyShipPoints next;
            next.id = data.getInt("user_id");
            next.name = data.getString("display_name");
            next.battlePoints = data.getInt("battle_points");
            next.nemesis = data.getBool("is_nemesis");
            if (next.id <= 0 || std::any_of(enemies_.begin(), enemies_.end(),
                                            [&](const auto& e) { return e.id == next.id; }))
                continue;
            auto old = std::find_if(previous.begin(), previous.end(),
                                    [&](const auto& e) { return e.id == next.id; });
            if (old != previous.end()) {
                next.x = old->x;
                next.y = old->y;
            } else
                updateBossGfx(next);
            if (isValidIconLocation(next.x, next.y) && !hasBossNearArea(next.x, next.y))
                enemies_.push_back(std::move(next));
        }
    }

    const MapView::EnemyShipPoints* MapView::enemyAt(float x, float y) const {
        if (!map_)
            return nullptr;
        double best = double(map_->tileWidth) * map_->tileWidth;
        const EnemyShipPoints* out = nullptr;
        auto consider = [&](const EnemyShipPoints& e) {
            double dx = e.x - x, dy = e.y - y, d = dx * dx + dy * dy;
            if (d < best) {
                best = d;
                out = &e;
            }
        };
        if (boss_)
            consider(*boss_);
        for (const auto& e : enemies_)
            consider(e);
        return out;
    }

    bool MapView::hasBossNearArea(int x, int y) const {
        if (!map_ || !boss_)
            return false;
        double dx = double(x) - boss_->x, dy = double(y) - boss_->y;
        return dx * dx + dy * dy < double(map_->tileWidth) * map_->tileWidth;
    }

    void MapView::snapPlayerToBoss(float& x, float& y) const {
        if (boss_ && std::isfinite(x) && std::isfinite(y) && hasBossNearArea(int(x), int(y))) {
            x = boss_->x;
            y = boss_->y;
        }
    }

    void MapView::snapPlayerToEnemyShips(float& x, float& y) const {
        if (!map_)
            return;
        double best = double(map_->tileWidth) * map_->tileWidth;
        const EnemyShipPoints* result = nullptr;
        for (const auto& p : enemies_) {
            double dx = p.x - x, dy = p.y - y, d = dx * dx + dy * dy;
            if (d < best) {
                best = d;
                result = &p;
            }
        }
        if (result) {
            x = result->x;
            y = result->y;
        }
    }

    bool MapView::selectEnemy(const EnemyShipPoints& enemy) {
        if (!canMove_ || !visible_ || !isValidIconLocation(enemy.x, enemy.y))
            return false;
        if (boss_ && boss_->id == enemy.id && boss_->x == enemy.x && boss_->y == enemy.y) {
            target_ = {float(enemy.x), float(enemy.y)};
            targetVisible_ = true;
            return true;
        }
        auto it = std::find_if(enemies_.begin(), enemies_.end(), [&](const auto& p) {
            return p.id == enemy.id && p.x == enemy.x && p.y == enemy.y;
        });
        if (it == enemies_.end())
            return false;
        target_ = {float(enemy.x), float(enemy.y)};
        targetVisible_ = true;
        return true;
    }

    void MapView::updateTreasureMapPoints() {
        treasure_.erase(
            std::remove_if(treasure_.begin(), treasure_.end(),
                           [&](const auto& p) { return !isValidIconLocation(p.x, p.y); }),
            treasure_.end());
    }
} // namespace rp
