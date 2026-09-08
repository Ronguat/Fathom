# Fathom – Spec

**Trigger: about to change how the game behaves.** The rules that govern play, derived from the
designer's rulings in `Docs/Decisions.md`'s dated entries. A doubt about a rule is a review-queue
question, never a fix. Every value here is a knob unless the tuning map says otherwise.

## Authority and frames

- **The server decides.** Damage, hits, ship state, water, sinking and every attribute are written
  on the server. What only a machine can know — its inputs, its rendered frame — is authored there
  and sent.
- **One shared simulation frame.** Movement runs on Mover with the Network Prediction backend at a
  fixed 60 frames a second. A client authors inputs for future frames; the server executes each
  frame once; a client reconciles by rollback. Every timing in this project is stated in frames.
- **The ocean is a pure function of world position and server time.** Height and normal come from
  one wave function evaluated identically on the server, on every client, and on the GPU. What
  replicates is a sea-state scalar and a wind vector; nothing about the surface itself does.
- **The ship is a deterministic kinematic body.** Its pose is a function of a compact replicated
  state — position, heading, speed, rudder, sail length, sail angle, anchor — the ocean, and the
  frame. Any machine reconstructs its pose at any past frame from a short state history, which is
  what makes rewinding it free and reconstruction through a lost packet smooth.
- **A player on a ship simulates in ship space.** Standing on a deck is based movement on the
  ship; leaving the deck and climbing back are movement-base changes, never teleports.
- **A hit resolves on the server** by sweeping the attacker's blade against defenders rewound to
  the frame the attacker was rendering, in the ship's space when both stand on it, which is
  where the attacker's client drew the defender. The rendered frame number is authored into
  every input command, never estimated from ping.
- **Hit timing is a knob**: the server may start an attack early by a fraction of the round trip,
  capped. Known settings are none, half with a 50 ms cap, and whole with an 80 ms cap; the Melee
  rung measures them under emulated latency, and the parry's rule is the same decision's other
  half. Neither is chosen by argument.
- **A field session** is packaged clients against the editor running as a dedicated server. Every
  producer writes the same trace on the same server-frame timeline.
- **What is drawn is one step behind the simulation**, interpolated by the prediction framework's
  leftover fraction of a step: pawns by Mover's smoothing, the ship's mesh between its hull's last
  two poses, the sea's time the same. The render rate is never capped to the simulation's.

## The ocean

A Gerstner sum of four components. One sea-state scalar from 0 to the ocean settings' ceiling,
fixed for a session, scales
every component's amplitude and steepness; wavelengths and each component's direction offset from
the wind are fixed per component, and the wind vector, also fixed for a session, gives the
direction. Wave time is the shared frame over the fixed rate. The CPU function and the GPU
material function share one formulation, `Shaders/FMOcean.ush` mirrored in
`Source/Fathom/Ocean/FMOcean.cpp`, and a standing regression row asserts that server, client and
GPU agree at the same point and frame within one centimetre. The components' constants live in
`Config/DefaultGame.ini`. The surface is drawn by a plane that follows the local viewer, snapped
to its own grid, over a flat plane that reaches the horizon; nothing about the surface depends on
where it is drawn.

## The ship

Sample points on the hull read the ocean; a spring-damper on heave, roll and pitch fits the hull
to the surface. Surge comes from sail length and the sail's angle against the wind through one
speed curve with a floor head to wind, so a ship under sail can always turn; yaw from a rudder
driven at a rate and scaled by speed, holding where it is left; a dropped anchor falls for a
fixed time with the ship sailing on, then lies where the ship was, and the ship runs to the end
of its line, catches, and is held there by a spring until the anchor is raised over a fixed
time. A station call rides the caller's input command, so the server and the caller's client
apply it at the same frame and the client's prediction is exact. A **station** is a place one
player occupies to
drive one ship input: wheel, sail length, sail angle, anchor, ladder, and later cannon.

## The deck

Based movement on the ship's hull box, with jumping; the pawn's position in the ship's frame is
what the loop measures and Melee rewinds. Falling off enters Swimming when the pawn sinks under
the ocean's height at its position, a mode that holds it near the surface at a capped swim speed;
the ladder station, called from within its radius, returns the player to the deck through the
simulation. A station is driven only from within its radius on the ship. Another player's pawn is
drawn from its ship-space state on the ship as this client holds it, so a player standing still is
drawn where they stand whatever the round trip.

## Combat

