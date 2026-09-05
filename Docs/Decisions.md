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

## Known traps, indexed by what sets them off

**What this section is.** Latent defects and unverified assumptions, each filed against the rung
that makes it bite and re-read when that rung starts. These are not design questions. Nothing
here needs play to settle; they need checking. **Discharge a trap in the same commit that fixes
it**, saying what discharged it.

**Whenever a client acts — *local state never replicates.*** A plain member written on the server
stays on the server, and a client's own check then passes what the server already failed. Decide
on the server, replicate the decision, apply everywhere. Inherited from TheDream's loose-tag
traps; the shape is the same without an ability system.

**Whenever two worlds' logs are read together — *two clocks.*** TheDream measured one event at
2.788 in one world and 3.242 in the other. The trace stamps the shared frame and never a clock;
a line without a frame is not evidence.

**Whenever actors are matched across worlds — *never by name.*** Each world numbers its own
actors; TheDream measured an 85 cm "desync" that was two characters swapped. Anchor on role,
position or a replicated identity.

**Whenever a based player resimulates — *Mover reads the base at its present pose.*** The
based-movement library takes the base component's current location and rotation *(headers,
2026-09-04, `BasedMovementUtils.cpp` around lines 127 to 138)*, so a resimulation of past frames
stands the player on the ship as it is now. The deterministic ship can answer "pose at frame N";
the patch goes in a project-local copy of the plugin. Bites at Deck.

**Whenever the ship is kinematic — *it must publish its velocity.*** Mover carries a standing
player by the base component's velocity *(headers, 2026-09-04, the same file around line 158)*,
which a component moved by hand has only if it is set every tick. Bites at Ship and Deck.

**Whenever a ship is a Mover actor — *spawn it, never place it.*** Mover's README warns that
level-placed instances with customised movement modes may not move *(README, 2026-09-04, verbatim
in the bootstrap entry)*. Bites at Ship.

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

**Whenever an autonomous push is expected — *no real push has run.*** The exact allow rule passed
the harness once with nothing to push *(2026-09-04)*; the first closedown proves it. Bites at
the first closedown.

## Tuning map — a verdict comes back, which knob moves

**Every row is a warning with a reason, never a lock.** A row opening *"nothing, without
re-deriving it"* names a relationship you would be breaking, not a value you may not touch.

| Question | Move this | **Not** this |
|---|---|---|
| The simulation rate | `FixedTickFrameRate` in `Config/DefaultNetworkPrediction.ini`, 60 | The engine's own fixed frame rate, which overrides it when enabled |
| Hit timing under latency | The advance fraction and its cap, per attack, once Melee builds them | The windup |
| Parry fairness | The parry rule, measured together with the advance | The window length |
| The net update rate | `NetServerMaxTickRate`, at the engine default | The simulation rate |
| The sea | The sea-state scalar | Any single wave component |
| Ship handling | The speed curve, the rudder rate, the anchor drag | The hull sample points, which shape the fit rather than the handling |

## Rung briefs — read the one you are picking up

**Trigger: starting a rung**, alongside the traps above. Each brief carries its scope, its bar,
its fallback, its coverage and its inheritance; the plan entry written before execution turns
the bar into numbers.

- **Harness** — The two-world regression loop, built from TheDream's `Tools/RegressionCheck` as
  reference, which lives in that repository: scenarios with roles, plans in frames, mutations the
  validator will not let you omit, a universal assertion set, a preflight. Plus the trace emitter
  with server-frame stamps and world tags, the client relay, the marker hotkey, the session bundle
  and its ingest script, a latency and loss knob per connection, the cost printout, a fixed-clock
  determinism check across worlds, the dedicated-server PIE mode, and a packaged client build.
  **Decide first** whether the harness pawn is a Mover pawn from this rung or the trace stamps
  server time until Deck; both are in the review queue. **Bar**: a scenario with two scripted
  clients runs unattended at 0, 50, 100 and 150 ms and prints rows and costs; a field bundle from
  a packaged client ingests into the same evaluator. **Fallback**: if the editor-as-server route
  fails, a listen server with emulated latency on the client, filed as a trap. **Coverage**: the
  loop's own smoke rows. **Inherits**: the two-world facts in `Docs/Debug-Instruments.md`, the
  input and clock tools in `FathomEditor`.
