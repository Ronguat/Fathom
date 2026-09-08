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
surface.set_editor_property("tangent_space_normal", False)
node = custom_node(surface, "return FMOceanDisplace(WorldPos.xy, Globals, W0, W1, W2, W3);", ("WorldPos",) + VECTORS)
normal = custom_node(surface, "return FMOceanNormal(WorldPos.xy, Globals, W0, W1, W2, W3);", ("WorldPos",) + VECTORS)
normal.set_editor_property("material_expression_editor_y", 200)
# The undisplaced position, so the pixel's normal is the wave's at the same source point the vertex left from.
world_pos = mel.create_material_expression(surface, unreal.MaterialExpressionWorldPosition, -700, -120)
world_pos.set_editor_property("world_position_shader_offset", unreal.WorldPositionIncludedOffsets.WPT_EXCLUDE_ALL_SHADER_OFFSETS)
mel.connect_material_expressions(world_pos, "", node, "WorldPos")
mel.connect_material_expressions(world_pos, "", normal, "WorldPos")
for i, n in enumerate(VECTORS):
    cp = mel.create_material_expression(surface, unreal.MaterialExpressionCollectionParameter, -700, 80 * i)
    cp.set_editor_property("collection", mpc)
    cp.set_editor_property("parameter_name", n)
    mel.connect_material_expressions(cp, "", node, n)
    mel.connect_material_expressions(cp, "", normal, n)
mel.connect_material_property(node, "", unreal.MaterialProperty.MP_WORLD_POSITION_OFFSET)
mel.connect_material_property(normal, "", unreal.MaterialProperty.MP_NORMAL)
# Deep water head-on, the sky's tint at a grazing angle, by the wave's own normal.
deep = mel.create_material_expression(surface, unreal.MaterialExpressionConstant3Vector, -300, 320)
deep.set_editor_property("constant", unreal.LinearColor(0.006, 0.05, 0.11, 1.0))
sky = mel.create_material_expression(surface, unreal.MaterialExpressionConstant3Vector, -300, 420)
sky.set_editor_property("constant", unreal.LinearColor(0.22, 0.40, 0.52, 1.0))
fresnel = mel.create_material_expression(surface, unreal.MaterialExpressionFresnel, -300, 520)
fresnel.set_editor_property("exponent", 4.0)
fresnel.set_editor_property("base_reflect_fraction", 0.03)
mel.connect_material_expressions(normal, "", fresnel, "Normal")
tint = mel.create_material_expression(surface, unreal.MaterialExpressionLinearInterpolate, -100, 400)
mel.connect_material_expressions(deep, "", tint, "A")
mel.connect_material_expressions(sky, "", tint, "B")
mel.connect_material_expressions(fresnel, "", tint, "Alpha")
mel.connect_material_property(tint, "", unreal.MaterialProperty.MP_BASE_COLOR)
spec = mel.create_material_expression(surface, unreal.MaterialExpressionConstant, -300, 620)
spec.set_editor_property("r", 1.0)
mel.connect_material_property(spec, "", unreal.MaterialProperty.MP_SPECULAR)
rough = mel.create_material_expression(surface, unreal.MaterialExpressionConstant, -300, 700)
rough.set_editor_property("r", 0.12)
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
