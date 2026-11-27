# Pass 45: behavioral evidence

Static original ARM32 library comparison; no emulator differential.

| Method / source | Original evidence | Reconstruction and limits |
|---|---|---|
| AccessorySprite::GetTimeString | 0x5628bc delegates to FormatTime_Sec2Str 0x49fe68; string 0xa61708 | Exact integer division and HH:MM:SS formatting |
| GridSprite::UpdateAnimations | 0x5cd068 adds slack to dt for each AE animation, then zeros slack | Corrected previous subtraction; actual OceanView still uses sampled animation clocks |
| GridSprite::ListenForAnimation | 0x5721f8 returns true | Matches base return |
| GridSprite::SetMute | 0x5cd0cc skips diving unit | Portable flag and accessory forwarding; not full audio backend |
| GridSprite::gotMsgAnimationStarted | 0x5cd014 calls a virtual function | Portable accessory notification only |
| Barracks::SetConstructionInfo | 0x5398d8 sets remaining float; at zero calls factory | Retains fractional timer and type; factory creation missing |
| Barracks::AddedToOcean | 0x539ab8 calls base and registers construction TempTick | Attachment flag only; remains in missing inventory |
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
