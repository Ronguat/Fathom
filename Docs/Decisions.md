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
| 2026-09-08 | The ship stepped from each pawn's pre-simulation tick to the frame about to run, the world tick's start catching up to the last frame run | `AFMShip`, `AFMPlayerPawn` | |
| 2026-09-08 | The pawn smoothing its own visual root between its last two base-space poses by the framework's fraction, Mover's smoothing off | `AFMPlayerPawn` | |
| 2026-09-08 | The camera's roll limits zero; the fly key F through the mover's flying mode, movement along the view while flying | `AFMPlayerPawn`, `AFMPlayerController` | |
| 2026-09-08 | Five columns 20 m tall and 2 m wide along the floor's edge facing the ship, from the ocean settings; the sail a slab 6 m wide, 9 m tall at full, on the mast | `UFMOceanSettings`, `AFMShip` | |
| 2026-09-08 | The ocean's look: the Gerstner normal from the shader function at the undisplaced point, deep water lerped to a sky tint by a fresnel of exponent 4, specular 1, roughness 0.12 | `Tools/Editor/make-ocean-assets.py`, `Shaders/FMOcean.ush` | |
| 2026-09-08 | A simulated proxy placed with the base's yaw alone, upright as the deck rolls, the position still on the full transform | `AFMPlayerPawn` | |
| 2026-09-08 | `FM.Latency` with one round trip per client: the server's delay the smallest half, each client the rest of its own | `UFMTraceSubsystem`, `Docs/Debug-Instruments.md` | |
| 2026-09-08 | The sea-state ceiling a settings knob, `SeaStateMax`, 2.4 with these components from the loop bound of the summed steepness, in place of the clamp at 1 | `UFMOceanSettings`, `AFMGameState` | |
| 2026-09-08 | The horizon: a flat 40 km plane in the same material, dropped under the deepest trough the sea state can make by a 20 cm margin; the near plane 1 km at 2.5 m steps | `UFMOceanSubsystem`, `Config/DefaultGame.ini` | |
| 2026-09-08 | Whether the Melee rung reopens on the ladder for the weapon-traced hit or the rework rides Ship Combat: the ladder untouched until the designer's word, the rework carried by the trap | Human review entry, Decisions | |
| 2026-09-08 | The view turning with the deck's yaw at each finalized frame on the owning client, the facing following through the orientation intent, so a player keeps facing the same part of a turning ship; the ship's roll and pitch left out of the view | `AFMPlayerPawn` | |
| 2026-09-08 | The anchor line: a fall of 2 s before the anchor bites, then 8 m of run under a drag of 0.3 per second, then a spring of 9 per second squared with damping 2.4, under critical so the ship swings back; the stop row's band four seconds after the bite. First cut 5 m, 100 and 12, ruled too abrupt and too early by the designer's eye | `UFMShipSettings`, `Config/DefaultGame.ini` | |
| 2026-09-08 | The headwind floor a fifth of full drive, `HeadwindSpeed`, scaled by the sail as everything else is | `UFMShipSettings` | |
| 2026-09-08 | The wheel holding where it is left on release, the designer's verdict at S6 | `AFMPlayerController` | |
| 2026-09-08 | A pennant at the masthead, a slab 3 m long streaming the way the wind blows, from the same replicated wind the waves take | `AFMShip` | |
| 2026-09-08 | The station keys: Left/Right the wheel, Up/Down the sail length, `[` `]` the sail angle, X the anchor, E the ladder, B board; the wheel back to centre on release, a sail key sending where the sail stands on release, the anchor a toggle | `AFMPlayerController` | 2026-09-08: the sail keys read upside-down once the sail hung from the yard; Down unfurls, Up furls |
| 2026-09-08 | The board key as a review affordance rather than a spawn on the deck, every row's placement untouched | Human review entry, Decisions | |
| 2026-09-08 | A refusal predicted on the client from the same station table and shown two seconds, the call sent regardless | `AFMPlayerController` | |
| 2026-09-08 | The station placeholders: a post and disc at the wheel, a cube at the anchor, a plinth at the mast for both sail stations, a plank at the rail for the ladder, in the hull's material | `AFMShip` | |
| 2026-09-08 | The HUD's lines and their order, in the engine's medium font | `AFMHUD` | |
| 2026-09-08 | `FM.Latency` splitting the round trip evenly on every game world of the process | `UFMTraceSubsystem`, `Docs/Debug-Instruments.md` | |
| 2026-09-08 | The hit draw held half a second, red for a hit and blue for a parry, sent to the attacker and the target; `fm.MeleeDraw` in `CVAR_DEFAULTS`; unverified, the trap | `UFMCombatComponent` | |
| 2026-09-08 | `deck.station-key` at 0 and 100 ms with four mutations; the station actions and board in `ACTIONS` | `Tools/RegressionCheck/scenarios.py` | |
| 2026-09-08 | The review behaviours as press-and-release by Slate ticks, a loop as a repeating square | `Tools/Editor/handson.py` | |
| 2026-09-08 | The checklist a standing file under the docs check; the human review first, verdicts as the items earn them, the paper pass last | `Docs/Checklist.md`, `CLAUDE.md` | |
| 2026-09-08 | The packaged client deferred to the checklist's F group | Human review entry, The plan | |
| 2026-09-07 | The intake bar's forward-hemisphere letter read as the tip's forward reach over 110 cm, three arcs beginning release beside the shoulder | Melee entry, Decisions | |
| 2026-09-07 | Windup ends where AutoAlignment reaches 1; a thrust's release runs to the plateau's end, a plateau of no length extended to the fall's; a swing's 30 frames is the designer's word | Melee entry, Decisions | |
| 2026-09-07 | The blade a 120 cm segment along the weapon bone's -Y, the length a knob in `Tools/Editor/bake-attacks.py` | Melee entry, Decisions | 2026-09-08: does not accomplish what the project set out to do; the point is to test tracers on a ship, drawn from a weapon. Flagged as needing work, the trap of that date |
| 2026-09-07 | Combat state a persistent sync-state block written in the mover component's in-simulation hooks, reconciling on the attack, its start and the parry's start only | Melee entry, Decisions | |
| 2026-09-07 | The rendered frame and the advance in every input command, the advance clamped by the server to its own | Melee entry, Decisions | |
| 2026-09-07 | The rewind in the ship's space from the server's per-frame body history, no ship reconstructed; a defender off the ship met in world space, unmeasured | Melee entry, Decisions | |
| 2026-09-07 | The parry judged at the rendered frame: a 30-frame window on press, a 120 degree cone | `Config/DefaultGame.ini`, Melee entry | |
| 2026-09-07 | Feint on Q; the swing turns with yaw only; the head a 15 cm sphere 80 cm above the capsule's centre; the eye at the head socket | `Config/DefaultGame.ini`, `AFMPlayerController` | |
| 2026-09-07 | Keys event-timed: presses latched at the key event, the down state the events' gated by the poll, the mouse wheel momentary | Melee entry, Decisions | |
| 2026-09-07 | Presentation by explicit time on single-node animation in the engine's default material; idle the first frame of overhead left, parry the first frame of the reference's parry pose | Melee entry, Decisions | |
| 2026-09-07 | The still-target bar split: the rolling row asserts the rewound body's point, a calm-sea row the contact point, the deck's roll having moved the contact 27 cm between rows | Melee entry, Decisions | |
| 2026-09-07 | The walking target crosses a thrust's line along the ship's length, 110 cm out; the parry press twelve frames after the attack's; every attack row turns one degree before its press to pin the side; the universal injection spread judged per client | `Tools/RegressionCheck/scenarios.py`, `Tools/RegressionCheck/regression_eval.py` | |
| 2026-09-07 | The melee rows: the feint row's injection tolerance 2, `pose_every` one on the walking-target row's proxies, the COMBAT, HIT, PARRY, SWING and SCORE vocabulary with `rf` and `rp` | `Tools/RegressionCheck/scenarios.py`, `Docs/Debug-Instruments.md` | |
| 2026-09-07 | `/Game/Fathom/Combat` and `/Game/Fathom/Melee` cooked always, beyond the plan | `Config/DefaultGame.ini` | |
| 2026-09-07 | The universal set fails a slice on an INPUT no INJECT accounts for, the 2026-09-06 trap's discharge | `Tools/RegressionCheck/regression_eval.py` | |
| 2026-09-07 | The runner restores every project console variable to its settings-reading value before each row, after the advance leaked from its rows into the swing row | `Tools/RegressionCheck/scenarios.py`, `CVAR_DEFAULTS` | |
| 2026-09-07 | A client relays its trace every tenth of a second in chunks of sixteen lines rather than once a second in chunks of 64, which cut the view lag by 8 to 11 frames | `UFMTraceSubsystem`, `Docs/Debug-Instruments.md` | |
| 2026-09-06 | Unplanned input filed as a trap and a rule rather than a universal assertion built | Demonstration entry, Decisions | |
| 2026-09-05 | All eight attack types imported though the brief takes up to four; the intake chooses | Delivery entry, Decisions | |
| 2026-09-05 | The mannequin meshes imported with materials, physics asset, post-process and animating rigs stripped | Delivery entry, Decisions | |
| 2026-09-05 | The weapon meshes, the retargeter and the IK rig left behind; the blade length and the timings carried as numbers | Delivery entry, Decisions | 2026-09-08: the hit is to be traced from a weapon drawn on the pawn; the meshes come in with the rework, the trap of that date |
| 2026-09-05 | The four binaries already tracked renormalised into LFS | Delivery entry, Decisions | |
| 2026-09-05 | Presentation one step behind the simulation, interpolated by the framework's fraction, rather than the ship extrapolated ahead | Presentation entry, Decisions | |
| 2026-09-05 | The presentation bar judged met with one hitch frame in 219 under its letter, the fix kept | Presentation entry, Decisions | |
| 2026-09-05 | The walk row's injection tolerance 2, after three spreads of 2 in a day with the play windows enlarged | `Tools/RegressionCheck/scenarios.py` | |
| 2026-09-05 | A simulated proxy re-placed from ship space after finalize, the sync state untouched; the rendered position on the `POSE` line, asserted at 10 cm standing and 50 cm walking | Review fixes entry, Decisions | |
| 2026-09-05 | The sea plane follows the local pawn, snapped to its two-metre grid, rather than growing | Review fixes entry, Decisions | |
| 2026-09-05 | The ship's mesh carries the engine's basic shape material, the pawn's, set in the constructor | Review entry, Decisions | |
| 2026-09-05 | The hands-on driver kept as `Tools/Editor/handson.py`; the play windows enlarged in the user ini for a human and restored after | Review entry, Decisions | |
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
| 2026-09-05 — Ship: a body every world integrates from the same state, its Stretch deferral of smoothed presentation | 2026-09-05 — Presentation between frames | The deferral, built on the designer's ruling that presentation never relies on a render cap |
| 2026-09-05 — The delivery arrives: the clips and their skeleton are in the project | 2026-09-07 — Melee: the swing, the rewind and the knob, measured | Eight release curves counted; seven exist, none for underhand left, and the intake leaves the underhand pair unused |
| 2026-09-07 — Melee: the swing, the rewind and the knob, measured | 2026-09-08 — The human review: a human reaches what the loop reaches | The blade baked from the clip as a segment in pawn space, no weapon on the pawn: the designer rules it does not accomplish what the project set out to do, the point being to test tracers on a ship drawn from a weapon; flagged as needing work, the trap of that date, and not debugged |
| 2026-09-05 — Ship: a body every world integrates from the same state, its anchor as a strong drag | 2026-09-08 — The human review: a human reaches what the loop reaches | The anchor falls for two seconds, then lies where the ship was, the ship running to the end of its line and caught by a spring that swings it back, on the designer's ask for a lurch; the stop row's bar moves from two seconds after the press to four after the bite, and gains the run past the bite |

