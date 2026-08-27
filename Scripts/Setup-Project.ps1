[CmdletBinding()]
param(
    [string] $EngineRoot,
    [switch] $SkipContent,
    [switch] $SkipTests
)

$ErrorActionPreference = 'Stop'
Set-StrictMode -Version Latest

$projectRoot = Split-Path -Parent $PSScriptRoot
$resolvedEngine = & (Join-Path $PSScriptRoot 'Resolve-Engine.ps1') -EngineRoot $EngineRoot

Push-Location $projectRoot
try {
    git config core.hooksPath .githooks
    & (Join-Path $PSScriptRoot 'Validate-Repository.ps1')
    & (Join-Path $PSScriptRoot 'Generate-ProjectFiles.ps1') -EngineRoot $resolvedEngine
    & (Join-Path $PSScriptRoot 'Build.ps1') -EngineRoot $resolvedEngine
    if (-not $SkipContent) {
        & (Join-Path $PSScriptRoot 'Create-InitialContent.ps1') -EngineRoot $resolvedEngine
        & (Join-Path $PSScriptRoot 'Create-TechDemoContent.ps1') -EngineRoot $resolvedEngine
    }
    if (-not $SkipTests) {
        & (Join-Path $PSScriptRoot 'Test.ps1') -EngineRoot $resolvedEngine -SkipBuild
        & (Join-Path $PSScriptRoot 'Smoke-Gameplay.ps1') -EngineRoot $resolvedEngine
    }
    & (Join-Path $PSScriptRoot 'Verify-Environment.ps1') -EngineRoot $resolvedEngine
} finally {
    Pop-Location
}

Write-Host 'UEGT1 setup and verification completed successfully.' -ForegroundColor Green
