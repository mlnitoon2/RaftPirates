# Startup, authentication and game networking

The original APK resources are used locally, not redistributed. The Android flow no longer offers diagnostic Connect, BBB ID/session entry or Offline Play. The production demo and practice-battle simulation have been removed; tests use explicit packet fixtures.

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

The original loading XML checks `lastLoginType()` and may auto-authenticate remembered accounts. Startup now uses splash, loading/login, initial world and resource manifest completion, loading outro, then the Set Sail title. The title enters the existing session. XML component literal assignments are interpreted with explicit game-event adapters rather than running all original SWIG bindings. The loader has a timeout fallback. Text rendering and layout remain approximate.

The ARM32 `social::bbb::Auth::registerNewUser` routine at `0x1e1200` contains the registration query fragments `u`, `u2`, `p`, `p2`, `c=1`, `t=bbb`. The earlier email-auth trace and original response shape support `auth.php`, `u`, `p`, `ue`, `pe`, `t=bbb`, `g=2`, `lang=en`, `client_version=1.0.4`, `platform=android`, plus device metadata. This reconstruction omits unresolved device fields and requires `ok`, `bbbId`, `sessId` and `serverIp` for login success. Registration requires a success response, then asks the player to log in; server-side semantics are not yet verified. Password recovery opens `lost_password.php` in the platform browser. The service must provide the recovery page. Facebook authentication remains unavailable. Passwords are not persisted or logged. The runtime endpoint is selected solely from the APK property.

## Runtime endpoint and startup gate

`assets/xml_resources/properties.xml` contains `<Property key="SERVER_IP" value="..."/>`. `sys::EngineConfig::LoadProperties` now reads the original `key` attribute; the game normalizes a bare hostname to HTTPS and appends `auth.php` to a base URL. Explicit HTTP and complete endpoint URLs are supported. No compile-time endpoint override is supported. Historical hostname availability is unverified. Legacy GET authentication is for authorized test accounts; do not downgrade HTTPS automatically.

The original `Java_com_sixwaves_raftpirates_MyLib_getAuthURL` at `0x1919b4` returns the website base literal at `0xa37660`, `https://raftpirates.6waves.com/`, rather than the configurable `auth.php` path. These two roles are now separate. The Java HTTP worker receives the resolved endpoint by value and has timeouts and redirects disabled. SmartFox connects to the returned game host on port 9933.

Cold initialization constructs the splash and loading screen. Saved login resumes during loading; otherwise the authentication menu is displayed. Valid `gs_player` and `gs_ocean` packets after a login request and completion of the resource manifest are required before the title is presented. Set Sail enters the loaded gameplay without a second login. A surface reinitialization on the same Android host preserves the session; a new host resets to the splash and loading screen. Touches are cancelled on pause. Failed connections clear the game view, player state, pending requests and session token. Duplicate connection events cannot send a second login request. The host fixture intercepts Java URL connections in process and checks the actual startup ordering, stale callbacks and duplicate callbacks. Physical Android execution remains unverified.

Desktop previews: `raftpirates_preview ASSET_ROOT OUTPUT.png --title`, `--login`, `--email`, `--registration`. Build with `-DRAFT_ASSET_ROOT=...` to run the original-asset menu regression. The `tests/java/smoke.py` fixture uses the unchanged APK property and does not launch an HTTP server.

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

`rp::RaftPirates` owns the portable game session, loading progress and keep-alive eligibility. JNI now routes connection, login, room and extension events through it. Network processing may be suspended explicitly; packets are queued in order (bounded to 1024) and discarded on connection loss. Pause stops game countdowns. This is a reconstruction using existing wire handlers; it does not establish original ABI equivalence. The Java integration test uses mock objects and a socket-free URL connection double, not the official live service.

## Transport and response recovery

Original `rp::RaftPirates::gotMsgOnConnection` (`0x4a446c`) creates `client_version`, `last_updated` and `language` login parameters and calls the `RaftPirates` zone. The reconstructed client version is 1.0.4; `last_updated` is zero without a complete validated cache; otherwise it is the persisted library timestamp. Original `checkKeepAlive` (`0x4a07b4`) compares elapsed time with the float constant 30.0 and calls `sfs::SFSClient::KeepAlive` (`0x6545d0`), which resolves Java `KeepAlive()V`. The JNI bridge now uses that SDK method instead of sending an invented extension command.

Early extension packets are queued until the SDK or application-level world readiness condition is satisfied and then processed in order. Malformed initial player snapshots cannot enter gameplay. Generic `gs_error` releases pending map/build/travel requests without applying optimistic changes; because the legacy error has no correlation ID, this is a conservative recovery policy rather than proof of exact original behavior. When an ocean response includes map coordinates, the server's values take precedence over the locally pending destination. Host session tests exercise these paths.

## Loading diagnostics and persistent login

