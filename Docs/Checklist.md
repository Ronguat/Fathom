# Checklist — the human review

Every item is **RED until the designer says its pass line held**. An agent never flips one green;
the loop's green is not evidence here. Three verdicts: **GREEN**, the pass line held exactly;
**RED**, it did not, or it could not be judged; **NOTE**, it held and something about it bothers
you. A NOTE is one line under the item, routed at the session's end to a knob, a Stretch line or a
queue row.

**Groups gate.** An item is attempted only when its *Needs* are green. Z runs at the start of every
session; a red Z is the session.

**The first red that is a defect ends forward progress.** Press M at the moment: it pins the frame,
and that is all the recording there is. The item is the session from then on: reproduce once,
capture through the engine (shots, tape, the trace around the marks), classify, route, fix or file.
Items independent of it may continue if time remains; nothing else does. The next session re-judges
every item the fix touched, and its dependents, from RED.

**Classifying a red.** *Broken*: it does not do what its Claim says; a trap, a fix, and a loop row or
instrument so the loop sees it next time. *Right by the numbers, wrong to the eye*: the loop measures
the wrong thing; an instrument that measures what you saw, then the fix. *The item is wrong*: its
setup or pass line was unfair; rewritten, dated. *Feel or comprehension*: a NOTE.

**The record.** This file holds verdicts: the line gets GREEN or NOTE, the date, the session and the
marks. The argument lives in `Docs/Decisions.md`, pointed at, never restated.

**Standard setup.** Play-in-editor with two clients, one window each; sea state 1, wind 30°; the ship
anchored; you are p1, boarded with B; I drive p2 and any station you ask for through the hands-on
driver's review behaviours. L0, L50, L100, L150: the emulated round trip in ms, split evenly,
`FM.Latency <ms>` in any window's console or `latency(ms)` from the driver. The hull is 24 by 8 m;
the wheel is at the stern, the anchor at the bow, both sail stations at the mast, the ladder at the
starboard rail, each with a placeholder on the deck.

**Keys.** WASD move, Space jump, left button horizontal, scroll down overhead, scroll up thrust,
right button parry, Q feint; Left/Right the wheel, Up/Down the sail length, `[` `]` the sail angle,
X the anchor, E the ladder, B board, M mark. The HUD's last two lines repeat this.

## Before Z1 could run — the prep, built 2026-09-08

- **P1** Station keys and a placeholder per station on the deck; the HUD names the nearest station
  and whether you are within its radius; a call from outside is sent, and the refusal the server
  will make is shown for a moment. The row `deck.station-key` drives the wheel by key and reads
  `SHIPIN` and `SHIPNO`. The key mapping is a queue row.
- **P2** The HUD, `AFMHUD`: world tag, frame, measured lag, advance frames, movement mode and
  ship-space place, nearest station, ship speed and station values, sea state, wind, combat phase,
  tallies, the notice, the key legend. Presentation, so a dated trap: the loop never reads it.
- **P3** Every hit and parry drawn on both clients for half a second: the blade, the rewound capsule
  and head, red for a hit, blue for a parry, on the ship as that world presents it; `fm.MeleeDraw 1`
  draws the local blade green on every release frame. Written; the draw never showed in a capture
  and is not to be debugged, the ruling under M below.
- **P4** Review behaviours for p2 in `Tools/Editor/handson.py`: `board`, `stand`, `walk`, `loop`,
  `swing`, `parry`, `feint`, `face`, `latency`, `advance`, `stop_review`.
- **P5** The packaged client, rebuilt with the editor closed and its join checked once: waits for
  the F group. F1 stays yours.
- **P6** This file, a row in `CLAUDE.md`'s table, the docs check green.

Item shape: **id title** · verdict / Claim / Needs, Setup / Do / Pass only if / If red / Judged.

## Z — every session starts here

**Z1 Both windows are in the game** · RED
Setup: standard, L0.
Do: look at both windows before touching anything; press B in each.
Pass only if: each shows the ship on the sea with a pawn on its deck, and the HUD reads C1 in yours and C2 in the other.
Judged: —

**Z2 The latency is what the item asks** · RED
Setup: L150.
Do: wait five seconds; read the HUD's measured lag in both windows.
Pass only if: both read within 10 ms of the round trip set.
Judged: —

**Z3 The mark pins a frame** · RED
Do: press M.
Pass only if: within ten seconds I quote a `MARK` line with its frame.
Judged: —

**Z4 You are in control** · RED
Do: turn the view with the mouse; walk forward with W.
Pass only if: the view turns and you walk along the deck, without a hitch in the first steps.
Judged: —

## O — the ocean

