# Native message inventory

All 172 originally listed message destructors are now represented by named C++ types in rp/Messages.h and out-of-line default destructors in Messages.cpp. The portable ABI is intentionally different from the original Ref/SWIG layouts. The default destructors release their owned standard-library or SFS values.

## Typed adapters

- `MsgAnimateGridSprite`: int parent = 0, id = 0; std::vector<std::string> animations;
- `MsgAutoSafeModeResponse`: bool enabled = false;
- `MsgBadZoneStatusResponse`: bool bad = false;
- `MsgBattleNameResponse`: std::string name;
- `MsgBattleScreenRaftsSelected`: int x = 0, y = 0;
- `MsgBlueprintDisplayRequest`: int blueprintId = 0;
- `MsgBlueprintPageResponse`: int page = 0;
- `MsgCheat`: std::string type;
- `MsgCheatDuplicatePlayer`: std::string username;
- `MsgCheatStartBattle`: std::string username;
- `MsgCollectedBlueprint`: std::string popupTitle, image, spriteSheet, text; int blueprintId = 0;
- `MsgCollectedCourage`: int amount = 0;
- `MsgCollectedFastclocks`: std::string popupTitle; int numFastclocks = 0;
- `MsgCollectedMapMove`: std::string popupTitle; int numMapMove = 0;
- `MsgCollectedMaterials`: std::string popupTitle; std::vector<int> materialIds, materialAmts;
- `MsgCollectedSonar`: std::string popupTitle; int numSonar = 0;
- `MsgConfirmBlueprintMaterialsPurchase`: int blueprintId = 0, goldPrice = 0;
- `MsgConfirmMapMovement`: bool confirmed = false;
- `MsgConfirmMaterialPurchase`: int materialId = 0, amount = 0, goldPrice = 0;
- `MsgConfirmRepairGridObjectWithGold`: int parent = 0, object = 0, goldCost = 0; bool all = false;
- `MsgConfirmZoneMovement`: bool confirmed = false;
- `MsgDiscoveryGetAllKnownResult`: std::string discoveries;
- `MsgDiscoveryMoveTo`: int id = 0;
- `MsgDiscoveryMoveToResult`: bool success = false;
- `MsgDiveDepthChange`: int depth = 0, maximum = 0;
- `MsgDiveUnitLoadChange`: int parent = 0, id = 0, load = 0, maximum = 0;
- `MsgEnableNetworkProcessing`: bool enabled = true;
- `MsgGetAllBlueprintsResponse`: std::vector<int> blueprints;
- `MsgGetAvailBlueprintsResponse`: std::vector<int> blueprints;
- `MsgGetAvailMaterialsResponse`: std::vector<int> materials;
- `MsgGetAvailStoreItemsResponse`: std::vector<int> items;
- `MsgGetMapActionStateUpdate`: int count = 0, maxcount = 0, time = 0;
- `MsgGridObjectRepairInfo`: int id = 0, goldCost = 0; bool all = false;
- `MsgGridObjectSelected`: int parent = 0, id = 0;
- `MsgHaveMaterialsRequest`: int blueprintId = 0;
- `MsgHaveMaterialsResponse`: bool haveMaterials = false;
- `MsgHealthLevelResponse`: float health = 0;
- `MsgHighlightHudItem`: std::string item;
- `MsgHudModeResponse`: int mode = 0;
- `MsgLoadPercentage`: float percentage = 0;
- `MsgLocalizedInsertText`: std::string text;
- `MsgMapMarkerMoved`: int x = 0, y = 0, enemy = 0; bool safe = false, sonar = false;
- `MsgMaterialInformationRequest`: int materialId = 0;
- `MsgMenuBattleAccept`: bool accept = false;
- `MsgMoveObjectRequest`: int parent = 0, object = 0, newParent = 0, x = 0, y = 0;
- `MsgNumWreckedRaftComponents`: int count = 0;
- `MsgNumWreckedRafts`: int count = 0;
- `MsgOceanSafetyUpdate`: bool safe = true;
- `MsgOceanViewPickResult`: float x = 0, y = 0;
- `MsgPurchaseItemRequest`: int item = 0;
- `MsgQuestOpened`: int id = 0;
- `MsgQuestRead`: int id = 0;
- `MsgQuestTypeInformationRequest`: int id = 0;
- `MsgQueueMenuPopup`: std::string menu; bool modal = true; std::shared_ptr<MsgBase> data;
- `MsgQuickTravelGetAllKnownResult`: std::string zones, discoveries;
- `MsgRecipeInformationRequest`: int blueprintId = 0;
- `MsgRecipeInformationResponse`: std::map<int,int> recipe;
- `MsgSelectedObjectAvailableActions`: std::vector<std::string> actions;
- `MsgSetAutoSafeMode`: bool enabled = false;
- `MsgSetBlueprintPage`: int page = 0;
- `MsgSetBlueprintSelected`: int blueprintId = 0;
- `MsgSetHudModeRequest`: int mode = 0;
- `MsgSetStoreCategory`: int category = 0;
- `MsgSetStoreItemPage`: int page = 0;
- `MsgSetStoreItemSelected`: int storeItemSelected = 0;
- `MsgShowStore`: std::string category;
- `MsgSimpleRequest`: int type = 0;
- `MsgSpecificMaterialAvailAmountRequest`: int materialId = 0;
- `MsgSpecificMaterialAvailAmountResponse`: int materialId = 0, amount = 0;
- `MsgStartBuildRequest`: int blueprintId = 0;
- `MsgStartTransition`: bool show = false;
- `MsgStoreCategoryResponse`: int category = 0;
- `MsgStoreDisplayStatus`: bool visible = false;
- `MsgStoreItemDisplayRequest`: int storeItemSelected = 0;
- `MsgStoreItemSelectedResponse`: int storeItemSelected = 0;
- `MsgStorePageResponse`: int page = 0;
- `MsgToggleBlockHudItem`: std::vector<std::string> block, unblock;
- `MsgTreasureMapGotOne`: std::string popupTitle, type; int typedata = 0;
- `MsgZoneGetAllKnownResult`: std::string zones;
- `MsgZoneMoveTo`: int id = 0;
- `MsgZoneMoveToResult`: bool success = false;

