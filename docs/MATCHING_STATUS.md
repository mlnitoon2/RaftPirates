# Matching status

Generated from defined function symbols. Name coverage is unique qualified method names, not source-code recovery or instruction matching. Overloads are grouped by name; signature overlap means at least one demangled signature overlaps. Neither metric establishes behavior or ABI equivalence. Binary matching is **not measured** (legacy ARM32 versus host build).

| Original build | Names matched | Coverage | Names with signature overlap | Binary match |
|---|---:|---:|---:|---|
| Raft Pirates (2013) | 188/1191 | 15.7851% | 106 | Not compared |

## Raft Pirates (2013)

Baseline SHA-256 (.tsv): `1544df78707236da43862b022529d1fff01e6f2f0ac37f52d50b3d454a39df86`

| Class or namespace | Name matches | Coverage | Signature overlap |
|---|---:|---:|---:|
| `rp` | 0/11 | 0.00% | 0 |
| `rp::AccessorySprite` | 0/11 | 0.00% | 0 |
| `rp::Barracks` | 0/11 | 0.00% | 0 |
| `rp::BattleArrowSprite` | 0/3 | 0.00% | 0 |
| `rp::BattleModeState` | 13/31 | 41.94% | 9 |
| `rp::BattleModeState::BattleDice` | 0/2 | 0.00% | 0 |
| `rp::BattlePlacementScreen` | 0/15 | 0.00% | 0 |
| `rp::BattlePointsViewSprite` | 0/4 | 0.00% | 0 |
| `rp::BattleShipPlacementSprite` | 0/23 | 0.00% | 0 |
| `rp::Blueprint` | 5/6 | 83.33% | 3 |
| `rp::BlueprintInfo` | 0/1 | 0.00% | 0 |
| `rp::BlueprintsDB` | 0/8 | 0.00% | 0 |
| `rp::BossBattleRaftHighlightSprite` | 0/5 | 0.00% | 0 |
| `rp::BuildModeState` | 6/63 | 9.52% | 2 |
| `rp::DamageIndicatorSprite` | 0/3 | 0.00% | 0 |
| `rp::Discovery` | 4/6 | 66.67% | 3 |
| `rp::DiscoveryDB` | 0/6 | 0.00% | 0 |
| `rp::DiscoveryGridObject` | 0/2 | 0.00% | 0 |
| `rp::DiscoveryModeState` | 6/9 | 66.67% | 3 |
| `rp::DiveMeterSprite` | 0/6 | 0.00% | 0 |
| `rp::DiveUnitReadoutSprite` | 0/6 | 0.00% | 0 |
| `rp::DiveUnitReadoutSprite::ReadoutLight` | 0/2 | 0.00% | 0 |
| `rp::DivingUnit` | 0/22 | 0.00% | 0 |
| `rp::DoStuffExtraData` | 0/1 | 0.00% | 0 |
| `rp::ExplosionFXSprite` | 0/8 | 0.00% | 0 |
| `rp::FireFXSprite` | 0/6 | 0.00% | 0 |
| `rp::FootprintSprite` | 0/7 | 0.00% | 0 |
| `rp::FootprintSprite::SpriteFootprint` | 0/1 | 0.00% | 0 |
| `rp::GfxBattleInfoDropDown` | 0/5 | 0.00% | 0 |
| `rp::GfxBattleScreen` | 0/11 | 0.00% | 0 |
| `rp::GfxOceanCheckerBoard` | 0/6 | 0.00% | 0 |
| `rp::GfxOceanGradient` | 0/7 | 0.00% | 0 |
| `rp::GfxOceanGrid` | 0/6 | 0.00% | 0 |
| `rp::GfxShipPreviewScreen` | 0/3 | 0.00% | 0 |
| `rp::GridObject` | 14/26 | 53.85% | 7 |
| `rp::GridObject::GridShapePiece` | 0/5 | 0.00% | 0 |
| `rp::GridObjectDataRes` | 0/3 | 0.00% | 0 |
| `rp::GridObjectInfo` | 0/1 | 0.00% | 0 |
| `rp::GridObjectPiece` | 2/2 | 100.00% | 1 |
| `rp::GridObjectsDB` | 0/8 | 0.00% | 0 |
| `rp::GridSprite` | 0/33 | 0.00% | 0 |
| `rp::GridSprite::SpritePiece` | 0/1 | 0.00% | 0 |
| `rp::ItemOnRaftPlacementSprite` | 0/3 | 0.00% | 0 |
| `rp::ItemPlacementSprite` | 0/7 | 0.00% | 0 |
| `rp::Loader` | 0/3 | 0.00% | 0 |
| `rp::LoaderAEAnim` | 0/5 | 0.00% | 0 |
| `rp::LoaderImage` | 0/4 | 0.00% | 0 |
| `rp::LoaderManifest` | 0/4 | 0.00% | 0 |
| `rp::LoaderMenu` | 0/4 | 0.00% | 0 |
| `rp::LoaderSpriteSheet` | 0/5 | 0.00% | 0 |
| `rp::LoaderXml` | 0/5 | 0.00% | 0 |
| `rp::LoadingManager` | 0/13 | 0.00% | 0 |
| `rp::MapView` | 7/46 | 15.22% | 2 |
| `rp::MapView::EnemyShipPoints` | 0/2 | 0.00% | 0 |
| `rp::MapView::TreasureMapPoints` | 0/1 | 0.00% | 0 |
| `rp::Material` | 3/6 | 50.00% | 2 |
| `rp::MaterialInfo` | 0/2 | 0.00% | 0 |
| `rp::MaterialsDB` | 0/6 | 0.00% | 0 |
| `rp::MovementArrowSprite` | 0/3 | 0.00% | 0 |
| `rp::MsgAnimateGridSprite` | 0/1 | 0.00% | 0 |
| `rp::MsgAttackRequest` | 0/1 | 0.00% | 0 |
| `rp::MsgAttackRequestFailed` | 0/1 | 0.00% | 0 |
| `rp::MsgAuthenticationChanged` | 0/1 | 0.00% | 0 |
| `rp::MsgAuthenticationFailed` | 0/1 | 0.00% | 0 |
| `rp::MsgAutoSafeModeRequest` | 0/1 | 0.00% | 0 |
| `rp::MsgAutoSafeModeResponse` | 0/1 | 0.00% | 0 |
| `rp::MsgBadZoneStatusRequest` | 0/1 | 0.00% | 0 |
| `rp::MsgBadZoneStatusResponse` | 0/1 | 0.00% | 0 |
| `rp::MsgBattleConfirmSwapStance` | 0/1 | 0.00% | 0 |
| `rp::MsgBattleInfoDropdown` | 0/1 | 0.00% | 0 |
| `rp::MsgBattleInfoRequest` | 0/1 | 0.00% | 0 |
| `rp::MsgBattleNameResponse` | 0/1 | 0.00% | 0 |
| `rp::MsgBattleOverConfirm` | 0/1 | 0.00% | 0 |
| `rp::MsgBattlePlacementState` | 0/1 | 0.00% | 0 |
| `rp::MsgBattleReport` | 0/1 | 0.00% | 0 |
| `rp::MsgBattleScreenRaftsSelected` | 0/1 | 0.00% | 0 |
| `rp::MsgBattleTurnStart` | 0/1 | 0.00% | 0 |
| `rp::MsgBlueprintDisplayRequest` | 0/1 | 0.00% | 0 |
| `rp::MsgBlueprintDisplayResponse` | 0/1 | 0.00% | 0 |
| `rp::MsgBlueprintPageResponse` | 0/1 | 0.00% | 0 |
| `rp::MsgBuildCancel` | 0/1 | 0.00% | 0 |
| `rp::MsgBuildConfirm` | 0/1 | 0.00% | 0 |
| `rp::MsgBuildMenuInformationResponse` | 0/1 | 0.00% | 0 |
| `rp::MsgBuildModeToggleView` | 0/1 | 0.00% | 0 |
| `rp::MsgCheat` | 0/1 | 0.00% | 0 |
| `rp::MsgCheatDuplicatePlayer` | 0/1 | 0.00% | 0 |
| `rp::MsgCheatStartBattle` | 0/1 | 0.00% | 0 |
| `rp::MsgCheckForHoles` | 0/1 | 0.00% | 0 |
| `rp::MsgCheckForNewDailyQuest` | 0/1 | 0.00% | 0 |
| `rp::MsgCheckForNewDailyQuestResult` | 0/1 | 0.00% | 0 |
| `rp::MsgCollectedBlueprint` | 0/1 | 0.00% | 0 |
| `rp::MsgCollectedCourage` | 0/1 | 0.00% | 0 |
| `rp::MsgCollectedFastclocks` | 0/1 | 0.00% | 0 |
| `rp::MsgCollectedMapMove` | 0/1 | 0.00% | 0 |
| `rp::MsgCollectedMaterials` | 0/1 | 0.00% | 0 |
| `rp::MsgCollectedSonar` | 0/1 | 0.00% | 0 |
| `rp::MsgConfirmBlueprintMaterialsPurchase` | 0/1 | 0.00% | 0 |
| `rp::MsgConfirmMapMovement` | 0/1 | 0.00% | 0 |
| `rp::MsgConfirmMaterialPurchase` | 0/1 | 0.00% | 0 |
| `rp::MsgConfirmMaterialPurchaseInfo` | 0/2 | 0.00% | 0 |
| `rp::MsgConfirmRepairGridObjectWithGold` | 0/1 | 0.00% | 0 |
| `rp::MsgConfirmZoneMovement` | 0/1 | 0.00% | 0 |
| `rp::MsgDaringMeterAdvance` | 0/1 | 0.00% | 0 |
| `rp::MsgDaringMeterAdvanceResult` | 0/1 | 0.00% | 0 |
| `rp::MsgDaringMeterInfo` | 0/1 | 0.00% | 0 |
| `rp::MsgDaringMeterInfoResponse` | 0/1 | 0.00% | 0 |
| `rp::MsgDaringMeterPercentResponse` | 0/1 | 0.00% | 0 |
| `rp::MsgDebugGatherAllUsers` | 0/1 | 0.00% | 0 |
| `rp::MsgDebugGatherAllUsersResult` | 0/1 | 0.00% | 0 |
| `rp::MsgDeselectObject` | 0/1 | 0.00% | 0 |
| `rp::MsgDestroyObjectRequest` | 0/1 | 0.00% | 0 |
| `rp::MsgDiscoveryGetAllKnownRequest` | 0/1 | 0.00% | 0 |
| `rp::MsgDiscoveryGetAllKnownResult` | 0/1 | 0.00% | 0 |
| `rp::MsgDiscoveryMoveTo` | 0/1 | 0.00% | 0 |
| `rp::MsgDiscoveryMoveToResult` | 0/1 | 0.00% | 0 |
| `rp::MsgDiveDepthChange` | 0/1 | 0.00% | 0 |
| `rp::MsgDiveUnitLoadChange` | 0/1 | 0.00% | 0 |
| `rp::MsgDivingGameDone` | 0/1 | 0.00% | 0 |
| `rp::MsgDivingGameDoneMoving` | 0/1 | 0.00% | 0 |
| `rp::MsgDivingGameShow` | 0/1 | 0.00% | 0 |
| `rp::MsgDoingFacebookAuth` | 0/1 | 0.00% | 0 |
| `rp::MsgEnableNetworkProcessing` | 0/1 | 0.00% | 0 |
| `rp::MsgExitBattleModeRequest` | 0/1 | 0.00% | 0 |
| `rp::MsgGetAllBlueprintsResponse` | 0/1 | 0.00% | 0 |
| `rp::MsgGetAvailBlueprintsResponse` | 0/1 | 0.00% | 0 |
| `rp::MsgGetAvailMaterialsResponse` | 0/1 | 0.00% | 0 |
| `rp::MsgGetAvailStoreItemsResponse` | 0/1 | 0.00% | 0 |
| `rp::MsgGetMapActionState` | 0/1 | 0.00% | 0 |
| `rp::MsgGetMapActionStateUpdate` | 0/1 | 0.00% | 0 |
| `rp::MsgGridObjectEvent` | 0/1 | 0.00% | 0 |
| `rp::MsgGridObjectRepairInfo` | 0/1 | 0.00% | 0 |
| `rp::MsgGridObjectSelected` | 0/1 | 0.00% | 0 |
| `rp::MsgHaveMaterialsRequest` | 0/1 | 0.00% | 0 |
| `rp::MsgHaveMaterialsResponse` | 0/1 | 0.00% | 0 |
| `rp::MsgHealthLevelRequest` | 0/1 | 0.00% | 0 |
| `rp::MsgHealthLevelResponse` | 0/1 | 0.00% | 0 |
| `rp::MsgHidePirateMap` | 0/1 | 0.00% | 0 |
| `rp::MsgHighlightHudItem` | 0/1 | 0.00% | 0 |
| `rp::MsgHudModeResponse` | 0/1 | 0.00% | 0 |
| `rp::MsgLoadFinished` | 0/1 | 0.00% | 0 |
| `rp::MsgLoadPercentage` | 0/1 | 0.00% | 0 |
| `rp::MsgLoadPreStart` | 0/1 | 0.00% | 0 |
| `rp::MsgLoadStart` | 0/1 | 0.00% | 0 |
| `rp::MsgLoadingAnimFinished` | 0/1 | 0.00% | 0 |
| `rp::MsgLocalizedInsertText` | 0/1 | 0.00% | 0 |
| `rp::MsgMapMarkerMoved` | 0/1 | 0.00% | 0 |
| `rp::MsgMaterialInformationRequest` | 0/1 | 0.00% | 0 |
| `rp::MsgMaterialInformationResponse` | 0/1 | 0.00% | 0 |
| `rp::MsgMaterialsChanged` | 0/1 | 0.00% | 0 |
| `rp::MsgMenuBattleAccept` | 0/1 | 0.00% | 0 |
| `rp::MsgMenuLogoutRequest` | 0/1 | 0.00% | 0 |
| `rp::MsgMoveObjectRequest` | 0/1 | 0.00% | 0 |
| `rp::MsgNameShip` | 0/1 | 0.00% | 0 |
| `rp::MsgNextWreckedRaft` | 0/1 | 0.00% | 0 |
| `rp::MsgNextWreckedRaftComponent` | 0/1 | 0.00% | 0 |
| `rp::MsgNumWreckedRaftComponents` | 0/1 | 0.00% | 0 |
| `rp::MsgNumWreckedRafts` | 0/1 | 0.00% | 0 |
| `rp::MsgOceanSafetyUpdate` | 0/1 | 0.00% | 0 |
| `rp::MsgOceanViewPickResult` | 0/1 | 0.00% | 0 |
| `rp::MsgPirateMapDiscoveries_GetAvail_Response` | 0/1 | 0.00% | 0 |
| `rp::MsgPirateMapDiscoveries_Info_Request` | 0/1 | 0.00% | 0 |
| `rp::MsgPirateMapDiscoveries_Info_Response` | 0/1 | 0.00% | 0 |
| `rp::MsgPirateMapDiscoveries_Page_Response` | 0/1 | 0.00% | 0 |
| `rp::MsgPlayerBuiltRaft` | 0/1 | 0.00% | 0 |
| `rp::MsgPreviousWreckedRaft` | 0/1 | 0.00% | 0 |
| `rp::MsgPreviousWreckedRaftComponent` | 0/1 | 0.00% | 0 |
| `rp::MsgPurchaseItemRequest` | 0/1 | 0.00% | 0 |
| `rp::MsgQuestData` | 0/2 | 0.00% | 0 |
| `rp::MsgQuestEvent` | 0/2 | 0.00% | 0 |
| `rp::MsgQuestInformationResponse` | 0/1 | 0.00% | 0 |
| `rp::MsgQuestOpened` | 0/1 | 0.00% | 0 |
| `rp::MsgQuestRead` | 0/1 | 0.00% | 0 |
| `rp::MsgQuestTypeInformationRequest` | 0/1 | 0.00% | 0 |
| `rp::MsgQuestTypeInformationResponse` | 0/1 | 0.00% | 0 |
| `rp::MsgQueueMenuPopup` | 0/1 | 0.00% | 0 |
| `rp::MsgQueuer` | 0/6 | 0.00% | 0 |
| `rp::MsgQuickTravelGetAllKnownRequest` | 0/1 | 0.00% | 0 |
| `rp::MsgQuickTravelGetAllKnownResult` | 0/1 | 0.00% | 0 |
| `rp::MsgRecipeInformationRequest` | 0/1 | 0.00% | 0 |
| `rp::MsgRecipeInformationResponse` | 0/1 | 0.00% | 0 |
| `rp::MsgReconnect` | 0/1 | 0.00% | 0 |
| `rp::MsgReenablePingSonar` | 0/1 | 0.00% | 0 |
| `rp::MsgRepairSelectedObject` | 0/1 | 0.00% | 0 |
| `rp::MsgRepairSelectedObjectAll` | 0/1 | 0.00% | 0 |
| `rp::MsgRequestBattlePlacementState` | 0/1 | 0.00% | 0 |
| `rp::MsgRequestNumWreckedRaftComponents` | 0/1 | 0.00% | 0 |
| `rp::MsgRequestNumWreckedRafts` | 0/1 | 0.00% | 0 |
| `rp::MsgRequestOceanSafetyUpdate` | 0/1 | 0.00% | 0 |
| `rp::MsgRequestPingSonar` | 0/1 | 0.00% | 0 |
| `rp::MsgSafetyFlagTimerUpdate` | 0/1 | 0.00% | 0 |
| `rp::MsgSalvageSelectedObject` | 0/1 | 0.00% | 0 |
| `rp::MsgSelectedObjectAvailableActions` | 0/1 | 0.00% | 0 |
| `rp::MsgSetAutoSafeMode` | 0/1 | 0.00% | 0 |
| `rp::MsgSetBlueprintPage` | 0/1 | 0.00% | 0 |
| `rp::MsgSetBlueprintSelected` | 0/1 | 0.00% | 0 |
| `rp::MsgSetHudModeRequest` | 0/1 | 0.00% | 0 |
| `rp::MsgSetStoreCategory` | 0/1 | 0.00% | 0 |
| `rp::MsgSetStoreItemPage` | 0/1 | 0.00% | 0 |
| `rp::MsgSetStoreItemSelected` | 0/1 | 0.00% | 0 |
| `rp::MsgSetupDaringPopup` | 0/1 | 0.00% | 0 |
| `rp::MsgSetupRankupPopup` | 0/1 | 0.00% | 0 |
| `rp::MsgSetupRepairAndSalvageDialog` | 0/1 | 0.00% | 0 |
| `rp::MsgSetupStatusPopup` | 0/1 | 0.00% | 0 |
| `rp::MsgShowBattlePointsRequest` | 0/1 | 0.00% | 0 |
| `rp::MsgShowNewDailyQuest` | 0/1 | 0.00% | 0 |
| `rp::MsgShowPirateInfo` | 0/1 | 0.00% | 0 |
| `rp::MsgShowPirateMap` | 0/1 | 0.00% | 0 |
| `rp::MsgShowStore` | 0/1 | 0.00% | 0 |
| `rp::MsgSimpleRequest` | 0/1 | 0.00% | 0 |
| `rp::MsgSparseGridObjectAdded` | 0/1 | 0.00% | 0 |
| `rp::MsgSparseGridObjectRemoved` | 0/1 | 0.00% | 0 |
| `rp::MsgSpecificMaterialAvailAmountRequest` | 0/1 | 0.00% | 0 |
| `rp::MsgSpecificMaterialAvailAmountResponse` | 0/1 | 0.00% | 0 |
| `rp::MsgStartBattleMode` | 0/1 | 0.00% | 0 |
| `rp::MsgStartBuildMode` | 0/1 | 0.00% | 0 |
| `rp::MsgStartBuildRequest` | 0/1 | 0.00% | 0 |
| `rp::MsgStartDialog` | 0/2 | 0.00% | 0 |
| `rp::MsgStartTransition` | 0/1 | 0.00% | 0 |
| `rp::MsgStoreCategoryResponse` | 0/1 | 0.00% | 0 |
| `rp::MsgStoreDisplayStatus` | 0/1 | 0.00% | 0 |
| `rp::MsgStoreItemDisplayRequest` | 0/1 | 0.00% | 0 |
| `rp::MsgStoreItemDisplayResponse` | 0/2 | 0.00% | 0 |
| `rp::MsgStoreItemSelectedResponse` | 0/1 | 0.00% | 0 |
| `rp::MsgStorePageResponse` | 0/1 | 0.00% | 0 |
| `rp::MsgTabbedItemScrollTo` | 0/2 | 0.00% | 0 |
| `rp::MsgToPositioningView` | 0/1 | 0.00% | 0 |
| `rp::MsgToggleBlockHudItem` | 0/1 | 0.00% | 0 |
| `rp::MsgTransitionDone` | 0/1 | 0.00% | 0 |
| `rp::MsgTreasureMapGotOne` | 0/1 | 0.00% | 0 |
| `rp::MsgTutorialStartBattleMove` | 0/1 | 0.00% | 0 |
| `rp::MsgUpdateCurrencyResponse` | 0/1 | 0.00% | 0 |
| `rp::MsgUpdateOceanView` | 0/1 | 0.00% | 0 |
| `rp::MsgUseClockOnSelectedObject` | 0/1 | 0.00% | 0 |
| `rp::MsgWreckageSelected` | 0/1 | 0.00% | 0 |
| `rp::MsgZoneGetAllKnownRequest` | 0/1 | 0.00% | 0 |
| `rp::MsgZoneGetAllKnownResult` | 0/1 | 0.00% | 0 |
| `rp::MsgZoneMoveTo` | 0/1 | 0.00% | 0 |
| `rp::MsgZoneMoveToResult` | 0/1 | 0.00% | 0 |
| `rp::Ocean` | 0/10 | 0.00% | 0 |
| `rp::OceanView` | 24/62 | 38.71% | 17 |
| `rp::OverlapTree` | 0/4 | 0.00% | 0 |
| `rp::PerimeterTree` | 0/3 | 0.00% | 0 |
| `rp::PerimeterWallSprite` | 0/10 | 0.00% | 0 |
| `rp::PerimeterWallSprite::SpriteBalloon` | 0/1 | 0.00% | 0 |
| `rp::PlacementSprite` | 0/9 | 0.00% | 0 |
| `rp::Player` | 23/30 | 76.67% | 15 |
| `rp::Quest` | 6/6 | 100.00% | 5 |
| `rp::QuestTypeDB` | 0/5 | 0.00% | 0 |
| `rp::QuestTypeInfo` | 0/1 | 0.00% | 0 |
| `rp::RPMenuHandler` | 8/47 | 17.02% | 0 |
| `rp::Raft` | 10/16 | 62.50% | 5 |
| `rp::RaftAttackSprite` | 0/5 | 0.00% | 0 |
| `rp::RaftComponent` | 11/15 | 73.33% | 7 |
| `rp::RaftComponentFactory` | 0/2 | 0.00% | 0 |
| `rp::RaftFactory` | 0/2 | 0.00% | 0 |
| `rp::RaftGridSprite` | 0/6 | 0.00% | 0 |
| `rp::RaftInfo` | 0/1 | 0.00% | 0 |
| `rp::RaftPirates` | 0/86 | 0.00% | 0 |
| `rp::RaftPiratesGameState` | 0/6 | 0.00% | 0 |
| `rp::SelectedWallSprite` | 0/7 | 0.00% | 0 |
| `rp::ShipPlacementSprite` | 0/11 | 0.00% | 0 |
| `rp::SparseGrid` | 10/15 | 66.67% | 1 |
| `rp::SplashFXSprite` | 0/6 | 0.00% | 0 |
| `rp::StoreDB` | 0/6 | 0.00% | 0 |
| `rp::ThrobingIndicatorSprite` | 0/3 | 0.00% | 0 |
| `rp::TouchControls` | 0/15 | 0.00% | 0 |
| `rp::Transition` | 0/8 | 0.00% | 0 |
| `rp::Transition::MsgDone` | 0/1 | 0.00% | 0 |
| `rp::UnitHealthBarSprite` | 0/7 | 0.00% | 0 |
| `rp::WaterBucketSprite` | 0/3 | 0.00% | 0 |
| `rp::Wreckage` | 0/14 | 0.00% | 0 |
| `rp::WreckageSprite` | 0/6 | 0.00% | 0 |
| `rp::Zone` | 10/13 | 76.92% | 9 |
| `rp::ZonesDB` | 0/7 | 0.00% | 0 |
| `rp::diving_game::DivingCollectable` | 1/1 | 100.00% | 0 |
| `rp::diving_game::DivingGame` | 25/28 | 89.29% | 15 |
| `rp::diving_game::DivingGame::ItemFloater` | 0/3 | 0.00% | 0 |
| `rp::diving_game::DivingMaterial` | 0/1 | 0.00% | 0 |
| `rp::diving_game::ScrollingLayer` | 0/4 | 0.00% | 0 |

