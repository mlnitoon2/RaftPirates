#include "rp/Blueprint.h"
#include <limits>
#include <stdexcept>

namespace rp {
    Blueprint::Blueprint(int id, int obj, int level, std::string name)
        : object(obj), unlock(level), id_(id), name_(std::move(name)) {}

    void Blueprint::init(int id) {
        id_ = id;
        object = unlock = 0;
        name_.clear();
        sheet.clear();
        image.clear();
        deprecated = false;
        purchaseZone = displayOrder = buildSeconds = 0;
        recipe.clear();
    }

    void Blueprint::initWithSFSObject(const sfs::SFSObjectWrapper& data) {
        init(data.getInt("blueprint_id"));
        object = data.getInt("object_id");
        unlock = data.getInt("unlock_level");
        name_ = data.getString("name");
        sheet = data.getString("sheet_id");
        image = data.getString("image_id");
        deprecated = data.getBool("depricated", data.getInt("depricated") != 0);
        purchaseZone = data.getInt("purchasable_at_zone");
        displayOrder = data.getInt("display_order");
        buildSeconds = data.getInt("build_seconds");
        for (int i = 1; i <= 4; ++i) {
            auto key = "material_" + std::to_string(i);
            int id = data.getInt(key), amount = data.getInt(key + "_amount");
            if (id > 0 && amount > 0) {
                if (recipe[id] > std::numeric_limits<int>::max() - amount)
                    throw std::runtime_error("Blueprint recipe exceeds integer range");
                recipe[id] += amount;
            }
        }
    }

    int Blueprint::id() const {
        return id_;
    }

    const std::string& Blueprint::name() const {
        return name_;
    }
} // namespace rp
