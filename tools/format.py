#!/usr/bin/env python3
"""Format tracked C/C++ source directories with the repository style."""
import argparse
from pathlib import Path
import shutil
import subprocess


def main():
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument("--check", action="store_true", help="Check without modifying files")
    parser.add_argument("--clang-format", default="clang-format", help="Formatter executable")
    args = parser.parse_args()
    executable = shutil.which(args.clang_format)
    if executable is None:
        parser.error("Install clang-format 17 or newer, or specify its path with --clang-format.")
    root = Path(__file__).resolve().parents[1]
    suffixes = {".c", ".cc", ".cpp", ".h", ".hpp", ".inc"}
    folders = ("include", "src", "android", "tests", "examples", "tools", "sfs")
    tracked = subprocess.check_output(["git", "ls-files", "-z"], cwd=root).split(b"\0")
    files = sorted(root / p for raw in tracked if raw
                   for p in [Path(raw.decode("utf-8"))]
                   if p.parts[0] in folders and p.suffix in suffixes)
    options = ["--dry-run", "--Werror"] if args.check else ["-i"]
    for path in files:
        subprocess.run([executable, "--style=file", *options, str(path)], check=True)
    print(f"{'Checked' if args.check else 'Formatted'} {len(files)} C/C++ files.")


if __name__ == "__main__":
    main()
