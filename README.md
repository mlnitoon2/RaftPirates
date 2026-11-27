# Raft Pirates

An unofficial reconstruction of **Raft Pirates**, an older mobile game from Big Blue Bubble and 6waves.

The goal is to make the game playable again and preserve its original gameplay, artwork, menus and mechanics. The client is being rebuilt in C++ using [HydraLegacy](https://github.com/mlnitoon2/HydraLegacy), a reconstruction of the engine that powered the original game.

## About the game

Build and expand your raft, dive for materials and blueprints, explore the ocean, construct weapons and take part in pirate battles.

## Try the Windows build

1. Open the [Windows build page](https://github.com/mlnitoon2/RaftPirates/actions/workflows/windows.yml) and select a successful build.
2. Download the ZIP from its artifacts and extract it.
3. Run `raftpirates_desktop.exe`.

A GitHub account may be needed to download build artifacts. Available original assets are included automatically in Windows packages. The asset collection is still incomplete, so additional files from your own original APK may be needed. A compatible game server is required for online gameplay.

[Full Windows instructions](docs/DESKTOP.md)

## Current progress

The client includes reconstructed startup, login, world loading, raft construction, diving, map navigation, shop and combat systems. These are at different stages of development; this is not a finished or fully playable release.

[Latest builds](https://github.com/mlnitoon2/RaftPirates/actions) · [Coverage](docs/COVERAGE.md) · [Remaining work](docs/REMAINING.md)

## For developers

- [Desktop build instructions](docs/DESKTOP.md)
- [Android and source builds](docs/BUILDING.md)
- [Game startup and networking](docs/STARTUP_AND_GAMEPLAY.md)
- [Asset information](docs/ASSETS.md)
- [Tutorial reconstruction](docs/TUTORIAL.md)
- [Original game behavior](docs/NATIVE_BEHAVIOR.md)

## About the project

This is an independent game preservation project, not an official release or the original source code. The client, assets and server compatibility are still being reconstructed. No reconstruction license has been selected.
