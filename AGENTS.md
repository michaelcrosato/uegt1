# UEGT1 engineering guide

## Project contract

- This is a native Windows Unreal Engine 5.8 C++ game targeting Win64 and DirectX 12 / Shader Model 6.
- Keep the project launchable after every substantial change. Prefer Unreal-native patterns, clear module boundaries, and assets that remain source-control friendly.
- Use C++ for durable gameplay systems and performance-sensitive behavior. Use Blueprints for asset composition, tuning, and designer-facing orchestration when that improves iteration.
- Never commit credentials or machine-local state. `Binaries`, `DerivedDataCache`, `Intermediate`, `Saved`, IDE state, packaged builds, and local environment files stay untracked.

## Standard commands

Run commands from the repository root with PowerShell 7:

```powershell
./Scripts/Verify-Environment.ps1
./Scripts/Generate-ProjectFiles.ps1
./Scripts/Build.ps1
./Scripts/Test.ps1
./Scripts/Open-Editor.ps1
```

`Scripts/Resolve-Engine.ps1` discovers the UE 5.8 launcher install. For a nonstandard/source build, pass `-EngineRoot` or set `UEGT1_ENGINE_ROOT` for the current process. Do not commit absolute engine paths.

## Verification

- After C++ or build-rule changes, run `Scripts/Build.ps1` and the narrowest relevant automation tests.
- After config/content changes, run `Scripts/Test.ps1`; visually exercise the changed flow when editor rendering or interaction matters.
- Inspect `Saved/Logs/UEGT1.log` when an editor, commandlet, build, or automation run fails.
- `Scripts/Setup-Project.ps1` is the end-to-end bootstrap: project generation, editor build, initial content, smoke tests, and environment verification.
- Keep tests under the `UEGT1.*` automation namespace so the standard test command discovers them.

## Git workflow

- Work on focused branches once normal feature development begins; keep `main` buildable.
- Make coherent commits with imperative messages. Review staged files before every commit.
- The repository hook and GitHub workflow validate structure and guard against generated files; they do not replace a real local UE build.
- Do not rewrite or discard user changes. Avoid committing unrelated work.

