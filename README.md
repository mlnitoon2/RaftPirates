# Raft Pirates

An unofficial reconstruction of **Raft Pirates**, an older mobile game from Big Blue Bubble and 6waves.

The goal is to make the game playable again and preserve its original gameplay, artwork, menus and mechanics. The client is being rebuilt in C++ using [HydraLegacy](https://github.com/mlnitoon2/HydraLegacy), a reconstruction of the engine that powered the original game.

## About the game

Build and expand your raft, dive for materials and blueprints, explore the ocean, construct weapons and take part in pirate battles.

## Try the Windows build

[**Download WindowsRaftPirates.zip**](https://github.com/mlnitoon2/RaftPirates/releases/download/windows-autobuild/WindowsRaftPirates.zip)

Extract the ZIP and run `raftpirates_desktop.exe`. The download is updated after a successful main-branch Windows build. If the first autobuild has not finished, check the [Windows build page](https://github.com/mlnitoon2/RaftPirates/actions/workflows/windows.yml).

The ZIP includes the executable, runtime DLLs, certificate bundle, build details and all currently committed original assets. Signing status is recorded in BUILD.txt. Available original assets are included automatically in Windows packages. The asset collection is still incomplete, so additional files from your own original APK may be needed. A compatible game server is required for online gameplay.

[Full Windows instructions](docs/BUILDING.md)

## Current progress

The client includes reconstructed startup, login, world loading, raft construction, diving, map navigation, shop and combat systems. These are at different stages of development; this is not a finished or fully playable release.

[Latest builds](https://github.com/mlnitoon2/RaftPirates/actions)

## Building Raft Pirates

See the [build guide](docs/BUILDING.md) for Windows, Linux, Android and asset setup.

## About the project

This is an independent game preservation project, not an official release or the original source code. The client, assets and server compatibility are still being reconstructed. No reconstruction license has been selected.
