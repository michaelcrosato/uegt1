# Lumen Wilds authoring and verification

Lumen Wilds is the separate `/Game/Maps/TechDemo` environment showcase selected from the launch menu. It is deliberately independent from Signal Grove's World Partition sampler: one deterministic C++ actor owns the terrain and repeated environment layers, while a Python authoring script owns the checked-in map, materials, and lighting stack.

## Composition and runtime contract

`AUEGT1TechDemoEnvironment` generates a 36,000 cm square valley from a fixed random seed. Its 129×129 `UProceduralMeshComponent` heightfield supplies complex collision and blends macro elevation, mid/detail Perlin layers, uplifted perimeter ridges, a depressed lake basin, and a flattened navigable approach. The same `SampleTerrainHeight` function controls mesh vertices, prop placement, player start, automation views, and tests.

The showcase is organized around five readable layers:

1. a walkable valley approach and creek line;
2. dense midground oak canopy with reserved camera and trail clearings;
3. irregular ridge rocks and forest silhouettes;
4. an organic procedural lake with a framed waterfall basin;
5. Sky Atmosphere, volumetric clouds/fog, real-time skylight capture, Lumen, and Virtual Shadow Maps for depth.

Repeated trees, rocks, grass, ferns, flowers, fallen logs, stream sections, and foam use HISM components. The current deterministic build reports 16,641 terrain vertices, more than 16,000 instances, and roughly 1,100 detailed trees. Foliage collision is intentionally limited; terrain, larger rocks, logs, and authored path-safe areas carry the useful traversal collision.

## Asset and material contract

The detailed oak mesh and its bark, leaf, frond, diffuse, specular, and normal-map assets come from the installed UE 5.8 ArchViz template resources and are checked in below `/Game/ArchVis/SampleScene/Tree`. The irregular rock mesh and its small dependency set come from the UE 5.8 SimSandbox template below `/Game/SimSandbox`. Keep those mount-relative paths intact: the source assets contain hard package references under those roots.

`Scripts/CreateTechDemoContent.py` authors four project materials:

- `M_TechTerrain` blends generated vertex masks with UE's bundled `T_ground_Moss_D` photographic surface and world-position variation;
- `M_TechSurface` supplies rough procedural response for path, wood, and rock layers;
- `M_TechFoliage` is two-sided foliage shading for procedural grass, ferns, and flowers;
- `M_TechWater` supplies the lake, creek, and waterfall response with restrained Fresnel and specular values.

The oak mesh keeps its authored multi-slot materials. Do not overwrite it with `M_TechFoliage`; doing so would discard the cutout atlas, bark normals, and branch material separation.

## Regeneration workflow

1. Change terrain, placement, or shoreline rules in `AUEGT1TechDemoEnvironment` and add/adjust an automation invariant where appropriate.
2. Change material graphs, sun/sky/fog, post processing, or map-level actors in `Scripts/CreateTechDemoContent.py`.
3. Run `Scripts/Build.ps1`.
4. Run `Scripts/Create-TechDemoContent.ps1 -Force` to recreate the map and four tech materials.
5. Run `Scripts/Test.ps1 -SkipBuild` and `Scripts/Package.ps1`.
6. Run `Scripts/Smoke-PackagedBuild.ps1`; inspect the three original-resolution files in `Saved/Screenshots/LumenWilds`.

The three deterministic views are Valley Approach, Lake Overlook, and Canopy Flight. Accept a visual change only when all three retain natural exposure, terrain texture, readable tree materials, a clean shoreline, the waterfall focal point, and no missing/default materials. Search the runtime log for missing packages whenever vegetation or rocks render white.

## Invariants

- `/Game/Maps/TechDemo` is explicitly cooked and menu travel is whitelisted to `Main` or `TechDemo` only.
- Terrain and placement remain deterministic for seed `584021`.
- The tested outer ridge remains substantially above the valley and the recommended player start remains safely above collision.
- Generated instance count remains above 10,000.
- Developer mode survives level travel and supplies zero-damage invincibility, fast traversal, and camera-relative flight.
- The foliage graphics switch controls the detailed tree layer and all procedural vegetation together.
- Bundled UE asset paths retain their `/Game/ArchVis` and `/Game/SimSandbox` package roots.
- The lake remains a top surface only; do not reintroduce a cylinder wall or visibly tiled grid.
