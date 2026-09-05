"""The fixture authority: one entry per regression scenario.

Imported by ue_regression_runner.py inside the editor and by regression_run.py outside it, so
nothing here imports `unreal`.

A scenario entry:

    "smoke-two-clients": dict(
        family="harness", covers=["two worlds", "cost"],
        worlds=("S", "C1", "C2"),                  # the server and every client the row drives
        latencies=(0, 50, 100, 150),               # round trips in ms; zero is required
        loss=0.0,                                  # packet loss, percent, both directions
        roles=dict(p1=("C1", (0.0, 0.0, 100.0), 0.0),      # role -> (client world, spawn, yaw)
                   p2=("C2", (200.0, 0.0, 100.0), 180.0)),
        cvars={},                                  # console variables set before play
        plan=[(0, "p1", "tap", "jump"), (60, "p2", "move", 1.0, 0.0, 30)],   # (frame, role, op, *args)
        stop=dict(duration=10.0),                  # server game seconds; or until=("TAG", n) + timeout
        mutations=[("drop", "COST", 1)],           # each must turn the row red
        golden=dict(keep=["x", "y"]),              # fields the skeleton keeps beside frame, world, tag
        allow=[],                                  # engine-warning substrings this row tolerates
    )

Plan frames are sixtieths of server game time from the row's start. A row runs once per latency
it lists, as `<id>@<ms>`; the round trip is split evenly between the two directions.
"""

# --- vocabulary ---------------------------------------------------------------

# Actions a plan may name. The runner resolves each to its key from the mapping contexts in
# IMC_PATHS at arm time; an action with no key fails validation there.
ACTIONS = ("jump", "move", "attack", "parry", "feint", "use")
IMC_PATHS = ()

# Plan ops the runner implements. tap/press/release/hold/move/stop_move drive a role's keys in its
# own client world; face turns its control rotation; teleport moves its server pawn; mark writes a
# MARK line.
OPS = ("tap", "press", "release", "hold", "move", "stop_move", "face", "teleport", "mark")

# The mechanics a row may claim to cover. The coverage map in Docs/Debug-Instruments.md is
# generated from these, so a claim outside the list fails at load rather than drifting.
MECHANICS = ("two worlds", "cost", "injection latency", "determinism")

# Rung order, which is the order the matrix lists families in.
FAMILIES = ("harness", "ocean", "ship", "deck", "melee", "ship-combat", "ship-to-ship")

# The harness floor's half-extent; a placement beyond it fails before PIE.
FLOOR_LIMIT = 5000.0

SCENARIOS = {}


# --- validation ---------------------------------------------------------------

def validate(resolve_action=None):
    """Every check that can fail before PIE. Returns a list of problems, empty when sound.
    resolve_action comes from the runner inside the editor and is skipped outside it."""
    problems = []
    for sid, s in sorted(SCENARIOS.items()):
        where = "%s: " % sid
        for key in ("family", "covers", "worlds", "latencies", "roles", "plan", "stop", "mutations"):
            if key not in s:
                problems.append(where + "missing '%s'" % key)
        if s.get("family") not in FAMILIES:
            problems.append(where + "family %r is not a rung" % (s.get("family"),))
        if not s.get("covers"):
            problems.append(where + "covers nothing -- name the mechanics the row asserts")
        for c in s.get("covers", ()):
            if c not in MECHANICS:
                problems.append(where + "covers '%s', which is not in MECHANICS" % c)
        if not s.get("mutations"):
            problems.append(where + "no mutation -- every row carries one that must turn it red")

        worlds = tuple(s.get("worlds", ()))
        clients = [w for w in worlds if w != "S"]
        if "S" not in worlds or not clients:
            problems.append(where + "worlds must name S and at least one client")
        for w in clients:
            if not (w.startswith("C") and w[1:].isdigit() and int(w[1:]) >= 1):
                problems.append(where + "world %r is not S or C<n>" % (w,))

        lats = tuple(s.get("latencies", ()))
        if 0 not in lats:
            problems.append(where + "latencies must include 0")
        if any((not isinstance(l, int)) or l < 0 for l in lats):
            problems.append(where + "a latency is not a non-negative integer of ms")
        if not (0.0 <= float(s.get("loss", 0.0)) < 100.0):
            problems.append(where + "loss is a percentage below 100")

        for role, placement in s.get("roles", {}).items():
            world, loc, _yaw = placement
            if world not in clients:
                problems.append(where + "role %s is in %r, which is not a client the row names" % (role, world))
            if max(abs(loc[0]), abs(loc[1])) > FLOOR_LIMIT:
                problems.append(where + "%s placed off the floor at %s" % (role, (loc[0], loc[1])))
        if len(set(p[0] for p in s.get("roles", {}).values())) != len(s.get("roles", {})):
            problems.append(where + "two roles share a client world")

        for step in s.get("plan", []):
            frame, role, op = step[0], step[1], step[2]
            if not isinstance(frame, int) or frame < 0:
                problems.append(where + "plan frame %r is not a frame" % (frame,))
            if op not in OPS:
                problems.append(where + "unknown op '%s'" % op)
            if role not in s.get("roles", {}):
                problems.append(where + "op %s names %s, which is not a role" % (op, role))
            if op in ("tap", "press", "release", "hold"):
                action = step[3]
                if action not in ACTIONS:
                    problems.append(where + "unknown action '%s'" % action)
                elif resolve_action is not None and not resolve_action(action):
                    problems.append(where + "action '%s' resolves to no key" % action)
                if op == "hold" and (len(step) < 5 or not isinstance(step[4], int) or step[4] <= 0):
                    problems.append(where + "hold needs a positive frame count")

        stop = s.get("stop", {})
        if "duration" not in stop and "until" not in stop:
            problems.append(where + "stop names neither a duration nor an until")
        if "until" in stop:
            u = stop["until"]
            if not (isinstance(u, tuple) and len(u) == 2 and isinstance(u[0], str)
                    and isinstance(u[1], int) and u[1] > 0):
                problems.append(where + "until must be (tag, n)")
            if "timeout" not in stop:
                problems.append(where + "an until needs a timeout")
        for m in s.get("mutations", []):
            if not (isinstance(m, tuple) and m and m[0] in ("shift", "drop", "dup", "set", "regex")):
                problems.append(where + "mutation %r is not shift, drop, dup, set or regex" % (m,))
    return problems


def by_family(family):
    return sorted(k for k, v in SCENARIOS.items() if v["family"] == family)


def runs_for(ids):
    """Every (scenario, latency) pair the ids expand to, in matrix order."""
    return [(sid, ms) for sid in ids for ms in SCENARIOS[sid]["latencies"]]


def run_id(sid, ms):
    return "%s@%d" % (sid, ms)


def split_run_id(rid):
    sid, _, ms = rid.rpartition("@")
    return sid, int(ms)
