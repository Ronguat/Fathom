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
right button parry, Q feint; Left/Right the wheel, Down unfurls the sail and Up furls it, `[` `]`
the sail angle, X the anchor, E the ladder, B board, F fly where you look and F again to drop, M
mark. The HUD's last two lines repeat this. The sail is the slab hanging from the yard, unfurled
as far as it is set and turned to its angle; the pennant at the masthead streams the way the wind
blows; the five columns on the floor's edge are still, for the ship to be seen moving against.

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

**Z1 Both windows are in the game** · GREEN
Setup: standard, L0.
Do: look at both windows before touching anything; press B in each.
Pass only if: each shows the ship on the sea with a pawn on its deck, and the HUD reads C1 in yours and C2 in the other.
Judged: 2026-09-08, session 1, the designer.

**Z2 The latency is what the item asks** · GREEN
Setup: L150.
Do: wait five seconds; read the HUD's measured lag in both windows.
Pass only if: both read within 10 ms of the round trip set.
Judged: 2026-09-08, session 1, the designer.

**Z3 The mark pins a frame** · GREEN
Do: press M.
Pass only if: within ten seconds I quote a `MARK` line with its frame.
Judged: 2026-09-08, session 1; marks C1 260 and C1 1564, quoted from the trace.

**Z4 You are in control** · GREEN
Do: turn the view with the mouse; walk forward with W.
Pass only if: the view turns and you walk along the deck, without a hitch in the first steps.
Judged: 2026-09-08, session 1, the designer.

## O — the ocean

**O1 The calm sea shows no edge** · GREEN
Claim: `Docs/Spec.md`, The ocean; the plane follows the pawn.
Needs: Z. Setup: sea 0, anchored, L0.
Do: a full turn amidships; then the bow and the stern, looking down over the rail and out.
Pass only if: no edge, seam, gap or pop anywhere.
If red: the plane's extent or its follow snap; shots at the mark.
Judged: 2026-09-08, session 1, GREEN, after the horizon plane and the 1 km near plane of that date.

**O2 Waves travel one way, whole** · GREEN
Needs: O1. Setup: sea 1, anchored, L0.
Do: from the bow, watch twenty seconds; point at the direction the crests travel.
Pass only if: crests are continuous, travel in one direction, never tear or pop.
Judged: 2026-09-08, session 1, the designer.

**O3 A storm reads as a storm** · GREEN
Needs: O2. Setup: sea 2, twice the loop's, anchored, L0.
Do: the same watch.
Pass only if: the waves are visibly larger and the surface has no loops or spikes.
If red: amplitude and steepness scale with the sea state; the ceiling is `SeaStateMax`, 2.4 with these components.
Judged: 2026-09-08, session 1, GREEN at sea 2, after the ceiling was raised from 1 that day; before it the designer could raise nothing past a gentle sea.

**O4 Both windows show the same sea** · GREEN
Needs: O2. Setup: sea 1, C1 at L150 and C2 at L50 through the driver's `latencies`, windows side by side.
Do: pick one crest as it reaches the bow in your window; find it in the other.
Pass only if: the same crest passes the same point at the same moment, as far as your eye can tell.
Judged: 2026-09-08, session 1, GREEN, "pretty cohesive". NOTE: the higher-latency window draws the ship and the sea a little ahead; each client leads the server by its own lead, 6 frames plus its round trip, so the windows differ by the difference of their leads, about a tenth of a second here. Expected; the Ship entry of 2026-09-05 and the Spec's authority section.

**O5 The sea follows you** · GREEN
Needs: O2. Setup: sea 1, L0.
Do: walk bow to stern and back twice, watching the water beside the hull.
Pass only if: no edge appears as you move, and the water never visibly jumps when the plane re-snaps under you; a wave passing through the hull's sides is not this item.
NOTE 2026-09-08: waves pierce the hull, the designer's words; the placeholder hull clips nothing, a facelift item beside the shader.
Judged: 2026-09-08, session 1, RED: an edge to the sea, stopping before the horizon, seen from the deck at sea 1; the horizon plane and the 1 km near plane followed the same session. Then RED again, the designer's words: needs a better shader for the ocean, hard to see depth currently; the shaded material followed. Then GREEN: walking the deck's length, no jump or hop the designer could see.

