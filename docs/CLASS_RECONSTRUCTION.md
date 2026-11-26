# RaftComponent

The class-specific reconstruction uses Raft Pirates ARM32 evidence:

| ELF address | Method | Observed behavior |
|---|---|---|
| `0x54b68c` | GetRemainingTime | Returns float stored at offset 0x28 |
| `0x54b694` | PostBuild | State equals 5 |
| `0x54b6a8` | Building | State equals 1 or PostBuild |
| `0x54b744` | GetBattlePoints | Returns zero during Building, otherwise base value |
| `0x54b76c` | GetTotalBattlePoints | Returns zero during Building, otherwise base total |
| `0x54b794` | TempTick | Decrements remaining time, clamps to zero; state 1 completion dispatches a native message |

The portable implementation has an integer wire-time field with fractional local time, and returns the completion condition without fabricating the original message or changing authoritative state. Clock acceleration, original message dispatch and base class ABI are not reconstructed. The Game model advances construction independently of its diving timer. Existing `Component` is an alias for `RaftComponent`.

## GridObject and GridObjectInfo

`GridObject` is now the shared user-instance base of Raft and RaftComponent, separate from the static `GridObjectInfo` catalogue. Native accessors at 0x41d830 (grid position), 0x41d844 (maximum battle points), 0x41d850 (destroyed iff state 2), 0x41dca0 (health = current / maximum), 0x41dcf4 (current BP), 0x41dd38 (state), 0x41dd74 (name), 0x41dda4 (type), 0x41ddd4 (static object ID), and 0x41e02c (bounded BP setter) establish behavior. Repair at 0x41e0e0 takes absolute BP, not an amount to add. The explicit catalogue argument avoids retaining raw pointers across library refreshes. This is a source-level equivalent, not the original C++ object layout.

`GridObjectInfo` (0x4358fc) and `GridObjectPiece` (0x431068) parse the original semicolon-separated footprint/asset data. The reconstruction bounds placement and uses `GridObject::Contains` for collision checks.

## Battle outcome and construction requests

`Raft::HandleBattleOutcome` at 0x55a260 reads nested `target_component` objects with integer `user_raft`, `user_raft_component_id`, `current_bp`, `state` and nested `target_raft` objects with `user_raft_id`, `current_bp`, `state`. The component ID and parent ID are compared before applying BP/state; BP is clamped to the known static maximum. The original wreckage and battle animation side effects are not reconstructed. The method is unit-tested but not wired to a guessed outer live combat envelope.

`BuildModeState::UseClockSelected` at 0x3c0808 sends `gs_use_clock` with integer `object_id`, `object_type`, `parent_id`; `repairSelected` at 0x3c0e84 sends `gs_build_repair` with integer `object_id`, `object_type`, optional `parent_id` for a component and integer `repair_all`. The port exposes component repair and clock controls, sending original packets; the local simulator acknowledges those actions with provisional test behavior. Selection is represented by IDs rather than original intrusive pointers. The full original build state and menus remain unrecovered.

## Java audio bridge

The original DEX contains `HydraGame.playMusic(String)`, `pauseMusic()`, `resumeMusic()`, `stopMusic()`, `playSound(String,float)` and the `SoundMan` implementation using Android MediaPlayer and AssetManager. The port calls them with bundled asset names and never swaps in an unrelated SDK. The Java mock verifies calls, including pause/resume and shutdown. Actual playback on Android remains untested.

## Menu, quest and map classes

`rp::Quest::initWithSFSObject` (ELF address `0x496dec`) reads `id` and `new` from one object and definition fields from a second; the portable handler accepts paired records or flat test fixtures. `quest_progress` uses `quest_dataid`, `quest_logid`, and `progress` (extension handler around `0x4b3b0c`). `gs_quest_read` sends `quest_id`. Live quest envelopes and reward flows remain unverified. `RPMenuHandler` uses typed arguments rather than the original native message ABI; it filters owned or unknown blueprint purchases and exposes recipe checks. `MapView::isPosOnLand` and `isPosInSafeWater` use the original TMX island and safe-water layers; map viewport and selection semantics are approximated. `SparseGrid` implements cell occupancy with a portable 64-bit coordinate key instead of the original overlap-tree implementation. The player edge-grid helper supports actual adjacent hull placement; original adjacency edge cases and network authority remain unverified.

## Ocean and encounter states

The original OceanView instructions (0x4705d0–0x470730) establish the isometric axes with 192 and 96 units and corresponding view conversion. The renderer and selection path share the reconstructed transform. BattleModeState implements a local practice loop using the recovered Raft::HandleBattleOutcome fields. DiscoveryModeState uses the original Barnacle Bill animation with the recovered discovery navigation state. Combat timing, enemy balance, local reward and discovery presentation are not instruction-equivalent original implementations.
