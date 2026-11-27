#pragma once
#include "rp/Raft.h"
#include "sfs/SFSObjectWrapper.h"
#include <functional>

namespace rp {
    class RaftFactory {
      public:
        using Builder = std::function<Raft(const sfs::SFSObjectWrapper&)>;

        RaftFactory() {
            RegistryAllClasses();
        }

        void RegistryAllClasses();
        Raft Build(const sfs::SFSObjectWrapper&, const std::string& name = "Raft") const;

      private:
        std::map<std::string, Builder> builders_;
    };
} // namespace rp
