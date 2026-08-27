import unreal


MAP_PATH = "/Game/Maps/TechDemo"
MATERIAL_FOLDER = "/Game/Materials"


def load_project_class(class_name):
    actor_class = unreal.load_class(None, f"/Script/UEGT1.{class_name}")
    if actor_class is None:
        raise RuntimeError(f"Unable to load /Script/UEGT1.{class_name}; build the editor target first")
    return actor_class


def spawn_actor(actor_subsystem, actor_class, location, label):
    actor = actor_subsystem.spawn_actor_from_class(actor_class, location)
    if actor is None:
        raise RuntimeError(f"Unable to spawn {label}")
    actor.set_actor_label(label)
    return actor


def add_scalar(material, name, value, x, y, prop):
    expression = unreal.MaterialEditingLibrary.create_material_expression(
        material, unreal.MaterialExpressionScalarParameter, x, y
    )
    expression.set_editor_property("parameter_name", name)
    expression.set_editor_property("default_value", value)
    unreal.MaterialEditingLibrary.connect_material_property(expression, "", prop)
    return expression


def create_tech_material(name, mode, roughness, specular, metallic):
    asset_path = f"{MATERIAL_FOLDER}/{name}"
    material = unreal.EditorAssetLibrary.load_asset(asset_path)
    if material is None:
        material = unreal.AssetToolsHelpers.get_asset_tools().create_asset(
            name, MATERIAL_FOLDER, unreal.Material, unreal.MaterialFactoryNew()
        )
    if material is None:
        raise RuntimeError(f"Unable to create {asset_path}")

    material.set_editor_property("used_with_instanced_static_meshes", True)
    material.set_editor_property("two_sided", mode == "foliage")
    if mode == "foliage":
        try:
            material.set_editor_property("shading_model", unreal.MaterialShadingModel.MSM_TWO_SIDED_FOLIAGE)
        except Exception as error:
            unreal.log_warning(f"Two-sided foliage shading model unavailable: {error}")
    unreal.MaterialEditingLibrary.delete_all_material_expressions(material)

    if mode == "terrain":
        source = unreal.MaterialEditingLibrary.create_material_expression(
            material, unreal.MaterialExpressionVertexColor, -720, -100
        )
        source_output = "RGB"
        moss_texture = unreal.EditorAssetLibrary.load_asset(
            "/Engine/StarterContent/Textures/T_ground_Moss_D"
        )
        if moss_texture is None:
            raise RuntimeError("Unable to load the bundled UE moss ground texture")
        moss_sample = unreal.MaterialEditingLibrary.create_material_expression(
            material, unreal.MaterialExpressionTextureSample, -720, -260
        )
        moss_sample.set_editor_property("texture", moss_texture)
        terrain_blend = unreal.MaterialEditingLibrary.create_material_expression(
            material, unreal.MaterialExpressionLinearInterpolate, -455, -125
        )
        terrain_blend.set_editor_property("const_alpha", 0.72)
        unreal.MaterialEditingLibrary.connect_material_expressions(source, "RGB", terrain_blend, "A")
        unreal.MaterialEditingLibrary.connect_material_expressions(moss_sample, "RGB", terrain_blend, "B")
        source = terrain_blend
        source_output = ""
    else:
        source = unreal.MaterialEditingLibrary.create_material_expression(
            material, unreal.MaterialExpressionVectorParameter, -720, -100
        )
        source.set_editor_property("parameter_name", "Color")
        source.set_editor_property("default_value", unreal.LinearColor(0.12, 0.28, 0.09, 1.0))
        source_output = "RGB"

    world_position = unreal.MaterialEditingLibrary.create_material_expression(
        material, unreal.MaterialExpressionWorldPosition, -760, 140
    )
    noise = unreal.MaterialEditingLibrary.create_material_expression(
        material, unreal.MaterialExpressionNoise, -520, 150
    )
    noise.set_editor_property("scale", 0.0018 if mode == "terrain" else 0.0035)
    noise.set_editor_property("levels", 4 if mode == "terrain" else 3)
    noise.set_editor_property("quality", 2)
    noise.set_editor_property("output_min", 0.62 if mode == "terrain" else 0.76)
    noise.set_editor_property("output_max", 0.94 if mode == "terrain" else 1.16)
    noise.set_editor_property("level_scale", 2.4)
    noise.set_editor_property("turbulence", True)
    unreal.MaterialEditingLibrary.connect_material_expressions(world_position, "XYZ", noise, "Position")

    varied_color = unreal.MaterialEditingLibrary.create_material_expression(
        material, unreal.MaterialExpressionMultiply, -270, -60
    )
    unreal.MaterialEditingLibrary.connect_material_expressions(source, source_output, varied_color, "A")
    unreal.MaterialEditingLibrary.connect_material_expressions(noise, "", varied_color, "B")

    base_output = varied_color
    base_pin = ""
    if mode == "water":
        shallow = unreal.MaterialEditingLibrary.create_material_expression(
            material, unreal.MaterialExpressionVectorParameter, -260, -235
        )
        shallow.set_editor_property("parameter_name", "ShallowColor")
        shallow.set_editor_property("default_value", unreal.LinearColor(0.028, 0.16, 0.13, 1.0))
        fresnel = unreal.MaterialEditingLibrary.create_material_expression(
            material, unreal.MaterialExpressionFresnel, -270, 170
        )
        fresnel.set_editor_property("exponent", 6.5)
        blend = unreal.MaterialEditingLibrary.create_material_expression(
            material, unreal.MaterialExpressionLinearInterpolate, -20, -80
        )
        unreal.MaterialEditingLibrary.connect_material_expressions(varied_color, "", blend, "A")
        unreal.MaterialEditingLibrary.connect_material_expressions(shallow, "RGB", blend, "B")
        unreal.MaterialEditingLibrary.connect_material_expressions(fresnel, "", blend, "Alpha")
        base_output = blend

    unreal.MaterialEditingLibrary.connect_material_property(base_output, base_pin, unreal.MaterialProperty.MP_BASE_COLOR)
    add_scalar(material, "Roughness", roughness, 70, 60, unreal.MaterialProperty.MP_ROUGHNESS)
    add_scalar(material, "Specular", specular, 70, 135, unreal.MaterialProperty.MP_SPECULAR)
    add_scalar(material, "Metallic", metallic, 70, 210, unreal.MaterialProperty.MP_METALLIC)
    if mode == "foliage":
        unreal.MaterialEditingLibrary.connect_material_property(varied_color, "", unreal.MaterialProperty.MP_SUBSURFACE_COLOR)

    unreal.MaterialEditingLibrary.recompile_material(material)
    if not unreal.EditorAssetLibrary.save_loaded_asset(material, only_if_is_dirty=False):
        raise RuntimeError(f"Unable to save {asset_path}")
    return asset_path


