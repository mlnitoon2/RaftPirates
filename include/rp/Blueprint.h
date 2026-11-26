#pragma once
#include "sfs/SFSObjectWrapper.h"
#include <string>

namespace rp {
    // Static catalogue entry; ID lookup is also used by owned blueprints.
    class Blueprint {
      public:
        Blueprint() = default;
        Blueprint(int id, int object, int unlock, std::string name);
        void init(int id);
        void initWithSFSObject(const sfs::SFSObjectWrapper& data);
        int id() const;
        const std::string& name() const;
        int object = 0, unlock = 0;

      private:
        int id_ = 0;
        std::string name_;
    };
} // namespace rp