- **Ocean** — The wave function in C++, the matching material function, the surface itself, the
  sea-state scalar and wind vector replicated once, wave time driven from server time. **Bar**:
  server, client and GPU agree on height at sampled points and frames within a pre-registered
  epsilon, one centimetre proposed; the ocean is visible in PIE. **Fallback**: a plane and a
  material sharing the function if the plugin's ocean body fights the open sea. **Coverage**: the
  determinism row, protocol two.
- **Ship** — The kinematic hull fit, the sail, wind, rudder and anchor model, the compact
  replicated state and its history, the stations, a placeholder hull from Geometry Script with
  deck collision, spawned at runtime. Whether the ship is itself a Mover actor or a custom
  integrator follows from Harness's frame decision. **Bar**: a client reconstructs the ship's pose
  within a pre-registered error at 100 ms and five percent loss; the ship sails, turns and stops.
  **Coverage**: ship-motion rows at every latency.
- **Deck** — Opens with the Mover recon: Character Movement and Mover on the same deterministic
  ship, the same scenario, one pre-registered bar. Then the base-at-frame patch through a
  project-local copy of the plugin if the recon needs it, the kinematic base publishing its
  velocity, jumping, falling off into the swim mode, the ladder station, and two scripted players
  seeing each other on the deck. **Bar**: deck-relative position error under the bar at 0, 50,
  100 and 150 ms with the ship rolling. **Fallback**: Route B, Character Movement with a custom
  replicated combat component, if Mover fails the bar after the patch. **This rung is the
  minimum answer's foundation.**
- **Melee** — **Halts without the clips and their skeleton in the project.** Opens with an intake
  sub-slice against a contract: clip list and directions, where windup ends and release begins,
  the blade socket, root motion or not, first and third person pairs; then blade curves baked per
  attack. Attack, parry and feint presses ride in the input command with the rendered frame;
  attack state sits in the sync state; the server sweeps the blade against rewound bodies and the
  reconstructed ship. The advance knob and the parry rule are measured at 0, 50, 100 and 150 ms
  across the three known settings, and the verdict goes to the review queue. First person is the
  aim frame; third person is the pre-registered fallback, a swap rather than a rebuild. **Bar**:
  per `Docs/Spec.md`. **Stripped**: chambers, glancing, ripostes, stamina.
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
  on a shape primitives cannot make; a ship art pack, declined as cosmetic.

## Symbol index — which entries discuss this thing

Current through **2026-09-04**. Regenerated, byte-sorted, one row per symbol; empty until the
first rung lands code.

| Symbol | Entries |
|---|---|

## 2026-09-04 — The wind-down closes the editor

### Next session's brief

Unchanged from the bootstrap entry below: **pick up at the Harness rung**, write its plan entry
first — the frame source before Deck, the smoke rows, the pre-registered bar — then build. No
budget is set; the designer winds sessions down manually. The editor is closed, the tree is clean,
and every commit is on the remote.

**Decision**, the designer's, asked as a question and ruled the same evening. TheDream leaves its
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
them. The designer's accounts of Mordhau's model and of their own project's half measure are in
`Docs/Reference-Model.md`, verbatim. In the shared timeline the arithmetic is exact: with the
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
a time. **Alternative.** TheDream's interrupt-on-WHAT regime, which suits a designer who owns every
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
and pipeline docs, and the regression loop, which the Harness rung builds from TheDream's as
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
which no Mover actor has exercised. The two-world recipe, adapted from TheDream's measurements
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
job; the `Terminal` plugin TheDream enables was left out. Nothing else.
