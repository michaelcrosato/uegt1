[CmdletBinding()]
param(
    [string] $Executable,
    [int] $TimeoutSeconds = 120
)

$ErrorActionPreference = 'Stop'
Set-StrictMode -Version Latest

$projectRoot = Split-Path -Parent $PSScriptRoot
if (-not $Executable) {
    $defaultBuild = Join-Path $projectRoot 'LocalBuilds\Windows-Development'
    $candidate = Get-ChildItem -LiteralPath $defaultBuild -Recurse -File -Filter 'UEGT1.exe' -ErrorAction SilentlyContinue | Select-Object -First 1
    if (-not $candidate) {
        throw 'No packaged UEGT1.exe was found. Run Scripts/Package.ps1 first or pass -Executable.'
    }
    $Executable = $candidate.FullName
}
$Executable = [IO.Path]::GetFullPath($Executable)
if (-not (Test-Path -LiteralPath $Executable -PathType Leaf)) {
    throw "Packaged executable not found: $Executable"
}

$logFolder = Join-Path $projectRoot 'Saved\Logs'
$screenshotFolder = Join-Path $projectRoot 'Saved\Screenshots'
New-Item -ItemType Directory -Path $logFolder,$screenshotFolder -Force | Out-Null
$runtimeLog = Join-Path $logFolder 'SignalGrove-PackagedSmoke.log'
$screenshot = Join-Path $screenshotFolder 'SignalGrove-PackagedSmoke.png'
foreach ($oldArtifact in @($runtimeLog, $screenshot)) {
    if (Test-Path -LiteralPath $oldArtifact) {
        Remove-Item -LiteralPath $oldArtifact -Force
    }
}

$arguments = @(
    '-RenderOffscreen',
    '-Windowed',
    '-ResX=1920',
    '-ResY=1080',
    '-unattended',
    '-nosplash',
    '-UEGT1SmokeComplete',
    "-UEGT1SmokeCapture=$screenshot",
    "-abslog=$runtimeLog"
)
Write-Host "Running rendered packaged smoke with $Executable"
$process = Start-Process -FilePath $Executable -ArgumentList $arguments -WorkingDirectory (Split-Path -Parent $Executable) -WindowStyle Hidden -PassThru
if (-not $process.WaitForExit($TimeoutSeconds * 1000)) {
    Stop-Process -Id $process.Id -Force
    throw "Packaged gameplay did not finish within $TimeoutSeconds seconds."
}
if ($process.ExitCode -ne 0) {
    throw "Packaged gameplay exited with code $($process.ExitCode). Inspect $runtimeLog."
}
if (-not (Test-Path -LiteralPath $runtimeLog -PathType Leaf)) {
    throw "Packaged gameplay did not create $runtimeLog."
}
if (-not (Test-Path -LiteralPath $screenshot -PathType Leaf)) {
    throw "Packaged gameplay did not create $screenshot."
}

$logText = Get-Content -LiteralPath $runtimeLog -Raw
$requiredSignals = @(
    'Runtime PlayerStart ready:',
    'Automated view: Location=V(Y=-1300.00',
    'Signal Grove ready: Seed=7319 Tiles=25 Instances=1350 Waystones=3',
    'Waystone activated: Id=EastRise',
    'Waystone activated: Id=WestHollow',
    'Waystone activated: Id=SouthWatch',
    'Signal Grove milestone complete: sanctuary restored.',
    'Automated gameplay smoke completed.'
)
foreach ($signal in $requiredSignals) {
    if (-not $logText.Contains($signal)) {
        throw "Packaged smoke is missing expected log signal: $signal"
    }
}
if ($logText.Contains('No authored biome tiles found')) {
    throw 'Packaged smoke used the runtime biome fallback instead of authored content.'
}
if ($logText.Contains("Couldn't spawn Pawn")) {
    throw 'Packaged smoke failed to spawn the first-person player pawn.'
}

Write-Host 'Packaged gameplay smoke passed: DX12 world load, objective completion, and rendered frame verified.'
Write-Host "Runtime log: $runtimeLog"
Write-Host "Screenshot: $screenshot"
