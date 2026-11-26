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
