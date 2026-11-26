# Local play and diving

The original APK supplies its Java activity and assets. After Set Sail, select OFFLINE PLAY. This creates a local test session, not a real saved account. Connect remains the private-server workflow. A sandbox session starts with materials, a core raft, a robot and a diving unit. The reconstructed HUD and temporary menu panels offer building, moving/removing components, purchases, map travel and daring progression. The local adapter generates the same response object types as the game protocol. There is no persistent storage or combat. Sandbox purchase prices and salvage amounts are provisional.

The diving overlay uses the original atlas and diving-unit animation. Drag horizontally to move, collect gears and avoid mines. Finish and collect to send `dive_collect`. The timed movement, collision and scoring model is an approximation; it does not replace the server's authoritative reward logic. The local simulator grants a fixed salvage reward, independent of local score.

The Player, Raft, RaftComponent, Blueprint and Material classes parse the original packet fields. The class layout and signatures are not original C++ ABI. Player battle-point summation and raft dimensions were checked against the ARM32 code; other state helpers use reconstructed semantics.

Desktop verification: configure with `-DRAFT_ASSET_ROOT=/path/to/local/assets` to include the original HUD and diving tests. Use `ctest --test-dir build --output-on-failure`. The Java mock verifies the original exported names and connection flow, not Android runtime execution.
