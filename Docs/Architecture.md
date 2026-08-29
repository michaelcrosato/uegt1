# UEGT1 v0.5 architecture

UEGT1 contains two selectable worlds behind one durable C++ runtime. Signal Grove's checked-in `Main` world supplies authored placement through World Partition and One File Per Actor. Lumen Wilds uses a separate `TechDemo` map and deterministic procedural environment actor. Game-instance subsystems preserve level selection and developer state across travel.

## Runtime ownership

```text
UGameInstance
├── UUEGT1SessionSubsystem (initial level selection)
└── UUEGT1DeveloperModeSubsystem (invincibility + flight state)

UWorld
└── UUEGT1TownSimulationSubsystem (batched clock, persistence, visuals)
    ├── FUEGT1TownSimulationModel (pure resident, venue, decision, and economy state)
    └── UEGT1DayNight (pure clock-to-environment curve)

AUEGT1GameMode
├── AUEGT1ExplorerCharacter
│   └── UUEGT1InteractionComponent ── IUEGT1Interactable
├── AUEGT1PlayerController
│   └── SUEGT1Menu ── UUEGT1GameUserSettings
├── AUEGT1MilestoneGameState
│   ├── AUEGT1Waystone × 3
│   └── AUEGT1Sanctuary
├── AUEGT1HUD
├── Main → AUEGT1WorldDirector
	    ├── AUEGT1Town (fourteen HISM layers + registered destination components)
	    │   └── AUEGT1TownResident × 100 (presentation-only actors + nearby thought bubbles)
    └── AUEGT1BiomeTile × 154 (eleven HISM layers per tile)
└── TechDemo → AUEGT1TechDemoEnvironment
    ├── procedural collision terrain + lake surface
    └── HISM forest, rocks, creek, waterfall, and ground detail

UUEGT1RegionSettings (DefaultGame.ini)
└── UEGT1WorldLayout::SampleRegion(position)
    └── biome weights + surface height + water depth + climate
```

- `UUEGT1DeveloperModeSubsystem` owns session-persistent developer state. The explorer consumes it for zero-damage invincibility, 2,100 cm/s walking, 4,200 cm/s sprinting, and 3,200–4,200 cm/s flying. `F8` toggles developer mode, `F9` toggles flight, and command-line automation can use `-UEGT1DevMode -UEGT1DevFlight`.
- `UUEGT1SessionSubsystem` records the first level selection so map travel does not reopen the launch selector.
- `FUEGT1TownSimulationModel` is the deterministic simulation authority. It owns the clock snapshot, venue reservations, named job sites and hourly wages, resident identity/home/job/location/action/money/needs, explicit household relationships, utility scores, thoughts, transactions, failures, and aggregate metrics without depending on a `UWorld`.
- `UUEGT1TownSimulationSubsystem` advances the model in batches, synchronizes lightweight resident presentation, applies the pure 24-hour `UEGT1DayNight` curve to the atmosphere sun, directional-light lux/color/shadows, real-time skylight, and fog, and serializes the full snapshot through Unreal SaveGame. It does not run per-NPC decision ticks.
- `AUEGT1ExplorerCharacter` owns first-person movement, camera feel, sprinting, jumping, flight ascent/descent, damage gating, and input dispatch.
- `AUEGT1PlayerController` owns launch/pause menu state, world-map open/close input, whitelisted travel to `Main` or `TechDemo`, input mode, and the quit request. `SUEGT1Menu` owns presentation while `UUEGT1GameUserSettings` owns persisted graphics state and runtime console-variable application.
- Input uses UE's supported legacy action/axis mapping bridge on the Enhanced Input player/component classes. This keeps v0.1 bindings data-light while leaving the plugin explicit for a later Input Action migration.
- `UUEGT1InteractionComponent` performs a throttled visibility trace and talks only through `IUEGT1Interactable`. New usable objects should implement that interface instead of changing the player.
- `AUEGT1MilestoneGameState` is the authority for registered and activated Waystone IDs. The HUD and sanctuary read or subscribe to this state; Waystones do not reach into either consumer.
- `UUEGT1RegionSettings` exposes the seed, base grid radius, west-only tile extension, town spine, transition distances, shoreline, sea level, and mountain height as project configuration. It is the correct place to tune the broad region without rewriting generators.
- `UEGT1WorldLayout::SampleRegion` is a pure coordinate sampler. It returns normalized town/meadow/farmland/highland/tropical/coast/ocean weights plus continuous elevation, water depth, temperature, and moisture. No presentation or Actor state is involved.
- `AUEGT1WorldDirector` validates world composition at startup, reports tile/instance/biome coverage, and recreates missing regional, town, or gameplay actors deterministically.
- `AUEGT1Town` consumes `UEGT1TownGeneration` for a seeded, connected westward street/intersection/block/lot layout, then owns its sidewalks, grass, buildings, 100 instanced bed frames/bedding sets, street furniture, pier, and lighthouse. Buildings and details are HISM instances, not individual Actors. Queryable lots, twenty named job sites, and explicit bed slots are generated from the same layout.
- `AUEGT1BiomeTile` turns sampler output into faceted sloped terrain, paths, conifers, broadleaf canopy, rocks, grass, crop rows, water/wave planes, and mountain silhouettes. It listens for graphics changes so the foliage switch immediately hides or restores vegetation and crop layers.
- `AUEGT1TechDemoEnvironment` is Lumen Wilds' single deterministic owner. It generates a 129×129 collision heightfield across 36,000 cm, an organic procedural lake surface, a creek/waterfall route, and more than 16,000 instanced details. The forest uses the UE 5.8 ArchViz oak asset with its bark, leaf, and normal-map set; repeated rocks use the UE template rock mesh. The foliage graphics switch hides all vegetation layers together.
- `AUEGT1HUD` is deliberately code-driven for this foundation. Its full-island map rasterizes `UEGT1WorldLayout::SampleRegion`, draws streets from `UEGT1TownGeneration`, and reads live venues/player/objectives; it owns no duplicate world state. A later CommonUI/UMG layer can replace presentation without changing gameplay state.

