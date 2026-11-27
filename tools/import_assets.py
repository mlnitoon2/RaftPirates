#!/usr/bin/env python3
"""Merge a complete original APK or partial original asset ZIP into the repository."""
import argparse
import hashlib
import json
from pathlib import Path, PurePosixPath
import stat
import zipfile


def merge(archive_path: Path, root: Path, inventory_path: Path) -> tuple[int, int]:
    inventory = {e["path"]: e for e in json.loads(inventory_path.read_text(encoding="utf-8"))}
    verified = []
    existing = 0
    names = set()
    with zipfile.ZipFile(archive_path) as archive:
        for item in archive.infolist():
            if item.is_dir() or not item.filename.startswith("assets/"):
                continue
            name = item.filename[len("assets/"):]
            path = PurePosixPath(name)
            if (not name or "\\" in name or ":" in name or path.is_absolute() or
                    any(p in ("", ".", "..") for p in name.split("/")) or
                    stat.S_ISLNK(item.external_attr >> 16)):
                raise ValueError(f"Unsafe path: {item.filename}")
            key = name.casefold()
            if key in names:
                raise ValueError(f"Duplicate asset: {name}")
            names.add(key)
            expected = inventory.get(name)
            if expected is None or item.file_size != expected["size"]:
                raise ValueError(f"Unexpected asset: {name}")
            data = archive.read(item)
            if hashlib.sha256(data).hexdigest() != expected["sha256"]:
                raise ValueError(f"Original hash mismatch: {name}")
            target = root.joinpath(*path.parts)
            if not target.resolve().is_relative_to(root.resolve()):
                raise ValueError(f"Asset escapes destination: {name}")
            if target.exists():
                if not target.is_file() or hashlib.sha256(target.read_bytes()).hexdigest() != expected["sha256"]:
                    raise ValueError(f"Refusing to replace a modified asset: {name}")
                existing += 1
            else:
                verified.append((target, data))
    if not names:
        raise ValueError("Archive has no assets/ files")
    # Validate the entire supplied batch before writing anything.
    created = []
    try:
        for target, data in verified:
            target.parent.mkdir(parents=True, exist_ok=True)
            with target.open("xb") as output:
                created.append(target)
                output.write(data)
    except Exception:
        for target in created:
            target.unlink(missing_ok=True)
        raise
    return len(created), existing


def main():
    repo = Path(__file__).resolve().parents[1]
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument("archive", type=Path)
    args = parser.parse_args()
    added, existing = merge(args.archive, repo / "assets", repo / "evidence/asset_inventory.json")
    print(f"Imported {added} original assets; {existing} already present and identical.")


if __name__ == "__main__":
    main()
