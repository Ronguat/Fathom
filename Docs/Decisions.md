# Fathom – Decisions

**Trigger: making a choice, picking up a rung, or ending a session.** Working sections first,
dated entries after them, newest first. **Never rewrite an entry — supersede it**, with a row in
the table below. An entry's shape: the decision, the alternatives, what reopens it, the
measurement if there is one.

## Review queue

Every WHAT an agent decided alone, non-blocking. The designer clears rows whenever they read
this; a verdict that differs supersedes the decision and never rewrites it. Newest first.

| Date | Decision taken alone | Recorded in | Verdict |
|---|---|---|---|
| 2026-09-05 | The Character Movement recon not run, its result unable to change the route once Mover met the bar unpatched | Deck entry, Decisions | |
| 2026-09-05 | A teleport lands in Falling; settle windows after ship inputs, input edges and the swim's start, transients reported | Deck entry, Decisions | |
| 2026-09-05 | The stations' places and radii, the ladder's deck point, the swim mode's float height, spring and speeds | `Config/DefaultGame.ini`, `UFMSwimMode` | |
| 2026-09-05 | The ship stepped at the world tick's start, before the prediction frame; ship-space pose as the deck measure | Deck entry, Decisions | |
| 2026-09-05 | Placements allowed out to the ocean's extent, 20 000 cm, rather than the floor's | `Tools/RegressionCheck/scenarios.py` | |
| 2026-09-05 | A station call queued for the controller's tick, the one home for a client's server call from a script | Ship entry, Decisions | |
| 2026-09-05 | The ship's numbers: top speed 1 000 cm/s, drag 0.3, anchor drag 20, turn rate 15 degrees a second, a 24 by 8 m hull, the fit at stiffness 6 and damping 4 | `Config/DefaultGame.ini` | |
| 2026-09-05 | The ship as a custom integrator rather than a Mover actor; inputs replicated with their frame, unpredicted; the reconstruction shown unsmoothed | Ship entry, Decisions | |
| 2026-09-05 | The stations as named inputs without occupancy until Deck | Ship entry, The plan | |
| 2026-09-05 | A per-scenario injection tolerance, three frames on the loss rows | Ocean entry, Decisions | |
| 2026-09-05 | The comment extractor retries an unterminated quote as text | The commit that fixed it | |
| 2026-09-05 | The ocean as this project's own plane and material rather than the Water plugin's body; the engine's Gerstner shape; amplitude and steepness scaling with the sea state, wavelengths and direction offsets fixed | Ocean entry, Decisions | |
| 2026-09-05 | The GPU read through a probe material into a float render target, once a second | Ocean entry, Decisions | |
| 2026-09-05 | The field session as a script: the editor with `-server`, one packaged client, the hotkey pressed into its window, a fixed wait for the server | `Tools/RegressionCheck/field-session.sh` | |
| 2026-09-05 | An asset-manager rule for game-feature data so the cook counts no error | `Config/DefaultGame.ini` | |
| 2026-09-05 | The injection pairing may move one frame within a row; the runner counts in the server's simulation frame; the frame rate is capped at the fixed rate during a run | Harness entry, Decisions | |
| 2026-09-05 | The engine's cylinder as the pawn's placeholder, a capsule deferred to Stretch | Harness entry, Decisions | |
| 2026-09-05 | The harness pawn is a Mover character from rung one; the trace frame is the prediction framework's pending frame plus offset | Harness entry, Decisions | |
| 2026-09-05 | Input polled from key state through a key table on the pawn; no Enhanced Input assets | Harness entry, Decisions | |
| 2026-09-05 | The world tag scheme, the relay to the server's session file, the bundle layout, the `POSE` line | Harness entry, Decisions | |
| 2026-09-05 | The determinism bar of 1 cm over the second half of a row, and three smoke rows as the Harness coverage | Harness entry, The plan | |
| 2026-09-05 | No ability system in the simulation spine, the manufactured constraint re-decided on the designer's word that the choice is the agent's | The repository stands alone entry, Decisions | |
| 2026-09-04 | The reference model dissolved into its homes rather than kept as the verbatim source | The repository stands alone entry | |
| 2026-09-04 | What the findings archive dropped as useless to this project | The commit that trimmed `Docs/Unreal-Findings.md` | |
| 2026-09-04 | The trace's log category `LogFMTrace`, its `COST`, `INPUT` and `MARK` tags, and the `REGRESSION` marker vocabulary the loop reads | `Docs/Debug-Instruments.md`, The trace | |
| 2026-09-04 | A round trip split evenly across both directions by `NetEmulation.PktLag` on every world | `Tools/RegressionCheck/ue_regression_runner.py` | |
| 2026-09-04 | The basic set as written, from the agent's recollection of Sea of Thieves, row by row | `Docs/Spec.md`, The basic set | |
| 2026-09-04 | A fixed simulation rate of 60 frames a second | Tuning map | |
| 2026-09-04 | Lean renderer settings: no Lumen, no ray tracing, no Substrate, no virtual shadow maps | `Config/DefaultEngine.ini` | |
| 2026-09-04 | The net server tick rate left at the engine default | Tuning map | |
| 2026-09-04 | The harness map is a floor, two player starts and a sky; the pawn is the engine's spectator until Harness | `Content/Fathom/Maps/L_Harness` | |
| 2026-09-04 | Comment volume growth warns rather than fails | `Tools/CommentCheck/comment-check.sh` | |
| 2026-09-04 | Whether the harness pawn is a Mover pawn from rung one, or the trace stamps server time until Deck | Harness brief | |

## What has been superseded

| Entry | Superseded by | What changed |
|---|---|---|
| 2026-09-04 — Fathom is bootstrapped: the spine, the regime and the inheritance | 2026-09-04 — The repository stands alone, and forgets its parent | The reference model's ownership: the file is dissolved into its homes and the ownership check removed; the Harness brief builds on the loop skeleton here rather than a reference elsewhere |

## Known traps, indexed by what sets them off

**What this section is.** Latent defects and unverified assumptions, each filed against the rung
that makes it bite and re-read when that rung starts. These are not design questions. Nothing
here needs play to settle; they need checking. **Discharge a trap in the same commit that fixes
it**, saying what discharged it.

**Whenever a client acts — *local state never replicates.*** A plain member written on the server
stays on the server, and a client's own check then passes what the server already failed. Decide
on the server, replicate the decision, apply everywhere.

**Whenever two worlds' logs are read together — *two clocks.*** One event was measured at
2.788 in one world and 3.242 in the other. The trace stamps the shared frame and never a clock;
a line without a frame is not evidence.

**Whenever actors are matched across worlds — *never by name.*** Each world numbers its own
actors; an 85 cm "desync" once measured was two characters swapped. Anchor on role,
position or a replicated identity.

**Whenever a based player resimulates — *Mover reads the base at its present pose.*** The
based-movement library takes the base component's current location and rotation *(headers,
2026-09-04, `BasedMovementUtils.cpp` around lines 127 to 138)*, so a resimulation of past frames
stands the player on the ship as it is now. The deterministic ship can answer "pose at frame N";
the patch goes in a project-local copy of the plugin. Bites at Deck.

