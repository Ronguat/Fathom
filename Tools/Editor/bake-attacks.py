"""Bakes the six attacks into UFMAttackData assets under /Game/Fathom/Combat from the delivered
first-person clips: the weapon socket at every frame of the clip in pawn space, the blade a
segment from it along its axis; windup ends where the clip's AutoAlignment curve reaches 1, and
release runs SWING_RELEASE_FRAMES from there for a swing, to the plateau's end for a thrust.
Run through run-in-editor.py with the editor open and PIE stopped. Prints one BAKE line per
attack and one for the eye, the head socket of the reference pose in pawn space.
"""
import unreal

ROOT = "/Game/Fathom/Melee"
OUT = "/Game/Fathom/Combat"
BLADE_LENGTH = 120.0
SWING_RELEASE_FRAMES = 30
BLADE_BONE = "weapon_r"
BLADE_SOCKET = "weapon_rSocket"
EYE_SOCKET = "headSocket"
MESH_OFFSET = unreal.Vector(0.0, 0.0, -88.0)
MESH_YAW = -90.0
RATE = 60.0
ATTACKS = [
    ("OVERHEAD", "LEFT", "Overhead_Left", "overhead_L_2h_fpp", "overhead_L_2h_tpp"),
    ("OVERHEAD", "RIGHT", "Overhead_Right", "overhead_R_2h_fpp", "overhead_R_2h_tpp"),
    ("HORIZONTAL", "LEFT", "Horizontal_Left", "horizontal_L_2h_fpp", "horizontal_L_2h_tpp"),
    ("HORIZONTAL", "RIGHT", "Horizontal_Right", "horizontal_R_2h_fpp", "horizontal_r_2h_tpp"),
    ("THRUST", "LEFT", "Thrust_Left", "thrust_L_2h_fpp", "thrust_L_2h_tpp"),
    ("THRUST", "RIGHT", "Thrust_Right", "thrust_R_2h_fpp", "thrust_R_2h_tpp"),
]
AXES = {"+X": unreal.Vector(1, 0, 0), "-X": unreal.Vector(-1, 0, 0), "+Y": unreal.Vector(0, 1, 0),
        "-Y": unreal.Vector(0, -1, 0), "+Z": unreal.Vector(0, 0, 1), "-Z": unreal.Vector(0, 0, -1)}

AP = unreal.AnimPoseExtensions
AL = unreal.AnimationLibrary
ML = unreal.MathLibrary
WORLD = unreal.AnimPoseSpaces.WORLD
arms = unreal.load_asset(ROOT + "/Rig/SKM_Mannequin_Arms")
mesh_rel = unreal.Transform(MESH_OFFSET, unreal.Rotator(roll=0.0, pitch=0.0, yaw=MESH_YAW), unreal.Vector(1, 1, 1))
options = unreal.AnimPoseEvaluationOptions()
options.set_editor_property("optional_skeletal_mesh", arms)
options.set_editor_property("extract_root_motion", False)


def say(s):
    unreal.log("BAKE " + s)


def pawn_space(component_transform):
    return ML.compose_transforms(component_transform, mesh_rel)


def socket_in_pawn_space(pose, socket):
    return pawn_space(AP.get_socket_pose(pose, socket, WORLD))


def release_window(seq, frames, swing=False):
    """(windup_frames, release_frames) from AutoAlignment: windup to the first key at 1; release
    SWING_RELEASE_FRAMES for a swing, else to the last key at 1 before the curve falls, or to the
    fall's end when the plateau has no length."""
    times, values = AL.get_float_keys(seq, "AutoAlignment")
    keys = list(zip(list(times), list(values)))
    start = next((t for t, v in keys if v >= 0.5), None)
    if start is None:
        return frames // 3, frames // 4
    after = [(t, v) for t, v in keys if t > start]
    fall = next((t for t, v in after if v < 0.5), None)
    high = [t for t, v in after if v >= 0.5 and (fall is None or t < fall)]
    end = high[-1] if high else fall
    if end is None or end <= start:
        end = fall if fall is not None and fall > start else start + 0.25
    windup = int(round(start * RATE))
    release = max(1, int(round(end * RATE)) - windup)
    if swing:
        release = min(SWING_RELEASE_FRAMES, frames - windup - 1)
    return windup, release


