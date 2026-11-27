# Remaining reconstruction

Pass 33: 503/1191 original qualified method names matched (42.2334%). 688 names remain absent. Target: 50%.

The comparison uses the original ARM32 ELF and the corresponding compiled host static archive. Matching a name does not prove signature, ABI or functional equivalence. Methods with existing names may still be partial. The list below is a symbol inventory, not a list of verified missing player-facing features.

## Pass 35 source-level updates (awaiting compiled measurement)

The Pass 33 symbol inventory is the last measured baseline; the list below is provisionally adjusted for source-level additions. Pass 34's ARM64 CI build succeeded. Source changes since that baseline include:

- `rp::RaftPirates::GotMsgShowStore` and `gotMsgBoughtCurrency` (Pass 34): portable catalog and purchase-response adapters, not original popup or billing implementations.
- `rp::RaftPirates::GotMsgStartTransition` and `GotTransitionMsgDone` (Pass 35): original network queue pause/resume behavior recovered at ARM addresses 0x4a180c and 0x4a1840. Native transition graphics and completion-event broadcast still need reconstruction; the new handlers are not yet attached to the actual loading/menu animation events.
- Next priorities: connect transition graphics and menu events, ship naming keyboard lifecycle, purchase/repair confirmation protocol, tutorial state, and original battle/build mode transitions. Do not infer new server commands from method names alone.

Names newly implemented in source are listed here and removed from the provisional list below. Recompute the compiled inventory from a host archive before changing measured counts.

## Pass 36 source-level updates

- OceanView: retained sprites keyed by raft/object ID with owned visual snapshots, geometry/priority updates, animation overrides, picking, opacity, health overlays and water-bucket eligibility. These are portable approximations; the original accessory sprite visuals, sound muting, named layers and boss gradient still need work. Explicit attack animations are not yet wired to battle packets. Existing render calls synchronize from authoritative player snapshots.
- RaftPirates: confirmed material purchase, blueprint-material request generation, gold repair, display-name keyboard result, tutorial bookkeeping (no external analytics), mode tracking, social account-change cleanup, touch keep-alive, four-menu preloading. Purchase/repair and name requests are available as typed methods, but original menu triggers and all server responses are still incomplete. No debug/cheat requests or new privileged behavior were inferred.
- Android game runtime now uses Hydra's EngineAndroid for keyboard, popup, resume, exit and URL callbacks. Pass 35 ARM64 CI succeeded; Pass 36 verification is pending.

## Pass 37 source review

- Recovered DiscoveryModeState's exact state name (`rp::DiscoveryMode`), made treasure-map point lifetime and MaterialInfo copy construction explicit, and reclassified five already implemented accessor names from the original inventory.

- All 172 attached rp::Msg destructor names have portable source types (81 typed payload adapters, 49 no-argument event adapters, 42 opaque SFS payload holders). These are destructor/name-level additions, NOT original ABI or complete message dispatch. See docs/PASS37_MESSAGES.md.
- Recovered original gs_quest_reward branches and added authoritative material/blueprint/fastclock/sonar updates, queued typed reward notices and native fallback dialogs. Map-action and treasure-map data still rely on normal server snapshots; original animated reward popup wiring remains open.
- Remaining four debug handlers now reproduce observed request fields as opt-in loopback-only development request builders; not registered to live gameplay. No server authorization bypass is implemented.

- RaftPirates: menu-load state/dispatch, startup saved-session reconnect guard, popup result state and display-only material reward summary. Existing known-travel query methods are reclassified from stale missing-name entries. The actual quest-reward wire format, full original menu triggers and arbitrary reconnect flows are not recovered. Debug/cheat methods remain unresolved rather than inventing privileged requests.

- OceanView: added remaining six named adapters: tick registration/cleanup, layer access, boss water tint, mute state and diver load indicator. Boss tint and load indicator are portable approximations; actual per-raft audio playback and original boss gradient are not implemented. Mute state is available but not connected to an audio emitter. Tick registration controls slack updates, not the original accessory tick ABI.
- Added explicit GridSprite::SpritePiece and TouchControls destruction.
- Pass 36 ARM64 build passed (45926117694).

## Pass 38 source review

- Transition uses original anim_loading_curtains.bin intro/idle/outro compositions and typed MsgDone events. Connected portable mode-change detection to the existing network queue pause/resume and input guards. Original transition trigger timing, alternate state flags, and exact animation timing are not fully recovered. Fallback is a short dark fade only if the asset is unavailable.

