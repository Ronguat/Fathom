# Unreal findings — what the surfaces reach, and what was learned trying

**Trigger: about to conclude something cannot be done, or about to record that it cannot.** This is
the lookup half of `Docs/Working-In-Unreal.md`: that file is read front to back before engine work,
and this one is consulted with a specific question in hand. Every measurement below predates this
project, made on the same engine version in the project this one was seeded from, and a finding is
an engine fact regardless of which project measured it. An asset or tool it names that is not in
this repository does not exist here: a route through one is a route this project would build again,
with the engine symbol already named.

**How to read it.** The **capability register** below is the working section; the **dated findings**
under it are an archive reached by search, never read front to back. `Working-In-Unreal.md`'s
opening section — how to treat a limit, and the four surfaces — governs everything here and is not
repeated.

**Every claim names a surface and a date**, and `Tools/DocsCheck/claim-scan.pl` fails the build
without them. An absence with neither reads identically whether it was tested once against one
toolset or everywhere.

**Silence here is not a limit.** A capability absent from the register has not been tested, not been
refuted.

---

## What is and is not scriptable

**Enumerate before concluding a toolset cannot do something.** `describe_toolset` overflows the
response limit on a large toolset, and the overflow is written to a file — so grep the file for
tool names rather than reading it:

```bash
grep -o '"name":"<toolset>\.[a-z_]*"' <saved-file> | sed 's/.*\.//;s/"//' | sort -u
```

That returned 51 tools for `BlueprintTools` in one call.

**The Unreal Python API is reachable, and nothing needs installing** *(confirmed 2026-08-21)*.
`PythonScriptPlugin` is an engine default, so it is enabled while absent from the uproject's plugin
array — checking there reads as "not installed" and is a filtered view. Editor Python is a second
scripting surface into the editor, wider than the toolsets: **every limit marked *(toolset)* is a
candidate to be lifted through it.** The cheap route is `Tools/Editor/run-in-editor.py`, the
remote-execution pipe; the fallback is a two-step REPL, `Type` a `py` statement into the status
bar's Cmd textbox with `submit: true`, then read what it printed with `LogsToolset.GetLogEntries` on
category `LogPython`.

**`ProgrammaticToolset` is not that surface** *(MCP, confirmed 2026-08-22, re-confirmed 2026-08-27)*
— its sandbox refuses `import unreal` and allows exactly `{math, datetime, re, time, json, copy}`. A
sandbox policy, not an API limit.

**Python wraps a class, struct or enum only if it is script-exposed** *(Python and the plugin
source, 2026-09-07)*: `PyGenUtil.cpp`'s `ShouldExportClass` wants `BlueprintType` on the class or
an ancestor, or a Blueprint-exposed field, and `ShouldExportEnum` the same on the enum. A plain
`UDataAsset` or `UDeveloperSettings` subclass and a plain `UENUM` get no wrapper: `dir(unreal)`
omits them, `get_type_from_class` answers the nearest wrapped ancestor, and `set_editor_property`
on their objects fails with *Failed to find property*. `UFMAttackData`, `UFMCombatSettings` and the
combat enums carry `BlueprintType` for this; `UFMShipSettings` and `UFMOceanSettings` do not and
are unreachable from Python.

**There is no IK Rig or IK Retargeter toolset** *(2026-08-21, read off a full `list_toolsets`
response — `ControlRigTools` is Control Rig, a different system, and `SkeletalMeshTools` is mesh,
bones and sockets)*. **Python carries the system in full** *(confirmed 2026-08-27)*: 129
`IKRig`/`IKRetarget` classes are exposed when the `IKRig` plugin is enabled, so the absent toolset is
a routing fact rather than a limit. Melee's intake retargets or swaps the mannequin through it.

### Assets and levels

- **Creating levels, BlendSpaces, AnimBlueprints, montages and curves from scratch is scriptable**
  *(Python, 2026-08-24)*: `LevelFactory`, `WorldFactory`, `BlendSpaceFactoryNew`, `BlendSpaceFactory1D`,
  `AnimBlueprintFactory`, `AnimCompositeFactory`, `AnimMontageFactory` and `CurveFactory`, through
  `AssetTools.create_asset`.
