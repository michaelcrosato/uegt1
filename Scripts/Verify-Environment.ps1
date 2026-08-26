[CmdletBinding()]
param(
    [string] $EngineRoot
)

$ErrorActionPreference = 'Stop'
Set-StrictMode -Version Latest

$projectRoot = Split-Path -Parent $PSScriptRoot
$failures = [System.Collections.Generic.List[string]]::new()

function Test-ExternalCommand([string] $Name) {
    if (-not (Get-Command $Name -ErrorAction SilentlyContinue)) {
        $script:failures.Add("Required command is not available: $Name")
        return $false
    }
    return $true
}

Write-Host "PowerShell $($PSVersionTable.PSVersion) ($($PSVersionTable.PSEdition))"
if ($PSVersionTable.PSVersion.Major -lt 7) {
    $failures.Add('PowerShell 7 or later is required.')
}

$hasGit = Test-ExternalCommand 'git'
$hasGh = Test-ExternalCommand 'gh'
if ($hasGit) {
    git --version
}
if ($hasGh) {
    gh --version | Select-Object -First 1
    & gh auth status
    if ($LASTEXITCODE -ne 0) {
        $failures.Add('GitHub CLI is not authenticated.')
    }
}

$vswhere = Join-Path ${env:ProgramFiles(x86)} 'Microsoft Visual Studio\Installer\vswhere.exe'
if (-not (Test-Path -LiteralPath $vswhere -PathType Leaf)) {
    $failures.Add('Visual Studio Installer discovery tool (vswhere.exe) was not found.')
} else {
    $visualStudio = & $vswhere -latest -products * -requires Microsoft.VisualStudio.Component.VC.Tools.x86.x64 -property installationPath
    if (-not $visualStudio) {
        $failures.Add('A Visual Studio installation with MSVC x64 tools was not found.')
    } else {
        Write-Host "MSVC toolchain: $visualStudio"
    }
}

$sdkRoot = Join-Path ${env:ProgramFiles(x86)} 'Windows Kits\10\Include'
$sdkVersions = @(Get-ChildItem -LiteralPath $sdkRoot -Directory -ErrorAction SilentlyContinue | Where-Object {
    try { [version]$_.Name -ge [version]'10.0.22621.0' } catch { $false }
})
if ($sdkVersions.Count -eq 0) {
    $failures.Add('Windows SDK 10.0.22621.0 or later was not found.')
} else {
    Write-Host "Windows SDK: $($sdkVersions[-1].Name)"
}

try {
    $resolvedEngine = & (Join-Path $PSScriptRoot 'Resolve-Engine.ps1') -EngineRoot $EngineRoot
    Write-Host "Unreal Engine: $resolvedEngine"
} catch {
    $failures.Add($_.Exception.Message)
}

& (Join-Path $PSScriptRoot 'Validate-Repository.ps1')
if ($LASTEXITCODE -ne 0) {
    $failures.Add('Repository validation failed.')
}

if ($failures.Count -gt 0) {
    Write-Host 'Environment verification failed:' -ForegroundColor Red
    $failures | ForEach-Object { Write-Host " - $_" -ForegroundColor Red }
    exit 1
}

Write-Host 'Environment verification passed.' -ForegroundColor Green

