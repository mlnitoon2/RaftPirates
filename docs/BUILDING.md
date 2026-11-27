# Building from source

Raft Pirates depends on [HydraLegacy](https://github.com/mlnitoon2/HydraLegacy). Clone the engine beside the game repository so CMake can locate it. See the [desktop guide](DESKTOP.md) for platform dependencies.

## Windows / Linux

```sh
python tools/prepare_assets.py /path/to/original.apk
cmake -S . -B build -G Ninja -DRAFT_DESKTOP=ON -DHYDRA_FETCH_LUA=ON
cmake --build build --target raftpirates_desktop
```

The Windows workflow includes the root assets directory in its packaged build. The original endpoint is configurable in the local assets' xml_resources/properties.xml. Use a compatible server.

## Android

Install the Android SDK, NDK r28c, CMake 3.22.1 and Python 3.

```sh
python build_arm64.py --sdk /path/to/android-sdk
```

Output: out/lib/arm64-v8a/libraftpirates.so. The APK still requires compatible Java classes and assets. Java code is not distributed here.

## Assets

See [Assets](ASSETS.md). The verified importer at tools/import_assets.py can merge files from an original APK or partial asset ZIP.

Live server and device parity remain unverified.
