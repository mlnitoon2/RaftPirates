# Remaining reconstruction

Pass 33: 503/1191 original qualified method names matched (42.2334%). 688 names remain absent. Target: 50%.

The comparison uses the original ARM32 ELF and the corresponding compiled host static archive. Matching a name does not prove signature, ABI or functional equivalence. Methods with existing names may still be partial. The list below is a symbol inventory, not a list of verified missing player-facing features.

## Priority gaps

- Original menu scripts: complete game/SWIG bindings, message dispatch, dynamic templates, tutorial and quest flows, all popups and transitions.
- Construction: original placement sprites, completion notifications, barracks, repair/salvage calculations, complete construction/upgrade flow and animation.
- Ocean: persistent sprite lifecycle, picking, particles, debris/wreckage, discovery interactions, accurate map confirmations and treasure movement.
- Combat: original battle HUD callbacks, placement, targeting, dice and turn synchronization, animation, plunder, boss-specific progression and real multiplayer integration.
- Account/social/store: original saved login migration, account management, Facebook/social integration, currency purchases, notifications.
- Server: validation, full multiplayer and boss extensions and persistence require a compatible backend. No live Android or multiplayer validation has been performed.

## Missing method names by class

### `rp::OceanView` (37)

`AddAnimSlack`, `AddGrid`, `AddSprite`, `AddToTick`, `CheckForBucket`, `FillPrioritySorter`, `FocalPoint`, `GetPriority`, `GetSpritePriority`, `Init`, `Layer`, `OceanView`, `PlayRaftAttackAnims`, `RemoveFromTick`, `RemoveGrid`, `RemoveSprite`, `SetAxiseseses`, `SetBossOcean`, `SetOcean`, `SetOrdering`, `SetRaftMute`, `SetRaftTransparent`, `SetTempPriorities`, `ShowWaterBuckets`, `UpdateAnimations`, `UpdateSprite`, `getGridSprite`, `gotMsgAnimateGridSprite`, `gotMsgDiveUnitLoadChange`, `gotMsgGfxLayerPickResult`, `gotMsgGridObjectEvent`, `gotMsgSparseGridObjectAdded`, `gotMsgSparseGridObjectRemoved`, `gotMsgUpdateOceanView`, `removeAllHealthBars`, `showHealthBars`, `tick`

### `rp::RaftPirates` (32)

`GotMsgDoingFacebookAuth`, `GotMsgOnMenuLoadGlobal`, `GotMsgShowStore`, `GotMsgStartTransition`, `GotTransitionMsgDone`, `QuestRewards`, `checkTutorialCompletion`, `checkTutorialStart`, `gotMsgAuthenticationChanged`, `gotMsgBoughtCurrency`, `gotMsgCheat`, `gotMsgCheatDuplicatePlayer`, `gotMsgCheatStartBattle`, `gotMsgConfirmBlueprintMaterialsPurchase`, `gotMsgConfirmMaterialPurchase`, `gotMsgConfirmRepairGridObjectWithGold`, `gotMsgDebugGatherAllUsers`, `gotMsgDiscoveryGetAllKnownRequest`, `gotMsgGameCenterAuthChanged`, `gotMsgGameCenterError`, `gotMsgKeyboardEntryResult`, `gotMsgNameShip`, `gotMsgPopupTextResult`, `gotMsgQuickTravelGetAllKnownRequest`, `gotMsgReconnect`, `gotMsgStartBattleMode`, `gotMsgStartBuildMode`, `gotMsgTouchDown`, `gotMsgTouchDrag`, `gotMsgTouchUp`, `gotMsgZoneGetAllKnownRequest`, `preloadMenus`

### `rp::BuildModeState` (29)

`ApplyFootprintToRaft`, `ChangeView`, `CleanupFootprints`, `CreateGridObject`, `HandleExtensionResponse`, `Pause`, `SendBonusMaterials`, `SendMsgSelectedObjectAvailableActions`, `SetCheckForHoles`, `cancelNotificationForRaftComponent`, `doPlaceObject`, `gotMsgBuildModeToggleView`, `gotMsgCheckForHoles`, `gotMsgDivingGameDone`, `gotMsgDivingGameDoneMoving`, `gotMsgDivingGameShow`, `gotMsgGridObjectEvent`, `gotMsgHighlightHudItem`, `gotMsgLoadingAnimFinished`, `gotMsgOfferWallReward`, `gotMsgSparseGridObjectAdded`, `gotMsgToggleBlockHudItem`, `gotMsgTransitionDone`, `gotMsgWreckageSelected`, `name`, `repairObject`, `sendSelectedObjectRepairDataWithGold`, `setupNotificationForRaftComponent`, `tick`

