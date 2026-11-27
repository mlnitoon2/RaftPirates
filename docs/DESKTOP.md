# Desktop build

Download the Windows ZIP from the latest successful [Windows desktop build](https://github.com/mlnitoon2/RaftPirates/actions/workflows/windows.yml). Extract it. If the assets folder is empty, copy the contents of an authorized original APK's assets directory into it. Launch raftpirates_desktop.exe. If assets are committed at the repository root, CI includes them automatically and host CI enables the original-asset tests. No Java, NDK, MSYS2 or compiler is required on the target Windows 10/11 x64 machine. The ZIP includes runtime libraries and a CA certificate bundle. The APK's properties.xml still controls SERVER_IP; point it to your compatible server. The original service may be unavailable.

The desktop client uses native HTTP auth, the Hydra SFSClient TCP subset, SDL2 input and SDL2_mixer MP3/WAV audio. Live server compatibility is not yet verified. The legacy auth protocol sends credentials in a query string; HTTPS is required except loopback or an explicit --allow-insecure-auth flag for your own test server. Do not reuse real account passwords for tests.

## Signing

The workflow signs and timestamps the executable with Authenticode when WINDOWS_SIGNING_PFX_BASE64 and WINDOWS_SIGNING_PFX_PASSWORD Actions secrets are configured for an authorized CA-issued code-signing PFX. An unsigned build is explicitly labeled unsigned. Hardware-backed certificates or cloud signing require a different setup. No self-signed certificate is installed, and signing cannot guarantee antivirus or SmartScreen acceptance. The package includes SHA256SUMS.txt. Never disable antivirus to run a flagged build; investigate the detection and verify the source revision.

### Certificate setup

Only set these repository secrets if your certificate provider supplies an authorized exportable code-signing PFX. On Windows, convert the file to Base64 locally using PowerShell:

```powershell
[Convert]::ToBase64String([IO.File]::ReadAllBytes("C:\path\to\signing.pfx")) | Set-Clipboard
```

Go to repository Settings > Secrets and variables > Actions > New repository secret. Paste the Base64 value into WINDOWS_SIGNING_PFX_BASE64 and the PFX export password into WINDOWS_SIGNING_PFX_PASSWORD. Never commit these values or share them in chat. Clear the clipboard afterward. Leave both secrets unset if you do not have a certificate.

Most newly issued publicly trusted signing certificates use hardware-backed keys, so a downloadable PFX may not be available. For automated Windows signing, consider [Azure Artifact Signing](https://learn.microsoft.com/en-us/azure/artifact-signing/quickstart) or a provider's supported cloud integration instead. Those require verified publisher identity and separate CI integration. Signing is not a malware scan; a valid signature can still accompany a flagged file.

### Windows build compatibility

HydraLegacy commit 3485b2e9d08e7e3e34426c8caf48786710115cea fixes timer cancellation for newer standalone Asio packages (the old error_code overload was removed). The Windows workflow fetches HydraLegacy's default branch and records the exact revision in BUILD.txt. No local dependency installation is required for the portable package.

## Build from source (optional)

MSYS2 UCRT64 dependencies: gcc, cmake, ninja, expat, libpng, zlib, freetype, SDL2, SDL2_mixer, asio, openssl, curl, nlohmann-json. On Linux install the corresponding development packages.

```sh
cmake -S . -B build -G Ninja -DCMAKE_BUILD_TYPE=Release -DRAFT_DESKTOP=ON -DHYDRA_FETCH_LUA=ON
cmake --build build --target raftpirates_desktop
./build/raftpirates_desktop assets_local
```

The reconstructed native TCP client uses SDK 1.7.5 protocol details but is not the official SDK. UDP, encrypted SFS sessions, reconnect and full room/user tracking are not implemented.

## Asset bundle and graphics

When an authorized assets/ directory is committed at the repository root, Windows CI copies it into the portable ZIP automatically. The desktop host enables atlas-aware bilinear sprite sampling and linear framebuffer presentation without changing the game's 640x400 logical coordinate system. This smooths pixel edges but is not yet native high-resolution rendering. The original @2x assets are not automatically selected.

Incremental asset imports are included in portable Windows builds even before the set is complete. BUILD.txt reports the included asset count and missing startup resources. Until the 710-file original asset set is present, merge the remaining assets from your authorized APK before launching. Host CI only enables original-asset tests when the expected file count and required startup markers are present; this is a completeness heuristic, not a hash audit.
