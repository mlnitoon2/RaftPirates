#!/usr/bin/env python3
"""Native-only NDK build. Does not modify an APK, package Java code, or sign anything."""

import argparse
import os
import pathlib
import shutil
import struct
import subprocess
import sys

ROOT = pathlib.Path(__file__).resolve().parent
WIN = os.name == "nt"
DEFAULT_NDK = "28.2.13676358"
DEFAULT_CMAKE = "3.22.1"


def run(*args):
    args = [str(x) for x in args]
    print("+", subprocess.list2cmdline(args), flush=True)

    if WIN and args[0].lower().endswith((".bat", ".cmd")):
        args = ["cmd.exe", "/d", "/c", subprocess.list2cmdline(args)]

    subprocess.run(args, check=True)


def binary(path):
    path = pathlib.Path(str(path) + (".exe" if WIN else ""))
    if not path.is_file():
        raise FileNotFoundError(path)
    return path


def main():
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument(
        "--sdk",
        type=pathlib.Path,
        default=os.environ.get("ANDROID_HOME") or os.environ.get("ANDROID_SDK_ROOT"),
    )
    parser.add_argument("--ndk", type=pathlib.Path, help="Existing NDK root (optional)")
    parser.add_argument("--ndk-version", default=DEFAULT_NDK)
    parser.add_argument("--cmake-version", default=DEFAULT_CMAKE)
    parser.add_argument(
        "--install-tools",
        action="store_true",
        help="Use existing sdkmanager to install NDK and CMake only",
    )
    parser.add_argument(
        "--hydra",
        type=pathlib.Path,
        default=ROOT.parent / "HydraLegacy",
        help="HydraLegacy source directory",
    )
    parser.add_argument("--jobs", type=int, default=min(os.cpu_count() or 2, 4))
    args = parser.parse_args()

    if not args.sdk:
        raise RuntimeError("Provide --sdk or set ANDROID_HOME.")

    sdk = args.sdk.resolve()
    ndk = (args.ndk or sdk / "ndk" / args.ndk_version).resolve()
    cmake_dir = sdk / "cmake" / args.cmake_version / "bin"

    if args.install_tools:
        manager = sdk / "cmdline-tools/latest/bin" / ("sdkmanager.bat" if WIN else "sdkmanager")
        if not manager.is_file():
            raise FileNotFoundError(manager)

        run(manager, "--sdk_root=" + str(sdk), "--licenses")
        run(
            manager,
            "--sdk_root=" + str(sdk),
            "ndk;" + args.ndk_version,
            "cmake;" + args.cmake_version,
        )

    toolchain = ndk / "build/cmake/android.toolchain.cmake"
    if not toolchain.is_file():
        raise RuntimeError(
            "NDK not found. Use --install-tools with existing SDK command-line tools or --ndk."
        )

    cmake = binary(cmake_dir / "cmake")
    ninja = binary(cmake_dir / "ninja")
    build = ROOT / "build-android-arm64"
    output = ROOT / "out/lib/arm64-v8a/libraftpirates.so"

    run(
        cmake,
        "-S", ROOT,
        "-B", build,
        "-G", "Ninja",
        "-DCMAKE_MAKE_PROGRAM=" + str(ninja),
        "-DCMAKE_TOOLCHAIN_FILE=" + str(toolchain),
        "-DHYDRA_LEGACY_DIR=" + str(args.hydra.resolve()),
        "-DANDROID_ABI=arm64-v8a",
        "-DANDROID_PLATFORM=android-23",
        "-DANDROID_STL=c++_static",
        "-DCMAKE_BUILD_TYPE=Release",
        "-DHYDRA_FETCH_DEPS=ON",
        "-DHYDRA_TOOLS=OFF",
        "-DBUILD_TESTING=OFF",
    )
    run(cmake, "--build", build, "--target", "raftpirates", "--parallel", str(max(args.jobs, 1)))

    library = build / "libraftpirates.so"
    data = library.read_bytes()
    if (
        data[:6] != b"\x7fELF\x02\x01"
        or struct.unpack_from("<H", data, 16)[0] != 3
        or struct.unpack_from("<H", data, 18)[0] != 183
    ):
        raise RuntimeError("Build did not produce an AArch64 ELF64 shared object.")

    output.parent.mkdir(parents=True, exist_ok=True)
    shutil.copy2(library, output)
    print(
        "ARM64 shared library:",
        output,
        "\nExperimental original-APK bridge. Full game compatibility is not established.",
    )


if __name__ == "__main__":
    try:
        main()
    except (OSError, RuntimeError, subprocess.CalledProcessError) as error:
        sys.exit("Build failed: " + str(error))
