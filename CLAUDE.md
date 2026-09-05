# Fathom – Ship Combat Prototype

## Project Intent
A **networked, server-authoritative** recreation of Sea of Thieves' ocean and ship combat from a
functionality standpoint: the physics that drive ship combat, ships as placeholders that carry the
behaviour of standing on a moving deck, and a barebones Mordhau-shaped melee, all replicated,
fought on those decks while they move. The designer's mission statement is quoted in
`Docs/Decisions.md`; this paragraph is its compression.

**The question is whether it can be built.** Success is desired, not expected; the goal is to
attempt it rather than be discouraged. Nothing here is tuned for feel: a feature exists to prove
a technical hurdle was overcome, and every richer variant is deferred to the Stretch rung.

**The minimum answer is rung four**: two players fighting on a rolling deck under real latency,
proven by scripted clients. Rungs five to seven are wanted; Stretch is the rest.

## Building for the network

Binding on all work; the model itself is `Docs/Spec.md`'s "Authority and frames" section.

- **One shared simulation frame.** Mover on the Network Prediction backend, fixed at 60 frames a
  second. Every timing is stated in frames, with a round trip in it.
- **The ocean is a pure function of position and server time.** Nothing about the surface
  replicates.
- **The ship is a deterministic kinematic body**, reconstructible at any past frame from compact
  replicated state, so rewinding it is free. Chaos does not drive it.
- **Players simulate in ship space.** Leaving and rejoining the deck are base changes, never
  teleports.
- **The server decides.** Hits resolve there against rewound bodies and a rewound ship; new state
  is a replicated property, never a local one; what only a machine can know is authored there and
  sent.
- **Hit timing is a knob, measured before it is argued.**

## Vocabulary

- **Frame** — the shared simulation frame; the unit of every timing.
- **Ship space** — the ship's local frame, where a based player simulates.
- **Sea state** — the one scalar the ocean function takes, calm to storm.
- **Station** — a place one player occupies to drive one ship input.
- **Advance** — the hit-timing knob: a fraction of the round trip, and its cap.
- **Windup, release, recovery** — an attack's three phases; release is when the blade can hit.
- **Rung** — a roster item with a mechanical bar. **Stretch** is the rung that receives every
  deferral.
- **Field session** — packaged clients against the editor as a dedicated server.
- **Bar** — the pre-registered number a recon or a rung must meet.

## Technical Preferences
- **C++** for core systems; Blueprint where it speeds iteration.
- **Mover with Network Prediction** for everything that moves. **No ability system in the simulation
  spine**; the decision and what reopens it are in `Docs/Decisions.md`.
- **Every tuning value is a UPROPERTY** or lives in data.
- **Placeholders throughout**, made with Geometry Script. Nothing is bought for looks.

## Implementation Conventions
- **`Fathom` is the codename and `FM` the prefix, both permanent**: the module, `/Script/Fathom.*`,
  the content root `/Game/Fathom/` and every class carry them, and renaming breaks every stored
  class path.
- **The title lives only in `ProjectName`** (`Config/DefaultGame.ini`).
- Content lives under `/Game/Fathom/`, subfoldered by kind; C++ mirrors it under
  `Source/Fathom/` as `Core`, `Ocean`, `Ship`, `Deck`, `Combat`, `Net`. Includes are relative to
  the module root, e.g. `#include "Core/FMGameMode.h"`.
- **Ships spawn at runtime.** Nothing that moves is placed in a map.

## Project Documentation

Standing files carry knowledge the code cannot. Each has an owner and a trigger. Keep them true
in the same commit that makes them wrong.

| File | Owner | Trigger |
|---|---|---|
| `Docs/Spec.md` | Agents | About to change how the game behaves |
| `Docs/Decisions.md` | Agents | Making a choice, picking up a rung, ending a session |
| `Docs/Working-In-Unreal.md` | Agents | Before work that touches the engine; sized to be read whole |
| `Docs/Unreal-Findings.md` | Agents | About to conclude something cannot be done |
| `Docs/Debug-Instruments.md` | Agents | About to measure something |
| `Docs/Closing-Down.md` | Agents | Winding down, whoever triggered it |

`Docs/Decisions.md` opens with its working sections — the **review queue**, the supersession
table, the **known traps**, the **tuning map**, the **rung briefs**, the **symbol index** — and
the dated entries follow. **Never rewrite an entry — supersede it.** An entry has a shape: the
decision, the alternatives, what reopens it, and the measurement if there is one.

**Durable knowledge belongs in these files, not in per-machine memory.** Memory points at the repo.

**One fact, one home; name the authority rather than restating the value.**

**`Tools/DocsCheck/docs-check.sh` is these files' integrity check.** Run it after any edit that
moves text between docs; closedown runs it always.

**Name the asset, not the C++ class.** A Blueprint override shadows a C++ default silently, so a
class is authoritative only until someone touches a details panel.

**Deliberately not kept: per-system design docs.** A doc describing a system drifts, then gets
trusted over the code.

**Comments carry WHAT, and HOW where the mechanism is not plain from reading — never WHY.** No
dates, no attributions, no history; `Tools/CommentCheck/comment-check.sh` fails on those, and
warns when a file's comment volume outgrows `Tools/CommentCheck/baseline.txt`, which the closedown
eye then judges.

## Communication

