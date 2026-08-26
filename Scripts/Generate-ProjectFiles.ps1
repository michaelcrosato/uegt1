[CmdletBinding()]
param(
    [string] $EngineRoot
)

$ErrorActionPreference = 'Stop'
Set-StrictMode -Version Latest

$projectRoot = Split-Path -Parent $PSScriptRoot
$projectFile = Join-Path $projectRoot 'UEGT1.uproject'
$resolvedEngine = & (Join-Path $PSScriptRoot 'Resolve-Engine.ps1') -EngineRoot $EngineRoot
$generator = Join-Path $resolvedEngine 'Engine\Build\BatchFiles\GenerateProjectFiles.bat'

Write-Host "Generating Visual Studio project files with $resolvedEngine"
& $generator "-project=$projectFile" -game -engine -progress
if ($LASTEXITCODE -ne 0) {
    throw "Unreal project generation failed with exit code $LASTEXITCODE."
}

$solution = Join-Path $projectRoot 'UEGT1.sln'
if (-not (Test-Path -LiteralPath $solution -PathType Leaf)) {
    throw "Project generation completed without creating $solution."
}
Write-Host "Generated $solution"

