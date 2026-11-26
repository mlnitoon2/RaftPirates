#include "rp/RaftPirates.h"
#include <stdexcept>

namespace {
    void check(bool result) {
        if (!result)
            throw std::runtime_error("Session regression");
    }
} // namespace

int main() {
    rp::RaftPirates game;
    game.DoConnect();
    game.gotMsgOnConnection(true);
    game.gotMsgOnLogin();
    game.gotMsgOnRoomJoin();
    check(game.connected() && game.roomJoined());
    game.ready = true;
    check(!game.checkKeepAlive(19) && game.checkKeepAlive(20));
    sfs::SFSObjectWrapper packet;
    packet.put("bad", int32_t(1));
    game.gotMsgEnableNetworkProcessing(false);
    game.gotMsgOnExtensionResponse("zonestatus", packet);
    check(!game.GotMsgBadZoneStatusRequest());
    game.gotMsgEnableNetworkProcessing(true);
    check(game.GotMsgBadZoneStatusRequest());
    game.daringRemaining = 10;
    game.Pause(true);
    game.tick(1);
    check(game.daringRemaining == 10);
    game.Pause(false);
    game.tick(1);
    check(game.daringRemaining == 9);
    game.pendingMap = rp::Cell{1, 1};
    game.gotMsgOnConnectionLost();
    check(!game.ready && !game.connected() && !game.pendingMap);
    game.gotMsgOnExtensionResponse("zonestatus", packet);
    game.gotMsgMenuLogoutRequest();
    check(!game.GotMsgBadZoneStatusRequest());
}