def blade_axis(thrust_seq, windup, release):
    """The weapon bone axis pointing furthest forward in pawn space at the thrust's release midpoint."""
    t = (windup + release * 0.5) / RATE
    pose = AP.get_anim_pose_at_time(thrust_seq, t, options)
    bone = pawn_space(AP.get_bone_pose(pose, BLADE_BONE, WORLD))
    best, best_dot = "+X", -2.0
    for name, axis in AXES.items():
        d = bone.transform_direction(axis)
        say("axis %s forward=%.3f" % (name, d.x))
        if d.x > best_dot:
            best, best_dot = name, d.x
    return best, AXES[best]


def bake(type_name, side_name, folder, fp_name, tp_name, axis):
    fp = unreal.load_asset("%s/Clips/%s/%s" % (ROOT, folder, fp_name))
    tp = unreal.load_asset("%s/Clips/%s/%s" % (ROOT, folder, tp_name))
    frames = int(AL.get_num_frames(fp))
    windup, release = release_window(fp, frames, swing=type_name != "THRUST")
    bases, tips = [], []
    for f in range(frames):
        pose = AP.get_anim_pose_at_time(fp, f / RATE, options)
        socket = socket_in_pawn_space(pose, BLADE_SOCKET)
        base = socket.translation
        tip = base + socket.transform_direction(axis) * BLADE_LENGTH
        bases.append(unreal.Vector(base.x, base.y, base.z))
        tips.append(unreal.Vector(tip.x, tip.y, tip.z))
    rel = tips[windup:windup + release]
    name = "DA_Attack_%s_%s" % (type_name.title(), side_name[0])
    path = OUT + "/" + name
    if unreal.EditorAssetLibrary.does_asset_exist(path):
        asset = unreal.EditorAssetLibrary.load_asset(path)
    else:
        asset = unreal.AssetToolsHelpers.get_asset_tools().create_asset(name, OUT, unreal.FMAttackData, unreal.DataAssetFactory())
    asset.set_editor_property("type", getattr(unreal.FMAttackType, type_name))
    asset.set_editor_property("side", getattr(unreal.FMAttackSide, side_name))
    asset.set_editor_property("first_person", fp)
    asset.set_editor_property("third_person", tp)
    asset.set_editor_property("windup_frames", windup)
    asset.set_editor_property("release_frames", release)
    asset.set_editor_property("total_frames", frames)
    asset.set_editor_property("blade_base", bases)
    asset.set_editor_property("blade_tip", tips)
    asset.set_editor_property("blade_length", BLADE_LENGTH)
    asset.set_editor_property("blade_axis", axis)
    saved = unreal.EditorAssetLibrary.save_loaded_asset(asset)
    say("attack=%s_%s asset=%s frames=%d windup=%d release=%d recovery=%d saved=%s" % (
        type_name.lower(), side_name[0].lower(), name, frames, windup, release, frames - windup - release, saved))
    say("attack=%s_%s release tip x=[%.0f, %.0f] y=[%.0f, %.0f] z=[%.0f, %.0f] span=%.0f" % (
        type_name.lower(), side_name[0].lower(),
        min(v.x for v in rel), max(v.x for v in rel), min(v.y for v in rel), max(v.y for v in rel),
        min(v.z for v in rel), max(v.z for v in rel),
        max(ML.vector_distance(a, b) for a in rel for b in rel)))
    say("attack=%s_%s frame0 base=(%.0f, %.0f, %.0f) tip=(%.0f, %.0f, %.0f)" % (
        type_name.lower(), side_name[0].lower(), bases[0].x, bases[0].y, bases[0].z, tips[0].x, tips[0].y, tips[0].z))


thrust = unreal.load_asset("%s/Clips/Thrust_Right/thrust_R_2h_fpp" % ROOT)
tw, tr = release_window(thrust, int(AL.get_num_frames(thrust)))
axis_name, axis = blade_axis(thrust, tw, tr)
say("blade axis %s from thrust_R_2h_fpp windup=%d release=%d" % (axis_name, tw, tr))
for entry in ATTACKS:
    bake(*entry, axis=axis)

ref = AP.get_anim_pose_at_time(thrust, 0.0, options)
head = pawn_space(AP.get_ref_bone_pose(ref, "head", WORLD))
sock = arms.find_socket(EYE_SOCKET)
sock_rel = unreal.Transform(sock.get_editor_property("relative_location"), sock.get_editor_property("relative_rotation"),
                            sock.get_editor_property("relative_scale"))
eye = ML.compose_transforms(sock_rel, head)
say("eye=(%.1f, %.1f, %.1f) head=(%.1f, %.1f, %.1f) in pawn space" % (
    eye.translation.x, eye.translation.y, eye.translation.z, head.translation.x, head.translation.y, head.translation.z))