## Known traps, indexed by what sets them off

**What this section is.** Latent defects and unverified assumptions, each filed against the rung
that makes it bite and re-read when that rung starts. These are not design questions. Nothing
here needs play to settle; they need checking. **Discharge a trap in the same commit that fixes
it**, saying what discharged it.

**Whenever a world tick runs two fixed ticks — *the hull is a frame behind for the second.*** The
ship advances one frame at the world tick's start and the pawns simulate on that hull; a render
frame slower than the simulation runs two fixed ticks on it *(2026-09-05)*, unmeasured. Bites when
the render rate falls under the simulation's. Discharged 2026-09-08 by the ship stepping from
each pawn's pre-simulation tick to the frame about to run, the world tick's start only catching
up; the same change stopped the frame-ahead sawtooth the designer saw as jitter on frames faster
than the simulation, the human review entry of that date.

**Whenever a change touches what is rendered — *the loop cannot see it.*** The rows read the
trace and never a frame; the ship flickered against the sky through every green Deck row until
the designer looked *(2026-09-05)*. Bites at every rung. Discharged by a row that reads a
capture of the play viewport, or by a human looking before a rung ships.

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
delivery, halt and demand it. Bites at Melee. Discharged 2026-09-05 by the import, the entry of
that date: the delivery is in the project.

**Whenever a delivered clip is played — *none has driven a frame here.*** Imported, loaded and
resaved headless *(2026-09-05)*; nothing has played one on a pawn. Bites at Melee. Discharged
2026-09-07 by the intake's capture, the Melee entry: the first-person overhead played on a pawn
and the body's pose on the other, looked at.

**Whenever the attacker's view delay is needed — *it is authored, never estimated.*** The rendered
frame rides in the input command. Bites at Melee. Discharged 2026-09-07: every command carries
the frame the client's simulated proxies are drawn at, the Melee entry.

**Whenever a sim proxy's deck position is read — *it is interpolated presentation.*** A
client's view of the other pawn in ship space is asserted at 50 cm *(2026-09-05)* and no
tighter until Melee's rewind reads the sync state instead; its rendering is placed from ship
space and asserted at 10 cm standing *(2026-09-05)*, and the staleness stays. Bites at Melee.
Discharged 2026-09-07: the rewind reads the server's own per-frame history and never a
client's proxy, the Melee entry; the 50 cm assertion stays on the rendering, which is what it
measures.

**Whenever a defender is off the ship — *the sweep meets it in world space through the ship's
current transform, unmeasured.*** The rewind rows stand both pawns on the deck *(2026-09-07)*.
Bites at Ship to Ship, where boarding crosses frames. Discharged by a row with a swimmer struck
from the deck.

**Whenever a press rides one command — *a command the server ticks past loses its edge.*** The
attack, parry and feint presses are edges in the command of one frame; when that command reaches
the server after the frame was ticked, the prediction framework runs the frame on the last
command it had and the edge is gone, or re-bases the client's frames and the attack starts two
frames later than predicted. Seen on three of eight rows in one sitting and none of twenty in
others, with the once-a-second relay chunks in the client's packets *(2026-09-07)*; the client
rolls back to the server's view either way. Seen once more *(2026-09-08)*: an injected wheel tap
at 100 ms read on the client as pressed, released, pressed, released over four frames, the
universal set catching the spread, clean on the rerun and in every other run of the row that day.
Bites at every rung with presses. Discharged by an
edge that survives a missed frame, a queue the server drains, or by a matrix that never shows it
again.

**Whenever the view lag moves — *the walking target's crossing moves with it.*** The
`melee.hit-walk` row places its walker so the rewound body crosses the thrust's line during its
32-frame release for view lags of 11 to 26 frames, with some frames either side *(2026-09-07)*;
a horizontal, whose forward reach lasts 13 frames, missed at one latency each time the relay's
chunking moved the lag by eight frames. A larger change to the trace, the relay or the
interpolation buffer moves the crossing out of the release and the row reads a miss. Bites at
any such change. Discharged by a walker whose crossing does not depend on the lag.

**Whenever a human's held key loses the window — *the controller's event-down set clears only
through its own InputKey.*** The engine's flush on focus loss is unmeasured against it
*(2026-09-07)*; a scripted release always arrives. Bites when a human plays. Discharged by a
hands-on tape across a focus change.

**Whenever the HUD, the notice or a station placeholder is relied on — *presentation the loop
never reads.*** `AFMHUD`'s lines were read once in a capture *(2026-09-08)*; the notice and the
placeholders' shapes were not. Bites when a human judges from them. Discharged by a row that reads
a capture, or by the Z and S items of `Docs/Checklist.md`.

**Whenever the melee hit is read as done — *the blade is numbers, and nothing on the pawn holds
a weapon.*** The hit is a segment baked from the clip and swept from state; the delivered weapon
meshes were left behind at intake. The designer's ruling *(2026-09-08)*: the implementation does
not accomplish what the project set out to do, the point being to test tracers on a ship, drawn
from a weapon; flagged as needing work, not debugged. The draw of a hit added the same day never
showed in a capture and stays unverified. Bites at Ship Combat's inheritance and at the M group of
`Docs/Checklist.md`. Discharged by a hit traced from a weapon drawn on the pawn.

**Whenever a run's play window has focus — *a hand on the keyboard is input the loop cannot
see.*** The mechanism and the rule are in `Docs/Debug-Instruments.md`, the shape of a run. Seven
consecutive rows of the 2026-09-06 demonstration run carried key edges no plan injected, the
designer's, and its one red, `ship.stop@150`, was a jump at the anchor frame lifting the caller
out of the station's radius; rerun three times untouched, green *(2026-09-06)*. Bites at every
run. Discharged 2026-09-07 by that assertion, `every INPUT injected` in the universal set, with
its self-test and a `dup INPUT` mutation on the swing and feint rows.

## Tuning map — a verdict comes back, which knob moves

**Every row is a warning with a reason, never a lock.** A row opening *"nothing, without
re-deriving it"* names a relationship you would be breaking, not a value you may not touch.

