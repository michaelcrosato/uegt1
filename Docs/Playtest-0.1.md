# Signal Grove v0.1 playtest

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
| Diagnostics | F3 | — |
| Exit standalone build | Alt+F4 | — |

## What to check

1. Movement starts immediately, mouse look feels stable, sprint widens the field of view, and jumping does not snag on the trail.
2. Tan paths remain readable through the low-poly forest and each path terminates at an amber Waystone.
3. Looking at a Waystone from close range displays `[E] Stabilize Waystone`.
4. Activating a Waystone changes its shards and light from amber to teal and increments the HUD counter exactly once.
5. After the third activation, the central sanctuary brightens and accelerates, and the HUD reports completion.
6. `F3` shows a plausible frame rate and position. Look for hitches, shadow instability, obvious instance pop, gaps at tile edges, or places where foliage blocks a reserved trail.

## Known v0.1 limits

- No save/load: objective progress resets when the session ends.
- No audio, characters, combat, inventory, quests, settings menu, input rebinding, map, or accessibility UI yet.
- Environment geometry intentionally uses engine primitives and procedural instances; landmark art and terrain sculpting are foundation-quality.
- The HUD is Canvas-based and keyboard-first. Gamepad interaction works, but prompts are not input-device adaptive.
- World Partition and HISM are established for scale, while the current playable footprint remains a compact vertical slice.

Record the hardware, average frame rate, completion time, any stuck locations, and the first moment navigation became unclear. Those observations should drive the next milestone more than raw content volume.
