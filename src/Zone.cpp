#include "rp/Zone.h"

namespace rp {
    void Zone::initWithSFSObject(const sfs::SFSObjectWrapper& data) {
        id_ = data.getInt("zone_id");
        name_ = data.getString("name");
        map_ = data.getString("mapfile");
        min_ = data.getInt("min_bp");
        max_ = data.getInt("max_bp");
        buffer_ = data.getInt("buffer_bp");
        limit_ = data.getInt("daringLimit");
        openX_ = data.getInt("defaultpos_open_x");
        openY_ = data.getInt("defaultpos_open_y");
        safeX_ = data.getInt("defaultpos_safe_x");
        safeY_ = data.getInt("defaultpos_safe_y");
    }

    int Zone::id() const {
        return id_;
    }

    const std::string& Zone::name() const {
        return name_;
    }

    const std::string& Zone::mapfile() const {
        return map_;
    }

    int Zone::min_bp() const {
        return min_;
    }

    int Zone::max_bp() const {
        return max_;
    }

    int Zone::buffer_bp() const {
        return buffer_;
    }

    int Zone::daringLimit() const {
        return limit_;
    }

    int Zone::defaultpos_open_x() const {
        return openX_;
    }

    int Zone::defaultpos_open_y() const {
        return openY_;
    }

    int Zone::defaultpos_safe_x() const {
        return safeX_;
    }

    int Zone::defaultpos_safe_y() const {
        return safeY_;
    }
} // namespace rp
