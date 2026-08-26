# Signal Grove v0.1 architecture

Signal Grove is a small vertical slice built to remain easy to extend. Durable behavior lives in C++; the checked-in `Main` world supplies authored placement through World Partition and One File Per Actor. The runtime director can reconstruct critical content if a map is damaged or temporarily empty.

## Runtime ownership

```text
AUEGT1GameMode
├── AUEGT1ExplorerCharacter
│   └── UUEGT1InteractionComponent ── IUEGT1Interactable
├── AUEGT1MilestoneGameState
│   ├── AUEGT1Waystone × 3
│   └── AUEGT1Sanctuary
├── AUEGT1HUD
└── AUEGT1WorldDirector
    └── AUEGT1BiomeTile × 25 (six HISM layers per tile)
```

- `AUEGT1ExplorerCharacter` owns first-person movement, camera feel, sprinting, jumping, and input dispatch.
- Input uses UE's supported legacy action/axis mapping bridge on the Enhanced Input player/component classes. This keeps v0.1 bindings data-light while leaving the plugin explicit for a later Input Action migration.
- `UUEGT1InteractionComponent` performs a throttled visibility trace and talks only through `IUEGT1Interactable`. New usable objects should implement that interface instead of changing the player.
- `AUEGT1MilestoneGameState` is the authority for registered and activated Waystone IDs. The HUD and sanctuary read or subscribe to this state; Waystones do not reach into either consumer.
- `AUEGT1WorldDirector` validates world composition at startup. Missing biome/gameplay actors are recreated deterministically from `UEGT1WorldLayout`.
- `AUEGT1BiomeTile` generates ground, trails, trees, rocks, grass, and boundary shapes into hierarchical instanced mesh components. It does not create an Actor per prop.
- `AUEGT1HUD` is deliberately code-driven for this foundation. A later CommonUI/UMG layer can replace presentation without changing gameplay state.

## World and visual contract

The current world is a five-by-five grid of 3,200 cm tiles (a roughly 160 m square playable grove). Three reserved trail corridors connect the central sanctuary to distinct Waystones. The deterministic seed is `7319`; authored and fallback layouts use the same constants.

The visual language uses chunky primitive silhouettes, deep forest/moss/fern greens, warm amber inactive signals, bright teal restored signals, pale stone, and warm directional light. Motion is systemic—floating shards, rotating sanctuary slabs, light pulses, head bob, and FOV interpolation—so the slice has life without character-animation dependencies.

The map is World Partition with 36 external actor packages. This is intentionally more infrastructure than the current footprint needs: future tiles, encounters, and landmarks can become spatially streamed content without replacing the map format or creating a source-control bottleneck.

## Performance contract

- Target: 1920×1080, 60 fps on an RTX 3060-class GPU.
- Default quality: level 2 for view distance, shadows, Lumen GI/reflections, post, effects, foliage, and shading; level 3 textures.
- Current authored biome: 1,350 instanced primitives across 25 tile actors.
- HISM culling is configured per prop layer; only ground and major rocks/trunks participate in collision.
- Interaction traces run at 20 Hz rather than every render frame.
- Dynamic lights do not cast shadows. Repeated ambient motion changes component transforms, not skeletal animation graphs.

Measure before increasing instance density, light radius/count, shadowed movable lights, or Lumen quality. Keep per-frame gameplay scans out of the world director and HUD.

## Extension rules

1. Put cross-system state in a dedicated state object or subsystem; do not make presentation the authority.
2. Add interactables through `IUEGT1Interactable` and keep the trace component generic.
3. Add deterministic layout data to `UEGT1WorldLayout` or a future data asset. Keep the content generator and runtime fallback aligned.
4. Prefer HISM/ISM, PCG, or pooled actors for repeated environment content. Use individual Actors for gameplay identity.
5. Preserve the `LogUEGT1` signals used by smoke automation, or update the assertions in the same change.
6. Run `Build.ps1`, `Test.ps1`, and `Smoke-Gameplay.ps1` after C++/content changes. Run the packaged rendered smoke before a playtest handoff.

## Diagnostics and automation

- `F3` toggles frame rate, world position, and focused interactable in-game.
- `uegt1.Debug.DrawInteraction 1` draws interaction traces.
- `Scripts/Smoke-Gameplay.ps1` verifies the authored World Partition load and expected content headlessly.
- `Scripts/Smoke-PackagedBuild.ps1` completes the objective in a Development package, captures a 1080p frame, and exits. `-UEGT1SmokeComplete` and `-UEGT1SmokeCapture=<path>` exist only to make that deterministic verification possible.