### `rp::BattleModeState` (24)

`BattleModeState`, `HideDropdown`, `Pause`, `PlayBattleAnimations`, `RaftSelect`, `StartWaiting`, `StopWaiting`, `SwapPopup`, `SwapStance`, `ToBattleView`, `ToPositioningView`, `endBattleAndPlunder`, `gotMsgBattleConfirmSwapStance`, `gotMsgBattleInfoDropdown`, `gotMsgBattleInfoRequest`, `gotMsgBattleOverConfirm`, `gotMsgExitBattleModeRequest`, `gotMsgHighlightHudItem`, `gotMsgQuestRead`, `gotMsgToPositioningView`, `gotMsgToggleBlockHudItem`, `gotMsgTransitionDone`, `name`, `tick`

### `rp::Wreckage` (14)

`Duplicate`, `GetTotalBattlePoints`, `GetUserObjectID`, `GetWreckageObject`, `Pieces`, `RegisterAllClasses`, `Repair`, `Salvage`, `SetState`, `SetWreckageObject`, `Wreckage`, `getAvailableActions`, `initWithSFSObject`, `~Wreckage`

### `rp` (11)

`Dostuff_place`, `IsBarracks`, `IsBossWeapon`, `IsComponent`, `IsDivingUnit`, `IsRaft`, `IsWreckage`, `MyDataSortPredicate`, `SortGridObjPredicate`, `Tokenize`, `performaIsoSort`

### `rp::AccessorySprite` (11)

`AccessorySprite`, `AddToTick`, `GetID`, `GetTimeString`, `Init`, `ListenForAnimation`, `RemoveFromTick`, `SetScale`, `Update`, `tick`, `~AccessorySprite`

### `rp::Barracks` (11)

`AddedToOcean`, `Barracks`, `Constructing`, `Duplicate`, `SetConstructionInfo`, `SetState`, `TempTick`, `UseClock`, `getAvailableActions`, `initWithSFSObject`, `~Barracks`

### `rp::BattleShipPlacementSprite` (11)

`BossCoreRaftsByMe`, `BossCoreRaftsInMe`, `ContainsGfx`, `HasAccessorySprite`, `HighlightAttackRafts`, `Select`, `SetPosition`, `SetPriority`, `SetScale`, `SetToObject`, `UpdateParticles`

### `rp::ShipPlacementSprite` (11)

`ContainsGfx`, `OkayToPlace`, `Pieces`, `SetObjectPosition`, `SetPosition`, `SetPositionOfSprites`, `SetPriority`, `SetToObject`, `ShipPlacementSprite`, `Update`, `~ShipPlacementSprite`

### `rp::GridSprite` (10)

`AddAccessorySprite`, `BuildPiece`, `GetAccessorySprite`, `HasAccessorySprite`, `ListenForAnimation`, `RemoveAccessorySprite`, `SetMute`, `UpdateParticles`, `UpdatePiece`, `gotMsgAnimationStarted`

### `rp::PerimeterWallSprite` (10)

`AddBalloons`, `Init`, `PerimeterWallSprite`, `RemoveFromTick`, `SetPriorities`, `SetPriority`, `SetVisible`, `Update`, `tick`, `~PerimeterWallSprite`

### `rp::MapView` (9)

`gotMsgAnimationFinished`, `gotMsgConfirmMapMovement`, `gotMsgHidePirateMap`, `gotMsgShowPirateMap`, `gotMsgToggleBlockHudItem`, `gotMsgTutorialStartBattleMove`, `setPlayer`, `tutorialBattleStartShipMove`, `updatePlayerInfo`

### `rp::PlacementSprite` (9)

`GetClosestPosition`, `OkayToPlace`, `PlacementSprite`, `Select`, `SetPosition`, `SetPriorities`, `SetToObject`, `Update`, `~PlacementSprite`

### `rp::ExplosionFXSprite` (8)

`ExplosionFXSprite`, `GetTime`, `Init`, `RemoveFromTick`, `Sparatic`, `Update`, `tick`, `~ExplosionFXSprite`

### `rp::RPMenuHandler` (8)

`gotMsgPopPopUpGlobal`, `gotMsgQuestRead`, `gotMsgRequestLocalizedChristeningText`, `gotMsgStartDialog`, `gotMsgStoreCategoryRequest`, `gotMsgStoreDisplayStatus`, `gotSimpleRequest`, `topPopUp`

### `rp::Transition` (8)

