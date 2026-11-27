#include "rp/RaftPirates.h"
#include <stdexcept>
#include <chrono>
#include <fstream>

namespace {
    void check(bool result) {
        if (!result)
            throw std::runtime_error("Session regression");
    }
} // namespace

int main() {
    rp::RaftPirates game;
    check(!game.gotMsgReconnect(false));
    check(game.gotMsgReconnect(true));
    check(!game.gotMsgReconnect(true));
    game.diveCollectPending = true;
    game.DoLostConnection(false);
    check(!game.diveCollectPending);
    game.DoLostConnection(true);
    game.GotMsgOnMenuLoadGlobal("hud");
    check(game.currentMenu() == "hud");
    game.gotMsgPopupTextResult(false);
    check(game.takePopupResult() == std::optional<bool>(false) && !game.takePopupResult());
    sfs::SFSObjectWrapper rewards;
    auto changes = std::make_shared<sfs::SFSArrayWrapper>();
    auto material = std::make_shared<sfs::SFSObjectWrapper>();
    material->put("id", int32_t(2));
    material->put("amt", int32_t(7));
    changes->values.push_back(material);
    rewards.put("materials_changed", changes);
    check(game.QuestRewards(rewards).at(2) == 7 && game.numOfMaterialPossessed(2u) == 0);
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
    check(game.GotMsgShowStore().empty());
    game.ready = true;
    check(game.gotMsgNameShip());
    check(!game.gotMsgNameShip());
    check(!game.gotMsgKeyboardEntryResult("Cancelled", true));
    check(game.gotMsgNameShip());
    auto nameRequest = game.gotMsgKeyboardEntryResult("The Example", false);
    check(nameRequest && nameRequest->payload.getString("c") == "set_display_name");
    check(nameRequest->payload.getSFSObj("p")->getString("newName") == "The Example");
    check(!game.gotMsgKeyboardEntryResult("Unsolicited", false));
    game.gotMsgOnExtensionResponse("gs_quest_reward", rewards);
    check(game.numOfMaterialPossessed(2u) == 7);
    auto reward = game.takeQuestRewardNotice();
    check(reward && dynamic_cast<rp::MsgCollectedMaterials*>(reward->message.get()));
    check(!game.takeQuestRewardNotice());
    rp::MsgCheat cheat;
    cheat.type = "test";
    check(!game.gotMsgCheat(cheat));
    game.serverName("127.0.0.1");
    game.enableDevelopmentCommands(true);
    check(game.gotMsgCheat(cheat).has_value());
    game.enableDevelopmentCommands(false);
    check(!game.gotMsgCheat(cheat));
    check(game.fastClockMaterialId() == 10);
    game.materialNames[10] = "raftparts";
    check(game.fastClockMaterialId() == 0);
    game.materialNames[9] = "clock";
    check(game.fastClockMaterialId() == 9);
    game.materialNames.erase(9);
    game.materialNames[10] = "Fast Clocks";
    check(game.fastClockMaterialId() == 10);
    game.materialNames.erase(10);
    game.materialNames[2] = "Wood";
    auto materialRequest = game.gotMsgConfirmMaterialPurchase(2, 10, 5);
    check(materialRequest && materialRequest->payload.getString("c") == "purchase_item");
    check(materialRequest->payload.getSFSObj("p")->getInt("material") == 2);
    check(materialRequest->payload.getSFSObj("p")->getInt("amount") == 10);
    check(materialRequest->payload.getSFSObj("p")->getInt("gold_cost") == 5);
    check(!game.gotMsgConfirmMaterialPurchase(2, -10, 5));
    check(game.gotMsgTouchDown(30) && game.gotMsgTouchUp(30) && !game.gotMsgTouchDrag(29));
    check(game.gotMsgStartBuildMode() && !game.inBattleMode());
    game.checkTutorialStart(1);
    game.checkTutorialCompletion(1);
    check(!game.tutorialStarted(1) && !game.tutorialCompleted(1));
    game.checkTutorialStart(189);
    game.checkTutorialCompletion(189);
    game.checkTutorialCompletion(83);
    check(game.tutorialStarted(189) && game.tutorialCompleted(189));
    check(game.tutorialCompleted(83));
    check(rp::RaftPirates::tutorialMilestone(76) == 7);
    check(rp::RaftPirates::tutorialMilestone(247) == 14);
    check(rp::RaftPirates::tutorialMilestone(261) == 32);
    check(rp::RaftPirates::tutorialMilestone(83) == 33);
    check(game.GotMsgShowStore().empty());
    sfs::SFSObjectWrapper failedPurchase;
    failedPurchase.put("result", false);
    game.gotMsgBoughtCurrency(failedPurchase);
    check(game.status == "Purchase failed");
    auto safe = game.GotMsgSetAutoSafeMode(true);
    check(safe && safe->payload.getString("c") == "set_auto_safe_enabled");
    check(safe->payload.getSFSObj("p")->getBool("is_enabled"));
    game.safe = false;
    game.daringTotal = 120;
    game.daringRemaining = 0;
    check(game.gotMsgDaringMeterAdvance().has_value());
    check(!game.gotMsgDaringMeterAdvance());
    game.gotMsgOnExtensionResponse("daring_advanceresult", {});
    check(game.gotMsgDaringMeterAdvance().has_value());
    sfs::SFSObjectWrapper packet;
    packet.put("bad", int32_t(1));
    game.gotMsgEnableNetworkProcessing(false);
    game.gotMsgOnExtensionResponse("zonestatus", packet);
    check(!game.GotMsgBadZoneStatusRequest());
    game.gotMsgEnableNetworkProcessing(true);
    check(game.GotMsgBadZoneStatusRequest());
    game.GotMsgStartTransition(true);
    check(game.transitionVisible());
    packet.put("bad", int32_t(0));
    game.gotMsgOnExtensionResponse("zonestatus", packet);
    check(game.GotMsgBadZoneStatusRequest());
    game.GotMsgStartTransition(false);
    check(!game.transitionVisible() && game.GotMsgBadZoneStatusRequest());
    game.GotTransitionMsgDone();
    check(!game.GotMsgBadZoneStatusRequest());
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
    game.GotMsgStartTransition(true);
    game.gotMsgOnConnectionLost();
    check(!game.transitionVisible());
    check(!game.ready && !game.connected() && !game.pendingMap);
    check(game.adminMessage().empty());
    game.gotMsgOnExtensionResponse("zonestatus", packet);
    game.gotMsgMenuLogoutRequest();
    check(!game.GotMsgBadZoneStatusRequest());
    // Recreate the supplied server's initial library sequence, including its timestamp
    // preceding the three store libraries. No synthetic local session is started.
    auto cache = std::filesystem::temp_directory_path() /
                 ("raft-libraries-" +
                  std::to_string(std::chrono::steady_clock::now().time_since_epoch().count()));
    game.DoConnect();
    game.loadStaticLibraries(cache, "server A");
    game.gotMsgOnConnection(true);
    game.gotMsgOnLogin();
    const std::pair<const char*, const char*> libraries[] = {
        {"lib_quest_type_data", "quest_type_data"},
        {"lib_mat_data", "material_data"},
        {"lib_bp_data", "blueprint_data"},
        {"lib_go_data", "gridobject_data"},
        {"lib_zone_data", "zone_data"},
        {"lib_discovery_data", "discovery_data"},
        {"lib_store_cur_data", "store_currency_data"},
        {"lib_store_group_data", "store_group_data"},
        {"lib_store_item_data", "store_item_data"}};
    sfs::SFSObjectWrapper timestamp;
    timestamp.put("timestamp", int64_t(123456));
    game.gotMsgOnExtensionResponse("lib_timestamp", timestamp);
    game.saveStaticLibraries();
    check(!std::filesystem::exists(cache));
    for (auto [command, key] : libraries) {
        sfs::SFSObjectWrapper library;
        library.put(key, std::make_shared<sfs::SFSArrayWrapper>());
        game.gotMsgOnExtensionResponse(command, library);
    }
    game.saveStaticLibraries();
    check(std::filesystem::exists(cache));
    rp::RaftPirates restored;
    restored.DoConnect();
    restored.loadStaticLibraries(cache, "server A");
    check(restored.libraryTimestamp == 123456 && !restored.ready);
    check(restored.login("10", "session").payload.getSFSObj("p")->getLong("last_updated") ==
          123456);
    restored.DoConnect();
    restored.loadStaticLibraries(cache, "server B");
    check(restored.libraryTimestamp == 0);
    std::filesystem::remove(cache);
}
