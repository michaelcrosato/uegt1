# UEGT1

UEGT1 is an Unreal Engine 5.8 C++ first-person project targeting Win64, DirectX 12, and Shader Model 6. Its launch menu offers two worlds: **Signal Grove**, the playable low-poly open-region adventure, and **Lumen Wilds**, a separate UE5 real-time environment showcase.

The v0.5 build adds a deterministic large-town simulation to Signal Grove: a west-expanded 14×11-tile region, 52 connected lots, 49 enterable furnished buildings, 100 autonomous residents with unique beds and explicit family/friend/roommate households, door/street-safe routing, four needs, utility planning, `$1` home groceries, and 20 roles at named grocery, factory, restaurant, professional-service, civic, healthcare, logistics, and retail businesses. The player shares the needs/economy/activity model and can sleep, eat/cook, shower, socialize, or work through 102 interior/world activity stations. A physically scaled 24-hour lighting cycle moves the atmosphere sun across the sky and coordinates sunlight, skylight, and fog through dawn, day, dusk, and night. Press `M` for a live full-island terrain map with streets, every building, the player, Sanctuary, Waystones, and a numbered key-service directory. The HUD also shows calendar date/time, local temperature, player needs, money, and the latest activity result. Citizens move smoothly at 440 cm/s, slightly below the player's 480 cm/s walk. Signal Grove retains its sanctuary objective, World Partition infrastructure, and graphics controls. Lumen Wilds remains a separate 360 m procedural environment showcase. Press `F8` for invincibility and fast traversal, then `F9` for flight; the menu exposes the same switches.

## Requirements

- Windows 10 22H2 or Windows 11 with a DirectX 12-capable GPU
- Unreal Engine 5.8 installed through Epic Games Launcher
- Visual Studio 2022 17.14 or newer with Game development with C++
- MSVC 14.38 or newer and Windows SDK 10.0.22621.0 or newer
- PowerShell 7, Git, and GitHub CLI

The checked-in `.vsconfig` describes the Visual Studio components used by this project.

## Bootstrap and verify

From PowerShell 7 in the repository root:

```powershell
./Scripts/Setup-Project.ps1
```

That command discovers UE 5.8, configures repository hooks, generates `UEGT1.sln`, builds the editor target, creates the initial map through Unreal Editor if needed, runs headless project automation, and verifies the environment.

For a custom engine build:

```powershell
./Scripts/Setup-Project.ps1 -EngineRoot 'D:\UnrealEngine'
```

Common individual commands:

```powershell
./Scripts/Generate-ProjectFiles.ps1
./Scripts/Build.ps1
./Scripts/Test.ps1
./Scripts/Smoke-Gameplay.ps1
./Scripts/Package.ps1
./Scripts/Smoke-PackagedBuild.ps1
./Scripts/Open-Editor.ps1
```

Generated folders, IDE state, test reports, logs, local builds, and secrets are intentionally ignored. Keep Unreal source assets (`Content/**/*.uasset` and `Content/**/*.umap`) under Git when they are intentional project content.

See [the architecture guide](Docs/Architecture.md) for system boundaries, [the town simulation guide](Docs/Town-Simulation.md) for generation, tuning, persistence, and extension rules, [the Lumen Wilds guide](Docs/Lumen-Wilds.md) for showcase authoring and verification, [the regional authoring guide](Docs/Region-Authoring.md) for Signal Grove's biome contract, and [the playtest guide](Docs/Playtest-0.1.md) for controls and acceptance checks.
