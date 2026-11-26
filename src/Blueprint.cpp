#include "rp/Blueprint.h"

namespace rp {
    Blueprint::Blueprint(int id, int obj, int level, std::string name)
        : object(obj), unlock(level), id_(id), name_(std::move(name)) {}

    void Blueprint::init(int id) {
        id_ = id;
        object = unlock = 0;
        name_.clear();
        sheet.clear();
        image.clear();
    }

    void Blueprint::initWithSFSObject(const sfs::SFSObjectWrapper& data) {
        init(data.getInt("blueprint_id"));
        object = data.getInt("object_id");
        unlock = data.getInt("unlock_level");
        name_ = data.getString("name");
        sheet = data.getString("sheet_id");
        image = data.getString("image_id");
    }

    int Blueprint::id() const {
        return id_;
    }

    const std::string& Blueprint::name() const {
        return name_;
    }
} // namespace rp
