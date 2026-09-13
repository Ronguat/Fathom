"""Bakes the six attacks into UFMAttackData assets under /Game/Fathom/Combat from the delivered
third-person clips: the hand socket at every frame of the clip in pawn space, the weapon on it by
the combat settings' mount, and TracerCount tracers spaced from the weapon's blade_base socket to
its blade_tip; windup ends where the first-person clip's AutoAlignment curve reaches 1, and
release runs SWING_RELEASE_FRAMES from there for a swing, to the plateau's end for a thrust. Run
through run-in-editor.py with the editor open and PIE stopped. Prints one BAKE line per attack and
one for the eye, the head socket of the reference pose in pawn space.
"""
import unreal

ROOT = "/Game/Fathom/Melee"
OUT = "/Game/Fathom/Combat"
WEAPON = ROOT + "/Weapon/SM_Greatsword"
SWING_RELEASE_FRAMES = 30
EYE_SOCKET = "headSocket"
RATE = 60.0
ATTACKS = [
    ("OVERHEAD", "LEFT", "Overhead_Left", "overhead_L_2h_fpp", "overhead_L_2h_tpp"),
    ("OVERHEAD", "RIGHT", "Overhead_Right", "overhead_R_2h_fpp", "overhead_R_2h_tpp"),
    ("HORIZONTAL", "LEFT", "Horizontal_Left", "horizontal_L_2h_fpp", "horizontal_L_2h_tpp"),
    ("HORIZONTAL", "RIGHT", "Horizontal_Right", "horizontal_R_2h_fpp", "horizontal_r_2h_tpp"),
    ("THRUST", "LEFT", "Thrust_Left", "thrust_L_2h_fpp", "thrust_L_2h_tpp"),
    ("THRUST", "RIGHT", "Thrust_Right", "thrust_R_2h_fpp", "thrust_R_2h_tpp"),
]

AP = unreal.AnimPoseExtensions
AL = unreal.AnimationLibrary
ML = unreal.MathLibrary
WORLD = unreal.AnimPoseSpaces.WORLD
K = unreal.get_default_object(unreal.FMCombatSettings)
mesh_rel = unreal.Transform(K.get_editor_property("mesh_offset"),
                            unreal.Rotator(roll=0.0, pitch=0.0, yaw=K.get_editor_property("mesh_yaw")), unreal.Vector(1, 1, 1))
mount = unreal.Transform(K.get_editor_property("weapon_offset"), K.get_editor_property("weapon_rotation"), unreal.Vector(1, 1, 1))
HAND_SOCKET = str(K.get_editor_property("weapon_socket"))
TRACERS = max(2, int(K.get_editor_property("tracer_count")))
arms = unreal.load_asset(ROOT + "/Rig/SKM_Mannequin_Arms")
body = unreal.load_asset(ROOT + "/Rig/SKM_Mannequin")
weapon = K.get_editor_property("weapon_mesh") or unreal.load_asset(WEAPON)


def options_for(mesh):
    o = unreal.AnimPoseEvaluationOptions()
    o.set_editor_property("optional_skeletal_mesh", mesh)
    o.set_editor_property("extract_root_motion", False)
    return o


arms_options, body_options = options_for(arms), options_for(body)


def say(s):
    unreal.log("BAKE " + s)


def pawn_space(component_transform):
    return ML.compose_transforms(component_transform, mesh_rel)


def weapon_points():
    """blade_base and blade_tip in the weapon's own space, its sockets in the mesh's reference pose, the bind pose the renderer uses."""
    probe = unreal.SkeletalMeshComponent()
    probe.set_skeletal_mesh_asset(weapon)
    points = []
    for key in ("blade_base_socket", "blade_tip_socket"):
        sock = weapon.find_socket(str(K.get_editor_property(key)))
        bone = probe.get_ref_pose_transform(probe.get_bone_index(sock.get_editor_property("bone_name")))
        points.append(bone.transform_location(sock.get_editor_property("relative_location")))
    return points


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


