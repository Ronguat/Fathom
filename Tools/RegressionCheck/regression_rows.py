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
        self.path = slice_path
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


def sail_agreement(ctx, r, s, first, last, label):
    """Each client's sail against the server's at the same frame from first to last: a station
    call takes effect a delay after its command, so every world has it before its frame and none
    corrects for it."""
    server = ship_lines(ctx, "S")
    for world in s["worlds"]:
        if world == "S":
            continue
        client = ship_lines(ctx, world)
        window = [f for f in sorted(set(server) & set(client)) if first <= f <= last]
        band(r, "%s sail against S %s (fraction)" % (world, label),
             [abs(server[f].fields["sail"] - client[f].fields["sail"]) for f in window], 0.0, 0.005, "")
        arrivals = [ln for ln in ctx.lines("SHIPREP", world) if "frame" in ln.fields and first <= ln.fields["frame"] <= last]
        late = [ln for ln in arrivals if ln.fields["frame"] < ln.fields["sf"]]
        r.add(bool(arrivals) and not late, "%s station inputs %s arrived before their frame, none corrected for" % (world, label),
              "%d arrived, %d late by %s frames" % (len(arrivals), len(late), [int(ln.fields["sf"] - ln.fields["frame"]) for ln in late[:3]]))


def server_from(ctx, frame):
    """The server's SHIP line at or after a frame."""
    server = ship_lines(ctx, "S")
    for f in sorted(server):
        if frame is not None and f >= frame:
            return server[f]
    return None


def server_at(ctx, rel_frame):
    """The server's SHIP line at or after a frame counted from the row's start."""
    return server_from(ctx, begin_frame(ctx) + rel_frame)


def effect_frame(ctx, station, nth=0):
    """The frame the nth call of a station the server applied takes effect, from its SHIPIN line:
    a row times a station's consequences from there, not from the plan's frame, the call delayed
    by the session's station delay."""
    calls = [ln for ln in ctx.lines("SHIPIN", "S", "input=%s" % station) if "sf" in ln.fields]
    return int(calls[nth].fields["sf"]) if len(calls) > nth else None


@row("ship.sail")
def ship_sail(ctx, r, s):
    ship_reconstruction(ctx, r, s, settle_after=60 + 30)
    start = begin_frame(ctx)
    sail_agreement(ctx, r, s, start + 60, start + 240, "through the three seconds after the press")
    last = server_at(ctx, 700)
    band(r, "speed on the server near the end (cm/s)", [last.fields["speed"]] if last else [], 800.0, 1100.0, "cm/s")
    first = server_at(ctx, 0)
    band(r, "travelled +X on the server (cm)", [last.fields["x"] - first.fields["x"]] if last and first else [], 3000.0, 20000.0, "cm")
    cost_sane(ctx, r)


def ship_turn_common(ctx, r, s):
    ship_reconstruction(ctx, r, s, settle_after=480 + 30)
    turn = effect_frame(ctx, "wheel")
    before, after = server_from(ctx, turn), server_from(ctx, turn + 240 if turn is not None else None)
    band(r, "heading change over four seconds from the rudder's effect (deg)",
         [yaw_gap(after.fields["yaw"], before.fields["yaw"])] if before and after else [], 30.0, 180.0, "deg")
    cost_sane(ctx, r)


@row("ship.turn")
def ship_turn(ctx, r, s):
    ship_turn_common(ctx, r, s)


ROWS["ship.turn-loss"] = ship_turn


