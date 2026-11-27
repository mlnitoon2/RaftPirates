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

- Original ARM32 RaftInfo constructor at 0x413180 reads type_id, battle_points, position_x, position_y (string addresses 0xa60afc, 0xa60468, 0xa60b04, 0xa60b10); the portable record preserves the four integer fields and order. MsgTabbedItemScrollTo at 0x4a05f4 copies a single string, now represented in the message record. Original menu scroll dispatch remains open. EnemyShipPoints additionally exposes the original named copy constructor (0x443244), using portable value semantics.
- Binary inspection found the DiscoveryGridObject constructor calls setGridObjectData("barnacle_bills") and WaterBucketSprite::tick subtracts dt from a one-second phase and changes scale. These methods remain listed until the corresponding asset/runtime bindings are recovered.

- Raft::Salvage checks eligibility only, preserving existing server-authoritative destruction request and core/connectivity checks. Ocean::initWithSFSObject consumes the supported gs_opponent player_object packet, integrated with battle startup; not a general ocean snapshot reconstruction.
- Four message types hold existing Quest, StoreItem, purchase/progress fields; two retrieval adapters in RPMenuHandler. Native payload layout and dispatcher wiring remain unverified. The unknown dialog/scroll messages remain listed.

- RaftFactory/RaftComponentFactory use named default builders and are called by Player/Raft snapshot parsing. Only the generic Raft and RaftComponent classes are registered; original specialized factories and their signatures remain unknown.
- EnemyShipPoints explicit construction and lifetime; ItemFloater owns movement inside the existing diving game. No original minigame mechanics or graphics parity is claimed.
- Pass 40 ARM64 build passed; Pass 41 uses the updated Hydra source.

## Pass 42 engine dependency update

- Pass 41 final ARM64 build 45930634598 passed and published. Hydra Pass 42 reaches 1263/1263 provisional source-name representations, including aliases and partial adapters; it does not reach full behavior or compiled-symbol equivalence. No new Raft Pirates method names were addressed in this engine-focused pass.
- This commit triggers the ARM64 workflow against the current HydraLegacy main revision. Pass 42 build and runtime results are not established yet.

## Pass 43 shop audit

- Inspected original APK assets/menus/tabbed_menu_{blueprints,misc,gold,diamonds,free,maps,inventory}.xml and menu_templates/TabbedMenuItem.xml. Original store categories use numeric requests (0 blueprints, 1 misc, 2 gold, 5 diamonds); original item template uses 20% of screen width, currency/price visibility changes by category, refresh listeners and native game messages. PortView currently consumes all Lua events, emulating only a subset of those scripts. This is a concrete shop compatibility gap independent of combat methods.
- Added category normalization for diamond/diamonds and gold/offerwall, viewport-relative item spacing, scroll-origin correction, original itemCost/currencyIcon visibility for cash/free/offer groups, and live refresh when item availability changes. Added category regression fixture. Category group aliases are a portable assumption; real server group mapping and free/social templates remain unverified.
- This batch does not add original method names; no coverage count increase. No runtime screenshot comparison performed.

## Pass 43 state and sprite review

