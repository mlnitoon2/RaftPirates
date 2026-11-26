# Pass 21 — original startup and authentication menu reconstruction

The original APK resources are used locally, not redistributed. The Android flow no longer offers diagnostic Connect, BBB ID/session entry or Offline Play. The desktop sandbox remains for regression tests.

| Stage | Original asset / evidence | Reconstruction |
|---|---|---|
| BBB splash | `menus/bbb_splash_menu.xml` | Existing splash |
| Set Sail title | `menus/title.xml` | Original button, version and BBB ID label |
| Loading curtains | `menus/loading_intro.xml`, `xml_bin/anim_loading_curtains.bin` | Intro, idle, idle 2 and outro compositions |
| Login method | `menus/login_menu.xml` | Original email/Facebook buttons; Facebook unavailable |
| Email login | `menus/login_email.xml` | Native keyboard, masked password, login and new user |
| Registration | `menus/login_registration.xml` | Matching password validation; request to configured server |
| Status | `menus/status_popup.xml` | Error and retry popup |
| Ocean/HUD | existing game/HUD implementation | Requires successful auth and game-player data |

The original loading XML checks `lastLoginType()` and may auto-authenticate remembered accounts. Pass 21 implements the first-time email path, not credential persistence or exact returning-player ordering. The existing title-before-login order is retained as a reconstruction decision, not verified original ordering. XML component literal assignments are interpreted with explicit game-event adapters rather than running all original SWIG bindings. The loader has a timeout fallback. Text rendering and layout are approximate.

The ARM32 `social::bbb::Auth::registerNewUser` routine at `0x1e1200` contains the registration query fragments `u`, `u2`, `p`, `p2`, `c=1`, `t=bbb`. The earlier email-auth trace and original response shape support `auth.php`, `u`, `p`, `ue`, `pe`, `t=bbb`, `g=2`, `lang=en`, `client_version=1.0.4`, `platform=android`, plus device metadata. This reconstruction omits unresolved device fields and requires `ok`, `bbbId`, `sessId` and `serverIp` for login success. Registration requires a success response, then asks the player to log in; server-side semantics are not yet verified. Password reset and Facebook authentication are unavailable. No real endpoint is hardcoded and no credentials are stored or logged by the reconstruction.

Use `python build_arm64.py --sdk "C:\godot-android" --auth-url "http://YOUR_TEST_SERVER:900/"`. HTTP compatibility may require the original manifest's cleartext setting. Use HTTPS and test-only accounts; legacy GET authentication is not a modern secure credential transport. The Java HTTP call runs on a worker thread with timeouts and redirect following disabled; SmartFox uses the returned host on port 9933. The game waits for player state before leaving the loading scene. A local mock HTTP/SmartFox handoff and original asset menus were tested; a live private server and Android ARM64 Pass 21 are not yet tested.

Desktop previews: `raftpirates_preview ASSET_ROOT OUTPUT.png --title`, `--login`, `--email`, `--registration`. Build with `-DRAFT_ASSET_ROOT=...` to run the original-asset menu regression. Configure `-DRAFT_AUTH_URL=http://127.0.0.1:19090/` for `tests/java/smoke.py`.

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


## Native session coordinator

`rp::RaftPirates` owns the portable game session, loading progress and keep-alive eligibility. JNI now routes connection, login, room and extension events through it. Network processing may be suspended explicitly; packets are queued in order (bounded to 1024) and discarded on connection loss. Pause stops game countdowns. This is a reconstruction using existing wire handlers; it does not establish original ABI equivalence. The Java integration test uses mock objects and a loopback auth service, not the official live service.