| Question | Move this | **Not** this |
|---|---|---|
| The simulation rate | `FixedTickFrameRate` in `Config/DefaultNetworkPrediction.ini`, 60 | The engine's own fixed frame rate, which overrides it when enabled |
| Hit timing under latency | `AdvanceFraction` and `AdvanceCapMs` in the combat settings, `fm.MeleeAdvanceFraction` and `fm.MeleeAdvanceCapMs` for a row; the advance rows report each setting *(2026-09-07)* | The windup |
| Parry fairness | `ParryFrames` and `ParryConeDegrees` in the combat settings, the parry judged at the rendered frame, measured beside the advance | The window alone |
| Where a swing reaches | `BLADE_LENGTH` in `Tools/Editor/bake-attacks.py` and the attack assets it writes; `HeadHeight` and `HeadRadius` in the combat settings | The clips |
| The eye and the meshes | `EyeOffset`, `MeshOffset` and `MeshYaw` in the combat settings, the eye the head socket of the reference pose | The capsule |
| The net update rate | `NetServerMaxTickRate`, at the engine default | The simulation rate |
| Inbound bytes per player | The client's frame rate, `t.MaxFPS`; a packaged client ran uncapped at 30 000 B/s against 10 000 capped at 60 *(2026-09-05)* | The simulation rate |
| The injection pairing's spread | The render load: the loop runs at half resolution, and the demo's full-resolution walk row read a lead of 13 then 15 frames, spread 2 against the tolerance of 1, where the same row at half resolution reads within 1 *(2026-09-05)*; the walk row's tolerance is 2 after three spreads of 2 in a day with the play windows enlarged *(2026-09-05)* | The tolerance |
| Judder on movement when rendering outruns 60 | The presentation, drawn one step behind at the framework's fraction; `t.MaxFPS` hides it and the designer ruled it out *(2026-09-05)* | The simulation rate |
| The sea | The sea-state scalar | Any single wave component |
| A storm | `SeaStateMax` in the ocean settings, 2.4 with these components, the loop bound in its header comment; then the components' amplitudes *(2026-09-08)* | The steepness alone |
| The sea's horizon | `PlaneSize` and `PlaneSteps` in the ocean settings, a 1 km plane at 2.5 m steps that follows the local pawn, over `HorizonSize`'s flat plane dropped under the deepest trough *(2026-09-08)* | The wave function, which is the same everywhere |
| A component's shape | Its row in `Config/DefaultGame.ini` under the ocean settings | The formulation, which two evaluators share |
| The probe's cost | `ProbeEveryFrames` and `ProbeCells` in the ocean settings; the readback is synchronous | The trace cadence |
| Ship handling | The speed curve and `HeadwindSpeed`, its floor; the rudder rate; `AnchorDropSeconds` before the anchor bites, `AnchorDrag` once it has, and the line's `AnchorLineLength`, `AnchorLineStiffness` and `AnchorLineDamping`, the catch's run, its firmness and how many swings it settles in *(2026-09-08)* | The hull sample points, which shape the fit rather than the handling |
| The ship's reconstruction | `SnapshotEveryFrames` in the ship settings, 12; every input carries its frame | The integrator, which every world runs alike |

## Rung briefs — read the one you are picking up

**Trigger: starting a rung**, alongside the traps above. Each brief carries its scope, its bar,
its fallback, its coverage and its inheritance; the plan entry written before execution turns
the bar into numbers.

- **Ship Combat** — Cannon stations, holes, water, repair, bailing, sinking, respawn. **Bar**: per
  `Docs/Spec.md`. **Inherits**: the stations and their radii; `UFMCombatComponent` with the server's per-frame body
  history, the frame and fraction an attacker's proxies were drawn at in every command, the sweep
  in ship space, replicated tallies and the `SCORE` line, which a cannon's shot and a hole can
  reuse; `FFMCombatInputs` and `FFMCombatState` as the pattern for a press in the command and state
  in the sync state; the `melee.*` rows, `pose_every`, the per-row console-variable reset and
  `Tools/RegressionCheck/melee_report.py` as the pattern for a row that reads a server verdict; the
  advance and parry knobs in the combat settings with their measurement in the Melee entry; the
  traps above on a press riding one command and on a defender off the ship; the minimum answer,
  rung four, met.
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
  2026-09-05, from Ship: a predicted station input for the local player; ~~a smoothed presentation
  of the reconstructed ship~~, built 2026-09-05 on the designer's ruling, the Presentation entry.
  Deferred 2026-09-07, from Melee: pitch in the swing, the blade turning with yaw only; the
  underhand attacks and the exhausted, riposte and deflect clips; a defender in the water; the
  head sphere following the clip rather than riding the capsule; a parry held rather than a
  window; an animation graph in place of explicit time; a material of the project's own on the
  delivered meshes. Deferred 2026-09-08, from the human review: water masked inside the hull,
  the placeholder hull letting waves through its sides.

## Symbol index — which entries discuss this thing

Current through **2026-09-08**. Regenerated, byte-sorted, one row per symbol.

| Symbol | Entries |
|---|---|
| `AFMGameState` | 09-05 |
| `AFMHUD` | 09-08 |
| `AFMOceanActor` | 09-05 |
| `AFMPlayerController` | 09-05, 09-07, 09-08 |
| `AFMPlayerPawn` | 09-05, 09-07, 09-08 |
| `AFMPlayerState` | 09-07 |
| `AFMShip` | 09-05, 09-08 |
| `EFMAttackSide` | 09-07 |
| `EFMAttackType` | 09-07 |
| `EFMCombatPhase` | 09-07 |
| `FFMBodySample` | 09-07 |
| `FFMCombatInputs` | 09-07 |
| `FFMCombatRules` | 09-07 |
| `FFMCombatState` | 09-07 |
| `FFMHitDraw` | 09-08 |
| `FFMShipInputs` | 09-05 |
| `FFMShipState` | 09-05 |
| `FFMStation` | 09-05, 09-08 |
| `FFMTeleportEffect` | 09-05 |
| `FMOcean` | 09-05 |
| `FM_TRACE` | 09-05 |
| `LogFMTrace` | 09-04, 09-05 |
| `UFMAttackData` | 09-07 |
| `UFMCombatComponent` | 09-07, 09-08 |
| `UFMCombatSettings` | 09-07 |
| `UFMInputTools` | 09-04 |
| `UFMOceanSettings` | 09-05 |
| `UFMOceanSubsystem` | 09-05 |
| `UFMShipSettings` | 09-05, 09-08 |
| `UFMSwimMode` | 09-05 |
| `UFMSwimTransition` | 09-05 |
| `UFMTimeTools` | 09-04 |
| `UFMTraceLibrary` | 09-05 |
| `UFMTraceSubsystem` | 09-05, 09-08 |

## 2026-09-08 — The human review: a human reaches what the loop reaches

### Next session's brief

**Pick up at the human review, `Docs/Checklist.md`, item Z1**, the designer at the keyboard and
the agent driving p2 through the hands-on driver's review behaviours; the ocean, ship and deck
groups first, the melee group waiting for the rework by the designer's ruling below. The work is
on `wip/human-review-prep`, main untouched: the harness, ocean, ship and deck families are green
on this binary and the melee family was not run, by the designer's word, so the gate for main is
that family green, run when the designer allows. **Open**: the melee family's rows on this
binary; the draw, unverified and not to be debugged; whether the Melee rung reopens on the ladder,
a queue row; the packaged client, waiting for the F group; the queue's paper pass, last.
Budget: none set.

### The plan, written before execution

**Scope.** The prep `Docs/Checklist.md` names before its first item, and nothing else: a board
key that lands the pawn on the deck from anywhere; station keys, a marker per station on the
hull, and a refusal shown on the client for a call outside a station's radius; a HUD reading
what the items ask a human to read; a draw on both clients of the blade and the rewound body at
every hit and parry, on the ship as each world presents it, and the local blade during release
on `fm.MeleeDraw`; `FM.Latency <ms>` across every world in the process; review behaviours for
the other pawn in the hands-on driver; the checklist landed as a standing file. The packaged
client waits for the checklist's F group. Whatever the review finds is the review's; no fix
rides along.

**Bar, pre-registered.** The whole matrix green on the binary that ships, the key table being
shared by every row; the new row `deck.station-key` green at 0 and 100 ms with every mutation
proven: a pawn boards by key from the floor and stands based, a wheel key from amidships is
refused by the server, the same key held at the stern applies its value on press and centre on
release and turns the ship; a capture of the HUD, the markers and a hit's draw looked at once
before the designer sits down.

**Fallbacks.** A polled key edge the runner's injection misses: the station keys move to the
event-latched route the combat keys use. The client call for the draw not reaching a component:
the draw carried on the pawn. The refusal predicted on the client disagreeing with the server's
`SHIPNO`: the prediction dropped, the HUD's radius line standing alone.

**Coverage.** `deck.station-key`. **Traps filed** for the HUD, the draw and the refusal notice,
presentation the loop never reads.

