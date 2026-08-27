# UEGT1 v0.4 architecture

UEGT1 contains two selectable worlds behind one durable C++ runtime. Signal Grove's checked-in `Main` world supplies authored placement through World Partition and One File Per Actor. Lumen Wilds uses a separate `TechDemo` map and deterministic procedural environment actor. Game-instance subsystems preserve level selection and developer state across travel.

## Runtime ownership

```text
UGameInstance
├── UUEGT1SessionSubsystem (initial level selection)
└── UUEGT1DeveloperModeSubsystem (invincibility + flight state)

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
    ├── AUEGT1Town (ten HISM layers)
    └── AUEGT1BiomeTile × 121 (eleven HISM layers per tile)
└── TechDemo → AUEGT1TechDemoEnvironment
    ├── procedural collision terrain + lake surface
    └── HISM forest, rocks, creek, waterfall, and ground detail

UUEGT1RegionSettings (DefaultGame.ini)
└── UEGT1WorldLayout::SampleRegion(position)
    └── biome weights + surface height + water depth + climate
```

- `UUEGT1DeveloperModeSubsystem` owns session-persistent developer state. The explorer consumes it for zero-damage invincibility, 2,100 cm/s walking, 4,200 cm/s sprinting, and 3,200–4,200 cm/s flying. `F8` toggles developer mode, `F9` toggles flight, and command-line automation can use `-UEGT1DevMode -UEGT1DevFlight`.
- `UUEGT1SessionSubsystem` records the first level selection so map travel does not reopen the launch selector.
- `AUEGT1ExplorerCharacter` owns first-person movement, camera feel, sprinting, jumping, flight ascent/descent, damage gating, and input dispatch.
- `AUEGT1PlayerController` owns launch/pause menu state, whitelisted travel to `Main` or `TechDemo`, input mode, and the quit request. `SUEGT1Menu` owns presentation while `UUEGT1GameUserSettings` owns persisted graphics state and runtime console-variable application.
- Input uses UE's supported legacy action/axis mapping bridge on the Enhanced Input player/component classes. This keeps v0.1 bindings data-light while leaving the plugin explicit for a later Input Action migration.
- `UUEGT1InteractionComponent` performs a throttled visibility trace and talks only through `IUEGT1Interactable`. New usable objects should implement that interface instead of changing the player.
- `AUEGT1MilestoneGameState` is the authority for registered and activated Waystone IDs. The HUD and sanctuary read or subscribe to this state; Waystones do not reach into either consumer.
- `UUEGT1RegionSettings` exposes the seed, grid extent, transition distances, shoreline, sea level, and mountain height as project configuration. It is the correct place to tune the broad region without rewriting generators.
- `UEGT1WorldLayout::SampleRegion` is a pure coordinate sampler. It returns normalized town/meadow/farmland/highland/tropical/coast/ocean weights plus continuous elevation, water depth, temperature, and moisture. No presentation or Actor state is involved.
- `AUEGT1WorldDirector` validates world composition at startup, reports tile/instance/biome coverage, and recreates missing regional, town, or gameplay actors deterministically.
- `AUEGT1Town` owns the central plaza, route-safe building grammar, street furniture, pier, and lighthouse. Buildings and details are HISM instances, not individual Actors.
- `AUEGT1BiomeTile` turns sampler output into faceted sloped terrain, paths, conifers, broadleaf canopy, rocks, grass, crop rows, water/wave planes, and mountain silhouettes. It listens for graphics changes so the foliage switch immediately hides or restores vegetation and crop layers.
- `AUEGT1TechDemoEnvironment` is Lumen Wilds' single deterministic owner. It generates a 129×129 collision heightfield across 36,000 cm, an organic procedural lake surface, a creek/waterfall route, and more than 16,000 instanced details. The forest uses the UE 5.8 ArchViz oak asset with its bark, leaf, and normal-map set; repeated rocks use the UE template rock mesh. The foliage graphics switch hides all vegetation layers together.
- `AUEGT1HUD` is deliberately code-driven for this foundation. A later CommonUI/UMG layer can replace presentation without changing gameplay state.

## World and visual contract

The world is an eleven-by-eleven grid of 3,200 cm tiles: a roughly 352 m square foundation. Unreal X is east/west and Y is north/south. The small town occupies the center; coast and ocean grow toward +X, farms toward -X, mountains toward +Y, and tropical vegetation toward -Y. Transition weights overlap across several tiles, including mixed northeast cliff coast and southeast tropical coast, so directions are regions rather than separate test maps.

Three reserved trail corridors still connect the central sanctuary to distinct Waystones. The deterministic seed is `7319`. Authored and fallback layouts, terrain height, route placement, runtime diagnostics, and automation all call the same sampler.

