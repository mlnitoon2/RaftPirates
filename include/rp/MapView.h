#pragma once
#include "sys/res/TmxMap.h"

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

        const std::optional<sys::res::TmxMap>& data() const {
            return map_;
        }

      private:
        std::optional<sys::res::TmxMap> map_;
        std::string path_;
    };
} // namespace rp
