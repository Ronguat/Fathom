# Debug instruments

**Trigger: about to measure something.** The trace, the two-world PIE recipe, the loop and the
field log. The loop is in `Tools/RegressionCheck/`, and `regression-run.sh --all` is the run.

## The trace

**One line shape, three producers, one reader.** A trace line is
`LogFMTrace: [server frame] [world] TAG key=value ...`, where the world is `S` for the server,
`C<n>` for a client, and the frame is the shared simulation frame. The PIE server, every PIE client
and every packaged build write the same shape, and the regression evaluator reads a field session
exactly as it reads a scenario. `UFMTraceSubsystem` in `Source/Fathom/Net/` emits it; **the frame
is the prediction framework's pending frame plus its server offset**, so a client stamps the server
frame it is predicting, six frames ahead of the server at zero latency and ten at 50 ms *(measured
2026-09-05)*. `C<n>` is the PIE instance under one process, or `-FMClient=<n>` on a packaged client,
`1` without it.

**Tags the loop reads.** `COST conn=<C<n>> in_bps=<n> out_bps=<n> tick_ms=<f> lag_ms=<f>
players=<n>`, written by the server per connection once a second, is protocol three's printout:
the connection's bytes each way, the server world's actor-tick wall time, the connection's measured
lag; the universal set fails a slice without one per client. `INPUT role=<r> action=<a>
edge=<pressed|released>` is written by the pawn as it authors an input command whose key state
changed, and pairs with the runner's `INJECT` marker; the universal set allows the pairing to move
by one frame within a row, the prediction framework's client throttle, and fails on two. `POSE
pid=<player id> sf=<frame> x= y= z= yaw= mode= base=<0|1> bx= by= bz= rx= ry= rz= wz=` is
written by every world for every pawn every sixth finalized frame, `sf` the frame the pawn last
finalized, the position its sync state's, `bx..bz` that position in the space of the base it
stands on when `base=1`, `rx..rz` the visual's position in the space of the base as it is drawn,
one step behind, and `wz` the
ocean's height under it; the determinism rows compare the world-space fields across worlds, the
deck rows the base-space ones, and the other client's rendering against the server's base-space
state. `ROLLBACK pid= n= to= from=` is
written by a client at the finalize after its pawn rolled back, `n` the count so far. `OCEAN sf=<frame> sea=<s> h0..h3=<cm>
g0..g3=<cm> gpu_max=<cm> gpu_mean=<cm>` is written once a second by every world: the CPU's
displacement height at four fixed points, the GPU's at the same points read back from the probe
material, and the GPU-against-CPU error over a 16 by 16 grid; a dedicated server writes the CPU
and `gpu=none`; `inv=<cm>` on the same line is the height inversion's residual at those points.
`SHIP id=<n> sf=<frame> x= y= z= yaw= pitch= roll= speed= sail= angle= rudder= anchor=` is
written every sixth frame by every world, the server's the truth and a client's its
reconstruction; `SHIPIN id=<n> sf=<frame> cmd=<frame> input=<station> value=<v>` by the server
when it records a station call, `cmd` the frame of the input command that carried it and `sf`
the frame it takes effect, the command's plus the session's station delay, which the caller's
client applies alike as a prediction, `value` the target applied, a key's release a hold filled
in from the station's projected position at that frame; and `SHIPNO id= sf= input= dist=` when
the caller stood farther from the station than its radius. `SHIPPRED id= sf= cmd= input= value=
changed=` is the caller's client recording the same call as its prediction, and `SHIPREP id= sf=
frame= sail= changed= pruned=` a client receiving the server's latest input, `changed` whether
it differed from what the client held at that frame, which a confirmed prediction never does,
and `frame` under `sf` an input that arrived after its frame, which the delay exists to prevent.
`STATIONDELAY frames= worst_ms= clients=` is the server setting the station delay, at its first
tick and whenever it changes. `COMBAT pid= sf= phase=<idle|windup|release|recovery|parry>
attack=<name|-> start=<frame> parry=<frame>` is written by every world for every pawn at a phase
change, `start` the frame the attack began in the sync state, `parry` the parry's; the melee rows
compare `start` and the phase order across worlds. `HIT pid=<attacker> sf= target= rf= rp= k=
part=<head|body> x= y= z= tx= ty= tz= moved= window= facing=` is written by the server at the
frame a blade met a body: `rf` the frame the attacker's command said its proxies were drawn from
and `rp` the fraction toward the next, between which the body is rewound; `k` the attack frame;
`x..z` the contact and `tx..tz` the rewound body's centre, both in the attacker's frame, ship
space when it stands on the ship; `moved` how far that body moved since; `window` and `facing`
whether it was parrying and facing. A proxy's own `POSE` labels `sf` with that same `rf`, so
the row reads the rendered position off the label. `PARRY pid= sf= target= rf= rp= k= margin=
x= y= z=` is the same for a blade a parry met, `margin` the frames the window had left at `rf`. `SWING pid= sf= attack= start= hits= parried=` is written by the server
as release ends. `SCORE pid= taken= dealt= parries=` is written by the server when a tally changes
and by a client when the replicated tallies arrive. `BOARD pid= sf=` is written by the server
when a pawn lands on the deck by the board key. `MARK category=<c>` is the marker hotkey,
`M`, and the console command `FM.Mark <category>`. Everything else is a rung's own vocabulary.

**Clients relay their trace to the server** every tenth of a second over a reliable call in
chunks of sixteen lines, into the server's session file, and into its log only outside PIE,
where the process log already carries every world once; once a second in chunks of 64 it crowded
a client's packets under per-frame poses and cost the server input commands *(2026-09-07)*. **A marker hotkey** drops a `MARK` line at the frame the player pressed it, which
is the only thing a remote human is asked to do. **Every world keeps a session bundle** under
`Saved/Fathom/Sessions/<PIE|Field>/<stamp>-<tag>/`: `trace.log`, and `meta.json` with the commit, the map, the
fixed rate, the frame range and per-connection averages, rewritten every ten seconds, at the end
and on `FM.Bundle`; the server's carries every world. `Tools/RegressionCheck/ingest_bundle.py`
turns a server bundle into a slice, runs the universal set and the cost readout on it, and lists
the marks. **A field session is `Tools/RegressionCheck/field-session.sh`**, the interactive editor
closed: the editor with `-server` on the harness map, the packaged client from
`Saved/Packaged/Windows/` joining as `C1`, the hotkey pressed in its window, then the ingest.
**A hands-on session is `Tools/Editor/handson.py`**, driven through `Tools/Editor/run-in-editor.py
-c` with the editor open: two-client PIE at an emulated round trip, both pawns on the deck, the
ship driven on request, per-frame tapes of the rendered ship, pawn and camera transforms on every
world, consecutive rendered frames through the `Shot` command into `Saved/Screenshots/`, and a
judder tape that holds a movement key and reads the camera's, the ship mesh's and the other
pawn's step between rendered frames; its review behaviours, `board`, `stand`, `walk`, `loop`,
`swing`, `parry`, `feint`, `face`, `latency`, `advance` and `stop_review`, drive the other pawn
for the items in `Docs/Checklist.md`. A
view mode or show flag reaches the play viewport only through the controller's console.
**A human at the play window** boards with B, flies where the view points with F and drops with
F again, drives the stations from their placeholders on
the deck with Left/Right, Up/Down, `[` `]`, X and E, sees the sail as a slab hanging from the
yard, the wind as a pennant at the masthead and the
floor's edge as five still columns, reads `AFMHUD`, the world tag, frame,
measured lag, advance, the session's station delay, mode, ship-space place or the distance to the ship and, swimming, the way back, nearest station and its radius, the ship's speed
and station values, sea state, wind, combat phase, tallies and the controller's notice, and
sets a round trip on every world in the process with `FM.Latency <ms>`, or one per client with
`FM.Latency <ms1> <ms2>`, the server carrying the smallest half. Every hit and parry the
server resolves is drawn on both clients for half a second, the blade with the rewound capsule
and head, red for a hit and blue for a parry, on the ship as that world presents it;
`fm.MeleeDraw 1` draws the local blade green on every release frame. The loop
reads none of this; what is rendered is asserted by nothing.

**The engine's replay system is the upgrade to recon**, recording the server for scrubbing in the
editor by frame. Its compatibility with the prediction framework is unmeasured, so the text trace
is the guarantee and the replay is the bonus.

## Two-world PIE

Measured 2026-08-15 and 2026-08-24, on this engine version, before this project existed.

**Both PIE worlds are addressable from editor Python under one process** *(Python, 2026-08-24)*:

```python
unreal.find_object(None, "/Game/Fathom/Maps/UEDPIE_0_L_Harness.L_Harness")   # server
unreal.find_object(None, "/Game/Fathom/Maps/UEDPIE_1_L_Harness.L_Harness")   # client 1
```

`GameplayStatics.get_all_actors_of_class` then works per world, and `get_local_role()` says which
is which: the server world reports `ROLE_AUTHORITY` on everything and carries the game mode; a
client world reports `ROLE_AUTONOMOUS_PROXY` on its own pawn and `ROLE_SIMULATED_PROXY` on the rest.
**`UFMInputTools` drives any world's player controller**, so every side of an exchange is
scriptable from one place.

**Never match actors across worlds by name.** Each world numbers its own actors, so the same name
in two worlds is two different pawns; an 85 cm "desync" once measured was two characters swapped.
Anchor on `get_local_role()`, on position, or on a replicated identity — the runner uses the
player id.

**Two worlds run two clocks** *(measured 2026-08-15)*. One event logged at 2.788 in one world and
3.242 in the other, which is why this project stamps the shared simulation frame and never wall
or world time. A reader pairing lines from two worlds by time gets plausible numbers that mean
nothing.

**The recipe.** Edit `Saved/Config/WindowsEditor/EditorPerProjectUserSettings.ini` **with the
editor closed** — it is rewritten on exit — setting `PlayNumberOfClients=2`, `PlayNetMode=PIE_Client`
and `RunUnderOneProcess=True` under `[/Script/UnrealEd.LevelEditorPlaySettings]`. `PIE_Client`
spawns a dedicated server and gives every human real latency; `PIE_ListenServer` gives the host
none and hides the defects this project exists to find. One process is what lets the runner address
every world; `False` writes `Saved/Logs/Fathom_2.log` for the client instead. The loop's preflight
reads that file, the settings having no Python class, and blocks on anything else. The file is
gitignored machine state. *Run this way 2026-09-05: the server is instance 0, the clients 1 and 2.*

## The regression loop

**Two-world from the first row.** A scenario in `Tools/RegressionCheck/scenarios.py` names the
worlds it drives, the round trips it runs at with zero required, its roles with a client world and
a placement each, a plan in frames, a stop condition, the mechanics it covers and at least one
mutation the validator will not let you omit. It runs once per round trip, as `<id>@<ms>`.
Assertions are keyed by scenario id in `regression_rows.py`, and a **universal set** in
`regression_eval.py` runs on every slice: trace lines well-formed, frames monotonic per world,
every world reporting, cost printed per connection, injection latency constant per client, every
`INPUT` accounted for by an `INJECT`, no unallowed engine warning. **Every assertion has failed once on purpose**: the self-tests corrupt a good slice
one way per row, and a run proves each scenario's mutations turn it red. Rows run on the fixed
clock through `UFMTimeTools`, and every run prints bandwidth per connection and server tick time
per player.

**A row's console variables** are set from its `cvars` after every name in `CVAR_DEFAULTS` is
restored to the value that reads the settings, so nothing a row sets reaches the next; a name
without a default fails validation. **A plan's ops** are `tap`, `press`, `release` and `hold` on an action, `move` and `stop_move`,
`face`, `teleport`, `mark`, and `ship <station> <value>`, which drives one of the ship's stations
from the role's own client through its controller; the runner writes a `SHIPOP` marker for it.
The stations are `wheel`, `sail_length`, `sail_angle`, `anchor` and `ladder`, each with a place
on the ship and a radius in the ship settings. The combat actions are `attack_overhead`,
`attack_horizontal`, `attack_thrust`, `parry` and `feint`; the mouse wheel's two keys are
momentary, one event and one `INJECT` press with no release, and a `face` before a press picks
the side. `pose_every` on a scenario sets every simulated proxy's `POSE` cadence for the row,
one for the row that reads a rendered position at the rewound frame; every pawn at every frame
lifted a client's lead from 6 frames to 17 *(2026-09-07)*, so the cadence stays on the proxies
alone. **The relay sets the view lag**: the frames between an attacker's own frame and the one
its proxies are drawn from read 19, 24, 31 and 37 at 0, 50, 100 and 150 ms with the relay in
once-a-second chunks of 64 lines, and 11, 17, 22 and 26 in tenth-second chunks of sixteen
*(2026-09-07)*. A row's steady state is asserted outside a settle
window: thirty frames after the last applied ship input, `settle_frames` on a scenario to widen
it, and eighteen frames after a pawn's own input edge; the transients inside are reported. A row
warms up 180 frames under its emulation before BEGIN, so the station delay the server sets
from the measured round trip has reached every world, and times a station's consequences from
the call's effect frame on its `SHIPIN` line rather than the plan's *(2026-09-08)*.

**The shape of a run.** `regression-run.sh` preflights, arms `ue_regression_runner.py` inside the
editor through the remote-execution pipe, and follows the log for the `REGRESSION` markers each
row emits; the runner starts play as a client with a dedicated server, addresses each world by its
PIE instance, caps the frame rate at the fixed rate so an emulated lag keeps its size in frames,
splits the round trip across both directions with `NetEmulation.PktLag`, places each role through
the simulation, counts frames in the server's own simulation frame, drives keys through
`UFMInputTools` on each role's own player controller, and writes a tape of every role's pose in
its own world and on the server. *First driven 2026-09-05: twelve rows in under a minute of wall
time, four seconds of play each.* **Hands off the keyboard while a run drives the play windows**:
the pawn polls key state, so a focused client window feeds a human's keys into a row exactly as
injected ones, and the universal set pairs `INJECT` to `INPUT` and flags no `INPUT` without one;
the trap is in `Docs/Decisions.md` *(2026-09-06)*.

**Coverage binds at plan time.** A rung's plan lists the rows it adds or files a dated trap naming
what is now untested. A loop that lags the surface still prints green.

### The scenario matrix

<!-- matrix:begin -->

| Scenario | Worlds | Round trips (ms) | Plan | Stop | Covers |
|---|---|---|---|---|---|
| `harness.idle` | S C1 C2 | 0, 50, 100, 150 | f120 p1 mark idle | 6 s | two worlds, cost, determinism |
| `harness.jump` | S C1 C2 | 0, 50, 100, 150 | f60 p1 tap jump | 6 s | two worlds, cost, injection latency, determinism |
| `harness.walk` | S C1 C2 | 0, 50, 100, 150 | f60 p1 move 0.0 1.0 120 | 6 s | two worlds, cost, injection latency, determinism |
| `harness.walk-loss` | S C1 C2 | 0, 100 | f60 p1 move 0.0 1.0 120 | 6 s | two worlds, cost, injection latency, determinism |
| `ocean.agree` | S C1 C2 | 0, 50, 100, 150 | - | 8 s | determinism, cost |
| `ship.sail` | S C1 C2 | 0, 50, 100, 150 | f60 p1 ship sail_length 1.0 | 12 s | determinism, cost |
| `ship.stop` | S C1 C2 | 0, 50, 100, 150 | f60 p1 ship sail_length 1.0; f360 p2 ship anchor 1.0 | 14 s | determinism, cost |
| `ship.turn` | S C1 C2 | 0, 50, 100, 150 | f60 p1 ship sail_length 1.0; f240 p2 ship wheel 1.0; f480 p2 ship wheel 0.0 | 12 s | determinism, cost |
| `ship.turn-loss` | S C1 C2 | 0, 100 | f60 p1 ship sail_length 1.0; f240 p2 ship wheel 1.0; f480 p2 ship wheel 0.0 | 12 s | determinism, cost |
| `deck.jump` | S C1 C2 | 0, 50, 100, 150 | f120 p1 ship sail_length 1.0; f300 p2 ship wheel 0.5; f420 p1 tap jump | 12 s | determinism, cost, injection latency |
| `deck.stand` | S C1 C2 | 0, 50, 100, 150 | f120 p1 ship sail_length 1.0; f300 p2 ship wheel 0.5 | 14 s | determinism, cost |
| `deck.station` | S C1 C2 | 0, 100 | f120 p2 ship wheel 1.0; f180 p1 ship wheel 1.0; f240 p1 ship wheel 0.0 | 6 s | two worlds, cost |
| `deck.station-key` | S C1 C2 | 0, 100 | f120 p1 tap board; f180 p1 hold sail_down 150; f240 p1 tap wheel_right; f300 p2 tap wheel_left; +1 more | 13 s | two worlds, cost, injection latency |
| `deck.swim` | S C1 C2 | 0, 50, 100, 150 | f360 p1 ship ladder 1.0 | 10 s | determinism, cost |
| `deck.walk` | S C1 C2 | 0, 50, 100, 150 | f120 p1 ship sail_length 1.0; f300 p2 ship wheel 0.5; f420 p1 move 0.0 1.0 90 | 14 s | determinism, cost, injection latency |
| `melee.advance-half` | S C1 C2 | 0, 50, 100, 150 | f120 p1 ship sail_length 1.0; f340 p1 face 1.0; f360 p1 tap attack_overhead; f372 p2 tap parry | 10 s | advance, combat, cost |
| `melee.advance-whole` | S C1 C2 | 0, 50, 100, 150 | f120 p1 ship sail_length 1.0; f340 p1 face 1.0; f360 p1 tap attack_overhead; f372 p2 tap parry | 10 s | advance, combat, cost |
| `melee.direction` | S C1 C2 | 0, 100 | f120 p1 ship sail_length 1.0; f300 p2 ship wheel 0.5; f400 p1 face -20.0; f420 p1 tap attack_horizontal; +2 more | 14 s | combat, cost |
| `melee.feint` | S C1 C2 | 0, 50, 100, 150 | f120 p1 ship sail_length 1.0; f300 p2 ship wheel 0.5; f400 p1 face 1.0; f420 p1 tap attack_horizontal; +1 more | 12 s | combat, cost |
| `melee.feint-loss` | S C1 C2 | 0, 100 | f120 p1 ship sail_length 1.0; f300 p2 ship wheel 0.5; f400 p1 face 1.0; f420 p1 tap attack_horizontal; +1 more | 12 s | combat, cost |
| `melee.hit` | S C1 C2 | 0, 50, 100, 150 | f120 p1 ship sail_length 1.0; f340 p1 face 1.0; f360 p1 tap attack_overhead | 10 s | rewind, combat, cost |
| `melee.hit-calm` | S C1 C2 | 0, 50, 100, 150 | f120 p1 ship sail_length 1.0; f340 p1 face 1.0; f360 p1 tap attack_overhead | 10 s | rewind, combat, cost |
| `melee.hit-walk` | S C1 C2 | 0, 50, 100, 150 | f120 p1 ship sail_length 1.0; f340 p1 face 89.0; f355 p2 move -1.0 0.0 100; f360 p1 tap attack_thrust | 10 s | rewind, combat, cost |
| `melee.parry` | S C1 C2 | 0, 50, 100, 150 | f120 p1 ship sail_length 1.0; f340 p1 face 1.0; f360 p1 tap attack_overhead; f372 p2 tap parry | 10 s | parry, combat, cost |
| `melee.parry-late` | S C1 C2 | 0, 50, 100, 150 | f120 p1 ship sail_length 1.0; f340 p1 face 1.0; f360 p1 tap attack_overhead; f412 p2 tap parry | 10 s | parry, combat, cost |
| `melee.swing` | S C1 C2 | 0, 50, 100, 150 | f120 p1 ship sail_length 1.0; f300 p2 ship wheel 0.5; f400 p1 face 1.0; f420 p1 tap attack_overhead | 12 s | combat, cost |

*Generated from `Tools/RegressionCheck/scenarios.py` by `Tools/RegressionCheck/gen-matrix.py`. Edit the fixtures there, never this table.*
<!-- matrix:end -->

### Coverage map

<!-- coverage:begin -->

| Mechanic | Rows asserting it |
|---|---|
| two worlds | `deck.station`, `deck.station-key`, `harness.idle`, `harness.jump`, `harness.walk`, `harness.walk-loss` |
| cost | `deck.jump`, `deck.stand`, `deck.station`, `deck.station-key`, `deck.swim`, `deck.walk`, `harness.idle`, `harness.jump`, `harness.walk`, `harness.walk-loss`, `melee.advance-half`, `melee.advance-whole`, `melee.direction`, `melee.feint`, `melee.feint-loss`, `melee.hit`, `melee.hit-calm`, `melee.hit-walk`, `melee.parry`, `melee.parry-late`, `melee.swing`, `ocean.agree`, `ship.sail`, `ship.stop`, `ship.turn`, `ship.turn-loss` |
| injection latency | `deck.jump`, `deck.station-key`, `deck.walk`, `harness.jump`, `harness.walk`, `harness.walk-loss` |
| determinism | `deck.jump`, `deck.stand`, `deck.swim`, `deck.walk`, `harness.idle`, `harness.jump`, `harness.walk`, `harness.walk-loss`, `ocean.agree`, `ship.sail`, `ship.stop`, `ship.turn`, `ship.turn-loss` |
| combat | `melee.advance-half`, `melee.advance-whole`, `melee.direction`, `melee.feint`, `melee.feint-loss`, `melee.hit`, `melee.hit-calm`, `melee.hit-walk`, `melee.parry`, `melee.parry-late`, `melee.swing` |
| rewind | `melee.hit`, `melee.hit-calm`, `melee.hit-walk` |
| parry | `melee.parry`, `melee.parry-late` |
| advance | `melee.advance-half`, `melee.advance-whole` |

*Generated from each row's `covers` in `Tools/RegressionCheck/scenarios.py` by `Tools/RegressionCheck/gen-matrix.py`.*
<!-- coverage:end -->

**The tables above are generated** from `Tools/RegressionCheck/scenarios.py` by
`Tools/RegressionCheck/gen-matrix.py`, which `docs-check` re-runs to catch a fixture edit that
forgot the doc.

## The post-change verification checklist

1. State whether the change touched a header, which decides whether the editor closed and rebuilt.
2. Build, and check the binary is newer than every source.
3. Run the rows the change can reach, selected by mechanism, at every latency they carry.
4. Read what the run says rather than its exit code: a red row blocks the push, an unproven
   mutation is worse than a red.
5. Report what was verified against what was merely written.
