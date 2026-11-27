# Desktop build

Download the Windows ZIP from the latest successful [Windows desktop build](https://github.com/mlnitoon2/RaftPirates/actions/workflows/windows.yml). Extract it, copy the contents of an authorized original APK's assets directory into the provided assets folder, and launch raftpirates_desktop.exe. No Java, NDK, MSYS2 or compiler is required on the target Windows 10/11 x64 machine. The ZIP includes runtime libraries and a CA certificate bundle. The APK's properties.xml still controls SERVER_IP; point it to your compatible server. The original service may be unavailable.

The desktop client uses native HTTP auth, the Hydra SFSClient TCP subset, SDL2 input and SDL2_mixer MP3/WAV audio. Live server compatibility is not yet verified. The legacy auth protocol sends credentials in a query string; HTTPS is required except loopback or an explicit --allow-insecure-auth flag for your own test server. Do not reuse real account passwords for tests.

## Signing

The workflow signs and timestamps the executable with Authenticode when WINDOWS_SIGNING_PFX_BASE64 and WINDOWS_SIGNING_PFX_PASSWORD Actions secrets are configured for an authorized CA-issued code-signing PFX. An unsigned build is explicitly labeled unsigned. Hardware-backed certificates or cloud signing require a different setup. No self-signed certificate is installed, and signing cannot guarantee antivirus or SmartScreen acceptance. The package includes SHA256SUMS.txt. Never disable antivirus to run a flagged build; investigate the detection and verify the source revision.

## Build from source (optional)

MSYS2 UCRT64 dependencies: gcc, cmake, ninja, expat, libpng, zlib, freetype, SDL2, SDL2_mixer, asio, openssl, curl, nlohmann-json. On Linux install the corresponding development packages.

```sh
cmake -S . -B build -G Ninja -DCMAKE_BUILD_TYPE=Release -DRAFT_DESKTOP=ON -DHYDRA_FETCH_LUA=ON
cmake --build build --target raftpirates_desktop
./build/raftpirates_desktop assets_local
```

The reconstructed native TCP client uses SDK 1.7.5 protocol details but is not the official SDK. UDP, encrypted SFS sessions, reconnect and full room/user tracking are not implemented.
