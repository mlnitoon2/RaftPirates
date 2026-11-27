# Raft Pirates

An unofficial C++ reconstruction of Raft Pirates for game preservation, built on [HydraLegacy](https://github.com/mlnitoon2/HydraLegacy). Java code is not distributed here. The root `assets/` directory is staged automatically next to the desktop executable and bundled in Windows CI artifacts; the available collection remains incomplete.

## Build

Clone HydraLegacy next to this repository. If the engine is private, use authorized access; CI uses the read-only `HYDRA_REPO_TOKEN` Actions secret.

### Android

Install Android SDK, NDK r28c, CMake 3.22.1 and Python 3.

```sh
python build_arm64.py --sdk /path/to/android-sdk
```

Output: `out/lib/arm64-v8a/libraftpirates.so`. The APK still needs its compatible Java classes and assets.

### Windows / Linux

```sh
python tools/prepare_assets.py /path/to/original.apk
cmake -S . -B build -G Ninja -DRAFT_DESKTOP=ON -DHYDRA_FETCH_LUA=ON
cmake --build build --target raftpirates_desktop
```

See [desktop instructions](docs/DESKTOP.md) for dependencies and launching. Download the packaged Windows ZIP from Actions for a dependency-free install. Native HTTP, SmartFox TCP and audio are implemented; live server parity remains unverified.

## Status

Android startup, authentication, world loading, construction, map, diving, shop and combat are implemented to varying degrees; device/gameplay parity is incomplete. The original endpoint is configurable via local assets' `xml_resources/properties.xml`.

[Coverage](docs/COVERAGE.md) · [Remaining work](docs/REMAINING.md) · [Networking](docs/STARTUP_AND_GAMEPLAY.md) · [Assets](docs/ASSETS.md) · [Tutorial](docs/TUTORIAL.md) · [Native evidence](docs/NATIVE_BEHAVIOR.md) · [CI](https://github.com/mlnitoon2/RaftPirates/actions)

Not the original source or a binary-equivalent replacement. No reconstruction license has been selected.
