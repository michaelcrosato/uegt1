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
$generationLog = Join-Path $projectRoot 'Saved\Logs\CreateInitialContent.log'
$externalActorFolder = Join-Path $projectRoot 'Content\__ExternalActors__\Maps\Main'
$materialFiles = @(
    (Join-Path $projectRoot 'Content\Materials\M_SignalSurface.uasset'),
    (Join-Path $projectRoot 'Content\Materials\M_SignalWater.uasset'),
    (Join-Path $projectRoot 'Content\Materials\M_SignalGlow.uasset')
)
$mapWasWorldPartition = Test-Path -LiteralPath $externalActorFolder -PathType Container
if (Test-Path -LiteralPath $generationLog) {
    Remove-Item -LiteralPath $generationLog -Force
}

Write-Host 'Generating the initial map through Unreal Editor'
& $editorCommand $projectFile -unattended -nop4 -nosplash -NullRHI "-ExecutePythonScript=$pythonScript" $forceArgument "-abslog=$generationLog"
if ($LASTEXITCODE -ne 0) {
    throw "Initial content generation failed with exit code $LASTEXITCODE."
}
if (-not (Test-Path -LiteralPath $generationLog -PathType Leaf) -or
    -not (Select-String -LiteralPath $generationLog -SimpleMatch 'UEGT1_CONTENT_GENERATION_SUCCEEDED' -Quiet)) {
    throw "Initial content generation did not report success. Inspect $generationLog."
}
if (-not (Test-Path -LiteralPath $mapFile -PathType Leaf)) {
    throw 'Unreal exited successfully but Content/Maps/Main.umap was not created.'
}
foreach ($materialFile in $materialFiles) {
    if (-not (Test-Path -LiteralPath $materialFile -PathType Leaf)) {
        throw "Unreal exited successfully but visual material '$materialFile' was not created."
    }
}

if (-not $mapWasWorldPartition) {
    Write-Host 'Converting Main to World Partition / One File Per Actor'
    & $editorCommand $projectFile '/Game/Maps/Main' -run=WorldPartitionConvertCommandlet -AllowCommandletRendering -unattended -nop4 -nosplash -log
    if ($LASTEXITCODE -ne 0) {
        throw "World Partition conversion failed with exit code $LASTEXITCODE."
    }
}

if (-not (Test-Path -LiteralPath $externalActorFolder -PathType Container)) {
    throw 'World Partition conversion completed without creating external actor packages.'
}
Write-Host "Created World Partition map $mapFile"
