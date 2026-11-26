# Local original assets

The original assets are not redistributed by this repository. To prepare a local copy for a future APK build or the desktop preview, use an original Raft Pirates APK you are authorized to use:

```powershell
python tools/prepare_assets.py "C:\path\to\RaftPirates.apk"
```

This extracts the actual files under `assets/` to `assets_local/`, preserving paths and adding a SHA-256 inventory for reproducibility. The directory is gitignored and the script refuses to overwrite an existing destination. The original-APK JNI bridge still extracts assets directly from its installed APK; the preparation script is not necessary for native-only builds. An APK packaging target would also need the existing Java classes/resources and the game's manifest. This script does not build or sign an APK. Only distribute assets with the relevant rights holder's permission.
