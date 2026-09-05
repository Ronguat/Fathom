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


# --- reading POSE -----------------------------------------------------------------

def role_pids(ctx):
    """role -> player id, from the ROLES marker."""
    out = {}
    for kind, rest in ctx.markers:
        if kind == "ROLES":
            for tok in rest.split()[1:]:
                if "=" in tok and ":" in tok:
                    role, where = tok.split("=", 1)
                    out[role] = int(where.split(":")[1])
    return out


def poses(ctx, world, pid):
    """sf -> the POSE line of one pawn in one world, the last line per simulation frame."""
    out = {}
    for ln in ctx.lines("POSE", world):
        if int(ln.fields.get("pid", -1)) == pid and "sf" in ln.fields:
            out[int(ln.fields["sf"])] = ln
    return out


def distance(a, b):
    return sum((a.fields[k] - b.fields[k]) ** 2 for k in ("x", "y", "z")) ** 0.5


# --- the harness assertions ---------------------------------------------------------

def determinism(ctx, r, s, tolerance_cm=1.0):
    """Each role's pawn on its own client against the same pawn on the server, at every matched
    simulation frame in the second half of the row."""
    pids = role_pids(ctx)
    for role, (world, _loc, _yaw) in sorted(s["roles"].items()):
        pid = pids.get(role, -1)
        server, client = poses(ctx, "S", pid), poses(ctx, world, pid)
        matched = sorted(set(server) & set(client))
        band(r, "%s POSE frames matched on S and %s" % (role, world), [len(matched)], 10, 10 ** 6, "")
        later = matched[len(matched) // 2:]
        band(r, "%s S vs %s distance, second half (cm)" % (role, world),
             [distance(server[f], client[f]) for f in later], 0.0, tolerance_cm, "cm")


def cost_sane(ctx, r):
    lines = ctx.lines("COST", "S")
    band(r, "server tick per COST line (ms)", [ln.fields["tick_ms"] for ln in lines], 0.0, 50.0, "ms")
    for conn in sorted(set(str(ln.fields.get("conn")) for ln in lines)):
        peak = max(ln.fields["in_bps"] for ln in lines if str(ln.fields.get("conn")) == conn)
        band(r, "%s inbound peak (B/s)" % conn, [peak], 1.0, 10 ** 9, "B/s")


@row("harness.idle")
def harness_idle(ctx, r, s):
    determinism(ctx, r, s)
    pids = role_pids(ctx)
    for role in sorted(s["roles"]):
        zs = [ln.fields["z"] for _sf, ln in sorted(poses(ctx, "S", pids.get(role, -1)).items())]
        later = zs[len(zs) // 2:]
        band(r, "%s stands still on the server, z spread (cm)" % role,
             [max(later) - min(later)] if later else [], 0.0, 1.0, "cm")
    cost_sane(ctx, r)


@row("harness.walk")
def harness_walk(ctx, r, s):
    determinism(ctx, r, s)
    pids = role_pids(ctx)
    p1 = sorted(poses(ctx, "S", pids.get("p1", -1)).items())
    if p1:
        first, last = p1[0][1], p1[-1][1]
        band(r, "p1 travelled +X on the server (cm)", [last.fields["x"] - first.fields["x"]], 1000.0, 2000.0, "cm")
        band(r, "p1 drift in Y on the server (cm)", [abs(last.fields["y"] - first.fields["y"])], 0.0, 20.0, "cm")
    else:
        r.add(False, "p1 travelled +X on the server (cm)", "no POSE for p1 on S")
    count(r, "p1 INPUT edges on C1", len(ctx.lines("INPUT", "C1", "role=p1 ")), 2)
    cost_sane(ctx, r)


ROWS["harness.walk-loss"] = harness_walk


@row("harness.jump")
def harness_jump(ctx, r, s):
    determinism(ctx, r, s)
    pids = role_pids(ctx)
    p1 = poses(ctx, "S", pids.get("p1", -1))
    press = ctx.first("INPUT", "C1", "action=jump edge=pressed")
    if p1 and press:
        before = [ln.fields["z"] for sf, ln in p1.items() if sf < press.frame]
        window = [ln.fields["z"] for sf, ln in p1.items() if press.frame <= sf <= press.frame + 60]
        after = [ln for sf, ln in p1.items() if press.frame + 120 <= sf <= press.frame + 180]
        base = before[-1] if before else None
        rest = min(ln.fields["z"] for ln in p1.values())
        band(r, "p1 jump height on the server (cm)",
             [max(window) - base] if window and base is not None else [], 80.0, 160.0, "cm")
        band(r, "p1 at rest height two seconds later (cm)", [ln.fields["z"] - rest for ln in after], 0.0, 2.0, "cm")
        equal(r, "p1 back in Walking two seconds later", [ln.fields.get("mode") for ln in after], "Walking")
        falling = [ln for sf, ln in p1.items() if press.frame <= sf <= press.frame + 60 and ln.fields.get("mode") == "Falling"]
        band(r, "p1 POSE lines in Falling during the jump", [len(falling)], 1, 20, "")
    else:
        r.add(False, "p1 jump height on the server (cm)", "no POSE for p1 on S, or no jump INPUT on C1")
    count(r, "p1 INPUT edges on C1", len(ctx.lines("INPUT", "C1", "role=p1 ")), 2)
    cost_sane(ctx, r)


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