The game logs each extension command name without its payload, reports invalid player snapshots explicitly, and retains a rendered first gameplay frame before leaving the loading screen. A five-second wall-clock fallback ends the loading outro after successful initialization. `gs_ocean` alone is not a complete player snapshot; it cannot authorize or fabricate a fleet or inventory. A missing snapshot times out with a generic player-facing message and a detailed log entry. The portable application runtime stores only the issued session token, BBB ID, game host and endpoint in application-private storage; reuse depends on server validity. Legacy SaveData migration is not implemented.

Original `isCheatEnabled()` at 0x36bc88 is a two-instruction false return. The reconstructed client instead reads the optional boolean `admin` property (false by default). This client-only setting does not authorize server operations. `gotMsgOnAdminMessage` is reconstructed as bounded inbound server text, dispatched on the next game tick to the original host popup API. The JNI callback reads the verified `message` key (ARM32 string at 0xa40d90). This does not grant permissions or enable privileged cheat requests.

## Login callback recovery

The legacy SDK expects `rl` in a successful login response to be an array of room-record arrays, not a single flat record. Invalid room-list serialization can interrupt SDK event delivery. The APK DEX `ResLogin.handleResponse` calls `SystemController.populateRoomList(rl)` before `setMySelf`. `populateRoomList` calls `getSFSArray(i)` for every room. If `rl` is flat, the SDK can fail before establishing its own user even while subsequent extension messages arrive. The client normally uses the SDK login event or an SDK user check. For legacy preservation servers that send initial world data but do not produce a native login callback, application initialization also accepts both validated `gs_player` and `gs_ocean` responses after the current login request, provided `SmartFox.isConnected()` is true. It does not create or mutate a Java SDK user and does not confer server permissions. An explicit rejection or disconnect invalidates the attempt. The APK DEX `SmartFox.send` checks transport connectivity and `ExtensionRequest.validate` checks nonempty command and parameters; neither requires a non-null `getMySelf()` for an extension request. Correct server login serialization is still recommended. The regression fixture checks this path with no SDK user or login callback, including the disconnected transport guard. The actual Android device is not tested.

## Desktop application

The optional `raftpirates_desktop` target runs the same portable GameRuntime lifecycle as Android, with SDL2 presentation and text input. The desktop host now authenticates using native HTTP and connects through Hydra's native TCP SmartFox client. Actual server interoperability and full multiplayer remain unverified. DEX files and Java classes are not needed for this executable. See [Desktop](DESKTOP.md).

## Device-feedback fixes

The original diving atlas contains a 220x288 panel, not a full-screen opaque background. Its host now alpha-composites onto the ocean frame and renders remaining time. The original inventory anchors its material amounts 50 pixels from the bottom; the compatibility multi-row layout now bottom-aligns above the rarity captions. The blueprint purchase button reserves a separate price and currency icon region, matching TabbedBlueprintFrame.xml's resize script. The quest panel uses the original 288x384 atlas dimensions, independent unread/progress anchors and content-length scroll sizing. These changes still need original-asset device validation. Server purchase failures are shown rather than silently dismissed. An empty gs_quest result means the server supplied no quests.

The supplied development server sends extension responses with controller 1/action 12, and handles keep_alive as an extension command. HydraLegacy now accepts action 12 as well as 13, and the desktop host sends the game's keep_alive extension rather than substituting a generic SDK ping. The APK's revision-1.5 animation parser also identifies .psi layer names with particle-fire/puff source textures in numerous raft objects (including anim_barracks and anim_bazooka_buccaneer). The engine no longer draws the emitter texture as a static image. This suppresses emitter placeholders; it does not establish accurate particle emission or repair unrelated white pixels. Requires HydraLegacy bd0d2532cd995dee864cb379b9c31597875b6ea1 or newer.

## Follow-up device feedback

The Android publisher strips nonessential symbols from the distribution copy and validates JNI exports afterward. Loading outro and title render over the player's raft. Quest log intentionally reveals screenWidth()/4 per original XML; quest details use explicit foreground layers and centered bounds. Blueprint purchases unlock the plan; after success the catalogue reopens on it so Build is the next action. Material possession alone does not grant a blueprint. Countdown placement is provisional until original device/native geometry is verified.

## Map and quest regression pass

The full 288x384 quest backing is placed at the left screen edge, not at a negative X coordinate. The zone picker renders the original map icons (using the atlas keys rather than the server map filenames), has independent hit testing, and sends zone_goto. The bottom-right travel button and bottom-left sonar button have distinct sprite and touch rectangles. Unhandled map touches reach world-map movement instead of the generic full-screen overlay. The optional original-asset HUD test checks the full quest frame, zone picker, travel acknowledgement and water-tile movement. The reconstructed server must maintain the current zone and use it to gate blueprint purchases.

## Follow-up UI pass

Quest frame and buttons use distinct foreground layers; map choices and tabs are above map backing; safety popup uses bounded text and a full-size background. Zero sonar charges now report a missing server resource rather than opening a shop in the development client (original XML opens the store). Admin HUD visibility is forced for this development reconstruction; server-side permissions remain unchanged. F11 toggles borderless fullscreen on desktop. Android presentation already fills the supplied GL surface; the APK activity controls the surface itself. Revision 1.5 particle source textures are omitted from static animation rendering; this is not a complete particle simulation.
