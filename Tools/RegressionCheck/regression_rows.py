"""Every row's assertions, and the vocabulary they are written in.

    regression_rows.py <run-id> <slice> [--tape <tape.tsv>]
    regression_rows.py --self-test

Prints one PASS or FAIL line per assertion and a "N passed, M failed" summary; exit 1 on any
FAIL, 2 when no row asserts the scenario. A row is a function in ROWS keyed by scenario id,
taking a Context, a Result and its scenario, and runs once per latency the scenario lists.
Every count-shaped assertion fails on n=0: a row that examined nothing has proven nothing.
"""
import argparse
import os
import sys
import tempfile

HERE = os.path.dirname(os.path.abspath(__file__))
sys.path.insert(0, HERE)
from regression_eval import Result, read, roles_from, fields  # noqa: E402

ROWS = {}


def row(sid):
    def bind(fn):
        ROWS[sid] = fn
        return fn
    return bind


# --- reading ----------------------------------------------------------------------

class Context(object):
    def __init__(self, slice_path, tape_path=None):
        self.trace, self.markers, self.raw, self.bad = read(slice_path)
        self.roles = roles_from(self.markers)
        self.latency = 0
        for kind, rest in self.markers:
            if kind == "BEGIN":
                self.latency = int(fields(rest).get("latency", 0))
        self.tape = self._tape(tape_path) if tape_path and os.path.exists(tape_path) else {}

    def _tape(self, path):
        """(role, world) -> samples of {frame, x, y, z, yaw}, in frame order."""
        out = {}
        with open(path) as fh:
            head = fh.readline().rstrip("\n").split("\t")
            for line in fh:
                parts = line.rstrip("\n").split("\t")
                if len(parts) < len(head):
                    continue
                s = dict(zip(head, parts))
                s["frame"] = int(s["frame"])
                for k in ("x", "y", "z", "yaw"):
                    s[k] = float(s[k])
                out.setdefault((s["role"], s["world"]), []).append(s)
        return out

    def lines(self, tag, world=None, contains=None):
        return [ln for ln in self.trace
                if ln.tag == tag and (world is None or ln.world == world)
                and (contains is None or contains in ln.text)]

    def first(self, tag, world=None, contains=None, after=None):
        for ln in self.lines(tag, world, contains):
            if after is None or ln.frame > after:
                return ln
        return None


# --- vocabulary -------------------------------------------------------------------

def band(r, label, values, lo, hi, unit="f"):
    """Every value inside [lo, hi], and at least one value."""
    values = list(values)
    if not values:
        r.add(False, label, "nothing to measure")
        return
    out = [v for v in values if v < lo or v > hi]
    r.add(not out, label, "%d in [%s, %s] %s%s" % (
        len(values), lo, hi, unit, "" if not out else ", %d outside: %s" % (len(out), out[:3])))


def equal(r, label, values, want):
    values = list(values)
    if not values:
        r.add(False, label, "nothing to compare")
        return
    out = [v for v in values if v != want]
    r.add(not out, label, "%d = %s%s" % (len(values), want, "" if not out else ", saw %s" % out[:3]))


def count(r, label, n, want):
    r.add(n == want and n > 0, label, "%d, want %d" % (n, want))


def frames_apart(a, b):
    """Frames between two trace lines, positive when b is later."""
    return b.frame - a.frame


# --- self-test --------------------------------------------------------------------

SELF_TEST_SLICE = """\
[t][  0]LogPython: REGRESSION BEGIN fixture@50 run=t latency=50 loss=0 idx=0
[t][  0]LogPython: REGRESSION ROLES fixture@50 p1=C1:7
[t][  0]LogFMTrace: [100] [C1] INPUT role=p1 action=jump edge=pressed
[t][  0]LogFMTrace: [112] [C1] JUMP role=p1
[t][  0]LogFMTrace: [200] [C1] INPUT role=p1 action=jump edge=pressed
[t][  0]LogFMTrace: [213] [C1] JUMP role=p1
"""


def self_test():
    """A correct band and a wrong one over the same slice, and a band over nothing: the
    instrument must pass the first and fail the other two."""
    fd, path = tempfile.mkstemp(suffix=".slice.log")
    with os.fdopen(fd, "w") as fh:
        fh.write(SELF_TEST_SLICE)
    try:
        ctx = Context(path)
    finally:
        os.remove(path)
    gaps = [frames_apart(i, j) for i, j in zip(ctx.lines("INPUT"), ctx.lines("JUMP"))]
    outcomes = []
    for label, lo, hi in (("control band", 10, 15), ("wrong band", 20, 30)):
        r = Result()
        band(r, label, gaps, lo, hi)
        outcomes.append(r.failed)
    r = Result()
    band(r, "empty band", [], 0, 1)
    outcomes.append(r.failed)
    r = Result()
    equal(r, "latency read", [ctx.latency], 50)
    outcomes.append(r.failed)
    if outcomes == [0, 1, 1, 0]:
        print("SELF-TEST PASSED (4 assertions)")
        return 0
    print("SELF-TEST FAIL: outcomes %s, want [0, 1, 1, 0]" % outcomes)
    return 1


def main():
    if "--self-test" in sys.argv[1:]:
        return self_test()
    ap = argparse.ArgumentParser()
    ap.add_argument("run_id")
    ap.add_argument("slice")
    ap.add_argument("--tape")
    a = ap.parse_args()
    import scenarios as SC
    sid, _ms = SC.split_run_id(a.run_id)
    if sid not in ROWS:
        print("  no row asserts %s" % sid)
        return 2
    ctx = Context(a.slice, a.tape)
    r = Result()
    ROWS[sid](ctx, r, SC.SCENARIOS[sid])
    r.show()
    print("  %d passed, %d failed" % (len(r.rows) - r.failed, r.failed))
    return 1 if r.failed else 0


if __name__ == "__main__":
    sys.exit(main())
