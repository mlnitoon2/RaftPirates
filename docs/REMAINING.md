# Compatibility and remaining work

The client is an independent C++ reconstruction. Original assets and Java classes are required from a compatible APK. A successful native build does not establish behavioral parity.

| Area | Remaining work / validation |
|---|---|
| Startup and account flow | Validate authentication, registration, saved sessions and reconnects on a compatible device and server. |
| Gameplay | Compare construction, repair, placement, travel, shop, quests and diving against original client traces. |
| Combat | Validate turn sequencing, boss transitions, timing, opponent snapshots, animation and reconnect behavior against server traces. |
| Event system | Connect remaining message handlers to live producers and consumers; verify original ownership and lifetime semantics. |
| UI and rendering | Compare menu transitions, hit tests, layers, particles, text, animation and screenshots to the original client. |
| Platform | Validate notifications, lifecycle, audio, SDK callbacks and unsupported social integrations. |
| Verification | Recompute compiled name/signature coverage, run host regressions, then test on Android with a compatible server. |

No remaining names were listed in the historical source-name inventory. That inventory includes partial adapters, so the work above remains open. See [coverage](COVERAGE.md), [message inventory](MESSAGE_INVENTORY.md) and [native behavior evidence](NATIVE_BEHAVIOR.md).

Detailed historical reconstruction notes and superseded reports remain available in the [archived source tree](https://github.com/mlnitoon2/RaftPirates/tree/1d6b3f82aff5b161488d03f0bc1a9f7fbffa5ef1/docs).

## Current device-reported issues

The latest Android feedback reports unresolved splash glow alignment, quest book content, material inventory, settings, auto-safe-water text, map HUD placement, blueprint selection stalls and diving gameplay. Native changes address these areas, but require another device test before closing them. Blueprint paging is an interim alternative to the original continuous scroll. Diving movement and rope composition are provisional until checked against the original game. The public host CI does not execute the original-assets tests unless `RAFT_ASSET_ROOT` is supplied.

Device feedback: splash, map and basic diving motion are now confirmed working; blueprint selection no longer freezes. Still verify quest data and display, construction placement/confirmation, automatic dive exit, enlarged recipe icons, safety close button and settings labels. A single quest icon may reflect the server's quest response rather than layout; do not synthesize missing quests.

The original game-specific Android Java callback inventory and JNI entry points are owned by RaftPirates. The shared engine exposes the reusable platform interfaces, presentation, JNI helpers and networking services. Original callback signatures are preserved; unsupported callbacks remain explicitly marked.
