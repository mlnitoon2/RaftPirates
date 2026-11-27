#include "rp/RaftPirates.h"
#include "rp/RPMenuHandler.h"
#include "rp/BuildModeState.h"
#include <limits>
#include <algorithm>
#include <cmath>
#include <stdexcept>
#include <array>
#include <fstream>
#include "sys/File.h"

namespace rp {
    namespace {
        constexpr std::array<const char*, 10> libraryCommands{
            "lib_quest_type_data", "lib_mat_data",         "lib_bp_data",
            "lib_go_data",         "lib_zone_data",        "lib_discovery_data",
            "lib_store_cur_data",  "lib_store_group_data", "lib_store_item_data",
            "lib_timestamp"};
    }

    RaftPirates::RaftPirates() = default;
    RaftPirates::~RaftPirates() = default;

    void RaftPirates::initialize() {
        static_cast<Game&>(*this) = Game{};
        connected_ = connecting_ = roomJoined_ = paused_ = false;
        processing_ = true;
        transitionVisible_ = false;
        namingShip_ = battleMode_ = false;
        startedTutorials_.clear();
        completedTutorials_.clear();
        queued_.clear();
        adminMessage_.clear();
        serverName_.clear();
        debugMessage_.clear();
        load_ = 0;
        libraryPackets_ = {};
        libraryFile_.clear();
        librarySource_.clear();
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
        transitionVisible_ = false;
        namingShip_ = battleMode_ = false;
        processing_ = true;
        pending.reset();
        pendingMove.reset();
        pendingZone.reset();
        pendingMap.reset();
        pendingDiscovery.reset();
        token.clear();
        onlineBattle.reset();
        sonarPending = readyForBattle = daringAdvancePending = false;
        sonarShips.clear();
        bossData.reset();
        queued_.clear();
        adminMessage_.clear();
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

    void RaftPirates::gotMsgOnAdminMessage(const std::string& message) {
        if (connected_)
            adminMessage_ = message.substr(0, 4096);
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
        gotMsgOnExtensionResponse2(command, data);
    }

    void RaftPirates::serverName(const std::string& host) {
        serverName_ = host;
    }

    const std::string& RaftPirates::serverName() const {
        return serverName_;
    }

    void RaftPirates::gotMsgOnExtensionResponse2(const std::string& command,
                                                 const sfs::SFSObjectWrapper& data) {
        if (connected_ && processing_)
            processExtension(command, data);
    }

    void RaftPirates::gotMsgOnRoomJoin2() {
        gotMsgOnRoomJoin();
    }

    bool RaftPirates::GotMsgAutoSafeModeRequest() const {
        return autoSafe;
    }

    std::optional<sfs::Message> RaftPirates::GotMsgSetAutoSafeMode(bool enabled) const {
        if (!ready || enabled == autoSafe)
            return {};
        sfs::SFSObjectWrapper p;
        p.put("is_enabled", enabled);
        return request("set_auto_safe_enabled", p);
    }

    std::optional<sfs::Message> RaftPirates::gotMsgDaringMeterAdvance() {
        return advanceDaring();
    }

    std::optional<sfs::Message> RaftPirates::gotMsgRequestPingSonar() {
        return ready ? pingSonar() : std::nullopt;
    }

    void RaftPirates::gotMsgReenablePingSonar() {
        sonarPending = false;
    }

    std::optional<sfs::Message> RaftPirates::gotMsgMenuBattleAccept(bool accept) {
        return ready ? onlineBattle.encounterResponse(accept) : std::nullopt;
    }

    std::optional<sfs::Message> RaftPirates::gotMsgPurchaseItemRequest(int item) const {
        if (!ready || !RPMenuHandler(*this).canPurchase(item))
            return {};
        return buyStoreItem(item);
    }

    std::optional<sfs::Message> RaftPirates::gotMsgConfirmZoneMovement(int id, bool confirmed) {
        if (!ready || !confirmed || !zones.count(id) || pendingZone)
            return {};
        return travel(id);
    }

    std::optional<sfs::Message> RaftPirates::HandleGoingToDiscovery(int id) {
        if (!ready || !discoveries.count(id) || pendingDiscovery)
            return {};
        return discover(id);
    }

    std::optional<sfs::Message> RaftPirates::gotMsgMapMarkerMoved(int x, int y, bool safe,
                                                                  int enemy, bool sonar) {
        if (!ready || pendingMap || x < -100 || x > 100 || y < -100 || y > 100)
            return {};
        return moveMap(x, y, safe, enemy, sonar);
    }

    std::pair<std::string, std::string> RaftPirates::gotMsgQuickTravelGetAllKnownRequest() const {
        return {knownZones, knownDiscoveries};
    }

    std::vector<int> RaftPirates::GotMsgCheckForNewDailyQuest() const {
        std::vector<int> out;
        for (auto& [id, q] : quests)
            if (q.visible && q.unread)
                out.push_back(id);
        return out;
    }

    std::optional<sfs::Message> RaftPirates::GotMsgShowNewDailyQuest(int id) {
        auto q = quests.find(id);
        if (!ready || q == quests.end() || !q->second.visible || !q->second.unread)
            return {};
        return openQuest(id);
    }

    void RaftPirates::gotMsgAuthComplete(const std::string& host) {
        DoConnect();
        serverName(host);
    }

    void RaftPirates::gotMsgAuthFailed() {
        DoLostConnection(true);
        status = "Login failed";
    }

    void RaftPirates::gotMsgAuthenticationFailed() {
        gotMsgAuthFailed();
    }

    void RaftPirates::gotMsgOnDebugMessage(const std::string& message) {
        debugMessage_ = message.substr(0, 4096);
    }

    const std::string& RaftPirates::debugMessage() const {
        return debugMessage_;
    }

    std::vector<int> RaftPirates::GotMsgShowStore() const {
        return ready ? RPMenuHandler(*this).gotMsgGetAvailStoreItemRequest()
                     : std::vector<int>{};
    }

    bool RaftPirates::gotMsgNameShip() {
        if (!connected_ || !ready || namingShip_)
            return false;
        namingShip_ = true;
        return true;
    }

    std::optional<sfs::Message> RaftPirates::gotMsgKeyboardEntryResult(const std::string& name,
                                                                         bool cancelled) {
        if (!namingShip_)
            return {};
        namingShip_ = false;
        if (!connected_ || !ready || cancelled || name.empty() || name.size() > 75 ||
            name.find_first_of("\r\n\t") != std::string::npos ||
            name.find('\0') != std::string::npos ||
            name.find_first_not_of(' ') == std::string::npos)
            return {};
        sfs::SFSObjectWrapper packet;
        packet.put("newName", name);
        return request("set_display_name", packet);
    }

    std::optional<sfs::Message> RaftPirates::gotMsgConfirmMaterialPurchase(int material, int amount,
                                                                             int goldCost) const {
        if (!connected_ || !ready || material <= 0 || amount <= 0 || goldCost < 0 ||
            !materialNames.count(material))
            return {};
        sfs::SFSObjectWrapper packet;
        packet.put("material", int32_t(material));
        packet.put("amount", int32_t(amount));
        packet.put("gold_cost", int32_t(goldCost));
        return request("purchase_item", packet);
    }

    std::vector<sfs::Message> RaftPirates::gotMsgConfirmBlueprintMaterialsPurchase(int blueprint) const {
        std::vector<sfs::Message> requests;
        const auto entry = blueprints.find(blueprint);
        if (!ready || !connected_ || entry == blueprints.end() || !owned.count(blueprint))
            return requests;
        for (const auto& [id, amount] : entry->second.recipe) {
            const auto missing = std::max(0, amount - numOfMaterialPossessed(unsigned(id)));
            if (!missing)
                continue;
            auto price = materialGoldCosts.find(id);
            if (price == materialGoldCosts.end() || !std::isfinite(price->second) || price->second < 0)
                return {};
            const double cost = std::ceil(price->second * missing);
            if (cost > std::numeric_limits<int>::max())
                return {};
            auto message = gotMsgConfirmMaterialPurchase(id, missing, int(cost));
            if (!message)
                return {};
            requests.push_back(std::move(*message));
        }
        return requests;
    }

    std::optional<sfs::Message> RaftPirates::gotMsgConfirmRepairGridObjectWithGold(
        int parent, int object, int goldCost, bool all) const {
        if (!ready || !connected_ || object <= 0 || parent < 0 || goldCost < 0 || battleMode_)
            return {};
        const GridObject* target = nullptr;
        if (!parent)
            target = GetRaftByID(object);
        else if (auto raft = GetRaftByID(parent))
            for (const auto& component : raft->components)
                if (component.id == object && !component.Building()) {
                    target = &component;
                    break;
                }
        if (!target)
            return {};
        auto def = target->getResourceData(objects);
        if (!def || (!all && target->bp >= def->bp))
            return {};
        sfs::SFSObjectWrapper packet;
        packet.put("repair_grid_object_id", int32_t(object));
        packet.put("gold_cost", int32_t(goldCost));
        packet.put("repair_all", int32_t(all));
        packet.put("object_id", int32_t(object));
        packet.put("object_type", int32_t(parent ? 1 : 0));
        if (parent)
            packet.put("parent_id", int32_t(parent));
        return request("purchase_repair", packet);
    }

    bool RaftPirates::gotMsgTouchDown(float elapsed) { return checkKeepAlive(elapsed); }
    bool RaftPirates::gotMsgTouchDrag(float elapsed) { return checkKeepAlive(elapsed); }
    bool RaftPirates::gotMsgTouchUp(float elapsed) { return checkKeepAlive(elapsed); }

    bool RaftPirates::gotMsgStartBattleMode() {
        if (!ready || !connected_ || onlineBattle.phase() == BattleModeState::Phase::Idle)
            return false;
        battleMode_ = true;
        return true;
    }

    bool RaftPirates::gotMsgStartBuildMode() {
        if (!ready || !connected_ || onlineBattle.phase() != BattleModeState::Phase::Idle)
            return false;
        battleMode_ = false;
        return true;
    }

    bool RaftPirates::inBattleMode() const { return battleMode_; }

    std::map<std::string, sys::res::XmlNode> RaftPirates::preloadMenus(
        const std::filesystem::path& root) {
        std::map<std::string, sys::res::XmlNode> result;
        for (const char* name : {"tabbed_menu_blueprints", "tabbed_menu_free",
                                 "tabbed_menu_inventory", "tabbed_menu_misc"}) {
            try {
                result.emplace(name, sys::res::loadXML(root / "menus" / (std::string(name) + ".xml")));
            } catch (const std::exception&) {
                // A missing optional menu is handled by the existing UI fallback.
            }
        }
        return result;
    }

    void RaftPirates::GotMsgDoingFacebookAuth() {
        // The Android preservation client has no Facebook authentication provider.
        if (!connected_ && !connecting_)
            status = "Facebook login is not available.";
    }

    void RaftPirates::gotMsgAuthenticationChanged() { handleLogOut(); }

    void RaftPirates::gotMsgGameCenterAuthChanged(bool usingGameCenter) {
        if (usingGameCenter)
            handleLogOut();
    }

    void RaftPirates::gotMsgGameCenterError(bool usingGameCenter) {
        if (usingGameCenter)
            handleLogOut();
    }

    void RaftPirates::checkTutorialStart(int id) {
        if (id > 0)
            startedTutorials_.insert(id);
    }

    void RaftPirates::checkTutorialCompletion(int id) {
        if (id > 0 && startedTutorials_.count(id))
            completedTutorials_.insert(id);
    }

    bool RaftPirates::tutorialStarted(int id) const { return startedTutorials_.count(id) != 0; }
    bool RaftPirates::tutorialCompleted(int id) const { return completedTutorials_.count(id) != 0; }

    void RaftPirates::gotMsgBoughtCurrency(const sfs::SFSObjectWrapper& packet) {
        if (connected_)
            extension("purchase_response", packet);
    }

    void RaftPirates::gotMsgBoughtItem(const sfs::SFSObjectWrapper& packet) {
        if (connected_)
            extension("purchase_response", packet);
    }

    void RaftPirates::gotMsgUpdateCurrency(const sfs::SFSObjectWrapper& packet) {
        if (connected_)
            extension("gs_materials_changed", packet);
    }

    void RaftPirates::processExtension(const std::string& command,
                                       const sfs::SFSObjectWrapper& data) {
        if (command == "purchase_response")
            gotMsgBoughtItem(data);
        else if (command == "gs_materials_changed")
            gotMsgUpdateCurrency(data);
        else
            extension(command, data);
        if (ready && connected_) {
            if (onlineBattle.phase() == BattleModeState::Phase::Idle)
                gotMsgStartBuildMode();
            else
                gotMsgStartBattleMode();
        }
        if (std::find(libraryCommands.begin(), libraryCommands.end(), command) !=
            libraryCommands.end())
            libraryPackets_.put(command, std::make_shared<sfs::SFSObjectWrapper>(data));
    }

    void RaftPirates::loadStaticLibraries(const std::filesystem::path& file,
                                          const std::string& source) {
        libraryFile_ = file;
        librarySource_ = source;
        std::error_code ec;
        auto size = std::filesystem::file_size(file, ec);
        if (ec || size > sfs::maxPacket)
            return;
        auto cache = sfs::decodeObject(sys::File::readAll(file));
        if (cache.getString("source") != source || cache.getInt("version") != 1)
            return;
        auto timestamp = cache.getLong("timestamp");
        if (timestamp <= 0)
            return;
        Game restored;
        for (const auto* command : libraryCommands) {
            auto packet = cache.getSFSObj(command);
            if (!packet)
                return;
            restored.extension(command, *packet);
        }
        restored.libraryTimestamp = timestamp;
        static_cast<Game&>(*this) = std::move(restored);
        // A complete cache is advertised to the server. Only a complete new snapshot replaces it.
    }

    void RaftPirates::saveStaticLibraries() const {
        if (libraryFile_.empty() || librarySource_.empty())
            return;
        auto time = libraryPackets_.getSFSObj("lib_timestamp");
        if (!time)
            return;
        auto timestamp = time->getLong("timestamp");
        if (timestamp <= 0)
            return;
        sfs::SFSObjectWrapper cache;
        cache.put("source", librarySource_);
        cache.put("version", int32_t(1));
        cache.put("timestamp", timestamp);
        for (const auto* command : libraryCommands) {
            auto packet = libraryPackets_.getSFSObj(command);
            if (!packet)
                return;
            cache.put(command, packet);
        }
        auto data = sfs::encodeObject(cache);
        if (data.size() > sfs::maxPacket)
            return;
        auto tmp = libraryFile_;
        tmp += ".tmp";
        std::ofstream output(tmp, std::ios::binary | std::ios::trunc);
        if (!output.write(reinterpret_cast<const char*>(data.data()), data.size()))
            throw std::runtime_error("Could not write static library cache");
        output.close();
        if (!output)
            throw std::runtime_error("Could not finish static library cache");
        std::filesystem::rename(tmp, libraryFile_);
    }

    void RaftPirates::setNetworkMsgProcessingEnabled(bool enabled) {
        processing_ = enabled;
        while (connected_ && processing_ && !queued_.empty()) {
            auto packet = std::move(queued_.front());
            queued_.pop_front();
            try {
                auto data = packet.payload.getSFSObj("p");
                if (data)
                    processExtension(packet.payload.getString("c"), *data);
            } catch (const std::exception& ex) {
                const auto command = packet.payload.getString("c");
                if (command == "gs_player")
                    throw;
                status = "Rejected " + command + ": " + ex.what();
            }
        }
    }

    void RaftPirates::gotMsgEnableNetworkProcessing(bool enabled) {
        setNetworkMsgProcessingEnabled(enabled);
    }

    void RaftPirates::GotMsgStartTransition(bool show) {
        transitionVisible_ = show;
        if (show)
            setNetworkMsgProcessingEnabled(false);
    }

    void RaftPirates::GotTransitionMsgDone() {
        setNetworkMsgProcessingEnabled(true);
    }

    bool RaftPirates::transitionVisible() const {
        return transitionVisible_;
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