## World and visual contract

The world is a fourteen-by-eleven grid of 3,200 cm tiles: roughly 448 m east/west by 352 m north/south. Three additional columns extend only toward -X, leaving the east, north, and south boundaries unchanged. Unreal X is east/west and Y is north/south. A large town follows a seeded spine from the sanctuary westward; coast and ocean grow toward +X, farms continue beyond the town toward -X, mountains toward +Y, and tropical vegetation toward -Y. Transition weights overlap across several tiles, including mixed northeast cliff coast and southeast tropical coast, so directions are regions rather than separate test maps.

Three reserved trail corridors still connect the central sanctuary to distinct Waystones. The deterministic seed is `7319`. Authored and fallback layouts, terrain height, route placement, runtime diagnostics, and automation all call the same sampler.

The visual language deliberately treats low-poly geometry like physical miniatures in believable light: chunky readable silhouettes, faceted slopes, warm plaster and teal roofs, deep climatic greens, ochre crops, pale coastal sand, teal water, warm amber signals, and photographic-scale sun/sky/fog/cloud lighting. The shared palette keeps the mixed treatment coherent. Three authored master materials separate matte surfaces, reflective water, and emissive signals; `UEGT1VisualMaterials` is the single runtime assignment path for their color and physical-response parameters.

The unbound visual stack uses a deterministic exposure target, local exposure to retain readable highlights and shadows, restrained bloom/lens response, Lumen ambient occlusion, real-time skylight capture, cloud shadows, light-shaft response, and low-density volumetric fog. These effects are authored in `CreateInitialContent.py` and remain subordinate to the player-facing feature switches in `UUEGT1GameUserSettings`. At runtime, simulation time drives a ±68° solar elevation and full azimuth traversal; the coordinated curve produces low-lux warm horizons, 65,000-lux noon, zero direct night sun, a dim 0.12 cool night skylight, matching fog tint, and deterministic eye adaptation from EV100 12.8 by day to 5.5 at midnight.

The map is World Partition with 167 external actor packages and existing HLOD layer assets. Future districts, authored set dressing, and landmarks can become spatially streamed content without replacing the map format or creating a source-control bottleneck.

Lumen Wilds is intentionally independent of Signal Grove's biome sampler and World Partition content. Its compact showcase-valley composition layers a navigable approach, dense midground canopy, ridge silhouettes, reflective lake, waterfall focal point, and atmospheric depth. `CreateTechDemoContent.py` owns its four materials and lighting actors. `M_TechTerrain` combines terrain vertex masks, procedural variation, and UE's bundled photographic moss surface; the oak assets retain their authored branch/leaf materials. See `Docs/Lumen-Wilds.md` before changing this map.

## Performance contract