- **Creating a Blueprint asset is scriptable** *(MCP, 2026-08-18)*: `BlueprintTools.create` takes
  `folder_path`, `asset_name` and an `asset_type` class reference and returns the new Blueprint,
  inheriting every C++ default; `set_parent` and `get_parent` reparent one. **`add_variable` does not
  make a variable live; compiling does** *(MCP and Python alike, confirmed 2026-08-15)*: the new
  property is unreadable on the CDO until `compile_blueprint` runs, which reads exactly like the
  write having failed.
- **Creation is per-toolset** *(confirmed 2026-08-12)*: `MaterialTools` and `MaterialInstanceTools`
  create and build whole graphs end to end. Check the toolset that owns the asset type before
  concluding a thing cannot be made.
- **Never duplicate a World Partition level to make a new map** *(engine behaviour, 2026-08-13)*:
  the external actor packages do not re-path and actors silently go missing. File → New Level → Empty.

### Clips, montages and notifies

- **A montage is creatable, notifiable and blend-configurable from Python** *(Python, confirmed
  2026-09-01)*: `AnimMontageFactory` with `target_skeleton` and `source_animation` creates one;
  `AnimationLibrary.add_animation_notify_state_event(montage, track, start, duration, cls)` places a
  notify state on a named track, `add_animation_notify_track` makes the track, and
  `get_animation_notify_events` with `get_anim_notify_event_trigger_time` / `_duration` reads them
  back; `blend_in` / `blend_out` and `blend_out_trigger_time` read and write. The library validates
  the track and `0 ≤ start ≤ play length`, outers the notify to the montage, and **does not dirty the
  package** — `modify()` dirties it, `EditorLoadingAndSavingUtils.save_packages` saves it, `git status`
  confirms it.
- **A clip's pose evaluates from Python at any time** *(Python, 2026-09-07)*:
  `AnimPoseExtensions.get_anim_pose_at_time(sequence, seconds, options)` with
  `AnimPoseEvaluationOptions.optional_skeletal_mesh` set returns a pose whose `get_bone_pose`,
  `get_socket_pose` and `get_ref_bone_pose` answer in `AnimPoseSpaces.WORLD`, which is component
  space; `MathLibrary.compose_transforms` then carries it into pawn space. The deprecated
  `AnimationLibrary.get_bone_pose_for_time` is not needed.
- **Bone poses come off the AnimSequence, never the montage** *(Python, 2026-09-01)*: a montage has
  no bone tracks, and sampling one returns zero motion at every frame, which reads as a clip that
  does not move. Melee's blade curves are baked from the sequences.
- **`AssetTools.duplicate` clones a montage with its skeleton intact, and its segment repoints by
  writing `slotAnimTracks` whole** *(MCP, 2026-08-15)*; that write is live, not a round-trip. Derived
  state is what stops it: `sequenceLength` keeps the source's value and does not recompute after a
  reflection write, and opening the montage recomputes it unaided. `slotAnimTracks` reads back in
  full structural detail *(2026-08-21)*.
- **Duplication carries the source's notifies** *(Python, 2026-08-27)*: a cloned montage brings its
  notify states with it, so never clone one carrying a timing window to make one that must not carry
  it. Read a duplicate's notifies before trusting it; the check is one `get_animation_notify_events`.
- **Montage sections read from Python and write from C++** *(Python and headers, 2026-08-24 to
  2026-08-28)*: `get_num_sections` and `get_section_name` read them; `ENGINE_API
  AddAnimCompositeSection(FName, float)` writes them. `composite_sections` as a property is refused
  from Python — *"is protected and cannot be read"* — so `FCompositeSection::NextSectionName`, which
  says whether sections chain, needs `ENGINE_API GetAnimCompositeSection(int32)` from an editor module.
- **`UAnimMontage.blend_mode_in` exposes inertialization per asset** *(Python, confirmed 2026-09-01)*
  and needs an Inertialization node in the anim graph, which is loud when absent: `No Inertialization
  node found for request from AnimGraphNode_Slot_0`.
- **Renaming an AnimNotify class is expensive** *(engine behaviour, 2026-08-15)*: placed notifies
  serialize against the class path.

### Curves and BlendSpaces

