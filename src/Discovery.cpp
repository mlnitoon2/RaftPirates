#include "rp/Discovery.h"

namespace rp {
    Discovery::~Discovery() = default;

    void Discovery::initWithSFSObject(const sfs::SFSObjectWrapper& data) {
        id_ = data.getInt("id");
        name_ = data.getString("name");
        zone_ = data.getInt("zoneLocation");
        quest_ = data.getInt("questgroup");
    }

    int Discovery::id() const {
        return id_;
    }

    const std::string& Discovery::name() const {
        return name_;
    }

    int Discovery::zoneid() const {
        return zone_;
    }

    int Discovery::questgroupid() const {
        return quest_;
    }
} // namespace rp