- GfxOceanGradient: six RGB colors recovered from original ARM32 GRADIENTCOLOURS (0xa60a74), original anim_water_peak.bin path recovered. Integrated gradient and waves into OceanView; original zone ordering, wave placements, interpolation and timing are not established.
- GfxOceanGrid uses the OceanView transform, original truncating FloatMod equivalent and raft cell line rendering. Integrated in OceanView in place of hardcoded 192-pixel grid geometry.
- MsgQueuer owns SFS packet copies and drains the existing session queue in order; original polymorphic clone ABI is not recreated.
- DivingUnit constructor/copy, eligibility, local clock update and animation selection; animation selection integrated into OceanView. RequestDive checks eligibility rather than sending its own packet; authoritative Game::dive requests remain unchanged. GridObject::TempTick returns true as the original ARM32 method does. DivingGame::setPos is used by Reset.
- Reclassified already-present action/accessor methods (GridObject, Raft, RaftComponent, DivingUnit, Zone).
- Original binary has no source-file debug table; new files use recovered class names, existing paths preserved. Pass 37 final ARM64 build passed (45927489860).

## Pass 39 source review

- GridShapePiece: decoded original Filename/TextureOffset and local cell access, optional x/y transpose and parent-relative world positions. Reconstructed dependency sorting uses pairwise occupied-cell dominance and visited-before-recursion ordering (Dostuff_place at 0x475208, MyDataSortPredicate at 0x46e718); original complete bounding-box comparator and tie-breaking are not verified. Applied to piece order within GridSprite, not global cross-raft sorting.
- ItemPlacementSprite integrates live Game placement validation into the original footprint overlay, using footprint_1x1_red for invalid positions. Native Sprite ownership, selection/touch integration and full animation ABI remain open. Tests extend the source fixture for shape sorting and metadata.

- GridObjectHelpers: ARM32 0x41bed0–0x41bf58 compares native type 0/1/2; 0x41c150–0x41c28c compares exact names boss, boss01–boss05, barracks, DivingUnit. Portable helpers take an explicit catalogue instead of an intrusive object data pointer. SortGridObjPredicate uses original x-then-y comparison at 0x3aa8b8; Tokenize consumes the input and one delimiter byte (single-byte separators used in existing grid data parser). Multi-byte delimiter semantics remain unverified.
- FootprintSprite and nested SpriteFootprint now own and render placement sprites from the original construction_footprints_junk.xml atlas, integrated in PortView. UnitHealthBarSprite uses original gfx/HUD/health_meter.png and health_meter_green.pvr, replacing synthetic bars in OceanView; scale, offset and fill clipping are portable approximations. Original tick at 0x62f8b4 returns immediately. Tests added for tokenization, classification and comparison; not yet executed.

## Pass 40 engine dependency update

- Pass 39 follow-up ARM64 build passed and published (45929263936).
- Pass 40 is engine-focused: no new Raft Pirates method names are claimed. Hydra adds resource/animation XML descriptors, compressed PVR decoding, optional JPEG, touch tracking and menu controls. New ARM64 workflow records the Hydra source revision before building; the Pass 40 library is not yet verified.

## Pass 41 source review

- Raft::Salvage checks eligibility only, preserving existing server-authoritative destruction request and core/connectivity checks. Ocean::initWithSFSObject consumes the supported gs_opponent player_object packet, integrated with battle startup; not a general ocean snapshot reconstruction.
- Four message types hold existing Quest, StoreItem, purchase/progress fields; two retrieval adapters in RPMenuHandler. Native payload layout and dispatcher wiring remain unverified. The unknown dialog/scroll messages remain listed.

- RaftFactory/RaftComponentFactory use named default builders and are called by Player/Raft snapshot parsing. Only the generic Raft and RaftComponent classes are registered; original specialized factories and their signatures remain unknown.
- EnemyShipPoints explicit construction and lifetime; ItemFloater owns movement inside the existing diving game. No original minigame mechanics or graphics parity is claimed.
- Pass 40 ARM64 build passed; Pass 41 uses the updated Hydra source.

## Priority gaps

- Original menu scripts: complete game/SWIG bindings, message dispatch, dynamic templates, tutorial and quest flows, all popups and transitions.
- Construction: original placement sprites, completion notifications, barracks, repair/salvage calculations, complete construction/upgrade flow and animation.
- Ocean: persistent sprite lifecycle, picking, particles, debris/wreckage, discovery interactions, accurate map confirmations and treasure movement.
- Combat: original battle HUD callbacks, placement, targeting, dice and turn synchronization, animation, plunder, boss-specific progression and real multiplayer integration.
- Account/social/store: original saved login migration, account management, Facebook/social integration, currency purchases, notifications.
- Server: validation, full multiplayer and boss extensions and persistence require a compatible backend. No live Android or multiplayer validation has been performed.

## Remaining method names by class (provisional source review)