`Transition`, `changeType`, `disableAllInput`, `gotMsgAnimationFinished`, `hide`, `show`, `tick`, `~Transition`

### `rp::DivingUnit` (7)

`AddedToOcean`, `DivingUnit`, `Duplicate`, `PlayAnimations`, `RequestDive`, `UseClock`, `getAvailableActions`

### `rp::FootprintSprite` (7)

`FootprintSprite`, `Init`, `SetColour`, `SetPriorities`, `SetPriority`, `Update`, `~FootprintSprite`

### `rp::GfxOceanGradient` (7)

`ComputeWorldState`, `GfxOceanGradient`, `InitWaves`, `Render`, `SetZone`, `tick`, `~GfxOceanGradient`

### `rp::ItemPlacementSprite` (7)

`ItemPlacementSprite`, `OkayToPlace`, `SetPosition`, `SetPriority`, `SetToObject`, `Update`, `~ItemPlacementSprite`

### `rp::SelectedWallSprite` (7)

`Init`, `SelectedWallSprite`, `SetPriorities`, `SetPriority`, `Update`, `tick`, `~SelectedWallSprite`

### `rp::UnitHealthBarSprite` (7)

`Init`, `SetColour`, `SetVisible`, `UnitHealthBarSprite`, `Update`, `tick`, `~UnitHealthBarSprite`

### `rp::DiveMeterSprite` (6)

`DiveMeterSprite`, `Select`, `SetPriorities`, `SetTime`, `tick`, `~DiveMeterSprite`

### `rp::DiveUnitReadoutSprite` (6)

`DiveUnitReadoutSprite`, `SetPriorities`, `SetVisible`, `gotMsgDiveDepthChange`, `tick`, `~DiveUnitReadoutSprite`

### `rp::FireFXSprite` (6)

`FireFXSprite`, `Init`, `SetColour`, `SetVisible`, `Update`, `~FireFXSprite`

### `rp::GfxOceanCheckerBoard` (6)

`FloatMod`, `GfxOceanCheckerBoard`, `Render`, `SetPosition`, `SetUnitSize`, `~GfxOceanCheckerBoard`

### `rp::GfxOceanGrid` (6)

`FloatMod`, `GfxOceanGrid`, `Render`, `SetPosition`, `SetUnitSize`, `~GfxOceanGrid`

### `rp::GridObject` (6)

`AddedToOcean`, `Salvage`, `TempTick`, `getAvailableActions`, `getName`, `setGridObjectData`

### `rp::MsgQueuer` (6)

`MsgQueuer`, `add`, `clear`, `numQueued`, `processOne`, `~MsgQueuer`

### `rp::RaftGridSprite` (6)

`Init`, `RaftGridSprite`, `SetPriorities`, `SetPriority`, `Update`, `~RaftGridSprite`

### `rp::RaftPiratesGameState` (6)

`GetStateName`, `Pause`, `RaftPiratesGameState`, `isReadyForBattle`, `tick`, `~RaftPiratesGameState`

### `rp::SplashFXSprite` (6)

`Init`, `RemoveFromTick`, `SplashFXSprite`, `Update`, `tick`, `~SplashFXSprite`

### `rp::WreckageSprite` (6)

`GetPriority`, `Init`, `SetPriority`, `UpdatePiece`, `WreckageSprite`, `~WreckageSprite`

### `rp::BossBattleRaftHighlightSprite` (5)

`BossBattleRaftHighlightSprite`, `Init`, `Update`, `tick`, `~BossBattleRaftHighlightSprite`

### `rp::GfxBattleInfoDropDown` (5)

`GfxBattleInfoDropDown`, `Hide`, `Show`, `aTickMethodThatsNotCalledTick`, `~GfxBattleInfoDropDown`

### `rp::GfxBattleScreen` (5)

`AddAccessory`, `SetObjectMute`, `SetObjectVisible`, `SetPlayerRaft`, `SetPlayerRaftsVisible`

### `rp::GridObject::GridShapePiece` (5)

`Filename`, `GetGridSpace`, `GetWorldSpace`, `GridShapePiece`, `TextureOffset`

### `rp::LoaderAEAnim` (5)

`getId`, `getType`, `load`, `prepare`, `~LoaderAEAnim`

### `rp::LoaderSpriteSheet` (5)

`getId`, `getType`, `load`, `prepare`, `~LoaderSpriteSheet`

### `rp::LoaderXml` (5)

`getId`, `getType`, `load`, `prepare`, `~LoaderXml`

### `rp::RaftAttackSprite` (5)

`Init`, `RaftAttackSprite`, `Update`, `tick`, `~RaftAttackSprite`

