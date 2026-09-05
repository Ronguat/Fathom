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


# --- the ship assertions ----------------------------------------------------------

def ship_lines(ctx, world):
    return dict((int(ln.fields["sf"]), ln) for ln in ctx.lines("SHIP", world) if "sf" in ln.fields)


def begin_frame(ctx):
    for kind, rest in ctx.markers:
        if kind == "BEGIN":
            return int(fields(rest).get("frame", 0))
    return 0


def yaw_gap(a, b):
    d = abs(a - b) % 360.0
    return min(d, 360.0 - d)


def ship_reconstruction(ctx, r, s, settle_after):
    """Each client's SHIP pose against the server's at the same frame from settle_after the row's
    start; the transient before it reported."""
    server = ship_lines(ctx, "S")
    band(r, "SHIP lines on S", [len(server)], 10, 10 ** 6, "")
    applied = [int(ln.fields["sf"]) for ln in ctx.lines("SHIPIN", "S") if "sf" in ln.fields]
    settle = int(s.get("settle_frames", 30))
    start = (max(applied) + settle) if applied else (begin_frame(ctx) + settle_after)
    for world in s["worlds"]:
        if world == "S":
            continue
        client = ship_lines(ctx, world)
        matched = sorted(set(server) & set(client))
        settled = [f for f in matched if f >= start]
        band(r, "%s SHIP frames matched with S after settle" % world, [len(settled)], 10, 10 ** 6, "")
        pos = [((server[f].fields["x"] - client[f].fields["x"]) ** 2 + (server[f].fields["y"] - client[f].fields["y"]) ** 2
                + (server[f].fields["z"] - client[f].fields["z"]) ** 2) ** 0.5 for f in settled]
        yaw = [yaw_gap(server[f].fields["yaw"], client[f].fields["yaw"]) for f in settled]
        band(r, "%s position error after settle (cm)" % world, pos, 0.0, 10.0, "cm")
        band(r, "%s heading error after settle (deg)" % world, yaw, 0.0, 1.0, "deg")
        early = [f for f in matched if f < start]
        peak = max([((server[f].fields["x"] - client[f].fields["x"]) ** 2 + (server[f].fields["y"] - client[f].fields["y"]) ** 2) ** 0.5
                    for f in early] or [0.0])
        over = [f for f in early if ((server[f].fields["x"] - client[f].fields["x"]) ** 2 + (server[f].fields["y"] - client[f].fields["y"]) ** 2) ** 0.5 > 10.0]
        r.add(True, "%s transient before settle" % world, "peak %.1f cm, %d sample(s) over 10 cm" % (peak, len(over)))


def server_at(ctx, rel_frame):
    """The server's SHIP line at or after a frame counted from the row's start."""
    server = ship_lines(ctx, "S")
    target = begin_frame(ctx) + rel_frame
    for f in sorted(server):
        if f >= target:
            return server[f]
    return None


@row("ship.sail")
def ship_sail(ctx, r, s):
    ship_reconstruction(ctx, r, s, settle_after=60 + 30)
    last = server_at(ctx, 700)
    band(r, "speed on the server near the end (cm/s)", [last.fields["speed"]] if last else [], 800.0, 1100.0, "cm/s")
    first = server_at(ctx, 0)
    band(r, "travelled +X on the server (cm)", [last.fields["x"] - first.fields["x"]] if last and first else [], 3000.0, 20000.0, "cm")
    cost_sane(ctx, r)


def ship_turn_common(ctx, r, s):
    ship_reconstruction(ctx, r, s, settle_after=480 + 30)
    before, after = server_at(ctx, 240), server_at(ctx, 480)
    band(r, "heading change over four seconds of rudder (deg)",
         [yaw_gap(after.fields["yaw"], before.fields["yaw"])] if before and after else [], 30.0, 180.0, "deg")
    cost_sane(ctx, r)


@row("ship.turn")
def ship_turn(ctx, r, s):
    ship_turn_common(ctx, r, s)


ROWS["ship.turn-loss"] = ship_turn


@row("ship.stop")
def ship_stop(ctx, r, s):
    ship_reconstruction(ctx, r, s, settle_after=360 + 30)
    moving, stopped = server_at(ctx, 350), server_at(ctx, 480)
    band(r, "speed before the anchor (cm/s)", [moving.fields["speed"]] if moving else [], 300.0, 1100.0, "cm/s")
    band(r, "speed two seconds after the anchor (cm/s)", [stopped.fields["speed"]] if stopped else [], 0.0, 20.0, "cm/s")
    cost_sane(ctx, r)


# --- the deck assertions ----------------------------------------------------------

def based_poses(ctx, world, pid):
    """sf -> the POSE line of one pawn in one world when it stands on a base."""
    return dict((sf, ln) for sf, ln in poses(ctx, world, pid).items() if ln.fields.get("base") == 1.0)


INPUT_SETTLE_FRAMES = 18