## S — the ship

**S1 The ship sits on the water** · GREEN
Claim: `Docs/Spec.md`, The ship; the hull fit.
Needs: O1, O2. Setup: sea 0 then sea 1, anchored, L0.
Do: from the rail, look down at the waterline on both sides; then from the bow and the stern.
Pass only if: at calm the water meets the hull's sides at one height; at 1 the hull rises and falls with each passing wave, never floating above nor sunk below.
Judged: 2026-09-08, session 1, GREEN, driven by the waves. NOTE: the hull looks shallow, a bit lifted; `HullCenterAboveWater` in the ship settings, 100 cm, for the designer's verdict.

**S2 The ship moves like a ship on water** · GREEN
Needs: S1. Setup: sea 1, anchored, L0; columns on the floor as a still reference.
Do: stand amidships and watch the horizon and the columns thirty seconds, anchored, then under sail.
Pass only if: roll and pitch are continuous and unhurried; no jitter, no snap, no rigid stillness.
If red: the fit's stiffness and damping, knobs in the tuning map; a render rate under the simulation's, the two-fixed-ticks trap.
Judged: 2026-09-08, session 1, RED: very jittery under way, milder but present at anchor. The session's defect; measured before it was touched: steady frames, the ship mesh's step swinging 44 to 2 246 cm/s, a one-frame sawtooth from the ship stepping per world tick rather than per simulation step, and the pawn moving only on frames that ran a step. Both fixed the same session, every line of the tape within a quarter of its median after. Then GREEN against the columns.

**S3 The stations can be found unaided** · GREEN
Claim: the basic set, the station rows.
Needs: Z. Setup: standard, L0, the HUD's station line covered or ignored.
Do: without me, find and name the wheel, the sail length, the sail angle, the anchor and the ladder from their placeholders.
Pass only if: you name all five from what you see.
Judged: 2026-09-08, session 1, the designer.

**S4 Sail** · GREEN
Needs: S3. Setup: sea 1, anchor raised, L0; you at the mast.
Do: hold Down; watch the sail unfurl, the HUD's speed and the water past the hull.
Pass only if: the ship visibly moves, the water moves past, and the speed reaches its top within about ten seconds.
Judged: 2026-09-08, session 1, "I think passes"; furling the sail glides the ship to a stop. NOTE, reported later: under latency an adjustment appeared instantly and reverted a little on release, the unpredicted station input; station calls now ride the input command and are predicted exactly, the same session; worth a look under 150 ms. NOTE 2, from the other window: the sail overshot and snapped back on the caller's release; by the designer's ruling a station call now takes effect a session-wide delay after its command, the HUD's "station delay", so no window corrects for it; the caller waits that long. Worth a look from both windows at L150.

**S5 Sail angle** · GREEN
Needs: S4. Setup: under sail, L0.
Do: read the wind off the pennant; hold `[`, then `]`; watch the sail turn and the speed.
Pass only if: the speed changes in the direction you predicted from the wind.
Judged: 2026-09-08, session 1, GREEN. NOTE: facing directly into the wind softlocks the ship at no speed; a floor on the speed scaling with how far the sail is unfurled, as the reference game does, followed the same session, `HeadwindSpeed` in the ship settings; GREEN again with the floor.

**S6 Wheel** · GREEN
Needs: S4. Setup: under sail, L0; you at the stern.
Do: hold Right five seconds; release; hold Left. Then the same at rest.
Pass only if: the ship turns the way you pressed, visibly faster under sail than at rest, and the rudder holds where you let go.
Judged: 2026-09-08, session 1, RED: the wheel recentred on release and should hold its position, the designer's words; changed the same session, then GREEN. NOTE, reported afterwards: at 170 ms very short taps were eaten while holds went through; a release now holds where the server has the rudder, the same session, the row tapping it at 100 ms; worth a tap under 150 ms when convenient.

