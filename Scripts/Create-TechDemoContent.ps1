[CmdletBinding()]
param(
    [string] $EngineRoot,
    [switch] $Force
)

$ErrorActionPreference = 'Stop'
Set-StrictMode -Version Latest

$projectRoot = Split-Path -Parent $PSScriptRoot
$projectFile = Join-Path $projectRoot 'UEGT1.uproject'
$mapFile = Join-Path $projectRoot 'Content\Maps\TechDemo.umap'
if ((Test-Path -LiteralPath $mapFile) -and -not $Force) {
    Write-Host 'Content/Maps/TechDemo.umap already exists; nothing to generate.'
    exit 0
}

$resolvedEngine = & (Join-Path $PSScriptRoot 'Resolve-Engine.ps1') -EngineRoot $EngineRoot
$editorCommand = Join-Path $resolvedEngine 'Engine\Binaries\Win64\UnrealEditor-Cmd.exe'
$pythonScript = Join-Path $PSScriptRoot 'CreateTechDemoContent.py'
$generationLog = Join-Path $projectRoot 'Saved\Logs\CreateTechDemoContent.log'
$materialFiles = @(
    (Join-Path $projectRoot 'Content\Materials\M_TechTerrain.uasset'),
    (Join-Path $projectRoot 'Content\Materials\M_TechSurface.uasset'),
    (Join-Path $projectRoot 'Content\Materials\M_TechFoliage.uasset'),
    (Join-Path $projectRoot 'Content\Materials\M_TechWater.uasset')
)
if (Test-Path -LiteralPath $generationLog) {
    Remove-Item -LiteralPath $generationLog -Force
}

Write-Host 'Generating the Lumen Wilds UE5 showcase level through Unreal Editor'
& $editorCommand $projectFile -unattended -nop4 -nosplash -NullRHI "-ExecutePythonScript=$pythonScript" '-UEGT1CreateTechDemo' "-abslog=$generationLog"
if ($LASTEXITCODE -ne 0) {
    throw "Tech-demo content generation failed with exit code $LASTEXITCODE."
}
if (-not (Test-Path -LiteralPath $generationLog -PathType Leaf) -or
    -not (Select-String -LiteralPath $generationLog -SimpleMatch 'UEGT1_TECH_DEMO_GENERATION_SUCCEEDED' -Quiet)) {
    throw "Tech-demo content generation did not report success. Inspect $generationLog."
}
if (-not (Test-Path -LiteralPath $mapFile -PathType Leaf)) {
    throw 'Unreal exited successfully but Content/Maps/TechDemo.umap was not created.'
}
foreach ($materialFile in $materialFiles) {
    if (-not (Test-Path -LiteralPath $materialFile -PathType Leaf)) {
        throw "Unreal exited successfully but tech-demo material '$materialFile' was not created."
    }
}
Write-Host "Created Lumen Wilds map $mapFile"
