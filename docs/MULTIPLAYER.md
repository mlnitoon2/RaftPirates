# Pass 23: multiplayer battle transport

`OnlineBattle` handles incoming extension commands independently of the desktop-only practice battle. `Game::extension` forwards recognized packets, and the JNI send path uses the existing SmartFox Java SDK. `Ocean` keeps a separate opponent snapshot. Disconnect clears the online encounter and sonar state. The temporary online battle controls are not the original battle layout. No Offline Play or manual Connect option was reintroduced.

## Outgoing protocol evidence

The following string references were read from ARM32 instructions in the original Raft Pirates 2013 ELF (SHA-256 in `MATCHING_STATUS.md`). Addresses are reference instruction addresses, not implementation addresses in this port. SFS message controller/action are unchanged from earlier passes.

| Command | Parameters | ARM32 references |
|---|---|---|
| `ping_sonar` | empty object | `0x4a7ce4` |
| `gs_encounter_response` | `attack` boolean | `0x4bb358`, `0x4bb394` |
| `gs_battle_select_raft` | `attack_raft`, `defend_raft`, `ai_selection`, `sync_counter` integers | `0x389204`, `0x389248`, `0x38928c`, `0x3892bc`, `0x3892ec` |
| `gs_battle_client_ready` | `confirm_swap_stance` boolean, `sync_counter` integer | `0x385b8c`, `0x385c4c`, `0x385c84` |
| `gs_battle_attack` | `attack_dice`, `sync_counter` integers | `0x383584`, `0x3835b8`, `0x3835e8` |
| `gs_battle_defend` | `defend_dice`, `sync_counter` integers | `0x3836b0`, `0x3836e4`, `0x383714` |
| `gs_battle_retreat` | `sync_counter` integer | `0x385940`, `0x3859a8` |
| `gs_battle_plunder` | empty object | `0x385dd4` |

## Incoming state

Recognized commands include `gs_encounter`, `gs_start_battle`, `gs_opponent`, `gs_battle_position`, `gs_battle_select_raft`, `gs_battle_client_ready`, `gs_battle_attack`, `gs_battle_defend`, `gs_swap_stance`, `gs_battle_opponent_time`, `gs_battle_over`, `gs_battle_retreat` and `gs_battle_plunder`. The reference `BattleModeState::HandleExtensionResponse` starts at `0x389b3c`. Opponent data is initialized from `player_object`, and the client retains authoritative battle result packets. It never simulates online damage or grants gold locally. The position and selection counter advancement are based on reference paths at `0x38a9b4` and `0x3887b4`. Older counters are ignored, newer unexpected counters mark the state desynchronized. Duplicate local sends are prevented while waiting.

## Current limitations

The state machine and GUI are provisional: the actual server sequencing, exact ready acknowledgement, encounter timing, outgoing positioning action, turn and stance progression, damage ownership, boss behavior, and plunder eligibility are not fully reconstructed. For example, result records may contain `target_raft`, `user_raft_id`, `target_component`, `user_raft_component_id`, `current_bp` and `state`, but ownership must be determined before applying them to a fleet. The UI currently displays the local raft and opponent name/BP; it is not a complete opponent ocean renderer. Opponent slot 1 is an internal view slot, not the opponent's network ID. No real two-client server session was run. The private server must implement corresponding commands and authoritative validation/persistence for online combat to work.

`tests/multiplayer.cpp` verifies sonar, acceptance, opponent initialization, raft selection, readiness, dice fields, duplicate guard, counter rejection, retreat, decline and plunder against synthetic fixtures. Host JNI smoke testing covers the existing login and SFS bridge, not a live multiplayer match.
