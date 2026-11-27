# Desktop build

The portable game code is independent of the APK's DEX. The Android build uses Java activity and SDK callbacks through JNI; the desktop build uses SDL2 and the same C++ runtime. Extract the original assets locally; no DEX conversion or Android emulator is needed to compile this target.

## Windows (MSYS2 UCRT64)

Install the dependencies from the UCRT64 shell:

```sh
pacman -S --needed mingw-w64-ucrt-x86_64-{gcc,cmake,ninja,expat,libpng,zlib,freetype,SDL2} python
```

Clone the authorized HydraLegacy and RaftPirates repositories next to one another. From RaftPirates:

```sh
python tools/prepare_assets.py "/c/path/to/RaftPirates.apk"
cmake -S . -B build-windows -G Ninja -DCMAKE_BUILD_TYPE=Release -DRAFT_DESKTOP=ON -DHYDRA_FETCH_LUA=ON
cmake --build build-windows --target raftpirates_desktop
./build-windows/raftpirates_desktop.exe assets_local
```

The desktop executable currently runs startup/loading and the original login menus; mouse and touch are forwarded to the game lifecycle. Text input uses the window title (passwords are masked), Enter confirms and Escape cancels or quits. Audio and live authentication/SmartFox connection are unavailable. No account session is saved or simulated. This is not yet a complete Windows game port. Use only compatible local assets. Keep the UCRT64 DLLs on PATH; standalone packaging is not supplied.

The older `raftpirates_preview` target still exports menu screenshots without SDL2. The Windows build uses C++17 and does not need JNI, a JDK, Android NDK or DEX. Android continues to use the official legacy Java SDK; the portable SFS codec is not a full client. Native socket/session handling, auth, audio, dialogs and Windows runtime tests are future work.
