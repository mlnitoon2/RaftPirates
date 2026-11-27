#!/usr/bin/env python3
"""Package a Windows executable, its transitive DLL imports and runtime data."""
import argparse
import hashlib
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
    bundled_assets = (asset_source / "menus").is_dir() and (
        asset_source / "xml_resources/properties.xml").is_file()
    if bundled_assets:
        shutil.copytree(asset_source, staging / "assets", dirs_exist_ok=True)
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
        "Copy original APK asset contents into assets/ and launch raftpirates_desktop.exe.\n"
        "Compatible server configuration is in assets/xml_resources/properties.xml.\n" +
        ("Original assets are included. No account data is included.\n" if bundled_assets
         else "No original assets or account data are included.\n") +
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
    with zipfile.ZipFile(output, "w", compression=zipfile.ZIP_DEFLATED) as archive:
        archive.writestr("assets/", "")
        for p in sorted(staging.rglob("*")):
            if p.is_file():
                archive.write(p, p.relative_to(staging).as_posix())
    print(f"Packaged {len(copied)} DLLs in {output}")


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
