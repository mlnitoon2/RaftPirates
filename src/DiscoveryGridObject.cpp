#include "rp/DiscoveryGridObject.h"

namespace rp {
    DiscoveryGridObject::DiscoveryGridObject(const std::map<int, GridObjectInfo>& db)
        : resource_(GridObjectDataRes::Get("barnacle_bills", db)) {
        if (resource_) {
            type = resource_->data().id;
            bp = resource_->data().bp;
        }
    }

    DiscoveryGridObject::~DiscoveryGridObject() = default;

    std::unique_ptr<GridObject> DiscoveryGridObject::Duplicate() const {
        return std::make_unique<DiscoveryGridObject>(*this);
    }
} // namespace rp
