# Reference model — the game being approximated, and Fathom's runway

Status: open

**Designer-owned.** Everything quoted below is the designer's, verbatim and dated; the headings
are structure and nothing else. Agents edit this file only on the designer's explicit
instruction, quoting them exactly. While the status line reads `open`, `docs-check` lists every
agent commit that touched it for the designer's review; once it reads `hardened <date>`, an agent
commit after that date fails the check — read this, do not touch it. Questions about the model go
to the review queue in `Docs/Decisions.md`, never into this file.

## Mission (2026-09-04)

> The core assertion is to reverse engineer/recreate the ocean from Sea of Thieves from a
> functionality standpoint as faithfully as possible. What that means is the physics that drive
> ship combat, in a live, networked, and fully replicated environment. The ships themselves can be
> minimally detailed placeholders, but they themselves need to carry the behavior of standing on a
> ship in Sea of Thieves, except elevated to a standard where skill-based melee combat, also
> replicated, can be performed on their decks while they are moving. The combat will be
> Mordhau-shaped from a technical perspective. In terms of how to go about this, I'm thinking a
> brand new project is probably correct, but I want that project to inherit all the exceptionally
> valuable scars this project itself contains, if that is indeed the correct approach. You and
> other Fable 5.1 instances will be in charge of attempting to build it, and success is not
> expected, but is desired. I am aware of the ambition of what I am asking for, but the goal is to
> attempt to build it anyway rather than let intimidation discourage even attempting it.

## Scope and end goal (2026-09-04)

> This is a pretty specifically scoped mission statement, unlike TheDream that can kinda keep
> growing to be many times more complex than is currently scoped, in a wide variety of directions.
> I suppose this project could too, but this one is very explicitly intended to accomplish a
> bespoke goal, rather than meandering.

> The end goal of this project is whether you can build it, not if some specific spec is followed
> to the letter.

## Combat (2026-09-04)

> This will be a new combat system. Not TheDream. It'll be a barebones, stripped down version of
> Mordhau's combat, scoped for technical proof of concept rather than good combat design or
> balance. No chambers, no glancing blows, nothing game-feel shaped that isn't proving a technical
> hurdle was overcome.

> I have worked on a previous project with Mordhau-shaped combat, so I already have all the
> animations we need for the combat. I don't have them immediately available, but I can absolutely
> guarantee they will be ready for this project by the time we make it to the combat slice.

## Ship (2026-09-04)

> Kinematic deterministic ship. A realistic ship crumple is not the point so much as damage dealt
> satisfyingly enough.

## Minimum and additionals (2026-09-04)

On the ladder's rungs, the fourth being two players fighting on a rolling deck under latency:

> Sure. We can go with 4 as the minimum, and I like your 5-7 as additionals.

## Legality and purchases (2026-09-04)

> Anything and everything is legal for getting this thing moving. You can also recommend the
> purchasing of additional assets/plugins if you feel it would legitimately help.

## Players (2026-09-04)

> At least 2 player, but we should be keeping an eye on if playercount looks bottleneck shaped for
> performance or other hurdles.

## Basic first (2026-09-04)

On the three ocean rows — sea state, wind, waves:

> All 3 of these should start with their most basic versions, and more complex and higher quality
> variants can be deferred to a stretch goals slice. This pattern can be applied to all similar
> decisions project wide, such as ship features, etc.

## Mordhau's hit model (2026-09-04)

> The way Mordhau handled this was by having its server tracers ahead of the attacker's local
> trace, so that it could resolve hits ahead of time, and attackers experienced "instant" hits up
> to 80 ping, but the tradeoffs of this were that players' windups were legitimately faster
> scaling with ping until 80 ping, equal to their ping, but this meant they were earlier, even at
> true speed, by half their ping, because the input reached the server in half their ping but was
> ahead by their full ping. Additionally, in some edge cases, you could get false positive hits on
> particularly high latency.

## The designer's previous project's variant (2026-09-04)

> On our project, we opted for a half measure, instead capped at 100 ping instead of 50: Attacks
> are ahead by half of the attacker's ping, so they do see some hit delay on higher ping, but they
> never gain a competitive advantage at any ping threshold. So that's another option if fairness
> is valued.

> Sorry, I meant capped at 100 instead of 80. But any attack was never more than 50ms faster,
> rather than up to 80ms faster.

## Animations, camera, verdicts and art (2026-09-04)

> Before building the combat, I need to have given Fathom the combat anim assets it needs. That's
> a blocker freebie; if I still haven't provided them by the time this project makes it to that
> step, halt and demand them outright. I think closer analysis of Mordhau's combat should answer
> the animation contract questions, but if there are any that remain, I will answer them upon
> request. Camera should be 1st person, all else being equal, and the incoming animations will
> feature 1st and 3rd person pairs, but if it proves to somehow become a hurdle, abort and regress
> to 3rd person. Verdicts on play should not need human verification if they're just "does this
> feature work" and not "is it well-tuned". And for ship art, assume all placeholders unless there
> is some a feature blocked on art. I respect the recommendation to buy a ship pack because I
> requested input for purchases, but cosmetic purchases are not necessary. If it does prove truly
> necessary, then I will set up an MCP bridge to Blender for crude dev art.

## Pre-launch answers (2026-09-04)

On the skeleton the combat clips target, on the previous project's code and write-ups, and on
when unattended sessions may hold the machine:

> 1. The skeleton will arrive with the anims.
> 2. These will be provided if they prove necessary.
> 3. I'll probably just leave this machine available for the next few days (that's a long runway
> in LLM time, as opposed to human time) and then wind down sessions and reclaim the machine as
> needed, and then start or restart a session when I'm done with the machine. AKA, manual.

## Sea of Thieves, per system

Not yet dispensed row by row. The agent's recollection of each system sits in `Docs/Spec.md` as
the basic set, and the review queue in `Docs/Decisions.md` holds it open for the designer's
corrections.
