# Startup screens and research gameplay controls

The original-APK JNI bridge now constructs the startup sequence using the original XML resources, rather than showing the research view immediately. No replacement Java Activity is needed.

- `rp::SplashScreen` loads `menus/bbb_splash_menu.xml` and the 480x320, 3.3667-second composition in `xml_bin/bbb_splash.bin`. It advances the original animation and transitions on completion or a tap. A four-second timeout is a reconstruction fallback. The original `gfx/black_bg` reference is absent from this APK; the background is cleared to black instead.
- `rp::TitleScreen` loads `menus/title.xml` and its `HudButton` template. It uses the original title image, `rp_menu.xml` atlas, `rp_font_title` font and English localization. Pressing and releasing Set Sail changes the button sprite and opens the research view. The version label explicitly identifies this as a preservation client. The native callbacks implement selected original event behavior rather than executing the full original Lua/SWIG interface; original title music is not yet implemented.

The research gameplay interface is distinct from the original HUD. Tap Menu to open Build, Shop, Inventory or Travel. Build selects an owned blueprint for placement; Select Part lets you pick an existing component on its raft grid and then move or remove it. Shop sends a purchase by store item name. Inventory displays server-provided material amounts. Travel sends a zone request. These controls do not create offline player data. Network actions require a connected private test server.

## Component movement protocol

ARM32 `rp::BuildModeState::gotMsgBuildConfirm` at `0x3c23e0` constructs the movement request using typed integer fields. PC-relative string references resolve as follows:

| Address of string-address calculation | Field |
|---|---|
| `0x3c24e0` | `object_id` |
| `0x3c254c` | `object_type` |
| `0x3c26e0` | `parent_id` |
| `0x3c25ac` | `pos_x` |
| `0x3c25e4` | `pos_y` |
| `0x3c2614` | `new_parent_id` |
| `0x3c2658` | `gs_build_update` |

The reconstructed game validates footprints against the 6x6 component grid, ignoring the component being moved. It waits for `gs_build_update` before committing position/parent changes. The supplied private server implementation sends an empty acknowledgement; this reconstruction accepts it, and also handles an explicit false/zero `success` as rejection. The rejection field is a compatibility convention, not a confirmed historical server field. This implementation supports component moves only, not moving whole rafts.

## Desktop preview

```sh
raftpirates_preview /path/to/extracted/assets splash.png --splash
raftpirates_preview /path/to/extracted/assets title.png --title
raftpirates_preview /path/to/extracted/assets menu.png --menu
```

The rendered screenshots were inspected using the supplied APK's extracted assets. Original asset redistribution is not required. The Java test doubles exercise splash skip, original title button press/release, research UI connection prompt, login, SFS fixture receipt and diving request. Android/ARM64 execution and a live server connection are still unverified.
