#!/usr/bin/env python3
"""Actual C++ <-> Go interoperability and CLI error checks; no external packages."""
import argparse
import re
import struct
from pathlib import Path
import subprocess
import tempfile

KINDS = ("Array", "OneLinked", "Linked", "Stack", "Queue", "OpenHash", "LinearHash", "Tree")
def canonical(raw, kind, binary=False):
    if binary:
        tag, count = raw[:4], struct.unpack_from("<Q", raw, 4)[0]
        offset, items = 12, []
        for _ in range(count):
            if kind == "Tree":
                items.append(struct.unpack_from("<i", raw, offset)[0]); offset += 4
            else:
                size = struct.unpack_from("<Q", raw, offset)[0]; offset += 8
                items.append(raw[offset:offset + size]); offset += size
        assert offset == len(raw)
    else:
        header, body = raw.split(b"\n", 1)
        tag, count = header.split(); count = int(count)
        if kind == "Tree":
            items = [int(v) for v in body.split()]
        else:
            items = [re.sub(rb"\\(.)", rb"\1", v, flags=re.DOTALL)
                     for v in re.findall(rb'"((?:\\.|[^"\\])*)"', body, flags=re.DOTALL)]
        assert len(items) == count
    if kind in ("OpenHash", "LinearHash"):
        items.sort()
    return tag, tuple(items)

def main():
    p = argparse.ArgumentParser()
    p.add_argument("--cpp", required=True)
    p.add_argument("--go", required=True)
    args = p.parse_args()
    cpp, go = str(Path(args.cpp).resolve()), str(Path(args.go).resolve())
    checks = 0
    def call(exe, *argv, ok=True):
        nonlocal checks
        result = subprocess.run([exe, *map(str, argv)], capture_output=True, text=True)
        assert (result.returncode == 0) == ok, (exe, argv, result.stdout, result.stderr)
        checks += 1
        return result
    with tempfile.TemporaryDirectory() as tmp:
        root = Path(tmp)
        for exe, folder in ((cpp, "cpp"), (go, "go")):
            call(exe, "--demo", root / folder)
        for kind in KINDS:
            for writer, reader, source in ((cpp, go, "cpp"), (go, cpp, "go")):
                text = root / source / (kind + ".txt")
                binary = root / source / (kind + ".bin")
                # Another language loads and writes both formats; first language reads again.
                out_bin, out_txt, final = root / "out.bin", root / "out.txt", root / "final.txt"
                call(reader, "--convert", kind, "text", text, "binary", out_bin)
                call(writer, "--convert", kind, "binary", out_bin, "text", final)
                assert canonical(final.read_bytes(), kind) == canonical(text.read_bytes(), kind), (kind, "text round trip")
                call(reader, "--convert", kind, "binary", binary, "text", out_txt)
                call(writer, "--convert", kind, "text", out_txt, "binary", out_bin)
                assert canonical(out_bin.read_bytes(), kind, True) == canonical(binary.read_bytes(), kind, True), (kind, "binary round trip")
            # Every truncation of a nonempty file must fail and preserve an existing output.
            raw = (root / "cpp" / (kind + ".bin")).read_bytes()
            for exe in (cpp, go):
                for cut in range(len(raw)):
                    bad, dest = root / "bad.bin", root / "unchanged.bin"
                    bad.write_bytes(raw[:cut]); dest.write_bytes(b"KEEP")
                    call(exe, "--convert", kind, "binary", bad, "binary", dest, ok=False)
                    assert dest.read_bytes() == b"KEEP"
                bad.write_bytes(raw + b"EXTRA")
                call(exe, "--convert", kind, "binary", bad, "binary", dest, ok=False)
        # Text strings with NUL, multiline content and escaped quotes, also UTF-8.
        sample = 'Array 4\n"line1\nline2"\n"nul\x00inside"\n"a\\\\b\\\"c"\n"тест"\n'
        text = root / "special.txt"; text.write_bytes(sample.encode())
        call(cpp, "--convert", "Array", "text", text, "binary", root / "special.bin")
        call(go, "--convert", "Array", "binary", root / "special.bin", "text", root / "special-out.txt")
        assert (root / "special-out.txt").read_bytes() == text.read_bytes()
        for exe in (cpp, go):
            call(exe, "--convert", "Tree", "text", root / "missing", "binary", root / "out", ok=False)
            call(exe, "--convert", "NoType", "text", text, "binary", root / "out", ok=False)
            call(exe, "--convert", "Array", "invalid", text, "binary", root / "out", ok=False)
    print(f"PASS: {checks} process checks, all 8 structures, both directions and formats")
if __name__ == "__main__":
    main()
