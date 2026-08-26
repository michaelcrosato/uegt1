# UEGT1

UEGT1 is the Unreal Engine 5.8 C++ project for **Signal Grove**, a first-person low-poly open-world foundation targeting Win64, DirectX 12, and Shader Model 6.

The v0.2 vertical slice is playable: explore a deterministic forest, follow three paths, stabilize three reusable interactable Waystones, and restore the central sanctuary. A launch/pause menu now provides persistent display, quality, and per-feature graphics controls plus an in-game quit path. The checked-in map uses World Partition and One File Per Actor; repeated biome geometry uses hierarchical instancing instead of one Actor per prop.

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

See [the architecture guide](Docs/Architecture.md) for system boundaries and extension rules, and [the playtest guide](Docs/Playtest-0.1.md) for controls, graphics settings, the test loop, and known limits.