@row("ship.stop")
def ship_stop(ctx, r, s):
    ship_reconstruction(ctx, r, s, settle_after=360 + 30)
    moving = server_at(ctx, 350)
    band(r, "speed before the anchor (cm/s)", [moving.fields["speed"]] if moving else [], 300.0, 1100.0, "cm/s")
    server = ship_lines(ctx, "S")
    dropped = effect_frame(ctx, "anchor")
    bites = [f for f in sorted(server) if dropped is not None and f >= dropped and server[f].fields.get("anchor") == 0.0]
    bite = bites[0] if bites else None
    band(r, "the anchor bites after its fall (frames after the call's effect)", [bite - dropped] if bite else [], 90, 180, "f")
    settled = server_from(ctx, bite + 240) if bite else None
    band(r, "speed four seconds after the bite (cm/s)", [abs(settled.fields["speed"])] if settled else [], 0.0, 20.0, "cm/s")
    if bite and settled:
        run = ((settled.fields["x"] - server[bite].fields["x"]) ** 2 + (settled.fields["y"] - server[bite].fields["y"]) ** 2) ** 0.5
        band(r, "the ship ran to the line's end and caught (cm past the bite)", [run], 300.0, 1500.0, "cm")
    else:
        r.add(False, "the ship ran to the line's end and caught (cm past the bite)", "no SHIP line at the bite or after")
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
    server = ship_lines(ctx, "S")
    before = server_from(ctx, effect_frame(ctx, "wheel"))
    after = server[max(server)] if server else None
    band(r, "the ship turned under the pawns, from the rudder's effect to the row's end (deg)",
         [yaw_gap(after.fields["yaw"], before.fields["yaw"])] if before and after else [], 15.0, 180.0, "deg")


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


@row("deck.station-key")
def deck_station_key(ctx, r, s):
    pids = role_pids(ctx)
    start = begin_frame(ctx)
    count(r, "p1 BOARD on the server", len(ctx.lines("BOARD", "S")), 1)
    based = [sf for sf in based_poses(ctx, "S", pids.get("p1", -1)) if sf >= start + 200]
    band(r, "p1 based POSE lines on S after boarding", [len(based)], 10, 10 ** 6, "")
    sail = ctx.lines("SHIPIN", "S", "input=sail_length")
    count(r, "sail calls applied by key at the mast, the hold's press and release", len(sail), 2)
    band(r, "sail set by the release, where it stood (fraction)", [sail[-1].fields.get("value", 0.0)] if sail else [], 0.9, 1.0, "")
    band(r, "the sail calls' delay after their commands (frames)", [ln.fields["sf"] - ln.fields["cmd"] for ln in sail if "cmd" in ln.fields], 8, 60, "f")
    if sail:
        sail_agreement(ctx, r, s, int(sail[0].fields["cmd"]), int(sail[-1].fields["sf"]) + 60, "through the hold and a second past the release")
    count(r, "wheel calls refused by distance, the tap's press and release", len(ctx.lines("SHIPNO", "S", "input=wheel")), 2)
    applied = ctx.lines("SHIPIN", "S", "input=wheel")
    count(r, "wheel calls applied by key from the wheel, a tap and a hold", len(applied), 4)
    values = [ln.fields.get("value") for ln in applied]
    r.add(len(values) == 4 and values[0] == -1.0 and -0.2 <= values[1] <= -0.01,
          "a two-frame tap keeps its effect: press left, then held a little left", "%s" % (values[:2],))
    r.add(len(values) == 4 and values[2] == 1.0 and values[3] >= 0.9, "the hold: press right, then held where it stood", "%s" % (values[2:],))
    held = effect_frame(ctx, "wheel", nth=2)
    before, after = server_from(ctx, held), server_from(ctx, held + 240 if held is not None else None)
    band(r, "heading change under the held key, from its effect (deg)",
         [yaw_gap(after.fields["yaw"], before.fields["yaw"])] if before and after else [], 15.0, 180.0, "deg")
    cost_sane(ctx, r)


