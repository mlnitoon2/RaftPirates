# Matching status

Generated from defined function symbols. Name coverage is unique qualified method names, not source-code recovery or instruction matching. Overloads are grouped by name; signature overlap means at least one demangled signature overlaps. Neither metric establishes behavior or ABI equivalence. Binary matching is **not measured** (legacy ARM32 versus host build).

| Original build | Names matched | Coverage | Names with signature overlap | Binary match |
|---|---:|---:|---:|---|
| Raft Pirates 2013 | 482/1191 | 40.4702% | 224 | Not compared |

## Raft Pirates 2013

Baseline SHA-256 (.so): `29544f0b70c947e23b05c01016a88b94b90e28fd204ba956daea16f6fdd26c13`

| Class or namespace | Name matches | Coverage | Signature overlap |
|---|---:|---:|---:|
| `rp` | 0/11 | 0.00% | 0 |
| `rp::AccessorySprite` | 0/11 | 0.00% | 0 |
| `rp::Barracks` | 0/11 | 0.00% | 0 |
| `rp::BattleArrowSprite` | 0/3 | 0.00% | 0 |
| `rp::BattleModeState` | 7/31 | 22.58% | 1 |
| `rp::BattleModeState::BattleDice` | 0/2 | 0.00% | 0 |
| `rp::BattlePlacementScreen` | 12/15 | 80.00% | 5 |
| `rp::BattlePointsViewSprite` | 0/4 | 0.00% | 0 |
| `rp::BattleShipPlacementSprite` | 12/23 | 52.17% | 3 |
| `rp::Blueprint` | 5/6 | 83.33% | 3 |
| `rp::BlueprintInfo` | 0/1 | 0.00% | 0 |
| `rp::BlueprintsDB` | 8/8 | 100.00% | 4 |
| `rp::BossBattleRaftHighlightSprite` | 0/5 | 0.00% | 0 |
| `rp::BuildModeState` | 34/63 | 53.97% | 8 |
| `rp::DamageIndicatorSprite` | 0/3 | 0.00% | 0 |
| `rp::Discovery` | 5/6 | 83.33% | 4 |
| `rp::DiscoveryDB` | 6/6 | 100.00% | 3 |
| `rp::DiscoveryGridObject` | 0/2 | 0.00% | 0 |
| `rp::DiscoveryModeState` | 8/9 | 88.89% | 3 |
| `rp::DiveMeterSprite` | 0/6 | 0.00% | 0 |
| `rp::DiveUnitReadoutSprite` | 0/6 | 0.00% | 0 |
| `rp::DiveUnitReadoutSprite::ReadoutLight` | 0/2 | 0.00% | 0 |
| `rp::DivingUnit` | 15/22 | 68.18% | 12 |
| `rp::DoStuffExtraData` | 0/1 | 0.00% | 0 |
| `rp::ExplosionFXSprite` | 0/8 | 0.00% | 0 |
| `rp::FireFXSprite` | 0/6 | 0.00% | 0 |
| `rp::FootprintSprite` | 0/7 | 0.00% | 0 |
| `rp::FootprintSprite::SpriteFootprint` | 0/1 | 0.00% | 0 |
| `rp::GfxBattleInfoDropDown` | 0/5 | 0.00% | 0 |
| `rp::GfxBattleScreen` | 6/11 | 54.55% | 4 |
| `rp::GfxOceanCheckerBoard` | 0/6 | 0.00% | 0 |
| `rp::GfxOceanGradient` | 0/7 | 0.00% | 0 |
| `rp::GfxOceanGrid` | 0/6 | 0.00% | 0 |
| `rp::GfxShipPreviewScreen` | 0/3 | 0.00% | 0 |
| `rp::GridObject` | 20/26 | 76.92% | 7 |
| `rp::GridObject::GridShapePiece` | 0/5 | 0.00% | 0 |
| `rp::GridObjectDataRes` | 0/3 | 0.00% | 0 |
| `rp::GridObjectInfo` | 1/1 | 100.00% | 0 |
| `rp::GridObjectPiece` | 2/2 | 100.00% | 1 |
| `rp::GridObjectsDB` | 7/8 | 87.50% | 1 |
| `rp::GridSprite` | 23/33 | 69.70% | 15 |
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
| `rp::LoadingManager` | 13/13 | 100.00% | 6 |
| `rp::MapView` | 37/46 | 80.43% | 15 |
| `rp::MapView::EnemyShipPoints` | 0/2 | 0.00% | 0 |
| `rp::MapView::TreasureMapPoints` | 0/1 | 0.00% | 0 |
| `rp::Material` | 5/6 | 83.33% | 4 |
| `rp::MaterialInfo` | 1/2 | 50.00% | 1 |
| `rp::MaterialsDB` | 6/6 | 100.00% | 2 |
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
| `rp::Ocean` | 9/10 | 90.00% | 5 |
| `rp::OceanView` | 25/62 | 40.32% | 17 |
| `rp::OverlapTree` | 0/4 | 0.00% | 0 |
| `rp::PerimeterTree` | 0/3 | 0.00% | 0 |
| `rp::PerimeterWallSprite` | 0/10 | 0.00% | 0 |
| `rp::PerimeterWallSprite::SpriteBalloon` | 0/1 | 0.00% | 0 |
| `rp::PlacementSprite` | 0/9 | 0.00% | 0 |
| `rp::Player` | 29/30 | 96.67% | 21 |
| `rp::Quest` | 6/6 | 100.00% | 5 |
| `rp::QuestTypeDB` | 5/5 | 100.00% | 2 |
| `rp::QuestTypeInfo` | 0/1 | 0.00% | 0 |
| `rp::RPMenuHandler` | 39/47 | 82.98% | 6 |
| `rp::Raft` | 14/16 | 87.50% | 7 |
| `rp::RaftAttackSprite` | 0/5 | 0.00% | 0 |
| `rp::RaftComponent` | 13/15 | 86.67% | 8 |
| `rp::RaftComponentFactory` | 0/2 | 0.00% | 0 |
| `rp::RaftFactory` | 0/2 | 0.00% | 0 |
| `rp::RaftGridSprite` | 0/6 | 0.00% | 0 |
| `rp::RaftInfo` | 0/1 | 0.00% | 0 |
| `rp::RaftPirates` | 33/86 | 38.37% | 10 |
| `rp::RaftPiratesGameState` | 0/6 | 0.00% | 0 |
| `rp::SelectedWallSprite` | 0/7 | 0.00% | 0 |
| `rp::ShipPlacementSprite` | 0/11 | 0.00% | 0 |
| `rp::SparseGrid` | 10/15 | 66.67% | 1 |
| `rp::SplashFXSprite` | 0/6 | 0.00% | 0 |
| `rp::StoreDB` | 6/6 | 100.00% | 3 |
| `rp::ThrobingIndicatorSprite` | 0/3 | 0.00% | 0 |
| `rp::TouchControls` | 14/15 | 93.33% | 6 |
| `rp::Transition` | 0/8 | 0.00% | 0 |
| `rp::Transition::MsgDone` | 0/1 | 0.00% | 0 |
| `rp::UnitHealthBarSprite` | 0/7 | 0.00% | 0 |
| `rp::WaterBucketSprite` | 0/3 | 0.00% | 0 |
| `rp::Wreckage` | 0/14 | 0.00% | 0 |
| `rp::WreckageSprite` | 0/6 | 0.00% | 0 |
| `rp::Zone` | 11/13 | 84.62% | 10 |
| `rp::ZonesDB` | 7/7 | 100.00% | 4 |
| `rp::diving_game::DivingCollectable` | 1/1 | 100.00% | 0 |
| `rp::diving_game::DivingGame` | 27/28 | 96.43% | 17 |
| `rp::diving_game::DivingGame::ItemFloater` | 0/3 | 0.00% | 0 |
| `rp::diving_game::DivingMaterial` | 0/1 | 0.00% | 0 |
| `rp::diving_game::ScrollingLayer` | 0/4 | 0.00% | 0 |

