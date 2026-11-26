# Local play and diving

The original APK supplies its Java activity and assets. After Set Sail, select OFFLINE PLAY. This creates a local test session, not a real saved account. Connect remains the private-server workflow. A sandbox session starts with materials, a core raft, a robot and a diving unit. The reconstructed HUD and temporary menu panels offer building, moving/removing components, repairing and using clocks, purchases, map travel and daring progression. The local adapter generates the same response object types as the game protocol. There is no persistent storage or original multiplayer combat. Repairs and clocks are free local test operations; online costs and results are server-authoritative. Sandbox purchase prices and salvage amounts are provisional.

The diving overlay uses the original atlas and diving-unit animation. Drag horizontally to move, collect gears and avoid mines. Finish and collect to send `dive_collect`. The timed movement, collision and scoring model is an approximation; it does not replace the server's authoritative reward logic. The local simulator grants a fixed salvage reward, independent of local score.

The Player, Raft, RaftComponent, Blueprint and Material classes parse the original packet fields. The class layout and signatures are not original C++ ABI. Player battle-point summation and raft dimensions were checked against the ARM32 code; other state helpers use reconstructed semantics.

Desktop verification: configure with `-DRAFT_ASSET_ROOT=/path/to/local/assets` to include the original HUD and diving tests. Use `ctest --test-dir build --output-on-failure`. The Java mock verifies the original exported names and connection flow, not Android runtime execution.

## Practice battle and discovery

Travel to pirate water in the offline session, then select PRACTICE BATTLE. Attack with the selected living raft; active weapons contribute battle points and the enemy retaliates after attacks. Wait for the cooldown between attacks. Victory offers a one-time 150 gold reward, defeat permits exiting. Enemy stats, damage and reward are local test mechanics, not recovered original balancing. No invented combat command is sent to a live server.

The travel panel lists the known Barnacle Bill discovery in the sandbox. Select it to enter a view using the original discovery animation, then return to the ocean. The location and return are supported; the original discovery quest sequence is not. The real-asset play test exercises both the practice battle and discovery.

Drag the main ocean area to pan the raft camera. A completed drag is consumed so that it does not also trigger construction. Camera and marker helpers do not add live combat support.
