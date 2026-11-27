#pragma once
#include "sfs/SFSObjectWrapper.h"
namespace rp {
    // ARM32 RaftInfo::RaftInfo reads four integer fields, in this order.
    struct RaftInfo {
        explicit RaftInfo(const sfs::SFSObjectWrapper&);
        int typeId=0,battlePoints=0,positionX=0,positionY=0;
    };
}
