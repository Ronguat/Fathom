"""A swing on the deck, captured: handson's two-client PIE with p2 placed in front of p1, one attack
key on p1, and a high-resolution screenshot of the game viewport, the first client's, at chosen
frames after the press, into Saved/Screenshots/WindowsEditor as melee_<action>_f<frame>.png.

    -c "import sys; sys.path.insert(0, '<Tools/Editor>'); import handson; print(handson.start())"
    -c "import handson; print(handson.ready())"
    -c "import handson, melee_capture; print(melee_capture.place(0))"
    -c "import melee_capture; print(melee_capture.swing('attack_overhead', (0, 30, 48, 52, 56, 80)))"
    -c "import melee_capture; print(melee_capture.report())"
"""
import unreal
import handson

STATE = {}
FACING = {"p1": ((0.0, 15200.0, 320.0), 0.0), "p2": ((200.0, 15200.0, 320.0), 180.0)}


def place(rtt_ms=0):
    handson.PLACES.update(FACING)
    return handson.setup(rtt_ms)


def key_for(action):
    table = unreal.get_default_object(unreal.FMPlayerController).get_editor_property("action_keys")
    for name, k in table.items():
        if str(name) == action:
            key = unreal.Key()
            key.import_text(k.export_text())
            return key
    raise KeyError(action)


def frame(role):
    return int(unreal.FMTraceLibrary.get_frame(handson.world(handson.TAGS[role])))


def swing(action="attack_overhead", at_frames=(0, 30, 48, 52, 56, 80), role="p1"):
    pc = handson.STATE["pcs"][role]
    key = key_for(action)
    unreal.FMInputTools.input_key(pc, key, True)
    unreal.FMInputTools.input_key(pc, key, False)
    STATE.update(role=role, action=action, start=frame(role), due=list(at_frames), taken=[], handle=None)
    STATE["handle"] = unreal.register_slate_post_tick_callback(_tick)
    return "pressed %s at frame %d" % (action, STATE["start"])


def _tick(dt):
    f = frame(STATE["role"]) - STATE["start"]
    if STATE["due"] and f >= STATE["due"][0]:
        STATE["due"].pop(0)
        unreal.AutomationLibrary.take_high_res_screenshot(640, 480, "melee_%s_f%03d.png" % (STATE["action"], max(f, 0)))
        STATE["taken"].append(f)
    if not STATE["due"] and STATE["handle"] is not None:
        unreal.unregister_slate_post_tick_callback(STATE["handle"])
        STATE["handle"] = None


def report():
    return "shots at frames %s, %d pending" % (STATE.get("taken"), len(STATE.get("due", [])))
