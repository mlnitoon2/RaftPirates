# Building from source

Raft Pirates depends on [HydraLegacy](https://github.com/mlnitoon2/HydraLegacy). Clone the engine beside the game repository so CMake can locate it. Desktop platform dependencies are listed below.

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

The verified importer at `tools/import_assets.py` can merge files from an original APK or partial asset ZIP; see the asset instructions below.

Live server and device parity remain unverified.

## Desktop build and setup

Download [WindowsRaftPirates.zip](https://github.com/mlnitoon2/RaftPirates/releases/download/windows-autobuild/WindowsRaftPirates.zip). The main-branch Windows workflow publishes this stable release asset after a successful build and package verification; the same file is available in the [Windows desktop build](https://github.com/mlnitoon2/RaftPirates/actions/workflows/windows.yml) artifacts. Extract it. If the assets folder is incomplete, copy the contents of an authorized original APK's assets directory into it. Launch raftpirates_desktop.exe. If assets are committed at the repository root, CI includes them automatically and host CI enables the original-asset tests. No Java, NDK, MSYS2 or compiler is required on the target Windows 10/11 x64 machine. The ZIP includes runtime libraries and a CA certificate bundle. The APK's properties.xml still controls SERVER_IP; point it to your compatible server. The original service may be unavailable.

The desktop client uses native HTTP auth, the Hydra SFSClient TCP subset, SDL2 input and SDL2_mixer MP3/WAV audio. Live server compatibility is not yet verified. The legacy auth protocol sends credentials in a query string; Explicit HTTP URLs are supported for private IPv4 LAN addresses and loopback servers. HTTPS is required for public servers unless you deliberately pass --allow-insecure-auth for your own test server. For example, set SERVER_IP to http://192.168.1.207:900 in properties.xml; /auth.php is appended automatically. Do not reuse real account passwords for tests.

### Signing

The workflow signs and timestamps the executable with Authenticode when WINDOWS_SIGNING_PFX_BASE64 and WINDOWS_SIGNING_PFX_PASSWORD Actions secrets are configured for an authorized CA-issued code-signing PFX. An unsigned build is explicitly labeled unsigned in BUILD.txt; the download filename stays the same. Hardware-backed certificates or cloud signing require a different setup. No self-signed certificate is installed, and signing cannot guarantee antivirus or SmartScreen acceptance. The package includes SHA256SUMS.txt. Never disable antivirus to run a flagged build; investigate the detection and verify the source revision.

### Certificate setup

Only set these repository secrets if your certificate provider supplies an authorized exportable code-signing PFX. On Windows, convert the file to Base64 locally using PowerShell:

```powershell
[Convert]::ToBase64String([IO.File]::ReadAllBytes("C:\path\to\signing.pfx")) | Set-Clipboard
```

Go to repository Settings > Secrets and variables > Actions > New repository secret. Paste the Base64 value into WINDOWS_SIGNING_PFX_BASE64 and the PFX export password into WINDOWS_SIGNING_PFX_PASSWORD. Never commit these values or share them in chat. Clear the clipboard afterward. Leave both secrets unset if you do not have a certificate.

Most newly issued publicly trusted signing certificates use hardware-backed keys, so a downloadable PFX may not be available. For automated Windows signing, consider [Azure Artifact Signing](https://learn.microsoft.com/en-us/azure/artifact-signing/quickstart) or a provider's supported cloud integration instead. Those require verified publisher identity and separate CI integration. Signing is not a malware scan; a valid signature can still accompany a flagged file.

### Windows build compatibility

HydraLegacy commit 3485b2e9d08e7e3e34426c8caf48786710115cea fixes timer cancellation for newer standalone Asio packages (the old error_code overload was removed). The Windows workflow fetches HydraLegacy's default branch and records the exact revision in BUILD.txt. No local dependency installation is required for the portable package.

### Build from source (optional)

MSYS2 UCRT64 dependencies: gcc, cmake, ninja, expat, libpng, zlib, freetype, SDL2, SDL2_mixer, asio, openssl, curl, nlohmann-json. On Linux install the corresponding development packages.

```sh
cmake -S . -B build -G Ninja -DCMAKE_BUILD_TYPE=Release -DRAFT_DESKTOP=ON -DHYDRA_FETCH_LUA=ON
cmake --build build --target raftpirates_desktop
./build/raftpirates_desktop assets_local
```

The reconstructed native TCP client uses SDK 1.7.5 protocol details but is not the official SDK. UDP, encrypted SFS sessions, reconnect and full room/user tracking are not implemented.

### Asset bundle and graphics

When an authorized assets/ directory is committed at the repository root, Windows CI copies it into the portable ZIP automatically. The desktop host enables atlas-aware bilinear sprite sampling and linear framebuffer presentation without changing the game's 640x400 logical coordinate system. The viewport is letterboxed to the original 640:400 aspect ratio, including input coordinates. This smooths pixel edges but is not yet native high-resolution rendering. Desktop audio and asset failures are recorded in desktop.log in the game's save directory. The original @2x assets are not automatically selected.

Incremental asset imports are included in portable Windows builds even before the set is complete. BUILD.txt reports the included asset count and missing startup resources. Until the 710-file original asset set is present, merge the remaining assets from your authorized APK before launching. Host CI only enables original-asset tests when the expected file count and required startup markers are present; this is a completeness heuristic, not a hash audit.

## Local original assets

The repository's `assets/` directory contains a partial original asset collection. The desktop target copies this directory beside its executable on every build, and Windows CI checks the staged and packaged asset hashes against the original inventory. An incomplete collection is reported in BUILD.txt and may still require local files before the game can run. To prepare a complete local copy, use an original Raft Pirates APK you are authorized to use:

```powershell
python tools/prepare_assets.py "C:\path\to\RaftPirates.apk"
```

This extracts the actual files under `assets/` to `assets_local/`, preserving paths and adding a SHA-256 inventory for reproducibility. The directory is gitignored and the script refuses to overwrite an existing destination. The original-APK JNI bridge still extracts assets directly from its installed APK; the preparation script is not necessary for native-only builds. An APK packaging target would also need the existing Java classes/resources and the game's manifest. This script does not build or sign an APK. Only distribute assets with the relevant rights holder's permission.

### Verify an extracted directory

The repository includes the expected SHA-256 and size for all 710 asset files in `evidence/asset_inventory.json`. Run `python tools/verify_assets.py assets_local` (or specify any extracted asset directory) to check the exact archive used for the reconstruction. An asset-only ZIP with an `assets/` root can be unpacked alongside the repository; pass `assets` as the verification path. The original archive itself remains a local input.

### Merge a verified batch for Windows builds

Use an authorized local APK or a partial asset ZIP with an `assets/` root:

```powershell
python tools/import_assets.py "C:\path\to\RaftPirates-50-new-assets.zip"
git add assets
git commit -m "Import verified original assets"
git push
```

The importer verifies every included file against the archived inventory before writing, skips identical files, and refuses changed or unrecognized files. It does not commit or push automatically. The Windows build copies the committed files into its runtime assets directory.