**O1 The calm sea shows no edge** · RED
Claim: `Docs/Spec.md`, The ocean; the plane follows the pawn.
Needs: Z. Setup: sea 0, anchored, L0.
Do: a full turn amidships; then the bow and the stern, looking down over the rail and out.
Pass only if: no edge, seam, gap or pop anywhere.
If red: the plane's extent or its follow snap; shots at the mark.
Judged: —

**O2 Waves travel one way, whole** · RED
Needs: O1. Setup: sea 1, anchored, L0.
Do: from the bow, watch twenty seconds; point at the direction the crests travel.
Pass only if: crests are continuous, travel in one direction, never tear or pop.
Judged: —

**O3 A storm reads as a storm** · RED
Needs: O2. Setup: sea 2, twice the loop's, anchored, L0.
Do: the same watch.
Pass only if: the waves are visibly larger and the surface has no loops or spikes.
If red: amplitude and steepness scale with the sea state and nothing clamps them; nobody has looked above 1.
Judged: —

**O4 Both windows show the same sea** · RED
Needs: O2. Setup: sea 1, L150, windows side by side.
Do: pick one crest as it reaches the bow in your window; find it in the other.
Pass only if: the same crest passes the same point at the same moment, as far as your eye can tell.
Judged: —

**O5 The sea follows you** · RED
Needs: O2. Setup: sea 1, L0.
Do: walk bow to stern and back twice, watching the water beside the hull.
Pass only if: the plane never shows an edge and never pops under the hull.
Judged: —

## S — the ship

**S1 The ship sits on the water** · RED
Claim: `Docs/Spec.md`, The ship; the hull fit.
Needs: O1, O2. Setup: sea 0 then sea 1, anchored, L0.
Do: from the rail, look down at the waterline on both sides; then from the bow and the stern.
Pass only if: at calm the water meets the hull's sides at one height; at 1 the hull rises and falls with each passing wave, never floating above nor sunk below.
Judged: —

**S2 The ship moves like a ship on water** · RED
Needs: S1. Setup: sea 1, anchored, L0.
Do: stand amidships and watch the horizon thirty seconds.
Pass only if: roll and pitch are continuous and unhurried; no jitter, no snap, no rigid stillness.
If red: the fit's stiffness and damping, knobs in the tuning map.
Judged: —

**S3 The stations can be found unaided** · RED
Claim: the basic set, the station rows.
Needs: Z. Setup: standard, L0, the HUD's station line covered or ignored.
Do: without me, find and name the wheel, the sail length, the sail angle, the anchor and the ladder from their placeholders.
Pass only if: you name all five from what you see.
Judged: —

**S4 Sail** · RED
Needs: S3. Setup: sea 1, anchor raised, L0; you at the mast.
Do: hold Up; watch the HUD's speed and the water past the hull.
Pass only if: the ship visibly moves, the water moves past, and the speed reaches its top within about ten seconds.
Judged: —

**S5 Sail angle** · RED
Needs: S4. Setup: under sail, L0.
Do: hold `[`, then `]`; watch the speed against the HUD's wind.
Pass only if: the speed changes in the direction you predicted from the wind.
Judged: —

**S6 Wheel** · RED
Needs: S4. Setup: under sail, L0; you at the stern.
Do: hold Right five seconds; release; hold Left. Then the same at rest.
Pass only if: the ship turns the way you pressed, visibly faster under sail than at rest, and the rudder centres when you let go.
Judged: —

**S7 Anchor** · RED
Needs: S4. Setup: under sail, L0; you at the bow.
Do: press X and count; press X again and count until the ship moves.
Pass only if: the ship stops within a few seconds, and the raise takes about eight seconds before it moves.
Judged: —

**S8 Out of radius** · RED
Needs: S3. Setup: standard, L0; you amidships.
Do: press Right; the wheel is at the stern.
Pass only if: the ship ignores it and the HUD tells you why.
Judged: —

**S9 The other window's ship is your ship** · RED
Needs: S6. Setup: sea 1, L150, side by side.
Do: sail and turn; watch both.
Pass only if: heading, speed and roll match moment to moment, as far as your eye can tell.
Judged: —

**S10 The anchor drop, seen from the other window** · RED
Needs: S7. Setup: under sail, L150.
Do: drop the anchor; watch the other window's ship.
Pass only if: it stops without a visible jump.
If red: the loop's stop row measured a transient of 186 cm at 150 ms; whether it shows is this item.
Judged: —

## D — the deck

