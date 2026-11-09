#!/usr/bin/env python3
"""C++ static analysis + focused cpplint rules, Go vet + gofmt."""
import os
from pathlib import Path
import subprocess
import sys
ROOT = Path(__file__).resolve().parents[1]
def run(args, **kw):
    subprocess.run([str(v) for v in args], check=True, **kw)
cpp = sorted((ROOT / "str").glob("*.cpp"))
headers = sorted((ROOT / "str").glob("*.h"))
for path in cpp:
    print("Clang analyzer:", path.name, flush=True)
    run(["clang++", "--analyze", "-std=c++17", "-I", ROOT / "str", "-Xanalyzer", "-analyzer-output=text", path])
run([sys.executable, "-m", "cpplint",
     "--filter=-,+runtime/explicit,+runtime/virtual,+runtime/arrays,+readability/casting,+readability/null,+readability/alt_tokens",
     *headers, *cpp, ROOT / "main.cpp"])
run(["go", "vet", "./..."], cwd=ROOT / "golang")
result = subprocess.check_output(["gofmt", "-l", "ds", "cmd"], cwd=ROOT / "golang", text=True)
if result.strip():
    raise SystemExit("Run gofmt for:\n" + result)
print("PASS: static analysis and configured style checks")
