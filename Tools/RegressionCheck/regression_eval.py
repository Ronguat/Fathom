"""Evaluates one row's slice: the universal set, the cost readout, golden skeletons, mutations.

    regression_eval.py --universal <slice> [--allow a,b]   the invariants every slice must hold
    regression_eval.py --cost      <slice>                 bandwidth per connection, server tick per player
    regression_eval.py --skeleton  <slice> [--keep x,y]    one line per trace event, for the golden diff
    regression_eval.py --golden    <slice> --id <id> [--keep x,y] [--accept]
    regression_eval.py --mutate kind:arg:arg <slice> --out <path>
    regression_eval.py --self-test

A trace line is `LogFMTrace: [<frame>] [<world>] TAG key=value ...`: the shared simulation
frame, the world (S, or C<n>), a tag of upper-case words, then fields. Markers are
`REGRESSION <KIND> ...` lines the runner writes. The universal set is what a row's own
assertions cannot see: a row asserts the mechanic it was written for and stays green while
something beside it leaks.
"""
import argparse
import os
import re
import sys
import tempfile

HERE = os.path.dirname(os.path.abspath(__file__))
GOLDEN_DIR = os.path.join(HERE, "golden")
ALLOWLIST = os.path.join(HERE, "log-allowlist.txt")

TRACE = re.compile(r"LogFMTrace: \[(\d+)\] \[(S|C\d+)\] ([A-Z][A-Z ]*?)(?: (.*))?$")
TRACE_ANY = re.compile(r"LogFMTrace: (.*)$")
MARKER = re.compile(r"REGRESSION ([A-Z]+) (.*)$")
# Frames an injection's delivery may move within a row: the prediction framework throttles an
# autonomous client's simulation frequency to keep the server's input buffer fed, one frame at a time.
INJECTION_SPREAD_TOL = 1
# Engine categories whose warnings and errors the universal set surfaces.
WARN_CATEGORIES = ("LogFathom", "LogFMTrace", "LogNetworkPrediction", "LogMover", "LogNet",
                   "LogNetPackageMap", "LogScript", "LogBlueprint", "LogPython")


class Line(object):
    __slots__ = ("frame", "world", "tag", "text", "fields")

    def __init__(self, frame, world, tag, text):
        self.frame, self.world, self.tag, self.text = frame, world, tag, text
        self.fields = fields(text)


def fields(text):
    out = {}
    for k, v in re.findall(r"([A-Za-z_][A-Za-z0-9_]*)=([^\s]+)", text or ""):
        try:
            out[k] = float(v)
        except ValueError:
            out[k] = v
    return out


def read(path):
    """(trace lines, markers, raw lines, malformed trace lines) in log order."""
    trace, markers, raw, bad = [], [], [], []
    for line in open(path, errors="replace"):
        line = line.rstrip("\n")
        raw.append(line)
        m = TRACE.search(line)
        if m:
            trace.append(Line(int(m.group(1)), m.group(2), m.group(3).strip(), m.group(4) or ""))
            continue
        if TRACE_ANY.search(line):
            bad.append(line)
            continue
        m = MARKER.search(line)
        if m:
            markers.append((m.group(1), m.group(2).rstrip()))
    return trace, markers, raw, bad


def roles_from(markers):
    """role -> world, from the ROLES marker: `ROLES <id> p1=C1:7 p2=C2:8`."""
    out = {}
    for kind, rest in markers:
        if kind == "ROLES":
            for tok in rest.split()[1:]:
                if "=" in tok:
                    role, where = tok.split("=", 1)
                    out[role] = where.split(":")[0]
    return out


# --- the universal set ----------------------------------------------------------

class Result(object):
    def __init__(self):
        self.rows = []

    def add(self, ok, label, detail):
        self.rows.append(("PASS" if ok else "FAIL", label, detail))

    @property
    def failed(self):
        return sum(1 for s, _, _ in self.rows if s != "PASS")

    def show(self, indent="  "):
        for status, label, detail in self.rows:
            print("%s%-6s %-34s %s" % (indent, status, label, detail))