### `rp::SparseGrid` (5)

`AddToTempTick`, `AddedToOcean`, `IsAncestor`, `RemoveFromTempTick`, `TempTick`

### `rp::BattlePointsViewSprite` (4)

`BattlePointsViewSprite`, `SetColour`, `tick`, `~BattlePointsViewSprite`

### `rp::LoaderImage` (4)

`getId`, `getType`, `load`, `~LoaderImage`

### `rp::LoaderManifest` (4)

`getId`, `getType`, `prepare`, `~LoaderManifest`

### `rp::LoaderMenu` (4)

`getId`, `getType`, `prepare`, `~LoaderMenu`

### `rp::OverlapTree` (4)

`GenerateOverlapTree`, `OverlapTree`, `Within`, `~OverlapTree`

### `rp::diving_game::ScrollingLayer` (4)

`ScrollingLayer`, `SetSpeed`, `addSprite`, `tick`

### `rp::BattleArrowSprite` (3)

`BattleArrowSprite`, `tick`, `~BattleArrowSprite`

### `rp::BattlePlacementScreen` (3)

`CheckBossCoreRafts`, `ContainsGfx`, `SetPriority`

### `rp::DamageIndicatorSprite` (3)

`DamageIndicatorSprite`, `tick`, `~DamageIndicatorSprite`

### `rp::GfxShipPreviewScreen` (3)

`GfxShipPreviewScreen`, `aTickMethodThatsNotCalledTick`, `~GfxShipPreviewScreen`

### `rp::GridObjectDataRes` (3)

`Get`, `GridObjectDataRes`, `~GridObjectDataRes`

### `rp::ItemOnRaftPlacementSprite` (3)

`ItemOnRaftPlacementSprite`, `SetPriority`, `~ItemOnRaftPlacementSprite`

### `rp::Loader` (3)

`load`, `prepare`, `~Loader`

### `rp::MovementArrowSprite` (3)

`MovementArrowSprite`, `tick`, `~MovementArrowSprite`

### `rp::PerimeterTree` (3)

`GeneratePerimeterTree`, `PerimeterTree`, `~PerimeterTree`

### `rp::ThrobingIndicatorSprite` (3)

`ThrobingIndicatorSprite`, `tick`, `~ThrobingIndicatorSprite`

### `rp::WaterBucketSprite` (3)

`WaterBucketSprite`, `tick`, `~WaterBucketSprite`

### `rp::diving_game::DivingGame::ItemFloater` (3)

`ItemFloater`, `tick`, `~ItemFloater`

### `rp::BattleModeState::BattleDice` (2)

`BattleDice`, `~BattleDice`

### `rp::DiscoveryGridObject` (2)

`DiscoveryGridObject`, `~DiscoveryGridObject`

### `rp::DiveUnitReadoutSprite::ReadoutLight` (2)

`ReadoutLight`, `~ReadoutLight`

### `rp::MapView::EnemyShipPoints` (2)

`EnemyShipPoints`, `~EnemyShipPoints`

### `rp::MsgConfirmMaterialPurchaseInfo` (2)

`MsgConfirmMaterialPurchaseInfo`, `~MsgConfirmMaterialPurchaseInfo`

### `rp::MsgQuestData` (2)

`MsgQuestData`, `~MsgQuestData`

### `rp::MsgQuestEvent` (2)

`MsgQuestEvent`, `~MsgQuestEvent`

### `rp::MsgStartDialog` (2)

`MsgStartDialog`, `~MsgStartDialog`

### `rp::MsgStoreItemDisplayResponse` (2)

`MsgStoreItemDisplayResponse`, `~MsgStoreItemDisplayResponse`

### `rp::MsgTabbedItemScrollTo` (2)

`MsgTabbedItemScrollTo`, `~MsgTabbedItemScrollTo`

### `rp::Raft` (2)

`Salvage`, `getAvailableActions`

### `rp::RaftComponent` (2)

`AddedToOcean`, `getAvailableActions`

### `rp::RaftComponentFactory` (2)

`Build`, `RegistryAllClasses`

### `rp::RaftFactory` (2)

`Build`, `RegistryAllClasses`

### `rp::Zone` (2)

`mapfile`, `name`

### `rp::Blueprint` (1)

`name`

### `rp::BlueprintInfo` (1)

`~BlueprintInfo`

### `rp::Discovery` (1)

`name`

### `rp::DiscoveryModeState` (1)

`name`

### `rp::DoStuffExtraData` (1)

`DoStuffExtraData`

### `rp::FootprintSprite::SpriteFootprint` (1)

`~SpriteFootprint`