def deck_relative(ctx, r, s, tolerance_cm=5.0, rendered_cm=50.0):
    """Each role's pawn in ship space on its client against the server at the same frame, second
    half, outside INPUT_SETTLE_FRAMES after the role's own input edges; based on every world
    throughout that half; the transient after an edge and the world-space error reported; the
    other client's view of the pawn, as its sync state holds it and as it renders it."""
    pids = role_pids(ctx)
    for role, (world, _loc, _yaw) in sorted(s["roles"].items()):
        pid = pids.get(role, -1)
        server_all, client_all = poses(ctx, "S", pid), poses(ctx, world, pid)
        matched = sorted(set(server_all) & set(client_all))
        later = matched[len(matched) // 2:]
        edges = [ln.frame for ln in ctx.lines("INPUT", world, "role=%s " % role)]
        after_edge = lambda f: any(0 <= f - e <= INPUT_SETTLE_FRAMES for e in edges)
        ship_err = lambda f: sum((server_all[f].fields.get(k, 0.0) - client_all[f].fields.get(k, 0.0)) ** 2 for k in ("bx", "by", "bz")) ** 0.5
        band(r, "%s POSE frames matched on S and %s" % (role, world), [len(later)], 10, 10 ** 6, "")
        equal(r, "%s based on S, second half" % role, [server_all[f].fields.get("base") for f in later], 1.0)
        equal(r, "%s based on %s, second half" % (role, world), [client_all[f].fields.get("base") for f in later], 1.0)
        both = [f for f in later if server_all[f].fields.get("base") == 1.0 and client_all[f].fields.get("base") == 1.0]
        band(r, "%s ship-space error S vs %s, second half (cm)" % (role, world), [ship_err(f) for f in both if not after_edge(f)], 0.0, tolerance_cm, "cm")
        edge_err = [ship_err(f) for f in both if after_edge(f)]
        r.add(True, "%s ship-space error within %d frames of an input edge" % (role, INPUT_SETTLE_FRAMES),
              "peak %.1f cm over %d sample(s)" % (max(edge_err) if edge_err else 0.0, len(edge_err)))
        world_err = [distance(server_all[f], client_all[f]) for f in later]
        r.add(True, "%s world-space error S vs %s, second half" % (role, world),
              "peak %.1f cm over %d sample(s)" % (max(world_err) if world_err else 0.0, len(world_err)))
        rollbacks = [ln.fields.get("n", 0.0) for ln in ctx.lines("ROLLBACK", world) if int(ln.fields.get("pid", -1)) == pid]
        r.add(True, "%s rollbacks on %s" % (role, world), "%d" % int(max(rollbacks) if rollbacks else 0))
        for other, (other_world, _l, _y) in sorted(s["roles"].items()):
            if other == role:
                continue
            seen = poses(ctx, other_world, pid)
            seen_later = [f for f in sorted(set(server_all) & set(seen)) if f >= (later[0] if later else 0)]
            seen_deck = [sum((server_all[f].fields.get(k, 0.0) - seen[f].fields.get(k, 0.0)) ** 2 for k in ("bx", "by", "bz")) ** 0.5
                         for f in seen_later if server_all[f].fields.get("base") == 1.0 and seen[f].fields.get("base") == 1.0]
            band(r, "%s as %s sees it, ship-space error vs S, second half (cm)" % (role, other_world), seen_deck, 0.0, 50.0, "cm")
            rendered = [sum((server_all[f].fields.get(k, 0.0) - seen[f].fields.get(rk, 0.0)) ** 2 for k, rk in (("bx", "rx"), ("by", "ry"), ("bz", "rz"))) ** 0.5
                        for f in seen_later if server_all[f].fields.get("base") == 1.0 and seen[f].fields.get("base") == 1.0]
            band(r, "%s as %s renders it, ship-space error vs S, second half (cm)" % (role, other_world), rendered, 0.0, rendered_cm, "cm")


def ship_turned(ctx, r):
    before, after = server_at(ctx, 300), server_at(ctx, 700)
    band(r, "the ship turned under the pawns (deg)", [yaw_gap(after.fields["yaw"], before.fields["yaw"])] if before and after else [], 15.0, 180.0, "deg")


@row("deck.stand")
def deck_stand(ctx, r, s):
    deck_relative(ctx, r, s, rendered_cm=10.0)
    pids = role_pids(ctx)
    for role in sorted(s["roles"]):
        based = sorted(based_poses(ctx, "S", pids.get(role, -1)).items())
        later = based[len(based) // 2:]
        for k in ("bx", "by"):
            vals = [ln.fields[k] for _sf, ln in later]
            band(r, "%s creep on the deck, %s spread on S (cm)" % (role, k), [max(vals) - min(vals)] if vals else [], 0.0, 50.0, "cm")
    ship_turned(ctx, r)
    cost_sane(ctx, r)


@row("deck.walk")
def deck_walk(ctx, r, s):
    deck_relative(ctx, r, s)
    pids = role_pids(ctx)
    based = sorted(based_poses(ctx, "S", pids.get("p1", -1)).items())
    start = begin_frame(ctx)
    before = [ln for sf, ln in based if sf <= start + 420]
    after = [ln for sf, ln in based if sf >= start + 560]
    if before and after:
        moved = ((after[0].fields["bx"] - before[-1].fields["bx"]) ** 2 + (after[0].fields["by"] - before[-1].fields["by"]) ** 2) ** 0.5
        band(r, "p1 walked across the deck on the server (cm)", [moved], 300.0, 1500.0, "cm")
    else:
        r.add(False, "p1 walked across the deck on the server (cm)", "no based POSE before and after the walk")
    count(r, "p1 INPUT edges on C1", len(ctx.lines("INPUT", "C1", "role=p1 ")), 2)
    ship_turned(ctx, r)
    cost_sane(ctx, r)


@row("deck.station")
def deck_station(ctx, r, s):
    count(r, "wheel calls refused by distance", len(ctx.lines("SHIPNO", "S", "input=wheel")), 1)
    count(r, "wheel calls applied from the wheel", len(ctx.lines("SHIPIN", "S", "input=wheel")), 2)
    cost_sane(ctx, r)


@row("deck.swim")
def deck_swim(ctx, r, s):
    pids = role_pids(ctx)
    pid = pids.get("p1", -1)
    start = begin_frame(ctx)
    server = poses(ctx, "S", pid)
    swimming = sorted(sf for sf, ln in server.items() if ln.fields.get("mode") == "Swimming")
    band(r, "p1 swims on the server within a second (frames after the drop)",
         [swimming[0] - start] if swimming else [], 0, 120, "f")
    settled = [sf for sf in swimming if sf >= swimming[0] + 60] if swimming else []
    afloat = [abs(server[sf].fields["z"] - server[sf].fields["wz"] - 40.0) for sf in settled if "wz" in server[sf].fields]
    band(r, "p1 floats at the surface a second into swimming (cm off)", afloat, 0.0, 30.0, "cm")
    rise = [abs(server[sf].fields["z"] - server[sf].fields["wz"] - 40.0) for sf in swimming[:10] if "wz" in server[sf].fields]
    r.add(True, "p1 rising to the surface, first second", "peak %.1f cm off over %d sample(s)" % (max(rise) if rise else 0.0, len(rise)))
    back = sorted(sf for sf, ln in server.items() if sf >= start + 360 and ln.fields.get("base") == 1.0)
    band(r, "p1 back on the deck within a second of the ladder (frames after the call)",
         [back[0] - (start + 360)] if back else [], 0, 60, "f")
    client = poses(ctx, "C1", pid)
    later = [sf for sf in sorted(set(server) & set(client)) if sf >= start + 450]
    deck = [sum((server[f].fields.get(k, 0.0) - client[f].fields.get(k, 0.0)) ** 2 for k in ("bx", "by", "bz")) ** 0.5
            for f in later if server[f].fields.get("base") == 1.0 and client[f].fields.get("base") == 1.0]
    band(r, "p1 ship-space error S vs C1 after the ladder (cm)", deck, 0.0, 5.0, "cm")
    swim_err = [distance(server[f], client[f]) for f in sorted(set(server) & set(client)) if server[f].fields.get("mode") == "Swimming"]
    r.add(True, "p1 world-space error S vs C1 while swimming", "peak %.1f cm over %d sample(s)" % (max(swim_err) if swim_err else 0.0, len(swim_err)))
    cost_sane(ctx, r)


# --- the ocean assertions -----------------------------------------------------------

@row("ocean.agree")
def ocean_agree(ctx, r, s):
    server = dict((int(ln.fields["sf"]), ln) for ln in ctx.lines("OCEAN", "S") if "sf" in ln.fields)
    band(r, "OCEAN lines on S", [len(server)], 4, 10 ** 6, "")
    equal(r, "sea state 1 on S", [round(ln.fields.get("sea", -1.0), 2) for ln in server.values()], 1.0)
    band(r, "waves exist on S, largest |h| (cm)",
         [max([abs(ln.fields["h%d" % k]) for ln in server.values() for k in range(4)] or [0.0])], 20.0, 10 ** 6, "cm")
    band(r, "inversion residual on S (cm)", [ln.fields.get("inv", 10 ** 6) for ln in server.values()], 0.0, 1.0, "cm")
    for world in s["worlds"]:
        if world == "S":
            continue
        client = dict((int(ln.fields["sf"]), ln) for ln in ctx.lines("OCEAN", world) if "sf" in ln.fields)
        matched = sorted(set(server) & set(client))
        band(r, "%s OCEAN frames matched with S" % world, [len(matched)], 4, 10 ** 6, "")
        equal(r, "sea state 1 on %s" % world, [round(ln.fields.get("sea", -1.0), 2) for ln in client.values()], 1.0)
        errors = [abs(server[f].fields["h%d" % k] - client[f].fields.get("g%d" % k, 10 ** 6))
                  for f in matched for k in range(4)]
        band(r, "S CPU vs %s GPU at four points (cm)" % world, errors, 0.0, 1.0, "cm")
        band(r, "%s GPU vs its own CPU over the grid, max (cm)" % world,
             [ln.fields.get("gpu_max", 10 ** 6) for ln in client.values()], 0.0, 1.0, "cm")
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