- **`UCurveFloat` and `UCurveVector` keys need C++** *(Python re-confirmed shut 2026-08-28; C++ built
  the same day)*. `FloatCurve` answers *"is protected and cannot be read"*, there is no `AddKey`
  UFUNCTION and no curve-editing library; the member is a public `FRichCurve`, so an editor-module
  function library authors keys directly. Creation stays with `AssetTools`.
- **A curve's shape reads from Python** *(Python, 2026-09-07)*: `CurveFloat.get_float_value`,
  `get_time_range` and `get_value_range` sample a `UCurveFloat`, and
  `AnimationLibrary.get_float_keys(sequence, name)` returns an animation curve's key times and
  values. Only the writes need C++. Melee's blade is not a curve asset: `Tools/Editor/bake-attacks.py`
  writes positions per frame into a `UFMAttackData` asset from Python.
- **A BlendSpace's sample count is never changed by reflection write** *(confirmed 2026-08-15)*:
  moving a sample keeps the array length and the cached triangulation valid, and opening the asset
  finishes the rebuild; removing one leaves the triangulation indexing the old length, and the engine
  dies on evaluation with `Array index out of bounds`. `UBlendSpace` exposes `AddSample`, `DeleteSample`,
  `EditSampleValue` and `ReplaceSampleAnimation` as `ENGINE_API` *(headers, 2026-08-24)*, which is the
  editor's own path.

### Skeletons

- **An animation asset's skeleton pointer is read-only, and the bulk route deletes things**
  *(Python, 2026-08-24)*. `Skeleton` refuses a reflection write on `AnimSequence`, `AnimMontage` and
  `SkeletalMesh` alike, and the anim types carry no `SetSkeleton` UFUNCTION, so there is no per-asset
  repoint. Three routes exist: `SkeletalMesh` reaches `SetSkeleton` through `call_method`;
  `AnimBlueprint.target_skeleton` is a plain writable property; and
  `EditorAssetLibrary.consolidate_assets(target, [sources])` repoints every referencer at once.
  Consolidate is per-skeleton, not per-asset, **deletes the sources** leaving `ObjectRedirector`s, and
  rewrites only packages already loaded — the rest resolve through the redirector until each is
  loaded, `modify()`d and saved. There is no `FixupReferencers` on `AssetTools` or `EditorAssetLibrary`
  *(Python, 2026-08-24)*; load-and-re-save is the fixup. Referencer counts stay stale until
  `scan_paths_synchronous(force_rescan=True)`; `git status` is the check, not the registry.
- **Sockets and bones read from Python through the mesh, no editor needed** *(Python, 2026-09-07)*:
  `SkeletalMesh.num_sockets` and `get_socket_by_index` list the mesh's and the skeleton's sockets
  with `socket_name`, `bone_name` and the relative transform, where `Skeleton.Sockets` is refused
  to reflection; a `SkeletalMeshComponent()` built in Python with `set_skeletal_mesh_asset` answers
  `get_num_bones`, `get_bone_name` and `get_bone_index` outside any world.
- **Bone positions in world space are readable live in PIE from Python** *(2026-08-28)*:
  `SkeletalMeshComponent.get_socket_location` resolves bone names, not only sockets, during a play
  session. With `GameplayStatics.set_global_time_dilation` it charts any bone through any event.

### Anim graphs and state machines

- **AnimGraph editing is scriptable from the toolset** *(confirmed 2026-08-11; `break_pins` read off
  the live schema 2026-08-27)*: `BlueprintTools` has `list_graphs`, `find_nodes`, `get_node_infos`,
  `create_node`, `connect_pins`, `break_pins`, `set_pin_value`, `retarget_node_class`,
  `compile_blueprint`, `delete_node`. Reconnecting an exec output replaces its existing link, which
  splices a node into a running chain. Change a node by a **partial** write to its `Node` struct; a
  full write clobbers pin-backed fields. An Inertialization node was created, spliced between a slot
  and the output, compiled and driven in PIE this way *(MCP, confirmed 2026-09-02)*.
- **Reading a state machine is fully open from Python** *(measured 2026-08-24)*:
  `BlueprintEditorLibrary.list_graphs` returns every graph in an AnimBlueprint as an object, and
  `graph.get_outer()` hands back the `AnimStateNode` / `AnimStateTransitionNode` that owns each.
  `find_nodes` and `get_node_infos` work on a state's interior **when the graph is addressed by its
  full object path**; addressed by name they return `[]`. `ObjectTools.list_properties` /
  `get_properties` / `set_properties` work on state and transition nodes by path.
