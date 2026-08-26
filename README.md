# UEGT1

UEGT1 is a native Windows Unreal Engine 5.8 C++ game project. It targets Win64, DirectX 12, and Shader Model 6.

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
./Scripts/Package.ps1
./Scripts/Open-Editor.ps1
```

Generated folders, IDE state, test reports, logs, local builds, and secrets are intentionally ignored. Keep Unreal source assets (`Content/**/*.uasset` and `Content/**/*.umap`) under Git when they are intentional project content.