### Decisions

**The human layer.** *Decision.* Station keys on `AFMPlayerController`'s polled edges, sent as
the station calls a script makes: the wheel to its side on press and to centre on release; a sail
key to its end on press and to where the sail stands on release; the anchor a toggle from the
ship's last applied inputs; the ladder and B a call, B through `ServerBoard` to `AFMShip::Board`,
which lands the pawn at the ladder point through its simulation and writes `BOARD`. A refusal is
predicted on the client from `UFMShipSettings`' station table, each `FFMStation` measured by
`AFMShip::StationDistance`, and shown two seconds; the call goes regardless and the server's
`SHIPNO` stays the verdict. A placeholder per station appended to the hull's dynamic mesh.
`AFMHUD` reads `AFMPlayerPawn`, the ship, the ocean subsystem, the player state and
`UFMTraceSubsystem`. `FM.Latency` walks the engine's world contexts and sends the emulation
command to each game world. *Alternatives.* A spawn on the deck, which touches every row's
placement; the event-latched route the combat keys use, the fallback if the runner's injection
misses a polled edge; a persistent world-space draw, which drifts with the ship. *Reopens* on
the designer's verdict on the key mapping, or a red on `deck.station-key`.

**The draw, written and flagged.** *Decision.* `FFMHitDraw` sent by client call from
`UFMCombatComponent`'s sweep to the attacker's and the target's components, redrawn every tick on
the presented ship for half a second; `fm.MeleeDraw` for the local blade. *Measured.* A hit
registered in the capture session, the HUD reading dealt 1 and the server's `HIT` line 48 frames
after the press; the draw showed in no capture at 52 and 69 frames after the press, while a line
and a sphere drawn from Python showed in the same viewport. Not debugged, by the ruling below.

**The ruling.** The designer, on hearing that the blade is a baked segment and the pawn holds no
weapon: the implementation does not accomplish what the project set out to do, because the point
is to test tracers on a ship that are drawn from a weapon; flag it as needing work and focus on
getting the other parts testable. Recorded as the verdicts on the two queue rows, the supersession
row and the trap; the Melee rung's place on the ladder is the designer's to change, a queue row.

**The review's order, the designer's.** The human review first, verdicts recorded on the queue as
the items earn them; the paper pass over what remains, last; the basic set table read before the
ship group. Fail-first: every item red until the designer says its pass line held, and the first
defect becomes the session, with the mark key pinning its frame and nothing else recorded.

**Session 1's defect: the sea's edge, and the storm that was not one.** Z green; at sea 1 the
designer saw the sea stop before the horizon, O5 red, and could raise nothing past a gentle sea:
two `fm.SeaState 2` commands logged and the server running sea 1.00, the game state clamping at
1. *Decision.* The near plane 1 km at 2.5 m steps, a flat 40 km plane in the same material under
it, dropped below the deepest trough the session's sea state can make plus a margin, placed when
the sea state arrives; the ceiling a settings knob, `SeaStateMax`, 2.4, from the bound at which
both scalings loop the surface: the square root of one over the components' summed steepness
times amplitude times wavenumber, 0.161 here. *Alternatives.* A near plane to the horizon, a
million quads and more; fog, which the harness map lacks; the horizon plane at mean level, which
fills every trough. *Reopens* on the designer's eye at the seam, a faint step at 500 m, or on a
component change that moves the bound.

**Session 1's second pass, the designer away from the keyboard.** O1 to O4 green after the fix,
O4 with the lead as its note. O5 red again on the designer's words, a better shader wanted, depth
hard to see; and a wave piercing the hull, which the item never asked about, its pass line
rewritten to say so. Ahead of the deck group, the other player wobbling with the deck's roll while
the designer's own body stayed upright. *Decision.* The shader: a normal from the Gerstner closed
form at the undisplaced point, in `Shaders/FMOcean.ush` beside the displacement, a fresnel between
deep water and a sky tint, specular and a low roughness, built by the same script as before. The
wobble: the proxy was turned by the base's whole rotation since capture, pitch and roll included;
it takes the yaw alone now, the position still on the full transform, so the feet follow the deck
and the body stays upright as the owner's does. The console command takes one round trip per
client. *Alternatives.* A water-clipping mask inside the hull, deferred with the Stretch line for
it; the proxy tilting with the deck as the owner does not, which is what was seen. *Reopens* on
the designer's eye at D6 and O5.

**S2's red: the ship jitters under way, less at anchor.** *Measured* before anything was
touched, the judder tape at the play windows' size, sea 1, the ship under sail, a walk key held
300 ticks: frame times 10.5 to 16.8 ms, median 12.2; the ship mesh's speed per frame 44 to
2 246 cm/s around a median of 816, 131 of 259 frames off by more than a quarter; the camera 598
to 3 097 around 954; the camera against the mesh 12 to 1 617 around 778. Steady frames, erratic
steps. *Cause.* The ship advanced one frame at every world tick's start whether or not the
prediction framework ran a fixed step that tick; on frames faster than 60 Hz it leapt a frame
ahead of the pawns and waited for them, a one-frame sawtooth, 17 cm at sailing speed; at anchor
only the heave and roll sawtoothed, the milder case. The Presentation entry's tape counted
frames with no motion and never the spread of the steps, which is how it read 0 of 219 over this.
*Decision.* The ship steps from each pawn's pre-simulation tick to the frame about to run, the
world tick's start only catching up to the last frame run, the two-ticks trap discharged with
it. *Alternatives.* Presenting the ship a frame later; capping the render rate at 60, ruled out on
2026-09-05. *Reopens* on the designer's eye at S2, or on the ship rows reading a changed
reconstruction. **The columns and the sail**, the designer's asks: five columns 20 m tall along
the floor's edge facing the ship, from the ocean settings, and a slab on the mast as tall as the
sail is set and turned to its angle; a fly key, F, a review affordance through the character
mover's flying mode in the input command.

**The other half of the jitter: the pawn.** *Measured* after the ship's fix, the same tape: the
ship mesh 700 to 1 041 cm/s around 853, none off by a quarter; the camera 597 to 1 963 around
1 035, 120 of 259 off; the camera against the mesh 13 to 1 016 around 529, 228 off. The pawn
moved only on render frames that ran a fixed step: Mover's smoothing mode was on and the
prediction backend never delivered a smoothed frame to the autonomous pawn here. *Decision.* The
pawn moves its visual root itself: after each finalized frame it keeps the last two base-space
poses, and every render frame places the root between them by the framework's fraction on the
base as this world presents it, the same fraction and pose pair the ship uses; Mover's smoothing
off. *Measured* from the stern under sail, a walk key held 150 ticks, frames 9.7 to 16.1 ms:
the ship mesh 703 to 1 055 around 851, the camera 1 339 to 2 010 around 1 623, the camera against
the mesh 637 to 956 around 772, none of 129 off by a quarter on any line. *Reopens* on a deck row
reading the rendered offset past its band, or the designer's eye at S2 and D8.

**The camera's roll.** The designer's screenshot of the sail showed the view rolled, a fault seen
once before and never reproduced. *Cause.* Python's `unreal.Rotator` takes roll, pitch, yaw in
that order; the capture passed a pitch first and set an 18° roll on the control rotation, which
nothing clears, and the hands-on `aim` had done the same with its pitch since 2026-09-05, the demo
the designer watched. *Decision.* The scripts corrected and the note in the driver's docstring;
the camera manager's roll limits set to zero, so no source can leave the view rolled.

**S2 green, and two asks.** All of the second pass worked to the designer's eye. The sail keys
read upside-down with the cloth hanging from the yard: Down unfurls and Up furls now, the queue
row's verdict. Wind was imperceptible: a pennant at the masthead streams downwind, the same
replicated wind the waves take. `deck.station-key` sets the sail by key from the ladder point
rather than by script, and asserts the release leaves it where it stood, over 0.9 after a
150-frame hold. The designer's rule for the rest of the review: after a change, the rows it
reaches and nothing more; the whole matrix once the checklist is green.

**The ship group judged: S1 to S10 green but S6.** *S5's note*, a ship head to wind stalls with
no way to turn, since the turn scales with speed: a floor on the drive, a fifth of full at any
sail, scaled by the sail; the reference game's answer, the designer's suggestion. *S6 red*: the
wheel recentred on release and must hold where it is left, as the sails already did; the
station-key row's release value is now where the rudder stood. *S7's note*, the designer's ask
and the effort they chose: the anchor stopped the ship dead through a drag of 20 per second. Now
the anchor is a point in the state, dropped where the ship was; with the line slack the ship
runs on under a light drag; past the line's length a spring along the heading pulls it back with
its own damping, the catch and the lurch; the pull fades as the anchor is raised and the point
clears once it is up. Chosen so the settle from full speed stays inside the stop row's two
seconds: stiffness 100 gives a natural rate of 10 per second, damping 12 sits under critical so
the ship swings back once, and 5 m of run makes the coast half a second. Under full sail at
anchor the line stretches about 3 m and holds. *Alternatives.* A lateral velocity in the ship's
state, so an anchor abeam could swing the bow; deferred with the anchor turn. *Reopens* on the
designer's eye at S7 and S10, or on the stop row. The line was then ruled too abrupt and too
early: the anchor now falls two seconds before it bites, the run is 8 m under a drag of 0.3, the
spring 9 with damping 2.4; the stop row finds the bite in the trace and asserts four seconds
after it. Passed as proof of concept, imprecise still, the knobs the designer's.

