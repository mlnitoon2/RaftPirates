#!/usr/bin/env python3
"""Generate a truthful per-revision verification dashboard; no original binary is redistributed."""
import argparse
import hashlib
import html
import json
import os
import re
import subprocess
from pathlib import Path

def names(binary, namespace, dynamic=False):
    data = subprocess.check_output(
        ["nm", *(["-D"] if dynamic else []), "--defined-only", "-C", str(binary)],
        text=True, errors="replace")
    pattern = re.compile(r"\b[ TtWw] (?:[^\s]+ )?(" + re.escape(namespace) + r"::[\w:~]+)\(")
    return {m.group(1) for line in data.splitlines() if (m := pattern.search(line))}

def main():
    p = argparse.ArgumentParser(description=__doc__)
    p.add_argument("--archive", type=Path, required=True)
    p.add_argument("--historical", type=Path, required=True)
    p.add_argument("--namespace", required=True)
    p.add_argument("--reference", type=Path, help="Optional locally supplied original ARM32 ELF")
    p.add_argument("--output", type=Path, required=True)
    p.add_argument("--tests", default="not recorded")
    args = p.parse_args()
    built = names(args.archive, args.namespace)
    historical = args.historical.read_text(encoding="utf-8")
    row = re.search(r"\| Raft Pirates 2013 \| (\d+)/(\d+) \|", historical)
    if not row:
        raise SystemExit("Historical baseline row missing")
    old_count, total = map(int, row.groups())
    subset = set(re.findall(r"^- \x60(" + re.escape(args.namespace) + r"::[\w:~]+)\x60$", historical, re.M))
    if len(subset) != old_count:
        raise SystemExit(f"Historical baseline list has {len(subset)} names; expected {old_count}")
    if args.reference:
        reference_sha = hashlib.sha256(args.reference.read_bytes()).hexdigest()
        baseline = names(args.reference, args.namespace, True)
        if not baseline:
            raise SystemExit("No original namespace symbols")
        total = len(baseline)
        matched = len(baseline & built)
        metric = "Full reference symbol-name overlap"
    else:
        reference_sha = "29544f0b70c947e23b05c01016a88b94b90e28fd204ba956daea16f6fdd26c13 (historical)"
        matched = len(subset & built)
        metric = "Verified lower bound from historical matched-name subset"
    repo = os.environ.get("GITHUB_REPOSITORY", "local")
    revision = os.environ.get("GITHUB_SHA", "local")
    data = dict(repository=repo, revision=revision, compiled_names=len(built),
                retained_historical_names=len(subset & built), historical_subset_size=old_count,
                known_matches=matched, original_total=total, metric=metric,
                reference_sha256=reference_sha, tests=args.tests,
                instruction_match=None, byte_match=None,
                build_sha256=hashlib.sha256(args.archive.read_bytes()).hexdigest())
    args.output.mkdir(parents=True, exist_ok=True)
    (args.output/"progress.json").write_text(json.dumps(data, indent=2)+"\n", encoding="utf-8")
    esc = html.escape
    rows = [("Compiled qualified names", len(built)),
            (metric, f"{matched}/{total} ({matched/total:.2%})"),
            ("Retained historical names", f"{len(subset & built)}/{old_count}"),
            ("Host tests", args.tests),
            ("Instruction equivalence", "Not measured"),
            ("Byte identity", "Not measured")]
    table = "".join(f"<tr><th>{esc(str(k))}</th><td>{esc(str(v))}</td></tr>" for k,v in rows)
    page = f"""<!doctype html><html lang="en"><meta charset="utf-8"><meta name="viewport" content="width=device-width,initial-scale=1"><title>{esc(repo)} verification</title>
<style>body{{font:16px system-ui;background:#111827;color:#e5e7eb;max-width:850px;margin:48px auto;padding:0 20px}}a{{color:#93c5fd}}table{{border-collapse:collapse;width:100%}}th,td{{border-bottom:1px solid #374151;text-align:left;padding:14px 10px}}code{{overflow-wrap:anywhere}}p{{line-height:1.6}}</style>
<h1>{esc(repo)} verification</h1><p>Source revision: <code>{esc(revision)}</code></p><table>{table}</table>
<p>This dashboard counts compiled C++ names, not recovered source or gameplay parity. Without the original ELF, the overlap is only a verified lower bound against previously identified names; newly matched names outside that subset are not counted. Host/modern ARM64 binaries are not byte-comparable with the 2013 ARM32 target.</p>
<p>Reference SHA-256: <code>{esc(reference_sha)}</code></p><p>Build archive SHA-256: <code>{esc(data['build_sha256'])}</code></p>
<p><a href="progress.json">Machine-readable report</a> · <a href="https://github.com/{esc(repo)}/actions">Build history</a></p></html>"""
    (args.output/"index.html").write_text(page, encoding="utf-8")

if __name__ == "__main__":
    main()