**Whenever the ship is kinematic — *it must publish its velocity.*** Mover carries a standing
player by the base component's velocity *(headers, 2026-09-04, the same file around line 158)*,
which a component moved by hand has only if it is set every tick. The hull box sets
`ComponentVelocity` from its frame-to-frame motion *(2026-09-05, unverified under a pawn)*.
Bites at Deck.

**Whenever the Water plugin's ocean body is used — *it wants an island and a bounded zone.*** Its
ocean is the outside of a spline inside a zone of finite extent; its wave clock is a locally
accumulated float with an override hook on the subsystem; its buoyancy is Chaos and is bypassed
*(headers, 2026-09-04)*. The fallback is a plane and a material sharing the wave function. Bites
at Ocean.

**Whenever the ocean's CPU and GPU are compared — *identity is asserted, never assumed.*** The
plugin's own depth-attenuation comment says it "should match the GPU version" *(headers,
2026-09-04, `GerstnerWaterWaves.h` line 189)*. Protocol two's row is the proof. Bites at Ocean.

**Whenever a dedicated server target is wanted — *installed builds may refuse it.*** The
installed-build marker is present and server-target support is unconfirmed *(UBT string search
inconclusive, 2026-09-04)*. The editor's `-server` switch is the route regardless. Bites at
Harness.

**Whenever a packaged client connects to the editor server — *versions must match.*** Untested
*(2026-09-04)*. Bites at Harness.

**Whenever the harness stamps a frame before Deck — *the frame comes from the prediction
framework.*** Either the harness pawn is a Mover pawn from rung one, or the trace stamps server
time until Deck; the Harness plan decides and the review queue carries it. Bites at Harness.

**Whenever the Melee rung opens — *the clips and their skeleton are the designer's delivery.*** The
pawn starts on the engine mannequin; the intake sub-slice swaps or retargets. Without the
delivery, halt and demand it. Bites at Melee.

**Whenever the attacker's view delay is needed — *it is authored, never estimated.*** The rendered
frame rides in the input command. Bites at Melee.

**Whenever a sim proxy's deck position is read — *it is interpolated presentation.*** A
client's view of the other pawn in ship space is asserted at 50 cm *(2026-09-05)* and no
tighter until Melee's rewind reads the sync state instead. Bites at Melee.

## Tuning map — a verdict comes back, which knob moves

**Every row is a warning with a reason, never a lock.** A row opening *"nothing, without
re-deriving it"* names a relationship you would be breaking, not a value you may not touch.

| Question | Move this | **Not** this |
|---|---|---|
| The simulation rate | `FixedTickFrameRate` in `Config/DefaultNetworkPrediction.ini`, 60 | The engine's own fixed frame rate, which overrides it when enabled |
| Hit timing under latency | The advance fraction and its cap, per attack, once Melee builds them | The windup |
| Parry fairness | The parry rule, measured together with the advance | The window length |
| The net update rate | `NetServerMaxTickRate`, at the engine default | The simulation rate |
| Inbound bytes per player | The client's frame rate, `t.MaxFPS`; a packaged client ran uncapped at 30 000 B/s against 10 000 capped at 60 *(2026-09-05)* | The simulation rate |
| The sea | The sea-state scalar | Any single wave component |
| A component's shape | Its row in `Config/DefaultGame.ini` under the ocean settings | The formulation, which two evaluators share |
| The probe's cost | `ProbeEveryFrames` and `ProbeCells` in the ocean settings; the readback is synchronous | The trace cadence |
| Ship handling | The speed curve, the rudder rate, the anchor drag | The hull sample points, which shape the fit rather than the handling |
| The ship's reconstruction | `SnapshotEveryFrames` in the ship settings, 12; every input carries its frame | The integrator, which every world runs alike |

## Rung briefs — read the one you are picking up

**Trigger: starting a rung**, alongside the traps above. Each brief carries its scope, its bar,
its fallback, its coverage and its inheritance; the plan entry written before execution turns
the bar into numbers.

- **Melee** — **Halts without the clips and their skeleton in the project.** Opens with an intake
  sub-slice against a contract: clip list and directions, where windup ends and release begins,
  the blade socket, root motion or not, first and third person pairs; then blade curves baked per
  attack. Attack, parry and feint presses ride in the input command with the rendered frame;
  attack state sits in the sync state; the server sweeps the blade against rewound bodies and the
  reconstructed ship. The advance knob and the parry rule are measured at 0, 50, 100 and 150 ms
  across the three known settings, and the verdict goes to the review queue. First person is the
  aim frame; third person is the pre-registered fallback, a swap rather than a rebuild. **Bar**:
  per `Docs/Spec.md`. **Stripped**: chambers, glancing, ripostes, stamina. **What the known settings
  cost, in the designer's account**: an advance of the whole round trip capped at 80 ms gave instant
  hits up to that ping, windups effectively faster by the ping up to the cap, hits landing early by
  half the ping, and false positives at high latency; an advance of half the round trip capped at
  50 ms gave some hit delay at higher ping and no competitive advantage at any ping. The measurement
  reports each of those. **The clips and their skeleton come from a separate Mordhau-shaped project
  of the designer's, not on this machine**, the same project whose half measure is the second
  setting above; its code and write-ups are available on request, as is an answer to any
  animation-contract question the clips leave open. **Inherits**: `AFMPlayerPawn` on Mover with
  its key table read as the input command is authored, the place for presses and the rendered
  frame; `POSE` with the pawn's ship-space position, the frame the sync state carries, and
  `ROLLBACK`; `AFMShip` reconstructible at any frame from its snapshot and history, the rewound
  ship; the deck rows' settle windows and transients as the baseline; the sim-proxy trap above;
  the loop with its thirteen scenarios, the trace, the field session script, and every earlier
  entry's measurements.
- **Ship Combat** — Cannon stations, holes, water, repair, bailing, sinking, respawn. **Bar**: per
  `Docs/Spec.md`.
- **Ship to Ship** — Contact response between two kinematic ships, and boarding across ships as a
  base change. **Bar**: per `Docs/Spec.md`.
- **Stretch** — Receives every deferral, one dated line each. Deferred 2026-09-04, from the basic
  set: storm regions and sea-state change over time; wind drift and gusts; a wave spectrum and
  chop; rope pairs and per-mast sails; class quirks; heel and wheel travel; the anchor turn and
  crew scaling; special shot; above-waterline holes and mast and wheel damage; a sink motion;
  islands and rock holes; harpoons and mermaids; every class but the sloop. Also deferred: the
  replay-system recon behind the text trace; a Blender bridge, triggered only by a feature blocked
  on a shape primitives cannot make; a ship art pack, declined as cosmetic. Deferred 2026-09-05: a
  capsule placeholder from Geometry Script, the engine's cylinder standing in. Deferred
  2026-09-05, from Ship: a predicted station input for the local player; a smoothed presentation
  of the reconstructed ship.

## Symbol index — which entries discuss this thing

