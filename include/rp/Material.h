#pragma once
#include "sfs/SFSObjectWrapper.h"
#include <string>

namespace rp {
    class Material {
      public:
        Material() = default;
        void init(int id, int amount);
        void initWithSFSObject(const sfs::SFSObjectWrapper& data);
        int id() const;
        int amount() const;
        const std::string& name() const;
        void setName(std::string name);

      private:
        int id_ = 0, amount_ = 0;
        std::string name_;
    };
} // namespace rp
