#pragma once
#include "rp/Game.h"
#include <deque>

namespace rp {
    // Native game-session coordinator. The platform bridge owns authentication and transport.
    class RaftPirates : public Game {
      public:
        RaftPirates();
        ~RaftPirates();
        void initialize();
        void DoConnect();
        void DoLostConnection(bool clearPlayer);
        void gotMsgOnConnection(bool success);
        void gotMsgOnLogin();
        void gotMsgOnLoginError();
        void gotMsgOnRoomJoin();
        void gotMsgOnLogout();
        void gotMsgOnConnectionLost();
        void gotMsgOnExtensionResponse(const std::string& command, const sfs::SFSObjectWrapper& data);
        void setNetworkMsgProcessingEnabled(bool enabled);
        void gotMsgEnableNetworkProcessing(bool enabled);
        void Pause(bool value);
        void tick(float dt);
        void tickNoSkip(float dt);
        bool checkKeepAlive(float elapsed);
        bool GotMsgBadZoneStatusRequest();
        int GotMsgGetMapActionState();
        bool isDaringZoneLimited();
        std::string gotMsgZoneGetAllKnownRequest();
        std::string gotMsgDiscoveryGetAllKnownRequest();
        sfs::Message gotMsgZoneMoveTo(int id);
        sfs::Message gotMsgDiscoveryMoveTo(int id);
        void gotMsgQuestEvent(int id, int progress);
        sfs::Message gotMsgQuestOpened(int id);
        double gotMsgDaringMeterInfo();
        void gotMsgLoadStart();
        void gotMsgLoadPercentage(float value);
        void gotMsgLoadFinished();
        void performLogout(bool clearPlayer);
        void handleLogOut();
        void gotMsgMenuLogoutRequest();
        bool connected() const {
            return connected_;
        }
        float loadPercentage() const {
            return load_;
        }
        bool roomJoined() const {
            return roomJoined_;
        }

      private:
        bool connected_ = false, connecting_ = false, roomJoined_ = false;
        bool processing_ = true, paused_ = false;
        float load_ = 0;
        std::deque<sfs::Message> queued_;
    };
} // namespace rp