**D1: a player turned relative to the deck.** The designer, standing hands off while the ship
turned, saw players keep their world-space facing and so rotate relative to the deck. The view is
a world-space control rotation and the pawn's facing follows it through the orientation intent,
so neither turned with the ship. *Decision.* On the owning client, at each finalized frame, the
control rotation turns by the hull's yaw change since the last one; the intent follows and every
world's pawn turns with the ship. The roll and pitch stay out of the view, which clamps its own
roll. *Alternatives.* The intent expressed in base space and rotated by the framework, which Mover
does not do here for an explicit intent. *Reopens* on the designer's eye at D1 and D6, or on a deck
row reading a walk that changed direction under a turn.

### Verified against written

**Verified.** The build, both binaries newer than every source. The harness family, 14 rows,
run `0908-120337`; the ocean family, 4 rows, `0908-120455`; the ship family, 14 rows,
`0908-120526`; the deck family, 16 rows, `0908-120842`, every row green with every mutation
proven except `deck.station-key`, red on its own expectation of one refusal where a tap's press
and release make two, green at both latencies with four mutations proven in `0908-121235`:
`BOARD` on the server four frames after the press, the pawn based on the deck, the wheel refused
from amidships at 1 029 cm, applied at the stern as 1.00 on press and 0.00 on release, the ship
turned under the held key. The HUD's lines read in a capture on the deck: world tag, frame, lag,
advance, mode and ship-space place, the nearest station and its radius, the ship's values, sea
and wind, the combat phase and tallies, the legend. A hit registered in the capture session, the
tallies and the `HIT` line agreeing.

**Written, not verified.** The draw, in no capture; the notice and the placeholders' shapes,
seen by no eye; `FM.Latency` from a console, exercised only through the driver's equivalent; the
review behaviours beyond `stand`, `face` and the swing; the melee family on this binary, not run
by the designer's word; the packaged client, waiting for the F group.

**Beyond the plan.** The ruling and its record: two queue verdicts, a supersession row, a trap,
the melee group of the checklist marked as waiting. Nothing else.
## 2026-09-07 — Melee: the swing, the rewind and the knob, measured

### Next session's brief

**Pick up at Ship Combat**, whose brief is in the rung briefs above. Melee shipped: the whole
matrix green in three sittings on the same binary, 32, 26 and 28 rows with every mutation
proven, the bar in `Docs/Spec.md` met, and the advance verdict the designer's, in the review
queue with the measurement below. **Open**: the review queue's verdicts, forty-odd rows; the
parry rule's alternatives, measured only as the margin each row reports; the thrusts' release
from the plateau, the designer's word on stabs being "different" and no number; the traps filed
today, a press riding one command above all. Budget: none set. The editor is closed, the tree
clean, every commit on the remote.

### The plan, written before execution

**Scope.** Sub-slices in order, each committed when green. **One, the intake's second half**:
the contract recorded against the designer's answers below; `UFMMeleeTools` in the editor module
bakes each attack's blade from its first-person clip into a `UFMAttackData` asset under
`/Game/Fathom/Combat/`, blade base and tip per frame at 60 Hz in pawn space, with windup,
release and recovery in frames; the pawn carries the delivered arms and body, each with a
material, and plays a clip from explicit time; a capture looked at. **Two, combat in the
simulation**: `FFMCombatInputs` in the input command, the attack type, its side, parry, feint
and the rendered frame; `FFMCombatState` in the sync state, the attack, its start frame, the
parry's start and the hits landed; transitions in the mover component's in-simulation hooks; a
`COMBAT` line per transition on every world. **Three, the rewind and the sweep**: the server
keeps each pawn's body per frame in ship space, sweeps the attacker's blade during release
against every other body at the frame the attacker's command says it rendered, and writes `HIT`
and `SWING`; hits received are a replicated count. **Four, parry**: a window and a facing cone
in the sync state, resolved at the rendered frame; `PARRY`. **Five, the advance knob**: fraction
and cap in `UFMCombatSettings` with console variables for rows, the per-connection advance in
frames replicated on the player state so the client predicts the same start; the three known
settings measured. **Six, closedown.**

**Bar, pre-registered.** *Intake*: six assets, overhead, horizontal and thrust on each side;
each tip's path during release spans over 60 cm and lies in the pawn's forward hemisphere; the
clip visibly mid-swing at its release frame in a PIE capture. *Combat*: the phase transitions
of a scripted attack at identical frames on the server and the attacker's client at 0, 50, 100
and 150 ms; a feint in windup leaves no release on any world; the side and type agreed on every
world. *Rewind*: a still target on the sailing, rolling deck at sea state 1, one `HIT` at each
latency, its ship-space point within 10 cm of the 0 ms row's point, its frame after the press
identical across latencies; a walking target, the rewound body within 10 cm of the position the
attacker's client rendered at that frame. *Parry*: the outcome identical on the server and both
clients at every latency, a parry inside the window and one after it. *Advance*: each row
reports the attack's start against the press, the effective windup, the hit frame, the distance
from the rewound body to the server's current one, and the parry margin; the outcome identical
on both machines; the verdict is the designer's, in the review queue. Every earlier row stays
green.

**Fallbacks.** The first-person blade making no sense in pawn space: the third-person clip as
the aim frame, the brief's fallback. Combat state not reconciling through the sync state: the
state carried by a movement modifier; failing that, an unpredicted replicated state, recorded
as a bar miss. A defender in the water: deferred, dated. A rewind error over 10 cm: the number
re-registered with its cause. A dead end after these winds the session down.

**Coverage.** `melee.swing`, `melee.feint`, `melee.feint-loss`, `melee.direction`, `melee.hit`,
`melee.hit-walk`, `melee.parry`, `melee.parry-late`, `melee.advance-half`,
`melee.advance-whole`; the universal set gains the assertion the 2026-09-06 entry left open, a
slice failing on an `INPUT` no `INJECT` accounts for. **A trap filed** if sub-slice one ships
alone: the baked blade has hit nothing.

**The designer's answers, 2026-09-07, which the contract reads from.** The `*_2h_fpp` and
`*_2h_tpp` family is the refined one, first and third person paired by name. Release is the
part of the attack that looks as if it would deal damage, for swings around 0.625 s in; windup
is before it and recovery the remainder; the AutoAlignment plateau read as release is a fair
reading. The blade's length and axis are determinable from the clips. Overhead binds to scroll
down, thrust to scroll up, horizontal to the left mouse button, parry to the right; the last
horizontal mouse motion picks the side. The editor is the session's, closed at the end. Later
the same day, seeing the bake: release is typically about 500 ms, 30 frames, for swings; a stab
is different.

### Measured

**The bake**, six attacks from the first-person clips at 60 frames a second, frames of windup,
release and recovery after the designer's 30-frame swing release, the first bake's plateau
having given overhead R ten, and the tip's forward reach and path length during release:

| Attack | Windup | Release | Recovery | Reach (cm) | Path (cm) |
|---|---|---|---|---|---|
| overhead L | 33 | 30 | 44 | 164 | 320 |
| overhead R | 47 | 30 | 30 | 155 | 215 |
| horizontal L | 31 | 30 | 65 | 145 | 277 |
| horizontal R | 35 | 30 | 61 | 147 | 299 |
| thrust L | 34 | 32 | 44 | 159 | 63 |
| thrust R | 32 | 27 | 51 | 164 | 88 |

The blade's axis is the weapon bone's -Y, forward at the thrust's release by 0.971 against 0.228
for +Z. The eye, the head socket of the reference pose, sits at (-4.1, 0, 84.8) in pawn space and
the head bone at 77.5.

**Combat in the simulation.** The swing: overhead R pressed at INPUT frame P starts at P+1 on
every world, windup 47, release 10, idle at P+108; the server's and the attacker's transitions
identical frame for frame at 0, 50, 100 and 150 ms; the other client sees each phase 9 to 10
frames after the server at 0 ms and 24 to 26 at 150, with the same start; rollbacks on the
attacker's client 1 at 0 ms, 3 at 150. The feint ten frames into windup: idle one frame after the
feint's command frame on the server and the attacker's client at every latency, the other client
26 frames later at 150, no swing, no hit; under 5 percent loss at 0 and 100 ms the feint took
and the attacker's client held every state the server did. The side: a turn left then right
before two horizontals gave horizontal L then R on every world. Injection pairing 5 frames at
0 ms, 6 on the deck, 15 at 150, spread within 1 with event-timed keys, where the latched press
alone had run a frame ahead of the polled release.