The visual language deliberately treats low-poly geometry like physical miniatures in believable light: chunky readable silhouettes, faceted slopes, warm plaster and teal roofs, deep climatic greens, ochre crops, pale coastal sand, teal water, warm amber signals, and photographic-scale sun/sky/fog/cloud lighting. The shared palette keeps the mixed treatment coherent. Three authored master materials separate matte surfaces, reflective water, and emissive signals; `UEGT1VisualMaterials` is the single runtime assignment path for their color and physical-response parameters.

The unbound visual stack uses fixed exposure for deterministic presentation, local exposure to retain readable highlights and shadows, restrained bloom/lens response, Lumen ambient occlusion, real-time skylight capture, cloud shadows, light-shaft response, and low-density volumetric fog. These effects are authored in `CreateInitialContent.py` and remain subordinate to the player-facing feature switches in `UUEGT1GameUserSettings`.

The map is World Partition with 134 external actor packages and existing HLOD layer assets. This is intentionally more infrastructure than the current footprint needs: future districts, tiles, authored set dressing, and landmarks can become spatially streamed content without replacing the map format or creating a source-control bottleneck.

Lumen Wilds is intentionally independent of Signal Grove's biome sampler and World Partition content. Its compact showcase-valley composition layers a navigable approach, dense midground canopy, ridge silhouettes, reflective lake, waterfall focal point, and atmospheric depth. `CreateTechDemoContent.py` owns its four materials and lighting actors. `M_TechTerrain` combines terrain vertex masks, procedural variation, and UE's bundled photographic moss surface; the oak assets retain their authored branch/leaf materials. See `Docs/Lumen-Wilds.md` before changing this map.

## Performance contract

- Target: 1920×1080, 60 fps on an RTX 3060-class GPU.
- Default quality: level 2 for view distance, shadows, Lumen GI/reflections, post, effects, foliage, and shading; level 3 textures.
- The recommended persistent profile is borderless 1920×1080 at 60 fps, VSync on, 100% resolution scale, and all optional effects enabled. Explicit feature switches override the related scalability result without changing unrelated groups.
- Signal Grove contains about 10,000 instanced primitives across 121 regional tiles plus roughly 100 town instances. Lumen Wilds reports 16,000+ deterministic instances and 16,641 collision-terrain vertices while sharing HISM components instead of creating one Actor per prop.
- HISM culling is configured per prop layer; only ground and major rocks/trunks participate in collision.
- Interaction traces run at 20 Hz rather than every render frame.
- Dynamic lights do not cast shadows. Repeated ambient motion changes component transforms, not skeletal animation graphs.

Measure before increasing instance density, light radius/count, shadowed movable lights, or Lumen quality. Keep per-frame gameplay scans out of the world director and HUD.

## Extension rules

1. Put cross-system state in a dedicated state object or subsystem; do not make presentation the authority.
2. Add interactables through `IUEGT1Interactable` and keep the trace component generic.
3. Put broad numeric tuning in `UUEGT1RegionSettings`; add new deterministic sampling behavior to `UEGT1WorldLayout`. Never fork editor and runtime generation constants.
4. Prefer HISM/ISM, PCG, or pooled actors for repeated environment content. Use individual Actors for gameplay identity.
5. Preserve the `LogUEGT1` signals used by smoke automation, or update the assertions in the same change.
6. Run `Build.ps1`, `Test.ps1`, and `Smoke-Gameplay.ps1` after C++/content changes. Run the packaged rendered smoke before a playtest handoff.
7. Keep material selection and parameter wiring in `UEGT1VisualMaterials`; add an authored master under `/Game/Materials` only when a genuinely different shading model is needed.

## Diagnostics and automation

- `F3` toggles frame rate, position, dominant biome, sampled ground/water height, temperature, moisture, and focused interactable in-game.
- `uegt1.Debug.DrawInteraction 1` draws interaction traces.
- `Scripts/Smoke-Gameplay.ps1` verifies the authored World Partition load and expected content headlessly.
- `Scripts/Smoke-PackagedBuild.ps1` verifies Signal Grove gameplay, graphics/menu/quit flows, the initial two-level selector, actual menu travel to Lumen Wilds, developer invincibility/fast-flight state, both cooked maps, and eight deterministic views across the two worlds. The `-UEGT1Smoke*`, `-UEGT1RegionCaptureFolder`, and `-UEGT1TechDemoCaptureFolder` arguments exist only for automation.

See `Docs/Region-Authoring.md` before changing Signal Grove's direction, scale, biome composition, terrain sampling, or authoring automation. See `Docs/Lumen-Wilds.md` for the showcase world.