- **Creating states and transitions needs C++, and the engine's own entry point is exported**
  *(headers and C++, 2026-08-24)*. `create_node` and `find_node_types` resolve the owning Blueprint
  from the graph's **immediate** outer, which for a nested graph is a node — *"Cannot cast type
  'AnimGraphNode_StateMachine' to 'Blueprint'"*, identical by path and by name. `EdGraph.Nodes` refuses
  reflection both directions and `EdGraph` exposes no methods to Python. From C++:
  `FEdGraphSchemaAction_NewStateNode::PerformAction` is `ANIMGRAPH_API` with a public header-inline
  `SpawnNodeFromTemplate<>`; `UEdGraph::Nodes` is public; `FBlueprintEditorUtils::FindBlueprintForGraph`
  walks the whole outer chain. An editor-module tool built on these read a machine's nodes, created a
  named state with its graph and result node, created a transition with its rule graph, populated a
  state with a sequence player and three transition rules, and compiled to a valid
  `AnimBlueprintGeneratedClass` *(C++, 2026-08-24)*. `SpawnNodeFromTemplate` carries a deprecating
  vector parameter, so the route is engine-version-coupled.
- **Transition direction is unreadable from Python or MCP, and C++ lifts it** *(re-tested
  2026-08-27)*: `UAnimStateTransitionNode::GetPreviousState()` and `GetNextState()` are public
  `ANIMGRAPH_API`, `AnimStateTransitionNode.h:175-176`.
- **A clipboard route exists in C++** *(headers, 2026-08-27)*: `FEdGraphUtilities::ExportNodesToText`,
  `ImportNodesFromText` and `CanImportNodesFromText` are `UNREALED_API`, `EdGraphUtilities.h:110-127`.
- **An anim node's On Update / On Become Relevant binds to a C++ function library, with no custom
  `UAnimInstance`** *(proven 2026-08-25)*. `UAnimGraphNode_Base`'s three function properties carry
  `AllowFunctionLibraries`; the contract is the `PrototypeFunction` named in the property's metadata —
  `(const FAnimUpdateContext&, const FAnimNodeReference&)` — plus `meta=(BlueprintThreadSafe)`, and a
  clean compile is the proof the binding took. **Writing the binding needs C++**: `FMemberReference`'s
  members are private `SaveGame` UPROPERTYs, so `get_editor_property` returns an opaque empty struct,
  while `SetExternalMember(FName, TSubclassOf<UObject>)` is `ENGINE_API`; the reference reaches the
  runtime node only at compile. `USequencePlayerLibrary` exposes `SetAccumulatedTime`, `SetPlayRate`,
  `SetSequence` and `ComputePlayRateFromDuration`, all `BlueprintThreadSafe`; the tick record advances
  the accumulator by `delta * rate` **after** the update function runs, so driving the playhead
  explicitly needs the rate at zero.
- **Optional anim-node properties become pins by writing `ShowPinForProperties`** *(confirmed
  2026-08-14)*: flip `bShowPin` where `bCanToggleVisibility` is true, compile, read the node back.
  `set_pin_value` then swaps the asset without touching the graph, which is the cheapest way to
  eyeball a clip: point the pin at it, PIE, capture, point it back.
- **A pawn's anim class swaps at runtime from Python** *(Python, 2026-09-02)*:
  `SkeletalMeshComponent.set_anim_instance_class(cls)` on the PIE pawn takes effect at once; restore
  by loading the shipping asset's `generated_class()`, since the component exposes no getter.
- **Absence of a montage is not absence of an animation** *(fixture behaviour, 2026-08-24)*: a
  state-machine state has no asset named for it, so a file search and a source grep both return
  nothing while the animation plays. Check whether the state's getter is `BlueprintPure` — that is the
  tell that the state machine can reach it. And **prove the instrument before believing an empty
  `find_nodes`**: it returns nodes on a top-level graph and `[]` on a nested one addressed by name.

### Pictures

