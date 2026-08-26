# Signal Grove regional authoring contract

This guide is the shortest reliable route for a future Codex session to understand, tune, regenerate, and verify the starting region.

## Coordinate and scale contract

| Direction | Unreal coordinate | Region | Primary visual grammar |
|---|---:|---|---|
| Center | near `(0, 0)` | Town | paved sanctuary plaza, small plaster buildings, teal roofs, lamps |
| East | `+X` | Coast → ocean | sand, descending shore, teal water, pier, lighthouse |
| West | `-X` | Farms | rolling fields, ochre/green crop rows, sparse trees |
| North | `+Y` | Highlands | rising faceted ground, rock, conifers, large peaks |
| South | `-Y` | Tropics | broadleaf canopy, tall trees, dense saturated understory |

The configured grid radius is 5 with 3,200 cm tiles, producing 121 tiles and a 35,200 cm square region. The seed is `7319`. Keep coordinates in world centimetres.

## Single source of truth

`UUEGT1RegionSettings` reads `/Script/UEGT1.UEGT1RegionSettings` from `Config/DefaultGame.ini`. It owns broad scale and transition parameters. `UEGT1WorldLayout::SampleRegion` consumes those values and returns:

- seven normalized biome weights;
- surface height and water depth;
- normalized temperature and moisture;
- the dominant biome for diagnostics and coverage.

The sampler is deterministic and side-effect free. Editor authoring calls exported C++ accessors rather than duplicating settings in Python. Runtime fallback, HUD diagnostics, automation tests, biome tiles, and regional screenshots use the same sampler. Preserve that property.

## Generation layers

`AUEGT1BiomeTile` is the repeatable regional unit. Each tile has separate HISM layers for terrain, routes, trunks, conifers, broadleaf canopy, rocks, grass, crops, water, wave accents, and mountain peaks. Terrain is a six-by-six set of rotated, overlapping box cells whose normals are derived from neighboring height samples. This produces walkable faceted slopes with a deliberate low-poly character.

`AUEGT1Town` is an independent center module. Its generated lots are rejected when they overlap a primary Waystone route. Town roads, plaza, walls, roofs, trim, pier, street furniture, and lighthouse use ten HISM layers. Add districts or landmark actors as separate modules rather than adding special cases to biome tiles.

`AUEGT1WorldDirector` owns validation and fallback only. It must not become a per-frame generator or a container for biome rules.

## Tuning and extension workflow

1. Change broad distances, scale, sea level, or mountain height in `Config/DefaultGame.ini`.
2. Change sampling equations only in `UEGT1WorldLayout.cpp`; add a test for every new directional invariant.
3. Change visual realization in `AUEGT1BiomeTile` or a dedicated landmark/district actor. Keep repeated geometry instanced.
4. Build the editor target.
5. Run `Scripts/Create-InitialContent.ps1 -Force` to recreate the checked-in World Partition actors. A successful clean run reports 121 tiles and no authoring warnings.
6. Run `Scripts/Test.ps1` and `Scripts/Smoke-Gameplay.ps1`.
7. Package and run `Scripts/Smoke-PackagedBuild.ps1`; inspect all five files in `Saved/Screenshots/Region` at original resolution.

To expand the footprint, raise `TileRadius` and regenerate. To increase detail without expanding scale, add biome-specific HISM layers or authored World Partition actors. Do not increase Actor count per prop.

## Required invariants

- Center, east, west, north, and south samples have the intended dominant region at established test coordinates.
- The coastline and other boundaries retain overlapping non-zero weights; no one-tile biome switches.
- North elevation exceeds the town by at least 1,200 cm at the tested outer sample.
- Ocean floor lies below sea level and reports positive water depth.
- All configured tile coordinates are authored and loaded; runtime fallback is an error in smoke tests.
- Primary routes and Waystone clearings remain free of procedural clutter and town lots.
- Foliage settings hide trunks, conifers, broadleaf canopy, grass, and crops together.
- Region generation remains deterministic for a fixed seed and settings profile.

## Advanced UE5 foundation already in use

- World Partition and One File Per Actor for spatial ownership and source-control isolation;
- HLOD layer assets for later regional proxy generation;
- HISM for repeated terrain, environment, town, and set-dressing geometry;
- Lumen GI/reflections, Virtual Shadow Maps, Nanite project support, Sky Atmosphere, volumetric clouds, height fog, and fixed photographic exposure;
- deterministic editor Python authoring plus runtime reconstruction;
- rendered packaged automation with fixed regional viewpoints.

The current geometry is intentionally asset-light. Production meshes, Megascans surfaces, PCG graphs, Water-system bodies, foliage assets, and richer materials can replace individual realization layers later without changing the sampler or regional ownership model.