**D1 Standing still is standing still** · RED
Claim: `Docs/Spec.md`, The deck.
Needs: S1, S2, S6. Setup: sea 1, L0; I sail and turn the ship.
Do: stand amidships, hands off, twenty seconds.
Pass only if: you do not drift across the deck, and the horizon rolls while your feet stay put.
Judged: —

**D2 Standing still at L150** · RED
Needs: D1. Setup: as D1, L150.
Pass only if: the same.
Judged: —

**D3 Walking the deck** · RED
Needs: D1. Setup: sea 1, L0, under sail and turning.
Do: bow to stern and back, twice, releasing the key at each end.
Pass only if: no pop, no slide after release, no sinking into or floating above the deck.
Judged: —

**D4 Walking the deck at L150** · RED
Needs: D3. Setup: as D3, L150.
Pass only if: the same.
Judged: —

**D5 Jumping** · RED
Needs: D4. Setup: sea 1, L150, under sail.
Do: jump three times in place, then three times while walking.
Pass only if: you land on the deck, not through it, roughly where you expected.
Judged: —

**D6 The other player walks the deck** · RED
Needs: D3. Setup: sea 1, L150; p2 walking a loop amidships.
Do: watch p2 thirty seconds.
Pass only if: on the deck, feet planted, continuous; no slide, no teleport, no pop.
Judged: —

**D7 You, as they see you** · RED
Needs: D6. Setup: sea 1, L150.
Do: walk a loop while watching the other window.
Pass only if: the same.
Judged: —

**D8 Smooth motion under held input** · RED
Claim: the Presentation entry's bar, `Docs/Decisions.md`.
Needs: D4, D6. Setup: sea 1, L100, under sail; p2 standing at the wheel.
Do: hold W along the deck ten seconds, watching the mast, the camera and p2.
Pass only if: you see no hitch.
If red: the loop read one frame in 219 under the bar's letter; this item asks whether it shows.
Judged: —

**D9 Off the rail** · RED
Needs: D3. Setup: sea 1, L0.
Do: walk off the side.
Pass only if: you fall, enter the water, float near the surface, swim with WASD, and can turn to see the ship.
Judged: —

**D10 The ladder** · RED
Claim: `Docs/Spec.md`, The deck; the base change.
Needs: D9, S3. Setup: from D9, L0, then again at L150.
Do: swim to the starboard side amidships; press E.
Pass only if: you are back on the deck at once and can walk, at both latencies.
Judged: —

**D11 Lost overboard** · RED
Needs: D9. Setup: from D9, L0; I sail the ship away.
Do: swim thirty seconds.
Pass only if: you can get back, or the screen tells you what happens now.
If red: nothing but the board key brings a lost swimmer back, and B is a review key. The finding is what a player is told.
Judged: —

## M — melee

*Waits for the rework, 2026-09-08.* The hit is a blade baked from the clip as numbers, with no
weapon on the pawn; the designer ruled that this does not accomplish what the project set out to
do, the point being to test tracers on a ship drawn from a weapon. The items stay as written, red,
and are judged once a weapon on the pawn does the hitting.

**M1 Your arms and sword** · RED
Claim: `Docs/Spec.md`, Combat; the delivery.
Needs: D3. Setup: standard, L0.
Do: look around at idle; walk.
Pass only if: the arms and the sword are visible and read as a stance; nothing clips through the camera.
Judged: —

**M2 The other body** · RED
Needs: M1, D6. Setup: p2 standing at arm's length.
Do: walk around p2.
Pass only if: a full body with a sword, feet on the deck, facing where their view faces.
Judged: —

**M3 Six attacks on your arms** · RED
Needs: M1. Setup: standard, L0, nobody in reach.
Do: move the mouse left, scroll down; right, scroll down; the same with scroll up, and with the left button.
Pass only if: each of the six plays whole, and you can name its type and side from the motion alone.
Judged: —

**M4 The side follows the mouse** · RED
Claim: `Docs/Spec.md`, Combat; the side rule.
Needs: M3. Setup: as M3.
Do: ten presses, alternating which way you last moved the mouse.
Pass only if: the side matches the last motion every time.
Judged: —

**M5 Their attacks, seen** · RED
Needs: M2. Setup: p2 at arm's length, L0.
Do: I swing each of the six at you, named in advance.
Pass only if: you can name each type and side on their body, and the blade arrives when the motion says it should.
Judged: —

**M6 Feint** · RED
Needs: M3, M2. Setup: L0; p2 watching.
Do: start an overhead; press Q during the windup.
Pass only if: your arms return to idle at once, and the other window's body does too.
Judged: —

**M7 Parry pose** · RED
Needs: M2. Setup: L0.
Do: press the right button; watch both windows.
Pass only if: the pose shows on your arms and on their body, and holds about half a second.
Judged: —

