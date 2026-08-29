# UEGT1 v0.5 playtest

## Goal

At launch, choose either **Signal Grove • Story World** or **Lumen Wilds • UE5 Tech Demo**. Signal Grove retains the sanctuary objective: stabilize the three amber Waystones and return the signal to the central sanctuary. Lumen Wilds is a free-exploration environment showcase built for walking and developer-mode flight.

## Launch

For the editor build, run `./Scripts/Open-Editor.ps1`, then press Play in the Main level.

For the standalone Development build, run `./Scripts/Package.ps1`, then launch `LocalBuilds\Windows-Development\UEGT1.exe`. The exact path is printed when packaging finishes.

## Controls

| Action | Keyboard/mouse | Gamepad |
|---|---|---|
| Move | W, A, S, D | Left stick |
| Look | Mouse | Right stick |
| Jump | Space | Bottom face button |
| Sprint | Left Shift | Left stick click |
| Stabilize Waystone | E | Left face button |
| Open/close world map | M | View/Back button |
| Open/close menu | Escape | Menu/Start button |
| Diagnostics | F3 | — |
| Toggle developer mode | F8 | — |
| Toggle developer flight | F9 | — |
| Ascend while flying | Space | Bottom face button |
| Descend while flying | Left Control | Right face button |
| Navigate menu | Mouse or arrow keys + Enter | Left stick/D-pad + bottom face button |
| Exit standalone build | Menu → Quit to Desktop | Menu → Quit to Desktop |

## Graphics and display

Open **Graphics & Display** from either the launch menu or pause menu. Display controls cover window mode, resolution, frame cap, resolution scale, VSync, and dynamic resolution. Independent quality selectors cover view distance, anti-aliasing, shadows, global illumination, reflections, textures, visual effects, post processing, foliage density, and shading.

**Optional Effects** can switch every feature together or individually: anti-aliasing, shadows, global illumination, reflections, ambient occlusion, bloom, motion blur, depth of field, lens flares, fog, and foliage geometry. **Apply & Save** commits changes across launches. **Back / Discard** reloads the last saved configuration. **Restore Recommended** stages the 1080p/60 fps High profile with Epic textures and all effects enabled; press **Apply & Save** to keep it.

## What to check

1. Movement starts immediately, mouse look feels stable, sprint widens the field of view, and jumping does not snag on the trail.
2. Tan paths remain readable through the low-poly forest and each path terminates at an amber Waystone.
3. Looking at a Waystone from close range displays `[E] Stabilize Waystone`.
4. Activating a Waystone changes its shards and light from amber to teal and increments the HUD counter exactly once.
5. After the third activation, the central sanctuary brightens and accelerates, and the HUD reports completion.
6. `F3` shows a plausible frame rate and position. Look for hitches, shadow instability, obvious instance pop, gaps at tile edges, or places where foliage blocks a reserved trail.
7. Press `M`. Confirm the map covers the complete region; shows terrain regions, connected town streets, all residence dots, all numbered services/parks, Waystones and trails, Sanctuary, and an accurate live `YOU` marker. Press `M` or Escape to close it.
8. Observe sunrise, noon, sunset, and night. The sun must cross the sky instead of rotating in place; warm horizons should lead to bright noon, and midnight must be genuinely dark while interior lamps and signals remain readable.
9. Open the menu, disable a few individual effects, apply, and confirm the world changes. Relaunch to verify the saved values return; then use **Restore Recommended** and apply it.
10. Use **Quit to Desktop** and confirm the standalone build exits cleanly without Alt+F4.

## Level selection and developer mode

1. Launch a fresh process and confirm the home menu names both Signal Grove and Lumen Wilds before gameplay begins.
2. Select Signal Grove, reopen the pause menu, and switch to Lumen Wilds. Confirm map travel completes without reopening the initial selector.
3. Press `F8`. The magenta developer banner must report `INVINCIBLE` and `4.2K SPEED`; incoming damage must resolve to zero.
4. Press `F9`. The banner must also report `FLIGHT`. Look upward and move forward to confirm camera-relative ascent, use Space/Control for vertical movement, and hold Shift for the maximum flight speed.
5. Disable flight with `F9`, then disable developer mode with `F8`. Normal walking/sprint tuning and damage behavior must return.