@row("deck.jump")
def deck_jump(ctx, r, s):
    deck_relative(ctx, r, s)
    pids = role_pids(ctx)
    pid = pids.get("p1", -1)
    start = begin_frame(ctx)
    server = poses(ctx, "S", pid)
    airborne = sorted(sf for sf, ln in server.items() if sf >= start + 420 and ln.fields.get("mode") == "Falling")
    band(r, "p1 airborne on the server within a second of the jump (frames after the press)",
         [airborne[0] - (start + 420)] if airborne else [], 0, 60, "f")
    band(r, "p1 in the air (POSE lines, one per six frames)", [len(airborne)], 2, 20, "")
    count(r, "p1 kept the deck as its base while airborne", sum(1 for sf in airborne if server[sf].fields.get("base") == 1.0), len(airborne))
    before = [ln for sf, ln in sorted(server.items()) if sf < start + 420 and ln.fields.get("base") == 1.0]
    landed = [ln for sf, ln in sorted(server.items()) if airborne and sf > airborne[-1] and ln.fields.get("mode") == "Walking" and ln.fields.get("base") == 1.0]
    if before and landed:
        moved = ((landed[0].fields["bx"] - before[-1].fields["bx"]) ** 2 + (landed[0].fields["by"] - before[-1].fields["by"]) ** 2) ** 0.5
        band(r, "p1 landed where it jumped, in the deck's frame (cm)", [moved], 0.0, 100.0, "cm")
    else:
        r.add(False, "p1 landed where it jumped, in the deck's frame (cm)", "no based POSE before the jump or after the landing")
    ship_turned(ctx, r)
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


# --- the melee assertions ---------------------------------------------------------

def combat(ctx, world, pid):
    """One pawn's COMBAT lines in one world, in order."""
    return [ln for ln in ctx.lines("COMBAT", world) if int(ln.fields.get("pid", -1)) == pid]


def phase_key(lines):
    return [(ln.fields.get("phase"), ln.frame, int(ln.fields.get("start", -1)), ln.fields.get("attack")) for ln in lines]


def phase_text(lines):
    return " ".join("%s@%d" % (ln.fields.get("phase"), ln.frame) for ln in lines) or "none"


def rollbacks_reported(ctx, r, world, pid, role):
    n = [ln.fields.get("n", 0.0) for ln in ctx.lines("ROLLBACK", world) if int(ln.fields.get("pid", -1)) == pid]
    r.add(True, "%s rollbacks on %s" % (role, world), "%d" % int(max(n) if n else 0))


def command_frame(press):
    """The frame a press's command runs at: an INPUT line stamps the pending frame, and the
    command it authors runs at the next."""
    return press.frame + 1


@row("melee.swing")
def melee_swing(ctx, r, s):
    pid = role_pids(ctx).get("p1", -1)
    press = ctx.first("INPUT", "C1", "action=attack_overhead edge=pressed")
    server, client, other = combat(ctx, "S", pid), combat(ctx, "C1", pid), combat(ctx, "C2", pid)
    want = ["windup", "release", "recovery", "idle"]
    r.add([ln.fields.get("phase") for ln in server] == want, "p1 phases on S", phase_text(server))
    starts = [int(ln.fields.get("start", -1)) for ln in server[:3]]
    r.add(bool(press) and bool(starts) and all(st == command_frame(press) for st in starts),
          "p1 attack start on S is the press's command frame", "press %s, start %s" % (press.frame if press else "none", starts))
    band(r, "p1 windup on S (frames)", [server[1].frame - server[0].frame] if len(server) > 1 else [], 20, 60, "f")
    band(r, "p1 release on S (frames)", [server[2].frame - server[1].frame] if len(server) > 2 else [], 5, 40, "f")
    band(r, "p1 attack length on S (frames)", [server[3].frame - server[0].frame] if len(server) > 3 else [], 60, 160, "f")
    r.add(bool(server) and phase_key(client) == phase_key(server), "p1 phases on C1 match S frame for frame", phase_text(client))
    r.add([ln.fields.get("phase") for ln in other] == want and [int(ln.fields.get("start", -1)) for ln in other[:3]] == starts,
          "p1 phases as C2 sees them, in order with the same start", phase_text(other))
    r.add(True, "C2 sees each phase after S (frames)", "%s" % [ln.frame - srv.frame for ln, srv in zip(other, server)])
    rollbacks_reported(ctx, r, "C1", pid, "p1")
    cost_sane(ctx, r)