Current through **2026-09-05**. Regenerated, byte-sorted, one row per symbol.

| Symbol | Entries |
|---|---|
| `AFMGameState` | 09-05 |
| `AFMOceanActor` | 09-05 |
| `AFMPlayerController` | 09-05 |
| `AFMPlayerPawn` | 09-05 |
| `AFMShip` | 09-05 |
| `FFMShipInputs` | 09-05 |
| `FFMShipState` | 09-05 |
| `FFMStation` | 09-05 |
| `FFMTeleportEffect` | 09-05 |
| `FMOcean` | 09-05 |
| `FM_TRACE` | 09-05 |
| `LogFMTrace` | 09-04, 09-05 |
| `UFMInputTools` | 09-04 |
| `UFMOceanSettings` | 09-05 |
| `UFMOceanSubsystem` | 09-05 |
| `UFMShipSettings` | 09-05 |
| `UFMSwimMode` | 09-05 |
| `UFMSwimTransition` | 09-05 |
| `UFMTimeTools` | 09-04 |
| `UFMTraceLibrary` | 09-05 |
| `UFMTraceSubsystem` | 09-05 |

## 2026-09-05 — Deck: standing on the ship, measured before it is argued

### Next session's brief

**Pick up at the Melee rung, which halts without the clips and their skeleton in the project**,
stop-list item five: demand them, then open with the intake sub-slice against the contract in the
brief. The four rungs below Melee are shipped and green; the minimum answer's foundation stands.
Budget: none set; the designer winds down manually. The editor is closed, the tree clean, every
commit on the remote. Verified against written is below the decisions.

### The plan, written before execution

**Scope.** Sub-slices in order, each committed when green. **One, standing**: the ship steps to
the frame about to be simulated before the prediction framework simulates it, so the base a
pawn reads is the pose of its own frame on every world; the `POSE` line gains `base`, `bx`, `by`
and `bz`, the pawn's position in ship space when it stands on a ship; two pawns dropped onto the
deck by teleport, the ship sailing and turning under them at sea state 1; the rows `deck.stand`
and `deck.walk`. **Two, the recon**: `AFMCharacterPawn` on Character Movement with the same key
table, chosen by the `fm.PawnRoute` console variable, on the same rows, its corrections counted
on a `CMCFIX` line. **Three, the patch**, only if sub-slice one fails its bar at latency: a
project-local copy of the Mover plugin whose based-movement reads a base implementing
`IFMBaseAtFrame` at the frame being resimulated, which `AFMShip` answers from its snapshot and
history. **Four, off the deck**: a movement mode transition into Swimming when the pawn sinks
under the ocean's height at its position, a swim mode held at the surface, and the `ladder`
station returning the pawn to the deck through the simulation. **Five, occupancy**: a station
has a place on the ship and a radius, and a call from farther away is refused. **Six, seeing
each other**: each client's view of the other pawn in ship space, reported against the server.

**Bar, pre-registered.** *Standing and walking*: on a rolling, sailing, turning deck at sea
state 1, the autonomous pawn's position in ship space on its own client against the server's at
the same frame within 5 cm at every matched sample in the second half of the row, at 0, 50, 100
and 150 ms, and `base=1` on every world throughout that half. *The recon*: the same rows under
`fm.PawnRoute=cmc`, the pawn based throughout the second half on every world, its ship-space
position on the client and on the server each spreading under 5 cm while standing, its
corrections reported; Route A stands if Mover meets its bar, patched or not, and Route B is
taken only if Mover fails after the patch and Character Movement meets this one. *Off the
deck*: a pawn teleported into the water is in Swimming within 60 frames and within 30 cm of the
ocean's height at its position for the rest of the row; the ladder returns it to `base=1` within
60 frames of the call. *Occupancy*: a station call from beyond its radius writes no `SHIPIN`; one
from within does. *Seeing each other*: each client's view of the other pawn in ship space within
50 cm of the server's at the same frame in the second half, reported and asserted. Every
earlier row stays green.

**Fallbacks.** Sub-slice one failing at latency: the patch, sub-slice three, then the bar
re-measured. Mover failing after the patch: Route B, the brief's fallback, if the recon's bar
holds there; the Melee brief re-planned on Character Movement. The swim transition fighting
Mover's mode machine: the pawn held at the surface by a layered move instead, filed as a trap.
A dead end after these winds the session down.

**Coverage.** `deck.stand`, `deck.walk`, their `cmc` variants, `deck.swim`, `deck.station`.
**A trap filed**: the sim proxy's ship-space position is interpolated presentation, asserted at
50 cm and no tighter until Melee's rewind reads the sync state instead.

### Decisions

**The ship steps at the world tick's start**, before the prediction framework's frame begins,
so both the base a pawn reads during its simulation and the `SHIP` line carry the frame the pawn
simulates. **Alternative**: the actor tick, one frame behind the pawn on every world alike, a
constant offset of one frame's ship motion. **Reopens** never; it is an ordering fact.

**The deck-relative measure is the pawn's sync-state position in the ship's frame at the same
simulation frame**, on the world that wrote the line. The ship's pose at that frame is the same
on every world once inputs have arrived, so the measure isolates the pawn.

**The Character Movement recon is not run.** Sub-slice one met the bar at every latency with no
patch, and the plan's own rule makes Route B's result unable to change the route from there.
Building the second pawn would also have needed a frame source of its own: with no prediction
instance on a client, the shared frame stands still, and the ship on that client with it.
**Alternative**: run it for the record. **Reopens** if a later rung fails its bar and its
fallback on Mover, when the recon runs with that rung's scenario.

**A teleport lands in Falling.** The harness hover trap bit on the first deck drop: at any
emulated latency a teleported pawn stayed at its drop height in Walking with no base for a whole
row, and at zero latency it fell. `FFMTeleportEffect` writes Falling into the output state, and
every drop since has landed. The trap is discharged by it.

**Steady state is asserted outside settle windows, and the transient inside is reported.** A
ship row asserts from thirty frames after the last applied input, `settle_frames` widening it to
sixty on the loss row after a lost input replication held the client's heading 1.06 degrees off
for thirty-one frames; a deck row asserts from eighteen frames after the pawn's own input edge,
after two samples at 100 ms read 21 and 13 cm eleven and seventeen frames after a walk began,
the server one frame of motion behind the client's prediction with no rollback, then level; the
swim row asserts from a second into swimming, the rise from the drop reported.

**Stations refuse from beyond their radius**, and the ladder is a station whose effect is a
teleport through the simulation to a deck point; the rows that drive stations now stand their
pawns at them.

### Verified against written