### `rp::GridObjectsDB` (1)

`getDataName`

### `rp::GridSprite::SpritePiece` (1)

`~SpritePiece`

### `rp::MapView::TreasureMapPoints` (1)

`~TreasureMapPoints`

### `rp::Material` (1)

`name`

### `rp::MaterialInfo` (1)

`MaterialInfo`

### `rp::MsgAnimateGridSprite` (1)

`~MsgAnimateGridSprite`

### `rp::MsgAttackRequest` (1)

`~MsgAttackRequest`

### `rp::MsgAttackRequestFailed` (1)

`~MsgAttackRequestFailed`

### `rp::MsgAuthenticationChanged` (1)

`~MsgAuthenticationChanged`

### `rp::MsgAuthenticationFailed` (1)

`~MsgAuthenticationFailed`

### `rp::MsgAutoSafeModeRequest` (1)

`~MsgAutoSafeModeRequest`

### `rp::MsgAutoSafeModeResponse` (1)

`~MsgAutoSafeModeResponse`

### `rp::MsgBadZoneStatusRequest` (1)

`~MsgBadZoneStatusRequest`

### `rp::MsgBadZoneStatusResponse` (1)

`~MsgBadZoneStatusResponse`

### `rp::MsgBattleConfirmSwapStance` (1)

`~MsgBattleConfirmSwapStance`

### `rp::MsgBattleInfoDropdown` (1)

`~MsgBattleInfoDropdown`

### `rp::MsgBattleInfoRequest` (1)

`~MsgBattleInfoRequest`

### `rp::MsgBattleNameResponse` (1)

`~MsgBattleNameResponse`

### `rp::MsgBattleOverConfirm` (1)

`~MsgBattleOverConfirm`

### `rp::MsgBattlePlacementState` (1)

`~MsgBattlePlacementState`

### `rp::MsgBattleReport` (1)

`~MsgBattleReport`

### `rp::MsgBattleScreenRaftsSelected` (1)

`~MsgBattleScreenRaftsSelected`

### `rp::MsgBattleTurnStart` (1)

`~MsgBattleTurnStart`

### `rp::MsgBlueprintDisplayRequest` (1)

`~MsgBlueprintDisplayRequest`

### `rp::MsgBlueprintDisplayResponse` (1)

`~MsgBlueprintDisplayResponse`

### `rp::MsgBlueprintPageResponse` (1)

`~MsgBlueprintPageResponse`

### `rp::MsgBuildCancel` (1)

`~MsgBuildCancel`

### `rp::MsgBuildConfirm` (1)

`~MsgBuildConfirm`

### `rp::MsgBuildMenuInformationResponse` (1)

`~MsgBuildMenuInformationResponse`

### `rp::MsgBuildModeToggleView` (1)

`~MsgBuildModeToggleView`

### `rp::MsgCheat` (1)

`~MsgCheat`

### `rp::MsgCheatDuplicatePlayer` (1)

`~MsgCheatDuplicatePlayer`

### `rp::MsgCheatStartBattle` (1)

`~MsgCheatStartBattle`

### `rp::MsgCheckForHoles` (1)

`~MsgCheckForHoles`

### `rp::MsgCheckForNewDailyQuest` (1)

`~MsgCheckForNewDailyQuest`

### `rp::MsgCheckForNewDailyQuestResult` (1)

`~MsgCheckForNewDailyQuestResult`

### `rp::MsgCollectedBlueprint` (1)

`~MsgCollectedBlueprint`

### `rp::MsgCollectedCourage` (1)

`~MsgCollectedCourage`

### `rp::MsgCollectedFastclocks` (1)

`~MsgCollectedFastclocks`

### `rp::MsgCollectedMapMove` (1)

`~MsgCollectedMapMove`

### `rp::MsgCollectedMaterials` (1)

`~MsgCollectedMaterials`

### `rp::MsgCollectedSonar` (1)

`~MsgCollectedSonar`

### `rp::MsgConfirmBlueprintMaterialsPurchase` (1)

`~MsgConfirmBlueprintMaterialsPurchase`

### `rp::MsgConfirmMapMovement` (1)

`~MsgConfirmMapMovement`

### `rp::MsgConfirmMaterialPurchase` (1)

`~MsgConfirmMaterialPurchase`

### `rp::MsgConfirmRepairGridObjectWithGold` (1)

`~MsgConfirmRepairGridObjectWithGold`

### `rp::MsgConfirmZoneMovement` (1)

`~MsgConfirmZoneMovement`

### `rp::MsgDaringMeterAdvance` (1)

`~MsgDaringMeterAdvance`

