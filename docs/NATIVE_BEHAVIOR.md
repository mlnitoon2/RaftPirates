# Native behavior evidence

Historical static comparison against the original ARM32 library; implementation notes describe the revision reviewed, not necessarily the current code. No emulator differential.

| Method / source | Original evidence | Reconstruction and limits |
|---|---|---|
| AccessorySprite::GetTimeString | 0x5628bc delegates to FormatTime_Sec2Str 0x49fe68; string 0xa61708 | Exact integer division and HH:MM:SS formatting |
| GridSprite::UpdateAnimations | 0x5cd068 adds slack to dt for each AE animation, then zeros slack | Corrected previous subtraction; actual OceanView still uses sampled animation clocks |
| GridSprite::ListenForAnimation | 0x5721f8 returns true | Matches base return |
| GridSprite::SetMute | 0x5cd0cc skips diving unit | Portable flag and accessory forwarding; not full audio backend |
| GridSprite::gotMsgAnimationStarted | 0x5cd014 calls a virtual function | Portable accessory notification only |
| Barracks::SetConstructionInfo | 0x5398d8 sets remaining float; at zero calls factory | Retains fractional timer and type; factory creation missing |
| Barracks::AddedToOcean | 0x539ab8 calls base and registers construction TempTick | Attachment flag only; requires further runtime verification |
| PlacementSprite base Select and OkayToPlace | 0x5d763c and 0x5d7644 return false | Matches base return; actual item subclass validates through Game |
| ExplosionFXSprite | 0x5b4874 tests raft/boss; 0x5b48a0 returns 1.5 / 0.5 | Recovered predicates and durations; original random spawn schedule not restored |
| MapView::gotMsgHidePirateMap | 0x443240 tail calls hideMap | Direct adapter used in PortView |
| RPMenuHandler::gotMsgStoreDisplayStatus | 0x4d493c stores visibility and toggles cross-sell | Visibility flag only, no deprecated ads |
| BossCoreRaftsInMe | 0x584ea8 compares BossCoreRaft, skips destroyed, traverses native children | Catalogue-backed IDs, no native pointer graph |
| BossCoreRaftsByMe | 0x584f68 copies cached vector | Portable adjacency cache, no native pointer ABI |
| Original tabbed_menu_misc.xml | gotStoreItemInfo sets Layer=HUD, instantiates TabbedMenuItem and initializes it | Layer corrected; original Lua dispatch remains intercepted |

Coverage represents source names, not fully verified original behavior. The 47-name reduction includes partial adapters and reclassified existing methods. Do not interpret as 47 exact methods or 84% game completion.

## Pass 46 static checks

- 0x243174/0x243190/0x2431e4 return static identifiers AE/SS/XMLDATA. 0x243288 returns true. 0x2434b8, 0x24352c, 0x2435a0 call XML, atlas and AE creation respectively. 0x24f700 reads AE dependencies; 0x250320 reads the atlas image. Host preparation uses the existing parsers instead of duplicating the original binary reader.

- 0x4ee794 calls virtual TempTick and erases completed objects; 0x4ee8e8 suppresses duplicates and propagates to parent grids. 0x4ee864 walks parent links; portable flat membership is not equivalent. 0x54b8f4 registers building components and broadcasts a message; only registration is restored.

## Diving panel and collection lifecycle

Original ARM32 instructions at `0x51a284` store panel start and destination coordinates and transition duration; they do not move the robot. The show handler at `0x3c1668` uses HUD mode 2 and a one-second entrance, the exit handler at `0x3ac28c` initiates a one-second move, and `0x3c2398` submits bonus collection after the move. The reconstruction separates panel motion from robot motion and delays outgoing requests until the exit completes. Panel compositing and material pickup mechanics still require visual and gameplay comparison.

The material parser at `0x45d54c` uses `material_id` and `amt`; the dive response handler at `0x3c541c` reads `bonus_items`. The client now reads these server-provided rows, preserves valid collected quantities on same-unit updates and does not invent rewards.

## Engine revision and accessory timer verification

The original ARM32 AccessorySprite::GetTimeString(int) at 0x5628bc calls RaftPirateHelper::FormatTime_Sec2Str at 0x49fe68. The helper divides by 60 twice and formats the hours/minutes/seconds with the literal at 0xa61708, matching the existing %02d:%02d:%02d implementation. No change to its timer arithmetic is necessary. The original APK contains xml_bin/anim_build_complete.bin, with red/green/blue/yellow intro, idle, float and pop compositions. GridSprite and PerimeterWallSprite now request AEFormat::Rev1_5, the engine's revision-based spelling for the original Raft Pirates layout. This corrects the desktop and Android compile errors from the removed game-specific enum. Animation transition behavior still requires device verification.