**Verified.** Run `0905-024327`: every row of the matrix green with every mutation proven, 46 rows in
285 s of wall time. On a rolling, sailing, turning deck at sea state 1 at 0, 50, 100 and 150 ms:
both pawns based on every world through the second half, ship-space error against the server
within 5 cm at every steady sample standing and walking, world-space peaks 0.2 to 0.3 cm while the ship holds its course and up to 31 cm in the frames a
client's ship still awaits a wheel input, three rollbacks a row at 150 ms, a creep of 3 cm along
and 15 cm across over seven seconds of rolling. Each
client's view of the other pawn in ship space within 50 cm of the server's. Stations: a wheel
call from the bow refused, two from the wheel applied, at 0 and 100 ms. Swimming: in Swimming 63 frames after the
drop at every latency, three over the sixty pre-registered, the row's band at 120 and the drop
200 cm above the water; afloat within 30 cm of the surface from a second in; back on the deck
21 to 27 frames after the ladder call, ship-space error after it within 5 cm, world-space
error while swimming 0.2 cm. The ship rows green with their pawns standing at the stations.
Both checks pass.

**Written, not verified.** The hull's published velocity under a pawn leaving the deck, which
nothing jumps off; the packaged client, not repackaged since Harness; the surface normal.

**Beyond the plan.** Sub-slice two not run, the decision above. Nothing else.

## 2026-09-05 — Ship: a body every world integrates from the same state

### Next session's brief

**Pick up at the Deck rung.** Write its plan entry first: the Mover recon, Character Movement
and Mover on the same deterministic ship, the same scenario, one pre-registered bar; then the
base-at-frame patch through a project-local copy of the plugin if the recon needs it, the hull
publishing its velocity verified under a standing pawn, jumping, falling off into the swim mode,
the ladder station, and two scripted players seeing each other on the deck, with station
occupancy claimed by the pawn at the station. Budget: none set; the designer winds down
manually. The editor is closed, the tree clean, every commit on the remote. Verified against
written is below the decisions.

### The plan, written before execution

**Scope.** The ship as a deterministic kinematic body. **One**: `AFMShip` in `Source/Fathom/Ship/`,
spawned by the game mode at runtime on the ocean beyond the floor; a box hull for collision with
a Geometry Script placeholder over it, moved by transform each frame with its velocity
published. **Two**: the integrator, one frame at a time in single floats on every world: sail
length, sail angle and rudder driven toward their targets at fixed rates; surge from sail length
and the sail's angle against the wind through one curve, toward a top speed with an acceleration
and a drag, the anchor a strong drag when down and raised over a fixed time; yaw from the rudder
at a rate scaled by speed; heave, roll and pitch fitted to the ocean at four hull points by a
spring-damper; every rate a setting in `UFMShipSettings`. **Three**: the compact state replicated
as a snapshot with its frame every twelve frames and the inputs with the frame they took effect;
a client integrates forward from the latest snapshot through its input history to its own frame,
and re-integrates when a snapshot or an input arrives late. **Four**: the stations as the named
inputs `wheel`, `sail_length`, `sail_angle` and `anchor`, driven by a pawn's server call with no
occupancy yet; the runner's `ship` op drives them from a role's client. **Five**: `SHIP` lines
every sixth frame from every world and `SHIPIN` on the server when an input takes effect; the
ocean's inversion residual on the `OCEAN` line. **Six**: the rows `ship.sail`, `ship.turn`,
`ship.stop` at 0, 50, 100 and 150 ms and `ship.turn-loss` at 0 and 100 ms with 5 percent loss.

**Bar, pre-registered.** On every client, the reconstructed pose against the server's at the
same frame: within 10 cm and 1 degree at every matched sample from thirty frames after the last
input change to the row's end, at every latency and under the loss row; the transient after an
input change reported as its peak and its length in frames, not asserted. The ship sails: its
speed on the server reaches four fifths of the top speed within the sail row; turns: its heading
changes by at least thirty degrees over four seconds of full rudder; stops: its speed falls
under 20 cm/s within two seconds of the anchor dropping. The ocean's inversion residual within
1 cm at four points every second at sea state 1. Every earlier row stays green.

**Fallbacks.** Reconstruction failing the bar through float drift between the server's running
state and a client's re-integration: the snapshot cadence halved, then the bar re-measured; a
dead end after that winds the session down. Collision or the deck placeholder fighting the
transform-driven motion: the visual mesh alone, collision filed for Deck.

**Coverage.** The four ship rows and the inversion field on the ocean row, which discharges the
Ship trap. **A trap filed**: station occupancy, the walk to a station and its claim, is Deck's,
where a pawn first stands on the deck.

### Decisions

**A custom integrator, not a Mover actor.** The ship is shared by every player and its inputs
arrive over the network from whoever holds a station; the prediction framework predicts one
owner's inputs and rolls one body back, which is the pawn's shape, not the ship's. The ship's
own shape is a snapshot plus an input history that any world integrates forward, the spine's
"reconstructible at any past frame". **Alternative**: a Mover actor with a custom mode, for the
based-movement plumbing it would give Deck for free. **Reopens** if Deck's recon finds Mover's
base handling needs the base to be a Mover body.

**Inputs replicate with their frame and are not predicted.** The helmsman feels the round trip
on the wheel; a client applies an input change at the frame the server did, re-integrating from
its snapshot when the change arrives late. **Alternative**: predicting the local player's station
input, which is a Stretch line.

**The reconstruction is presentation as well**, snaps included; smoothing is a Stretch line.

**A station call is queued for the controller's tick.** Sent from the runner's callback between
world ticks, a reliable server call on the pawn and then on the controller never reached the
server and logged nothing; sent from `PlayerTick` it always did. The queue is the one home for
that rule; the trace relay, sent from a world-tick delegate, never needed it.

### Verified against written

**Verified.** Run `0905-020254`: thirty-two rows, every one green, every mutation proven, 184 s of
wall time. The ship sails to 947 cm/s on the server and covers 6 700 cm in the sail row; turns
more than thirty degrees over four seconds of rudder; stops under 20 cm/s within two seconds of
the anchor. **Reconstruction** on both clients against the server at the same frame, after the
settle window, within 10 cm and 1 degree on every row at 0, 50, 100 and 150 ms and under 5
percent loss; identical to the two decimals printed once settled. **Transients before settle**:
sail and turn rows peak 0.2 cm at 0 and 50 ms, 0.4 to 1.3 cm at 100 and 150; the stop row peaks
10 cm at 0 ms, 93 at 50, 140 at 100 and 186 at 150, three samples over 10 cm at most, the
anchor's instant drag running on the client until its input arrives. **The ocean's inversion
residual** reads 0.002 to 0.019 cm at four points every second at sea state 1, which discharges
the Ship trap. Server tick 0.60 to 0.63 ms with the ship; inbound 15 800 to 17 000 B/s per
connection, the SHIP relay lines added. The harness and ocean rows green on the same binary.
Both checks pass.

**Written, not verified.** The hull's published velocity, which nothing stands on until Deck;
the surface normal, still unwritten; the hull box as a movement base; the ship in a packaged
client, not repackaged since Harness.

**Beyond the plan.** The station call queued for the controller's tick after two silent drops
from the runner's callback, a finding in `Docs/Unreal-Findings.md`. Nothing else.

## 2026-09-05 — Ocean: one function, three evaluators

### Next session's brief