def universal(trace, markers, raw, bad, r, allow=(), injection_tolerance=INJECTION_SPREAD_TOL):
    roles = roles_from(markers)
    worlds = sorted(set(["S"] + list(roles.values())), key=lambda w: (w != "S", w))

    r.add(not bad, "trace lines well-formed",
          "%d line(s) without frame, world and tag, first: %s" % (len(bad), bad[0][-80:]) if bad
          else "%d lines" % len(trace))

    out_of_order = []
    last = {}
    for ln in trace:
        if ln.world in last and ln.frame < last[ln.world]:
            out_of_order.append("%s f=%d after f=%d" % (ln.world, ln.frame, last[ln.world]))
        last[ln.world] = ln.frame
    r.add(not out_of_order, "frames monotonic per world",
          "; ".join(out_of_order[:3]) if out_of_order else "%d world(s)" % len(last))

    silent = [w for w in worlds if w not in last]
    r.add(not silent, "every world reports",
          "no trace from %s" % ", ".join(silent) if silent else ", ".join(worlds))

    clients = [w for w in worlds if w != "S"]
    costed = set(str(ln.fields.get("conn", "")) for ln in trace if ln.tag == "COST" and ln.world == "S")
    uncosted = [w for w in clients if w not in costed]
    r.add(not uncosted, "cost printed per connection",
          "no COST line for %s" % ", ".join(uncosted) if uncosted else "%d connection(s)" % len(clients))

    injected, consumed = injections(trace, markers)
    by_role = injection_deltas(trace, markers)
    if by_role:
        spans = dict((role, max(d) - min(d)) for role, d in sorted(by_role.items()))
        worst = max(spans.values())
        r.add(worst <= injection_tolerance, "injection latency constant",
              "%d injection(s), %s" % (sum(len(d) for d in by_role.values()),
                                       "; ".join("%s %d frame(s)%s" % (role, d[0], "" if spans[role] == 0 else ", spread %d" % spans[role])
                                                 for role, d in sorted(by_role.items()))))
    else:
        r.add(True, "injection latency constant", "no injections")

    extra = ["%s %s %s" % k for k in sorted(consumed) if len(consumed[k]) > len(injected.get(k, []))]
    r.add(not extra, "every INPUT injected",
          "%d unaccounted for: %s" % (len(extra), "; ".join(extra[:3])) if extra
          else "%d INPUT line(s)" % sum(len(v) for v in consumed.values()))

    allow = list(allow) + allowlist()
    warns = [l for l in raw
             if any(c + ": Warning:" in l or c + ": Error:" in l for c in WARN_CATEGORIES)
             and not any(a in l for a in allow)]
    r.add(not warns, "no unallowed engine warnings",
          "%d line(s), first: %s" % (len(warns), warns[0][-90:]) if warns else "none")


def allowlist():
    if not os.path.exists(ALLOWLIST):
        return []
    return [l.split("#")[0].strip() for l in open(ALLOWLIST) if l.split("#")[0].strip()]


def injections(trace, markers):
    """Per (role, action, edge), the frames of every INJECT marker and of every INPUT line:
    `INJECT <id> frame=<f> <role> <action> <press|release>` against
    `INPUT role=<r> action=<a> edge=<pressed|released>`."""
    edges = {"press": "pressed", "release": "released"}
    injected = {}
    for kind, rest in markers:
        if kind != "INJECT":
            continue
        toks = rest.split()
        f = fields(rest).get("frame")
        if f is None or len(toks) < 5:
            continue
        injected.setdefault((toks[-3], toks[-2], edges.get(toks[-1], toks[-1])), []).append(int(f))
    consumed = {}
    for ln in trace:
        if ln.tag == "INPUT":
            key = (str(ln.fields.get("role")), str(ln.fields.get("action")), str(ln.fields.get("edge")))
            consumed.setdefault(key, []).append(ln.frame)
    return injected, consumed


def injection_deltas(trace, markers):
    """Frames from each INJECT marker to the INPUT line that carries it, paired in order, per
    role: each client leads the server by its own margin."""
    injected, consumed = injections(trace, markers)
    out = {}
    for key, sent in injected.items():
        got = consumed.get(key, [])
        out.setdefault(key[0], []).extend(g - s for s, g in zip(sent, got))
    return dict((role, d) for role, d in out.items() if d)


# --- cost -----------------------------------------------------------------------

def cost(trace):
    """Per connection, the mean of in_bps, out_bps, tick_ms, lag_ms and players off the server's
    COST lines."""
    sums, n = {}, {}
    for ln in trace:
        if ln.tag != "COST" or ln.world != "S":
            continue
        conn = str(ln.fields.get("conn", "?"))
        n[conn] = n.get(conn, 0) + 1
        acc = sums.setdefault(conn, {})
        for k in ("in_bps", "out_bps", "tick_ms", "lag_ms", "players"):
            acc[k] = acc.get(k, 0.0) + float(ln.fields.get(k, 0.0))
    out = {}
    for conn in sorted(sums):
        out[conn] = dict((k, v / n[conn]) for k, v in sums[conn].items())
        out[conn]["samples"] = n[conn]
    return out


