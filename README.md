# Raft Pirates

Unofficial native source reconstruction for game preservation. Engine dependency: [HydraLegacy](https://github.com/mlnitoon2/HydraLegacy).

An experimental source reconstruction of parts of the 2013–2015 Hydra engine and Raft Pirates. The original Java/Dalvik classes and assets are supplied by your existing APK. This is **not** the recovered original source and **not yet a fully playable replacement** for the original native game.

## Native-only Android build

Clone both repositories next to each other (not nested):

```sh
git clone https://github.com/mlnitoon2/HydraLegacy.git
git clone https://github.com/mlnitoon2/RaftPirates.git
cd RaftPirates
```

The default engine checkout is `../HydraLegacy`. To choose another location pass `--hydra` to the Python build script or `-DHYDRA_LEGACY_DIR=/path/to/HydraLegacy` to CMake. The tested engine snapshot is `7adac3752475e4c0eb90df8b3bc7691ea5780447`.

The two repositories are independent CMake targets. The game links Hydra statically. The root build produces one shared library named `libraftpirates.so`. It does not build an APK, modify the original APK, create a Java frontend or sign anything.

Use Android NDK r28c (or compatible NDK) and CMake, not Visual Studio or Android Studio. On Windows, if the SDK is already at `C:\godot-android`, run:

```powershell
python build_arm64.py --sdk "C:\godot-android"
```

If that SDK has its command-line tools and Java installed, but lacks NDK/CMake, add `--install-tools`. This installs only those two SDK packages and accepts SDK licenses interactively. First native configure downloads the pinned Expat and libpng source packages. The NDK itself supplies Clang and zlib. The SDK tools need a JDK only for the SDK manager; the native compilation does not compile Java. No IDE is required.

Output on successful compilation:

```
out/lib/arm64-v8a/libraftpirates.so
```

The script checks ELF64, little-endian, ET_DYN and machine AArch64 before copying the output. It selects API 23, static libc++, and a 16 KiB maximum ELF page size. The resulting file is the intended replacement at `lib/arm64-v8a/libraftpirates.so` in the specific archived APK. **Do not copy the desktop test library into an APK.** Keep the original archive backed up. No signing steps are included.

The Android build itself has not been run: no Android NDK was available in the reconstruction environment. No working Android library is bundled. Older Android target-SDK restrictions, native library packaging, original Java SDK behavior on new Android, and GLES behavior remain possible blockers. This is not guaranteed to launch on an ARM64-only phone.

## What the experimental replacement does

- Matches all 94 native declarations in the exact archived APK, including 91 exports found in its original ELF. This is JNI name/signature coverage, not complete game implementation. 25 callbacks have implementations or partial behavior; 69 are unsupported compatibility stubs, visibly logged if called. `getAuthURL()` is an explicit placeholder. Original C++ object ABI is not reproduced.
- Uses `MyLib.init(String,int,int,float,float)` to extract APK assets to app-private cache. No new APK assets need to be added. The original APK supplies its existing Java `GLSurfaceView`, keyboard activity and SmartFox Java SDK.
- Presents a temporary native research interface via GLES1 with the original raft sprites and TMX map. A CPU renderer is used, not the complete original GPU engine.
- The Connect control asks for a private server host, a BBB ID, and a session token from your local test server. The Java SDK's `Login(user,session,zone,params)` is used. The official HTTP auth flow is not restored; no account credentials are saved, and no official server endpoint is used by the bridge. Port 9933 is currently fixed. Use test-only tokens, not live credentials. Other old Java SDK code bundled in the APK is outside the scope of the bridge.
- Incoming libraries/player packets, map movement, placement, basic blueprint purchases, dive start/collection, build completion, repair response state, and discovery response state are represented in C++ to varying degrees. The game model also reads shop item, currency and group libraries, retains dive depth and state, and records daring timer and pause fields. Store purchases by item ID are exposed in C++; they are not yet connected to the temporary UI. Not all actions have controls. Placement and purchase use temporary controls, not the original shop. Map rendering and safe-water testing are approximate; normalized map wire keys are verified. No actual player persistence.
- Original menus, full Lua API, diving minigame, combat, discovery scenes, music/audio playback, social platform functions, advertising/billing integrations, and many more native behaviors remain missing.

## Verify locally (desktop)

On Linux with CMake, Ninja, C++17, zlib/Expat/libpng development headers, and an optional JDK:

```sh
cmake -S . -B build -G Ninja -DCMAKE_BUILD_TYPE=Release -DHYDRA_DESKTOP_JNI=ON
cmake --build build --parallel
ctest --test-dir build --output-on-failure
python tests/java/smoke.py build/libraftpirates.so /path/to/original.apk
```

The smoke test uses generated Java test doubles with recovered signatures and synthetic SFS fixtures. It is not an Android/real-server test. If you only need the portable runtime, omit `HYDRA_DESKTOP_JNI` and the Java smoke test. Windows desktop builds can use MSYS2 (see [HydraLegacy build guide](https://github.com/mlnitoon2/HydraLegacy/blob/main/docs/BUILDING.md)).

Read the [engine README](https://github.com/mlnitoon2/HydraLegacy) and [game port notes](docs/PORT_NOTES.md). Original assets and native binaries are not distributed. Use the original local APK to test or extract. The two directories contain source and research reference data only.

## Project layout

- `include/rp/`, `src/`: reconstructed game state, protocol and research view.
- `android/`: original-APK JNI compatibility bridge.
- `tests/`: C++ protocol and mock Java integration tests.
- `tools/`: JNI descriptor inventory and stub generator.
- `evidence/`: recovered JNI descriptors and test reports.

No license for the new reconstruction code has been selected. Original game and engine belong to their respective rights holders; this project is unofficial. Do not commit original APKs, proprietary scripts, credentials or private server data.