- **The user can send images**, and this toolset's hard limits are overwhelmingly visual — a state
  graph's interior, a notify track, a BlendSpace grid, a details panel. Ask. But a screenshot of
  something `get_node_infos` can report is slower *and* worse, and an image request resting on a
  stale assumption re-certifies the wall as fact: check the limit is real first.
- **`CaptureViewport` renders the *editor* world, not PIE** *(2026-08-24)*, and returns the PNG inline
  as base64, which overflows the response limit into a tool-results file.
  **`AutomationLibrary.take_high_res_screenshot` is the PIE route** *(2026-08-24)*: it captures the
  game viewport with the debug HUD live and writes straight to `Saved/Screenshots/WindowsEditor/`.
  Shots stall the tick, so a timing measured in the same run as a shot is contaminated *(fixture
  behaviour, 2026-09-02)*.
- **Navigating to a state graph's interior is a C++ route, not an absence** *(headers, 2026-08-27)*:
  `FKismetEditorUtilities::BringKismetToFocusAttentionOnObject(const UObject*)` is `UNREALED_API`,
  `KismetEditorUtilities.h:442`, and `FBlueprintEditor` exposes `OpenGraphAndBringToFront`,
  `JumpToHyperlink` and `JumpToPin`, `BlueprintEditor.h:260-265`. Python reaches none of them
  *(2026-08-27)*: `AssetEditorSubsystem.open_editor_for_assets` opens the asset and
  `BlueprintEditorLibrary` offers only `refresh_*`.

---

## Dated findings — newest first

An archive reached by search. Each entry keeps its date and the surface it measured on.

## 2026-09-08 — A movement mode's outer is its mover component

**A Mover movement mode created with the pawn as its outer crashes the editor at a row's first
frame** *(C++, 2026-09-08)*: `UBaseMovementMode` finds its component by casting its outer, and the
walking and falling modes dereference it when registered, an access violation in the Mover
module reading a small address. A mode added from the pawn's constructor with
`CreateDefaultSubobject` has the pawn as outer; only the runtime `AddMovementModeFromObject`
reparents. **Stand in for a default mode through the pawn's object initializer**:
`SetNestedDefaultSubobjectClass<UFMDeckWalkingMode>(TEXT("Mover.DefaultWalkingMode"))` in the
pawn constructor's `Super(...)` call, and the mode is born inside the component with the right
outer. A mode that never reads its component, the swim mode here, survives the wrong outer, which
is why the earlier pattern looked safe.

## 2026-09-07 — The melee intake reads the delivery and bakes the blade from Python

**The delivery answered its own contract from Python** *(Python, 2026-09-07)*: the skeleton's
sockets through the mesh, `weapon_rSocket` on `weapon_r` at zero offset and `headSocket` on `head`
among eight; the seventy bone names through a component built in Python; every release curve's
shape through `CurveFloat.get_float_value`, a 0 to 1 ramp over one second; the `AutoAlignment`
keys through `AnimationLibrary.get_float_keys`. **The blade bakes from Python**: `AnimPoseExtensions`
evaluates each clip at every frame and `bake-attacks.py` writes the socket's position and the
blade's tip into a `UFMAttackData` asset created through `AssetTools.create_asset` with
`DataAssetFactory`. **The wrappers had to be asked for**: the class and its enums were invisible
to Python until marked `BlueprintType`, the register's rule above. **`Shot` from a play world's
console took the editor viewport six times in six** *(PIE, 2026-09-07)*, its camera far off the
map looking at sky, where `AutomationLibrary.take_high_res_screenshot` took the first client's
game viewport every time; `Tools/Editor/melee_capture.py` uses the latter. Both stall the tick,
so the frames a capture is asked for arrive late and bunched.

## 2026-09-05 — A delivery moves between projects by a rename in the source editor

