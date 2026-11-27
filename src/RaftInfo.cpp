#include "rp/RaftInfo.h"

namespace rp {
    RaftInfo::RaftInfo(const sfs::SFSObjectWrapper& data)
        : typeId(data.getInt("type_id")), battlePoints(data.getInt("battle_points")),
          positionX(data.getInt("position_x")), positionY(data.getInt("position_y")) {}
} // namespace rp
