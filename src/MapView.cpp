#include "rp/MapView.h"
#include <algorithm>
#include <cmath>
#include <stdexcept>

namespace rp {
    MapView::MapView() = default;
    MapView::~MapView() = default;

    void MapView::setMapFile(const std::filesystem::path& root, const std::string& name) {
        if (name.empty()) {
            map_.reset();
            path_.clear();
            return;
        }
        std::filesystem::path p(name);
        if (p.is_absolute() || p.has_root_name() || p.extension() != ".tmx")
            throw std::runtime_error("Invalid map resource path");
        for (auto& part : p)
            if (part == "..")
                throw std::runtime_error("Invalid map resource path");
        auto full = (root / p).lexically_normal().string();
        if (path_ == full && map_)
            return;
        auto next = sys::res::TmxMap::Load(full);
        map_ = std::move(next);
        path_ = full;
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