**A file copy across a plugin mount breaks imports** *(engine behaviour, 2026-09-05)*: packages
store absolute package paths, so an asset copied from a plugin's content folder into `/Game/` on
disk resolves none of its references. `EditorAssetLibrary.rename_asset` inside the source
project's editor rewrites every referencer and leaves redirectors behind; a folder renamed that
way and then copied resolved every reference in the receiving editor, 123 of 123 *(Python,
2026-09-05)*, and `EditorAssetLibrary.save_directory` with `only_if_is_dirty` false resaved them
as this engine's packages. `AssetRegistry.get_dependencies` with hard package references only
walks the closure without play, and a native class reached through a physics asset showed there
as its `/Script/` module. A clip's contract reads from `AnimationLibrary`: `get_sequence_length`,
`get_num_frames`, `get_rate_scale`, `get_animation_notify_events`, `get_animation_curve_names`
*(Python, 2026-09-05)*; `SkeletalMesh.materials` writes whole as a list of `SkeletalMaterial`, and
`physics_asset`, `post_process_anim_blueprint` and `default_animating_rig` clear to None.

## 2026-09-05 — What the designer's review found in the rendered frame

**A dynamic mesh with no material assigned flickers against the sky** *(PIE, 2026-09-05)*: the
ship's `UDynamicMeshComponent`, built by Geometry Script with no material set, reported no material
slot and rendered through the engine's default surface material. Wherever the sky lay behind it,
its pixels came out washed toward the sky's colour with the boundary on the horizon line, and the
designer saw the mast and the deck flicker out every frame; the pawns' static meshes and the
ocean's dynamic mesh, both carrying a material, were untouched. Anti-aliasing, motion blur,
screen-space reflections, fog and the aerial perspective's fast apply made no difference toggled
at runtime; disabling the atmosphere removed the wash; assigning any material removed it and, on
the designer's word, the flicker. A dynamic mesh gets its material in the constructor.

**Mover places a based simulated proxy through the base pose captured with its state** *(engine
source, 2026-09-05)*: `FMoverDefaultSyncState::Interpolate` interpolates in base space when the
bases match, `GetLocation_WorldSpace` then transforms by the captured `MovementBasePos` and
`MovementBaseQuat`, and `UMoverComponent::FinalizeFrame` sets the component there. On a client
whose base is predicted ahead, the proxy trails the base by the round trip and the lead: 230 cm at
100 ms on a ship at 8 m/s, and a bob against the deck when still. `OnPostFinalize` fires after
that placement, and a re-placement through the base's current transform there holds until the
next finalize.

**A console command sent with a world context never reaches the play viewport** *(PIE,
2026-09-05)*: `viewmode` and `show` through `SystemLibrary.execute_console_command(world, ...)`
changed nothing in the captures; a player controller's `console_command` is the route. The `Shot`
command captures the next viewport drawn: three times in four the play viewport the editor hosts,
once an editor viewport showing the editor world.

## 2026-09-05 — What the loop's first drive found Python does and does not reach

**Play settings are not in Python** *(Python, 2026-09-05)*: `unreal.LevelEditorPlaySettings` does
not exist, and the CDO loaded by path, `/Script/UnrealEd.Default__LevelEditorPlaySettings`, answers
*Failed to find property* for `play_net_mode`. The gitignored user ini is the read, and it is what
the running editor loaded.

**Pawn and player-state accessors are properties, not methods** *(Python, 2026-09-05)*:
`Pawn.get_player_state()` and `PlayerState.get_player_id()` do not exist;
`get_editor_property("player_state")` and `get_editor_property("player_id")` answer.

**`UnrealEditorSubsystem.get_editor_world()` answers None during play** *(Python, 2026-09-05)*,
with `LogUtils: Error: The Editor is currently in a play mode`. A PIE world is still addressable
by package path, `/Game/Fathom/Maps/UEDPIE_<n>_L_Harness.L_Harness`, from the level's package name
read before play starts.

**The engine ships no capsule among its basic shapes** *(engine content, 2026-09-05)*:
`/Engine/BasicShapes/` holds Cone, Cube, Cylinder, Plane and Sphere.

**A teleported Mover pawn hovers in Walking until something changes its input** *(PIE,
2026-09-05)*: dropped by a teleport effect 270 cm above a deck it stayed at the drop height with
no base for a whole row under any emulated latency, and fell at once at zero; a teleport effect
that also writes `Falling` into the output sync state lands every time. Mover's mode classes
are `MinimalAPI` with generated constructors, so a C++ mode derives from `UBaseMovementMode` and
uses the exported `UMovementUtils` statics rather than subclassing `UFlyingMode`; a mode's
`GetWorld()` reaches the world subsystems, where a context lookup through the mover component
returned nothing inside the simulation.