- Target: 1920×1080, 60 fps on an RTX 3060-class GPU.
- Default quality: level 2 for view distance, shadows, Lumen GI/reflections, post, effects, foliage, and shading; level 3 textures.
- The recommended persistent profile is borderless 1920×1080 at 60 fps, VSync on, 100% resolution scale, and all optional effects enabled. Explicit feature switches override the related scalability result without changing unrelated groups.
- Signal Grove contains roughly 12,500 instanced primitives across 154 regional tiles, plus the expanded town, HISM-backed furnished building shells, 102 query-only activity stations, and 100 lightweight resident visuals. Lumen Wilds reports 16,000+ deterministic instances and 16,641 collision-terrain vertices while sharing HISM components instead of creating one Actor per prop.
- Resident decisions and authoritative movement advance in one world subsystem at a configurable four-Hz real-time batch cadence. Default citizen translation is 440 cm/s versus the player's 480 cm/s walk. Presentation interpolates translation and facing every frame between samples; the pure model can fast-forward off-screen or in automation without spawning Actors. HUD-projected thought bubbles anchor to interpolated positions and are limited to the nearest twelve residents within 30 metres.
- HISM culling is configured per prop layer; only ground and major rocks/trunks participate in collision.
- Interaction traces run at 20 Hz rather than every render frame.
- Dynamic lights do not cast shadows. Each building has one attenuation-limited interior light, so only nearby rooms affect a view. Repeated ambient motion changes component transforms, not skeletal animation graphs.

Measure before increasing instance density, light radius/count, shadowed movable lights, or Lumen quality. Keep per-frame gameplay scans out of the world director and HUD.

## Extension rules

1. Put cross-system state in a dedicated state object or subsystem; do not make presentation the authority.
2. Add interactables through `IUEGT1Interactable` and keep the trace component generic.
3. Put broad numeric tuning in `UUEGT1RegionSettings`; add new deterministic sampling behavior to `UEGT1WorldLayout`. Never fork editor and runtime generation constants.
4. Prefer HISM/ISM, PCG, or pooled actors for repeated environment content. Use individual Actors for gameplay identity.
5. Preserve the `LogUEGT1` signals used by smoke automation, or update the assertions in the same change.
6. Run `Build.ps1`, `Test.ps1`, and `Smoke-Gameplay.ps1` after C++/content changes. Run the packaged rendered smoke before a playtest handoff.
7. Keep material selection and parameter wiring in `UEGT1VisualMaterials`; add an authored master under `/Game/Materials` only when a genuinely different shading model is needed.
8. Add town behavior through config action definitions and model rules. Keep resident Actors presentation-only, and generate query destinations from the same seeded layout as the visuals.

## Diagnostics and automation

- `F3` toggles frame rate, position, dominant biome, sampled ground/water height, temperature, moisture, and focused interactable in-game.
- In Signal Grove, `F3` also opens the town inspector; `F4` cycles residents, while `F5`/`F6` save and restore the complete town simulation.
- `uegt1.Debug.DrawInteraction 1` draws interaction traces.
- `Scripts/Smoke-Gameplay.ps1` verifies the authored World Partition load and expected content headlessly.
- `Scripts/Smoke-PackagedBuild.ps1` verifies Signal Grove gameplay, graphics/menu/quit flows, the initial two-level selector, actual menu travel to Lumen Wilds, developer invincibility/fast-flight state, both cooked maps, and fourteen deterministic views across the two worlds. The additional environment sequence renders dawn, noon, golden hour, midnight, and the full-island map, and requires all four lighting-phase log contracts. The `-UEGT1Smoke*`, `-UEGT1RegionCaptureFolder`, `-UEGT1EnvironmentCaptureFolder`, and `-UEGT1TechDemoCaptureFolder` arguments exist only for automation.
- `-UEGT1SmokeFocusResident` positions a rendered smoke capture at a modeled citizen so overhead thought presentation can be visually verified.
- `-UEGT1SmokeFocusInterior` positions a rendered smoke capture inside a home, advances the automation-only view to midday, and verifies the HUD/activity prompt/furnished room composition.

See `Docs/Region-Authoring.md` before changing Signal Grove's direction, scale, biome composition, terrain sampling, or authoring automation. See `Docs/Lumen-Wilds.md` for the showcase world.
See `Docs/Town-Simulation.md` before changing town generation, resident state, decision scoring, economy tuning, persistence, or simulation performance.
