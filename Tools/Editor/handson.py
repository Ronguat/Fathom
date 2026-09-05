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
NAMES = ("x", "y", "z", "pitch", "yaw", "roll", "px", "py", "pz", "pyaw", "cx", "cy", "cz", "cpitch", "cyaw", "sf")


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
                row[tag] = (l.x, l.y, l.z, r.pitch, r.yaw, r.roll, pl.x, pl.y, pl.z, pr.yaw,
                            cl.x, cl.y, cl.z, cr.pitch, cr.yaw, pawn.get_sim_frame())
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
    """The largest and mean change between consecutive ticks of every taped value, per world."""
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
    dts = [dt for dt, _ in TAPE["rows"]]
    if dts:
        out.append("tick dt mean %.1f ms, max %.1f ms" % (1000 * sum(dts) / len(dts), 1000 * max(dts)))
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