**Pick up at the Ship rung.** Write its plan entry first: the kinematic hull fit on
`UFMOceanSubsystem::HeightAt`, the sail, wind, rudder and anchor model, the compact replicated
state and its history, the stations, a Geometry Script hull with deck collision spawned at
runtime; whether the ship is a Mover actor or a custom integrator, decided in the entry; the
pre-registered reconstruction error at 100 ms and 5 percent loss; the ship-motion rows at every
latency. Budget: none set; the designer winds down manually. The editor is closed, the tree
clean, every commit on the remote. Verified against written is below the decisions.

### The plan, written before execution

**Scope.** The ocean as a function of position and server time, evaluated three ways and proven
equal. **One**: the wave function, a four-component Gerstner sum in `Source/Fathom/Ocean/` and
the same formulation in `Shaders/FMOcean.ush`, which a material custom node includes; the sea
state and wind derived into the components' amplitude, steepness and direction, the constants in
`UFMOceanSettings`. **Two**: a game state replicating the sea-state scalar and the wind vector
once, set on the server from the settings or the `fm.SeaState` and `fm.WindAngle` console
variables. **Three**: a world subsystem that feeds the material parameter collection every tick
with the frame's time, spawns a Geometry Script plane wearing the ocean material on every world
that renders, and once a second draws a probe material into a float render target, reads it back,
and writes an `OCEAN` line: the CPU displacement at four fixed points, the GPU's at the same
points, and the GPU-against-CPU error over a grid. **Four**: the assets, a parameter collection
and two materials, authored by an editor Python script and committed; the `ocean.agree` scenario
and its row.

**Bar, pre-registered.** `ocean.agree` at 0, 50, 100 and 150 ms at sea state 1: on every client,
the server's CPU displacement and the client's GPU displacement at the same four points and the
same frame within 1 cm at every matched sample, at least four matched samples per row; the
client's GPU-against-CPU grid error within 1 cm on every line; the replicated sea state read back
as 1 on every world; the ocean visible in PIE, a client viewport's screenshot read. The fourteen
harness rows stay green.

**Fallbacks.** The float render target reading back nothing: a half-float target, whose
precision is a tenth of the bar at the amplitudes here. The Python-authored material failing to
compile: the Water plugin's ocean body, the brief's first route, with its clock overridden from
the frame. A dead end after these winds the session down.

**Coverage.** `ocean.agree`, covering determinism. **A trap filed**: the height-at-a-point
inversion and the surface normal, written for the Ship rung, are unasserted until the hull fit
uses them.

### Decisions

**A plane and a material of this project's own, not the Water plugin's ocean body**, taking the
brief's fallback first. The plugin's body wants a zone and an island spline, keeps a locally
accumulated wave clock, and evaluates its waves from a parameter texture; every one fights a
function of position and server time. **Alternative**: the plugin, for its shoreline, underwater
and buoyancy work. **Reopens** when a Stretch line needs one of those.

**The formulation is the engine's Gerstner shape**: wavenumber `2π/λ`, angular speed
`sqrt(980·k)`, phase `k·(P·d) − ω·t` wrapped by `frac` before the sine on both sides so a
long session's phase keeps its precision in single floats, horizontal displacement
`−Q·A·d·sin`, height `A·cos`. Amplitude and steepness scale linearly with the sea state;
wavelengths and the direction offsets from the wind are fixed per component. Time is the
shared frame over sixty as a single float.

**The GPU is read through a probe material**: a pixel's UV names a sample point on a grid, the
material writes the displacement plus an offset into a float target, and the CPU reads the
target back in the same tick it set the time. Identity is asserted by that readback, never by
the shared file. The probe takes its parameters as its own vector parameters on a dynamic
instance, set in the tick that draws it; the surface reads the collection, whose render state
the world flushes on its own schedule.

**A row may declare how far its injection pairing may move.** Under 5 percent loss at 100 ms the
pairing moved two frames within a row; `injection_tolerance` on the scenario, one frame by
default, three on the loss rows. **Alternative**: one tolerance for every row, which would have
loosened the lossless rows for the loss rows' sake.

### Verified against written

**Verified.** `ocean.agree` at 0, 50, 100 and 150 ms: on both clients, the server's CPU
displacement and the client's GPU displacement at four points and the same frame agree to the
two decimals printed, and the client's GPU against its own CPU over the 16 by 16 grid reads a
largest error of 0.001 cm and a mean of 0.0004 cm on every line; the replicated sea state read
back as 1 on every world; every mutation proven. The surface is visible: a client screenshot
from the floor's edge, `Saved/Screenshots/WindowsEditor/ocean_edge.png`, shows the displaced
plane with crests along its horizon. The fourteen harness rows green on the same binary, the
loss rows under a declared tolerance of three frames. Both checks pass.

**Written, not verified.** `FMOcean::HeightAt`, the inversion, and any normal: unasserted until
the Ship rung's hull fit reads them, the trap below. The ocean in a packaged client, which was
not repackaged after this rung. The collection's time reaching the surface material in the
same frame the probe measured; the probe reads its own parameters, and the surface is
presentation.

**Measured beside the bar.** The ocean rows' inbound bytes ran 12 000 to 14 000 B/s per
connection against 10 000 on the harness rows, the OCEAN relay lines added; measured lag on the
ocean rows ran 54, 95 and 133 ms at 50, 100 and 150, against 44, 77 and 108 on the harness rows,
the probe's synchronous readback once a second on each client the suspect, unconfirmed.

**Beyond the plan.** The comment checker's extractor blanked quoted strings before it looked for
comment markers, so an apostrophe inside a comment swallowed its closing marker and forty-five
lines after it; an unterminated quote is now retried as text. A per-scenario injection
tolerance, since 5 percent loss moved the pairing two frames within a row. The trace parser
accepts field names with digits. Nothing else.

## 2026-09-05 — Harness: the loop drives its first frames

### Next session's brief

**Pick up at the Ocean rung.** Write its plan entry first: the wave function in C++ and the
matching material function, the sea-state scalar and wind vector replicated once, wave time from
server time; the pre-registered epsilon, one centimetre proposed; the determinism row, protocol
two, as a scenario in `Tools/RegressionCheck/scenarios.py` with a row asserting server, client and
GPU agreement at sampled points and frames. Budget: none set; the designer winds down manually.
The editor is closed, the tree clean, every commit on the remote. Verified against written is
below the measurements.

### The plan, written before execution

**Scope.** The Harness brief in full, three sub-slices in order. **One**: the trace emitter, the
cost printout, the client relay, the marker hotkey and the session bundle in C++, with a Mover
character as the harness pawn; the loop's runner adapted to the pawn's key table; one rebuild.
**Two**: the two-world PIE settings, the runner proven against PIE, three smoke rows green at every
latency. **Three**: a packaged client against the editor as a dedicated server, the bundle
ingested by a new script into the same evaluator.

