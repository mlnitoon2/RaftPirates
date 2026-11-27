#pragma once
#include "sys/msg/MsgReceiver.h"
#include "sfs/SFSObjectWrapper.h"
#include "rp/Quest.h"
#include "rp/StoreDB.h"
#include <map>

namespace rp {
    // Portable message lifetimes, NOT the original ARM/SWIG object layout.
    // For messages whose fields have not been established, retain the supplied payload
    // as an owned SFS object instead of inventing pointer fields.
    struct OpaqueGameMessage : MsgBase {
        sfs::SFSObjectWrapper payload;
    };

    struct MsgConfirmMaterialPurchaseInfo : MsgBase {
        MsgConfirmMaterialPurchaseInfo(int material,int count,int gold);
        ~MsgConfirmMaterialPurchaseInfo() override;
        int materialId=0,amount=0,goldPrice=0;
    };

    struct MsgQuestData : MsgBase {
        MsgQuestData(const Quest& value);
        ~MsgQuestData() override;
        Quest quest;
    };

    struct MsgQuestEvent : MsgBase {
        MsgQuestEvent(int id,int value);
        ~MsgQuestEvent() override;
        int questId=0,progress=0;
    };

    struct MsgStoreItemDisplayResponse : MsgBase {
        MsgStoreItemDisplayResponse(const StoreItem& value);
        ~MsgStoreItemDisplayResponse() override;
        StoreItem item;
    };

    struct MsgTabbedItemScrollTo : MsgBase {
        explicit MsgTabbedItemScrollTo(const std::string&);
        ~MsgTabbedItemScrollTo() override;
        std::string item;
    };

    struct MsgAnimateGridSprite : MsgBase {
        ~MsgAnimateGridSprite() override;
        int parent = 0, id = 0; std::vector<std::string> animations;
    };

    struct MsgAttackRequest : OpaqueGameMessage {
        ~MsgAttackRequest() override;
    };

    struct MsgAttackRequestFailed : OpaqueGameMessage {
        ~MsgAttackRequestFailed() override;
    };

    struct MsgAuthenticationChanged : OpaqueGameMessage {
        ~MsgAuthenticationChanged() override;
    };

    struct MsgAuthenticationFailed : OpaqueGameMessage {
        ~MsgAuthenticationFailed() override;
    };

    struct MsgAutoSafeModeRequest : MsgBase {
        ~MsgAutoSafeModeRequest() override;
    };

    struct MsgAutoSafeModeResponse : MsgBase {
        ~MsgAutoSafeModeResponse() override;
        bool enabled = false;
    };

    struct MsgBadZoneStatusRequest : MsgBase {
        ~MsgBadZoneStatusRequest() override;
    };

    struct MsgBadZoneStatusResponse : MsgBase {
        ~MsgBadZoneStatusResponse() override;
        bool bad = false;
    };

    struct MsgBattleConfirmSwapStance : MsgBase {
        ~MsgBattleConfirmSwapStance() override;
    };

    struct MsgBattleInfoDropdown : OpaqueGameMessage {
        ~MsgBattleInfoDropdown() override;
    };

    struct MsgBattleInfoRequest : OpaqueGameMessage {
        ~MsgBattleInfoRequest() override;
    };

    struct MsgBattleNameResponse : MsgBase {
        ~MsgBattleNameResponse() override;
        std::string name;
    };

    struct MsgBattleOverConfirm : MsgBase {
        ~MsgBattleOverConfirm() override;
    };

    struct MsgBattlePlacementState : OpaqueGameMessage {
        ~MsgBattlePlacementState() override;
    };

    struct MsgBattleReport : OpaqueGameMessage {
        ~MsgBattleReport() override;
    };

    struct MsgBattleScreenRaftsSelected : MsgBase {
        ~MsgBattleScreenRaftsSelected() override;
        int x = 0, y = 0;
    };

    struct MsgBattleTurnStart : OpaqueGameMessage {
        ~MsgBattleTurnStart() override;
    };

    struct MsgBlueprintDisplayRequest : MsgBase {
        ~MsgBlueprintDisplayRequest() override;
        int blueprintId = 0;
    };

    struct MsgBlueprintDisplayResponse : OpaqueGameMessage {
        ~MsgBlueprintDisplayResponse() override;
    };

