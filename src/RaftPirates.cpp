#include "rp/RaftPirates.h"
#include <algorithm>
#include <cmath>
#include <stdexcept>

namespace rp {
    RaftPirates::RaftPirates() = default;
    RaftPirates::~RaftPirates() = default;

    void RaftPirates::initialize() {
        static_cast<Game&>(*this) = Game{};
        connected_ = connecting_ = roomJoined_ = paused_ = false;
        processing_ = true;
        queued_.clear();
        load_ = 0;
    }

    void RaftPirates::DoConnect() {
        initialize();
        connecting_ = true;
        status = "Connecting";
    }

    void RaftPirates::DoLostConnection(bool clearPlayer) {
        if (clearPlayer)
            initialize();
        connected_ = connecting_ = roomJoined_ = ready = false;
        pending.reset();
        pendingMove.reset();
        pendingZone.reset();
        pendingMap.reset();
        pendingDiscovery.reset();
        token.clear();
        onlineBattle.reset();
        sonarPending = readyForBattle = false;
        sonarShips.clear();
        bossData.reset();
        queued_.clear();
        status = "Disconnected";
    }

    void RaftPirates::gotMsgOnConnection(bool success) {
        if (!success)
            DoLostConnection(false);
        else if (connecting_)
            status = "Logging in";
    }

    void RaftPirates::gotMsgOnLogin() {
        if (!connecting_)
            return;
        connecting_ = false;
        connected_ = true;
        status = "Loading player";
        if (processing_)
            setNetworkMsgProcessingEnabled(true);
    }

    void RaftPirates::gotMsgOnLoginError() {
        DoLostConnection(false);
        status = "Login rejected";
    }

    void RaftPirates::gotMsgOnRoomJoin() {
        if (connected_ || connecting_)
            roomJoined_ = true;
    }

    void RaftPirates::gotMsgOnLogout() {
        DoLostConnection(true);
    }

    void RaftPirates::gotMsgOnConnectionLost() {
        DoLostConnection(false);
    }

    void RaftPirates::gotMsgOnExtensionResponse(const std::string& command,
                                                const sfs::SFSObjectWrapper& data) {
        if (!connected_ && !connecting_)
            return;
        if (!processing_ || !connected_) {
            if (queued_.size() >= 1024)
                throw std::runtime_error("Network queue limit");
            queued_.push_back(Game::request(command, data));
            return;
        }
        extension(command, data);
    }

    void RaftPirates::setNetworkMsgProcessingEnabled(bool enabled) {
        processing_ = enabled;
        while (connected_ && processing_ && !queued_.empty()) {
            auto packet = std::move(queued_.front());
            queued_.pop_front();
            accept(packet);
        }
    }

    void RaftPirates::gotMsgEnableNetworkProcessing(bool enabled) {
        setNetworkMsgProcessingEnabled(enabled);
    }

    void RaftPirates::Pause(bool value) {
        paused_ = value;
    }

    void RaftPirates::tick(float dt) {
        if (!paused_)
            tickNoSkip(dt);
    }

    void RaftPirates::tickNoSkip(float dt) {
        if (std::isfinite(dt) && dt >= 0)
            Game::tick(dt);
    }

    bool RaftPirates::checkKeepAlive(float elapsed) {
        return connected_ && std::isfinite(elapsed) && elapsed >= 30;
    }

    bool RaftPirates::GotMsgBadZoneStatusRequest() {
        return badZone;
    }

    int RaftPirates::GotMsgGetMapActionState() {
        return mapActions;
    }

    bool RaftPirates::isDaringZoneLimited() {
        auto info = zoneData.find(zone);
        return info != zoneData.end() && info->second.daringLimit() > 0 &&
               daring >= info->second.daringLimit();
    }

    std::string RaftPirates::gotMsgZoneGetAllKnownRequest() {
        return knownZones;
    }

    std::string RaftPirates::gotMsgDiscoveryGetAllKnownRequest() {
        return knownDiscoveries;
    }

    sfs::Message RaftPirates::gotMsgZoneMoveTo(int id) {
        return travel(id);
    }

    sfs::Message RaftPirates::gotMsgDiscoveryMoveTo(int id) {
        return discover(id);
    }

    void RaftPirates::gotMsgQuestEvent(int id, int progress) {
        auto it = quests.find(id);
        if (it != quests.end())
            it->second.setProgressPercentage(progress);
    }

    sfs::Message RaftPirates::gotMsgQuestOpened(int id) {
        return openQuest(id);
    }

    double RaftPirates::gotMsgDaringMeterInfo() {
        return daringProgress();
    }

    void RaftPirates::gotMsgLoadStart() {
        load_ = 0;
    }

    void RaftPirates::gotMsgLoadPercentage(float value) {
        if (std::isfinite(value))
            load_ = std::clamp(value, 0.f, 100.f);
    }

    void RaftPirates::gotMsgLoadFinished() {
        load_ = 100;
    }

    void RaftPirates::performLogout(bool clearPlayer) {
        DoLostConnection(clearPlayer);
    }

    void RaftPirates::handleLogOut() {
        performLogout(true);
    }

    void RaftPirates::gotMsgMenuLogoutRequest() {
        handleLogOut();
    }

} // namespace rp
