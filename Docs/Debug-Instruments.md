# Debug instruments

**Trigger: about to measure something.** The trace, the two-world PIE recipe, the loop's shape
and the field log. The loop itself is the Harness rung's deliverable; what stands here before it
ships is the design it builds to, marked as such.

## The trace

**One line shape, three producers, one reader.** A trace line is
`[server frame] [world] TAG key=value ...`, where the world is `S` for the server, `C<n>` for a
client, and the frame is the shared simulation frame. The PIE server, every PIE client and every
packaged build write the same shape, and the regression evaluator reads a field session exactly
as it reads a scenario. *Designed 2026-09-04; nothing emits it until the Harness rung ships.*

**Clients relay their trace to the server** in batches over a reliable call, so the server log
carries every world reconciled by frame; each client also writes its own file as the fallback for
a dropped connection. **A marker hotkey** drops a `MARK category=<c>` line at the frame the player
pressed it, which is the only thing a remote human is asked to do. **A session bundle** is written
at server shutdown and on a console command: every trace, the commit hash, the knob values, the
sea state, and per-connection latency and bandwidth. An ingest script turns a bundle into the
evaluator's row format and lists the marks.

**The engine's replay system is the upgrade to recon**, recording the server for scrubbing in the
editor by frame. Its compatibility with the prediction framework is unmeasured, so the text trace
is the guarantee and the replay is the bonus.

## Two-world PIE

Inherited from TheDream's measurements of 2026-08-15 and 2026-08-24, on the same engine.

**Both PIE worlds are addressable from editor Python under one process** *(Python, 2026-08-24)*:

```python
unreal.find_object(None, "/Game/Fathom/Maps/UEDPIE_0_L_Harness.L_Harness")   # server
unreal.find_object(None, "/Game/Fathom/Maps/UEDPIE_1_L_Harness.L_Harness")   # client
```

`GameplayStatics.get_all_actors_of_class` then works per world, and `get_local_role()` says which
is which: the server world reports `ROLE_AUTHORITY` on everything and carries the game mode; the
client world reports `ROLE_AUTONOMOUS_PROXY` on its own pawn and `ROLE_SIMULATED_PROXY` on the
rest. **`UFMInputTools` drives either world's player controller**, so both sides of an exchange are
scriptable from one place.

**Never match actors across worlds by name.** Each world numbers its own actors, so the same name
in two worlds is two different pawns; TheDream measured an 85 cm "desync" that was two characters
swapped. Anchor on `get_local_role()`, on position, or on a replicated identity.

**Two worlds run two clocks** *(measured 2026-08-15)*. One event logged at 2.788 in one world and
3.242 in the other, which is why this project stamps the shared simulation frame and never wall
or world time. A reader pairing lines from two worlds by time gets plausible numbers that mean
nothing.

**The recipe.** Edit `Saved/Config/WindowsEditor/EditorPerProjectUserSettings.ini` **with the
editor closed** — it is rewritten on exit — setting `PlayNumberOfClients=2` and the net mode under
`[/Script/UnrealEd.LevelEditorPlaySettings]`. `PIE_Client` spawns a dedicated server and gives
every human real latency; `PIE_ListenServer` gives the host none and hides the defects this
project exists to find. `RunUnderOneProcess=True` is easier to drive and writes no client log;
`False` writes `Saved/Logs/Fathom_2.log` for the client. The file is gitignored machine state.
Restore it afterwards.

## The regression loop

**Shape inherited from TheDream, two-world from the first row.** A scenario names its roles with
placements, its knobs, a plan in frames, a stop condition, the mechanics it covers and at least
one mutation the validator will not let you omit. Assertions are keyed by row id, with a universal
set that runs on every row. **Every new assertion is made to fail once on purpose**: a band nobody
has seen reject anything is indistinguishable from one that could never fail. Rows run at several emulated
latencies, zero included, on the fixed clock through `UFMTimeTools`, and every run prints
bandwidth per connection and server tick time per player.

**Coverage binds at plan time.** A rung's plan lists the rows it adds or files a dated trap naming
what is now untested. A loop that lags the surface still prints green.

## The post-change verification checklist

1. State whether the change touched a header, which decides whether the editor closed and rebuilt.
2. Build, and check the binary is newer than every source.
3. Run the rows the change can reach, selected by mechanism, at every latency they carry.
4. Read what the run says rather than its exit code: a red row blocks the push, an unproven
   mutation is worse than a red.
5. Report what was verified against what was merely written.
