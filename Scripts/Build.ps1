[CmdletBinding()]
param(
    [ValidateSet('Debug', 'DebugGame', 'Development', 'Shipping', 'Test')]
    [string] $Configuration = 'Development',
    [ValidateSet('Editor', 'Game')]
    [string] $Target = 'Editor',
    [string] $EngineRoot,
    [switch] $Clean
)

$ErrorActionPreference = 'Stop'
Set-StrictMode -Version Latest

$projectRoot = Split-Path -Parent $PSScriptRoot
$projectFile = Join-Path $projectRoot 'UEGT1.uproject'
$resolvedEngine = & (Join-Path $PSScriptRoot 'Resolve-Engine.ps1') -EngineRoot $EngineRoot
$buildScript = Join-Path $resolvedEngine 'Engine\Build\BatchFiles\Build.bat'
$targetName = if ($Target -eq 'Editor') { 'UEGT1Editor' } else { 'UEGT1' }
$arguments = @($targetName, 'Win64', $Configuration, $projectFile, '-WaitMutex', '-NoHotReloadFromIDE')
if ($Clean) {
    $arguments += '-Clean'
}

Write-Host "Building $targetName Win64 $Configuration with $resolvedEngine"
& $buildScript @arguments
if ($LASTEXITCODE -ne 0) {
    throw "Unreal build failed with exit code $LASTEXITCODE."
}

