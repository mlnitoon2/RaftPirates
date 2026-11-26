#include "rp/Material.h"
#include <algorithm>

namespace rp {
    void Material::init(int id, int amount) {
        id_ = id;
        amount_ = std::max(0, amount);
        name_.clear();
    }

    void Material::initWithSFSObject(const sfs::SFSObjectWrapper& data) {
        init(data.getInt("material"), data.getInt("amount"));
    }

    int Material::id() const {
        return id_;
    }

    int Material::amount() const {
        return amount_;
    }

    const std::string& Material::name() const {
        return name_;
    }

    void Material::setName(std::string name) {
        name_ = std::move(name);
    }
} // namespace rp
