# Multiplayer and boss battle transport

`BattleModeState` now owns the reconstructed online path. The reconstruction-only `OnlineBattle` source files were removed. `Game::extension` forwards recognized packets, and the JNI send path uses the existing SmartFox Java SDK. `Ocean` keeps a separate opponent snapshot. Disconnect clears the online encounter and sonar state. Battle presentation loads the original encounter confirmation, positioning HUD, battle HUD, retreat confirmation and battle-over popup XML. The original action bindings are only partially reconstructed. No Offline Play or manual Connect option was reintroduced.

## Outgoing protocol evidence

The following string references were read from ARM32 instructions in the original Raft Pirates 2013 ELF (SHA-256 in `MATCHING_STATUS.md`). Addresses are reference instruction addresses, not implementation addresses in this port. SFS message controller/action are documented in the transport implementation.

| Command | Parameters | ARM32 references |
|---|---|---|
| `ping_sonar` | empty object | `0x4a7ce4` |
| `gs_encounter_response` | `attack` boolean | `0x4bb358`, `0x4bb394` |
| `gs_battle_position` | `attack_x`, `attack_y`, `sync_counter` integers | `0x389480`, `0x3894b0`, `0x3894e0`, `0x389510` |
| `gs_battle_select_raft` | `attack_raft`, `defend_raft`, `ai_selection`, `sync_counter` integers | `0x389204`, `0x389248`, `0x38928c`, `0x3892bc`, `0x3892ec` |
| `gs_battle_client_ready` | `confirm_swap_stance` boolean, `sync_counter` integer | `0x385b8c`, `0x385c4c`, `0x385c84` |
| `gs_battle_attack` | `attack_dice`, `sync_counter` integers | `0x383584`, `0x3835b8`, `0x3835e8` |
| `gs_battle_defend` | `defend_dice`, `sync_counter` integers | `0x3836b0`, `0x3836e4`, `0x383714` |
| `gs_battle_retreat` | `sync_counter` integer | `0x385940`, `0x3859a8` |
| `gs_battle_plunder` | empty object | `0x385dd4` |

## Incoming state

Recognized commands include `gs_encounter`, `gs_start_battle`, `gs_opponent`, `gs_battle_position`, `gs_battle_select_raft`, `gs_battle_client_ready`, `gs_battle_attack`, `gs_battle_defend`, `gs_swap_stance`, `gs_battle_opponent_time`, `gs_battle_over`, `gs_battle_retreat` and `gs_battle_plunder`. The reference `BattleModeState::HandleExtensionResponse` starts at `0x389b3c`. Opponent data is initialized from `player_object`, and the client retains authoritative battle result packets. It never simulates online damage or grants gold locally. The position and selection counter advancement are based on reference paths at `0x38a9b4` and `0x3887b4`. Older counters are ignored, newer unexpected counters mark the state desynchronized. Duplicate local sends are prevented while waiting.

## Pass 24: result application and boss handling

The original `gotMsgBattleScreenRaftsSelected` sends `gs_battle_position` with integer coordinates. `SendRaftSelection` selects intact rafts and accepts authoritative selected IDs. Battle attack responses carry `results`; `BattleModeState::tick` in the reference advances the counter near `0x38e800`. The host reconstruction applies results once to both selected rafts using `Raft::HandleBattleOutcome`, rejects ambiguous overlapping fleet-local IDs, advances the counter, and requires another ready acknowledgement. Stance changes use `gs_swap_stance` and `confirm_swap_stance`. The opponent is rendered separately with the existing ocean asset renderer.

`gs_opponent.is_boss` marks a boss encounter. The map's `boss_data` object (`0x44a934`) contains `user_id`, `display_name`, `battle_points`, `zone`, and `raft_info`; the client retains this record and shows the boss name and BP when its zone matches. Exact boss map position and encounter initiation are not yet recovered. No fake boss coordinates, AI, damage or loot are generated.

## Pass 25: battle presentation and map encounters

`BattlePlacementScreen`, `BattleShipPlacementSprite` and `GfxBattleScreen` implement selection, shared-world placement previews, adjacency and collision feedback, opponent and player fleet views, original atlas-backed BP meters, dice selection and retreat confirmation. `GridSprite` draws all pieces of a grid object rather than just the first asset. Visual priorities, camera motion and asset placement remain approximations. Changes to authoritative raft BP trigger a floating damage amount and the original explosion animation; no damage is computed locally for online battles.

The original `MapView::updateBoss` at `0x44a914` reads the boss record without coordinates, and `updateBossGfx` at `0x44759c` selects an unsafe-water square. `updateEnemyShips` at `0x44dbd4` reads sonar `infos`, including IDs, names, fleets, BP and nemesis flags. The reconstruction selects stable unsafe-water squares locally, avoiding player, treasure and other encounter markers. The original `RaftPirates::gotMsgMapMarkerMoved` at `0x4a3e9c` sends `gs_map_move` with `map_x`, `map_y`, boolean `safe`, integer `treasure_id`, integer `enemy_id`, and integer `usedSonar` (0/1). Selecting a reconstructed marker now sends its ID using that existing command. The original random placement distribution and distance thresholds are not reproduced exactly.

## Current limitations

The battle controls load original XML, sprites and font assets; only selected actions and literal initialization properties are bound. The complete original Lua scripts and layout behavior are not restored. Exact result ownership when raft IDs collide, synchronization ordering, selection after destruction, timing and animation, boss phases and reward eligibility are not completely recovered. The server must send authoritative outcomes and implement matching commands. No actual multiplayer server session or complete boss fight was tested.

`tests/multiplayer.cpp` exercises synthetic map/boss and opponent records, position requests, raft selection, ready gating, dice fields, server-authoritative damage, counter advancement, stance swaps, duplicate handling, retreat, decline and plunder. Host JNI smoke testing checks the login/SFS bridge, not live combat.
