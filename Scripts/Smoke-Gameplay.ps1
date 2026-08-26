[CmdletBinding()]
param(
    [string] $EngineRoot
)

$ErrorActionPreference = 'Stop'
Set-StrictMode -Version Latest

$projectRoot = Split-Path -Parent $PSScriptRoot
$projectFile = Join-Path $projectRoot 'UEGT1.uproject'
$runtimeLog = Join-Path $projectRoot 'Saved\Logs\SignalGrove-RuntimeSmoke.log'
$resolvedEngine = & (Join-Path $PSScriptRoot 'Resolve-Engine.ps1') -EngineRoot $EngineRoot
$editorCommand = Join-Path $resolvedEngine 'Engine\Binaries\Win64\UnrealEditor-Cmd.exe'

Write-Host 'Loading Signal Grove as a headless game world'
& $editorCommand $projectFile '/Game/Maps/Main' -game -NullRHI -unattended -nop4 -nosplash '-ExecCmds=Quit' "-abslog=$runtimeLog"
if ($LASTEXITCODE -ne 0) {
    throw "Gameplay smoke failed with exit code $LASTEXITCODE. Inspect $runtimeLog."
}
if (-not (Test-Path -LiteralPath $runtimeLog -PathType Leaf)) {
    throw "Gameplay smoke did not create $runtimeLog."
}

$logText = Get-Content -LiteralPath $runtimeLog -Raw
$requiredSignals = @(
    'Game Engine Initialized.',
    'LoadMap: /Game/Maps/Main',
    'Regional foundation ready: Seed=7319 Tiles=121 Expected=121',
    'Town foundation ready: Buildings=',
    'Waystone registered: Id=EastRise',
    'Waystone registered: Id=WestHollow',
    'Waystone registered: Id=SouthWatch'
)
foreach ($signal in $requiredSignals) {
    if (-not $logText.Contains($signal)) {
        throw "Gameplay smoke is missing expected log signal: $signal"
    }
}
if ($logText.Contains('No authored biome tiles found')) {
    throw 'Gameplay smoke used the runtime biome fallback instead of the authored World Partition content.'
}
if ($logText.Contains('No authored town found') -or $logText.Contains('Regional tile coverage mismatch')) {
    throw 'Gameplay smoke did not load the complete authored town and regional tile coverage.'
}

Write-Host "Gameplay smoke passed: authored 121-tile regional World Partition, town, and all three Waystones loaded."
Write-Host "Runtime log: $runtimeLog"