**M8 The phases read** · RED
Needs: M3. Setup: L0; the HUD's combat line.
Do: one attack of each type, watching the phase change.
Pass only if: windup, release and recovery change when the motion says so, and no new attack starts until recovery ends.
Judged: —

**M9 Hits on a still target, L0** · RED
Claim: `Docs/Spec.md`, Combat; the sweep.
Needs: M3, M2. Setup: p2 still at arm's length, L0.
Do: ten swings, some at the body, some deliberately at the air beside it.
Pass only if: the HUD's hit count rises when and only when the blade visibly passes the body.
Judged: —

**M10 Hits on a still target, L150** · RED
Needs: M9. Setup: as M9, L150.
Pass only if: the same.
Judged: —

**M11 Head and body** · RED
Needs: M9. Setup: L0; p2 still.
Do: an overhead onto the head; a horizontal at the chest.
Pass only if: the draw's sphere lights for the first and its capsule for the second, and I read `part=head` then `part=body` off the trace.
Judged: —

**M12 The rewind shows** · RED
Claim: `Docs/Spec.md`, Combat; the hit at the frame you rendered.
Needs: M10, D6. Setup: L150; p2 walking across in front of you.
Do: thrust as p2 crosses; look at the drawn capsule.
Pass only if: the drawn capsule sits on the body as your screen showed it at the moment of the thrust, and the hit counts.
Judged: —

**M13 Hit while backing off** · RED
Needs: M12. Setup: L150; p2 attacks as you walk backward out of reach.
Do: three tries.
Pass only if: every time you were hit, your screen showed you within reach at that moment; the draw on your window shows where the server put you.
If red: the advance knob's cost made visible, not a defect; the verdict goes with M19.
Judged: —

**M14 Parry in time, L0** · RED
Claim: `Docs/Spec.md`, Combat; the parry window and cone.
Needs: M7, M9. Setup: p2 swings on a count, L0.
Do: parry as the blade comes; ten tries, some deliberately late.
Pass only if: your eye and the HUD agree on every try: parried when you were in time, hit when not.
Judged: —

**M15 Parry in time, L150** · RED
Needs: M14. Setup: as M14, L150.
Pass only if: the same.
Judged: —

**M16 A late parry fails** · RED
Needs: M14. Setup: L0.
Do: parry only after the blade has passed.
Pass only if: the hit lands and no parry counts.
Judged: —

**M17 The cone** · RED
Needs: M14. Setup: L0.
Do: parry while facing away from p2's swing.
Pass only if: the hit lands.
Judged: —

**M18 Both windows keep one score** · RED
Needs: M10, M15. Setup: L150, after a bout of ten exchanges.
Do: read the tallies in both windows.
Pass only if: hits taken, hits dealt and parries agree between windows and match what you counted.
Judged: —

**M19 The advance, blind** · RED
Claim: the advance rows in the review queue.
Needs: M12, M13. Setup: L150; three rounds of M10 and M12, the setting hidden from you.
Do: rate each round on whether hits landed where you saw them. Then I reveal.
Pass only if: you rated all three. The verdict goes to the queue row on the advance.
Judged: —

## F — the packaged client

**F1 It runs** · RED
Needs: M9, D10, P5. Setup: the editor as server; the packaged client on this machine, one window.
Do: join.
Pass only if: the ship, the sea and your pawn on the deck appear, and the HUD reads C1.
Judged: —

**F2 The short pass** · RED
Needs: F1. Setup: real loopback; p2 driven from the editor.
Do: S4, S6, D3, D9, D10, M9 and M14 again.
Pass only if: every one holds as it did in the editor.
Judged: —

**F3 The bundle** · RED
Needs: F1.
Do: press M at three moments.
Pass only if: the ingested bundle shows your three marks at their frames.
Judged: —

## C — cold read, no editor

**C1 The combat component's header** · RED
Do: open `Source/Fathom/Combat/FMCombatComponent.h`; before reading a comment, say in two sentences what it does; then read.
Pass only if: the comments confirm what you said, and nothing in them surprised you.
Judged: —

**C2 The ship's header** · RED
Do: the same for `Source/Fathom/Ship/FMShip.h`.
Pass only if: the same.
Judged: —

**C3 The newest dated entry** · RED
Do: open `Docs/Decisions.md` at its top entry; find the measured table and the open items.
Pass only if: both within two minutes.
Judged: —

**C4 The Spec against the game** · RED
Needs: M18.
Do: read `Docs/Spec.md`'s Combat section.
Pass only if: nothing in it contradicts what you saw.
Judged: —

The review closes when C4 is judged.