def bake(type_name, side_name, folder, fp_name, tp_name, base, tip):
    fp = unreal.load_asset("%s/Clips/%s/%s" % (ROOT, folder, fp_name))
    tp = unreal.load_asset("%s/Clips/%s/%s" % (ROOT, folder, tp_name))
    fp_frames, tp_frames = int(AL.get_num_frames(fp)), int(AL.get_num_frames(tp))
    frames = min(fp_frames, tp_frames)
    tag = "%s_%s" % (type_name.lower(), side_name[0].lower())
    if fp_frames != tp_frames:
        say("attack=%s first-person %d frames, third-person %d, baking %d" % (tag, fp_frames, tp_frames, frames))
    windup, release = release_window(fp, frames, swing=type_name != "THRUST")
    tracers, tips = [], []
    for f in range(frames):
        pose = AP.get_anim_pose_at_time(tp, f / RATE, body_options)
        hand = pawn_space(AP.get_socket_pose(pose, HAND_SOCKET, WORLD))
        wt = ML.compose_transforms(mount, hand)
        for i in range(TRACERS):
            a = i / float(TRACERS - 1)
            p = unreal.Vector(base.x + (tip.x - base.x) * a, base.y + (tip.y - base.y) * a, base.z + (tip.z - base.z) * a)
            q = wt.transform_location(p)
            tracers.append(unreal.Vector(q.x, q.y, q.z))
        tips.append(tracers[-1])
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
    asset.set_editor_property("tracer_count", TRACERS)
    asset.set_editor_property("tracers", tracers)
    asset.set_editor_property("blade_length", ML.vector_distance(base, tip))
    saved = unreal.EditorAssetLibrary.save_loaded_asset(asset)
    say("attack=%s asset=%s frames=%d windup=%d release=%d recovery=%d tracers=%d saved=%s" % (
        tag, name, frames, windup, release, frames - windup - release, TRACERS, saved))
    say("attack=%s release tip x=[%.0f, %.0f] y=[%.0f, %.0f] z=[%.0f, %.0f] span=%.0f" % (
        tag, min(v.x for v in rel), max(v.x for v in rel), min(v.y for v in rel), max(v.y for v in rel),
        min(v.z for v in rel), max(v.z for v in rel), max(ML.vector_distance(a, b) for a in rel for b in rel)))
    b0, t0 = tracers[0], tracers[TRACERS - 1]
    say("attack=%s frame0 base=(%.0f, %.0f, %.0f) tip=(%.0f, %.0f, %.0f)" % (tag, b0.x, b0.y, b0.z, t0.x, t0.y, t0.z))


base, tip = weapon_points()
say("weapon=%s blade_base=(%.1f, %.1f, %.1f) blade_tip=(%.1f, %.1f, %.1f) length=%.1f tracers=%d mount=%s" % (
    weapon.get_name(), base.x, base.y, base.z, tip.x, tip.y, tip.z, ML.vector_distance(base, tip), TRACERS, mount))
for entry in ATTACKS:
    bake(*entry, base=base, tip=tip)

thrust = unreal.load_asset("%s/Clips/Thrust_Right/thrust_R_2h_fpp" % ROOT)
ref = AP.get_anim_pose_at_time(thrust, 0.0, arms_options)
head = pawn_space(AP.get_ref_bone_pose(ref, "head", WORLD))
sock = arms.find_socket(EYE_SOCKET)
sock_rel = unreal.Transform(sock.get_editor_property("relative_location"), sock.get_editor_property("relative_rotation"),
                            sock.get_editor_property("relative_scale"))
eye = ML.compose_transforms(sock_rel, head)
say("eye=(%.1f, %.1f, %.1f) head=(%.1f, %.1f, %.1f) in pawn space" % (
    eye.translation.x, eye.translation.y, eye.translation.z, head.translation.x, head.translation.y, head.translation.z))