def create_materials():
    unreal.EditorAssetLibrary.make_directory(MATERIAL_FOLDER)
    paths = [
        create_tech_material("M_TechTerrain", "terrain", 0.91, 0.16, 0.0),
        create_tech_material("M_TechSurface", "surface", 0.94, 0.12, 0.0),
        create_tech_material("M_TechFoliage", "foliage", 0.79, 0.23, 0.0),
        create_tech_material("M_TechWater", "water", 0.035, 0.98, 0.12),
    ]
    unreal.log(f"UEGT1 tech-demo materials authored: {', '.join(paths)}")


def configure_lighting(actor_subsystem):
    sun = spawn_actor(actor_subsystem, unreal.DirectionalLight, unreal.Vector(0.0, 0.0, 6500.0), "Lumen Wilds Sun")
    sun.set_actor_rotation(unreal.Rotator(roll=0.0, pitch=-31.0, yaw=-42.0), False)
    sun_component = sun.get_component_by_class(unreal.DirectionalLightComponent)
    sun_component.set_editor_property("intensity", 60000.0)
    sun_component.set_editor_property("light_color", unreal.Color(255, 232, 207, 255))
    sun_component.set_editor_property("atmosphere_sun_light", True)
    sun_component.set_editor_property("volumetric_scattering_intensity", 1.22)
    sun_component.set_editor_property("contact_shadow_length", 0.018)
    sun_component.set_editor_property("enable_light_shaft_occlusion", True)
    sun_component.set_editor_property("enable_light_shaft_bloom", True)
    sun_component.set_editor_property("cast_cloud_shadows", True)
    sun_component.set_editor_property("cloud_shadow_strength", 0.34)
    sun_component.set_editor_property("cast_shadows_on_atmosphere", True)
    sun_component.set_editor_property("cast_shadows_on_clouds", True)

    sky_light = spawn_actor(actor_subsystem, unreal.SkyLight, unreal.Vector(0.0, 0.0, 4200.0), "Lumen Wilds Sky Light")
    sky_component = sky_light.get_component_by_class(unreal.SkyLightComponent)
    sky_component.set_editor_property("intensity", 1.9)
    sky_component.set_editor_property("real_time_capture", True)
    sky_component.set_editor_property("cubemap_resolution", 256)
    sky_component.set_editor_property("lower_hemisphere_is_black", False)
    sky_component.set_editor_property("lower_hemisphere_color", unreal.LinearColor(0.055, 0.085, 0.065, 1.0))
    sky_component.set_editor_property("volumetric_scattering_intensity", 1.05)

    spawn_actor(actor_subsystem, unreal.SkyAtmosphere, unreal.Vector(), "Lumen Wilds Atmosphere")
    cloud = spawn_actor(actor_subsystem, unreal.VolumetricCloud, unreal.Vector(), "Lumen Wilds Cloudscape")
    cloud_component = cloud.get_component_by_class(unreal.VolumetricCloudComponent)
    cloud_component.set_editor_property("layer_bottom_altitude", 6.5)
    cloud_component.set_editor_property("layer_height", 9.0)

    fog = spawn_actor(actor_subsystem, unreal.ExponentialHeightFog, unreal.Vector(0.0, 3200.0, -160.0), "Lumen Wilds Valley Mist")
    fog_component = fog.get_component_by_class(unreal.ExponentialHeightFogComponent)
    fog_component.set_editor_property("fog_density", 0.0034)
    fog_component.set_editor_property("fog_height_falloff", 0.105)
    fog_component.set_editor_property("fog_max_opacity", 0.46)
    fog_component.set_editor_property("enable_volumetric_fog", True)
    fog_component.set_editor_property("volumetric_fog_scattering_distribution", 0.36)
    fog_component.set_editor_property("volumetric_fog_albedo", unreal.Color(202, 226, 214, 255))
    fog_component.set_editor_property("volumetric_fog_extinction_scale", 0.68)
    fog_component.set_editor_property("volumetric_fog_distance", 28000.0)
    fog_component.set_editor_property("volumetric_fog_start_distance", 500.0)

    post = spawn_actor(actor_subsystem, unreal.PostProcessVolume, unreal.Vector(), "Lumen Wilds Cinematic Grade")
    post.set_editor_property("unbound", True)
    settings = post.get_editor_property("settings")
    settings.set_editor_property("override_auto_exposure_min_brightness", True)
    settings.set_editor_property("override_auto_exposure_max_brightness", True)
    settings.set_editor_property("auto_exposure_min_brightness", 12.05)
    settings.set_editor_property("auto_exposure_max_brightness", 12.05)
    settings.set_editor_property("override_bloom_intensity", True)
    settings.set_editor_property("bloom_intensity", 0.34)
    settings.set_editor_property("override_bloom_threshold", True)
    settings.set_editor_property("bloom_threshold", 1.1)
    settings.set_editor_property("override_vignette_intensity", True)
    settings.set_editor_property("vignette_intensity", 0.10)
    settings.set_editor_property("override_color_saturation", True)
    settings.set_editor_property("color_saturation", unreal.Vector4(1.00, 1.02, 0.98, 1.0))
    settings.set_editor_property("override_color_contrast", True)
    settings.set_editor_property("color_contrast", unreal.Vector4(1.01, 1.01, 1.01, 1.0))
    settings.set_editor_property("override_ambient_occlusion_intensity", True)
    settings.set_editor_property("ambient_occlusion_intensity", 0.35)
    settings.set_editor_property("override_ambient_occlusion_radius", True)
    settings.set_editor_property("ambient_occlusion_radius", 140.0)
    settings.set_editor_property("override_lumen_ambient_occlusion_intensity", True)
    settings.set_editor_property("lumen_ambient_occlusion_intensity", 0.40)
    settings.set_editor_property("override_local_exposure_highlight_contrast_scale", True)
    settings.set_editor_property("local_exposure_highlight_contrast_scale", 0.56)
    settings.set_editor_property("override_local_exposure_shadow_contrast_scale", True)
    settings.set_editor_property("local_exposure_shadow_contrast_scale", 0.52)
    settings.set_editor_property("override_local_exposure_detail_strength", True)
    settings.set_editor_property("local_exposure_detail_strength", 1.0)
    post.set_editor_property("settings", settings)

    spawn_actor(actor_subsystem, unreal.SphereReflectionCapture, unreal.Vector(0.0, 3200.0, 900.0), "Lumen Wilds Lake Reflection")


