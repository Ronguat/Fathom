"""A hands-on session inside the open editor, driven through run-in-editor.py -c: PIE with two
clients, both pawns placed on the deck under an emulated round trip, the ship driven on request,
and per-frame tapes of what is rendered.

    -c "import sys; sys.path.insert(0, '<Tools/Editor>'); import handson; print(handson.start())"
    -c "import handson; print(handson.ready())"          # poll until 'ready'
    -c "import handson; print(handson.setup(100))"       # round trip in ms, pawns on the deck
    -c "import handson; print(handson.ship('p1', 'sail_length', 1.0))"
    -c "import handson; print(handson.tape(150))"        # then tape_report()
    -c "import handson; print(handson.shots(4))"         # consecutive rendered frames to Saved/Screenshots
    -c "import handson; print(handson.stop())"

A view mode or show flag reaches the play viewport only through the controller: console('p1', 'viewmode unlit').
"""
import unreal

les = unreal.get_editor_subsystem(unreal.LevelEditorSubsystem)
ues = unreal.get_editor_subsystem(unreal.UnrealEditorSubsystem)
LEVEL = None
STATE = {}
TAGS = {"p1": "C1", "p2": "C2"}
PLACES = {"p1": ((0.0, 15200.0, 320.0), 0.0), "p2": ((-1000.0, 15000.0, 320.0), 180.0)}


def start(sea="1.0", wind="30"):
    global LEVEL
    folder, name = ues.get_editor_world().get_outer().get_name().rsplit("/", 1)
    LEVEL = (folder, name)
    unreal.SystemLibrary.execute_console_command(None, "fm.SeaState %s" % sea)
    unreal.SystemLibrary.execute_console_command(None, "fm.WindAngle %s" % wind)
    les.editor_request_begin_play()
    return "play requested"


def world(tag):
    folder, name = LEVEL
    inst = 0 if tag == "S" else int(tag[1:])
    return unreal.find_object(None, "%s/UEDPIE_%d_%s.%s" % (folder, inst, name, name))


def pid(ps):
    return int(ps.get_editor_property("player_id")) if ps else -1


def ready():
    if not les.is_in_play_in_editor():
        return "not in PIE"
    worlds = dict((t, world(t)) for t in ("S", "C1", "C2"))
    missing = [t for t, w in worlds.items() if w is None]
    if missing:
        return "no world " + ",".join(missing)
    pcs, pawns = {}, {}
    for role, tag in TAGS.items():
        pc = unreal.GameplayStatics.get_player_controller(worlds[tag], 0)
        pawn = pc.get_controlled_pawn() if pc else None
        if pawn is None:
            return role + " has no pawn yet"
        p = pid(pc.get_editor_property("player_state"))
        if p < 0:
            return role + " has no player id yet"
        server_pawn = None
        for sp in unreal.GameplayStatics.get_all_actors_of_class(worlds["S"], unreal.Pawn):
            if pid(sp.get_editor_property("player_state")) == p:
                server_pawn = sp
        if server_pawn is None:
            return role + " has no server pawn yet"
        pcs[role] = pc
        pawns[(role, tag)] = pawn
        pawns[(role, "S")] = server_pawn
    STATE.update(worlds=worlds, pcs=pcs, pawns=pawns)
    return "ready"