    struct MsgBlueprintPageResponse : MsgBase {
        ~MsgBlueprintPageResponse() override;
        int page = 0;
    };

    struct MsgBuildCancel : MsgBase {
        ~MsgBuildCancel() override;
    };

    struct MsgBuildConfirm : OpaqueGameMessage {
        ~MsgBuildConfirm() override;
    };

    struct MsgBuildMenuInformationResponse : OpaqueGameMessage {
        ~MsgBuildMenuInformationResponse() override;
    };

    struct MsgBuildModeToggleView : MsgBase {
        ~MsgBuildModeToggleView() override;
    };

    struct MsgCheat : MsgBase {
        ~MsgCheat() override;
        std::string type;
    };

    struct MsgCheatDuplicatePlayer : MsgBase {
        ~MsgCheatDuplicatePlayer() override;
        std::string username;
    };

    struct MsgCheatStartBattle : MsgBase {
        ~MsgCheatStartBattle() override;
        std::string username;
    };

    struct MsgCheckForHoles : MsgBase {
        ~MsgCheckForHoles() override;
    };

    struct MsgCheckForNewDailyQuest : MsgBase {
        ~MsgCheckForNewDailyQuest() override;
    };

    struct MsgCheckForNewDailyQuestResult : OpaqueGameMessage {
        ~MsgCheckForNewDailyQuestResult() override;
    };

    struct MsgCollectedBlueprint : MsgBase {
        ~MsgCollectedBlueprint() override;
        std::string popupTitle, image, spriteSheet, text; int blueprintId = 0;
    };

    struct MsgCollectedCourage : MsgBase {
        ~MsgCollectedCourage() override;
        int amount = 0;
    };

    struct MsgCollectedFastclocks : MsgBase {
        ~MsgCollectedFastclocks() override;
        std::string popupTitle; int numFastclocks = 0;
    };

    struct MsgCollectedMapMove : MsgBase {
        ~MsgCollectedMapMove() override;
        std::string popupTitle; int numMapMove = 0;
    };

    struct MsgCollectedMaterials : MsgBase {
        ~MsgCollectedMaterials() override;
        std::string popupTitle; std::vector<int> materialIds, materialAmts;
    };

    struct MsgCollectedSonar : MsgBase {
        ~MsgCollectedSonar() override;
        std::string popupTitle; int numSonar = 0;
    };

    struct MsgConfirmBlueprintMaterialsPurchase : MsgBase {
        ~MsgConfirmBlueprintMaterialsPurchase() override;
        int blueprintId = 0, goldPrice = 0;
    };

    struct MsgConfirmMapMovement : MsgBase {
        ~MsgConfirmMapMovement() override;
        bool confirmed = false;
    };

    struct MsgConfirmMaterialPurchase : MsgBase {
        ~MsgConfirmMaterialPurchase() override;
        int materialId = 0, amount = 0, goldPrice = 0;
    };

    struct MsgConfirmRepairGridObjectWithGold : MsgBase {
        ~MsgConfirmRepairGridObjectWithGold() override;
        int parent = 0, object = 0, goldCost = 0; bool all = false;
    };

    struct MsgConfirmZoneMovement : MsgBase {
        ~MsgConfirmZoneMovement() override;
        bool confirmed = false;
    };

    struct MsgDaringMeterAdvance : MsgBase {
        ~MsgDaringMeterAdvance() override;
    };

    struct MsgDaringMeterAdvanceResult : OpaqueGameMessage {
        ~MsgDaringMeterAdvanceResult() override;
    };

    struct MsgDaringMeterInfo : MsgBase {
        ~MsgDaringMeterInfo() override;
    };

    struct MsgDaringMeterInfoResponse : OpaqueGameMessage {
        ~MsgDaringMeterInfoResponse() override;
    };

    struct MsgDaringMeterPercentResponse : OpaqueGameMessage {
        ~MsgDaringMeterPercentResponse() override;
    };

    struct MsgDebugGatherAllUsers : MsgBase {
        ~MsgDebugGatherAllUsers() override;
    };

    struct MsgDebugGatherAllUsersResult : OpaqueGameMessage {
        ~MsgDebugGatherAllUsersResult() override;
    };