Matched names:

- `rp::BattleModeState::BattleModeState`
- `rp::BattleModeState::Pause`
- `rp::BattleModeState::RaftSelect`
- `rp::BattleModeState::SendRaftSelection`
- `rp::BattleModeState::StartWaiting`
- `rp::BattleModeState::StopWaiting`
- `rp::BattleModeState::ToBattleView`
- `rp::BattleModeState::ToPositioningView`
- `rp::BattleModeState::endBattleAndPlunder`
- `rp::BattleModeState::gotMsgAttackRequest`
- `rp::BattleModeState::gotMsgExitBattleModeRequest`
- `rp::BattleModeState::tick`
- `rp::BattleModeState::~BattleModeState`
- `rp::Blueprint::Blueprint`
- `rp::Blueprint::id`
- `rp::Blueprint::init`
- `rp::Blueprint::initWithSFSObject`
- `rp::Blueprint::~Blueprint`
- `rp::BuildModeState::BuildModeState`
- `rp::BuildModeState::UseClockSelected`
- `rp::BuildModeState::deselectObject`
- `rp::BuildModeState::repairSelected`
- `rp::BuildModeState::selectObject`
- `rp::BuildModeState::~BuildModeState`
- `rp::Discovery::id`
- `rp::Discovery::initWithSFSObject`
- `rp::Discovery::questgroupid`
- `rp::Discovery::zoneid`
- `rp::DiscoveryModeState::DiscoveryModeState`
- `rp::DiscoveryModeState::LeaveDiscovery`
- `rp::DiscoveryModeState::LeaveDiscoveryReally`
- `rp::DiscoveryModeState::Pause`
- `rp::DiscoveryModeState::tick`
- `rp::DiscoveryModeState::~DiscoveryModeState`
- `rp::GridObject::Contains`
- `rp::GridObject::Destroyed`
- `rp::GridObject::GetBattlePoints`
- `rp::GridObject::GetGridObjectID`
- `rp::GridObject::GetGridPosition`
- `rp::GridObject::GetHealth`
- `rp::GridObject::GetTotalBattlePoints`
- `rp::GridObject::GetUserObjectID`
- `rp::GridObject::Repair`
- `rp::GridObject::SetBattlePoints`
- `rp::GridObject::SetState`
- `rp::GridObject::getResourceData`
- `rp::GridObject::getType`
- `rp::GridObject::~GridObject`
- `rp::GridObjectPiece::GridObjectPiece`
- `rp::GridObjectPiece::~GridObjectPiece`
- `rp::MapView::MapView`
- `rp::MapView::isPosInSafeWater`
- `rp::MapView::isPosOnLand`
- `rp::MapView::isValidIconLocation`
- `rp::MapView::limitPlayerPos`
- `rp::MapView::setMapFile`
- `rp::MapView::~MapView`
- `rp::Material::id`
- `rp::Material::init`
- `rp::Material::initWithSFSObject`
- `rp::OceanView::AxisX`
- `rp::OceanView::AxisY`
- `rp::OceanView::CalculateHelpers`
- `rp::OceanView::ConvertToView`
- `rp::OceanView::ConvertToViewSpecial`
- `rp::OceanView::ConvertToWorld`
- `rp::OceanView::Height`
- `rp::OceanView::Offset`
- `rp::OceanView::Position`
- `rp::OceanView::Priority`
- `rp::OceanView::ScreenToWorld`
- `rp::OceanView::SetHeight`
- `rp::OceanView::SetOffset`
- `rp::OceanView::SetPosition`
- `rp::OceanView::SetPriority`
- `rp::OceanView::SetUnitSize`
- `rp::OceanView::SetWidth`
- `rp::OceanView::SetZoom`
- `rp::OceanView::Unit`
- `rp::OceanView::Update`
- `rp::OceanView::Width`
- `rp::OceanView::WorldToScreen`
- `rp::OceanView::Zoom`
- `rp::OceanView::~OceanView`
- `rp::Player::AddRaft`
- `rp::Player::GetAvailableEdgeGridSpace`
- `rp::Player::GetDiveUnit`
- `rp::Player::GetRaftAtPosition`
- `rp::Player::GetRaftByID`
- `rp::Player::RaftCleanupRequired`
- `rp::Player::RaftRepairRequired`
- `rp::Player::RaftsConnectedToRaft`
- `rp::Player::RemoveRaft`
- `rp::Player::canBuildRaft`
- `rp::Player::currentBP`
- `rp::Player::giveBlueprint`
- `rp::Player::giveMaterial`
- `rp::Player::hasBlueprint`
- `rp::Player::hasMaterial`
- `rp::Player::hasRaft`
- `rp::Player::initWithSFSObject`
- `rp::Player::isSafe`
- `rp::Player::isZoneKnown`
- `rp::Player::mainRaft`
- `rp::Player::numOfMaterialPossessed`
- `rp::Player::setSafe`
- `rp::Player::~Player`
- `rp::Quest::Quest`
- `rp::Quest::initWithSFSObject`
- `rp::Quest::markOpened`
- `rp::Quest::markRead`
- `rp::Quest::setProgressPercentage`
- `rp::Quest::~Quest`
- `rp::RPMenuHandler::gotMsgGetAllBlueprintsRequest`
- `rp::RPMenuHandler::gotMsgGetAvailBlueprintsRequest`
- `rp::RPMenuHandler::gotMsgGetAvailMaterialsRequest`
- `rp::RPMenuHandler::gotMsgGetAvailStoreItemRequest`
- `rp::RPMenuHandler::gotMsgHaveMaterialsRequest`
- `rp::RPMenuHandler::gotMsgQuestInformationRequest`
- `rp::RPMenuHandler::gotMsgRecipeInformationRequest`
- `rp::RPMenuHandler::gotMsgSpecificMaterialAvailAmountRequest`
- `rp::Raft::GetBattlePoints`
- `rp::Raft::GetHealth`
- `rp::Raft::GetTotalBattlePoints`
- `rp::Raft::GridDimensionsX`
- `rp::Raft::GridDimensionsY`
- `rp::Raft::HandleBattleOutcome`
- `rp::Raft::Raft`
- `rp::Raft::SetState`
- `rp::Raft::initWithSFSObject`
- `rp::Raft::~Raft`
- `rp::RaftComponent::Building`
- `rp::RaftComponent::GetBattlePoints`
- `rp::RaftComponent::GetRemainingTime`
- `rp::RaftComponent::GetTotalBattlePoints`
- `rp::RaftComponent::PostBuild`
- `rp::RaftComponent::RaftComponent`
- `rp::RaftComponent::SetRemainingTime`
- `rp::RaftComponent::SetState`
- `rp::RaftComponent::TempTick`
- `rp::RaftComponent::initWithSFSObject`
- `rp::RaftComponent::~RaftComponent`
- `rp::SparseGrid::CanAddObject`
- `rp::SparseGrid::GenerateKey`
- `rp::SparseGrid::SparseGrid`
- `rp::SparseGrid::addObject`
- `rp::SparseGrid::getObjectAt`
- `rp::SparseGrid::getObjectByID`
- `rp::SparseGrid::hasObject`
- `rp::SparseGrid::moveObject`
- `rp::SparseGrid::removeObject`
- `rp::SparseGrid::~SparseGrid`
- `rp::Zone::buffer_bp`
- `rp::Zone::daringLimit`
- `rp::Zone::defaultpos_open_x`
- `rp::Zone::defaultpos_open_y`
- `rp::Zone::defaultpos_safe_x`
- `rp::Zone::defaultpos_safe_y`
- `rp::Zone::id`
- `rp::Zone::initWithSFSObject`
- `rp::Zone::max_bp`
- `rp::Zone::min_bp`
- `rp::diving_game::DivingCollectable::DivingCollectable`
- `rp::diving_game::DivingGame::CheckMenuButtonStates`
- `rp::diving_game::DivingGame::ConfirmDiveFinish`
- `rp::diving_game::DivingGame::DivingGame`
- `rp::diving_game::DivingGame::GetNextGearX`
- `rp::diving_game::DivingGame::GetNextMine`
- `rp::diving_game::DivingGame::InitLevel`
- `rp::diving_game::DivingGame::NextLevel`
- `rp::diving_game::DivingGame::NextShape`
- `rp::diving_game::DivingGame::Reset`
- `rp::diving_game::DivingGame::SaveYourself`
- `rp::diving_game::DivingGame::UpdateGearCount`
- `rp::diving_game::DivingGame::gotMsgUpdate`
- `rp::diving_game::DivingGame::moveDiver`
- `rp::diving_game::DivingGame::moveTo`
- `rp::diving_game::DivingGame::robotCollidesWithItem`
- `rp::diving_game::DivingGame::spawnItem`
- `rp::diving_game::DivingGame::touchCancel`
- `rp::diving_game::DivingGame::touchDown`
- `rp::diving_game::DivingGame::touchDrag`
- `rp::diving_game::DivingGame::touchUp`
- `rp::diving_game::DivingGame::updateButtonStates`
- `rp::diving_game::DivingGame::wasExitButtonTouched`
- `rp::diving_game::DivingGame::wasLeftButtonTouched`
- `rp::diving_game::DivingGame::wasRightButtonTouched`
- `rp::diving_game::DivingGame::~DivingGame`
