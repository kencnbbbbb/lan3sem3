#!/usr/bin/env python3
"""Compare actual C++/Go operations and serialized states to a Python reference.

The adapters call the public container APIs; they do not reimplement algorithms.
Random seed and input are deterministic. No third-party Python packages required.
"""
import argparse
import os
from pathlib import Path
import random
import subprocess
from cross_language_test import canonical, KINDS

ROOT = Path(__file__).resolve().parents[1]


def encoded(text):
    return text.encode().hex() or "-"


def decoded(text):
    return bytes.fromhex(text).decode() if text != "-" else ""


def tree_structure(values):
    paths = ["root"]
    for i in range(1, len(values)):
        paths.append(paths[(i - 1) // 2] + ("->left" if i % 2 else "->right"))
    return "".join(f"{p} = {v}\n" for p, v in zip(paths, values))


def scenarios(kind, count):
    rng = random.Random(325)
    values = []
    capacity = 16
    commands, expected = [], []
    strings = ["", "a", "dup", "two words", 'a"b', "a\\b", "Привет", "line\nbreak", "nul\x00byte"]
    if kind == "Array":
        ops = ["push", "insert", "get", "set", "erase", "clear", "clone", "len", "cap", "empty"]
    elif kind in ("OneLinked", "Linked"):
        ops = ["pushback", "pushfront", "after", "before", "popback", "popfront", "popafter", "popbefore", "find", "del", "reverse", "clear", "clone", "len", "empty"]
    elif kind in ("Stack", "Queue"):
        ops = ["push", "pop", "peek", "clear", "clone", "len", "empty"]
    elif kind == "Tree":
        ops = ["insert", "contains", "complete", "height", "clear", "clone", "len", "empty", "structure"]
    else:
        ops = ["insert", "del", "contains", "find", "clear", "clone", "len", "empty"]
    # Force growth / duplicate / boundary cases before the mixed random sequence.
    prefix = 180
    for step in range(count + prefix):
        op = ("insert" if kind in ("Tree", "OpenHash", "LinearHash") else
              "pushback" if kind in ("OneLinked", "Linked") else "push") if step < prefix else rng.choice(ops)
        value = (f"key{step}" if step < prefix else rng.choice(strings))
        if values and step >= prefix and rng.random() < .6:
            value = str(rng.choice(values))
        other = rng.choice(strings)
        index = rng.choice([-2147483648, 2147483647, -7, 0, 10, 10, 20]) if kind == "Tree" else rng.randint(-2, len(values) + 2)
        status, result = "OK", ""
        before = list(values)
        try:
            if op == "len": result = str(len(values))
            elif op == "empty": result = str(int(not values))
            elif op == "cap": result = str(capacity)
            elif op == "clear": values.clear()
            elif op == "clone": result = str(len(set(values + [value])) if kind.endswith("Hash") else len(values) + 1)
            elif kind == "Array":
                if op in ("insert", "get", "set", "erase"):
                    limit = len(values) + (op == "insert")
                    if not 0 <= index < limit: raise ValueError()
                if op == "push": values.append(value)
                elif op == "insert": values.insert(index, value)
                elif op == "get": result = values[index]
                elif op == "set": values[index] = value
                elif op == "erase": values.pop(index)
                if len(values) > capacity: capacity *= 2
            elif kind in ("OneLinked", "Linked"):
                if op == "pushback": values.append(value)
                elif op == "pushfront": values.insert(0, value)
                elif op == "reverse": result = "".join(" " + v for v in reversed(values))
                elif op == "find": result = str(values.index(value) if value in values else -1)
                elif op == "del":
                    result = str(int(value in values))
                    if value in values: values.remove(value)
                elif op in ("after", "before"):
                    position = values.index(value)
                    values.insert(position + (op == "after"), other)
                else:
                    position = (len(values) - 1 if op == "popback" else 0 if op == "popfront" else
                                values.index(value) + (1 if op == "popafter" else -1))
                    if not 0 <= position < len(values): raise ValueError()
                    result = values.pop(position)
            elif kind in ("Stack", "Queue"):
                if op == "push": values.append(value)
                else:
                    if not values: raise ValueError()
                    position = len(values) - 1 if kind == "Stack" else 0
                    result = values[position] if op == "peek" else values.pop(position)
            elif kind == "Tree":
                if op == "insert": values.append(index); result = "1"
                elif op == "contains": result = str(int(index in values))
                elif op == "height": result = str(len(values).bit_length())
                elif op == "complete": result = "1"
                elif op == "structure": result = tree_structure(values)
            else:
                present = value in values
                if op == "insert":
                    result = str(int(not present))
                    if not present: values.append(value)
                elif op == "del":
                    result = str(int(present))
                    if present: values.remove(value)
                else: result = str(int(present))
        except ValueError:
            status, result, values = "ERROR", "", before
        commands.append(f"{op} {index} {encoded(value)} {encoded(other)}\n")
        state = tuple(sorted(v.encode() for v in values)) if kind.endswith("Hash") else tuple(values if kind == "Tree" else [v.encode() for v in values])
        expected.append((status, result, state))
    return "".join(commands), expected


def run_driver(exe, kind, commands):
    result = subprocess.run([str(exe), kind], input=commands, capture_output=True, text=True, check=True, timeout=60)
    output = []
    for line in result.stdout.splitlines():
        status, value, state = line.split("\t")
        _, items = canonical(bytes.fromhex(state), kind)
        output.append((status, decoded(value), items))
    return output


def main():
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument("--build", default=str(ROOT / "build/parity"))
    parser.add_argument("--cxx", default=os.environ.get("CXX", "clang++"))
    parser.add_argument("--go", default="go")
    parser.add_argument("--operations", type=int, default=1000)
    parser.add_argument("--out", default=str(ROOT / "reports/semantic_parity.txt"))
    args = parser.parse_args()
    build = Path(args.build).resolve(); build.mkdir(parents=True, exist_ok=True)
    cpp, go = build / "cpp-parity", build / "go-parity"
    subprocess.run([args.cxx, "-std=c++17", "-O2", "-Wall", "-Wextra", "-pedantic",
                    *map(str, sorted((ROOT / "str").glob("*.cpp"))), str(ROOT / "tests/parity/cpp_driver.cpp"), "-o", str(cpp)], check=True)
    subprocess.run([args.go, "build", "-mod=vendor", "-o", str(go), str(ROOT / "tests/parity/go_driver.go")], cwd=ROOT / "golang", check=True)
    lines, total = [], 0
    for kind in KINDS:
        commands, expected = scenarios(kind, args.operations)
        actual_cpp, actual_go = run_driver(cpp, kind, commands), run_driver(go, kind, commands)
        assert len(actual_cpp) == len(actual_go) == len(expected), kind
        for i, (c, g, e) in enumerate(zip(actual_cpp, actual_go, expected)):
            assert c == g == e, (kind, i, commands.splitlines()[i], "C++", c, "Go", g, "reference", e)
        total += len(expected)
        lines.append(f"PASS: {kind}: {len(expected)} operations + state comparisons to Python reference")
    lines.append(f"PASS: {total} operation steps; C++ == Go == reference; 8 structure/mode cases")
    text = "\n".join(lines) + "\n"
    dest = Path(args.out); dest.parent.mkdir(parents=True, exist_ok=True); dest.write_text(text)
    print(text, end="")


if __name__ == "__main__":
    main()