Matched names:

- `rp::BattleModeState::GetStateName`
- `rp::BattleModeState::HandleExtensionResponse`
- `rp::BattleModeState::SendClientReady`
- `rp::BattleModeState::SendRaftSelection`
- `rp::BattleModeState::gotMsgAttackRequest`
- `rp::BattleModeState::gotMsgBattleScreenRaftsSelected`
- `rp::BattleModeState::~BattleModeState`
- `rp::BattlePlacementScreen::BattlePlacementScreen`
- `rp::BattlePlacementScreen::GetClosestPosition`
- `rp::BattlePlacementScreen::GetWorldPosition`
- `rp::BattlePlacementScreen::OkayToPlace`
- `rp::BattlePlacementScreen::Pieces`
- `rp::BattlePlacementScreen::Reset`
- `rp::BattlePlacementScreen::Select`
- `rp::BattlePlacementScreen::SetPosition`
- `rp::BattlePlacementScreen::SetToObject`
- `rp::BattlePlacementScreen::Update`
- `rp::BattlePlacementScreen::gotMsgRequestBattlePlacementState`
- `rp::BattlePlacementScreen::~BattlePlacementScreen`
- `rp::BattleShipPlacementSprite::BattleShipPlacementSprite`
- `rp::BattleShipPlacementSprite::GetClosestPosition`
- `rp::BattleShipPlacementSprite::HighlightRaftsNextTo`
- `rp::BattleShipPlacementSprite::OkayToPlace`
- `rp::BattleShipPlacementSprite::OnlyNextToWreckage`
- `rp::BattleShipPlacementSprite::Pieces`
- `rp::BattleShipPlacementSprite::Revert`
- `rp::BattleShipPlacementSprite::SelectRaft`
- `rp::BattleShipPlacementSprite::SetObjectPosition`
- `rp::BattleShipPlacementSprite::SetPositionOfSprites`
- `rp::BattleShipPlacementSprite::Update`
- `rp::BattleShipPlacementSprite::~BattleShipPlacementSprite`
- `rp::Blueprint::Blueprint`
- `rp::Blueprint::id`
- `rp::Blueprint::init`
- `rp::Blueprint::initWithSFSObject`
- `rp::Blueprint::~Blueprint`
- `rp::BlueprintsDB::BlueprintsDB`
- `rp::BlueprintsDB::getAllBlueprintsAvail`
- `rp::BlueprintsDB::getBlueprintDataFromSFSObjectWrapper`
- `rp::BlueprintsDB::getBlueprintInfo`
- `rp::BlueprintsDB::getBlueprintInfoFromGridObjectID`
- `rp::BlueprintsDB::initWithSFSObject`
- `rp::BlueprintsDB::load`
- `rp::BlueprintsDB::save`
- `rp::BuildModeState::BuildModeState`
- `rp::BuildModeState::CheckForHoles`
- `rp::BuildModeState::FinishWaiting`
- `rp::BuildModeState::GetStateName`
- `rp::BuildModeState::StartWaiting`
- `rp::BuildModeState::UseClockSelected`
- `rp::BuildModeState::checkFocusRaft`
- `rp::BuildModeState::deselectObject`
- `rp::BuildModeState::gotMsgBuildCancel`
- `rp::BuildModeState::gotMsgBuildConfirm`
- `rp::BuildModeState::gotMsgDeselectObject`
- `rp::BuildModeState::gotMsgDestroyObjectRequest`
- `rp::BuildModeState::gotMsgGridObjectSelected`
- `rp::BuildModeState::gotMsgMoveObjectRequest`
- `rp::BuildModeState::gotMsgNextWreckedRaft`
- `rp::BuildModeState::gotMsgNextWreckedRaftComponent`
- `rp::BuildModeState::gotMsgPreviousWreckedRaft`
- `rp::BuildModeState::gotMsgPreviousWreckedRaftComponent`
- `rp::BuildModeState::gotMsgRepairSelectedObject`
- `rp::BuildModeState::gotMsgRepairSelectedObjectAll`
- `rp::BuildModeState::gotMsgRequestNumWreckedRaftComponents`
- `rp::BuildModeState::gotMsgRequestNumWreckedRafts`
- `rp::BuildModeState::gotMsgSalvageSelectedObject`
- `rp::BuildModeState::gotMsgStartBuildRequest`
- `rp::BuildModeState::gotMsgUseClockOnSelectedObject`
- `rp::BuildModeState::initWreckedRaftCheck`
- `rp::BuildModeState::initWreckedRaftComponentCheck`
- `rp::BuildModeState::isReadyForBattle`
- `rp::BuildModeState::repairSelected`
- `rp::BuildModeState::salvageSelected`
- `rp::BuildModeState::selectObject`
- `rp::BuildModeState::selectWreckedRaft`
- `rp::BuildModeState::selectWreckedRaftComponent`
- `rp::BuildModeState::~BuildModeState`
- `rp::Discovery::id`
- `rp::Discovery::initWithSFSObject`
- `rp::Discovery::questgroupid`
- `rp::Discovery::zoneid`
- `rp::Discovery::~Discovery`
- `rp::DiscoveryDB::DiscoveryDB`
- `rp::DiscoveryDB::buildData`
- `rp::DiscoveryDB::getById`
- `rp::DiscoveryDB::initWithSFSObject`
- `rp::DiscoveryDB::load`
- `rp::DiscoveryDB::save`
- `rp::DiscoveryModeState::DiscoveryModeState`
- `rp::DiscoveryModeState::GotMsgTransitionDone`
- `rp::DiscoveryModeState::HandleExtensionResponse`
- `rp::DiscoveryModeState::LeaveDiscovery`
- `rp::DiscoveryModeState::LeaveDiscoveryReally`
- `rp::DiscoveryModeState::Pause`
- `rp::DiscoveryModeState::tick`
- `rp::DiscoveryModeState::~DiscoveryModeState`
- `rp::DivingUnit::Diving`
- `rp::DivingUnit::FinishDive`
- `rp::DivingUnit::GetDiveTime`
- `rp::DivingUnit::GetLoadValue`
- `rp::DivingUnit::GetMaxLoad`
- `rp::DivingUnit::OkayToDive`
- `rp::DivingUnit::SetDiveLevel`
- `rp::DivingUnit::SetLoadValue`
- `rp::DivingUnit::SetState`
- `rp::DivingUnit::StartDive`
- `rp::DivingUnit::TempTick`
- `rp::DivingUnit::initWithSFSObject`
- `rp::DivingUnit::numAvailableBonusMaterials`
- `rp::DivingUnit::numCollectedBonusMaterials`
- `rp::DivingUnit::~DivingUnit`
- `rp::GfxBattleScreen::GfxBattleScreen`
- `rp::GfxBattleScreen::Render`
- `rp::GfxBattleScreen::SwoopIn`
- `rp::GfxBattleScreen::SwoopOut`
- `rp::GfxBattleScreen::tick`
- `rp::GfxBattleScreen::~GfxBattleScreen`
- `rp::GridObject::Beside`
- `rp::GridObject::Contains`
- `rp::GridObject::Destroyed`
- `rp::GridObject::Duplicate`
- `rp::GridObject::GetBattlePoints`
- `rp::GridObject::GetGridObjectID`
- `rp::GridObject::GetGridPosition`
- `rp::GridObject::GetHealth`
- `rp::GridObject::GetTotalBattlePoints`
- `rp::GridObject::GetUserObjectID`
- `rp::GridObject::GetWorldPosition`
- `rp::GridObject::Middleish`
- `rp::GridObject::Overlap`
- `rp::GridObject::Pieces`
- `rp::GridObject::Repair`
- `rp::GridObject::SetBattlePoints`
- `rp::GridObject::SetState`
- `rp::GridObject::getResourceData`
- `rp::GridObject::getType`
- `rp::GridObject::~GridObject`
- `rp::GridObjectInfo::GridObjectInfo`
- `rp::GridObjectPiece::GridObjectPiece`
- `rp::GridObjectPiece::~GridObjectPiece`
- `rp::GridObjectsDB::GridObjectsDB`
- `rp::GridObjectsDB::getBattlePointsFromGridObject`
- `rp::GridObjectsDB::getDataString`
- `rp::GridObjectsDB::getNewGridObjectData`
- `rp::GridObjectsDB::initWithSFSObject`
- `rp::GridObjectsDB::load`
- `rp::GridObjectsDB::save`
- `rp::GridSprite::AddAnimSlack`
- `rp::GridSprite::Alpha`
- `rp::GridSprite::Attack`
- `rp::GridSprite::ContainsGfx`
- `rp::GridSprite::GetMaxOrder`
- `rp::GridSprite::GetOrder`
- `rp::GridSprite::GetPriority`
- `rp::GridSprite::GetVisible`
- `rp::GridSprite::GetWorldPosition`
- `rp::GridSprite::GridSprite`
- `rp::GridSprite::HasAttack`
- `rp::GridSprite::Init`
- `rp::GridSprite::Pieces`
- `rp::GridSprite::Select`
- `rp::GridSprite::SetAnimation`
- `rp::GridSprite::SetColour`
- `rp::GridSprite::SetOrder`
- `rp::GridSprite::SetPriorities`
- `rp::GridSprite::SetPriority`
- `rp::GridSprite::SetVisible`
- `rp::GridSprite::Update`
- `rp::GridSprite::UpdateAnimations`
- `rp::GridSprite::~GridSprite`
- `rp::LoadingManager::LoadingManager`
- `rp::LoadingManager::addAEAnim`
- `rp::LoadingManager::addManifest`
- `rp::LoadingManager::addMenu`
- `rp::LoadingManager::addSheet`
- `rp::LoadingManager::addSprite`
- `rp::LoadingManager::addXml`
- `rp::LoadingManager::cancel`
- `rp::LoadingManager::resourceExists`
- `rp::LoadingManager::setPercentage`
- `rp::LoadingManager::start`
- `rp::LoadingManager::update`
- `rp::LoadingManager::~LoadingManager`
- `rp::MapView::DeleteShipPreview`
- `rp::MapView::MapView`
- `rp::MapView::addTreasureMapPoint`
- `rp::MapView::clearEnemyShips`
- `rp::MapView::clearTreasureMapPoints`
- `rp::MapView::getSafeSquares`
- `rp::MapView::getUnsafeSquares`
- `rp::MapView::hasBossNearArea`
- `rp::MapView::hasPlayerNearArea`
- `rp::MapView::hasSonarIconNearArea`
- `rp::MapView::hasXNearArea`
- `rp::MapView::hideMap`
- `rp::MapView::isPosInSafeWater`
- `rp::MapView::isPosOnLand`
- `rp::MapView::isValidIconLocation`
- `rp::MapView::limitPlayerPos`
- `rp::MapView::moveShip`
- `rp::MapView::movementAllowed`
- `rp::MapView::selectEnemy`
- `rp::MapView::setMapFile`
- `rp::MapView::setPlayerPosition`
- `rp::MapView::setPlayerVisible`
- `rp::MapView::setTargetVisible`
- `rp::MapView::showMap`
- `rp::MapView::snapPlayerToBoss`
- `rp::MapView::snapPlayerToEnemyShips`
- `rp::MapView::snapPlayerToTreasureMaps`
- `rp::MapView::tick`
- `rp::MapView::touchCancel`
- `rp::MapView::touchDown`
- `rp::MapView::touchDrag`
- `rp::MapView::touchUp`
- `rp::MapView::updateBoss`
- `rp::MapView::updateBossGfx`
- `rp::MapView::updateEnemyShips`
- `rp::MapView::updateTreasureMapPoints`
- `rp::MapView::~MapView`
- `rp::Material::Material`
- `rp::Material::id`
- `rp::Material::init`
- `rp::Material::initWithSFSObject`
- `rp::Material::~Material`
- `rp::MaterialInfo::~MaterialInfo`
- `rp::MaterialsDB::MaterialsDB`
- `rp::MaterialsDB::getMaterialInfo`
- `rp::MaterialsDB::getMaterialInfoFromSFSData`
- `rp::MaterialsDB::initWithSFSObject`
- `rp::MaterialsDB::load`
- `rp::MaterialsDB::save`
- `rp::Ocean::GetPlayer`
- `rp::Ocean::Ocean`
- `rp::Ocean::addPlayer`
- `rp::Ocean::gotMsgPlayerBuiltRaft`
- `rp::Ocean::init`
- `rp::Ocean::movePlayer`
- `rp::Ocean::removePlayer`
- `rp::Ocean::tick`
- `rp::Ocean::~Ocean`
- `rp::OceanView::AxisX`
- `rp::OceanView::AxisY`
- `rp::OceanView::CalculateHelpers`
- `rp::OceanView::ConvertToView`
- `rp::OceanView::ConvertToViewSpecial`
- `rp::OceanView::ConvertToWorld`
- `rp::OceanView::Height`
- `rp::OceanView::Offset`
- `rp::OceanView::Pick`
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
- `rp::Player::EastMostPoint`
- `rp::Player::GetAvailableEdgeGridSpace`
- `rp::Player::GetDiveUnit`
- `rp::Player::GetRaftAtPosition`
- `rp::Player::GetRaftByID`
- `rp::Player::Player`
- `rp::Player::RaftCleanupRequired`
- `rp::Player::RaftRepairRequired`
- `rp::Player::RaftsConnectedToRaft`
- `rp::Player::RemoveRaft`
- `rp::Player::WestMostPoint`
- `rp::Player::canBuildRaft`
- `rp::Player::currentBP`
- `rp::Player::getBoundingBox`
- `rp::Player::giveBlueprint`
- `rp::Player::giveMaterial`
- `rp::Player::goldCostToBuild`
- `rp::Player::hasBlueprint`
- `rp::Player::hasMaterial`
- `rp::Player::hasRaft`
- `rp::Player::init`
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
- `rp::QuestTypeDB::QuestTypeDB`
- `rp::QuestTypeDB::getQuestTypeInfo`
- `rp::QuestTypeDB::initWithSFSObject`
- `rp::QuestTypeDB::load`
- `rp::QuestTypeDB::save`
- `rp::RPMenuHandler::RPMenuHandler`
- `rp::RPMenuHandler::destroyCurMenuData`
- `rp::RPMenuHandler::displayPopUpImmediately`
- `rp::RPMenuHandler::enableNewQuestPopupHandling`
- `rp::RPMenuHandler::gotMsgBlueprintDisplayRequest`
- `rp::RPMenuHandler::gotMsgBlueprintPageRequest`
- `rp::RPMenuHandler::gotMsgGetAllBlueprintsRequest`
- `rp::RPMenuHandler::gotMsgGetAvailBlueprintsRequest`
- `rp::RPMenuHandler::gotMsgGetAvailMaterialsRequest`
- `rp::RPMenuHandler::gotMsgGetAvailStoreItemRequest`
- `rp::RPMenuHandler::gotMsgHaveMaterialsRequest`
- `rp::RPMenuHandler::gotMsgHudModeRequest`
- `rp::RPMenuHandler::gotMsgMaterialInformationRequest`
- `rp::RPMenuHandler::gotMsgQuestInformationRequest`
- `rp::RPMenuHandler::gotMsgQuestTypeInformationRequest`
- `rp::RPMenuHandler::gotMsgQueueMenuPopup`
- `rp::RPMenuHandler::gotMsgRecipeInformationRequest`
- `rp::RPMenuHandler::gotMsgRecipeSelectedRequest`
- `rp::RPMenuHandler::gotMsgRequestOceanSafetyUpdate`
- `rp::RPMenuHandler::gotMsgSetBlueprintPage`
- `rp::RPMenuHandler::gotMsgSetBlueprintSelected`
- `rp::RPMenuHandler::gotMsgSetHudModeRequest`
- `rp::RPMenuHandler::gotMsgSetStoreCategory`
- `rp::RPMenuHandler::gotMsgSetStoreItemPage`
- `rp::RPMenuHandler::gotMsgSetStoreItemSelected`
- `rp::RPMenuHandler::gotMsgShowBattlePointsRequest`
- `rp::RPMenuHandler::gotMsgSpecificMaterialAvailAmountRequest`
- `rp::RPMenuHandler::gotMsgStoreItemDisplayRequest`
- `rp::RPMenuHandler::gotMsgStoreItemPageRequest`
- `rp::RPMenuHandler::gotMsgStoreItemSelectedRequest`
- `rp::RPMenuHandler::hasAnyQueuedPopups`
- `rp::RPMenuHandler::popMenu`
- `rp::RPMenuHandler::popPopUp`
- `rp::RPMenuHandler::pushMenu`
- `rp::RPMenuHandler::pushPopUp`
- `rp::RPMenuHandler::setHudMode`
- `rp::RPMenuHandler::showQuestDialogImmediately`
- `rp::RPMenuHandler::tick`
- `rp::RPMenuHandler::~RPMenuHandler`
- `rp::Raft::CanAddObject`
- `rp::Raft::Duplicate`
- `rp::Raft::GetBattlePoints`
- `rp::Raft::GetHealth`
- `rp::Raft::GetTotalBattlePoints`
- `rp::Raft::GridDimensionsX`
- `rp::Raft::GridDimensionsY`
- `rp::Raft::HandleBattleOutcome`
- `rp::Raft::Raft`
- `rp::Raft::SetState`
- `rp::Raft::initWithSFSObject`
- `rp::Raft::raftSizeX`
- `rp::Raft::raftSizeY`
- `rp::Raft::~Raft`
- `rp::RaftComponent::Building`
- `rp::RaftComponent::Duplicate`
- `rp::RaftComponent::GetBattlePoints`
- `rp::RaftComponent::GetRemainingTime`
- `rp::RaftComponent::GetTotalBattlePoints`
- `rp::RaftComponent::PostBuild`
- `rp::RaftComponent::RaftComponent`
- `rp::RaftComponent::SetRemainingTime`
- `rp::RaftComponent::SetState`
- `rp::RaftComponent::TempTick`
- `rp::RaftComponent::UseClock`
- `rp::RaftComponent::initWithSFSObject`
- `rp::RaftComponent::~RaftComponent`
- `rp::RaftPirates::DoConnect`
- `rp::RaftPirates::DoLostConnection`
- `rp::RaftPirates::GotMsgBadZoneStatusRequest`
- `rp::RaftPirates::GotMsgGetMapActionState`
- `rp::RaftPirates::Pause`
- `rp::RaftPirates::RaftPirates`
- `rp::RaftPirates::checkKeepAlive`
- `rp::RaftPirates::gotMsgDaringMeterInfo`
- `rp::RaftPirates::gotMsgDiscoveryMoveTo`
- `rp::RaftPirates::gotMsgEnableNetworkProcessing`
- `rp::RaftPirates::gotMsgLoadFinished`
- `rp::RaftPirates::gotMsgLoadPercentage`
- `rp::RaftPirates::gotMsgLoadStart`
- `rp::RaftPirates::gotMsgMenuLogoutRequest`
- `rp::RaftPirates::gotMsgOnAdminMessage`
- `rp::RaftPirates::gotMsgOnConnection`
- `rp::RaftPirates::gotMsgOnConnectionLost`
- `rp::RaftPirates::gotMsgOnExtensionResponse`
- `rp::RaftPirates::gotMsgOnLogin`
- `rp::RaftPirates::gotMsgOnLoginError`
- `rp::RaftPirates::gotMsgOnLogout`
- `rp::RaftPirates::gotMsgOnRoomJoin`
- `rp::RaftPirates::gotMsgQuestEvent`
- `rp::RaftPirates::gotMsgQuestOpened`
- `rp::RaftPirates::gotMsgZoneMoveTo`
- `rp::RaftPirates::handleLogOut`
- `rp::RaftPirates::initialize`
- `rp::RaftPirates::isDaringZoneLimited`
- `rp::RaftPirates::performLogout`
- `rp::RaftPirates::setNetworkMsgProcessingEnabled`
- `rp::RaftPirates::tick`
- `rp::RaftPirates::tickNoSkip`
- `rp::RaftPirates::~RaftPirates`
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
- `rp::StoreDB::StoreDB`
- `rp::StoreDB::getStoreInfo`
- `rp::StoreDB::initWithSFSObject`
- `rp::StoreDB::initializeStore`
- `rp::StoreDB::load`
- `rp::StoreDB::save`
- `rp::TouchControls::LockView`
- `rp::TouchControls::LockZoom`
- `rp::TouchControls::PanTo`
- `rp::TouchControls::Pick`
- `rp::TouchControls::SetLockedView`
- `rp::TouchControls::SetZoomLimits`
- `rp::TouchControls::TouchControls`
- `rp::TouchControls::gotMsgOceanViewPickResult`
- `rp::TouchControls::onZoom`
- `rp::TouchControls::tick`
- `rp::TouchControls::touchCancel`
- `rp::TouchControls::touchDown`
- `rp::TouchControls::touchDrag`
- `rp::TouchControls::touchUp`
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
- `rp::Zone::~Zone`
- `rp::ZonesDB::ZonesDB`
- `rp::ZonesDB::buildData`
- `rp::ZonesDB::getById`
- `rp::ZonesDB::initWithSFSObject`
- `rp::ZonesDB::load`
- `rp::ZonesDB::save`
- `rp::ZonesDB::~ZonesDB`
- `rp::diving_game::DivingCollectable::DivingCollectable`
- `rp::diving_game::DivingGame::AutoScaleText`
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
- `rp::diving_game::DivingGame::onMsgTouchState`
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