    struct MsgDeselectObject : MsgBase {
        ~MsgDeselectObject() override;
    };

    struct MsgDestroyObjectRequest : OpaqueGameMessage {
        ~MsgDestroyObjectRequest() override;
    };

    struct MsgDiscoveryGetAllKnownRequest : MsgBase {
        ~MsgDiscoveryGetAllKnownRequest() override;
    };

    struct MsgDiscoveryGetAllKnownResult : MsgBase {
        ~MsgDiscoveryGetAllKnownResult() override;
        std::string discoveries;
    };

    struct MsgDiscoveryMoveTo : MsgBase {
        ~MsgDiscoveryMoveTo() override;
        int id = 0;
    };

    struct MsgDiscoveryMoveToResult : MsgBase {
        ~MsgDiscoveryMoveToResult() override;
        bool success = false;
    };

    struct MsgDiveDepthChange : MsgBase {
        ~MsgDiveDepthChange() override;
        int depth = 0, maximum = 0;
    };

    struct MsgDiveUnitLoadChange : MsgBase {
        ~MsgDiveUnitLoadChange() override;
        int parent = 0, id = 0, load = 0, maximum = 0;
    };

    struct MsgDivingGameDone : MsgBase {
        ~MsgDivingGameDone() override;
    };

    struct MsgDivingGameDoneMoving : OpaqueGameMessage {
        ~MsgDivingGameDoneMoving() override;
    };

    struct MsgDivingGameShow : OpaqueGameMessage {
        ~MsgDivingGameShow() override;
    };

    struct MsgDoingFacebookAuth : MsgBase {
        ~MsgDoingFacebookAuth() override;
    };

    struct MsgEnableNetworkProcessing : MsgBase {
        ~MsgEnableNetworkProcessing() override;
        bool enabled = true;
    };

    struct MsgExitBattleModeRequest : MsgBase {
        ~MsgExitBattleModeRequest() override;
    };

    struct MsgGetAllBlueprintsResponse : MsgBase {
        ~MsgGetAllBlueprintsResponse() override;
        std::vector<int> blueprints;
    };

    struct MsgGetAvailBlueprintsResponse : MsgBase {
        ~MsgGetAvailBlueprintsResponse() override;
        std::vector<int> blueprints;
    };

    struct MsgGetAvailMaterialsResponse : MsgBase {
        ~MsgGetAvailMaterialsResponse() override;
        std::vector<int> materials;
    };

    struct MsgGetAvailStoreItemsResponse : MsgBase {
        ~MsgGetAvailStoreItemsResponse() override;
        std::vector<int> items;
    };

    struct MsgGetMapActionState : MsgBase {
        ~MsgGetMapActionState() override;
    };

    struct MsgGetMapActionStateUpdate : MsgBase {
        ~MsgGetMapActionStateUpdate() override;
        int count = 0, maxcount = 0, time = 0;
    };

    struct MsgGridObjectEvent : OpaqueGameMessage {
        ~MsgGridObjectEvent() override;
    };

    struct MsgGridObjectRepairInfo : MsgBase {
        ~MsgGridObjectRepairInfo() override;
        int id = 0, goldCost = 0; bool all = false;
    };

    struct MsgGridObjectSelected : MsgBase {
        ~MsgGridObjectSelected() override;
        int parent = 0, id = 0;
    };

    struct MsgHaveMaterialsRequest : MsgBase {
        ~MsgHaveMaterialsRequest() override;
        int blueprintId = 0;
    };

    struct MsgHaveMaterialsResponse : MsgBase {
        ~MsgHaveMaterialsResponse() override;
        bool haveMaterials = false;
    };

    struct MsgHealthLevelRequest : OpaqueGameMessage {
        ~MsgHealthLevelRequest() override;
    };

    struct MsgHealthLevelResponse : MsgBase {
        ~MsgHealthLevelResponse() override;
        float health = 0;
    };

    struct MsgHidePirateMap : MsgBase {
        ~MsgHidePirateMap() override;
    };

    struct MsgHighlightHudItem : MsgBase {
        ~MsgHighlightHudItem() override;
        std::string item;
    };

    struct MsgHudModeResponse : MsgBase {
        ~MsgHudModeResponse() override;
        int mode = 0;
    };