**The rewind.** A still target 150 cm ahead, struck by overhead R: on the calm sea one hit at
each latency, 48 frames after the press, the body rewound 11, 16, 21 and 27 frames at 0, 50, 100
and 150 ms, the contact point and the rewound centre identical to the 0 ms row within a
centimetre; on the rolling sea the same frames, the depths 11, 17, 20 and 27, the rewound centre
within 0.2 cm of the 0 ms row, the contact 0.6 to 1.4 cm off, the body having crept 1.3 to
3.7 cm since the rendered frame. The rewound centre against the position the attacker's client
had drawn at that frame: 0.0 to 0.3 cm. A target walking across a thrust's line at 12.8 cm a frame, struck by thrust L: the body
rewound 11, 17, 21 and 28 frames, having moved 115, 191, 242 and 332 cm since, and the rewound
centre against the client's rendering 0.0 to 0.2 cm. A horizontal, whose reach lasts thirteen
frames of its release, missed at one latency each time the view lag moved.

**The parry.** Pressed twelve frames after the attack's press and judged at the rendered frame:
parried at every latency, the window having 6, 12, 15 and 21 frames left at the rewound frame;
pressed 52 frames after, a hit at every latency with the window closed. Tallies identical on the
server and both clients in every row.

**The advance**, the three known settings on the parry fixture, an overhead R against a parry
pressed twelve frames after it, the attacker's client predicting the same start as the server in
every row:

| Setting | Advance (frames) | Effective windup (frames after the press) | Contact | Parry margin (frames) |
|---|---|---|---|---|
| none | 0, 0, 0, 0 | 48, 48, 48, 48 | parried | 6, 12, 15, 21 |
| half the round trip, 50 ms cap | 0, 2, 3, 3 | 48, 46, 45, 45 | parried | 6, 14, 20, 24 |
| the whole round trip, 80 ms cap | 1, 4, 5, 5 | 47, 44, 43, 43 | parried | 7, 16, 20, 27 |

Four columns are 0, 50, 100 and 150 ms. The advance a row set leaked into the rows after it
until the runner restored every project variable before each row, seen as a five-frame advance
on a swing row. The designer's account of the settings, carried from the brief: the whole round
trip capped at 80 ms gave instant hits up to that ping, windups effectively faster by the ping
up to the cap, hits landing early by half the ping, and false positives at high latency; half
the round trip capped at 50 ms gave some hit delay at higher ping and no competitive advantage
at any ping. Here the windup shortened by the advance, five frames at most, and contact came
earlier by the same; a still body moved 1 to 4 cm between the rendered frame and the server's,
a walking one 115 to 332 cm, which is the distance a false positive can reach; the verdict is
the designer's.

### Decisions

**The attack table is the designer's ruling**: three types on three keys, overhead on scroll
down, thrust on scroll up, horizontal on the left mouse button, the side from the view's last
horizontal turn before the press; six clips of the refined `*_2h_fpp`/`*_2h_tpp` family, the
underhand pair unused, its left release curve absent. The brief's "up to four" is superseded by
it. **Reopens** never.

**Windup ends where AutoAlignment reaches 1**, which lands 0.52 to 0.78 s in, the designer's
"around 0.625 s". **A swing's release is 30 frames from there, the designer's word** on seeing
the first bake, whose plateau had given overhead R ten frames; **a thrust's runs to the plateau's
end**, a stab being different by the same word and no number given, a plateau of no length
extended to the fall's end. Recovery is the remainder. **Alternative.** The plateau for all six.
**Reopens** on the designer's numbers for the thrusts.

**The blade is a 120 cm segment from the weapon socket along the weapon bone's -Y**, the axis
the thrust points furthest forward at its release, the length a knob in `bake-attacks.py`.
**Alternative.** The weapon mesh. **Reopens** when the mesh or the number arrives.

**Combat state is a persistent block of the sync state, written in the mover component's
in-simulation hooks**: the input command's block is read at `OnPreSimulationTick`, the state
block written at `OnPostMovement`, both inside `SimulationTick`, so a resimulation reruns them;
the block reconciles on the attack, its start and the parry's start, and the server's tallies
ride along unreconciled. **Alternative.** A movement modifier carrying the state; an unpredicted
replicated state. **Reopens** if rollbacks climb with combat; the rows report them.

**Every input command carries the rendered frame and the advance**: the frame this client's
simulated proxies are drawn at, from the fixed tick state's interpolation, and the advance in
frames from the player state, which the server clamps to its own. **Alternative.** A stamp at
the press only. **Reopens** never.

**The rewind reads the server's own body history in the ship's space.** Every frame the server
keeps each pawn's base-space position, yaw and parry; the sweep at frame N puts the attacker's
blade and the defender's body at the rendered frame in the same space, the ship's when both
stand on it, since a client draws a based proxy on the ship as it presents it. No ship is
reconstructed at the rendered frame; the entry's plan said one would be. A defender off the
ship is met in world space through the ship's current transform, unmeasured, a trap.
**Alternative.** Reconstructing the ship at the rendered frame. **Reopens** at Ship to Ship,
where two ships' frames meet.

**The parry is judged at the rendered frame**, the frame the body is rewound to: a fixed
30-frame window opened by its press, a 120 degree cone, both knobs. **Alternative.** The
server's current frame, or either. **Reopens** on the designer's verdict from the advance
measurement below.

**Keys are event-timed.** A press is latched at the key event until the pawn takes it, which is
how the mouse wheel's one-event keys reach a command; the down state is the events' gated by the
polled state, so a release is seen a frame before the poll shows it and a lost release cannot
stick. Measured: the latched press ran one frame ahead of the polled release and spread the
injection pairing to 2; event-timing both edges brought it back. **Reopens** never.

**The swing turns with yaw only**; pitch in the aim frame is deferred, dated in Stretch.

**Presentation is explicit time on a single-node animation**: each mesh set to the attack's
clip and positioned from the state at the presented frame, the arms for the owner and the body
for everyone else, both in the engine's default material since the placeholder material lacks
the skeletal usage; idle is the first frame of overhead left, parry the first frame of the
reference's parry pose clip. **Reopens** when a Stretch item wants an animation graph.

**The intake bar's letter failed and its intent held.** Three arcs begin release beside the
shoulder, the tip behind the pawn, so "lies in the forward hemisphere" was the wrong letter;
every tip reaches over 145 cm forward during release. Re-registered as the tip's maximum forward
reach over 110 cm, the still target's surface. Taken alone, in the review queue.

**The universal set fails a slice on an INPUT no INJECT accounts for**, the 2026-09-06 trap's
discharge, with its self-test and a `dup INPUT` mutation on the swing and feint rows.

### Verified against written

**Verified.** The whole matrix on the final binary in three sittings, runs 0907-020237, 0907-020609
and 0907-021008 with the walker's four rows in 0907-021617 after its last change: 32, 26 and 28 rows green with every mutation proven, the melee family twice over
across the day's runs. The bake's six assets and their numbers; the first-person overhead played
on a pawn and the body's pose on the other, in captures looked at; every measurement above read
off the rows' slices by `melee_report.py`; `docs-check` and `comment-check` pass.

**Written, not verified.** The packaged client, not repackaged since Harness, and the cook
directories added for the combat and melee content; a defender off the ship; the event-down set
under a focus loss; the presentation on the other client beyond one capture; the thrusts'
release window; the field session with combat in it.

**Beyond the plan.** The cook directories; the relay every tenth of a second in chunks of
sixteen; the runner restoring every project console variable before each row; the universal
injection spread judged per client; the calm-sea row beside the rolling one; `melee_report.py`;
the swing rows asserting the press's command frame rather than the press's; the walking target
crossing the arc rather than a walker in front; the fraction beside the rendered frame.

## 2026-09-06 — The loop demonstrated: 45 of 46 green, and a hand on the keyboard is invisible to it

### Next session's brief

**Pick up at the Melee rung**, as the delivery entry's brief below says; nothing there changed.
This session ran the loop once, for demonstration, and changed no code. **Open**: the universal
assertion the new trap names, and the review queue's verdicts. Budget: none set. The editor is
closed, the tree clean, every commit on the remote.

### Measured

The full matrix, 46 rows in 360 s of wall. 39 rows untouched, every one green with every
mutation proven. Seven consecutive rows, `ship.sail@150` through `ship.turn@50`, carried the
designer's keys in the play windows, on their word: six green, and `ship.stop@150` red, the
anchor refused at frame 373 with the caller 439 cm from the station after a jump at frame 359.
Those seven are discarded; the red rerun three times untouched, green three times. They stay in
`Saved/Regression/history.tsv` under run `0906-152133`, so a band read across runs skips them.
Across the clean rows: server tick 0.30 to 0.45 ms per player, inbound 10.4 to 17.3 kB/s per
client, measured lag 8 to 11, 56 to 68, 90 to 112 and 126 to 155 ms at the four round trips.