### `rp::MsgDaringMeterAdvanceResult` (1)

`~MsgDaringMeterAdvanceResult`

### `rp::MsgDaringMeterInfo` (1)

`~MsgDaringMeterInfo`

### `rp::MsgDaringMeterInfoResponse` (1)

`~MsgDaringMeterInfoResponse`

### `rp::MsgDaringMeterPercentResponse` (1)

`~MsgDaringMeterPercentResponse`

### `rp::MsgDebugGatherAllUsers` (1)

`~MsgDebugGatherAllUsers`

### `rp::MsgDebugGatherAllUsersResult` (1)

`~MsgDebugGatherAllUsersResult`

### `rp::MsgDeselectObject` (1)

`~MsgDeselectObject`

### `rp::MsgDestroyObjectRequest` (1)

`~MsgDestroyObjectRequest`

### `rp::MsgDiscoveryGetAllKnownRequest` (1)

`~MsgDiscoveryGetAllKnownRequest`

### `rp::MsgDiscoveryGetAllKnownResult` (1)

`~MsgDiscoveryGetAllKnownResult`

### `rp::MsgDiscoveryMoveTo` (1)

`~MsgDiscoveryMoveTo`

### `rp::MsgDiscoveryMoveToResult` (1)

`~MsgDiscoveryMoveToResult`

### `rp::MsgDiveDepthChange` (1)

`~MsgDiveDepthChange`

### `rp::MsgDiveUnitLoadChange` (1)

`~MsgDiveUnitLoadChange`

### `rp::MsgDivingGameDone` (1)

`~MsgDivingGameDone`

### `rp::MsgDivingGameDoneMoving` (1)

`~MsgDivingGameDoneMoving`

### `rp::MsgDivingGameShow` (1)

`~MsgDivingGameShow`

### `rp::MsgDoingFacebookAuth` (1)

`~MsgDoingFacebookAuth`

### `rp::MsgEnableNetworkProcessing` (1)

`~MsgEnableNetworkProcessing`

### `rp::MsgExitBattleModeRequest` (1)

`~MsgExitBattleModeRequest`

### `rp::MsgGetAllBlueprintsResponse` (1)

`~MsgGetAllBlueprintsResponse`

### `rp::MsgGetAvailBlueprintsResponse` (1)

`~MsgGetAvailBlueprintsResponse`

### `rp::MsgGetAvailMaterialsResponse` (1)

`~MsgGetAvailMaterialsResponse`

### `rp::MsgGetAvailStoreItemsResponse` (1)

`~MsgGetAvailStoreItemsResponse`

### `rp::MsgGetMapActionState` (1)

`~MsgGetMapActionState`

### `rp::MsgGetMapActionStateUpdate` (1)

`~MsgGetMapActionStateUpdate`

### `rp::MsgGridObjectEvent` (1)

`~MsgGridObjectEvent`

### `rp::MsgGridObjectRepairInfo` (1)

`~MsgGridObjectRepairInfo`

### `rp::MsgGridObjectSelected` (1)

`~MsgGridObjectSelected`

### `rp::MsgHaveMaterialsRequest` (1)

`~MsgHaveMaterialsRequest`

### `rp::MsgHaveMaterialsResponse` (1)

`~MsgHaveMaterialsResponse`

### `rp::MsgHealthLevelRequest` (1)

`~MsgHealthLevelRequest`

### `rp::MsgHealthLevelResponse` (1)

`~MsgHealthLevelResponse`

### `rp::MsgHidePirateMap` (1)

`~MsgHidePirateMap`

### `rp::MsgHighlightHudItem` (1)

`~MsgHighlightHudItem`

### `rp::MsgHudModeResponse` (1)

`~MsgHudModeResponse`

### `rp::MsgLoadFinished` (1)

`~MsgLoadFinished`

### `rp::MsgLoadPercentage` (1)

`~MsgLoadPercentage`

### `rp::MsgLoadPreStart` (1)

`~MsgLoadPreStart`

### `rp::MsgLoadStart` (1)

`~MsgLoadStart`

### `rp::MsgLoadingAnimFinished` (1)

`~MsgLoadingAnimFinished`

### `rp::MsgLocalizedInsertText` (1)

`~MsgLocalizedInsertText`

### `rp::MsgMapMarkerMoved` (1)

`~MsgMapMarkerMoved`

### `rp::MsgMaterialInformationRequest` (1)

`~MsgMaterialInformationRequest`

### `rp::MsgMaterialInformationResponse` (1)

`~MsgMaterialInformationResponse`

### `rp::MsgMaterialsChanged` (1)