def cost_text(c):
    if not c:
        return "no COST lines"
    return "; ".join("%s in %.0f out %.0f bps, lag %.0f ms, tick %.2f ms, %.2f/player (%d)" % (
        k, v["in_bps"], v["out_bps"], v["lag_ms"], v["tick_ms"],
        v["tick_ms"] / v["players"] if v["players"] else 0.0, v["samples"]) for k, v in sorted(c.items()))


# --- golden skeletons -------------------------------------------------------------

# A skeleton line that reappears with identical content within SHIFT_TOL frames counts as shifted,
# not changed, and the shifted count is reported.
SHIFT_TOL = 2


def skeleton(trace, keep=()):
    """One line per event: frame, world, tag, and the fields in keep. Frames are the shared
    simulation frame as the line carries it."""
    rows = {}
    for ln in trace:
        kept = ["%s=%s" % (k, ln.text.split(k + "=", 1)[1].split()[0]) for k in keep if (k + "=") in ln.text]
        rows.setdefault(ln.frame, []).append("f=%d %s %s %s" % (ln.frame, ln.world, ln.tag, " ".join(kept)))
    out = []
    for f in sorted(rows):
        out.extend(sorted(rows[f]))
    return out


def split_line(line):
    m = re.match(r"f=(-?\d+) (.*)$", line)
    if not m:
        return None, line
    return int(m.group(1)), m.group(2)


def compare(ref, now):
    """Lines in one set and not the other, after content-identical lines within SHIFT_TOL frames
    of each other are paired off. Returns (gone, new, shifted)."""
    gone = [l for l in ref if l not in now]
    new = [l for l in now if l not in ref]
    shifted, still_gone, pool = 0, [], list(new)
    for g in gone:
        f, content = split_line(g)
        match = None
        for n in pool:
            fn, cn = split_line(n)
            if cn == content and f is not None and fn is not None and abs(fn - f) <= SHIFT_TOL:
                match = n
                break
        if match is None:
            still_gone.append(g)
        else:
            pool.remove(match)
            shifted += 1
    return still_gone, pool, shifted


def golden(rid, trace, accept, keep):
    os.makedirs(GOLDEN_DIR, exist_ok=True)
    ref_path = os.path.join(GOLDEN_DIR, "%s.skeleton" % rid)
    now = skeleton(trace, keep)
    if accept or not os.path.exists(ref_path):
        with open(ref_path, "w") as fh:
            fh.write("\n".join(now) + "\n")
        return "ACCEPTED", "%d lines written to golden/%s.skeleton" % (len(now), rid)
    ref = [l.rstrip("\n") for l in open(ref_path) if l.strip()]
    if ref == now:
        return "SAME", "%d lines match" % len(now)
    gone, new, shifted = compare(ref, now)
    if not gone and not new:
        return "SAME", "%d lines, %d shifted within %d f" % (len(now), shifted, SHIFT_TOL)
    detail = "%d -> %d lines" % (len(ref), len(now))
    if shifted:
        detail += ", %d shifted within %d f" % (shifted, SHIFT_TOL)
    if gone:
        detail += " | gone: " + "; ".join(gone[:4])
    if new:
        detail += " | new: " + "; ".join(new[:4])
    return "CHANGED", detail


# --- mutations -------------------------------------------------------------------
# A row nobody has seen reject anything is indistinguishable from one that cannot. Each mutation
# is applied to a copy of the slice and the row must then go red.

