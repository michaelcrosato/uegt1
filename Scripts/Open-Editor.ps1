[CmdletBinding()]
param(
    [string] $EngineRoot
)

$ErrorActionPreference = 'Stop'
Set-StrictMode -Version Latest

$projectRoot = Split-Path -Parent $PSScriptRoot
$projectFile = Join-Path $projectRoot 'UEGT1.uproject'
$resolvedEngine = & (Join-Path $PSScriptRoot 'Resolve-Engine.ps1') -EngineRoot $EngineRoot
$editor = Join-Path $resolvedEngine 'Engine\Binaries\Win64\UnrealEditor.exe'

Start-Process -FilePath $editor -ArgumentList @($projectFile, '-log') -WorkingDirectory $projectRoot
Write-Host "Opened UEGT1 with $editor"