`~MsgMaterialsChanged`

### `rp::MsgMenuBattleAccept` (1)

`~MsgMenuBattleAccept`

### `rp::MsgMenuLogoutRequest` (1)

`~MsgMenuLogoutRequest`

### `rp::MsgMoveObjectRequest` (1)

`~MsgMoveObjectRequest`

### `rp::MsgNameShip` (1)

`~MsgNameShip`

### `rp::MsgNextWreckedRaft` (1)

`~MsgNextWreckedRaft`

### `rp::MsgNextWreckedRaftComponent` (1)

`~MsgNextWreckedRaftComponent`

### `rp::MsgNumWreckedRaftComponents` (1)

`~MsgNumWreckedRaftComponents`

### `rp::MsgNumWreckedRafts` (1)

`~MsgNumWreckedRafts`

### `rp::MsgOceanSafetyUpdate` (1)

`~MsgOceanSafetyUpdate`

### `rp::MsgOceanViewPickResult` (1)

`~MsgOceanViewPickResult`

### `rp::MsgPirateMapDiscoveries_GetAvail_Response` (1)

`~MsgPirateMapDiscoveries_GetAvail_Response`

### `rp::MsgPirateMapDiscoveries_Info_Request` (1)

`~MsgPirateMapDiscoveries_Info_Request`

### `rp::MsgPirateMapDiscoveries_Info_Response` (1)

`~MsgPirateMapDiscoveries_Info_Response`

### `rp::MsgPirateMapDiscoveries_Page_Response` (1)

`~MsgPirateMapDiscoveries_Page_Response`

### `rp::MsgPlayerBuiltRaft` (1)

`~MsgPlayerBuiltRaft`

### `rp::MsgPreviousWreckedRaft` (1)

`~MsgPreviousWreckedRaft`

### `rp::MsgPreviousWreckedRaftComponent` (1)

`~MsgPreviousWreckedRaftComponent`

### `rp::MsgPurchaseItemRequest` (1)

`~MsgPurchaseItemRequest`

### `rp::MsgQuestInformationResponse` (1)

`~MsgQuestInformationResponse`

### `rp::MsgQuestOpened` (1)

`~MsgQuestOpened`

### `rp::MsgQuestRead` (1)

`~MsgQuestRead`

### `rp::MsgQuestTypeInformationRequest` (1)

`~MsgQuestTypeInformationRequest`

### `rp::MsgQuestTypeInformationResponse` (1)

`~MsgQuestTypeInformationResponse`

### `rp::MsgQueueMenuPopup` (1)

`~MsgQueueMenuPopup`

### `rp::MsgQuickTravelGetAllKnownRequest` (1)

`~MsgQuickTravelGetAllKnownRequest`

### `rp::MsgQuickTravelGetAllKnownResult` (1)

`~MsgQuickTravelGetAllKnownResult`

### `rp::MsgRecipeInformationRequest` (1)

`~MsgRecipeInformationRequest`

### `rp::MsgRecipeInformationResponse` (1)

`~MsgRecipeInformationResponse`

### `rp::MsgReconnect` (1)

`~MsgReconnect`

### `rp::MsgReenablePingSonar` (1)

`~MsgReenablePingSonar`

### `rp::MsgRepairSelectedObject` (1)

`~MsgRepairSelectedObject`

### `rp::MsgRepairSelectedObjectAll` (1)

`~MsgRepairSelectedObjectAll`

### `rp::MsgRequestBattlePlacementState` (1)

`~MsgRequestBattlePlacementState`

### `rp::MsgRequestNumWreckedRaftComponents` (1)

`~MsgRequestNumWreckedRaftComponents`

### `rp::MsgRequestNumWreckedRafts` (1)

`~MsgRequestNumWreckedRafts`

### `rp::MsgRequestOceanSafetyUpdate` (1)

`~MsgRequestOceanSafetyUpdate`

### `rp::MsgRequestPingSonar` (1)

`~MsgRequestPingSonar`

### `rp::MsgSafetyFlagTimerUpdate` (1)

`~MsgSafetyFlagTimerUpdate`

### `rp::MsgSalvageSelectedObject` (1)

`~MsgSalvageSelectedObject`

### `rp::MsgSelectedObjectAvailableActions` (1)

`~MsgSelectedObjectAvailableActions`

### `rp::MsgSetAutoSafeMode` (1)

`~MsgSetAutoSafeMode`

### `rp::MsgSetBlueprintPage` (1)

`~MsgSetBlueprintPage`

### `rp::MsgSetBlueprintSelected` (1)

`~MsgSetBlueprintSelected`

