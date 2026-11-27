#!/usr/bin/env python3
"""Package a Windows executable, its transitive DLL imports and runtime data."""
import argparse
import hashlib
import json
import os
from pathlib import Path
import re
import shutil
import subprocess
import zipfile


def imports(binary):
    result = subprocess.run(["objdump", "-p", str(binary)], check=True,
                            capture_output=True, text=True)
    return re.findall(r"DLL Name:\s*([^\s]+)", result.stdout, re.IGNORECASE)


def package(exe, prefix, output, status, revision, hydra_revision):
    staging = output.parent / "portable"
    if staging.exists():
        shutil.rmtree(staging)
    staging.mkdir(parents=True)
    (staging / "assets").mkdir()
    asset_source = Path(__file__).resolve().parents[1] / "assets"
    asset_files = list(p for p in asset_source.rglob("*") if p.is_file())
    # The CMake target stages runtime data even on an assets-only rebuild.
    # Verify every repository asset is present and identical in the staged build.
    build_assets = exe.parent / "assets"
    inventory_path = asset_source.parent / "evidence" / "asset_inventory.json"
    inventory = {entry["path"]: entry for entry in
                 json.loads(inventory_path.read_text(encoding="utf-8"))}
    verified = {}
    for source in asset_files:
        relative = source.relative_to(asset_source).as_posix()
        actual = hashlib.sha256(source.read_bytes()).hexdigest()
        expected = inventory.get(relative)
        if expected is None or source.stat().st_size != expected["size"] or actual != expected["sha256"]:
            raise RuntimeError(f"Asset does not match original inventory: {relative}")
        compiled = build_assets / relative
        if not compiled.is_file() or hashlib.sha256(compiled.read_bytes()).hexdigest() != actual:
            raise RuntimeError(f"Missing or mismatched staged build asset: {relative}")
        destination = staging / "assets" / relative
        destination.parent.mkdir(parents=True, exist_ok=True)
        shutil.copy2(compiled, destination)
        verified[relative] = actual
    required_assets = (
        "menus/loading_intro.xml", "menus/hud.xml", "menus/title.xml",
        "xml_resources/properties.xml", "xml_resources/rp_menu.xml",
        "xml_bin/game_manifest.bin", "gfx/menu/menu_overlay.pvr",
        "font/rp_font_title.font", "font/rp_font_title.alphabet",
        "gfx/font/rp_font_title.png", "text/en.utf8",
    )
    missing = [path for path in required_assets if not (asset_source / path).is_file()]
    bundled_assets = not missing and len(verified) == len(inventory)
    asset_status = (
        f"Included {len(asset_files)} of {len(inventory)} original asset files.\n"
        + ("Original asset inventory complete.\n" if bundled_assets else
           "Incomplete asset bundle. Merge the remaining original APK assets before launching.\n")
        + ("Missing startup assets: " + ", ".join(missing) + "\n" if missing else "")
    )
    shutil.copy2(exe, staging / exe.name)

    bin_dir = prefix / "bin"
    available = {p.name.lower(): p for p in bin_dir.glob("*.dll")}
    system = Path(os.environ["SystemRoot"]) / "System32"
    system_names = {p.name.lower() for p in system.glob("*.dll")}
    queue = [staging / exe.name]
    copied = set()
    # SDL_mixer can dynamically load the MP3 backend rather than importing it.
    for name in ("libmpg123-0.dll",):
        if name in available:
            destination = staging / available[name].name
            shutil.copy2(available[name], destination)
            queue.append(destination)
            copied.add(name)

    while queue:
        binary = queue.pop()
        for name in imports(binary):
            key = name.lower()
            if key in copied or key.startswith(("api-ms-win-", "ext-ms-win-")):
                continue
            if key in available:
                src = available[key]
                dest = staging / src.name
                shutil.copy2(src, dest)
                queue.append(dest)
                copied.add(key)
            elif key in system_names:
                continue
            else:
                raise RuntimeError(f"Unresolved runtime DLL: {name} (imported by {binary.name})")

    ca_candidates = [prefix / "ssl/certs/ca-bundle.crt",
                     prefix / "etc/ssl/certs/ca-bundle.crt",
                     prefix / "etc/pki/ca-trust/extracted/pem/tls-ca-bundle.pem",
                     prefix / "bin/curl-ca-bundle.crt",
                     prefix / "share/curl/ca-bundle.crt"]
    ca = next((p for p in ca_candidates if p.is_file()), None)
    if ca is None:
        raise RuntimeError("MSYS2 CA certificate bundle not found")
    shutil.copy2(ca, staging / "ca-bundle.crt")
    license_dir = prefix / "share/licenses"
    if license_dir.is_dir():
        shutil.copytree(license_dir, staging / "licenses", dirs_exist_ok=True)

    (staging / "BUILD.txt").write_text(
        f"RaftPirates: {revision}\nHydraLegacy: {hydra_revision}\n"
        f"Authenticode: {status}\n"
        "The included assets/ directory is staged automatically at build time.\n"
        "If the bundle is incomplete, merge the remaining authorized original assets before launching.\n"
        "Compatible server configuration is in assets/xml_resources/properties.xml.\n" +
        asset_status +
        "If antivirus reports a detection, stop and investigate. Do not disable protection.\n",
        encoding="utf-8")
    digest_lines = []
    for p in sorted(staging.rglob("*")):
        if p.is_file():
            digest_lines.append(f"{hashlib.sha256(p.read_bytes()).hexdigest()}  "
                                f"{p.relative_to(staging).as_posix()}")
    (staging / "SHA256SUMS.txt").write_text("\n".join(digest_lines) + "\n", encoding="utf-8")

    # Verify Windows loads the executable with the packaged dependencies and without MSYS2 on PATH.
    env = os.environ.copy()
    env["PATH"] = str(system) + os.pathsep + os.environ["SystemRoot"]
    subprocess.run([str((staging / exe.name).resolve()), "--version"],
                   cwd=staging, env=env, check=True, timeout=30)
    # Publish only after verifying the complete portable package.
    temporary = output.with_suffix(".zip.tmp")
    with zipfile.ZipFile(temporary, "w", compression=zipfile.ZIP_DEFLATED) as archive:
        archive.writestr("assets/", "")
        for p in sorted(staging.rglob("*")):
            if p.is_file():
                archive.write(p, p.relative_to(staging).as_posix())
    with zipfile.ZipFile(temporary) as archive:
        if archive.testzip() is not None:
            raise RuntimeError("Corrupt ZIP member")
        expected_members = {p.relative_to(staging).as_posix() for p in staging.rglob("*") if p.is_file()}
        actual_members = [name for name in archive.namelist() if not name.endswith("/")]
        if set(actual_members) != expected_members or len(actual_members) != len(expected_members):
            raise RuntimeError("Portable ZIP contents differ from staging")
        for name in expected_members:
            if hashlib.sha256(archive.read(name)).hexdigest() != hashlib.sha256((staging / name).read_bytes()).hexdigest():
                raise RuntimeError(f"Packaged runtime file hash mismatch: {name}")
        packaged = {name[len("assets/"):] for name in archive.namelist()
                    if name.startswith("assets/") and not name.endswith("/")}
        if packaged != set(verified):
            raise RuntimeError("Packaged asset paths differ from the repository")
        for relative, digest in verified.items():
            if hashlib.sha256(archive.read("assets/" + relative)).hexdigest() != digest:
                raise RuntimeError(f"Packaged asset hash mismatch: {relative}")
    temporary.replace(output)
    print(f"Packaged {len(copied)} DLLs and {len(verified)} verified original assets in {output}")


def main():
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument("--exe", type=Path, required=True)
    parser.add_argument("--prefix", type=Path, required=True)
    parser.add_argument("--output", type=Path, required=True)
    parser.add_argument("--status", choices=("signed", "unsigned"), required=True)
    parser.add_argument("--revision", required=True)
    parser.add_argument("--hydra-revision", required=True)
    args = parser.parse_args()
    package(args.exe.resolve(), args.prefix.resolve(), args.output.resolve(),
            args.status, args.revision, args.hydra_revision)


if __name__ == "__main__":
    main()
