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
        golden=dict(keep=["x", "y"]),              # optional: skeleton fields beside frame, world, tag
        allow=[],                                  # engine-warning substrings this row tolerates
        injection_tolerance=1,                     # optional: frames the injection pairing may move
        settle_frames=30,                          # optional: frames after the last applied input before a ship row asserts
        pose_every=6,                              # optional: frames between POSE lines on every simulated proxy
    )

Plan frames are sixtieths of server game time from the row's start. A row runs once per latency
it lists, as `<id>@<ms>`; the round trip is split evenly between the two directions.
"""

# --- vocabulary ---------------------------------------------------------------

# Actions a plan may name. The runner resolves each to its key from the controller's key table,
# KEY_TABLE_CLASS's ActionKeys, at arm time; an action with no key fails validation there.
ACTIONS = ("move_forward", "move_back", "move_left", "move_right", "jump", "mark",
           "attack_horizontal", "attack_overhead", "attack_thrust", "parry", "feint")
KEY_TABLE_CLASS = "FMPlayerController"

# What the move op holds for a direction: X right, Y forward.
MOVE_ACTIONS = dict(forward="move_forward", back="move_back", left="move_left", right="move_right")

# Plan ops the runner implements. tap/press/release/hold/move/stop_move drive a role's keys in its
# own client world; face turns its control rotation; teleport moves its server pawn; mark writes a
# MARK line.
OPS = ("tap", "press", "release", "hold", "move", "stop_move", "face", "teleport", "mark", "ship")

# The ship's stations, which the ship op drives from a role's client.
SHIP_INPUTS = ("wheel", "sail_length", "sail_angle", "anchor", "ladder")

# The mechanics a row may claim to cover. The coverage map in Docs/Debug-Instruments.md is
# generated from these, so a claim outside the list fails at load rather than drifting.
MECHANICS = ("two worlds", "cost", "injection latency", "determinism", "combat", "rewind", "parry", "advance")

# Rung order, which is the order the matrix lists families in.
FAMILIES = ("harness", "ocean", "ship", "deck", "melee", "ship-combat", "ship-to-ship")

# The ocean plane's half-extent; a placement beyond it fails before PIE.
FLOOR_LIMIT = 20000.0

# Every project console variable a row may set, with the value that reads the settings; the
# runner restores these before each row, so nothing a row sets reaches the next.
CVAR_DEFAULTS = {"fm.SeaState": "-1", "fm.WindAngle": "-1000", "fm.MeleeAdvanceFraction": "-1", "fm.MeleeAdvanceCapMs": "-1"}

# One mutation every harness row carries: the server's POSE positions zeroed, which the
# determinism assertion must catch.
POSE_SERVER_ZERO = ("regex", r"(\[S\] POSE pid=\d+ sf=\d+ x=)[-\d.]+", r"\g<1>0.00")
SHIP_SERVER_ZERO = ("regex", r"(\[S\] SHIP id=\d+ sf=\d+ x=)[-\d.]+", r"\g<1>0.00")

SCENARIOS = {
    "harness.idle": dict(
        family="harness", covers=["two worlds", "cost", "determinism"],
        worlds=("S", "C1", "C2"), latencies=(0, 50, 100, 150), loss=0.0,
        roles=dict(p1=("C1", (0.0, -200.0, 100.0), 0.0),
                   p2=("C2", (0.0, 200.0, 100.0), 180.0)),
        cvars={},
        plan=[(120, "p1", "mark", "idle")],
        stop=dict(duration=6.0),
        mutations=[("set", "COST", "tick_ms", "99.00"), POSE_SERVER_ZERO],
        allow=[],
    ),
    "harness.walk": dict(
        family="harness", covers=["two worlds", "cost", "injection latency", "determinism"],
        worlds=("S", "C1", "C2"), latencies=(0, 50, 100, 150), loss=0.0,
        roles=dict(p1=("C1", (-400.0, -200.0, 100.0), 0.0),
                   p2=("C2", (0.0, 200.0, 100.0), 180.0)),
        cvars={},
        plan=[(60, "p1", "move", 0.0, 1.0, 120)],
        stop=dict(duration=6.0),
        mutations=[("drop", "INPUT", 1), POSE_SERVER_ZERO],
        allow=[],
    ),
    "harness.walk-loss": dict(
        family="harness", covers=["two worlds", "cost", "injection latency", "determinism"],
        worlds=("S", "C1", "C2"), latencies=(0, 100), loss=5.0,
        roles=dict(p1=("C1", (-400.0, -200.0, 100.0), 0.0),
                   p2=("C2", (0.0, 200.0, 100.0), 180.0)),
        cvars={},
        plan=[(60, "p1", "move", 0.0, 1.0, 120)],
        stop=dict(duration=6.0),
        mutations=[("drop", "INPUT", 1), POSE_SERVER_ZERO],
        allow=[],
        injection_tolerance=3,
    ),
    "ship.sail": dict(
        family="ship", covers=["determinism", "cost"],
        worlds=("S", "C1", "C2"), latencies=(0, 50, 100, 150), loss=0.0,
        roles=dict(p1=("C1", (0.0, 15200.0, 320.0), 0.0),
                   p2=("C2", (-1000.0, 15000.0, 320.0), 180.0)),
        cvars={"fm.SeaState": "0.5", "fm.WindAngle": "0"},
        plan=[(60, "p1", "ship", "sail_length", 1.0)],
        stop=dict(duration=12.0),
        mutations=[SHIP_SERVER_ZERO, ("set", "SHIP", "speed", "0.00")],
        allow=[],
    ),
    "ship.turn": dict(
        family="ship", covers=["determinism", "cost"],
        worlds=("S", "C1", "C2"), latencies=(0, 50, 100, 150), loss=0.0,
        roles=dict(p1=("C1", (0.0, 15200.0, 320.0), 0.0),
                   p2=("C2", (-1000.0, 15000.0, 320.0), 180.0)),
        cvars={"fm.SeaState": "0.5", "fm.WindAngle": "0"},
        plan=[(60, "p1", "ship", "sail_length", 1.0), (240, "p2", "ship", "wheel", 1.0), (480, "p2", "ship", "wheel", 0.0)],
        stop=dict(duration=12.0),
        mutations=[SHIP_SERVER_ZERO, ("set", "SHIP", "yaw", "0.00")],
        allow=[],
    ),
    "ship.turn-loss": dict(
        family="ship", covers=["determinism", "cost"],
        worlds=("S", "C1", "C2"), latencies=(0, 100), loss=5.0,
        roles=dict(p1=("C1", (0.0, 15200.0, 320.0), 0.0),
                   p2=("C2", (-1000.0, 15000.0, 320.0), 180.0)),
        cvars={"fm.SeaState": "0.5", "fm.WindAngle": "0"},
        plan=[(60, "p1", "ship", "sail_length", 1.0), (240, "p2", "ship", "wheel", 1.0), (480, "p2", "ship", "wheel", 0.0)],
        stop=dict(duration=12.0),
        mutations=[SHIP_SERVER_ZERO, ("set", "SHIP", "yaw", "0.00")],
        allow=[],
        injection_tolerance=3,
        settle_frames=60,
    ),
    "ship.stop": dict(
        family="ship", covers=["determinism", "cost"],
        worlds=("S", "C1", "C2"), latencies=(0, 50, 100, 150), loss=0.0,
        roles=dict(p1=("C1", (0.0, 15200.0, 320.0), 0.0),
                   p2=("C2", (1000.0, 15000.0, 320.0), 180.0)),
        cvars={"fm.SeaState": "0.5", "fm.WindAngle": "0"},
        plan=[(60, "p1", "ship", "sail_length", 1.0), (360, "p2", "ship", "anchor", 1.0)],
        stop=dict(duration=10.0),
        mutations=[SHIP_SERVER_ZERO, ("set", "SHIP", "speed", "999.00")],
        allow=[],
    ),
    "deck.stand": dict(
        family="deck", covers=["determinism", "cost"],
        worlds=("S", "C1", "C2"), latencies=(0, 50, 100, 150), loss=0.0,
        roles=dict(p1=("C1", (0.0, 15200.0, 320.0), 0.0),
                   p2=("C2", (-1000.0, 15000.0, 320.0), 180.0)),
        cvars={"fm.SeaState": "1.0", "fm.WindAngle": "30"},
        plan=[(120, "p1", "ship", "sail_length", 1.0), (300, "p2", "ship", "wheel", 0.5)],
        stop=dict(duration=14.0),
        mutations=[("regex", r"(\[S\] POSE pid=\d+ sf=\d+ .*? bx=)[-\d.]+", r"\g<1>999.00"), ("set", "POSE", "base", "0"),
                   ("regex", r"(\[C1\] POSE pid=\d+ sf=\d+ .*? rx=)[-\d.]+", r"\g<1>999.00")],
        allow=[],
    ),
    "deck.walk": dict(
        family="deck", covers=["determinism", "cost", "injection latency"],
        worlds=("S", "C1", "C2"), latencies=(0, 50, 100, 150), loss=0.0, injection_tolerance=2,
        roles=dict(p1=("C1", (0.0, 15200.0, 320.0), 0.0),
                   p2=("C2", (-1000.0, 15000.0, 320.0), 180.0)),
        cvars={"fm.SeaState": "1.0", "fm.WindAngle": "30"},
        plan=[(120, "p1", "ship", "sail_length", 1.0), (300, "p2", "ship", "wheel", 0.5), (420, "p1", "move", 0.0, 1.0, 90)],
        stop=dict(duration=14.0),
        mutations=[("regex", r"(\[S\] POSE pid=\d+ sf=\d+ .*? bx=)[-\d.]+", r"\g<1>999.00"), ("drop", "INPUT", 1)],
        allow=[],
    ),
    "deck.station": dict(
        family="deck", covers=["two worlds", "cost"],
        worlds=("S", "C1", "C2"), latencies=(0, 100), loss=0.0,
        roles=dict(p1=("C1", (-1000.0, 15000.0, 320.0), 0.0),
                   p2=("C2", (1000.0, 15000.0, 320.0), 180.0)),
        cvars={"fm.SeaState": "0.5", "fm.WindAngle": "0"},
        plan=[(120, "p2", "ship", "wheel", 1.0), (180, "p1", "ship", "wheel", 1.0), (240, "p1", "ship", "wheel", 0.0)],
        stop=dict(duration=6.0),
        mutations=[("drop", "SHIPIN", 1), ("regex", r"SHIPNO", r"SHIPNIL")],
        allow=[],
    ),
    "deck.swim": dict(
        family="deck", covers=["determinism", "cost"],
        worlds=("S", "C1", "C2"), latencies=(0, 50, 100, 150), loss=0.0,
        roles=dict(p1=("C1", (0.0, 15700.0, 200.0), 90.0),
                   p2=("C2", (0.0, 15200.0, 320.0), 180.0)),
        cvars={"fm.SeaState": "0.5", "fm.WindAngle": "0"},
        plan=[(360, "p1", "ship", "ladder", 1.0)],
        stop=dict(duration=10.0),
        mutations=[("regex", r"(\[S\] POSE .*? mode=)Swimming", r"\g<1>Walking"), ("set", "POSE", "base", "0")],
        allow=[],
    ),
    "ocean.agree": dict(
        family="ocean", covers=["determinism", "cost"],
        worlds=("S", "C1", "C2"), latencies=(0, 50, 100, 150), loss=0.0,
        roles=dict(p1=("C1", (0.0, -200.0, 100.0), 0.0),
                   p2=("C2", (0.0, 200.0, 100.0), 180.0)),
        cvars={"fm.SeaState": "1.0", "fm.WindAngle": "30"},
        plan=[],
        stop=dict(duration=8.0),
        mutations=[("regex", r"(\[S\] OCEAN .*? h0=)[-\d.]+", r"\g<1>999.00"), ("set", "OCEAN", "gpu_max", "9.000")],
        allow=[],
    ),
    "harness.jump": dict(
        family="harness", covers=["two worlds", "cost", "injection latency", "determinism"],
        worlds=("S", "C1", "C2"), latencies=(0, 50, 100, 150), loss=0.0,
        roles=dict(p1=("C1", (0.0, -200.0, 100.0), 0.0),
                   p2=("C2", (0.0, 200.0, 100.0), 180.0)),
        cvars={},
        plan=[(60, "p1", "tap", "jump")],
        stop=dict(duration=6.0),
        mutations=[("drop", "INPUT", 1), POSE_SERVER_ZERO],
        allow=[],
    ),
    "melee.swing": dict(
        family="melee", covers=["combat", "cost"],
        worlds=("S", "C1", "C2"), latencies=(0, 50, 100, 150), loss=0.0,
        roles=dict(p1=("C1", (0.0, 15200.0, 320.0), 0.0),
                   p2=("C2", (-1000.0, 15000.0, 320.0), 180.0)),
        cvars={"fm.SeaState": "1.0", "fm.WindAngle": "30"},
        plan=[(120, "p1", "ship", "sail_length", 1.0), (300, "p2", "ship", "wheel", 0.5), (400, "p1", "face", 1.0), (420, "p1", "tap", "attack_overhead")],
        stop=dict(duration=12.0),
        mutations=[("drop", "COMBAT", 1), ("set", "COMBAT", "start", "0"), ("dup", "INPUT", 1)],
        allow=[],
    ),
    "melee.feint": dict(
        family="melee", covers=["combat", "cost"],
        worlds=("S", "C1", "C2"), latencies=(0, 50, 100, 150), loss=0.0,
        roles=dict(p1=("C1", (0.0, 15200.0, 320.0), 0.0),
                   p2=("C2", (-1000.0, 15000.0, 320.0), 180.0)),
        cvars={"fm.SeaState": "1.0", "fm.WindAngle": "30"},
        plan=[(120, "p1", "ship", "sail_length", 1.0), (300, "p2", "ship", "wheel", 0.5),
              (400, "p1", "face", 1.0), (420, "p1", "tap", "attack_horizontal"), (430, "p1", "tap", "feint")],
        stop=dict(duration=12.0),
        mutations=[("regex", r"(\[S\] COMBAT pid=\d+ sf=\d+ phase=)idle", r"\g<1>release"), ("dup", "INPUT", 1)],
        allow=[],
        injection_tolerance=2,
    ),
    "melee.feint-loss": dict(
        family="melee", covers=["combat", "cost"],
        worlds=("S", "C1", "C2"), latencies=(0, 100), loss=5.0,
        roles=dict(p1=("C1", (0.0, 15200.0, 320.0), 0.0),
                   p2=("C2", (-1000.0, 15000.0, 320.0), 180.0)),
        cvars={"fm.SeaState": "1.0", "fm.WindAngle": "30"},
        plan=[(120, "p1", "ship", "sail_length", 1.0), (300, "p2", "ship", "wheel", 0.5),
              (400, "p1", "face", 1.0), (420, "p1", "tap", "attack_horizontal"), (430, "p1", "tap", "feint")],
        stop=dict(duration=12.0),
        mutations=[("regex", r"(\[S\] COMBAT .*? attack=)horizontal", r"\g<1>vertical")],
        allow=[],
        injection_tolerance=3,
        settle_frames=60,
    ),
    "melee.direction": dict(
        family="melee", covers=["combat", "cost"],
        worlds=("S", "C1", "C2"), latencies=(0, 100), loss=0.0,
        roles=dict(p1=("C1", (0.0, 15200.0, 320.0), 0.0),
                   p2=("C2", (-1000.0, 15000.0, 320.0), 180.0)),
        cvars={"fm.SeaState": "1.0", "fm.WindAngle": "30"},
        plan=[(120, "p1", "ship", "sail_length", 1.0), (300, "p2", "ship", "wheel", 0.5),
              (400, "p1", "face", -20.0), (420, "p1", "tap", "attack_horizontal"),
              (600, "p1", "face", 20.0), (620, "p1", "tap", "attack_horizontal")],
        stop=dict(duration=14.0),
        mutations=[("regex", r"horizontal_l", "horizontal_r")],
        allow=[],
    ),
    "melee.hit": dict(
        family="melee", covers=["rewind", "combat", "cost"],
        worlds=("S", "C1", "C2"), latencies=(0, 50, 100, 150), loss=0.0,
        roles=dict(p1=("C1", (0.0, 15200.0, 320.0), 0.0),
                   p2=("C2", (150.0, 15200.0, 320.0), 180.0)),
        cvars={"fm.SeaState": "1.0", "fm.WindAngle": "30"},
        plan=[(120, "p1", "ship", "sail_length", 1.0), (340, "p1", "face", 1.0), (360, "p1", "tap", "attack_overhead")],
        stop=dict(duration=10.0),
        mutations=[("drop", "HIT", 1), ("set", "HIT", "tx", "999.0"), ("regex", r"(\[C2\] SCORE pid=\d+ taken=)1", r"\g<1>0")],
        allow=[],
    ),
    "melee.hit-calm": dict(
        family="melee", covers=["rewind", "combat", "cost"],
        worlds=("S", "C1", "C2"), latencies=(0, 50, 100, 150), loss=0.0,
        roles=dict(p1=("C1", (0.0, 15200.0, 320.0), 0.0),
                   p2=("C2", (150.0, 15200.0, 320.0), 180.0)),
        cvars={"fm.SeaState": "0.0", "fm.WindAngle": "0"},
        plan=[(120, "p1", "ship", "sail_length", 1.0), (340, "p1", "face", 1.0), (360, "p1", "tap", "attack_overhead")],
        stop=dict(duration=10.0),
        mutations=[("drop", "HIT", 1), ("set", "HIT", "x", "999.0"), ("regex", r"(\[C2\] SCORE pid=\d+ taken=)1", r"\g<1>0")],
        allow=[],
    ),
    # The walker crosses a thrust's line 110 cm ahead, along the ship's length; the thrust reaches
    # past the walker for its whole release, so the rewound body meets the blade for view lags
    # of 11 to 26 frames and some frames either side.
    "melee.hit-walk": dict(
        family="melee", covers=["rewind", "combat", "cost"],
        worlds=("S", "C1", "C2"), latencies=(0, 50, 100, 150), loss=0.0,
        roles=dict(p1=("C1", (0.0, 14900.0, 320.0), 90.0),
                   p2=("C2", (421.0, 15010.0, 320.0), 270.0)),
        cvars={"fm.SeaState": "1.0", "fm.WindAngle": "30"},
        plan=[(120, "p1", "ship", "sail_length", 1.0), (340, "p1", "face", 89.0),
              (355, "p2", "move", -1.0, 0.0, 100), (360, "p1", "tap", "attack_thrust")],
        stop=dict(duration=10.0),
        mutations=[("drop", "HIT", 1), ("set", "HIT", "tx", "999.0"), ("regex", r"(\[C2\] SCORE pid=\d+ taken=)1", r"\g<1>0")],
        allow=[],
        pose_every=1,
        injection_tolerance=2,
    ),
    "melee.parry": dict(
        family="melee", covers=["parry", "combat", "cost"],
        worlds=("S", "C1", "C2"), latencies=(0, 50, 100, 150), loss=0.0,
        roles=dict(p1=("C1", (0.0, 15200.0, 320.0), 0.0),
                   p2=("C2", (150.0, 15200.0, 320.0), 180.0)),
        cvars={"fm.SeaState": "1.0", "fm.WindAngle": "30"},
        plan=[(120, "p1", "ship", "sail_length", 1.0), (340, "p1", "face", 1.0), (360, "p1", "tap", "attack_overhead"), (372, "p2", "tap", "parry")],
        stop=dict(duration=10.0),
        mutations=[("drop", "PARRY", 1), ("regex", r"(\[C1\] SCORE pid=\d+ taken=\d+ dealt=\d+ parries=)1", r"\g<1>0")],
        allow=[],
    ),
    "melee.parry-late": dict(
        family="melee", covers=["parry", "combat", "cost"],
        worlds=("S", "C1", "C2"), latencies=(0, 50, 100, 150), loss=0.0,
        roles=dict(p1=("C1", (0.0, 15200.0, 320.0), 0.0),
                   p2=("C2", (150.0, 15200.0, 320.0), 180.0)),
        cvars={"fm.SeaState": "1.0", "fm.WindAngle": "30"},
        plan=[(120, "p1", "ship", "sail_length", 1.0), (340, "p1", "face", 1.0), (360, "p1", "tap", "attack_overhead"), (412, "p2", "tap", "parry")],
        stop=dict(duration=10.0),
        mutations=[("drop", "HIT", 1), ("regex", r"(\[C2\] SCORE pid=\d+ taken=)1", r"\g<1>0")],
        allow=[],
    ),
    "melee.advance-half": dict(
        family="melee", covers=["advance", "combat", "cost"],
        worlds=("S", "C1", "C2"), latencies=(0, 50, 100, 150), loss=0.0,
        roles=dict(p1=("C1", (0.0, 15200.0, 320.0), 0.0),
                   p2=("C2", (150.0, 15200.0, 320.0), 180.0)),
        cvars={"fm.SeaState": "1.0", "fm.WindAngle": "30", "fm.MeleeAdvanceFraction": "0.5", "fm.MeleeAdvanceCapMs": "50"},
        plan=[(120, "p1", "ship", "sail_length", 1.0), (340, "p1", "face", 1.0), (360, "p1", "tap", "attack_overhead"), (372, "p2", "tap", "parry")],
        stop=dict(duration=10.0),
        mutations=[("regex", r"\[S\] (HIT|PARRY) ", r"[S] NONE "), ("regex", r"(\[C2\] SCORE pid=\d+ taken=)\d+", r"\g<1>9")],
        allow=[],
    ),
    "melee.advance-whole": dict(
        family="melee", covers=["advance", "combat", "cost"],
        worlds=("S", "C1", "C2"), latencies=(0, 50, 100, 150), loss=0.0,
        roles=dict(p1=("C1", (0.0, 15200.0, 320.0), 0.0),
                   p2=("C2", (150.0, 15200.0, 320.0), 180.0)),
        cvars={"fm.SeaState": "1.0", "fm.WindAngle": "30", "fm.MeleeAdvanceFraction": "1.0", "fm.MeleeAdvanceCapMs": "80"},
        plan=[(120, "p1", "ship", "sail_length", 1.0), (340, "p1", "face", 1.0), (360, "p1", "tap", "attack_overhead"), (372, "p2", "tap", "parry")],
        stop=dict(duration=10.0),
        mutations=[("regex", r"\[S\] (HIT|PARRY) ", r"[S] NONE "), ("regex", r"(\[C2\] SCORE pid=\d+ taken=)\d+", r"\g<1>9")],
        allow=[],
    ),
}


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
            if op == "ship":
                if len(step) < 5 or step[3] not in SHIP_INPUTS:
                    problems.append(where + "ship op needs a station in SHIP_INPUTS and a value")
                elif not isinstance(step[4], (int, float)):
                    problems.append(where + "ship op value must be a number")
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
        tol = s.get("injection_tolerance", 1)
        if not isinstance(tol, int) or tol < 0:
            problems.append(where + "injection_tolerance must be a non-negative frame count")
        every = s.get("pose_every", 6)
        if not isinstance(every, int) or every <= 0:
            problems.append(where + "pose_every must be a positive frame count")
        for name in s.get("cvars", {}):
            if name not in CVAR_DEFAULTS:
                problems.append(where + "cvar %s has no default in CVAR_DEFAULTS, so it would leak into the next row" % name)
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