    struct MsgLoadFinished : MsgBase {
        ~MsgLoadFinished() override;
    };

    struct MsgLoadPercentage : MsgBase {
        ~MsgLoadPercentage() override;
        float percentage = 0;
    };

    struct MsgLoadPreStart : MsgBase {
        ~MsgLoadPreStart() override;
    };

    struct MsgLoadStart : MsgBase {
        ~MsgLoadStart() override;
    };

    struct MsgLoadingAnimFinished : MsgBase {
        ~MsgLoadingAnimFinished() override;
    };

    struct MsgLocalizedInsertText : MsgBase {
        ~MsgLocalizedInsertText() override;
        std::string text;
    };

    struct MsgMapMarkerMoved : MsgBase {
        ~MsgMapMarkerMoved() override;
        int x = 0, y = 0, enemy = 0; bool safe = false, sonar = false;
    };

    struct MsgMaterialInformationRequest : MsgBase {
        ~MsgMaterialInformationRequest() override;
        int materialId = 0;
    };

    struct MsgMaterialInformationResponse : OpaqueGameMessage {
        ~MsgMaterialInformationResponse() override;
    };

    struct MsgMaterialsChanged : OpaqueGameMessage {
        ~MsgMaterialsChanged() override;
    };

    struct MsgMenuBattleAccept : MsgBase {
        ~MsgMenuBattleAccept() override;
        bool accept = false;
    };

    struct MsgMenuLogoutRequest : MsgBase {
        ~MsgMenuLogoutRequest() override;
    };

    struct MsgMoveObjectRequest : MsgBase {
        ~MsgMoveObjectRequest() override;
        int parent = 0, object = 0, newParent = 0, x = 0, y = 0;
    };

    struct MsgNameShip : MsgBase {
        ~MsgNameShip() override;
    };

    struct MsgNextWreckedRaft : MsgBase {
        ~MsgNextWreckedRaft() override;
    };

    struct MsgNextWreckedRaftComponent : MsgBase {
        ~MsgNextWreckedRaftComponent() override;
    };

    struct MsgNumWreckedRaftComponents : MsgBase {
        ~MsgNumWreckedRaftComponents() override;
        int count = 0;
    };

    struct MsgNumWreckedRafts : MsgBase {
        ~MsgNumWreckedRafts() override;
        int count = 0;
    };

    struct MsgOceanSafetyUpdate : MsgBase {
        ~MsgOceanSafetyUpdate() override;
        bool safe = true;
    };

    struct MsgOceanViewPickResult : MsgBase {
        ~MsgOceanViewPickResult() override;
        float x = 0, y = 0;
    };

    struct MsgPirateMapDiscoveries_GetAvail_Response : OpaqueGameMessage {
        ~MsgPirateMapDiscoveries_GetAvail_Response() override;
    };

    struct MsgPirateMapDiscoveries_Info_Request : OpaqueGameMessage {
        ~MsgPirateMapDiscoveries_Info_Request() override;
    };

    struct MsgPirateMapDiscoveries_Info_Response : OpaqueGameMessage {
        ~MsgPirateMapDiscoveries_Info_Response() override;
    };

    struct MsgPirateMapDiscoveries_Page_Response : OpaqueGameMessage {
        ~MsgPirateMapDiscoveries_Page_Response() override;
    };

    struct MsgPlayerBuiltRaft : OpaqueGameMessage {
        ~MsgPlayerBuiltRaft() override;
    };

    struct MsgPreviousWreckedRaft : MsgBase {
        ~MsgPreviousWreckedRaft() override;
    };

    struct MsgPreviousWreckedRaftComponent : MsgBase {
        ~MsgPreviousWreckedRaftComponent() override;
    };

    struct MsgPurchaseItemRequest : MsgBase {
        ~MsgPurchaseItemRequest() override;
        int item = 0;
    };

    struct MsgQuestInformationResponse : OpaqueGameMessage {
        ~MsgQuestInformationResponse() override;
    };

    struct MsgQuestOpened : MsgBase {
        ~MsgQuestOpened() override;
        int id = 0;
    };

    struct MsgQuestRead : MsgBase {
        ~MsgQuestRead() override;
        int id = 0;
    };

