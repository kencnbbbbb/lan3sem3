#!/usr/bin/env python3
"""Enforce the coverage gate for every production file with instrumented statements."""
import argparse
from collections import defaultdict
from pathlib import Path
p = argparse.ArgumentParser()
p.add_argument("profile")
args = p.parse_args()
counts = defaultdict(lambda: [0, 0])
for line in Path(args.profile).read_text().splitlines()[1:]:
    block, statements, executed = line.split()
    path = block.rsplit(":", 1)[0]
    statements, executed = int(statements), int(executed)
    counts[path][0] += statements
    if executed:
        counts[path][1] += statements
for path, (total, covered) in sorted(counts.items()):
    percent = 100 * covered / total if total else 100
    print(f"{path}: {covered}/{total} = {percent:.2f}%")
    if percent < 85:
        raise SystemExit(f"Coverage below 85%: {path}")
total = sum(v[0] for v in counts.values())
covered = sum(v[1] for v in counts.values())
print(f"TOTAL: {covered}/{total} = {100 * covered / total:.2f}%")
