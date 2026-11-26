#!/usr/bin/env python3
"""Check a local extracted asset directory against the archived APK inventory."""

import argparse
import hashlib
import json
from pathlib import Path


def verify(root: Path, inventory: Path) -> bool:
    expected = json.loads(inventory.read_text(encoding="utf-8"))
    errors = []
    for entry in expected:
        path = root / entry["path"]
        if not path.is_file():
            errors.append(f"Missing: {entry['path']}")
            continue
        if path.stat().st_size != entry["size"]:
            errors.append(f"Size mismatch: {entry['path']}")
            continue
        digest = hashlib.sha256()
        with path.open("rb") as source:
            while True:
                chunk = source.read(1024 * 1024)
                if not chunk:
                    break
                digest.update(chunk)
        if digest.hexdigest() != entry["sha256"]:
            errors.append(f"SHA-256 mismatch: {entry['path']}")
    for error in errors:
        print(error)
    print(f"Verified {len(expected) - len(errors)}/{len(expected)} original assets")
    return not errors


def main() -> None:
    parser = argparse.ArgumentParser(description=__doc__)
    repository = Path(__file__).resolve().parents[1]
    parser.add_argument("root", type=Path, nargs="?", default=repository / "assets_local")
    parser.add_argument("--inventory", type=Path,
                        default=repository / "evidence" / "asset_inventory.json")
    args = parser.parse_args()
    raise SystemExit(0 if verify(args.root, args.inventory) else 1)


if __name__ == "__main__":
    main()