## Lumen Wilds visual checks

1. From the approach, confirm the moss terrain is continuous, the path remains navigable, and dense textured oak layers lead toward the lake.
2. Fly to the lake. Confirm its irregular shoreline has no cylindrical side wall or grid tiling, and the creek/waterfall meets the basin cleanly.
3. Inspect bark, branch, and leaf-card detail at walking distance; distant instances should cull without breaking the forest silhouette.
4. Fly above the canopy and check terrain ridges, scattered rock formations, fallen logs, grass/ferns, volumetric atmosphere, cloud shadows, and the lake reflection as one coherent composition.
5. Disable **Foliage Geometry** in Graphics & Display. Trees and ground vegetation must hide together; restoring the setting must rebuild the full view.

## Regional exploration checks

1. **Center — town:** the sanctuary sits in an open paved plaza, surrounded by a readable cluster of small buildings and street lamps. The main roads remain unobstructed. Look east for the pier and lighthouse.
2. **East — waterfront:** meadow gives way to sand and descending shoreline before a broad teal ocean surface. The pier carries the town silhouette into the water; northeast terrain should read as higher rocky coast rather than a hard biome seam.
3. **West — town expansion and countryside:** follow the connected streets west through the long new district, then continue beyond it until buildings give way to rolling ochre/green fields. Farm and meadow colors should interleave through the transition instead of changing on one tile boundary.
4. **North — mountains:** ground elevation rises continuously into darker rocky terrain, conifers, and large faceted peaks. The town should remain visible below from the first slopes.
5. **South — tropics:** canopy becomes broader, taller, denser, and more saturated with heavier understory. Southeast should pick up coastal moisture and water colors naturally.
6. Toggle `F3` while crossing each boundary. The dominant region, ground height, water depth, temperature, and moisture should change plausibly without discontinuities.

## Town simulation checks

1. In Signal Grove, confirm the inspector reports 100 residents and the westward town contains a visibly larger connected street/building footprint.
2. Confirm residents leave homes and move continuously along the connected street grid rather than teleporting between samples or moving through buildings. At normal walk speed, the player should gradually overtake a citizen without sprinting.
3. Inspect the sleeping porches around homes and confirm the generated bed-frame/bedding pairs total one per citizen. Press `F3`, then `F4` repeatedly and verify each inspected resident shows a unique bed ID belonging to its home.
4. Verify each inspected resident also has identity, optional job, money, four needs, a schedule, a chosen action/destination, utility scores, and a replan reason.
5. Observe morning work/food movement and evening sleep/social movement. The inspector clock and sun should advance together.
6. Find a resident with low funds. Confirm it selects work, a free home meal, or the park instead of remaining stuck on an unaffordable action.
7. Press `F5`, note the inspected resident, bed, and clock, wait for state to change, then press `F6`. Confirm the seed, time, resident location/action, bed assignment, money, and needs return.

## Known v0.5 limits

- Waystone objective progress still resets when the session ends; F5/F6 persistence currently covers the town simulation only.
- No audio, characters, combat, inventory, quests, input rebinding, or accessibility UI yet. The world map is a static full-region overview without zoom, pan, filters, or custom waypoints.
- Signal Grove intentionally retains its low-poly foundation. Lumen Wilds uses detailed bundled UE tree/rock assets and a photographic terrain layer, but it is a compact procedural showcase rather than a Quixel/Megascans production biome.
- The ocean is currently a visual/lighting foundation without swimming, buoyancy, waves, or water gameplay. The pier provides a safe authored approach to it.
- The HUD is Canvas-based. The menu supports mouse, keyboard, and gamepad navigation, but gameplay prompts are not input-device adaptive.
- World Partition and HISM support the 154-tile region and 100-resident town; deeper interiors and district-specific gameplay remain future work.

Record the hardware, average frame rate, completion time, any stuck locations, and the first moment navigation became unclear. Those observations should drive the next milestone more than raw content volume.