**A server call made from the Slate post-tick callback never reaches the server** *(PIE,
2026-09-05)*: `ServerDriveShip`, a reliable server function on the client's pawn and then on its
player controller, called from the runner's callback between world ticks, produced no server
execution and no warning, twice; the same function queued and sent from the controller's
`PlayerTick` reached the server every time, and the trace relay, sent from a world-tick delegate,
always had. Send a client's server call from inside the world's tick.

**Materials are authored from Python** *(Python, 2026-09-05, `Tools/Editor/make-ocean-assets.py`)*:
`AssetTools.create_asset` with `MaterialFactoryNew` and `MaterialParameterCollectionFactoryNew`,
`MaterialEditingLibrary.create_material_expression`, `connect_material_expressions` by output and
input name, `connect_material_property`, `recompile_material` returning the compile errors,
`EditorAssetLibrary.save_loaded_asset`. A custom node takes `code`, `output_type`, `inputs` as
`CustomInput` structs and `include_file_paths`. A vector parameter's four-float output is named
`RGBA`; its unnamed-looking main pin is `RGB`, three floats. A collection parameter's output is
four floats already. Two materials compiled and rendered from this route; the compile errors read
back through the return value named the file and line.

**`/Project` is already a shader directory** *(engine source and a crash, 2026-09-05)*:
`LaunchEngineLoop.cpp` line 2557 maps it to `<project>/Shaders` when that folder exists, so a
material custom node includes `/Project/FMOcean.ush` with no module code, and a module mapping it
again asserts at startup. The module's loading phase stays Default.

## 2026-09-03 — The clock, PIE and the shipping input path are all scriptable

**A fixed time step is a function library away** *(C++ headers then an editor module, 2026-09-03)*.
`UEngine::UpdateTimeAndHandleMaxTickRate` reads `FApp::UseFixedTimeStep()` every tick under
`WITH_FIXED_TIME_STEP_SUPPORT`, which `TargetRules.bWithFixedTimeStepSupport` defaults to true, and
`FApp::SetUseFixedTimeStep` / `SetFixedDeltaTime` are public statics. **Only the command line sets
them otherwise**: no console variable and no Python symbol does. `UFMTimeTools` wraps them and returns
`FApp::UseFixedTimeStep()`, so a build without support reads back false rather than free-running
silently. **Measured**: 1800 ticks at 1/60 gave exactly 30.000 s of game time in 15.5 s of wall, and
ten samples of a span that spread 35 ms free-running came back identical.

**PIE is driven from in-editor Python** *(Python, 2026-09-03)*. `LevelEditorSubsystem` exposes
`editor_request_begin_play`, `editor_request_end_play` and `is_in_play_in_editor`. Both requests are
asynchronous, begin play uses the first active level viewport and takes no start transform, and
`unreal.log_flush()` plus a share-readable live log let a script tail its own trace.

**Input reaches the shipping path, not just the action** *(C++ headers then PIE, 2026-09-03)*.
`APlayerController::InputKey(const FInputKeyEventArgs&)` is `ENGINE_API` and is the call the game
viewport makes; `FInputKeyEventArgs::CreateSimulated` builds the args. So `UFMInputTools::InputKey`
runs key state, the mapping context and its modifiers, the action's triggers and the binding, where
Enhanced Input's `InjectInputForAction` enters one layer lower and bypasses the mapping. **Proven on a
player pawn**: a single key tap produced the input line, the activation and the release timing.

**Handles that are not where you would look.** `FKey`'s name is **not** reflection-writable —
`set_editor_property("key_name", ...)` returns success and leaves the struct empty — so build one with
`import_text("LeftMouseButton")`. Python property names drop the `b` prefix.
`EditorLevelLibrary.get_all_level_actors` is deprecated and still works; `EditorActorSubsystem` is the
replacement. `unreal.EditorPerformanceSettings` is not exposed to Python. The engine's interpreter is
`C:/Program Files (x86)/UE_5.8/Engine/Binaries/ThirdParty/Python3/Win64/python.exe` and there is no
other Python on this machine. The raw log's prefix is `[wall clock][frame % 1000]`; a PIE session is
bounded by `Bringing World` and `BeginTearingDown`; `unreal.log` lines land in the project log under
`LogPython` with the same prefix.