**Bar, pre-registered.** `regression-run.sh --all` exits 0: `harness.idle`, `harness.walk` and
`harness.jump` at 0, 50, 100 and 150 ms, twelve rows, each row's mutations proven and the
universal set clean. **Determinism**: the autonomous pawn on its own client against the same pawn
on the server, at the same simulation frame, within 1 cm at every matched sample in the second
half of every row, at every latency. **Injection latency**: constant within a row, reported in
frames. **Cost**: bandwidth per connection, server tick time and measured lag printed for every
row; no threshold, this is the first measurement. **Field**: one packaged Development client
joins the editor running with `-server`, stays ten seconds or more, and the bundle it leaves
ingests into a slice on which the universal set passes and at least one `MARK` is listed.

**Fallbacks.** The Mover pawn not moving under a dedicated PIE server within sub-slice one: the
trace stamps server time until Deck, the review queue's other option, filed as a trap. The
editor-as-server route failing: a listen server with emulated latency on the client, the brief's
fallback, filed as a trap. Packaging refused by the installed build: the field bar met with a
second editor instance as the client, filed as a trap. A dead end after these winds the session
down.

**Coverage.** The three scenarios above, each naming the mechanics it asserts; the coverage map
fills from them. **Budget.** None set; the designer winds down manually.

### Decisions

**The harness pawn is a Mover character from this rung.** The trace's frame is the prediction
framework's own: the world manager's pending frame plus its server offset, one definition on
every world, so a client stamps the server frame it is predicting. **Alternative**: server time
until Deck, which would have stamped an estimated clock, the two-clocks trap in a new coat, and
rebuilt the harness's proof at Deck. **Reopens** if the pawn fails sub-slice one's fallback.

**Input is key state polled when the input command is authored**, from a key table on the pawn,
no Enhanced Input assets. The runner reads the same table off the class default object, so a
rebind moves the fixture with the game. **Alternative**: mapping-context assets, created and kept
through the editor's silent-failure surfaces for no value the simulation reads. **Reopens** when
a rung needs an input no key state expresses.

**The world tag** is `S` for any server and `C<n>` for a client, `n` the PIE instance under one
process and the `-FMClient=<n>` switch on a packaged client, `1` without it. **A client relays its
trace to the server's session file**, and to the server's log only outside PIE, where the process
log already carries every world once. **The bundle** is a directory per world under
`Saved/Fathom/Sessions/`, PIE worlds under `PIE/` and the rest under `Field/`: the trace and a
`meta.json` rewritten every ten seconds and at the end. **`POSE` lines** carry the simulation frame the pawn last finalized and its sync-state
location, the determinism row's evidence; the actor's transform is presentation.

**Code is authored to be read and reviewed by human collaborators**, the designer's ruling of
2026-09-05, now in `CLAUDE.md`: no smothering in comments, the code speaking for itself.

**The runner counts in the server's simulation frame.** Its first drive counted server game time
in sixtieths, and the rounding put a one-frame spread on the injection pairing. Now a row's frame
zero is the server's shared frame at `BEGIN`, so the `INJECT` marker and the `INPUT` line sit on
one timeline. **The universal set allows the pairing to move by one frame within a row**: at zero
latency it moved once between a press at frame 60 and its release at 180, with no fault logged;
the prediction framework throttles an autonomous client's simulation frequency to keep the server's
input buffer fed *(headers, 2026-09-05, `NetworkPredictionWorldManager.cpp` lines 98 to 106)*, one
frame at a time. Two frames still fail. **The frame rate is capped at the fixed rate for a run**:
free-running, the editor played 6.5 game seconds in 4.1 wall seconds, and an emulated lag counts
wall milliseconds, so uncapped it would shrink in frames.

**The pawn's placeholder is the engine's cylinder**, the engine shipping no capsule shape
*(engine content, 2026-09-05)*; a capsule from Geometry Script is a Stretch line.

### Verified against written

**Verified.** The matrix: `regression-run.sh --all`, fourteen rows at 0, 50, 100 and 150 ms with
5 percent loss on two of them, every row green, every mutation proven, run again on the binary
that ships. The runner's every route the trap named: worlds by PIE instance, the player-id
match, `NetEmulation.PktLag` and `PktLoss`, teleport through the simulation, keys through
`UFMInputTools`, the fixed clock through `UFMTimeTools`. The trace from three worlds with one
frame definition; the relay, 224 client lines in a PIE server's bundle and the field client's
1 172 in the server's log; COST per connection; POSE from every world; the hotkey `M` as
`MARK category=hotkey` at frame 590 of the field session. **The field session**: the editor
with `-server` on the harness map, the packaged Development client from `Saved/Packaged/Windows/`
joining as `C1` (`Join succeeded` in the server log), 23 COST samples over the session, the
bundle ingested by `ingest_bundle.py`, the universal set passing on it. The field numbers:
lag 31 ms on the loopback with the server ticking at the engine's 30 Hz default, server tick
2.40 ms, inbound 30 000 B/s against 2 600 out, the client's frame rate uncapped. Both checks pass.

**Written, not verified.** `FM.Bundle` and `FM.Mark` as console commands, which nothing typed;
`meta.json`'s `connections` on a client, always empty by construction; the field script's
graceful server stop, which `taskkill` without `/F` delivered once. The packaged client's own
bundle under its Saved directory was not read.

**Beyond the plan.** A fourth scenario, `harness.walk-loss`, at 5 percent loss; `run_report.py`
for the lead and lag per row; the asset-manager rule for the game-feature data type in
`Config/DefaultGame.ini`, without which the cook counts an error; the session bundles split into
`PIE/` and `Field/`. Nothing else.

### Measured

Run `0905-010830`, fourteen rows in 57 s of wall time, every row green, twenty-eight mutations
proven, the universal set clean throughout. Per connection, two players, one process:

| Round trip | Lag measured (ms) | Client lead (frames) | In / out (B/s) | Server tick (ms) |
|---|---|---|---|---|
| 0 | 6 to 7 | 6, once 7 | 10 500 to 11 000 / 9 100 to 9 500 | 0.52 to 0.57 |
| 50 | 44 | 10, once 11 | 10 000 to 10 600 / 8 900 to 9 600 | 0.54 to 0.55 |
| 100 | 76 to 77 | 13 | 10 100 to 10 700 / 9 300 to 9 500 | 0.53 to 0.56 |
| 150 | 108 to 109 | 16 | 10 300 to 10 600 / 9 400 to 9 700 | 0.53 to 0.56 |

The server tick is the server world's actor tick alone, 0.27 ms per player. **Determinism**: the
autonomous pawn on its client against the same pawn on the server at the same simulation frame,
every matched sample in the second half of every row within 1 cm, idle, walking and jumping, at
every round trip and at 5 percent loss. Walking 120 frames covered 1 000 to 2 000 cm on the server
at every latency; the jump rose 80 to 160 cm and came back to Walking within two seconds.

## 2026-09-04 — The repository stands alone, and forgets its parent

### Next session's brief

This session continues after a touch-base; the brief below the plan holds until then. **Pick up
at the Harness rung**: its brief now builds on the loop skeleton in `Tools/RegressionCheck/`,
which has driven no frame. Write the Harness plan entry first.

### The plan, written before execution

