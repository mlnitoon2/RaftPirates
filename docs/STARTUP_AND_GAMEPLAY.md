# Startup, authentication and game networking

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

The ARM32 `social::bbb::Auth::registerNewUser` routine at `0x1e1200` contains the registration query fragments `u`, `u2`, `p`, `p2`, `c=1`, `t=bbb`. The earlier email-auth trace and original response shape support `auth.php`, `u`, `p`, `ue`, `pe`, `t=bbb`, `g=2`, `lang=en`, `client_version=1.0.4`, `platform=android`, plus device metadata. This reconstruction omits unresolved device fields and requires `ok`, `bbbId`, `sessId` and `serverIp` for login success. Registration requires a success response, then asks the player to log in; server-side semantics are not yet verified. Password reset and Facebook authentication are unavailable. Credentials are not persisted or logged by the reconstruction. The runtime endpoint is selected from the APK property or an explicit build override.

## Runtime endpoint and startup gate

`assets/xml_resources/properties.xml` contains `<Property key="SERVER_IP" value="..."/>`. `sys::EngineConfig::LoadProperties` now reads the original `key` attribute; the game normalizes a bare hostname to HTTPS and appends `auth.php` to a base URL. Explicit HTTP and complete endpoint URLs are supported. `RAFT_AUTH_URL` is an optional override, not a requirement. Historical hostname availability is unverified. Legacy GET authentication is for authorized test accounts; do not downgrade HTTPS automatically.

The original `Java_com_sixwaves_raftpirates_MyLib_getAuthURL` at `0x1919b4` returns the website base literal at `0xa37660`, `https://raftpirates.6waves.com/`, rather than the configurable `auth.php` path. These two roles are now separate. The Java HTTP worker receives the resolved endpoint by value and has timeouts and redirects disabled. SmartFox connects to the returned game host on port 9933.

Cold initialization constructs the title and optional splash only. Set Sail constructs the login screen; the gameplay view is allocated only after a successful SmartFox login event and a complete `gs_player` snapshot. A surface reinitialization preserves an active session instead of restarting the flow. Failed connections clear the game view, player state, pending requests and session token. Login errors before Set Sail are ignored. Duplicate connection events cannot send a second login request. The host fixture verifies the XML endpoint with no compiled override and checks stale/duplicate callbacks. Physical Android execution and real server availability are unverified.

Desktop previews: `raftpirates_preview ASSET_ROOT OUTPUT.png --title`, `--login`, `--email`, `--registration`. Build with `-DRAFT_ASSET_ROOT=...` to run the original-asset menu regression. Build with an empty `RAFT_AUTH_URL` for `tests/java/smoke.py`, which changes the property in a temporary fixture APK.

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

## Transport and response recovery

Original `rp::RaftPirates::gotMsgOnConnection` (`0x4a446c`) creates `client_version`, `last_updated` and `language` login parameters and calls the `RaftPirates` zone. The reconstructed client version is 1.0.4; `last_updated` remains zero because persistent catalogue synchronization is not reconstructed. Original `checkKeepAlive` (`0x4a07b4`) compares elapsed time with the float constant 30.0 and calls `sfs::SFSClient::KeepAlive` (`0x6545d0`), which resolves Java `KeepAlive()V`. The JNI bridge now uses that SDK method instead of sending an invented extension command.

Early extension packets are queued until login succeeds and then processed in order. Malformed initial player snapshots cannot enter gameplay. Generic `gs_error` releases pending map/build/travel requests without applying optimistic changes; because the legacy error has no correlation ID, this is a conservative recovery policy rather than proof of exact original behavior. When an ocean response includes map coordinates, the server's values take precedence over the locally pending destination. Host session tests exercise these paths.
