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
$menuLog = Join-Path $logFolder 'SignalGrove-MenuSmoke.log'
$menuScreenshot = Join-Path $screenshotFolder 'SignalGrove-MenuSmoke.png'
$regionLog = Join-Path $logFolder 'SignalGrove-RegionSmoke.log'
$regionScreenshotFolder = Join-Path $screenshotFolder 'Region'
$regionScreenshots = @(
    (Join-Path $regionScreenshotFolder '01-CenterTown.png'),
    (Join-Path $regionScreenshotFolder '02-EastWaterfront.png'),
    (Join-Path $regionScreenshotFolder '03-WestFarmland.png'),
    (Join-Path $regionScreenshotFolder '04-NorthHighlands.png'),
    (Join-Path $regionScreenshotFolder '05-SouthTropics.png')
)
New-Item -ItemType Directory -Path $regionScreenshotFolder -Force | Out-Null
foreach ($oldArtifact in @($runtimeLog, $screenshot, $menuLog, $menuScreenshot, $regionLog) + $regionScreenshots) {
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
    '-UEGT1SmokeResetSettings',
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
    'Automated smoke restored recommended settings before capture.',
    'Automated view: Location=V(Y=-1350.00',
    'Regional foundation ready: Seed=7319 Tiles=121 Expected=121',
    'Town foundation ready: Buildings=',
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
if ($logText.Contains('No authored town found') -or $logText.Contains('Regional tile coverage mismatch')) {
    throw 'Packaged smoke did not load the complete authored regional foundation.'
}
if ($logText.Contains("Couldn't spawn Pawn")) {
    throw 'Packaged smoke failed to spawn the first-person player pawn.'
}
if ($logText.Contains('missing usage flag') -or $logText.Contains('Default Material will be used in game')) {
    throw 'Packaged smoke substituted Unreal default materials; inspect material usage flags.'
}

Write-Host 'Packaged gameplay smoke passed: DX12 regional world load, objective completion, and rendered town frame verified.'
Write-Host "Runtime log: $runtimeLog"
Write-Host "Screenshot: $screenshot"

$menuArguments = @(
    '-RenderOffscreen',
    '-Windowed',
    '-ResX=1920',
    '-ResY=1080',
    '-unattended',
    '-nosplash',
    '-UEGT1SmokeResetSettings',
    "-UEGT1SmokeMenuCapture=$menuScreenshot",
    "-abslog=$menuLog"
)
Write-Host 'Running rendered menu/settings/quit smoke'
$menuProcess = Start-Process -FilePath $Executable -ArgumentList $menuArguments -WorkingDirectory (Split-Path -Parent $Executable) -WindowStyle Hidden -PassThru
if (-not $menuProcess.WaitForExit($TimeoutSeconds * 1000)) {
    Stop-Process -Id $menuProcess.Id -Force
    throw "Packaged menu smoke did not finish within $TimeoutSeconds seconds."
}
if ($menuProcess.ExitCode -ne 0) {
    throw "Packaged menu smoke exited with code $($menuProcess.ExitCode). Inspect $menuLog."
}
if (-not (Test-Path -LiteralPath $menuLog -PathType Leaf) -or -not (Test-Path -LiteralPath $menuScreenshot -PathType Leaf)) {
    throw 'Packaged menu smoke did not create both its runtime log and screenshot.'
}

$menuLogText = Get-Content -LiteralPath $menuLog -Raw
$requiredMenuSignals = @(
    'Automated smoke restored recommended settings before capture.',
    'Game menu opened: Initial=false',
    'Graphics settings page opened.',
    'Automated graphics menu screenshot requested:',
    'Graphics settings applied: Quality=-1 Resolution=X=1920 Y=1080 Scale=100 VSync=On Optional=AllOn',
    'Automated menu smoke restored recommended settings.',
    'Menu quit requested; exiting to desktop.'
)
foreach ($signal in $requiredMenuSignals) {
    if (-not $menuLogText.Contains($signal)) {
        throw "Packaged menu smoke is missing expected log signal: $signal"
    }
}

$packagedRoot = Split-Path -Parent $Executable
$settingsFile = Get-ChildItem -LiteralPath $packagedRoot -Recurse -File -Filter 'GameUserSettings.ini' -ErrorAction SilentlyContinue |
    Where-Object { $_.FullName -like '*\Saved\Config\Windows\GameUserSettings.ini' } |
    Select-Object -First 1
if (-not $settingsFile) {
    throw 'Packaged menu smoke did not persist GameUserSettings.ini.'
}
$settingsText = Get-Content -LiteralPath $settingsFile.FullName -Raw
$customSectionMatch = [regex]::Match(
    $settingsText,
    '(?ms)^\[/Script/UEGT1\.UEGT1GameUserSettings\]\r?\n(?<Body>.*?)(?=^\[|\z)'
)
if (-not $customSectionMatch.Success) {
    throw 'Persisted settings are missing the UEGT1 custom settings section.'
}
$customSection = $customSectionMatch.Groups['Body'].Value
foreach ($setting in @(
    'DesiredScreenWidth=1920',
    'DesiredScreenHeight=1080'
)) {
    if (-not $customSection.Contains($setting)) {
        throw "Packaged menu smoke did not persist recommended setting: $setting"
    }
}
if ($customSection -match '(?m)^b[A-Za-z]+Enabled=False\r?$') {
    throw 'Packaged menu smoke persisted an optional graphics feature as disabled after restoring recommendations.'
}
if ($settingsText -notmatch '(?m)^sg\.ResolutionQuality=100(?:\.0+)?\r?$' -or
    $settingsText -notmatch '(?m)^sg\.ViewDistanceQuality=2\r?$' -or
    $settingsText -notmatch '(?m)^sg\.TextureQuality=3\r?$') {
    throw 'Packaged menu smoke did not persist the recommended quality profile.'
}

Write-Host 'Packaged menu smoke passed: settings screen rendered, recommendations persisted, and the in-menu quit path exited cleanly.'
Write-Host "Menu log: $menuLog"
Write-Host "Menu screenshot: $menuScreenshot"
Write-Host "Persisted settings: $($settingsFile.FullName)"

$regionArguments = @(
    '-RenderOffscreen',
    '-Windowed',
    '-ResX=1920',
    '-ResY=1080',
    '-unattended',
    '-nosplash',
    '-UEGT1SmokeResetSettings',
    "-UEGT1RegionCaptureFolder=$regionScreenshotFolder",
    "-abslog=$regionLog"
)
Write-Host 'Running five-view rendered regional smoke'
$regionProcess = Start-Process -FilePath $Executable -ArgumentList $regionArguments -WorkingDirectory (Split-Path -Parent $Executable) -WindowStyle Hidden -PassThru
if (-not $regionProcess.WaitForExit($TimeoutSeconds * 1000)) {
    Stop-Process -Id $regionProcess.Id -Force
    throw "Packaged regional capture did not finish within $TimeoutSeconds seconds."
}
if ($regionProcess.ExitCode -ne 0 -or -not (Test-Path -LiteralPath $regionLog -PathType Leaf)) {
    throw "Packaged regional capture failed with exit code $($regionProcess.ExitCode). Inspect $regionLog."
}
foreach ($regionScreenshot in $regionScreenshots) {
    if (-not (Test-Path -LiteralPath $regionScreenshot -PathType Leaf)) {
        throw "Packaged regional capture did not create $regionScreenshot."
    }
}
$regionLogText = Get-Content -LiteralPath $regionLog -Raw
foreach ($view in @('CenterTown', 'EastWaterfront', 'WestFarmland', 'NorthHighlands', 'SouthTropics')) {
    if (-not $regionLogText.Contains("Automated regional screenshot requested: View=$view")) {
        throw "Packaged regional capture is missing the $view view."
    }
}
foreach ($biomeSignal in @('Dominant=Town', 'Dominant=Coast', 'Dominant=Farmland', 'Dominant=Highlands', 'Dominant=Tropical')) {
    if (-not $regionLogText.Contains($biomeSignal)) {
        throw "Packaged regional capture is missing expected sampler signal: $biomeSignal"
    }
}
if (-not $regionLogText.Contains('Regional foundation ready: Seed=7319 Tiles=121 Expected=121') -or
    $regionLogText.Contains('Regional tile coverage mismatch')) {
    throw 'Packaged regional capture did not load the complete configured tile grid.'
}
if ($regionLogText.Contains('missing usage flag') -or $regionLogText.Contains('Default Material will be used in game')) {
    throw 'Packaged regional capture substituted Unreal default materials; inspect material usage flags.'
}
Write-Host 'Packaged regional smoke passed: town, waterfront, farmland, highlands, and tropics rendered from deterministic viewpoints.'
Write-Host "Regional log: $regionLog"
Write-Host "Regional screenshots: $regionScreenshotFolder"
