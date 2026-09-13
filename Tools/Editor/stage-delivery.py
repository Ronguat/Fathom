"""Stages part of the melee delivery inside the reference project's own editor: strips a weapon
mesh's materials, renames each listed asset into /Game/Fathom/Melee so the rename rewrites every
referencer, then saves the folder. Run through run-in-editor.py with that editor open and nothing
else answering the remote-execution pipe. Prints one STAGE line per step; the files under the
reference's Content/Fathom/Melee are then copied into this project and audited by
audit-melee-delivery.py.

Edit MOVES for what travels: (source path, destination path). The skeleton the clips name must
move to the path this project holds it at, or the copied clips arrive pointing at nothing.
"""
import unreal

EAL = unreal.EditorAssetLibrary
DEST_ROOT = "/Game/Fathom/Melee"
STRIP_MATERIALS = ["/MobiusCore/Weapons/Greatsword/Assets/SM_Greatsword"]
MOVES = [
    ("/MobiusCore/Animations/Rig/SKM_Mannequin_Skeleton", DEST_ROOT + "/Rig/SKM_Mannequin_Skeleton"),
    ("/MobiusCore/Weapons/Greatsword/Assets/OldAssets/SM_Claymore_Rig_Skeleton", DEST_ROOT + "/Weapon/SM_Claymore_Rig_Skeleton"),
    ("/MobiusCore/Weapons/Greatsword/Assets/SM_Greatsword", DEST_ROOT + "/Weapon/SM_Greatsword"),
    ("/MobiusCore/Animations/2Handed/idle/Idle_2h_FPP", DEST_ROOT + "/Clips/Stance/Idle_2h_FPP"),
    ("/MobiusCore/Animations/2Handed/idle/Idle_Stance_2h", DEST_ROOT + "/Clips/Stance/Idle_Stance_2h"),
    ("/MobiusCore/Animations/2Handed/Cycle/fpp_2h_walking", DEST_ROOT + "/Clips/Walk/fpp_2h_walking"),
    ("/MobiusCore/Animations/Locomotion/Cycles/LOCO_N_Walk_F", DEST_ROOT + "/Clips/Walk/LOCO_N_Walk_F"),
]


def say(s):
    unreal.log("STAGE " + s)


for path in STRIP_MATERIALS:
    mesh = EAL.load_asset(path)
    mats = list(mesh.get_editor_property("materials"))
    for m in mats:
        m.set_editor_property("material_interface", None)
    mesh.set_editor_property("materials", mats)
    say("stripped %d material slot(s) on %s" % (len(mats), path))

for src, dst in MOVES:
    if EAL.does_asset_exist(dst):
        say("exists %s" % dst)
        continue
    if not EAL.does_asset_exist(src):
        say("MISSING %s" % src)
        continue
    ok = EAL.rename_asset(src, dst)
    say("rename %s -> %s: %s" % (src, dst, ok))

saved = EAL.save_directory(DEST_ROOT, False, True)
say("save_directory %s: %s" % (DEST_ROOT, saved))
for src, dst in MOVES:
    say("check %s exists=%s" % (dst, EAL.does_asset_exist(dst)))
