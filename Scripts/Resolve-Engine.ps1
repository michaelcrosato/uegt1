[CmdletBinding()]
param(
    [string] $EngineRoot
)

$ErrorActionPreference = 'Stop'
Set-StrictMode -Version Latest

$projectRoot = Split-Path -Parent $PSScriptRoot
$projectFile = Join-Path $projectRoot 'UEGT1.uproject'
$association = (Get-Content -LiteralPath $projectFile -Raw | ConvertFrom-Json).EngineAssociation
$candidates = [System.Collections.Generic.List[string]]::new()

if ($EngineRoot) {
    $candidates.Add($EngineRoot)
}
if ($env:UEGT1_ENGINE_ROOT) {
    $candidates.Add($env:UEGT1_ENGINE_ROOT)
}

$registryPaths = @(
    "HKLM:\SOFTWARE\EpicGames\Unreal Engine\$association",
    "HKLM:\SOFTWARE\WOW6432Node\EpicGames\Unreal Engine\$association"
)
foreach ($registryPath in $registryPaths) {
    $registryItem = Get-ItemProperty -LiteralPath $registryPath -ErrorAction SilentlyContinue
    if ($registryItem -and $registryItem.PSObject.Properties.Name -contains 'InstalledDirectory') {
        $candidates.Add($registryItem.InstalledDirectory)
    }
}

$userBuilds = Get-ItemProperty -LiteralPath 'HKCU:\SOFTWARE\Epic Games\Unreal Engine\Builds' -ErrorAction SilentlyContinue
if ($userBuilds -and $userBuilds.PSObject.Properties.Name -contains $association) {
    $candidates.Add($userBuilds.PSObject.Properties[$association].Value)
}

$manifestFolder = Join-Path $env:ProgramData 'Epic\EpicGamesLauncher\Data\Manifests'
if (Test-Path -LiteralPath $manifestFolder) {
    foreach ($manifest in Get-ChildItem -LiteralPath $manifestFolder -Filter '*.item' -File -ErrorAction SilentlyContinue) {
        try {
            $item = Get-Content -LiteralPath $manifest.FullName -Raw | ConvertFrom-Json
            if (($item.AppName -eq "UE_$association") -or ($item.DisplayName -eq "Unreal Engine $association")) {
                $candidates.Add($item.InstallLocation)
            }
        } catch {
            # Ignore unrelated or partially-written launcher manifests.
        }
    }
}

foreach ($drive in Get-PSDrive -PSProvider FileSystem) {
    $candidates.Add((Join-Path $drive.Root "Program Files\Epic Games\UE_$association"))
    $candidates.Add((Join-Path $drive.Root "Epic Games\UE_$association"))
}

foreach ($candidate in $candidates | Select-Object -Unique) {
    if (-not $candidate) {
        continue
    }
    $resolvedCandidate = [Environment]::ExpandEnvironmentVariables($candidate).TrimEnd('\', '/')
    $buildScript = Join-Path $resolvedCandidate 'Engine\Build\BatchFiles\Build.bat'
    if (Test-Path -LiteralPath $buildScript -PathType Leaf) {
        Write-Output $resolvedCandidate
        exit 0
    }
}

throw "Unreal Engine $association was not found. Install it with Epic Games Launcher or pass -EngineRoot / set UEGT1_ENGINE_ROOT."
