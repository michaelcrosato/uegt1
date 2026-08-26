import unreal


MAP_PATH = "/Game/Maps/Main"


def spawn_actor(actor_subsystem, actor_class, location, label):
    actor = actor_subsystem.spawn_actor_from_class(actor_class, location)
    if actor is None:
        raise RuntimeError(f"Unable to spawn {label}")
    actor.set_actor_label(label)
    return actor


def main():
    level_subsystem = unreal.get_editor_subsystem(unreal.LevelEditorSubsystem)
    actor_subsystem = unreal.get_editor_subsystem(unreal.EditorActorSubsystem)

    if unreal.EditorAssetLibrary.does_asset_exist(MAP_PATH):
        unreal.EditorAssetLibrary.delete_asset(MAP_PATH)

    if not level_subsystem.new_level(MAP_PATH):
        raise RuntimeError(f"Unable to create {MAP_PATH}")

    cube_mesh = unreal.load_asset("/Engine/BasicShapes/Cube.Cube")
    if cube_mesh is None:
        raise RuntimeError("Unable to load the engine cube mesh")

    floor = spawn_actor(actor_subsystem, unreal.StaticMeshActor, unreal.Vector(0.0, 0.0, -50.0), "Graybox Floor")
    floor.static_mesh_component.set_static_mesh(cube_mesh)
    floor.set_actor_scale3d(unreal.Vector(20.0, 20.0, 0.5))

    player_start = spawn_actor(actor_subsystem, unreal.PlayerStart, unreal.Vector(0.0, 0.0, 150.0), "Player Start")
    player_start.set_actor_rotation(unreal.Rotator(0.0, 0.0, 0.0), False)

    directional_light = spawn_actor(actor_subsystem, unreal.DirectionalLight, unreal.Vector(0.0, 0.0, 500.0), "Sun")
    directional_light.set_actor_rotation(unreal.Rotator(-45.0, -35.0, 0.0), False)
    directional_light.directional_light_component.set_editor_property("intensity", 8.0)

    sky_light = spawn_actor(actor_subsystem, unreal.SkyLight, unreal.Vector(0.0, 0.0, 300.0), "Sky Light")
    sky_light.light_component.set_editor_property("intensity", 1.0)

    fill_light = spawn_actor(actor_subsystem, unreal.PointLight, unreal.Vector(0.0, 0.0, 450.0), "Fill Light")
    fill_light.point_light_component.set_editor_property("intensity", 5000.0)
    fill_light.point_light_component.set_editor_property("attenuation_radius", 1800.0)

    spawn_actor(actor_subsystem, unreal.ExponentialHeightFog, unreal.Vector(0.0, 0.0, 0.0), "Atmosphere Fog")

    if not level_subsystem.save_current_level():
        raise RuntimeError(f"Unable to save {MAP_PATH}")

    unreal.log(f"UEGT1 initial content created at {MAP_PATH}")


main()

