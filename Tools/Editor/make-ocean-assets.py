"""Creates or rewrites the ocean's assets under /Game/Fathom/Ocean: the parameter collection
MPC_FMOcean, the surface material M_FMOcean, the probe material M_FMOceanProbe.

    python Tools/Editor/run-in-editor.py Tools/Editor/make-ocean-assets.py

PIE must be stopped. Both materials evaluate Shaders/FMOcean.ush through a custom node; the
surface reads the collection, the probe reads its own vector parameters. Saves the three assets.
"""
import unreal

PATH = "/Game/Fathom/Ocean"
INCLUDE = "/Project/FMOcean.ush"
VECTORS = ("Globals", "W0", "W1", "W2", "W3")

tools = unreal.AssetToolsHelpers.get_asset_tools()
mel = unreal.MaterialEditingLibrary
eal = unreal.EditorAssetLibrary


def asset(name, cls, factory):
    path = "%s/%s" % (PATH, name)
    if eal.does_asset_exist(path):
        return eal.load_asset(path)
    return tools.create_asset(name, PATH, cls, factory)


def custom_node(mat, code, inputs):
    node = mel.create_material_expression(mat, unreal.MaterialExpressionCustom, -300, 0)
    node.set_editor_property("code", code)
    node.set_editor_property("output_type", unreal.CustomMaterialOutputType.CMOT_FLOAT3)
    node.set_editor_property("include_file_paths", [INCLUDE])
    ins = []
    for n in inputs:
        ci = unreal.CustomInput()
        ci.set_editor_property("input_name", n)
        ins.append(ci)
    node.set_editor_property("inputs", ins)
    return node


mpc = asset("MPC_FMOcean", unreal.MaterialParameterCollection, unreal.MaterialParameterCollectionFactoryNew())
params = []
for n in VECTORS:
    p = unreal.CollectionVectorParameter()
    p.set_editor_property("parameter_name", n)
    params.append(p)
mpc.set_editor_property("vector_parameters", params)

surface = asset("M_FMOcean", unreal.Material, unreal.MaterialFactoryNew())
mel.delete_all_material_expressions(surface)
surface.set_editor_property("two_sided", True)
node = custom_node(surface, "return FMOceanDisplace(WorldPos.xy, Globals, W0, W1, W2, W3);", ("WorldPos",) + VECTORS)
world_pos = mel.create_material_expression(surface, unreal.MaterialExpressionWorldPosition, -700, -120)
mel.connect_material_expressions(world_pos, "", node, "WorldPos")
for i, n in enumerate(VECTORS):
    cp = mel.create_material_expression(surface, unreal.MaterialExpressionCollectionParameter, -700, 80 * i)
    cp.set_editor_property("collection", mpc)
    cp.set_editor_property("parameter_name", n)
    mel.connect_material_expressions(cp, "", node, n)
mel.connect_material_property(node, "", unreal.MaterialProperty.MP_WORLD_POSITION_OFFSET)
color = mel.create_material_expression(surface, unreal.MaterialExpressionConstant3Vector, -300, 320)
color.set_editor_property("constant", unreal.LinearColor(0.01, 0.08, 0.16, 1.0))
mel.connect_material_property(color, "", unreal.MaterialProperty.MP_BASE_COLOR)
rough = mel.create_material_expression(surface, unreal.MaterialExpressionConstant, -300, 460)
rough.set_editor_property("r", 0.15)
mel.connect_material_property(rough, "", unreal.MaterialProperty.MP_ROUGHNESS)

probe = asset("M_FMOceanProbe", unreal.Material, unreal.MaterialFactoryNew())
mel.delete_all_material_expressions(probe)
probe.set_editor_property("shading_model", unreal.MaterialShadingModel.MSM_UNLIT)
node = custom_node(probe, "return FMOceanProbe(UV, Globals, W0, W1, W2, W3, Probe);", ("UV",) + VECTORS + ("Probe",))
uv = mel.create_material_expression(probe, unreal.MaterialExpressionTextureCoordinate, -700, -120)
mel.connect_material_expressions(uv, "", node, "UV")
for i, n in enumerate(VECTORS + ("Probe",)):
    vp = mel.create_material_expression(probe, unreal.MaterialExpressionVectorParameter, -900, 80 * i)
    vp.set_editor_property("parameter_name", n)
    if not mel.connect_material_expressions(vp, "RGBA", node, n):
        append = mel.create_material_expression(probe, unreal.MaterialExpressionAppendVector, -600, 80 * i)
        mel.connect_material_expressions(vp, "RGB", append, "A")
        mel.connect_material_expressions(vp, "A", append, "B")
        mel.connect_material_expressions(append, "", node, n)
mel.connect_material_property(node, "", unreal.MaterialProperty.MP_EMISSIVE_COLOR)

for m in (surface, probe):
    errors = mel.recompile_material(m)
    print("%s: %s" % (m.get_name(), "compiled" if not errors else "; ".join(str(e) for e in errors)))
for a in (mpc, surface, probe):
    print("%s saved=%s" % (a.get_name(), eal.save_loaded_asset(a)))