### `rp::MsgSetHudModeRequest` (1)

`~MsgSetHudModeRequest`

### `rp::MsgSetStoreCategory` (1)

`~MsgSetStoreCategory`

### `rp::MsgSetStoreItemPage` (1)

`~MsgSetStoreItemPage`

### `rp::MsgSetStoreItemSelected` (1)

`~MsgSetStoreItemSelected`

### `rp::MsgSetupDaringPopup` (1)

`~MsgSetupDaringPopup`

### `rp::MsgSetupRankupPopup` (1)

`~MsgSetupRankupPopup`

### `rp::MsgSetupRepairAndSalvageDialog` (1)

`~MsgSetupRepairAndSalvageDialog`

### `rp::MsgSetupStatusPopup` (1)

`~MsgSetupStatusPopup`

### `rp::MsgShowBattlePointsRequest` (1)

`~MsgShowBattlePointsRequest`

### `rp::MsgShowNewDailyQuest` (1)

`~MsgShowNewDailyQuest`

### `rp::MsgShowPirateInfo` (1)

`~MsgShowPirateInfo`

### `rp::MsgShowPirateMap` (1)

`~MsgShowPirateMap`

### `rp::MsgShowStore` (1)

`~MsgShowStore`

### `rp::MsgSimpleRequest` (1)

`~MsgSimpleRequest`

### `rp::MsgSparseGridObjectAdded` (1)

`~MsgSparseGridObjectAdded`

### `rp::MsgSparseGridObjectRemoved` (1)

`~MsgSparseGridObjectRemoved`

### `rp::MsgSpecificMaterialAvailAmountRequest` (1)

`~MsgSpecificMaterialAvailAmountRequest`

### `rp::MsgSpecificMaterialAvailAmountResponse` (1)

`~MsgSpecificMaterialAvailAmountResponse`

### `rp::MsgStartBattleMode` (1)

`~MsgStartBattleMode`

### `rp::MsgStartBuildMode` (1)

`~MsgStartBuildMode`

### `rp::MsgStartBuildRequest` (1)

`~MsgStartBuildRequest`

### `rp::MsgStartTransition` (1)

`~MsgStartTransition`

### `rp::MsgStoreCategoryResponse` (1)

`~MsgStoreCategoryResponse`

### `rp::MsgStoreDisplayStatus` (1)

`~MsgStoreDisplayStatus`

### `rp::MsgStoreItemDisplayRequest` (1)

`~MsgStoreItemDisplayRequest`

### `rp::MsgStoreItemSelectedResponse` (1)

`~MsgStoreItemSelectedResponse`

### `rp::MsgStorePageResponse` (1)

`~MsgStorePageResponse`

### `rp::MsgToPositioningView` (1)

`~MsgToPositioningView`

### `rp::MsgToggleBlockHudItem` (1)

`~MsgToggleBlockHudItem`

### `rp::MsgTransitionDone` (1)

`~MsgTransitionDone`

### `rp::MsgTreasureMapGotOne` (1)

`~MsgTreasureMapGotOne`

### `rp::MsgTutorialStartBattleMove` (1)

`~MsgTutorialStartBattleMove`

### `rp::MsgUpdateCurrencyResponse` (1)

`~MsgUpdateCurrencyResponse`

### `rp::MsgUpdateOceanView` (1)

`~MsgUpdateOceanView`

### `rp::MsgUseClockOnSelectedObject` (1)

`~MsgUseClockOnSelectedObject`

### `rp::MsgWreckageSelected` (1)

`~MsgWreckageSelected`

### `rp::MsgZoneGetAllKnownRequest` (1)

`~MsgZoneGetAllKnownRequest`

### `rp::MsgZoneGetAllKnownResult` (1)

`~MsgZoneGetAllKnownResult`

### `rp::MsgZoneMoveTo` (1)

`~MsgZoneMoveTo`

### `rp::MsgZoneMoveToResult` (1)

`~MsgZoneMoveToResult`

### `rp::Ocean` (1)

`initWithSFSObject`

### `rp::PerimeterWallSprite::SpriteBalloon` (1)

`~SpriteBalloon`

### `rp::Player` (1)

`getResourceFiles`

### `rp::QuestTypeInfo` (1)

`~QuestTypeInfo`

### `rp::RaftInfo` (1)

`RaftInfo`

### `rp::TouchControls` (1)

`~TouchControls`

### `rp::Transition::MsgDone` (1)

`~MsgDone`

### `rp::diving_game::DivingGame` (1)

`setPos`

### `rp::diving_game::DivingMaterial` (1)

`DivingMaterial`

