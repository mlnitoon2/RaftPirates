#pragma once
#include "rp/GridObjectDataRes.h"

namespace rp {
    class DiscoveryGridObject : public GridObject {
      public:
        explicit DiscoveryGridObject(const std::map<int, GridObjectInfo>&);
        ~DiscoveryGridObject() override;
        std::unique_ptr<GridObject> Duplicate() const override;

        const GridObjectInfo* definition() const {
            return resource_ ? &resource_->data() : nullptr;
        }

      private:
        std::shared_ptr<const GridObjectDataRes> resource_;
    };
} // namespace rp
