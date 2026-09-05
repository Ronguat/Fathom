# Debug instruments

**Trigger: about to measure something.** The trace, the two-world PIE recipe, the loop and the
field log. The loop's skeleton is in `Tools/RegressionCheck/`; what it has not done is drive a
frame, and the Harness rung is where it first does.

## The trace

**One line shape, three producers, one reader.** A trace line is
`LogFMTrace: [server frame] [world] TAG key=value ...`, where the world is `S` for the server,
`C<n>` for a client, and the frame is the shared simulation frame. The PIE server, every PIE client
and every packaged build write the same shape, and the regression evaluator reads a field session
exactly as it reads a scenario. *Designed 2026-09-04; nothing emits it until the Harness rung ships.*

**Tags the loop reads.** `COST conn=<C<n>> in_bps=<n> out_bps=<n> tick_ms=<f>`, written by the
server per connection once a second, is protocol three's printout; the universal set fails a slice
without one per client. `INPUT role=<r> action=<a> edge=<pressed|released>` is written by the world
that consumed an injected key, and pairs with the runner's `INJECT` marker to assert a constant
injection latency. `MARK category=<c>` is the marker hotkey. Everything else is a rung's own
vocabulary.

**Clients relay their trace to the server** in batches over a reliable call, so the server log
carries every world reconciled by frame; each client also writes its own file as the fallback for a
dropped connection. **A marker hotkey** drops a `MARK` line at the frame the player pressed it, which
is the only thing a remote human is asked to do. **A session bundle** is written at server shutdown
and on a console command: every trace, the commit hash, the knob values, the sea state, and
per-connection latency and bandwidth. An ingest script turns a bundle into the evaluator's slice
format and lists the marks.

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
blocks on anything else. The file is gitignored machine state.

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

**The shape of a run.** `regression-run.sh` preflights, arms `ue_regression_runner.py` inside the
editor through the remote-execution pipe, and follows the log for the `REGRESSION` markers each
row emits; the runner starts play as a client with a dedicated server, addresses each world by its
PIE instance, splits the round trip across both directions with `NetEmulation.PktLag`, drives keys
through `UFMInputTools` on each role's own player controller, and writes a tape of every role's
pose in its own world and on the server. *Written 2026-09-04 against the measurements above; no
row has run.*

**Coverage binds at plan time.** A rung's plan lists the rows it adds or files a dated trap naming
what is now untested. A loop that lags the surface still prints green.

### The scenario matrix

<!-- matrix:begin -->

| Scenario | Worlds | Round trips (ms) | Plan | Stop | Covers |
|---|---|---|---|---|---|
| *none yet* | | | | | |

*Generated from `Tools/RegressionCheck/scenarios.py` by `Tools/RegressionCheck/gen-matrix.py`. Edit the fixtures there, never this table.*
<!-- matrix:end -->

### Coverage map

<!-- coverage:begin -->

| Mechanic | Rows asserting it |
|---|---|
| two worlds | **none** |
| cost | **none** |
| injection latency | **none** |
| determinism | **none** |

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
