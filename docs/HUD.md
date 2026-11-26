# Original normal HUD adapter

`rp::Hud` loads `menus/hud.xml` and expands its templates from the caller's local APK assets. It uses original atlas entries from `xml_resources/rp_menu.xml`, bitmap font `rp_font_title`, and the English localization table. No assets are distributed here.

The original HUD scripts call game-specific Lua/SWIG functions that have not been reconstructed. The adapter binds selected lifecycle events and touch callbacks directly to C++ instead of silently executing incomplete scripts. The 640×400 research viewport uses explicit button offsets so hit areas remain on screen. Unimplemented tutorial, object-selection, diving, cheat and instruction branches are hidden. The full original script state machine and animations are not yet supported.

| Original element | Native adapter behavior |
|---|---|
| QuestMenu | Sends `gs_quest`; opens a temporary quest-count panel |
| MainMenu | Opens the temporary shop/build/inventory/travel UI |
| PirateMap | Opens the map view |
| EventNotification | Toggles construction view |
| SafeFlag / BadZoneFlag | Displays the ocean safety status and opens an information panel |
| DaringMeter | Shows level, timer, progress, chest and lock; sends `daring_advance` only when locally eligible |

The quest badge is a proxy for a nonempty quest list, not the original unread/new-quest state. The countdown and progress are based on the existing server timer model. The server remains authoritative; the client does not increment the level itself. Daring is locked in safe water, bad zones, paused states or at a positive zone limit. The original script's complete percentage smoothing, liquid-courage and tutorial behaviors remain unimplemented.

The independent Zone and Discovery classes read the original wire fields. Native references: `Zone::initWithSFSObject` at `0x5118f8`, `Zone::id` at `0x511f9c`, `Zone::daringLimit` at `0x511a24`; `Discovery::initWithSFSObject` at `0x3d3674`, `id` at `0x3d36d8`, `questgroupid` at `0x3d385c`, `zoneid` at `0x3d3a10`. The old name/map lookup tables are kept for existing views. The original discovery scene is not implemented by these metadata classes.

## Local tests

Configure the combined desktop build with an extracted original asset directory (containing `menus`, `menu_templates`, `gfx`, etc.):

```sh
cmake -S . -B build -G Ninja -DHYDRA_LUA=OFF -DRAFT_ASSET_ROOT=/path/to/assets
cmake --build build --parallel
ctest --test-dir build --output-on-failure
```

The optional `raftpirates_original_hud` test loads the real menu files and verifies all supported HUD actions, including the resulting SFS commands. The JNI smoke test covers the startup-to-login transition, the HUD quest request and a diving request with mock Java callbacks. The native Android runtime and a real private server have not been tested.

To render the title-screen preview:

```sh
./build/raftpirates_preview /path/to/assets preview.png
```