**S7 Anchor** · GREEN
Needs: S4. Setup: under sail, L0; you at the bow.
Do: press X and count; press X again and count until the ship moves.
Pass only if: the anchor falls for a moment with the ship sailing on, then the ship runs to the line's end, catches, swings back and settles; the raise takes about eight seconds before it moves.
Judged: 2026-09-08, session 1, GREEN. NOTE: the stop was instant, the speed set to zero; the designer asked for a lurch, the ship catching against a taut anchor line and pulled back to it, which followed the same session. Then, on the designer's eye: too abrupt, and the speed changed before the anchor reached bottom; a drop time and a softer line followed. Then GREEN: still a bit imprecise, a major improvement, enough as proof of concept; the line's knobs are in the tuning map.

**S8 Out of radius** · GREEN
Needs: S3. Setup: standard, L0; you amidships.
Do: press Right; the wheel is at the stern.
Pass only if: the ship ignores it and the HUD tells you why.
Judged: 2026-09-08, session 1, the designer.

**S9 The other window's ship is your ship** · GREEN
Needs: S6. Setup: sea 1, L150, side by side.
Do: sail and turn; watch both.
Pass only if: heading, speed and roll match moment to moment, as far as your eye can tell.
Judged: 2026-09-08, session 1, "very impressed".

**S10 The anchor drop, seen from the other window** · GREEN
Needs: S7. Setup: under sail, L150.
Do: drop the anchor; watch the other window's ship.
Pass only if: it stops without a visible jump.
If red: the loop's stop row measured a transient of 186 cm at 150 ms; whether it shows is this item.
Judged: 2026-09-08, session 1, the designer, before the anchor line; GREEN again with the line's catch. The ship group complete.

## D — the deck

**D1 Standing still is standing still** · GREEN
Claim: `Docs/Spec.md`, The deck.
Needs: S1, S2, S6. Setup: sea 1, L0; I sail and turn the ship.
Do: stand amidships, hands off, twenty seconds.
Pass only if: you do not drift across the deck, the deck tilts with the waves under your planted feet, the horizon stays level, and you keep facing the same part of the ship as it turns.
Judged: 2026-09-08, session 1, GREEN on the drift. NOTE, a defect: players kept their world-space facing as the ship turned, so a player with no input appeared to rotate relative to the deck; the view now turns with the deck's yaw each simulated frame and the facing follows, the same session; to be looked at again.

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

**D5 Jumping** · GREEN
Needs: D4. Setup: sea 1, L150, under sail.
Do: jump three times in place, then three times while walking.
Pass only if: you land on the deck, not through it, roughly where you expected.
Judged: 2026-09-08, session 1, RED: a jump flew off the edge of the deck, no inertia on leaving ship space, the designer's words. The cause: Mover imparts a base's physics velocity on departure and the kinematic hull has none. Changed the same session: a jump keeps the deck as its base while the pawn is over the hull, so the deck carries it and it lands where it jumped; a pawn enters ship space only by touching the deck, the designer's rule. Then GREEN. NOTE: from the other window, a player jumping off the ship snaps; taken up under D9.

**D6 The other player walks the deck** · RED
Needs: D3. Setup: sea 1, L150; p2 walking a loop amidships.
Do: watch p2 thirty seconds.
Pass only if: on the deck, feet planted, continuous; no slide, no teleport, no pop, no wobble.
If red: seen ahead of its turn on 2026-09-08, the other player wobbling with the deck's roll while your own body stays upright; the proxy was placed with the base's full turn, pitch and roll included, and now takes its yaw alone.
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
Needs: D3. Setup: sea 1, L0; then once more at L150 watched from the other window.
Do: walk off the side.
Pass only if: you fall, enter the water, float near the surface, swim with WASD, and can turn to see the ship; from the other window you leave the rail without a snap.
Judged: 2026-09-08, session 1, the designer saw the snap from the other window ahead of this item; the proxy tape read it at 165 cm in one frame at 100 ms under way, and the gap now closes at 300 cm a second; to be judged.

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
