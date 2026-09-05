"""Turns a field session's bundle into a slice the evaluator reads, and lists its marks.

    ingest_bundle.py <bundle dir> [--id <slice id>] [--out <dir>]

The bundle is a server world's directory under Saved/Fathom/Sessions/: trace.log carrying the
server's lines and every relayed client line, and meta.json. The slice wraps the trace in the
markers a regression row would have: BEGIN with the mean measured lag as its latency, ROLES with
one role per connection, END. Then the universal set and the cost readout run on it, and every
MARK line is listed with its frame and world. Exit 1 if the universal set fails.
"""
import argparse
import json
import os
import re
import subprocess
import sys
import time

HERE = os.path.dirname(os.path.abspath(__file__))
sys.path.insert(0, HERE)
import paths  # noqa: E402
from regression_eval import TRACE  # noqa: E402

EVAL = os.path.join(HERE, "regression_eval.py")


def load(bundle):
    trace = os.path.join(bundle, "trace.log")
    meta = os.path.join(bundle, "meta.json")
    if not os.path.exists(trace):
        raise SystemExit("no trace.log in %s" % bundle)
    lines = [l.rstrip("\n") for l in open(trace, encoding="utf-8", errors="replace")]
    info = json.load(open(meta, encoding="utf-8")) if os.path.exists(meta) else {}
    return lines, info


def roles(lines, info):
    """One role per client world seen: the tags meta names, else the tags the trace carries."""
    tags = [c["conn"] for c in info.get("connections", []) if str(c.get("conn", "")).startswith("C")]
    if not tags:
        seen = set()
        for l in lines:
            m = TRACE.search("LogFMTrace: " + l)
            if m and m.group(2) != "S":
                seen.add(m.group(2))
        tags = sorted(seen, key=lambda t: int(t[1:]))
    pids = {}
    for l in lines:
        m = re.search(r"\[(C\d+)\] POSE pid=(\d+)", l)
        if m and m.group(1) not in pids:
            pids[m.group(1)] = int(m.group(2))
    return dict(("p%d" % (i + 1), (tag, pids.get(tag, 0))) for i, tag in enumerate(tags))


def write_slice(lines, info, sid, out_dir):
    conns = info.get("connections", [])
    lag = sum(float(c.get("lag_ms", 0.0)) for c in conns) / len(conns) if conns else 0.0
    r = roles(lines, info)
    stamp = "[%s][  0]" % time.strftime("%Y.%m.%d-%H.%M.%S:000")
    out = [
        "%sLogPython: REGRESSION BEGIN %s run=field latency=%d loss=0 idx=0" % (stamp, sid, round(lag)),
        "%sLogPython: REGRESSION ROLES %s %s" % (stamp, sid, " ".join(
            "%s=%s:%d" % (role, tag, pid) for role, (tag, pid) in sorted(r.items()))),
    ]
    out.extend("%sLogFMTrace: %s" % (stamp, l) for l in lines)
    out.append("%sLogPython: REGRESSION END %s status=ok frames=%d" % (
        stamp, sid, int(info.get("last_frame", 0)) - int(info.get("first_frame", 0))))
    os.makedirs(out_dir, exist_ok=True)
    path = os.path.join(out_dir, "%s.slice.log" % sid)
    with open(path, "w", encoding="utf-8") as fh:
        fh.write("\n".join(out) + "\n")
    return path, r, lag


def main():
    ap = argparse.ArgumentParser()
    ap.add_argument("bundle")
    ap.add_argument("--id", default=None)
    ap.add_argument("--out", default=os.path.join(paths.REG, "field"))
    a = ap.parse_args()
    bundle = os.path.abspath(a.bundle)
    sid = a.id or "field@" + os.path.basename(bundle.rstrip("\\/"))
    lines, info = load(bundle)
    path, r, lag = write_slice(lines, info, sid, a.out)
    print("  bundle %s: %d line(s), commit %s, mean lag %.0f ms" % (
        os.path.basename(bundle), len(lines), info.get("commit", "?")[:9] or "?", lag))
    print("  roles: %s" % ", ".join("%s=%s:%d" % (k, v[0], v[1]) for k, v in sorted(r.items())))
    print("  slice: %s" % os.path.relpath(path, paths.ROOT))
    u = subprocess.run([paths.ENGINE_PY, EVAL, path, "--universal"], capture_output=True, text=True)
    print(u.stdout.rstrip())
    c = subprocess.run([paths.ENGINE_PY, EVAL, path, "--cost"], capture_output=True, text=True)
    print("  cost:" + c.stdout.rstrip())
    marks = []
    for l in lines:
        m = TRACE.search("LogFMTrace: " + l)
        if m and m.group(3).strip() == "MARK":
            marks.append("f=%s %s %s" % (m.group(1), m.group(2), m.group(4) or ""))
    print("  marks: %s" % ("; ".join(marks) if marks else "none"))
    return u.returncode


if __name__ == "__main__":
    sys.exit(main())