    struct MsgQuestTypeInformationRequest : MsgBase {
        ~MsgQuestTypeInformationRequest() override;
        int id = 0;
    };

    struct MsgQuestTypeInformationResponse : OpaqueGameMessage {
        ~MsgQuestTypeInformationResponse() override;
    };

    struct MsgQueueMenuPopup : MsgBase {
        ~MsgQueueMenuPopup() override;
        std::string menu; bool modal = true; std::shared_ptr<MsgBase> data;
    };

    struct MsgQuickTravelGetAllKnownRequest : MsgBase {
        ~MsgQuickTravelGetAllKnownRequest() override;
    };

    struct MsgQuickTravelGetAllKnownResult : MsgBase {
        ~MsgQuickTravelGetAllKnownResult() override;
        std::string zones, discoveries;
    };

    struct MsgRecipeInformationRequest : MsgBase {
        ~MsgRecipeInformationRequest() override;
        int blueprintId = 0;
    };

    struct MsgRecipeInformationResponse : MsgBase {
        ~MsgRecipeInformationResponse() override;
        std::map<int,int> recipe;
    };

    struct MsgReconnect : MsgBase {
        ~MsgReconnect() override;
    };

    struct MsgReenablePingSonar : MsgBase {
        ~MsgReenablePingSonar() override;
    };

    struct MsgRepairSelectedObject : MsgBase {
        ~MsgRepairSelectedObject() override;
    };

    struct MsgRepairSelectedObjectAll : MsgBase {
        ~MsgRepairSelectedObjectAll() override;
    };

    struct MsgRequestBattlePlacementState : MsgBase {
        ~MsgRequestBattlePlacementState() override;
    };

    struct MsgRequestNumWreckedRaftComponents : MsgBase {
        ~MsgRequestNumWreckedRaftComponents() override;
    };

    struct MsgRequestNumWreckedRafts : MsgBase {
        ~MsgRequestNumWreckedRafts() override;
    };

    struct MsgRequestOceanSafetyUpdate : MsgBase {
        ~MsgRequestOceanSafetyUpdate() override;
    };

    struct MsgRequestPingSonar : MsgBase {
        ~MsgRequestPingSonar() override;
    };

    struct MsgSafetyFlagTimerUpdate : OpaqueGameMessage {
        ~MsgSafetyFlagTimerUpdate() override;
    };

    struct MsgSalvageSelectedObject : MsgBase {
        ~MsgSalvageSelectedObject() override;
    };

    struct MsgSelectedObjectAvailableActions : MsgBase {
        ~MsgSelectedObjectAvailableActions() override;
        std::vector<std::string> actions;
    };

    struct MsgSetAutoSafeMode : MsgBase {
        ~MsgSetAutoSafeMode() override;
        bool enabled = false;
    };

    struct MsgSetBlueprintPage : MsgBase {
        ~MsgSetBlueprintPage() override;
        int page = 0;
    };

    struct MsgSetBlueprintSelected : MsgBase {
        ~MsgSetBlueprintSelected() override;
        int blueprintId = 0;
    };

    struct MsgSetHudModeRequest : MsgBase {
        ~MsgSetHudModeRequest() override;
        int mode = 0;
    };

    struct MsgSetStoreCategory : MsgBase {
        ~MsgSetStoreCategory() override;
        int category = 0;
    };

    struct MsgSetStoreItemPage : MsgBase {
        ~MsgSetStoreItemPage() override;
        int page = 0;
    };

    struct MsgSetStoreItemSelected : MsgBase {
        ~MsgSetStoreItemSelected() override;
        int storeItemSelected = 0;
    };

    struct MsgSetupDaringPopup : OpaqueGameMessage {
        ~MsgSetupDaringPopup() override;
    };

    struct MsgSetupRankupPopup : OpaqueGameMessage {
        ~MsgSetupRankupPopup() override;
    };

    struct MsgSetupRepairAndSalvageDialog : OpaqueGameMessage {
        ~MsgSetupRepairAndSalvageDialog() override;
    };

    struct MsgSetupStatusPopup : OpaqueGameMessage {
        ~MsgSetupStatusPopup() override;
    };

