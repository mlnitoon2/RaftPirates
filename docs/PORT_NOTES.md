# Raft Pirates native bridge

This module links against adjacent HydraLegacy. The target `raftpirates` is intended to replace the old `libraftpirates.so` in the exact archived APK inspected. It is not a full game source recovery. Build via the repository `build_arm64.py`; the desktop test target is not Android-compatible.

## Original APK interface

`tools/jni_inventory.py` extracts the Java native descriptors from `classes.dex`. `evidence/jni_inventory.json` contains 94 declarations, and `evidence/jni_coverage.json` labels 26 implemented/partial callbacks and 68 unsupported stubs. All 91 original ELF JNI export names occur in the desktop build plus three Java declarations absent in the original ELF. The stubs log once when used. There is no binary C++ ABI compatibility with the old engine.

The original MyLib and HydraGame Java classes load the shared library and call initGL/init/tick/touch. Assets are read from the APK path passed to init, and extracted to the app cache. Original Java SmartFox `ClientServices` is used with the recovered `Login(username, session, zone, SFSObject)` ordering. Desktop generated mock Java classes verify signatures and argument handling, not Android activity/GL/network behavior.

Startup uses the original title and login XML. Authentication reads the sole endpoint from properties.xml, then uses the returned game host and session with the existing Java SmartFox SDK. The desktop sandbox is for regression testing only; the Android application has no diagnostic Connect or Offline Play controls. Android-specific services and native exports are in HydraLegacy. Billing, social and advertising callbacks remain incomplete. Current physical-device compatibility and server interoperability are unverified.

## Recovered protocol additions

`rp::RaftPirates::gotMsgMapMarkerMoved` (0x4a3e9c) builds `gs_map_move` with `map_x` and `map_y` Int, `safe` Bool, `treasure_id`/`enemy_id`/`usedSonar` Int. The coordinate conversion multiplies a normalized screen offset by 100. In `gs_player`, the handler at 0x4b22fc clamps each axis to [-95,+95] before converting to screen position. Earlier reconstructed world-pixel map positions were therefore not correct on the wire. The reconstructed preview maps this approximately onto the TMX display rectangle; the original camera transform has not been fully reproduced.

Grid object data_string now retains multiple piece records and combines occupied cells for placement validation. The original multi-piece sprite placement behavior is still incomplete. Other additions: timer countdowns, map action state, repair/completion state updates, discovery travel result handling. The discovery scene, combat and diving minigame have partial reconstructions. Not all new handlers have UI controls. See the recovered protocol notes above; original XML and scripts must be extracted locally.

## Tests

Linux C++ build and CTest; independent synthetic Java SFS fixture passed in the exact-signature mock JNI test with `-Xcheck:jni`. Both original-asset raft and map previews rendered. APK extraction produced 710 assets and five valid TMX files. Automated NDK builds are published by CI; current physical device behavior remains unverified. The actual original Android Java classes were not executed in these tests.