Barebones Mordhau, proving exactly five things: a predicted attack start both machines agree on
in time; weapon-sweep hits resolved on the server against rewound bodies and a rewound ship, in
ship space; a parry window that resolves the same way on both machines under latency; a feint,
which is combat state rolling back; and attack direction from mouse motion, replicated as intent.

Three attack types, overhead, horizontal and thrust, each on either side: the type is the key
pressed, the side is the direction of the view's last horizontal turn before the press. First
person is the aim frame, with third person as the pre-registered fallback: the blade is a
segment from the weapon socket along the blade's axis, baked from the first-person clip at every
frame in pawn space, so the server sweeps from state alone and never from a skeletal pose; the
swing turns with the pawn's yaw and not with its pitch. An attack is windup, release and recovery
in frames from its data asset: windup ends where the clip's AutoAlignment curve reaches 1, a
swing's release runs 30 frames from there and a thrust's to the plateau's end. Hit volumes are a
capsule and a head sphere, the head tested first, and a body is met at most once per swing. The
server sweeps during release against every other body as it stood at the frame the attacker's
command says it rendered, in the attacker's ship space when it stands on the ship. A parry is a
fixed window opened by its press, with a facing cone, judged at that same rendered frame; a
feint is a cancel during windup. The advance starts an attack early by a fraction of the round
trip, capped, the client predicting the same start from the advance its player state carries.
Deliberately absent: chambers, glancing blows, ripostes, stamina, pitch in the swing, and
anything that is feel rather than proof.

## Ship combat

A cannon is a station with an aim cone, one ammunition, a fixed reload and a ballistic arc that
inherits ship velocity, validated on the server. A hit below the waterline makes a hole of one
size; water level integrates on the server from the open holes; holding on a hole repairs it; a
bucket bails. Water past a threshold sinks the ship, which respawns with its crew at a spawn point.

## Ship to ship

Ships push apart on contact with no damage. Boarding is a ladder station on another ship.

## The basic set

Every feature starts here; the right-hand column is deferred to the Stretch rung, one dated line
per deferral in its brief. The rows are the agent's recollection of Sea of Thieves, ruled basic
first by the designer and held open in the review queue for row-by-row correction.

| Row | Basic version, in | Deferred to Stretch |
|---|---|---|
| Sea state | One global scalar, set at server start, replicated once | Storm regions, change over time |
| Wind | One global vector, fixed for the session; waves travel with it | Drift, gusts |
| Waves | One Gerstner sum scaled by the sea-state scalar | Spectrum, chop |
| Sail length | One continuous value driven at a fixed rate from one station | Rope pairs, per-mast sails |
| Sail angle | Continuous, rate-limited; one speed curve of angle against wind | Class quirks |
| Wheel | Rudder driven at a rate; turn rate scales with speed; no heel | Heel, wheel travel |
| Anchor | Drop lets the ship run to the end of its line and catch; raise takes a fixed time at the capstan | Anchor turn, crew scaling |
| Cannons | One ammunition, fixed cone, fixed reload, arc inheriting ship velocity | Special shot |
| Damage | Holes below the waterline, one size; water rises; repair by holding; bail by bucket | Above-waterline holes, mast and wheel damage |
| Sinking | Threshold, then respawn at a spawn point with the crew | Sink motion |
| Ship contact | Ships push apart; no damage; open ocean, no rocks | Islands, rock holes |
| Boarding | Swim mode and a ladder station | Harpoons, mermaids |
| Classes | Sloop | The rest |

## Bars per rung

A rung is complete when its bar holds mechanically, with scripted clients under emulated latency;
no human verdict is needed for "does it work". The briefs in `Docs/Decisions.md` carry the
pre-registered numbers.

| Rung | Bar |
|---|---|
| Harness | Two-world loop runs unattended, stamps server frames, emulates latency, prints cost per player, ingests a field bundle |
| Ocean | Server, client and GPU agree on height at the same point and frame within the stated epsilon |
| Ship | Ship pose reconstructs on a client within the stated error under emulated latency and loss |
| Deck | Two scripted players stand and move on the rolling deck with deck-relative error under the stated bar at 0, 50, 100 and 150 ms |
| Melee | A scripted swing lands at the same deck-relative point at every emulated latency; parry and feint resolve identically on both machines |
| Ship combat | A cannon hit holes, floods, is repaired, bailed and sinks a ship on the server, seen identically by both clients |
| Ship to ship | Two ships collide and separate without desync; a boarding survives the base change |