**Movement mode and velocity are reachable on a Character pawn from Python** *(2026-09-03)*:
`pawn.get_editor_property("character_movement").set_movement_mode(unreal.MovementMode.MOVE_FLYING, 0)`
holds a pawn at a height, and `pawn.get_velocity()` says when a carry has finished.

**`GameplayStatics.create_player` is available** *(Python, 2026-09-03)* and was not used; the loop
here takes its clients from the play settings instead.

## 2026-09-02 — Small facts from a charting script

**A run script cannot receive arguments through the remote-execution pipe** *(Python, 2026-09-02)*:
`sys.argv` inside the editor is not the caller's, so every script here reads a JSON beside itself.
**The editor's Python does not open Git Bash paths**: `/c/Users/...` raises `FileNotFoundError`;
`C:/Users/...` works. **A live instance refuses `EditDefaultsOnly` writes on both scripting surfaces**
*(Python and MCP, 2026-09-02)* — *"cannot be edited on instances"* — so a value trial goes through the
CDO between PIE sessions: write it, `compile_blueprint`, start PIE, leave the package unsaved and it
reverts with the editor.

## 2026-08-28 — `/mcp` Reconnect registers a failed server mid-session

Every route *inside* the session is shut, but the client's `/mcp` panel carries a **Reconnect**
control, and using it delivered both servers' tools inside the turn with no restart *(MCP, confirmed
2026-08-28)*. **Four server states, and one is stuck** *(Bash, 2026-08-28)*: **connecting** delivers
its tools when ready, **connected-then-dropped** redials itself, **disconnected** withdraws them, and
**failed at the initial connect** stays failed until something reconnects it. Reconnection repairs a
stream that dropped on an established session; a failed initial connect never built one.

**`reconnectMcpServer` is an IPC method behind a UI control that the model cannot reach** *(Bash,
confirmed 2026-08-28 — no tool exposes it, no listener, no matching named pipe)*. So the move is to
ask for the click. **A config write does not reach the running session** *(Bash, confirmed
2026-08-28)*: the client list is built at startup, so a server added to `.mcp.json` mid-session
constructs no client. Whether Reconnect re-reads config is untested.

**The plugin is an ordinary MCP streamable-HTTP server and `curl` speaks to it directly** *(Bash,
confirmed 2026-08-28)*: `initialize` with `protocolVersion` `2025-06-18`, keep the `Mcp-Session-Id`
response header, send `notifications/initialized`, then `tools/list` and `tools/call`. Responses
arrive as plain JSON, not SSE; the session id survives across separate shell calls; `toolset_name`
wants the full dotted path `list_toolsets` prints. `Tools/McpBridge/ue-mcp.sh` is that route. The
harness's `ConnectionRefused` is a recording of session start, not a probe; the port is the live
signal.

## 2026-08-27 — A claim about a surface names the surface and the date

**What the pattern was.** Not wrong facts: claims that omitted which surface was tried and when. An
MCP-only result read identically to one tested across MCP, editor Python and C++, so nothing invited
a re-test and the claim hardened into a constraint — once into a *"design constraint"* that shaped a
plan against a capability in daily use. Across three sweeps, fourteen surface claims were re-tested
and twelve fell, and every refutation was already inside the repository: four lines below, in the
same tool list, or in that day's own commit.

**The mechanism**: `Tools/DocsCheck/claim-scan.pl`, wired as a FAIL. A block is shortlisted when it
carries an absence phrase, names a callable, and lacks either a surface word or an ISO date. Three
"no surface reaches this" categories satisfy it — `engine behaviour`, `fixture behaviour`, `machine
fact` — because each tells the reader not to hunt a wider surface. **What it does not catch**: a
claim phrased without a callable, and a wrong claim that carries a surface and a date. It enforces
form, not truth.

**Two test-design errors worth copying.** Four probes returned nothing because the address was wrong,
not because the capability was absent — a guessed API name, and the Blueprint asset where the CDO was
wanted. And `is_editor_property_overridden` returns an enum whose every member is truthy: tested with
`if r:` it reported every name as overridden; compared against `EditorPropertyValueState.OVERRIDDEN`
it reported none. **A test that cannot return "no" is not a test.**