def main():
    create_materials()
    level_subsystem = unreal.get_editor_subsystem(unreal.LevelEditorSubsystem)
    actor_subsystem = unreal.get_editor_subsystem(unreal.EditorActorSubsystem)

    if unreal.EditorAssetLibrary.does_asset_exist(MAP_PATH):
        if not level_subsystem.load_level(MAP_PATH):
            raise RuntimeError(f"Unable to load {MAP_PATH}")
        for actor in actor_subsystem.get_all_level_actors():
            if not actor_subsystem.destroy_actor(actor):
                raise RuntimeError(f"Unable to remove existing actor {actor.get_name()}")
    elif not level_subsystem.new_level(MAP_PATH):
        raise RuntimeError(f"Unable to create {MAP_PATH}")

    environment_class = load_project_class("UEGT1TechDemoEnvironment")
    environment = spawn_actor(actor_subsystem, environment_class, unreal.Vector(), "Lumen Wilds Procedural Environment")
    environment.set_editor_property("is_spatially_loaded", False)
    configure_lighting(actor_subsystem)

    player_start = spawn_actor(actor_subsystem, unreal.PlayerStart, unreal.Vector(0.0, -13200.0, 300.0), "Lumen Wilds Player Start")
    player_start.set_actor_rotation(unreal.Rotator(roll=0.0, pitch=0.0, yaw=90.0), False)

    world = unreal.get_editor_subsystem(unreal.UnrealEditorSubsystem).get_editor_world()
    world_settings = world.get_world_settings()
    world_settings.set_editor_property("kill_z", -7000.0)
    world_settings.set_editor_property("enable_world_bounds_checks", True)

    if not level_subsystem.save_current_level():
        raise RuntimeError(f"Unable to save {MAP_PATH}")
    unreal.EditorLoadingAndSavingUtils.save_dirty_packages(True, True)
    unreal.log("UEGT1 Lumen Wilds authored: collision terrain, forest, ground cover, lake, creek, waterfall, rocks, atmosphere")
    unreal.log("UEGT1_TECH_DEMO_GENERATION_SUCCEEDED")


main()
