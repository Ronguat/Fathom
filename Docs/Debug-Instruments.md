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
pid=<player id> sf=<frame> x= y= z= yaw= mode=` is written by every world for every pawn every
sixth finalized frame, `sf` the frame the pawn last finalized and the position its sync state's,
which the determinism rows compare across worlds. `OCEAN sf=<frame> sea=<s> h0..h3=<cm>
g0..g3=<cm> gpu_max=<cm> gpu_mean=<cm>` is written once a second by every world: the CPU's
displacement height at four fixed points, the GPU's at the same points read back from the probe
material, and the GPU-against-CPU error over a 16 by 16 grid; a dedicated server writes the CPU
and `gpu=none`; `inv=<cm>` on the same line is the height inversion's residual at those points.
`SHIP id=<n> sf=<frame> x= y= z= yaw= pitch= roll= speed= sail= angle= rudder= anchor=` is
written every sixth frame by every world, the server's the truth and a client's its
reconstruction, and `SHIPIN id=<n> sf=<frame> input=<station> value=<v>` by the server at the
frame a station input took effect. `MARK category=<c>` is the marker hotkey, `M`, and the console
command `FM.Mark <category>`. Everything else is a rung's own vocabulary.

**Clients relay their trace to the server** once a second over a reliable call, into the
server's session file, and into its log only outside PIE, where the process log already carries
every world once. **A marker hotkey** drops a `MARK` line at the frame the player pressed it, which
is the only thing a remote human is asked to do. **Every world keeps a session bundle** under
`Saved/Fathom/Sessions/<PIE|Field>/<stamp>-<tag>/`: `trace.log`, and `meta.json` with the commit, the map, the
fixed rate, the frame range and per-connection averages, rewritten every ten seconds, at the end
and on `FM.Bundle`; the server's carries every world. `Tools/RegressionCheck/ingest_bundle.py`
turns a server bundle into a slice, runs the universal set and the cost readout on it, and lists
the marks. **A field session is `Tools/RegressionCheck/field-session.sh`**, the interactive editor
closed: the editor with `-server` on the harness map, the packaged client from
`Saved/Packaged/Windows/` joining as `C1`, the hotkey pressed in its window, then the ingest.

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
every world reporting, cost printed per connection, injection latency constant, no unallowed
engine warning. **Every assertion has failed once on purpose**: the self-tests corrupt a good slice
one way per row, and a run proves each scenario's mutations turn it red. Rows run on the fixed
clock through `UFMTimeTools`, and every run prints bandwidth per connection and server tick time
per player.

**A plan's ops** are `tap`, `press`, `release` and `hold` on an action, `move` and `stop_move`,
`face`, `teleport`, `mark`, and `ship <station> <value>`, which drives one of the ship's stations
from the role's own client through its controller; the runner writes a `SHIPOP` marker for it.

**The shape of a run.** `regression-run.sh` preflights, arms `ue_regression_runner.py` inside the
editor through the remote-execution pipe, and follows the log for the `REGRESSION` markers each
row emits; the runner starts play as a client with a dedicated server, addresses each world by its
PIE instance, caps the frame rate at the fixed rate so an emulated lag keeps its size in frames,
splits the round trip across both directions with `NetEmulation.PktLag`, places each role through
the simulation, counts frames in the server's own simulation frame, drives keys through
`UFMInputTools` on each role's own player controller, and writes a tape of every role's pose in
its own world and on the server. *First driven 2026-09-05: twelve rows in under a minute of wall
time, four seconds of play each.*

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
| `ship.stop` | S C1 C2 | 0, 50, 100, 150 | f60 p1 ship sail_length 1.0; f360 p1 ship anchor 1.0 | 10 s | determinism, cost |
| `ship.turn` | S C1 C2 | 0, 50, 100, 150 | f60 p1 ship sail_length 1.0; f240 p1 ship wheel 1.0; f480 p1 ship wheel 0.0 | 12 s | determinism, cost |
| `ship.turn-loss` | S C1 C2 | 0, 100 | f60 p1 ship sail_length 1.0; f240 p1 ship wheel 1.0; f480 p1 ship wheel 0.0 | 12 s | determinism, cost |

*Generated from `Tools/RegressionCheck/scenarios.py` by `Tools/RegressionCheck/gen-matrix.py`. Edit the fixtures there, never this table.*
<!-- matrix:end -->

### Coverage map

<!-- coverage:begin -->

| Mechanic | Rows asserting it |
|---|---|
| two worlds | `harness.idle`, `harness.jump`, `harness.walk`, `harness.walk-loss` |
| cost | `harness.idle`, `harness.jump`, `harness.walk`, `harness.walk-loss`, `ocean.agree`, `ship.sail`, `ship.stop`, `ship.turn`, `ship.turn-loss` |
| injection latency | `harness.jump`, `harness.walk`, `harness.walk-loss` |
| determinism | `harness.idle`, `harness.jump`, `harness.walk`, `harness.walk-loss`, `ocean.agree`, `ship.sail`, `ship.stop`, `ship.turn`, `ship.turn-loss` |

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
