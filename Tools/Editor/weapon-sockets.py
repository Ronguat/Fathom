"""Puts the blade_base and blade_tip sockets the combat settings name on the weapon mesh, from its
bounds: the blade runs along the bounds' longest axis, the tip at the end farther from the mesh's
origin, the base BASE_FROM_ORIGIN cm from the origin toward the tip, in the mesh's reference pose, which is what the renderer binds to. The sockets come from the MCP
skeletal mesh toolset, parented to the root bone, since a socket name and bone are read-only to
reflection; this script moves them. Run
through run-in-editor.py with the editor open and PIE stopped; prints one SOCKET line per socket
and saves the mesh. Running again moves the sockets rather than adding more.
"""
import unreal

BASE_FROM_ORIGIN = 15.0
WEAPON = "/Game/Fathom/Melee/Weapon/SM_Greatsword"

AP = unreal.AnimPoseExtensions
WORLD = unreal.AnimPoseSpaces.WORLD
K = unreal.get_default_object(unreal.FMCombatSettings)
weapon = K.get_editor_property("weapon_mesh") or unreal.load_asset(WEAPON)
bounds = weapon.get_bounds()
origin = [bounds.origin.x, bounds.origin.y, bounds.origin.z]
extent = [bounds.box_extent.x, bounds.box_extent.y, bounds.box_extent.z]
axis = max(range(3), key=lambda i: extent[i])
lo, hi = origin[axis] - extent[axis], origin[axis] + extent[axis]
tip_along = lo if abs(lo) > abs(hi) else hi
sign = 1.0 if tip_along > 0.0 else -1.0


def along(v):
    c = [0.0, 0.0, 0.0]
    c[axis] = v
    return unreal.Vector(c[0], c[1], c[2])


points = [(str(K.get_editor_property("blade_base_socket")), along(sign * BASE_FROM_ORIGIN)),
          (str(K.get_editor_property("blade_tip_socket")), along(tip_along))]
probe = unreal.SkeletalMeshComponent()
probe.set_skeletal_mesh_asset(weapon)
root = str(probe.get_bone_name(0))
root_transform = probe.get_ref_pose_transform(0)
for name, component_point in points:
    relative = root_transform.inverse_transform_location(component_point)
    sock = weapon.find_socket(name)
    if sock is None:
        unreal.log("SOCKET %s missing: SocketName is read-only to reflection, add it with the skeletal mesh toolset first" % name)
        continue
    sock.set_editor_property("relative_location", relative)
    sock.set_editor_property("relative_rotation", unreal.Rotator(0.0, 0.0, 0.0))
    unreal.log("SOCKET %s bone=%s component=(%.1f, %.1f, %.1f) relative=(%.1f, %.1f, %.1f)" % (
        name, root, component_point.x, component_point.y, component_point.z, relative.x, relative.y, relative.z))
saved = unreal.EditorAssetLibrary.save_loaded_asset(weapon)
unreal.log("SOCKET axis=%s bounds=[%.1f, %.1f] blade_length=%.1f sockets=%d saved=%s" % (
    "xyz"[axis], lo, hi, abs(tip_along - sign * BASE_FROM_ORIGIN), weapon.num_sockets(), saved))