@row("melee.feint")
def melee_feint(ctx, r, s):
    pid = role_pids(ctx).get("p1", -1)
    feint = ctx.first("INPUT", "C1", "action=feint edge=pressed")
    for world in s["worlds"]:
        lines = combat(ctx, world, pid)
        r.add([ln.fields.get("phase") for ln in lines] == ["windup", "idle"], "p1 feinted in windup on %s" % world, phase_text(lines))
    server = combat(ctx, "S", pid)
    band(r, "p1 idle after the feint on S (frames after the press)",
         [server[1].frame - feint.frame] if feint and len(server) > 1 else [], 0, 1, "f")
    swings, hits = len(ctx.lines("SWING", "S")), len(ctx.lines("HIT", "S"))
    r.add(swings == 0 and hits == 0, "no swing on S", "%d SWING, %d HIT" % (swings, hits))
    rollbacks_reported(ctx, r, "C1", pid, "p1")
    cost_sane(ctx, r)


@row("melee.feint-loss")
def melee_feint_loss(ctx, r, s):
    pid = role_pids(ctx).get("p1", -1)
    server, client = combat(ctx, "S", pid), combat(ctx, "C1", pid)
    seen = lambda lines: set((ln.fields.get("phase"), int(ln.fields.get("start", -1)), ln.fields.get("attack")) for ln in lines)
    r.add(bool(server) and seen(server) <= seen(client) and phase_key(server)[-1][::2] == phase_key(client)[-1][::2],
          "p1 on C1 saw every state S held and ends in the same one", "S %s; C1 %s" % (phase_text(server), phase_text(client)))
    took = "release" not in [ln.fields.get("phase") for ln in server]
    r.add(True, "the feint took on S", "yes" if took else "no, the swing went through")
    rollbacks_reported(ctx, r, "C1", pid, "p1")
    cost_sane(ctx, r)


@row("melee.direction")
def melee_direction(ctx, r, s):
    pid = role_pids(ctx).get("p1", -1)
    for world in s["worlds"]:
        names = [ln.fields.get("attack") for ln in combat(ctx, world, pid) if ln.fields.get("phase") == "windup"]
        r.add(names == ["horizontal_l", "horizontal_r"], "p1 sides from the last turn on %s" % world, " ".join(names) or "none")
    cost_sane(ctx, r)


def hits(ctx, attacker, target):
    return [ln for ln in ctx.lines("HIT", "S")
            if int(ln.fields.get("pid", -1)) == attacker and int(ln.fields.get("target", -1)) == target]


def scores_agree(ctx, r, s):
    """The last SCORE per pawn on every world reads the same tallies as the server's; a pawn whose
    tallies never changed has none anywhere."""
    pids = role_pids(ctx)
    for role in sorted(s["roles"]):
        pid = pids.get(role, -1)
        last = {}
        for world in s["worlds"]:
            lines = [ln for ln in ctx.lines("SCORE", world) if int(ln.fields.get("pid", -1)) == pid]
            last[world] = (lines[-1].fields.get("taken"), lines[-1].fields.get("dealt"), lines[-1].fields.get("parries")) if lines else None
        r.add(all(v == last["S"] for v in last.values()),
              "%s tallies agree on every world" % role,
              "no tallies" if last["S"] is None else " ".join("%s=%s" % (w, v) for w, v in sorted(last.items())))


def reference_hit(ctx, s):
    """The 0 ms row's HIT line from the same run, when this is a later latency of it."""
    if ctx.latency == 0:
        return None
    base = os.path.basename(ctx.path).split("@")[0]
    ref = os.path.join(os.path.dirname(ctx.path), "%s@0.slice.log" % base)
    if not os.path.exists(ref):
        return None
    pids = role_pids(ctx)
    other = Context(ref)
    lines = hits(other, role_pids(other).get("p1", -1), role_pids(other).get("p2", -1))
    press = other.first("INPUT", "C1", "edge=pressed")
    return (lines[0], press) if lines and press else None