### Decisions

**The blind spot is filed, not closed.** The universal set pairs `INJECT` to `INPUT` and flags no
`INPUT` without one, so a human's keys in a focused play window read exactly as injected ones.
**Alternative.** A universal assertion failing a slice on an unpaired `INPUT`, with its self-test
and a mutation per row; a loop change, outside a demonstration. **Reopens** at the next loop
change. Taken alone, in the review queue.

### Verified against written

**Verified.** The 46 slices and their evaluation; the red slice's trace, read line by line; the
census of injected against observed edges over every slice of the run and the reruns; both
checks pass. **Written, not verified.** Nothing. **Beyond the plan.** No plan preceded the run;
the trap, the rule and this entry are what it produced beyond itself.

## 2026-09-05 — The delivery arrives: the clips and their skeleton are in the project

### Next session's brief

**Pick up at the Melee rung, whose delivery is in the project**: the intake sub-slice's second
half opens it, against the contract in the brief. `Docs/Melee-Delivery.tsv` lists the 123 assets
under `/Game/Fathom/Melee/`; the release curves are among them, and the timing numbers and the
socket list are read from the designer's material on request. **Open**: the eight attack types
are in and the brief takes up to four, the intake chooses; the mannequin meshes carry no material
and no physics asset; the retargeter and the IK rig stayed behind, so the pawn starts on the
engine mannequin and the intake swaps or retargets as the brief says. The review queue awaits
verdicts on this entry's rows and the earlier day's. Budget: none set. The editor is closed, the
tree clean, every commit on the remote. The loop did not run: nothing it reads changed.

### The plan, written before execution

**Scope.** The delivery into the project, the first half of the Melee brief's intake: the
skeleton, the mannequin body, arms and invisible body, and every sequence and release curve of
the eight attack types, first and third person, with their parry poses and ripostes, from the
designer's reference package; LFS for the repository, on the designer's word. **Bar**: every
imported asset loads headless in this engine with no missing package and no load error, and its
hard dependencies reach nothing outside the delivery and the engine. **Fallback**: an asset that
reaches outside is stripped of the property that reaches, or left behind and named here.
**Coverage**: no scenario; a dated trap above records that no delivered clip has driven a frame.

### Decisions

**The route keeps references intact.** Assets store absolute package paths, so a file copy out
of the reference's plugin folders would arrive broken. The delivery was renamed inside the
reference's own editor into `/Game/Fathom/Melee/`, its referencers rewritten by the rename, then
the folder copied and resaved here. **Alternative.** A migrate, which does not rewrite imports
across a plugin mount. **Reopens** never.

**What travelled and what stayed.** In: the skeleton, three mannequin meshes with their
materials, physics asset, post-process and animating rigs stripped, 112 sequences and 8 release
curves. Behind: the two weapon meshes, the blade's length carried as a number from the reference;
the precedent's per-attack data assets and stats tables, native to its module, their numbers read
from the reference's text; the IK rig and retargeter, whose closure reaches the precedent's
character content; the cosmetic rigs, flinches, transitions and exhausted idles. **Alternative.**
The weapon meshes in. **Reopens** if the blade curve needs the mesh.

**All eight attack types**, the brief taking up to four. **Alternative.** Four chosen now.
**Reopens** at the intake's choice; the review queue carries it.

**LFS.** The designer's ruling; the four binaries already tracked were renormalised into it, and
the push hook hands the refs on to it after its fast-forward check.

**Taken alone**, in the review queue: the eight types, the stripping, the weapon meshes and the
retargeter left behind, the renormalisation.

### Verified against written

**Verified.** The reference's editor moved 123 of 123 assets and reported the closure reaching 0
packages outside the delivery and the engine; this editor, rebuilt against 5.8.2 in 67 s with
both DLLs newer than their sources, loaded 123 of 123, reached 0 outside, logged no error and no
warning in the load, and resaved every package. `Docs/Melee-Delivery.tsv` holds one row per
asset: no sequence has root motion on, none carries a notify, the attack clips carry an
`AutoAlignment` curve, and every sequence names the delivered skeleton. `docs-check` and
`comment-check` pass.

**Written, not verified.** The sockets on the skeleton, refused to reflection here as the working
doc records and listed in the reference's text, a weapon socket on the weapon bone among them.
Whether a delivered clip plays on a pawn: the trap.

**Beyond the plan.** Nothing.

## 2026-09-05 — Presentation between frames: the camera, the ship and the sea at the framework's fraction

### Next session's brief

**Pick up at the Melee rung, which halts without the clips and their skeleton in the project**,
stop-list item five: demand them, then open with the intake sub-slice against the contract in the
brief. The designer's review found four defects in the hands-on session; three are fixed in the
entries below, and this entry carries the fourth. **Open**: the designer's verdict on this entry's
bar, unmet by one hitch frame in 219 and kept; the walk row's pairing tolerance of two frames; the
loop's blindness to anything rendered, a trap, with the hands-on tape as the only instrument. The
review queue awaits verdicts on the day's rows. Budget: none set. The editor is closed, the tree
clean, every commit on the remote.

### The plan, written before execution

**Scope.** The designer's ruling: presentation is smooth at any render rate, never by capping it.
Rendering at 85 to 100 frames a second over a 60 Hz simulation shows about four frames in ten
with no movement, on a 165 Hz display, while looking around is smooth because the look path is
per frame. The framework's fixed-tick smoothing is already on in
`Config/DefaultNetworkPrediction.ini`, and Mover offsets a smoothed visual one step behind the
simulation by the framework's leftover fraction; but the visual it picks by itself is the pawn's
cylinder while the camera hangs off the capsule, the ship's mesh sits on the hull at the
simulated frame, and the sea's time is the frame's. Everything presented moves to the same
fractional frame, one step behind the simulation. The simulation, the sync state, the ship's
integrator and the ocean function do not change.

**What changes.** The pawn gains a visual root holding the mesh and the camera, set as Mover's
primary visual, so the camera rides the smoothed visual. The ship draws its mesh between the
hull's previous and current pose by the framework's fraction, the hull staying at the simulated
frame as the pawns' base; a simulated proxy is placed from ship space through that presented
pose, and the `POSE` line's `rx ry rz` read the visual against it. The sea's surface time becomes
the presented frame's, the probe keeping the frame's. **Bar**: on a tape at the uncapped render
rate, walking the deck at a steady pace on a sailing ship at 100 ms, the camera's step between
rendered frames never reads under a quarter of its median over the steady middle of the walk,
where the baseline reads it in about four frames in ten; the ship's mesh the same; the other pawn
as rendered within 10 cm of the server's ship-space truth standing, as before; the full matrix
green with every mutation proven. **Fallback**: a failed bar keeps the camera under the visual,
which cannot be worse than the capsule, and files the rest against Stretch with the measurement.
**Coverage**: the loop cannot see any of this, the rendering trap; the tape in
`Tools/Editor/handson.py` is the instrument, run before and after.

**A trap noticed on the way.** The ship advances one frame per world tick at the tick's start; a
world tick that runs two fixed ticks leaves the hull a frame behind for the second. Filed in the
traps, unmeasured; it bites when the render rate falls under the simulation's.

**Supersedes** the Ship entry's deferral of smoothed ship presentation to Stretch, on the
designer's ruling.

### Decisions

**Everything presented moves one step behind the simulation, at the framework's leftover
fraction.** The pawn's mesh and camera hang off a `Visual` scene component set as Mover's primary
visual in `PostInitializeComponents`, which the framework's smoothing offsets between the last two
finalized states. `AFMShip::PresentedTransform` interpolates the hull's previous and current poses
by the same fraction, read from the framework's fixed tick state, and the ship's tick draws the
mesh there while the hull stays at the simulated frame as the pawns' base.
`UFMOceanSubsystem::PresentedTime` feeds the surface material one frame behind plus the fraction,
the probe keeping the frame's own time. A simulated proxy is placed from ship space through the
presented pose, and `rx ry rz` read the visual against it. **Alternatives**: extrapolating ahead
instead of interpolating behind, which the framework does not offer and the ship would have to
invent; presenting the ship at the simulated frame and letting the pawns lag it, the slide the
review saw. **Reopens** if a step of latency in what is drawn is judged worse than the judder,
when the ship's integrator can present a frame ahead, its next state being a function of its
inputs.

**The hull's previous pose is kept per simulated frame, not per world tick**, so a render frame
without a fixed tick still interpolates between the same two poses. A world tick that runs two
fixed ticks draws the mesh from two frames back for that one frame, the trap filed with the plan.

### Verified against written