def setup(rtt_ms=100):
    for w in STATE["worlds"].values():
        unreal.SystemLibrary.execute_console_command(w, "NetEmulation.PktLag %d" % (rtt_ms // 2))
    for role, (loc, yaw) in PLACES.items():
        STATE["pawns"][(role, "S")].harness_teleport(unreal.Vector(*loc), yaw)
        STATE["pcs"][role].set_control_rotation(unreal.Rotator(0.0, 0.0, yaw))
    return "placed at %d ms" % rtt_ms


def ship(role, station, value):
    STATE["pawns"][(role, TAGS[role])].drive_ship(station, float(value))
    return "%s drives %s to %s" % (role, station, value)


def teleport(role, loc, yaw=0.0):
    STATE["pawns"][(role, "S")].harness_teleport(unreal.Vector(*loc), yaw)
    return "%s placed" % role


def console(role, command):
    STATE["pcs"][role].console_command(command)
    return command


def ship_in(tag):
    ships = unreal.GameplayStatics.get_all_actors_of_class(world(tag), unreal.FMShip)
    return ships[0] if ships else None


def aim(role="p1", pitch=12.0):
    """Points the role's camera at the ship's centre on its own client."""
    import math
    tag = TAGS[role]
    d = ship_in(tag).get_actor_location() - STATE["pawns"][(role, tag)].get_actor_location()
    yaw = math.degrees(math.atan2(d.y, d.x))
    STATE["pcs"][role].set_control_rotation(unreal.Rotator(pitch, 0.0, yaw))
    return "aimed yaw %.0f at %.0f cm" % (yaw, d.length())


def where():
    out = []
    for (role, tag), pawn in sorted(STATE["pawns"].items()):
        l = pawn.get_sim_location()
        out.append("%s@%s (%.0f, %.0f, %.0f) f%d" % (role, tag, l.x, l.y, l.z, pawn.get_sim_frame()))
    return "; ".join(out)


def stop():
    for w in STATE.get("worlds", {}).values():
        if w is not None:
            unreal.SystemLibrary.execute_console_command(w, "NetEmulation.Off")
    les.editor_request_end_play()
    return "ending"


# --- per-frame tapes: what is rendered, sampled at every Slate tick ------------------------------

TAPE = dict(handle=None, left=0, rows=[], err=None)
NAMES = ("x", "y", "z", "pitch", "yaw", "roll", "px", "py", "pz", "pyaw", "cx", "cy", "cz", "cpitch", "cyaw", "sf", "sea_dx", "sea_dy")


def sea_in(tag):
    seas = unreal.GameplayStatics.get_all_actors_of_class(world(tag), unreal.FMOceanActor)
    return seas[0] if seas else None


def _tape_stop():
    if TAPE["handle"] is not None:
        unreal.unregister_slate_post_tick_callback(TAPE["handle"])
        TAPE["handle"] = None


def _tape_tick(dt):
    TAPE["left"] -= 1
    try:
        row = {}
        for tag, role in (("S", "p1"), ("C1", "p1"), ("C2", "p2")):
            s, pawn = ship_in(tag), STATE["pawns"].get((role, tag))
            if s and pawn:
                l, r = s.get_actor_location(), s.get_actor_rotation()
                pl, pr = pawn.get_actor_location(), pawn.get_actor_rotation()
                cam = STATE["pcs"][role].player_camera_manager if tag != "S" else None
                cl = cam.get_camera_location() if cam else unreal.Vector()
                cr = cam.get_camera_rotation() if cam else unreal.Rotator()
                sea = sea_in(tag)
                sl = sea.get_actor_location() if sea else pl
                row[tag] = (l.x, l.y, l.z, r.pitch, r.yaw, r.roll, pl.x, pl.y, pl.z, pr.yaw,
                            cl.x, cl.y, cl.z, cr.pitch, cr.yaw, pawn.get_sim_frame(), abs(sl.x - pl.x), abs(sl.y - pl.y))
        TAPE["rows"].append((dt, row))
    except Exception as e:
        TAPE["err"], TAPE["left"] = repr(e), 0
    if TAPE["left"] <= 0:
        _tape_stop()


def tape(frames=150):
    _tape_stop()
    TAPE.update(left=frames, rows=[], err=None)
    TAPE["handle"] = unreal.register_slate_post_tick_callback(_tape_tick)
    return "taping %d ticks" % frames


def tape_report():
    """The largest and mean change between consecutive ticks of every taped value, per world, and
    the sea plane's largest offset from the pawn it follows."""
    out = ["%d ticks, running=%s, err=%s" % (len(TAPE["rows"]), TAPE["handle"] is not None, TAPE["err"])]
    for tag in ("S", "C1", "C2"):
        rows = [r[tag] for _dt, r in TAPE["rows"] if tag in r]
        if len(rows) < 2:
            continue
        jumps = [[abs(rows[i][k] - rows[i - 1][k]) for k in range(len(NAMES))] for i in range(1, len(rows))]
        mx = [max(j[k] for j in jumps) for k in range(len(NAMES))]
        mean = [sum(j[k] for j in jumps) / len(jumps) for k in range(len(NAMES))]
        out.append("%s max  %s" % (tag, " ".join("%s=%.2f" % (n, v) for n, v in zip(NAMES, mx))))
        out.append("%s mean %s" % (tag, " ".join("%s=%.2f" % (n, v) for n, v in zip(NAMES, mean))))
        if tag != "S":
            out.append("%s sea plane offset from the pawn: max dx %.0f dy %.0f cm" % (tag, max(r[16] for r in rows), max(r[17] for r in rows)))
    dts = [dt for dt, _ in TAPE["rows"]]
    if dts:
        out.append("tick dt mean %.1f ms, max %.1f ms" % (1000 * sum(dts) / len(dts), 1000 * max(dts)))
    return "\n".join(out)


# --- judder: the camera's and the ship's step between rendered frames while walking -------------

JUDDER = dict(handle=None, left=0, rows=[], err=None, role="p1", key=None)


def _key(name):
    k = unreal.Key()
    k.import_text(name)
    return k


def _judder_tick(dt):
    JUDDER["left"] -= 1
    try:
        role = JUDDER["role"]
        tag = TAGS[role]
        cam = STATE["pcs"][role].player_camera_manager
        ship = ship_in(tag)
        mesh = ship.get_components_by_class(unreal.DynamicMeshComponent)[0]
        other = _other_pawn(tag, STATE["pawns"][(role, tag)])
        JUDDER["rows"].append((dt, cam.get_camera_location(), mesh.get_world_location(), other.get_actor_location() if other else unreal.Vector()))
    except Exception as e:
        JUDDER["err"], JUDDER["left"] = repr(e), 0
    if JUDDER["left"] <= 0 and JUDDER["handle"] is not None:
        unreal.FMInputTools.input_key(STATE["pcs"][JUDDER["role"]], JUDDER["key"], False)
        unreal.unregister_slate_post_tick_callback(JUDDER["handle"])
        JUDDER["handle"] = None


def judder_tape(frames=300, role="p1", key_name="W"):
    """Holds a movement key on the role's client for the tape's length, sampling per Slate tick."""
    JUDDER.update(left=frames, rows=[], err=None, role=role, key=_key(key_name))
    unreal.FMInputTools.input_key(STATE["pcs"][role], JUDDER["key"], True)
    JUDDER["handle"] = unreal.register_slate_post_tick_callback(_judder_tick)
    return "taping judder for %d ticks, %s held on %s" % (frames, key_name, role)


def judder_report(skip=40):
    """Over the steady middle of the tape: the median step between rendered frames and how many
    frames step under a quarter of it, for the camera, the ship's mesh and the other pawn."""
    rows = JUDDER["rows"][skip:-skip] if len(JUDDER["rows"]) > 2 * skip else JUDDER["rows"]
    out = ["%d ticks (%d in the middle), running=%s, err=%s" % (len(JUDDER["rows"]), len(rows), JUDDER["handle"] is not None, JUDDER["err"])]
    for name, idx in (("camera", 1), ("ship mesh", 2), ("other pawn", 3)):
        steps = sorted((rows[i][idx] - rows[i - 1][idx]).length() for i in range(1, len(rows)))
        if not steps:
            continue
        median = steps[len(steps) // 2]
        still = sum(1 for s in steps if s < 0.25 * median)
        out.append("%s: median step %.2f cm, %d of %d frames under a quarter of it (%.0f%%), min %.2f max %.2f"
                   % (name, median, still, len(steps), 100.0 * still / len(steps), steps[0], steps[-1]))
    dts = [dt for dt, _c, _m, _o in JUDDER["rows"]]
    if dts:
        out.append("frame dt mean %.1f ms" % (1000 * sum(dts) / len(dts)))
    return "\n".join(out)


# --- the other pawn as each client renders it, against the server's truth in ship space ---------

PROXY = dict(handle=None, left=0, rows=[], err=None)
STANDING_Z = 150.0 + 88.0


def _other_pawn(tag, own):
    for p in unreal.GameplayStatics.get_all_actors_of_class(world(tag), unreal.FMPlayerPawn):
        if p != own:
            return p
    return None


def _ship_local(tag, actor):
    return ship_in(tag).get_actor_transform().inverse_transform_location(actor.get_actor_location())


def _proxy_tick(dt):
    PROXY["left"] -= 1
    try:
        row = {}
        for tag, own_role, other_role in (("C1", "p1", "p2"), ("C2", "p2", "p1")):
            other = _other_pawn(tag, STATE["pawns"][(own_role, tag)])
            truth = STATE["pawns"][(other_role, "S")]
            if other and truth:
                l, t = _ship_local(tag, other), _ship_local("S", truth)
                row[tag] = (l.x, l.y, l.z, t.x, t.y, t.z, other.get_sim_frame(), truth.get_sim_frame())
        PROXY["rows"].append((dt, row))
    except Exception as e:
        PROXY["err"], PROXY["left"] = repr(e), 0
    if PROXY["left"] <= 0 and PROXY["handle"] is not None:
        unreal.unregister_slate_post_tick_callback(PROXY["handle"])
        PROXY["handle"] = None


def proxy_tape(frames=200):
    PROXY.update(left=frames, rows=[], err=None)
    PROXY["handle"] = unreal.register_slate_post_tick_callback(_proxy_tick)
    return "taping the other pawn for %d ticks" % frames


def proxy_report():
    """Per client: the rendered other pawn's jump per tick, its distance from the server's
    ship-space truth, its height against a pawn standing on the deck, and its frame behind the server's."""
    out = ["%d ticks, running=%s, err=%s" % (len(PROXY["rows"]), PROXY["handle"] is not None, PROXY["err"])]
    for tag in ("C1", "C2"):
        rows = [r[tag] for _dt, r in PROXY["rows"] if tag in r]
        if len(rows) < 2:
            continue
        jumps = [sum((rows[i][k] - rows[i - 1][k]) ** 2 for k in range(3)) ** 0.5 for i in range(1, len(rows))]
        errs = [sum((r[k] - r[k + 3]) ** 2 for k in range(3)) ** 0.5 for r in rows]
        height = [r[2] - STANDING_Z for r in rows]
        lead = [r[6] - r[7] for r in rows]
        out.append("%s other pawn rendered, ship space: jump/tick max %.1f mean %.1f cm; vs server truth max %.0f mean %.0f cm; height over standing %+.0f..%+.0f cm; frame behind server %d..%d"
                   % (tag, max(jumps), sum(jumps) / len(jumps), max(errs), sum(errs) / len(errs), min(height), max(height), -max(lead), -min(lead)))
    return "\n".join(out)


SHOT = dict(handle=None, left=0, taken=0, err=None)


def _shot_tick(dt):
    try:
        unreal.SystemLibrary.execute_console_command(world("C1"), "Shot")
        SHOT["taken"] += 1
    except Exception as e:
        SHOT["err"], SHOT["left"] = repr(e), 0
    SHOT["left"] -= 1
    if SHOT["left"] <= 0 and SHOT["handle"] is not None:
        unreal.unregister_slate_post_tick_callback(SHOT["handle"])
        SHOT["handle"] = None


def shots(frames=4):
    """Consecutive rendered frames, one Shot per Slate tick, to Saved/Screenshots/WindowsEditor."""
    SHOT.update(left=frames, taken=0, err=None)
    SHOT["handle"] = unreal.register_slate_post_tick_callback(_shot_tick)
    return "capturing %d frames" % frames


def shots_report():
    return "%d taken, running=%s, err=%s" % (SHOT["taken"], SHOT["handle"] is not None, SHOT["err"])


# --- review behaviours: the other pawn does what the checklist's item asks ------------------------

REVIEW = dict(handle=None, steps=[], held=None, left=0, loop=False)


def _action_key(action):
    table = unreal.get_default_object(unreal.FMPlayerController).get_editor_property("action_keys")
    for name, k in table.items():
        if str(name) == action:
            return k
    raise KeyError(action)


def _press(role, action, ticks=2):
    """A press now and its release after some ticks; the mouse wheel releases at once."""
    pc = STATE["pcs"][role]
    k = _action_key(action)
    unreal.FMInputTools.input_key(pc, k, True)
    if k.export_text() in ("MouseScrollUp", "MouseScrollDown"):
        unreal.FMInputTools.input_key(pc, k, False)
        return
    _schedule([(ticks, role, k)])


def _review_stop():
    if REVIEW["handle"] is not None:
        unreal.unregister_slate_post_tick_callback(REVIEW["handle"])
        REVIEW["handle"] = None
    if REVIEW["held"] is not None:
        role, k = REVIEW["held"]
        unreal.FMInputTools.input_key(STATE["pcs"][role], k, False)
        REVIEW["held"] = None
    REVIEW.update(steps=[], left=0, loop=False)


def _review_tick(dt):
    REVIEW["left"] -= 1
    if REVIEW["left"] > 0:
        return
    if REVIEW["held"] is not None:
        role, k = REVIEW["held"]
        unreal.FMInputTools.input_key(STATE["pcs"][role], k, False)
        REVIEW["held"] = None
    if not REVIEW["steps"]:
        _review_stop()
        return
    ticks, role, k = REVIEW["steps"].pop(0)
    if REVIEW["loop"]:
        REVIEW["steps"].append((ticks, role, k))
    unreal.FMInputTools.input_key(STATE["pcs"][role], k, True)
    REVIEW.update(held=(role, k), left=ticks)


def _schedule(steps, loop=False):
    """Holds each (ticks, role, key) in turn; loop repeats the sequence until stop_review()."""
    _review_stop()
    REVIEW.update(steps=list(steps), loop=loop, left=0)
    REVIEW["handle"] = unreal.register_slate_post_tick_callback(_review_tick)


def latency(ms):
    for w in STATE["worlds"].values():
        unreal.SystemLibrary.execute_console_command(w, "NetEmulation.PktLag %d" % (int(ms) // 2) if int(ms) > 0 else "NetEmulation.Off")
    return "round trip %s ms on every world" % ms


def advance(fraction, cap_ms):
    """The advance knob for every world; the server reads it within a second."""
    unreal.SystemLibrary.execute_console_command(None, "fm.MeleeAdvanceFraction %s" % fraction)
    unreal.SystemLibrary.execute_console_command(None, "fm.MeleeAdvanceCapMs %s" % cap_ms)
    return "advance %s of the round trip, capped at %s ms" % (fraction, cap_ms)


def board(role="p2"):
    _press(role, "board")
    return "%s boards" % role


def stand(role, x, y, yaw=0.0):
    """Places the role on the deck at a ship-space point, facing yaw degrees off the ship's heading."""
    ship = ship_in("S")
    tf = ship.get_actor_transform()
    loc = tf.transform_location(unreal.Vector(x, y, 250.0))
    heading = ship.get_actor_rotation().yaw + yaw
    STATE["pawns"][(role, "S")].harness_teleport(loc, heading)
    STATE["pcs"][role].set_control_rotation(unreal.Rotator(0.0, 0.0, heading))
    return "%s stands at (%.0f, %.0f) facing %.0f" % (role, x, y, heading)


def walk(role, ticks=120, action="move_forward"):
    _schedule([(ticks, role, _action_key(action))])
    return "%s holds %s for %d ticks" % (role, action, ticks)


def loop(role="p2", side=90):
    """Walks a square on the deck until stop_review(): forward, right, back, left."""
    _schedule([(side, role, _action_key(a)) for a in ("move_forward", "move_right", "move_back", "move_left")], loop=True)
    return "%s walks a loop, %d ticks a side" % (role, side)


def swing(role="p2", attack="overhead", side="right"):
    """One attack: a one-degree turn toward the side, then the press."""
    pc = STATE["pcs"][role]
    rot = pc.get_control_rotation()
    pc.set_control_rotation(unreal.Rotator(rot.roll, rot.pitch, rot.yaw + (1.0 if side == "right" else -1.0)))
    _press(role, "attack_" + attack)
    return "%s swings %s %s" % (role, attack, side)


def parry(role="p2"):
    _press(role, "parry")
    return "%s parries" % role


def feint(role="p2"):
    _press(role, "feint")
    return "%s feints" % role


def face(role, other="p1"):
    """Turns the role to face the other pawn on its own client."""
    import math
    tag = TAGS[role]
    own = STATE["pawns"][(role, tag)]
    d = _other_pawn(tag, own).get_actor_location() - own.get_actor_location()
    yaw = math.degrees(math.atan2(d.y, d.x))
    STATE["pcs"][role].set_control_rotation(unreal.Rotator(0.0, 0.0, yaw))
    return "%s faces %s at %.0f cm" % (role, other, d.length())


def stop_review():
    _review_stop()
    return "review behaviours stopped, keys released"
