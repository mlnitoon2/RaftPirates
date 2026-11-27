#include "rp/MsgQueuer.h"
#include <stdexcept>
namespace rp {
    MsgQueuer::MsgQueuer()=default;
    MsgQueuer::~MsgQueuer()=default;
    void MsgQueuer::add(sfs::Message message) {
        if(packets_.size()>=1024)throw std::runtime_error("Network queue limit");
        packets_.push_back(std::move(message));
    }
    void MsgQueuer::clear(){packets_.clear();}
    size_t MsgQueuer::numQueued() const{return packets_.size();}
    std::optional<sfs::Message> MsgQueuer::processOne(){
        if(packets_.empty())return {};
        auto packet=std::move(packets_.front());packets_.pop_front();return packet;
    }
}