- BuildModeState and BattleModeState: recovered exact state names and original return-only callbacks, added placement/repair routing, wreckage selection, connectivity checks, battle waiting timer and confirmation adapters. SendMsgSelectedObjectAvailableActions was already present and is reclassified. The portable countdown never authorizes server actions; original HUD/dispatcher wiring remains incomplete.
- Wreckage: original ARM32 methods at 0x50f588, 0x50f5e8 and 0x50f614 delegate ID, battle points and state to the wrapped object; 0x50fd9c reads offsetX/offsetY. Portable class owns an independent clone, not the original live grid transfer. Repair, salvage, registration, shape delegation and available actions remain unresolved.
- Barracks: Constructing checks state 4 (0x5398c0); native UseClock and SetState are return-only; snapshot and action queries delegate to RaftComponent. Portable countdown supports state 4 but does not create completed objects or deliver the original event. The component factory still stores values and cannot preserve subclasses.
- AccessorySprite: original base Init/Update/tick/SetScale return immediately, ListenForAnimation returns false. Portable attachment uses owned GridSprite entries and its tick loop rather than native global dispatcher. No concrete accessory visuals are added; GetTimeString and native registration behavior remain unverified.
- BattleShipPlacementSprite: portable selection, object/position adapters and adjacency highlighting, now refreshed when moving a fleet. Original weak UpdateParticles returns immediately; boss-core queries, graphics ownership, priority and scale remain open.
- 55 additional names addressed provisionally, including partial adapters and already-present methods. No new compiled comparison or runtime claim. Hydra static parity findings: [PASS43_PARITY.md](https://github.com/mlnitoon2/HydraLegacy/blob/main/docs/PASS43_PARITY.md).

## Pass 44 loader, geometry and visual review

- LoaderImage, LoaderManifest, LoaderMenu and Loader: recovered native SPR/MANI/MENU identifiers and integrated job dispatch. Manifest expansion is transactional; native resource-manager ABI is not reproduced.
- OverlapTree and PerimeterTree: portable occupied-cell bounds and exterior-edge extraction, with exclusive upper bounds; not the original recursive tree layout. Raft placement uses the bounds check. GridObjectDataRes and DiscoveryGridObject use catalogue snapshots, including the original barnacle_bills lookup.
- ScrollingLayer: speed multiplier and vertical motion from original instruction review; original diving atlas tiles wired into the minigame. Parallax rates and recycling are approximations.
- DamageIndicatorSprite: native 1.5-second lifetime and 20-pixel/second rise. BattleArrowSprite, WaterBucketSprite and ThrobingIndicatorSprite: native timing/scale constants and asset identifiers inform portable overlay implementations. ItemOnRaftPlacementSprite routes component placement through the existing placement adapter; full native priority behavior is unverified.
- Hydra named-layer fix preserves layer names and aligns text, sprite and touch ordering; exact original global layer ordering is not recovered.
- ARM64 build 45933481659 passed for the loader/geometry/battle/diving batch, using the preceding Hydra revision. Later changes and combined build need verification. Regression fixtures added but host tests and device visual parity are not yet established.
- 46 requested names provisionally addressed; 20 requested names remain. No new compiled original-symbol measurement.

## Pass 45 behavior and inventory review

- 47 requested names have provisional source representations, including adapters and two existing menu query methods. 27 requested names remain. Barracks::AddedToOcean was added only as an attachment flag; keep it in the remaining list until native-equivalent construction tick registration is integrated. No fresh compiled-symbol measurement.
- GridSprite animation slack is now added to elapsed time and consumed once, matching ARM32 0x5cd068. Accessory ownership, animation callbacks, piece updates and mute state are wired to the existing visual path; actual audio and particle backends are incomplete. AccessorySprite formats countdowns using the native %02d:%02d:%02d (0xa61708).
- MapView uses server snapshots with interpolated marker movement; show/hide and confirmed movement adapters are used by PortView. Tutorial and HUD-block handlers retain only portable state, not full native message parity.
- PlacementSprite shares base behavior with ItemPlacementSprite. Original base Select/OkayToPlace return false (0x5d763c/0x5d7644); nearest eligible edge and distance cutoff are portable choices. ShipPlacementSprite and PerimeterWallSprite are still open.
- ExplosionFXSprite uses 1.5 seconds for raft and 0.5 seconds for non-raft from native GetTime 0x5b48a0, with native Sparatic predicate (raft or boss weapon). Portable renderer uses an effect timer and existing animation, not original random burst generation.
- BossCoreRaftsInMe uses recovered name BossCoreRaft (0xa5f574) and excludes destroyed rafts; ByMe returns cached adjacent core IDs, not original raw raft pointers. No boss battle rule changes.
- Menu status/category/popups/quest adapters retain server authority; original Lua message response schema is incomplete. Original misc shop XML sets the item layer to HUD; portable item nodes now receive this value. Cross-sell is intentionally not revived.
- Pass 44 final ARM64 compile and 94 JNI exports passed; publication refused a stale revision because docs moved main. Pass 45 source commits not yet runtime tested. See PASS45_PARITY.md for evidence and limitations.

## Priority gaps

- Original menu scripts: complete game/SWIG bindings, message dispatch, dynamic templates, tutorial and quest flows, all popups and transitions.
- Construction: original placement sprites, completion notifications, barracks, repair/salvage calculations, complete construction/upgrade flow and animation.
- Ocean: persistent sprite lifecycle, picking, particles, debris/wreckage, discovery interactions, accurate map confirmations and treasure movement.
- Combat: original battle HUD callbacks, placement, targeting, dice and turn synchronization, animation, plunder, boss-specific progression and real multiplayer integration.
- Account/social/store: original saved login migration, account management, Facebook/social integration, currency purchases, notifications.
- Server: validation, full multiplayer and boss extensions and persistence require a compatible backend. No live Android or multiplayer validation has been performed.

## Pass 46 resource loaders

- Three resource loaders now use native type strings AE, SS and XMLDATA, and delegate to the existing resource cache. AE and sheet preparation enqueue image dependencies; native dependency ordering and asset fallback remain unverified. XML preparation returns true. This is a 15-name provisional source addition, not compiled coverage.

- Added six provisional grid/component names: temporary tick registration, duplicate suppression, completion removal and construction registration. Parent-grid propagation, lifetime ownership and native construction messages are not restored; caller must manage object lifetimes.

## Pass 47 battle display and placement

- Added five GfxBattleScreen adapters: accessory attachment, object visibility and mute, selected-raft focus and fleet opacity. Original 0x3f0e20 preserves visibility of the selected raft and children; 0x3f1adc recursively mutes; 0x3f1b8c updates selection and mute. Portable host has two views rather than one. Component opacity is not persistent across every snapshot update, and animated camera transition is approximate.
- Added three placement methods: original 0x572814 checks opposing boss cores and reverts the first ship, 0x572338 checks the first ship only, 0x5722e0 routes priority indices by piece count. Host graphics association is explicit; original scene construction still missing.
- Added ship preview constructor, destructor and tick forwarding (0x412e60); host renders RaftInfo snapshots without original menu scene assembly. Eleven new names provisionally; no compiled or runtime measurement.

- Cannon accessory: original ID 15, two nonlooping front/back compositions, core/raft selection and completion removal (0x5ffe7c, 0x5fff3c); original asset xml_bin/raft_cannons_anim.bin. Wired into OceanView::PlayRaftAttackAnims. Native per-cannon offsets, layer ordering and packet triggers remain incomplete.
- Movement arrow: inverse zoom, centered alignment and -34 offset from 0x5e70bc; portable graphics must be configured by caller. Battle points: override -1 uses current BP, zoom opacity 0.3–0.4, inverse zoom capped at 8, return-only SetColour (0x57bdcc/0x57c5bc); caller provides font and scene placement. Twelve additional provisional names; exact original layout and UI wiring not claimed.

- Ten further names: BattleDice copy/destruction (0x382740/0x384f04), ReadoutLight zero initialization/ref destruction (0x5a8c24/0x5a90a0), MsgStartDialog constructor argument order and owned message data (0x342720/0x33a670), BlueprintInfo three strings (0x36bdf4), SpriteBalloon derived SpritePiece/string cleanup (0x5f0fe0), QuestTypeInfo three strings (0x497fb8), DivingMaterial centered sprite at priority 50 (0x529ff8). These are portable lifetime/data adapters. Dialog dispatch, dice/readout/balloon scene assembly and automatic atlas acquisition are not restored; the enclosing readout and perimeter classes remain incomplete.

## Pass 48 object lifecycle and localization

- Original APK ARMv7 ELF SHA-256 29544f0b70c947e23b05c01016a88b94b90e28fd204ba956daea16f6fdd26c13. Read with readelf/c++filt and llvm-objdump --demangle.
- GridObject AddedToOcean (0x41e118) sends event 10; host observer replaces global dispatcher. Salvage (0x41dc18) removes from parent grid; portable occupancy removal only. setGridObjectData (0x41e18c) resolves a named resource; host stores a shared snapshot.
- Barracks 0x539ab8 calls base and registers itself when Constructing; overload now does that. Existing by-value component storage still prevents native factory polymorphism.
- Wreckage 0x50f6e0 delegates piece count for wrapped components; 0x51124c registers OtherWreckage, EnemyParts, EnemyWreckage, Wreckage; 0x50fba4 removes wrapper, repairs original and reinserts at original position. Host returns a repaired owned snapshot for the caller to replace, not original grid ownership. 0x50f71c delegates salvage then removes wrapper; 0x510a1c returns wrapped actions or empty. No server-authoritative state is fabricated.
- RPMenuHandler 0x4d5798 loads TUT_CHRISTEN_D (0xa636e8), finds first two percent delimiters, replaces with player name, sends localized message. Portable formatter returns the text; native message hookup still pending.
- Ten new provisional names, not verified build/runtime coverage.

- BattleShipPlacementSprite 0x584bd8 and 0x584c14 check base then attached child; 0x584d00 routes flattened priority by base piece count, 0x58506c propagates scale. Host resolves retained OceanView sprites by raft/component IDs (no stored dangling sprite pointers), flattens graphics pieces and binds actual battle views. Existing Pieces() still denotes raft snapshots for public indexing.
- ShipPlacementSprite 0x61949c traverses owned sprites, 0x619574 counts pieces, 0x619650 delegates priority by piece count, 0x619764 tests occupied position then child validity, 0x6193f4 resets to object. Portable copied raft/child preview uses catalogue footprints, checked grid translation, original-object reset and explicit graphics submission. Original native Ocean membership, drag flags and child validation callbacks not yet equivalent. Fifteen provisional names added.

## Remaining method names by class (provisional source review)

581 names from the Pass 33 inventory now have named source implementations through Pass 48 and have been removed from the provisional list below. This is not a compiled coverage increase; signatures and complete behavior remain unverified. The historical compiled measurements in MATCHING_STATUS.md and the README are unchanged.

### `rp::BuildModeState` (20)

`ApplyFootprintToRaft`, `ChangeView`, `CleanupFootprints`, `CreateGridObject`, `HandleExtensionResponse`, `SendBonusMaterials`, `cancelNotificationForRaftComponent`, `gotMsgBuildModeToggleView`, `gotMsgDivingGameDone`, `gotMsgDivingGameDoneMoving`, `gotMsgDivingGameShow`, `gotMsgGridObjectEvent`, `gotMsgHighlightHudItem`, `gotMsgLoadingAnimFinished`, `gotMsgOfferWallReward`, `gotMsgToggleBlockHudItem`, `gotMsgTransitionDone`, `sendSelectedObjectRepairDataWithGold`, `setupNotificationForRaftComponent`, `tick`

### `rp::BattleModeState` (11)

`PlayBattleAnimations`, `SwapPopup`, `SwapStance`, `ToBattleView`, `ToPositioningView`, `gotMsgBattleInfoDropdown`, `gotMsgBattleInfoRequest`, `gotMsgHighlightHudItem`, `gotMsgQuestRead`, `gotMsgToggleBlockHudItem`, `gotMsgTransitionDone`

### `rp::PerimeterWallSprite` (10)

`AddBalloons`, `Init`, `PerimeterWallSprite`, `RemoveFromTick`, `SetPriorities`, `SetPriority`, `SetVisible`, `Update`, `tick`, `~PerimeterWallSprite`

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

