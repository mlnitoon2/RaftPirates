# Local original assets

The repository's `assets/` directory contains a partial original asset collection. The desktop target copies this directory beside its executable on every build, and Windows CI checks the staged and packaged asset hashes against the original inventory. An incomplete collection is reported in BUILD.txt and may still require local files before the game can run. To prepare a complete local copy, use an original Raft Pirates APK you are authorized to use:

```powershell
python tools/prepare_assets.py "C:\path\to\RaftPirates.apk"
```

This extracts the actual files under `assets/` to `assets_local/`, preserving paths and adding a SHA-256 inventory for reproducibility. The directory is gitignored and the script refuses to overwrite an existing destination. The original-APK JNI bridge still extracts assets directly from its installed APK; the preparation script is not necessary for native-only builds. An APK packaging target would also need the existing Java classes/resources and the game's manifest. This script does not build or sign an APK. Only distribute assets with the relevant rights holder's permission.

## Verify an extracted directory

The repository includes the expected SHA-256 and size for all 710 asset files in `evidence/asset_inventory.json`. Run `python tools/verify_assets.py assets_local` (or specify any extracted asset directory) to check the exact archive used for the reconstruction. An asset-only ZIP with an `assets/` root can be unpacked alongside the repository; pass `assets` as the verification path. The original archive itself remains a local input.

## Merge a verified batch for Windows builds

Use an authorized local APK or a partial asset ZIP with an `assets/` root:

```powershell
python tools/import_assets.py "C:\path\to\RaftPirates-50-new-assets.zip"
git add assets
git commit -m "Import verified original assets"
git push
```

The importer verifies every included file against the archived inventory before writing, skips identical files, and refuses changed or unrecognized files. It does not commit or push automatically. The Windows build copies the committed files into its runtime assets directory.