    struct MsgShowBattlePointsRequest : MsgBase {
        ~MsgShowBattlePointsRequest() override;
    };

    struct MsgShowNewDailyQuest : MsgBase {
        ~MsgShowNewDailyQuest() override;
    };

    struct MsgShowPirateInfo : OpaqueGameMessage {
        ~MsgShowPirateInfo() override;
    };

    struct MsgShowPirateMap : MsgBase {
        ~MsgShowPirateMap() override;
    };

    struct MsgShowStore : MsgBase {
        ~MsgShowStore() override;
        std::string category;
    };

    struct MsgSimpleRequest : MsgBase {
        ~MsgSimpleRequest() override;
        int type = 0;
    };

    struct MsgSparseGridObjectAdded : OpaqueGameMessage {
        ~MsgSparseGridObjectAdded() override;
    };

    struct MsgSparseGridObjectRemoved : OpaqueGameMessage {
        ~MsgSparseGridObjectRemoved() override;
    };

    struct MsgSpecificMaterialAvailAmountRequest : MsgBase {
        ~MsgSpecificMaterialAvailAmountRequest() override;
        int materialId = 0;
    };

    struct MsgSpecificMaterialAvailAmountResponse : MsgBase {
        ~MsgSpecificMaterialAvailAmountResponse() override;
        int materialId = 0, amount = 0;
    };

    struct MsgStartBattleMode : MsgBase {
        ~MsgStartBattleMode() override;
    };

    struct MsgStartBuildMode : MsgBase {
        ~MsgStartBuildMode() override;
    };

    struct MsgStartBuildRequest : MsgBase {
        ~MsgStartBuildRequest() override;
        int blueprintId = 0;
    };

    struct MsgStartTransition : MsgBase {
        ~MsgStartTransition() override;
        bool show = false;
    };

    struct MsgStoreCategoryResponse : MsgBase {
        ~MsgStoreCategoryResponse() override;
        int category = 0;
    };

    struct MsgStoreDisplayStatus : MsgBase {
        ~MsgStoreDisplayStatus() override;
        bool visible = false;
    };

    struct MsgStoreItemDisplayRequest : MsgBase {
        ~MsgStoreItemDisplayRequest() override;
        int storeItemSelected = 0;
    };

    struct MsgStoreItemSelectedResponse : MsgBase {
        ~MsgStoreItemSelectedResponse() override;
        int storeItemSelected = 0;
    };

    struct MsgStorePageResponse : MsgBase {
        ~MsgStorePageResponse() override;
        int page = 0;
    };

    struct MsgToPositioningView : MsgBase {
        ~MsgToPositioningView() override;
    };

    struct MsgToggleBlockHudItem : MsgBase {
        ~MsgToggleBlockHudItem() override;
        std::vector<std::string> block, unblock;
    };

    struct MsgTransitionDone : MsgBase {
        ~MsgTransitionDone() override;
    };

    struct MsgTreasureMapGotOne : MsgBase {
        ~MsgTreasureMapGotOne() override;
        std::string popupTitle, type; int typedata = 0;
    };

    struct MsgTutorialStartBattleMove : OpaqueGameMessage {
        ~MsgTutorialStartBattleMove() override;
    };

    struct MsgUpdateCurrencyResponse : OpaqueGameMessage {
        ~MsgUpdateCurrencyResponse() override;
    };

    struct MsgUpdateOceanView : MsgBase {
        ~MsgUpdateOceanView() override;
    };

    struct MsgUseClockOnSelectedObject : MsgBase {
        ~MsgUseClockOnSelectedObject() override;
    };

    struct MsgWreckageSelected : OpaqueGameMessage {
        ~MsgWreckageSelected() override;
    };

    struct MsgZoneGetAllKnownRequest : MsgBase {
        ~MsgZoneGetAllKnownRequest() override;
    };

    struct MsgZoneGetAllKnownResult : MsgBase {
        ~MsgZoneGetAllKnownResult() override;
        std::string zones;
    };

    struct MsgZoneMoveTo : MsgBase {
        ~MsgZoneMoveTo() override;
        int id = 0;
    };

    struct MsgZoneMoveToResult : MsgBase {
        ~MsgZoneMoveToResult() override;
        bool success = false;
    };
} // namespace rp
