#pragma once
#include "sfs/SFSObjectWrapper.h"

namespace rp {
    class Discovery {
      public:
        ~Discovery();
        void initWithSFSObject(const sfs::SFSObjectWrapper& data);
        int id() const;
        const std::string& name() const;
        int zoneid() const;
        int questgroupid() const;

      private:
        int id_ = 0, zone_ = 0, quest_ = 0;
        std::string name_;
    };
} // namespace rp
