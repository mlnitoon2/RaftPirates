#include "rp/RaftFactory.h"
#include <stdexcept>
namespace rp {
    void RaftFactory::RegistryAllClasses(){
        builders_.clear();
        builders_.emplace("Raft",[](const auto& data){Raft value;value.initWithSFSObject(data);return value;});
    }
    Raft RaftFactory::Build(const sfs::SFSObjectWrapper& data,const std::string& name) const{
        auto it=builders_.find(name);
        if(it==builders_.end())throw std::invalid_argument("Unsupported class: "+name);
        return it->second(data);
    }
}