def apply_mutation(lines, mut):
    """A mutated copy of the slice's lines. mut is (kind, ...) per scenarios.py: shift moves one
    tag's frames by N, drop removes the first N lines of a tag, dup repeats them, set rewrites one
    field on every line of a tag, regex substitutes on every line."""
    kind = mut[0]
    out = []
    if kind == "shift":
        tag, delta = mut[1], int(mut[2])
        for line in lines:
            m = TRACE.search(line)
            if m and m.group(3).strip() == tag:
                line = line.replace("[%s]" % m.group(1), "[%d]" % (int(m.group(1)) + delta), 1)
            out.append(line)
    elif kind == "drop":
        tag, n = mut[1], int(mut[2])
        for line in lines:
            m = TRACE.search(line)
            if m and m.group(3).strip() == tag and n > 0:
                n -= 1
                continue
            out.append(line)
    elif kind == "dup":
        tag, n = mut[1], int(mut[2])
        for line in lines:
            out.append(line)
            m = TRACE.search(line)
            if m and m.group(3).strip() == tag and n > 0:
                n -= 1
                out.append(line)
    elif kind == "set":
        tag, field, value = mut[1], mut[2], str(mut[3])
        pat = re.compile(r"(\b%s=)([^\s]+)" % re.escape(field))
        for line in lines:
            m = TRACE.search(line)
            if m and m.group(3).strip() == tag:
                line = pat.sub(lambda g: g.group(1) + value, line, count=1)
            out.append(line)
    elif kind == "regex":
        pat, rep = mut[1], mut[2]
        out = [re.sub(pat, rep, line) for line in lines]
    else:
        raise ValueError("unknown mutation %r" % (mut,))
    return out


def mutate_file(src, dst, mut):
    lines = open(src, errors="replace").read().split("\n")
    changed = apply_mutation(lines, mut)
    with open(dst, "w", errors="replace") as fh:
        fh.write("\n".join(changed))
    return sum(1 for a, b in zip(lines, changed) if a != b) or abs(len(lines) - len(changed))


# --- self-test -------------------------------------------------------------------

GOOD_SLICE = """\
[2026.01.01-00.00.00:000][  0]LogPython: REGRESSION BEGIN smoke@0 run=t latency=0 loss=0 idx=0
[2026.01.01-00.00.00:000][  0]LogPython: REGRESSION ROLES smoke@0 p1=C1:7 p2=C2:8
[2026.01.01-00.00.00:000][  0]LogFMTrace: [100] [S] COST conn=C1 in_bps=1200 out_bps=8000 tick_ms=0.40
[2026.01.01-00.00.00:000][  0]LogFMTrace: [100] [S] COST conn=C2 in_bps=1100 out_bps=8100 tick_ms=0.40
[2026.01.01-00.00.00:000][  0]LogPython: REGRESSION INJECT smoke@0 frame=10 p1 jump press
[2026.01.01-00.00.00:000][  0]LogFMTrace: [111] [C1] INPUT role=p1 action=jump edge=pressed
[2026.01.01-00.00.00:000][  0]LogPython: REGRESSION INJECT smoke@0 frame=40 p1 jump press
[2026.01.01-00.00.00:000][  0]LogFMTrace: [141] [C1] INPUT role=p1 action=jump edge=pressed
[2026.01.01-00.00.00:000][  0]LogFMTrace: [150] [C2] MOVE role=p2 x=10.0 y=0.0
[2026.01.01-00.00.00:000][  0]LogFMTrace: [160] [S] MOVE role=p2 x=10.0 y=0.0
[2026.01.01-00.00.00:000][  0]LogPython: REGRESSION END smoke@0 status=ok frames=200
"""

BAD_LINE = "[2026.01.01-00.00.00:000][  0]LogFMTrace: MOVE without a frame\n"
BAD_ORDER = "[2026.01.01-00.00.00:000][  0]LogFMTrace: [90] [S] MOVE role=p2 x=0.0 y=0.0\n"
BAD_WARN = "[2026.01.01-00.00.00:000][  0]LogNet: Warning: something the row did not ask for\n"
BAD_INPUT = "[2026.01.01-00.00.00:000][  0]LogFMTrace: [170] [C1] INPUT role=p1 action=jump edge=released\n"


def _universal_on(text, allow=()):
    fd, path = tempfile.mkstemp(suffix=".slice.log")
    with os.fdopen(fd, "w") as fh:
        fh.write(text)
    try:
        trace, markers, raw, bad = read(path)
    finally:
        os.remove(path)
    r = Result()
    universal(trace, markers, raw, bad, r, allow)
    return dict((label, status) for status, label, _ in r.rows)


