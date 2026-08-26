[CmdletBinding()]
param(
    [string] $EngineRoot,
    [switch] $Force
)

$ErrorActionPreference = 'Stop'
Set-StrictMode -Version Latest

$projectRoot = Split-Path -Parent $PSScriptRoot
$projectFile = Join-Path $projectRoot 'UEGT1.uproject'
$mapFile = Join-Path $projectRoot 'Content\Maps\Main.umap'
if ((Test-Path -LiteralPath $mapFile) -and -not $Force) {
    Write-Host 'Content/Maps/Main.umap already exists; nothing to generate.'
    exit 0
}

$resolvedEngine = & (Join-Path $PSScriptRoot 'Resolve-Engine.ps1') -EngineRoot $EngineRoot
$editorCommand = Join-Path $resolvedEngine 'Engine\Binaries\Win64\UnrealEditor-Cmd.exe'
$pythonScript = Join-Path $PSScriptRoot 'CreateInitialContent.py'
$forceArgument = if ($Force) { '-UEGT1ForceContent' } else { '-UEGT1CreateContent' }

Write-Host 'Generating the initial map through Unreal Editor'
& $editorCommand $projectFile -unattended -nop4 -nosplash -NullRHI "-ExecutePythonScript=$pythonScript" $forceArgument -log
if ($LASTEXITCODE -ne 0) {
    throw "Initial content generation failed with exit code $LASTEXITCODE."
}
if (-not (Test-Path -LiteralPath $mapFile -PathType Leaf)) {
    throw 'Unreal exited successfully but Content/Maps/Main.umap was not created.'
}
Write-Host "Created $mapFile"