**Scope.** The designer's ruling, quoted with the name elided: *"this repo needs to be able to
survive without any form of knowledge of [the parent project], and then needs to forget about
[it]."* Three sub-slices, in order: the loop's project-independent skeleton ported into `Tools/RegressionCheck/` with this
project's names and none of the parent's scenarios; the reference model dissolved into the homes
its rulings already had; every mention of the parent, its symbols, its assets and its history
removed from the working tree, the parent's target and DLL names in the build commands included.

**Bar.** A repository-wide search for the parent's name, its symbol prefix, its asset names and
its commit hashes finds nothing outside git history, quoted below. Every path a doc names
resolves in this repository. The loop's self-tests pass, every universal assertion having failed
once on purpose. Both checks pass.

**Fallback.** None needed for an editing job. If a loop file cannot be made standalone in the
session, it is left out and the Harness brief says what it still has to build.

**Coverage.** No game capability is added, so no scenario is. The loop skeleton's own self-tests
are its coverage; a dated trap below records that it has not driven PIE.

### Decisions

**The reference model dissolves.** The designer's ruling: *"the reference model's structure as
some sort of immutable sacred text is a bit overzealous, I suspect ... your call."* Taken:
dissolved. Every ruling in it already governed from a working home; the two that did not, the
hit-model tradeoffs Melee measures and the resources available on request, go to the Melee
brief. Its mission statement stands below, one sentence about inheriting from the parent removed on the
designer's word that the words are theirs to sterilise. The ownership check, the hardening status and the file go.
**Alternative.** Keep it as the verbatim source every transcription is checked against.
**Reopens** if a transcription is found to have added something no entry supports.

**Dated entries are edited, name only.** The designer's ruling, *"Change it to your needs,"*
over the log's never-rewrite rule. Nine lines change the parent's name to *the parent project*
and nothing else; no decision text moves.

**The findings archive is trimmed to what this project can use**, on the designer's ruling that
anything signed off as useless may go. Kept: engine-surface facts, PIE driving, clip, notify,
curve and skeleton authoring for Melee, the MCP registration facts. Dropped: the ability system,
the parent's assets, the narrative of the parent's own audits. The commit lists the headings
dropped.

**The loop skeleton is standalone infrastructure**, the designer's choice of the two offered.
What it is: the orchestrator, the in-editor runner, the evaluator, the preflight, the matrix
generator, the scenario schema and its validator, written against this project's trace shape.
What it is not: proven. It has driven no frame.

**No ability system in the simulation spine.** The designer's word, 2026-09-05: the constraint in
`CLAUDE.md` was manufactured, the Gameplay Ability System is welcome, and the choice is the agent's.
Taken on the spine's terms. Combat state lives in the Mover sync state so that a feint is a rollback
and every timing is a frame; GAS predicts by prediction key against wall time and confirms by RPC,
a second prediction model on a second clock, and every protocol timing would have to be bridged
between them. The stripped combat carries one attribute and no effect worth a framework.
**Alternatives.** GAS for the whole of combat with hand-rolled rewind, the Lyra shape; or a hybrid,
attributes, effects and cues in GAS applied on the server as the result of a hit the simulation
resolved, the timing-critical core staying in the sync state. **Reopens** with Route B at the Deck
recon, where Character Movement and a timestamp-keyed combat component are GAS's own ground, or
when a rung's plan finds the hybrid cheaper than hand-rolled attributes. The findings the trim
dropped about GAS's scripting surfaces are in git history before `fa7ba94`.

### Verified against written

**Verified.** The bar's search, run 2026-09-05 over the working tree with `.git`, `Binaries`,
`Intermediate`, `Saved`, `DerivedDataCache` and `Content` excluded, the name itself elided here
and only here:

```
grep -rn -i -E '<the name>|\bU?TD[A-Z][A-Za-z]+|TD\.Debug|LogTDCombat|/Game/<the name>|L_CombatTest|ABP_Combat|GA_(Attack|Block|Parry)|AM_(Attack|Dodge)|AS_Sword|IMC_Combat|IMC_Default|BP_TrainingDummy|BP_PlayerCharacter|Anim-Pipeline|Combat-Spec|Combat-Values|ValuesSnapshot|ClipScan|AnimPipeline|SkeletonCheck|casc-run|regression-check\.sh|20121e3|1391d54|ue_seed_cells|ue_chart_ab|ue_ab_metrics' .
```

finds nothing. `docs-check` passes with no WARN, 25 pointers resolving and its self-test at 27
assertions; `comment-check` passes with the two block-size WARNs the last closedown judged. The
loop's self-tests pass, 16 assertions in the evaluator and 4 in the rows, each universal row having
failed on one deliberate corruption; the offline preflight passes on zero scenarios; every loop
file compiles under the engine's interpreter; the matrix generator fills its regions and
`docs-check` reads them fresh. The toolset snapshot was regenerated from this editor's live
registry through the bridge, 52 toolsets against the inherited 56: the four gone are plugins this
project does not enable. **The push trap is discharged**: the reflog shows five `update by push`
entries from this clone on 2026-09-04, and the bootstrap session's transcript carries its push
tool calls with `main -> main` results. **The three headers newer than the DLLs** were comment
edits: the comment baseline committed before them recorded nine and twelve comment lines on two
headers that now carry one each, and one more word on the third.

**Written, not verified.** `ue_regression_runner.py` in full: the world addressing by PIE
instance, the player-id match, `NetEmulation.PktLag` as the latency knob, the live read of the
play settings in the orchestrator's preflight. The trap above names them. Nothing has driven PIE.

**Beyond the plan.** The comment baseline was regenerated: three rows fell to the files' current
counts, one row fell with the ownership check's removal, and the new entry script gained a row.
Nothing else.

### The mission statement, sterilised

> The core assertion is to reverse engineer/recreate the ocean from Sea of Thieves from a
> functionality standpoint as faithfully as possible. What that means is the physics that drive
> ship combat, in a live, networked, and fully replicated environment. The ships themselves can be
> minimally detailed placeholders, but they themselves need to carry the behavior of standing on a
> ship in Sea of Thieves, except elevated to a standard where skill-based melee combat, also
> replicated, can be performed on their decks while they are moving. The combat will be
> Mordhau-shaped from a technical perspective. You and other Fable 5.1 instances will be in
> charge of attempting to build it, and success is not expected, but is desired. I am aware of
> the ambition of what I am asking for, but the goal is to attempt to build it anyway rather than
> let intimidation discourage even attempting it.

## 2026-09-04 — The wind-down closes the editor

### Next session's brief

Unchanged from the bootstrap entry below: **pick up at the Harness rung**, write its plan entry
first — the frame source before Deck, the smoke rows, the pre-registered bar — then build. No
budget is set; the designer winds sessions down manually. The editor is closed, the tree is clean,
and every commit is on the remote.

**Decision**, the designer's, asked as a question and ruled the same evening. The parent project leaves its
editor open at closedown because a session winding down there does not end the designer's own;
Fathom's sessions are unattended and sequential, so the closedown ends by quitting the editor
gracefully. **Why**: every session starts from one state, editor closed and restore file empty,
which the startup check reads cleanly; an idle editor held nearly four gigabytes on a machine the
designer reclaims; both projects' MCP servers bind the same port; and any header change closes it
anyway. **Cost**: a launch of a minute or two next session, with the bridge and the Python runner
reaching the editor before any tool registers. **Reopens** on the designer's word, and the
exception is written in: present and saying so, they keep it open.

