import unreal


MAP_PATH = "/Game/Maps/Main"


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


def make_always_loaded(actor):
    actor.set_editor_property("is_spatially_loaded", False)
    return actor


def configure_lighting(actor_subsystem):
    sun = make_always_loaded(spawn_actor(actor_subsystem, unreal.DirectionalLight, unreal.Vector(0.0, 0.0, 600.0), "Signal Grove Sun"))
    sun.set_actor_rotation(unreal.Rotator(roll=0.0, pitch=-38.0, yaw=-28.0), False)
    sun_component = sun.get_component_by_class(unreal.DirectionalLightComponent)
    sun_component.set_editor_property("intensity", 65000.0)
    sun_component.set_editor_property("light_color", unreal.Color(255, 206, 160, 255))
    sun_component.set_editor_property("atmosphere_sun_light", True)

    sky_light = make_always_loaded(spawn_actor(actor_subsystem, unreal.SkyLight, unreal.Vector(0.0, 0.0, 420.0), "Signal Grove Sky Light"))
    sky_component = sky_light.get_component_by_class(unreal.SkyLightComponent)
    sky_component.set_editor_property("intensity", 0.72)
    sky_component.set_editor_property("real_time_capture", True)

    make_always_loaded(spawn_actor(actor_subsystem, unreal.SkyAtmosphere, unreal.Vector(0.0, 0.0, 0.0), "Signal Grove Atmosphere"))
    make_always_loaded(spawn_actor(actor_subsystem, unreal.VolumetricCloud, unreal.Vector(0.0, 0.0, 0.0), "Signal Grove Cloud Layer"))

    fog = make_always_loaded(spawn_actor(actor_subsystem, unreal.ExponentialHeightFog, unreal.Vector(0.0, 0.0, -80.0), "Signal Grove Mist"))
    fog_component = fog.get_component_by_class(unreal.ExponentialHeightFogComponent)
    fog_component.set_editor_property("fog_density", 0.002)
    fog_component.set_editor_property("fog_height_falloff", 0.18)

    post_process = make_always_loaded(spawn_actor(actor_subsystem, unreal.PostProcessVolume, unreal.Vector(), "Signal Grove Exposure"))
    post_process.set_editor_property("unbound", True)
    settings = post_process.get_editor_property("settings")
    settings.set_editor_property("override_auto_exposure_min_brightness", True)
    settings.set_editor_property("override_auto_exposure_max_brightness", True)
    settings.set_editor_property("auto_exposure_min_brightness", 12.8)
    settings.set_editor_property("auto_exposure_max_brightness", 12.8)
    post_process.set_editor_property("settings", settings)


def create_gameplay_actors(actor_subsystem):
    world_director_class = load_project_class("UEGT1WorldDirector")
    biome_tile_class = load_project_class("UEGT1BiomeTile")
    town_class = load_project_class("UEGT1Town")
    sanctuary_class = load_project_class("UEGT1Sanctuary")
    waystone_class = load_project_class("UEGT1Waystone")

    make_always_loaded(spawn_actor(actor_subsystem, world_director_class, unreal.Vector(0.0, 0.0, 0.0), "Signal Grove World Director"))
    spawn_actor(actor_subsystem, town_class, unreal.Vector(0.0, 0.0, 0.0), "Signal Grove Town")
    spawn_actor(actor_subsystem, sanctuary_class, unreal.Vector(0.0, 0.0, 0.0), "Signal Grove Sanctuary")

    waystone_ids = unreal.UEGT1EditorAuthoringLibrary.get_region_waystone_ids()
    waystone_locations = unreal.UEGT1EditorAuthoringLibrary.get_region_waystone_locations()
    for waystone_id, location in zip(waystone_ids, waystone_locations):
        location.z = unreal.UEGT1EditorAuthoringLibrary.get_region_surface_height(location)
        waystone = spawn_actor(actor_subsystem, waystone_class, location, f"Waystone {waystone_id}")
        waystone.initialize_waystone(waystone_id)

    world_seed = unreal.UEGT1EditorAuthoringLibrary.get_region_world_seed()
    tile_radius = unreal.UEGT1EditorAuthoringLibrary.get_region_tile_radius()
    tile_size = unreal.UEGT1EditorAuthoringLibrary.get_region_tile_size()
    for tile_y in range(-tile_radius, tile_radius + 1):
        for tile_x in range(-tile_radius, tile_radius + 1):
            location = unreal.Vector(tile_x * tile_size, tile_y * tile_size, 0.0)
            tile = spawn_actor(actor_subsystem, biome_tile_class, location, f"Biome Tile {tile_x:+d} {tile_y:+d}")
            tile.initialize_tile(unreal.IntPoint(tile_x, tile_y), world_seed)

    return tile_radius


def main():
    level_subsystem = unreal.get_editor_subsystem(unreal.LevelEditorSubsystem)
    actor_subsystem = unreal.get_editor_subsystem(unreal.EditorActorSubsystem)

    if unreal.EditorAssetLibrary.does_asset_exist(MAP_PATH):
        if not level_subsystem.load_level(MAP_PATH):
            raise RuntimeError(f"Unable to load {MAP_PATH}")
        preserved_classes = {"WorldDataLayers", "WorldPartitionMiniMap"}
        for actor in actor_subsystem.get_all_level_actors():
            if actor.get_class().get_name() not in preserved_classes:
                if not actor_subsystem.destroy_actor(actor):
                    raise RuntimeError(f"Unable to remove existing actor {actor.get_name()}")
    elif not level_subsystem.new_level(MAP_PATH):
        raise RuntimeError(f"Unable to create {MAP_PATH}")

    tile_radius = create_gameplay_actors(actor_subsystem)
    configure_lighting(actor_subsystem)

    player_start = spawn_actor(actor_subsystem, unreal.PlayerStart, unreal.Vector(0.0, -1350.0, 110.0), "Signal Grove Player Start")
    player_start.set_actor_rotation(unreal.Rotator(roll=0.0, pitch=0.0, yaw=90.0), False)
    make_always_loaded(player_start)

    world = unreal.get_editor_subsystem(unreal.UnrealEditorSubsystem).get_editor_world()
    world_settings = world.get_world_settings()
    world_settings.set_editor_property("kill_z", -2500.0)
    world_settings.set_editor_property("enable_world_bounds_checks", True)

    if not unreal.UEGT1EditorAuthoringLibrary.convert_level_actors_to_external_packages(world):
        raise RuntimeError("Unable to enable One File Per Actor packaging")

    if not level_subsystem.save_current_level():
        raise RuntimeError(f"Unable to save {MAP_PATH}")

    unreal.EditorLoadingAndSavingUtils.save_dirty_packages(True, True)
    unreal.log(f"UEGT1 regional foundation authored at {MAP_PATH} with {(tile_radius * 2 + 1) ** 2} biome tiles")
    unreal.log("UEGT1_CONTENT_GENERATION_SUCCEEDED")


main()
