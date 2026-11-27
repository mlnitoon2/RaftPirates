#include "rp/RaftComponentFactory.h"
#include <stdexcept>

namespace rp {
    void RaftComponentFactory::RegistryAllClasses() {
        builders_.clear();
        builders_.emplace("RaftComponent", [](const auto& data) {
            RaftComponent value;
            value.initWithSFSObject(data);
            return value;
        });
    }

    RaftComponent RaftComponentFactory::Build(const sfs::SFSObjectWrapper& data,
                                              const std::string& name) const {
        auto it = builders_.find(name);
        if (it == builders_.end())
            throw std::invalid_argument("Unsupported class: " + name);
        return it->second(data);
    }
} // namespace rp
