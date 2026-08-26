# Signal Grove v0.2 playtest

## Goal

Walk the three branching paths, stabilize each amber Waystone, and return the signal to the central sanctuary. A successful run ends when the sanctuary core turns teal and the completion message appears. Expected first-run time is about 5–10 minutes.

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
| Open/close menu | Escape | Menu/Start button |
| Diagnostics | F3 | — |
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
7. Open the menu, disable a few individual effects, apply, and confirm the world changes. Relaunch to verify the saved values return; then use **Restore Recommended** and apply it.
8. Use **Quit to Desktop** and confirm the standalone build exits cleanly without Alt+F4.

## Known v0.2 limits

- No save/load: objective progress resets when the session ends.
- No audio, characters, combat, inventory, quests, input rebinding, map, or accessibility UI yet.
- Environment geometry intentionally uses engine primitives and procedural instances; landmark art and terrain sculpting are foundation-quality.
- The HUD is Canvas-based. The menu supports mouse, keyboard, and gamepad navigation, but gameplay prompts are not input-device adaptive.
- World Partition and HISM are established for scale, while the current playable footprint remains a compact vertical slice.

Record the hardware, average frame rate, completion time, any stuck locations, and the first moment navigation became unclear. Those observations should drive the next milestone more than raw content volume.