## No-argument event adapters

`MsgAutoSafeModeRequest`, `MsgBadZoneStatusRequest`, `MsgBattleConfirmSwapStance`, `MsgBattleOverConfirm`, `MsgBuildCancel`, `MsgBuildModeToggleView`, `MsgCheckForHoles`, `MsgCheckForNewDailyQuest`, `MsgDaringMeterAdvance`, `MsgDaringMeterInfo`, `MsgDebugGatherAllUsers`, `MsgDeselectObject`, `MsgDiscoveryGetAllKnownRequest`, `MsgDivingGameDone`, `MsgDoingFacebookAuth`, `MsgExitBattleModeRequest`, `MsgGetMapActionState`, `MsgHidePirateMap`, `MsgLoadFinished`, `MsgLoadPreStart`, `MsgLoadStart`, `MsgLoadingAnimFinished`, `MsgMenuLogoutRequest`, `MsgNameShip`, `MsgNextWreckedRaft`, `MsgNextWreckedRaftComponent`, `MsgPreviousWreckedRaft`, `MsgPreviousWreckedRaftComponent`, `MsgQuickTravelGetAllKnownRequest`, `MsgReconnect`, `MsgReenablePingSonar`, `MsgRepairSelectedObject`, `MsgRepairSelectedObjectAll`, `MsgRequestBattlePlacementState`, `MsgRequestNumWreckedRaftComponents`, `MsgRequestNumWreckedRafts`, `MsgRequestOceanSafetyUpdate`, `MsgRequestPingSonar`, `MsgSalvageSelectedObject`, `MsgShowBattlePointsRequest`, `MsgShowNewDailyQuest`, `MsgShowPirateMap`, `MsgStartBattleMode`, `MsgStartBuildMode`, `MsgToPositioningView`, `MsgTransitionDone`, `MsgUpdateOceanView`, `MsgUseClockOnSelectedObject`, `MsgZoneGetAllKnownRequest`

## Opaque payload, original fields still unresolved

`MsgAttackRequest`, `MsgAttackRequestFailed`, `MsgAuthenticationChanged`, `MsgAuthenticationFailed`, `MsgBattleInfoDropdown`, `MsgBattleInfoRequest`, `MsgBattlePlacementState`, `MsgBattleReport`, `MsgBattleTurnStart`, `MsgBlueprintDisplayResponse`, `MsgBuildConfirm`, `MsgBuildMenuInformationResponse`, `MsgCheckForNewDailyQuestResult`, `MsgDaringMeterAdvanceResult`, `MsgDaringMeterInfoResponse`, `MsgDaringMeterPercentResponse`, `MsgDebugGatherAllUsersResult`, `MsgDestroyObjectRequest`, `MsgDivingGameDoneMoving`, `MsgDivingGameShow`, `MsgGridObjectEvent`, `MsgHealthLevelRequest`, `MsgMaterialInformationResponse`, `MsgMaterialsChanged`, `MsgPirateMapDiscoveries_GetAvail_Response`, `MsgPirateMapDiscoveries_Info_Request`, `MsgPirateMapDiscoveries_Info_Response`, `MsgPirateMapDiscoveries_Page_Response`, `MsgPlayerBuiltRaft`, `MsgQuestInformationResponse`, `MsgQuestTypeInformationResponse`, `MsgSafetyFlagTimerUpdate`, `MsgSetupDaringPopup`, `MsgSetupRankupPopup`, `MsgSetupRepairAndSalvageDialog`, `MsgSetupStatusPopup`, `MsgShowPirateInfo`, `MsgSparseGridObjectAdded`, `MsgSparseGridObjectRemoved`, `MsgTutorialStartBattleMove`, `MsgUpdateCurrencyResponse`, `MsgWreckageSelected`

These messages store an SFSObjectWrapper instead of guessing original field offsets. They need native dispatcher and SWIG binding work.

Original ARM evidence: QuestRewards at 0x4ae048 reads quest_title, materials_changed (id, amt), blueprints_added (blueprint_id), fastclocks_added/num_fastclocks (material 10), sonar_added/num_sonar (material 12), mapmove_added/num_mapmove, treasuremaps. Original menus listen for MsgCollectedMaterials, MsgCollectedBlueprint, MsgCollectedFastclocks, MsgCollectedSonar, MsgCollectedMapMove and MsgTreasureMapGotOne. The adapter uses the existing server connection and typed notification queue. It never grants rewards from local progress alone. Original display names/fields were recovered from APK menu XML; not every inferred portable field has been verified.

Debug functions (0x4bbb14, 0x4b07ec, 0x4bc6e4, 0x4ad8ac) are only available through explicit opt-in on a loopback test server. They are not automatically dispatched. Server-side authorization remains mandatory.