def still_target(ctx, r, s, contact_cm):
    """One hit on a standing target: the rewound body's ship-space centre against the 0 ms row's,
    and the contact point too when contact_cm is set, which a calm sea allows."""
    pids = role_pids(ctx)
    p1, p2 = pids.get("p1", -1), pids.get("p2", -1)
    lines = hits(ctx, p1, p2)
    count(r, "p1 hits p2 on S", len(lines), 1)
    press = ctx.first("INPUT", "C1", "action=attack_overhead edge=pressed")
    if lines and press:
        hit = lines[0]
        band(r, "contact within the body's radius of the rewound centre (cm)",
             [((hit.fields["x"] - hit.fields["tx"]) ** 2 + (hit.fields["y"] - hit.fields["ty"]) ** 2) ** 0.5], 0.0, 35.0, "cm")
        r.add(True, "hit frame after the press, rewind depth, body moved since, part",
              "%d f, %d f, %.1f cm, %s" % (hit.frame - press.frame, hit.frame - int(hit.fields["rf"]), hit.fields.get("moved", -1.0), hit.fields.get("part")))
        ref = reference_hit(ctx, s)
        if ref:
            ref_hit, ref_press = ref
            band(r, "rewound body's ship-space point against the 0 ms row (cm)",
                 [sum((hit.fields[k] - ref_hit.fields[k]) ** 2 for k in ("tx", "ty", "tz")) ** 0.5], 0.0, 10.0, "cm")
            contact = sum((hit.fields[k] - ref_hit.fields[k]) ** 2 for k in ("x", "y", "z")) ** 0.5
            if contact_cm is None:
                r.add(True, "contact point against the 0 ms row (cm)", "%.1f cm, the deck's roll moves it" % contact)
            else:
                band(r, "contact point against the 0 ms row (cm)", [contact], 0.0, contact_cm, "cm")
                equal(r, "hit frame after the press, as at 0 ms", [hit.frame - press.frame], ref_hit.frame - ref_press.frame)
        else:
            r.add(True, "rewound body's ship-space point against the 0 ms row (cm)",
                  "this is the 0 ms row" if ctx.latency == 0 else "no 0 ms row in this run")
    scores_agree(ctx, r, s)
    cost_sane(ctx, r)


@row("melee.hit")
def melee_hit(ctx, r, s):
    still_target(ctx, r, s, contact_cm=None)


@row("melee.hit-calm")
def melee_hit_calm(ctx, r, s):
    still_target(ctx, r, s, contact_cm=10.0)


def rendered_at(ctx, world, pid, frame):
    """A pawn's rendered ship-space position in one world at the frame its POSE line labels, or a
    straight line between the two labels around it."""
    seen = poses(ctx, world, pid)
    if frame in seen:
        ln = seen[frame]
        return [ln.fields[k] for k in ("rx", "ry", "rz")]
    before = [f for f in seen if f < frame]
    after = [f for f in seen if f > frame]
    if not before or not after:
        return None
    fa, fb = max(before), min(after)
    a, b = seen[fa], seen[fb]
    t = (frame - fa) / float(fb - fa)
    return [a.fields[k] + (b.fields[k] - a.fields[k]) * t for k in ("rx", "ry", "rz")]


@row("melee.hit-walk")
def melee_hit_walk(ctx, r, s):
    pids = role_pids(ctx)
    p1, p2 = pids.get("p1", -1), pids.get("p2", -1)
    lines = hits(ctx, p1, p2)
    count(r, "p1 hits the walking p2 on S", len(lines), 1)
    press = ctx.first("INPUT", "C1", "action=attack_thrust edge=pressed")
    if lines and press:
        hit = lines[0]
        rf = int(hit.fields["rf"])
        seen = rendered_at(ctx, "C1", p2, rf)
        err = [sum((hit.fields[k] - seen[i]) ** 2 for i, k in enumerate(("tx", "ty", "tz"))) ** 0.5] if seen else []
        band(r, "rewound body against what C1 rendered at that frame (cm)", err, 0.0, 10.0, "cm")
        r.add(True, "hit frame after the press, rewind depth, body moved since",
              "%d f, %d f, %.1f cm" % (hit.frame - press.frame, hit.frame - rf, hit.fields.get("moved", -1.0)))
    scores_agree(ctx, r, s)
    cost_sane(ctx, r)


