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
    game.gotMsgOnAdminMessage("stale");
    check(game.adminMessage().empty());
    game.DoConnect();
    game.gotMsgOnConnection(true);
    sfs::SFSObjectWrapper early;
    early.put("bad", int32_t(1));
    game.gotMsgOnExtensionResponse("zonestatus", early);
    check(!game.GotMsgBadZoneStatusRequest());
    game.gotMsgOnLogin();
    check(game.GotMsgBadZoneStatusRequest());
    early.put("bad", int32_t(0));
    game.gotMsgOnExtensionResponse("zonestatus", early);
    game.gotMsgOnRoomJoin();
    check(game.connected() && game.roomJoined());
    game.gotMsgOnAdminMessage("Maintenance soon");
    check(game.adminMessage() == "Maintenance soon");
    check(!game.checkKeepAlive(29) && game.checkKeepAlive(30));
    check(game.login("10", "session").payload.getSFSObj("p")->getString("client_version") ==
          "1.0.4");
    game.ready = true;
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
    game.pendingZone = 2;
    game.pending = rp::PendingBuild{};
    game.gotMsgOnExtensionResponse("gs_error", {});
    check(!game.pendingMap && !game.pendingZone && !game.pending);
    game.pendingMap = rp::Cell{1, 1};
    sfs::SFSObjectWrapper ocean;
    ocean.put("map_x", int32_t(3));
    ocean.put("map_y", int32_t(4));
    game.gotMsgOnExtensionResponse("gs_ocean", ocean);
    check(game.mapX == 3 && game.mapY == 4 && !game.pendingMap);
    sfs::SFSObjectWrapper invalid;
    invalid.put("player_object", std::make_shared<sfs::SFSObjectWrapper>());
    bool rejected = false;
    try {
        game.gotMsgOnExtensionResponse("gs_player", invalid);
    } catch (const std::runtime_error&) {
        rejected = true;
    }
    check(rejected && game.ready && game.mapX == 3 && game.mapY == 4);
    game.pendingMap = rp::Cell{1, 1};
    game.gotMsgOnConnectionLost();
    check(!game.ready && !game.connected() && !game.pendingMap);
    check(game.adminMessage().empty());
    game.gotMsgOnExtensionResponse("zonestatus", packet);
    game.gotMsgMenuLogoutRequest();
    check(!game.GotMsgBadZoneStatusRequest());
}
