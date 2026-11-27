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
        void gotMsgOnAdminMessage(const std::string&);

        const std::string& adminMessage() const {
            return adminMessage_;
        }

        void gotMsgOnConnectionLost();
        void serverName(const std::string& host);
        const std::string& serverName() const;
        void gotMsgOnExtensionResponse2(const std::string&, const sfs::SFSObjectWrapper&);
        void gotMsgOnRoomJoin2();
        bool GotMsgAutoSafeModeRequest() const;
        std::optional<sfs::Message> GotMsgSetAutoSafeMode(bool enabled) const;
        std::optional<sfs::Message> gotMsgDaringMeterAdvance();
        std::optional<sfs::Message> gotMsgRequestPingSonar();
        void gotMsgReenablePingSonar();
        std::optional<sfs::Message> gotMsgMenuBattleAccept(bool accept);
        std::optional<sfs::Message> gotMsgPurchaseItemRequest(int item) const;
        std::optional<sfs::Message> gotMsgConfirmZoneMovement(int id, bool confirmed);
        std::optional<sfs::Message> HandleGoingToDiscovery(int id);
        std::optional<sfs::Message> gotMsgMapMarkerMoved(int x, int y, bool safe, int enemy,
                                                         bool sonar);
        std::pair<std::string, std::string> gotMsgQuickTravelGetAllKnownRequest() const;
        std::vector<int> GotMsgCheckForNewDailyQuest() const;
        std::optional<sfs::Message> GotMsgShowNewDailyQuest(int id);
        void gotMsgAuthComplete(const std::string& host);
        void gotMsgAuthFailed();
        void gotMsgAuthenticationFailed();
        void gotMsgOnDebugMessage(const std::string& message);
        const std::string& debugMessage() const;
        void gotMsgBoughtItem(const sfs::SFSObjectWrapper& packet);
        void gotMsgUpdateCurrency(const sfs::SFSObjectWrapper& packet);
        void gotMsgOnExtensionResponse(const std::string& command,
                                       const sfs::SFSObjectWrapper& data);
        void setNetworkMsgProcessingEnabled(bool enabled);
        void gotMsgEnableNetworkProcessing(bool enabled);
        void loadStaticLibraries(const std::filesystem::path& file, const std::string& source);
        void saveStaticLibraries() const;
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
        std::string adminMessage_, serverName_, debugMessage_;
        std::filesystem::path libraryFile_;
        std::string librarySource_;
        sfs::SFSObjectWrapper libraryPackets_;
        void processExtension(const std::string&, const sfs::SFSObjectWrapper&);
    };
} // namespace rp