def self_test():
    """Every universal assertion passes on the good slice and each fails on one deliberate
    corruption, and every mutation kind changes a line."""
    bad = 0

    def expect(desc, ok):
        nonlocal bad
        if not ok:
            print("SELF-TEST FAIL: " + desc)
            bad += 1

    good = _universal_on(GOOD_SLICE)
    expect("good slice passes every row", all(v == "PASS" for v in good.values()))
    expect("well-formed fails on a frameless line",
           _universal_on(GOOD_SLICE + BAD_LINE)["trace lines well-formed"] == "FAIL")
    expect("monotonic fails on a frame going backwards",
           _universal_on(GOOD_SLICE + BAD_ORDER)["frames monotonic per world"] == "FAIL")
    silent = GOOD_SLICE.replace("[150] [C2] MOVE role=p2 x=10.0 y=0.0\n", "")
    expect("every world fails when a client is silent",
           _universal_on(silent)["every world reports"] == "FAIL")
    uncosted = GOOD_SLICE.replace("COST conn=C2 ", "NOTE conn=C2 ")
    expect("cost fails on a connection without COST",
           _universal_on(uncosted)["cost printed per connection"] == "FAIL")
    jitter = GOOD_SLICE.replace("[141] [C1] INPUT", "[143] [C1] INPUT")
    expect("injection latency fails on a spread",
           _universal_on(jitter)["injection latency constant"] == "FAIL")
    expect("warnings fail on an unallowed engine warning",
           _universal_on(GOOD_SLICE + BAD_WARN)["no unallowed engine warnings"] == "FAIL")
    expect("every INPUT injected fails on an INPUT no INJECT accounts for",
           _universal_on(GOOD_SLICE + BAD_INPUT)["every INPUT injected"] == "FAIL")
    expect("warnings pass when the line is allowed",
           _universal_on(GOOD_SLICE + BAD_WARN, ["did not ask"])["no unallowed engine warnings"] == "PASS")

    lines = GOOD_SLICE.split("\n")
    for mut in (("shift", "COST", 5), ("drop", "INPUT", 1), ("dup", "MOVE", 1),
                ("set", "COST", "tick_ms", "9.99"), ("regex", r"role=p2", "role=px")):
        changed = apply_mutation(lines, mut)
        expect("mutation %s changes the slice" % mut[0], changed != lines)
    trace, _m, _r, _b = read_text(GOOD_SLICE)
    expect("skeleton keeps the named field",
           any("tick_ms=0.40" in l for l in skeleton(trace, ["tick_ms"])))
    gone, new, shifted = compare(skeleton(trace), skeleton(read_text(GOOD_SLICE.replace("[160] [S]", "[161] [S]"))[0]))
    expect("a one-frame shift reads as shifted, not changed", not gone and not new and shifted == 1)
    c = cost(trace)
    expect("cost averages per connection", abs(c["C1"]["out_bps"] - 8000.0) < 1e-6 and c["C2"]["samples"] == 1)
    if bad:
        return 1
    print("SELF-TEST PASSED (17 assertions)")
    return 0


def read_text(text):
    fd, path = tempfile.mkstemp(suffix=".slice.log")
    with os.fdopen(fd, "w") as fh:
        fh.write(text)
    try:
        return read(path)
    finally:
        os.remove(path)


def main():
    if "--self-test" in sys.argv[1:]:
        return self_test()
    ap = argparse.ArgumentParser()
    ap.add_argument("slice")
    ap.add_argument("--universal", action="store_true")
    ap.add_argument("--cost", action="store_true")
    ap.add_argument("--skeleton", action="store_true")
    ap.add_argument("--golden", action="store_true")
    ap.add_argument("--id")
    ap.add_argument("--accept", action="store_true")
    ap.add_argument("--allow", default="", help="warning substrings this row tolerates")
    ap.add_argument("--injection-tolerance", type=int, default=INJECTION_SPREAD_TOL,
                    help="frames the injection pairing may move within the row")
    ap.add_argument("--keep", default="", help="fields the skeleton keeps")
    ap.add_argument("--mutate", help="kind:arg:arg, written to --out")
    ap.add_argument("--out")
    a = ap.parse_args()

    if a.mutate:
        n = mutate_file(a.slice, a.out, tuple(a.mutate.split(":")))
        print("mutated %d line(s) -> %s" % (n, a.out))
        return 0

    trace, markers, raw, bad = read(a.slice)
    keep = [k for k in a.keep.split(",") if k]
    if a.skeleton:
        print("\n".join(skeleton(trace, keep)))
        return 0
    if a.cost:
        print("  " + cost_text(cost(trace)))
        return 0
    if a.golden:
        status, detail = golden(a.id, trace, a.accept, keep)
        print("%s %s" % (status, detail))
        return 0
    r = Result()
    universal(trace, markers, raw, bad, r, [x for x in a.allow.split(",") if x], a.injection_tolerance)
    r.show()
    return 1 if r.failed else 0


if __name__ == "__main__":
    sys.exit(main())
