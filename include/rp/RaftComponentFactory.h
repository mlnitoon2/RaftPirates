#pragma once
#include "rp/RaftComponent.h"
#include "sfs/SFSObjectWrapper.h"
#include <functional>
namespace rp {
    class RaftComponentFactory {
      public:
        using Builder=std::function<RaftComponent(const sfs::SFSObjectWrapper&)>;
        RaftComponentFactory(){RegistryAllClasses();}
        void RegistryAllClasses();
        RaftComponent Build(const sfs::SFSObjectWrapper&,const std::string& name="RaftComponent") const;
      private:
        std::map<std::string,Builder> builders_;
    };
}
