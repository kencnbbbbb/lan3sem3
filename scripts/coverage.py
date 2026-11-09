#!/usr/bin/env python3
"""Run all three C++ suites and enforce >=85% line coverage of the ADT library."""
import argparse
import json
import os
from pathlib import Path
import shutil
import subprocess
import sys

ROOT = Path(__file__).resolve().parents[1]
def tool(name):
    found = shutil.which(name)
    if found:
        return found
    if shutil.which("xcrun"):
        return subprocess.check_output(["xcrun", "--find", name], text=True).strip()
    raise SystemExit(f"Required tool not found: {name}")
def run(args, **kwargs):
    return subprocess.run([str(v) for v in args], check=True, **kwargs)
def main():
    parser = argparse.ArgumentParser()
    parser.add_argument("--build", default=str(ROOT / "build"))
    parser.add_argument("--out", default=str(ROOT / "reports"))
    args = parser.parse_args()
    build, out = Path(args.build).resolve(), Path(args.out).resolve()
    cov, prof = tool("llvm-cov"), tool("llvm-profdata")
    sources = sorted(str(f) for f in (ROOT / "str").glob("*") if f.suffix in (".cpp", ".h"))
    result = {}
    for name in ("gtest", "boost", "catch2"):
        target = out / name
        target.mkdir(parents=True, exist_ok=True)
        for stale in build.glob(f"{name}-*.profraw"):
            stale.unlink()
        command = [build / f"test_{name}"]
        if name == "gtest":
            command.append(f"--gtest_output=xml:{target / 'tests.xml'}")
        elif name == "boost":
            command += ["--log_format=XML", f"--log_sink={target / 'tests.xml'}"]
        else:
            command += ["--reporter", "junit", "--out", str(target / "tests.xml")]
        env = dict(os.environ, LLVM_PROFILE_FILE=str(build / f"{name}-%p.profraw"))
        with (target / "tests.txt").open("w") as log:
            run(command, env=env, stdout=log, stderr=subprocess.STDOUT, cwd=ROOT)
        raw = sorted(build.glob(f"{name}-*.profraw"))
        if not raw:
            raise SystemExit("No profile data; configure with -DLAB3_COVERAGE=ON")
        profile = build / f"{name}.profdata"
        run([prof, "merge", "-sparse", *raw, "-o", profile])
        common = [build / f"test_{name}", f"-instr-profile={profile}", *sources]
        run([cov, "show", *common, "-format=html", f"-output-dir={target}", "-show-line-counts-or-regions", "-show-branches=count"])
        summary = subprocess.check_output([cov, "export", *map(str, common), "-summary-only"], text=True)
        data = json.loads(summary)
        (target / "coverage.json").write_text(json.dumps(data, indent=2))
        total = data["data"][0]["totals"]
        result[name] = {key: total[key] for key in ("lines", "functions", "branches", "regions")}
        with (target / "summary.txt").open("w") as log:
            run([cov, "report", *common], stdout=log)
        percent = total["lines"]["percent"]
        print(f"{name}: {percent:.2f}% lines")
        if percent < 85:
            raise SystemExit(f"{name}: coverage below 85%")
    (out / "cpp_coverage_summary.json").write_text(json.dumps(result, indent=2) + "\n")
if __name__ == "__main__":
    main()