## 2026-09-04 — Fathom is bootstrapped: the spine, the regime and the inheritance

### Next session's brief

**Pick up at the Harness rung.** Write its plan entry first: decide the frame source before Deck
(a Mover pawn from this rung, or server time until then), list the smoke rows, pre-register the
bar in `Docs/Decisions.md`'s brief, then build. Budget: none set; the designer winds sessions down
manually and reclaims the machine as needed. Verified versus written is at the end of this entry.

### The spine

**Decision.** The ocean is a pure function of position and server time; the ship is a
deterministic kinematic body reconstructible at any past frame from compact replicated state;
players simulate in ship space; hits resolve on the server against rewound bodies and a rewound
ship, keyed by a rendered frame the attacker authors into its input command. **Alternatives.** A
Chaos rigid-body ship with engine physics prediction: ramming for free, at the cost of
determinism, smooth client reconstruction and free rewind, on an experimental path whose own
README says physics pawns desync against non-physics actors. **Reopens** if the Deck recon fails
its bar and its fallback. **Measurement.** None yet; the 5.8 headers were read, and their facts
are filed as traps.

### Route A

**Decision.** Combat lives inside the Mover simulation: presses and the rendered frame in the
input command, attack state in the sync state, feints as rollback, the blade path baked from the
clips as data so the server never needs a skeletal pose. **Alternative.** Route B, Character
Movement with a custom replicated combat component and timestamp-keyed rewind buffers. **Reopens**
at the Deck recon, which runs both routes on the same scenario against one pre-registered bar.

### The hit-timing knob

**Decision.** The server may start an attack early by a fraction of the round trip, capped; the
Melee rung measures the three known settings under emulated latency and the parry rule with
them. The designer's accounts of Mordhau's model and of their own project's half measure are in the
Melee brief. In the shared timeline the arithmetic is exact: with the
client ahead by a margin, an advance of the round trip plus that margin confirms hits at the
moment the local release touches the target, and lands the server's release half a round trip
before the true one; half the round trip lands it on the true time. **Alternative.** Choosing the
rule by argument. **Reopens** never; it is a knob.

### Basic first

**Decision**, the designer's: every feature starts at its most basic version, and richer variants
go to a Stretch rung, one dated line per deferral. The basic set is in `Docs/Spec.md`, the
deferrals in the Stretch brief, and the row-by-row corrections are the review queue's first item.

### The regime

**Decision**, the designer's, quoted: *"Are greenlights even really necessary? All that's really
necessary is the ability to interrupt and wind down when needed"*, and *"Still provide the
designer all the WHATs and WHYs, but they're no longer blocking."* So: no greenlights; a written
plan before execution that nobody approves; the stop list; the review queue; agent-triggered
wind-down on four conditions; main green with unverified work on a `wip/` branch; one session at
a time. **Alternative.** The parent project's interrupt-on-WHAT regime, which suits a designer who owns every
WHAT and does not suit an experiment whose WHATs are settled. **Reopens** on the designer's word.

### The field log

**Decision.** The designer's requirement, quoted: *"there needs to be some sort of log that can
find its way back to an agent for processing, to be used for debugging anything experienced in a
multiplayer session with zero connection to a coding session or an editor."* Protocol seven is the
answer; its shape is in `Docs/Debug-Instruments.md` and the Harness rung builds it.

### The reference model's ownership

**Decision**, the designer's: the verbatim form. *"I'm willing to go with verbatim. The end goal of
this project is whether you can build it, not if some specific spec is followed to the letter."*
and *"a clause simply stating 'read this, but don't touch it' after we harden it is something I
think is safe enough to trust."* The status line and `docs-check`'s ownership rule are that clause.

### The inheritance

**Decision.** Inherited by manifest, not by copy: the working rules where general, rewritten into
`CLAUDE.md`; `Docs/Working-In-Unreal.md`, `Docs/Unreal-Findings.md` and `Docs/Closing-Down.md`
carried over with names swapped, an inherited note, and the ability-system traps pruned;
`Tools/DocsCheck`, `Tools/CommentCheck`, `Tools/McpBridge` and the editor Python runner carried
over; the input and clock tools ported as `UFMInputTools` and `UFMTimeTools`; the multiplayer
traps re-filed here. **Not inherited**: the combat spec, the combat code, the animation library
and pipeline docs, and the regression loop, which the Harness rung builds from the parent project's as
reference. **Adjusted**: comment volume growth warns rather than fails; golden skeletons wait
until the surface stops moving; entries carry a template; "play wins" reads "measurement wins";
the clip-fits-the-duration rule is a default. **Reopens** if a pruned trap bites here.

### Choices taken alone, in the review queue

Lean renderer settings, the simulation rate, the untouched net tick rate, the harness map's
contents, the comment-volume demotion, and the frame-source question for Harness. The map also
carries a directional light, a sky light and a sky atmosphere beyond the floor and two player
starts described, so that a screenshot shows anything.

### Verified in the bootstrap, against merely written

**Verified.** The editor target builds: `Result: Succeeded`, 57 s in the build accelerator, and
both DLLs newer than every source with no patch files. The editor opens the project. The harness
map was created and saved through the remote-execution pipe, six actors, and `L_Harness.umap`
is on disk. Play-in-editor ran on it: the game world reported `FMGameMode` and
`FMPlayerController`, and play ended cleanly. The MCP server, started by console command,
answered the bridge with the registry and the current level; relaunched with the per-user
auto-start setting written, it started on its own, opened the harness map by default, and
answered again. Both check scripts pass their
self-tests, 32 and 24 assertions, and pass on the repository; the pre-push hook rejects a fake
deletion and a fake rewrite and accepts a new branch. Both repositories carry a ruleset blocking
deletions and force pushes on the default branch, read back through the API.

**Written, not verified.** The prediction settings in `Config/DefaultNetworkPrediction.ini`,
which no Mover actor has exercised. The two-world recipe, adapted from the parent project's measurements
and not run here. The trace, the field log and every rung bar, which are designs. The autonomous
push, which passed the harness once with nothing to push and has never sent a commit.

### Machine facts

The GitHub CLI is installed at `C:/Program Files/GitHub CLI/gh.exe`, logged in as the designer,
without a git credential helper of its own: git pushes still go through the credential manager
exactly as before. The MCP server's auto-start is a per-user editor setting, written to
`Saved/Config/WindowsEditor/EditorPerProjectUserSettings.ini` and gitignored, so a fresh clone or
machine starts without the server; `ModelContextProtocol.StartServer` starts it in a running
editor. Blender 3.6 is installed; the 3.4 folder is a leftover.

### Beyond what was described

The light and sky actors above; the Geometry Scripting plugin enabled, since placeholders are its
job; the `Terminal` plugin the parent project enables was left out. Nothing else.