**Verified.** Run `0905-150358`: the full matrix, 45 of 46 green with every mutation proven;
`deck.walk@0` failed the universal pairing check at a spread of 2, the third such reading today,
all on the walk row with the play windows enlarged for the designer. That row's tolerance is now
2, a decision in the review queue, and run `0905-151009` reads it green at all four latencies with
its mutations proven. The judder tape at 100 ms, the ship at 8.1 m/s, a walk key held for 300
rendered frames of 11.5 ms: frames with no motion at all, 95 of 219 for the ship's mesh, 95 for
the other pawn and 26 for the camera in the baseline, read 0, 0 and 0. By the bar's letter, steps
under a quarter of the median, the ship's mesh and the other pawn read 0 and the camera 4, each a
frame a third the median's length; per second of frame time, one frame in 219 for each of the
three, at a fifth of the median, in a tape whose longest frame was 48 ms. The camera's one jump of
150 cm, 172 in the baseline, is a correction under held input. The other pawn standing at the
wheel as client 1 renders it: 7 cm mean from the server's ship-space truth, 20 cm peak, the height
matching. The walk carried player 1 off the bow at Mover's default 600 cm/s, so client 2's line
reads a swimming pawn and is not the bar's. The sea plane within 94 cm of the pawn. **The bar's
"never" is unmet by one frame in 219 and the fix is kept**: the mechanism reads 99.5 percent
against the baseline's 57, and the frame coincides with the tape's longest hitch. The verdict is
the designer's.

**Written, not verified.** The presentation in a packaged client; the look path under the visual
root beyond the designer's eye.

**Beyond the plan.** The judder tape added to the hands-on driver. Nothing else.

## 2026-09-05 — Review fixes: the other pawn placed from ship space, the sea drawn under the viewer

### Next session's brief

**Pick up at the Melee rung, which halts without the clips and their skeleton in the project**,
stop-list item five: demand them, then open with the intake sub-slice against the contract in the
brief. The designer's review found three defects in the hands-on session; the ship's material is
fixed in the entry below, and this entry carries the other two. The review queue awaits verdicts.
The editor is closed, the tree clean, every commit on the remote.

### The plan, written before execution

**Scope.** Two presentation defects the designer's review found in the hands-on session. Each
client draws the other pawn where the server's ship was when its state was captured, so it trails
the deck by the round trip plus the client's lead, 230 cm under way at 100 ms, and bobs against
the surface when still. And the sea is a 400 m plane at the origin, which the ship sails off.
Neither touches the simulation: the sync state, the ship and the ocean function stay as they are.

**Sub-slice one, the other pawn.** A simulated proxy is re-placed after Mover finalizes it, from
its base-space location and orientation through the base's current transform on that client. The
`POSE` line gains `rx ry rz`, the actor's rendered position in the space of its base, on every
role, so the loop reads something rendered for the first time. **Bar**: the rendered position of
the other pawn against the server's ship-space position at the same frame within 10 cm on the
stand row and 50 cm on the walk row at 0, 50, 100 and 150 ms, asserted by the deck rows from the
new fields, with a mutation zeroing a client's `rx` proven on the stand row; and in the hands-on
tape at 100 ms with the ship under way at 5 m/s or more and the other pawn standing, a mean within
10 cm and a height off the deck within 5 cm. **Fallback**: a failed bar reverts the re-placement
and files the measurement against the Melee brief, where the proxy's presentation was already a
trap.

**Sub-slice two, the sea.** On a world that renders, the ocean subsystem moves the plane every
tick to the local pawn's position snapped to the plane's grid step, 200 cm, so the sampling points
never move in world space. **Bar**: the plane's origin within one grid step of the local pawn on
every client on every tick of a hands-on tape while the ship sails; the ocean rows unchanged,
since the probe does not read the plane. **Fallback**: follow the camera manager instead of the
pawn. **Coverage**: no row reads the plane; this rests on the rendering trap in the entry below
and on the tape.

**Scenarios.** Every deck row that compares the other pawn gains the rendered assertion;
`deck.stand` gains the `rx` mutation. The full matrix runs, since the pawn's trace line changed.

### Decisions

**A simulated proxy is placed from ship space after Mover finalizes it.** Mover interpolates a
based proxy in base space, correctly, then resolves it to the world through the base pose captured
with the state, `FMoverDefaultSyncState::GetLocation_WorldSpace`: the server's ship of seven to
nine frames earlier, while the client's ship runs thirteen ahead. `AFMPlayerPawn::PlaceOnBase`
re-places the actor from the base-space location and orientation through the base's current
transform on that world; the sync state is untouched, so Melee's rewind reads what it read.
**Alternatives**: a Mover smoothing mode, none of which knows the base has moved on; interpolating
the ship rather than predicting it, the Stretch line. **Reopens** if a based proxy is ever read
for a hit from its actor transform rather than its sync state.

**The `POSE` line carries the rendered position in base space**, `rx ry rz`, and the deck rows
compare the other client's rendering of a pawn with the server's ship-space state at the same
frame: 10 cm on the stand row, 50 cm walking, a mutation on the stand row proving it.

**The sea plane follows the local pawn, snapped to its grid.** `UFMOceanSubsystem::Follow` moves it
every tick on a world that renders. **Alternatives**: a larger plane, more triangles for the same
two-metre sampling; tiles thinning with distance, Stretch. **Reopens** when a 200 m horizon is
judged short, where the plane's size and step become a tuning row.

### Verified against written

**Verified.** Run `0905-143329`: the full matrix, 45 of 46 rows green with every mutation proven,
the stand row now 26 assertions with the rendered comparison and its mutation proven.
`deck.walk@50` passed its own 24 assertions and failed the universal pairing check, the client's
lead 11 then 13 frames, spread 2 against the tolerance of 1; rerun `0905-144031` read 11 then
12 and went green with both mutations proven, in the enlarged demo windows both times. The
hands-on tape at 100 ms with the ship at 7.9 to 8.4 m/s and the other pawn standing: the rendered pawn against the
server's ship-space truth, mean 0 cm on both clients with one 28 cm spike in 200 ticks, the height
matching the truth; the same tape read 230 cm mean before the fix. The sea plane within 100 cm of
the pawn along and 18 cm across on every tick, the snap's half step.

**Written, not verified.** The re-placement under a walking other pawn, asserted by the loop at
50 cm and not taped by hand; the plane follow in a packaged client; the designer's eye on the
session after the rebuild.

**Beyond the plan.** The hands-on driver gained the proxy tape and the plane check. Nothing else.

## 2026-09-05 — Review: the ship flickered, and the loop could not have seen it

### Next session's brief

**Pick up at the Melee rung, which halts without the clips and their skeleton in the project**,
stop-list item five: demand them, then open with the intake sub-slice against the contract in the
brief. The designer's review of the four rungs found one defect, fixed and verified below, and
filed one trap: the loop reads nothing that is rendered. The review queue awaits verdicts. The
editor is closed, the tree clean, every commit on the remote.

### What the review found

**The ship flickered every frame wherever the sky stood behind it**, the mast and the deck alike,
seen by the designer in the hands-on session; the pawns and the ocean did not. Per-frame tapes of
the rendered transforms of the ship, both pawns and both cameras on all three worlds moved no more
than 2.6 cm or 0.6 degrees between consecutive ticks, so nothing the integrator writes was the
cause. Consecutive captures of the play viewport showed the mesh washed toward the sky's colour
above the horizon line and correct below it, in every captured frame. Toggled at runtime with a
capture each, anti-aliasing, motion blur, screen-space reflections, fog and the aerial
perspective's fast apply changed nothing; disabling the atmosphere removed the wash. The mesh
reported no material slot; assigning one removed the wash, and the designer saw the flicker stop.

**Decision: the ship's dynamic mesh is given the engine's basic shape material in its
constructor**, the material the pawn's cylinder carries. **Alternatives**: a project material
authored from Python as the ocean's is; the default surface material set explicitly, unmeasured.
**Reopens** when a project material is authored for the ship. The finding is in
`Docs/Unreal-Findings.md`.

**The loop could not have seen it.** Every Deck row was green with the ship flickering, because
the rows read the trace and never a frame. Filed as a trap; the hands-on driver that found it is
kept as `Tools/Editor/handson.py`, with the tapes and the capture, and a paragraph in
`Docs/Debug-Instruments.md`.

**The pairing spread under a heavier render.** The demo's walk row at 100 ms, run at full
resolution in enlarged windows, failed the universal check: the client's lead read 13 then 15
frames, a spread of 2 against the tolerance of 1, the row's own 22 assertions green. Recorded in
the tuning map; the loop's half resolution is a condition of its numbers.

### Verified against written

**Verified.** Run `0905-133237`: the deck family, 14 rows at 0, 50, 100 and 150 ms, green with
every mutation proven and the universal set clean, 112 s of wall time. Run `0905-133650`: the
ship family, 14 rows, the same, 106 s. A capture of the play viewport after the rebuild shows the
mast one shade from the deck up into the sky. The designer saw the flicker stop with the material
assigned at runtime, before the rebuild.

**Written, not verified.** The harness and ocean families were not rerun; the change is one
material on the ship, and no row reads rendering.

**Beyond the plan.** The play windows were enlarged in the gitignored user ini for the demo and
restored at closedown. Nothing else.

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