351 names from the Pass 33 inventory now have named source implementations through Pass 41 and have been removed from the provisional list below. This is not a compiled coverage increase; signatures and complete behavior remain unverified. The historical compiled measurements in MATCHING_STATUS.md and the README are unchanged.

### `rp::BuildModeState` (29)

`ApplyFootprintToRaft`, `ChangeView`, `CleanupFootprints`, `CreateGridObject`, `HandleExtensionResponse`, `Pause`, `SendBonusMaterials`, `SendMsgSelectedObjectAvailableActions`, `SetCheckForHoles`, `cancelNotificationForRaftComponent`, `doPlaceObject`, `gotMsgBuildModeToggleView`, `gotMsgCheckForHoles`, `gotMsgDivingGameDone`, `gotMsgDivingGameDoneMoving`, `gotMsgDivingGameShow`, `gotMsgGridObjectEvent`, `gotMsgHighlightHudItem`, `gotMsgLoadingAnimFinished`, `gotMsgOfferWallReward`, `gotMsgSparseGridObjectAdded`, `gotMsgToggleBlockHudItem`, `gotMsgTransitionDone`, `gotMsgWreckageSelected`, `name`, `repairObject`, `sendSelectedObjectRepairDataWithGold`, `setupNotificationForRaftComponent`, `tick`

### `rp::BattleModeState` (24)

`BattleModeState`, `HideDropdown`, `Pause`, `PlayBattleAnimations`, `RaftSelect`, `StartWaiting`, `StopWaiting`, `SwapPopup`, `SwapStance`, `ToBattleView`, `ToPositioningView`, `endBattleAndPlunder`, `gotMsgBattleConfirmSwapStance`, `gotMsgBattleInfoDropdown`, `gotMsgBattleInfoRequest`, `gotMsgBattleOverConfirm`, `gotMsgExitBattleModeRequest`, `gotMsgHighlightHudItem`, `gotMsgQuestRead`, `gotMsgToPositioningView`, `gotMsgToggleBlockHudItem`, `gotMsgTransitionDone`, `name`, `tick`

### `rp::Wreckage` (14)

`Duplicate`, `GetTotalBattlePoints`, `GetUserObjectID`, `GetWreckageObject`, `Pieces`, `RegisterAllClasses`, `Repair`, `Salvage`, `SetState`, `SetWreckageObject`, `Wreckage`, `getAvailableActions`, `initWithSFSObject`, `~Wreckage`

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

### `rp::DivingUnit` (1)

`AddedToOcean`

### `rp::SelectedWallSprite` (7)

`Init`, `SelectedWallSprite`, `SetPriorities`, `SetPriority`, `Update`, `tick`, `~SelectedWallSprite`

### `rp::DiveMeterSprite` (6)

`DiveMeterSprite`, `Select`, `SetPriorities`, `SetTime`, `tick`, `~DiveMeterSprite`

### `rp::DiveUnitReadoutSprite` (6)

`DiveUnitReadoutSprite`, `SetPriorities`, `SetVisible`, `gotMsgDiveDepthChange`, `tick`, `~DiveUnitReadoutSprite`

### `rp::FireFXSprite` (6)

`FireFXSprite`, `Init`, `SetColour`, `SetVisible`, `Update`, `~FireFXSprite`

### `rp::GfxOceanCheckerBoard` (6)

`FloatMod`, `GfxOceanCheckerBoard`, `Render`, `SetPosition`, `SetUnitSize`, `~GfxOceanCheckerBoard`

### `rp::GridObject` (3)

`AddedToOcean`, `Salvage`, `setGridObjectData`

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

### `rp::BattleModeState::BattleDice` (2)

`BattleDice`, `~BattleDice`

### `rp::DiscoveryGridObject` (2)

`DiscoveryGridObject`, `~DiscoveryGridObject`

### `rp::DiveUnitReadoutSprite::ReadoutLight` (2)

`ReadoutLight`, `~ReadoutLight`

### `rp::MsgStartDialog` (2)

`MsgStartDialog`, `~MsgStartDialog`

### `rp::MsgTabbedItemScrollTo` (2)

`MsgTabbedItemScrollTo`, `~MsgTabbedItemScrollTo`

### `rp::RaftComponent` (1)

`AddedToOcean`

### `rp::BlueprintInfo` (1)

`~BlueprintInfo`

### `rp::PerimeterWallSprite::SpriteBalloon` (1)

`~SpriteBalloon`

### `rp::QuestTypeInfo` (1)

`~QuestTypeInfo`

### `rp::RaftInfo` (1)

`RaftInfo`

### `rp::diving_game::DivingMaterial` (1)

`DivingMaterial`

