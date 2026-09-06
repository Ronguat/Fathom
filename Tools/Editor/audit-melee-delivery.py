"""Loads every asset of the melee delivery, checks that its hard dependencies stay inside the
delivery and the engine, resaves each as this engine's package, and writes Docs/Melee-Delivery.tsv,
one row per asset with what the intake contract asks of it. Headless:

  UnrealEditor-Cmd.exe Fathom.uproject -run=pythonscript -script="<abs>/audit-melee-delivery.py"
      -nullrhi -unattended -nosplash -nopause -NoSound -stdout -FullStdOutLogOutput

The summary line is `AUDIT assets=<n> loaded=<n> failed=<n> outside=<n>`; a failed load or a
package reached outside the delivery is listed before it.
"""
import os

import unreal

ROOT = "/Game/Fathom/Melee"
ENGINE_ROOTS = ("/Engine/", "/ACLPlugin/", "/Script/")

reg = unreal.AssetRegistryHelpers.get_asset_registry()
reg.scan_paths_synchronous([ROOT], True)
assets = sorted(reg.get_assets_by_path(ROOT, True), key=lambda a: str(a.package_name))
pkgs = set(str(a.package_name) for a in assets)
opts = unreal.AssetRegistryDependencyOptions(include_soft_package_references=False,
                                             include_hard_package_references=True,
                                             include_searchable_names=False,
                                             include_soft_management_references=False,
                                             include_hard_management_references=False)
outside = {}
for p in pkgs:
    for d in reg.get_dependencies(p, opts) or []:
        d = str(d)
        if d not in pkgs and not d.startswith(ENGINE_ROOTS):
            outside.setdefault(d, []).append(p)
for d in sorted(outside):
    unreal.log("AUDIT outside: %s <- %s" % (d, ", ".join(sorted(outside[d])[:3])))

lib = unreal.AnimationLibrary
rows, failed = [], []


def describe(a, obj, cls, folder):
    if cls == "AnimSequence":
        notifies = lib.get_animation_notify_events(obj)
        curves = list(lib.get_animation_curve_names(obj, unreal.RawCurveTrackTypes.RCT_FLOAT))
        skel = obj.get_editor_property("skeleton")
        rows.append((folder, str(a.asset_name), cls, "%.4f" % lib.get_sequence_length(obj),
                     str(lib.get_num_frames(obj)), "%.3f" % lib.get_rate_scale(obj),
                     "on" if obj.get_editor_property("enable_root_motion") else "off",
                     str(len(notifies)), ";".join(str(c) for c in curves),
                     skel.get_name() if skel else ""))
    elif cls == "Skeleton":
        # Sockets are refused to reflection; the reference's text layer lists them.
        rows.append((folder, str(a.asset_name), cls, "", "", "", "", "", "", ""))
    elif cls == "SkeletalMesh":
        mats = obj.get_editor_property("materials")
        rows.append((folder, str(a.asset_name), cls, "", "", "", "", "",
                     ";".join("%s=%s" % (m.material_slot_name, m.material_interface.get_name() if m.material_interface else "none") for m in mats),
                     obj.get_editor_property("skeleton").get_name() if obj.get_editor_property("skeleton") else ""))
    else:
        rows.append((folder, str(a.asset_name), cls, "", "", "", "", "", "", ""))


for a in assets:
    pkg, cls = str(a.package_name), str(a.asset_class_path.asset_name)
    obj = a.get_asset()
    if obj is None:
        failed.append(pkg)
        continue
    folder = pkg[len(ROOT) + 1:].rsplit("/", 1)[0]
    try:
        describe(a, obj, cls, folder)
    except Exception as exc:
        failed.append("%s (%s)" % (pkg, exc))
    if len(rows) % 40 == 0:
        unreal.SystemLibrary.collect_garbage()

saved = unreal.EditorAssetLibrary.save_directory(ROOT, False, True)
unreal.log("AUDIT save_directory: %s" % saved)

project = unreal.Paths.convert_relative_path_to_full(unreal.Paths.project_dir())
path = os.path.join(project, "Docs", "Melee-Delivery.tsv")
with open(path, "w", encoding="utf-8", newline="\n") as f:
    f.write("# The melee delivery as it sits under /Game/Fathom/Melee: one row per asset, written by\n")
    f.write("# Tools/Editor/audit-melee-delivery.py after the import. Regenerate after any change there.\n#\n")
    f.write("folder\tasset\tclass\tlength_s\tframes\trate_scale\troot_motion\tnotifies\tcurves_or_sockets_or_materials\tskeleton\n")
    for r in rows:
        f.write("\t".join(r) + "\n")
unreal.log("AUDIT assets=%d loaded=%d failed=%d outside=%d" % (len(assets), len(rows), len(failed), len(outside)))
for pkg in failed:
    unreal.log("AUDIT failed: %s" % pkg)