def parries(ctx, attacker, target):
    return [ln for ln in ctx.lines("PARRY", "S")
            if int(ln.fields.get("pid", -1)) == attacker and int(ln.fields.get("target", -1)) == target]


@row("melee.parry")
def melee_parry(ctx, r, s):
    pids = role_pids(ctx)
    p1, p2 = pids.get("p1", -1), pids.get("p2", -1)
    lines = parries(ctx, p1, p2)
    count(r, "p1 parried by p2 on S", len(lines), 1)
    r.add(not hits(ctx, p1, p2), "no hit on S", "%d HIT" % len(hits(ctx, p1, p2)))
    press = ctx.first("INPUT", "C2", "action=parry edge=pressed")
    if lines and press:
        r.add(True, "contact after the parry press, rewind depth, frames left in the window at the rewound frame",
              "%d f, %d f, %d f" % (lines[0].frame - press.frame, lines[0].frame - int(lines[0].fields["rf"]), int(lines[0].fields.get("margin", -1))))
    scores_agree(ctx, r, s)
    cost_sane(ctx, r)


@row("melee.parry-late")
def melee_parry_late(ctx, r, s):
    pids = role_pids(ctx)
    p1, p2 = pids.get("p1", -1), pids.get("p2", -1)
    lines = hits(ctx, p1, p2)
    count(r, "p1 hits p2 on S through a late parry", len(lines), 1)
    r.add(not parries(ctx, p1, p2), "no parry on S", "%d PARRY" % len(parries(ctx, p1, p2)))
    press = ctx.first("INPUT", "C2", "action=parry edge=pressed")
    if lines and press:
        r.add(True, "contact against the parry press, window open at the rewound frame",
              "%d f, %d" % (lines[0].frame - press.frame, int(lines[0].fields.get("window", -1))))
    scores_agree(ctx, r, s)
    cost_sane(ctx, r)


def advance_report(ctx, r, s):
    """What one advance setting did: the attack's start against the press, the effective windup,
    the resolution's frame, the rewound body against the current one, the parry margin."""
    pids = role_pids(ctx)
    p1, p2 = pids.get("p1", -1), pids.get("p2", -1)
    press = ctx.first("INPUT", "C1", "edge=pressed")
    server = combat(ctx, "S", p1)
    starts = [int(ln.fields.get("start", -1)) for ln in server if ln.fields.get("phase") == "windup"]
    release = [ln for ln in server if ln.fields.get("phase") == "release"]
    resolved = hits(ctx, p1, p2) + parries(ctx, p1, p2)
    count(r, "the swing resolved on S, hit or parry", len(resolved), 1)
    if press and starts:
        r.add(True, "attack start ahead of the press's command frame on S (frames)", "%d" % (command_frame(press) - starts[0]))
    if press and release:
        r.add(True, "effective windup on S (frames after the press)", "%d" % (release[0].frame - press.frame))
    if press and resolved:
        ln = resolved[0]
        r.add(True, "%s on S: frames after the press, rewind depth, body moved since, margin" % ln.tag,
              "%d f, %d f, %s cm, %s f" % (ln.frame - press.frame, ln.frame - int(ln.fields["rf"]),
                                           ln.fields.get("moved", "-"), ln.fields.get("margin", "-")))
    client = combat(ctx, "C1", p1)
    client_starts = [int(ln.fields.get("start", -1)) for ln in client if ln.fields.get("phase") == "windup"]
    r.add(bool(starts) and client_starts[:1] == starts[:1], "C1 predicted the same start as S", "S %s, C1 %s" % (starts[:1], client_starts[:1]))
    rollbacks_reported(ctx, r, "C1", p1, "p1")
    scores_agree(ctx, r, s)
    cost_sane(ctx, r)


@row("melee.advance-half")
def melee_advance_half(ctx, r, s):
    advance_report(ctx, r, s)


@row("melee.advance-whole")
def melee_advance_whole(ctx, r, s):
    advance_report(ctx, r, s)


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