**Use as few words as you possibly can, without any form of signal degradation.** Every instance,
project-wide, whatever the context — code comments, decision entries, conversation. A useful table
earns its space; a second example does not. **Efficient, not expressive.**

**A transcription may lose detail; it may never add any.** Sharpening a source — into a term, a
category, a general rule — is a decision taken quietly and reads as more authoritative than what it
came from. Point at the source; if it says less, say less.

**Report what was found, not who found it.** Note a contribution in a clause if it matters to the
reasoning and move on; do not tally credit and do not apologise. A wrong claim still gets corrected
plainly.

## Working Rules

**Decide, record, continue.** The designer owns the mission, not the WHATs inside it. When a
choice is ambiguous, take the most basic reversible option, record it in the review queue with
the alternatives and what would reopen it, and go on. Silence from the designer means basic, not
"not reached yet". A decision the designer dislikes is superseded, never rewritten.

**A written plan precedes execution, and nobody approves it.** Before a rung or a sub-slice, a
dated entry states the scope, the pre-registered bar, the fallback a failed bar triggers, and the
scenarios it adds or the dated trap naming what is now untested. The report is then checkable
against the plan, which is what stops a session rationalising mid-run.

**The stop list** — the only things that wait for the designer:
1. Changing the ladder, the minimum, or the mission.
2. A purchase, or a dependency beyond engine plugins.
3. Destructive or history-rewriting git; deleting anything the designer made.
4. Modifying the engine install. A project-local plugin copy is fine.
5. Opening the Melee rung without the delivered clips and their skeleton: halt and demand them.
6. A recon that failed its bar and its fallback.

**Wind-down is agent-triggered** by four conditions — scope done, a stop-list item reached, the
session brief's budget reached, a dead end after the fallback — and `Docs/Closing-Down.md` then
runs in full and ends with the editor closed, so every session starts the same way. **Main stays
green**: verified units commit to main and push when the gate is green; unverified work goes to a
named `wip/` branch the handoff points at. **The handoff writes the next session's brief**, so
"continue" is a complete prompt. **One session at a time holds the editor and the repository.**

**Report what was verified versus merely written**, the assets touched and values set, and what
was done beyond the plan, or that nothing was. A number in a report or a commit is a measurement.

**When measurement and rationale disagree, measurement wins.** **Instrument before theorising.**
**Never claim something does not exist from a filtered view**: search the authoritative source
unfiltered, quote the command, date every absence claim. **Before declaring a tooling limit,
exhaust the surfaces** — MCP, editor Python, the engine's headers, then C++ in `FathomEditor`; a
limit filed as a design constraint needs re-testing hardest. **When a doc describes something
that exists, read the thing.** **Do not delete lines you did not write without asking.**

**At startup, check that the previous session wound down**: a clean tree or one the handoff
accounts for, and `Saved/Autosaves/PackageRestoreData.json` reading `Packages: []` against a
closed editor. **MCP tools register only if the editor was open when the session started**; the
bridge in `Tools/McpBridge/` reaches the editor regardless.

**Commit in verified units, with the `Co-Authored-By` trailer on every commit an agent authors.**
A push is `git push origin main` and nothing else: the hook in `Tools/GitHooks/` rejects
deletions and non-fast-forward pushes from this clone, and the repository's ruleset rejects the
same from anywhere.

**Any plan adding a capability lists the scenarios it adds, or files a dated trap.** It binds at
plan time; a loop that lags the surface still prints green.

## The protocols

1. **No single-world green counts.** The loop is two-world from the first row, and every row runs
   at several emulated latencies, zero included.
2. **Determinism is a standing row** from the Ocean rung on: server, client and GPU sample the same
   point at the same frame, and the row asserts agreement.
3. **Cost is printed every run**: bandwidth per connection and server tick time per player.
4. **The hit-timing rule is a knob**, fraction and cap, measured under latency before it is argued.
5. **Coverage binds at plan time**: scenarios in the package, or a dated trap.
6. **Every timing is stated in frames**, with a round trip in it.
7. **The field log**: one trace shape from PIE server, PIE client and packaged build alike, relayed
   to the server, with a marker hotkey and a session bundle an ingest script reads. The shape is
   in `Docs/Debug-Instruments.md`.
8. **Basic first.** Every feature starts at its most basic version; richer variants go to the
   Stretch rung, one dated line per deferral in its brief.

## Current Focus

> **~~Bootstrap~~ → Harness → Ocean → Ship → Deck → Melee → Ship Combat → Ship to Ship → Stretch**

**Pick up at Harness.** Its brief is in `Docs/Decisions.md`, and the handoff at the top of the
newest dated entry there is the session brief. Deck opens with the Mover recon and carries Route B
as its fallback; Melee opens only with the clips and their skeleton in the project.

### When a rung ships

Route every consequence, then leave only the strikethrough.

| Consequence | Goes to |
|---|---|
| A rule that still governs play | `Docs/Spec.md` |
| A latent defect or unverified assumption | the traps section of `Docs/Decisions.md` |
| A derived value, or which knob moves for which complaint | the tuning map there |
| A deferral | the Stretch brief, dated |
| What the next rung inherits | its brief |
| What a symbol is, does or requires | its header comment, and nothing else |
| A WHAT decided alone | the review queue |
| The argument behind any of it | its dated entry, where it already is |
