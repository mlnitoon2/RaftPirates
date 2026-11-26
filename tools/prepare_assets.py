#!/usr/bin/env python3
"""Extract original APK assets locally, preserving their paths and a SHA-256 inventory."""

import argparse
import hashlib
import json
from pathlib import Path, PurePosixPath
import stat
import tempfile
import zipfile


def extract(apk: Path, destination: Path) -> int:
    destination = destination.resolve()
    if destination.exists():
        raise FileExistsError(f"Destination already exists: {destination}")
    destination.parent.mkdir(parents=True, exist_ok=True)
    with tempfile.TemporaryDirectory(dir=destination.parent) as temporary:
        staging = Path(temporary) / "assets"
        staging.mkdir()
        entries = []
        seen = set()
        with zipfile.ZipFile(apk) as archive:
            for info in archive.infolist():
                if info.is_dir() or not info.filename.startswith("assets/"):
                    continue
                relative = info.filename[len("assets/"):]
                path = PurePosixPath(relative)
                if (not relative or "\\" in relative or ":" in relative
                        or any(part in ("", ".", "..") for part in relative.split("/"))
                        or path.is_absolute() or stat.S_ISLNK(info.external_attr >> 16)):
                    raise ValueError(f"Unsafe asset path: {info.filename}")
                key = relative.casefold()
                if key in seen:
                    raise ValueError(f"Duplicate asset path: {relative}")
                seen.add(key)
                target = staging.joinpath(*path.parts)
                target.parent.mkdir(parents=True, exist_ok=True)
                digest = hashlib.sha256()
                with archive.open(info) as source, target.open("xb") as output:
                    while chunk := source.read(1024 * 1024):
                        digest.update(chunk)
                        output.write(chunk)
                entries.append({"path": relative, "size": info.file_size,
                                "sha256": digest.hexdigest()})
        if not entries:
            raise ValueError("No assets/ files in the input APK")
        if (staging / "asset_inventory.json").exists():
            raise ValueError("Asset inventory name conflicts with original asset")
        (staging / "asset_inventory.json").write_text(
            json.dumps(sorted(entries, key=lambda item: item["path"]), indent=2) + "\n",
            encoding="utf-8",
        )
        staging.rename(destination)
    return len(entries)


def main() -> None:
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument("apk", type=Path)
    parser.add_argument("--output", type=Path,
                        default=Path(__file__).resolve().parents[1] / "assets_local")
    args = parser.parse_args()
    count = extract(args.apk, args.output)
    print(f"Extracted {count} assets to {args.output}")


if __name__ == "__main__":
    main()
