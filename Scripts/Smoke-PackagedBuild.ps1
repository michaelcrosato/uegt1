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
$levelMenuLog = Join-Path $logFolder 'SignalGrove-LevelMenuSmoke.log'
$levelMenuScreenshot = Join-Path $screenshotFolder 'SignalGrove-LevelMenuSmoke.png'
$regionLog = Join-Path $logFolder 'SignalGrove-RegionSmoke.log'
$regionScreenshotFolder = Join-Path $screenshotFolder 'Region'
$regionScreenshots = @(
    (Join-Path $regionScreenshotFolder '01-CenterTown.png'),
    (Join-Path $regionScreenshotFolder '02-WestTownExpansion.png'),
    (Join-Path $regionScreenshotFolder '03-EastWaterfront.png'),
    (Join-Path $regionScreenshotFolder '04-WestFarmland.png'),
    (Join-Path $regionScreenshotFolder '05-NorthHighlands.png'),
    (Join-Path $regionScreenshotFolder '06-SouthTropics.png')
)
$environmentLog = Join-Path $logFolder 'SignalGrove-DayNightMapSmoke.log'
$environmentScreenshotFolder = Join-Path $screenshotFolder 'DayNightMap'
$environmentScreenshots = @(
    (Join-Path $environmentScreenshotFolder '01-Dawn.png'),
    (Join-Path $environmentScreenshotFolder '02-Noon.png'),
    (Join-Path $environmentScreenshotFolder '03-GoldenHour.png'),
    (Join-Path $environmentScreenshotFolder '04-Midnight.png'),
    (Join-Path $environmentScreenshotFolder '05-WorldMap.png')
)
$techDemoLog = Join-Path $logFolder 'LumenWilds-PackagedSmoke.log'
$techDemoScreenshotFolder = Join-Path $screenshotFolder 'LumenWilds'
$techDemoScreenshots = @(
    (Join-Path $techDemoScreenshotFolder '01-ValleyApproach.png'),
    (Join-Path $techDemoScreenshotFolder '02-LakeOverlook.png'),
    (Join-Path $techDemoScreenshotFolder '03-CanopyFlight.png')
)
New-Item -ItemType Directory -Path $regionScreenshotFolder -Force | Out-Null
New-Item -ItemType Directory -Path $environmentScreenshotFolder -Force | Out-Null
New-Item -ItemType Directory -Path $techDemoScreenshotFolder -Force | Out-Null
foreach ($oldArtifact in @($runtimeLog, $screenshot, $menuLog, $menuScreenshot, $levelMenuLog, $levelMenuScreenshot, $regionLog, $environmentLog, $techDemoLog) + $regionScreenshots + $environmentScreenshots + $techDemoScreenshots) {
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
    'Regional foundation ready: Seed=7319 Tiles=154 Expected=154',
	'Town foundation ready: Buildings=49',
	'Interiors=49 ActivityStations=102',
	'Venues=52 Streets=18 Beds=100',
	'Town simulation ready: Seed=7319 NPCs=100 Spawned=100 Venues=52 Jobs=20 Beds=100 AssignedBeds=100 RelatedHouseholds=100',
	'Resident visual interpolation observed:',
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

$levelMenuArguments = @(
    '-RenderOffscreen',
    '-Windowed',
    '-ResX=1920',
    '-ResY=1080',
    '-unattended',
    '-nosplash',
    "-UEGT1LevelMenuCapture=$levelMenuScreenshot",
    "-abslog=$levelMenuLog"
)
Write-Host 'Running rendered level-selection menu smoke'
$levelMenuProcess = Start-Process -FilePath $Executable -ArgumentList $levelMenuArguments -WorkingDirectory (Split-Path -Parent $Executable) -WindowStyle Hidden -PassThru
if (-not $levelMenuProcess.WaitForExit($TimeoutSeconds * 1000)) {
    Stop-Process -Id $levelMenuProcess.Id -Force
    throw "Packaged level-selection menu did not finish within $TimeoutSeconds seconds."
}
if ($levelMenuProcess.ExitCode -ne 0 -or
    -not (Test-Path -LiteralPath $levelMenuLog -PathType Leaf) -or
    -not (Test-Path -LiteralPath $levelMenuScreenshot -PathType Leaf)) {
    throw "Packaged level-selection menu smoke failed. Inspect $levelMenuLog."
}
$levelMenuLogText = Get-Content -LiteralPath $levelMenuLog -Raw
foreach ($signal in @(
    'Game menu opened: Initial=true',
    'Automated level-selection menu screenshot requested:',
    'Menu quit requested; exiting to desktop.'
)) {
    if (-not $levelMenuLogText.Contains($signal)) {
        throw "Packaged level-selection menu smoke is missing expected log signal: $signal"
    }
}
Write-Host 'Packaged level-selection menu smoke passed: initial menu rendered with deterministic quit behavior.'
Write-Host "Level-selection screenshot: $levelMenuScreenshot"

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
Write-Host 'Running six-view rendered regional smoke'
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
foreach ($view in @('CenterTown', 'WestTownExpansion', 'EastWaterfront', 'WestFarmland', 'NorthHighlands', 'SouthTropics')) {
    if (-not $regionLogText.Contains("Automated regional screenshot requested: View=$view")) {
        throw "Packaged regional capture is missing the $view view."
    }
}
foreach ($biomeSignal in @('Dominant=Town', 'Dominant=Coast', 'Dominant=Farmland', 'Dominant=Highlands', 'Dominant=Tropical')) {
    if (-not $regionLogText.Contains($biomeSignal)) {
        throw "Packaged regional capture is missing expected sampler signal: $biomeSignal"
    }
}
if (-not $regionLogText.Contains('Regional foundation ready: Seed=7319 Tiles=154 Expected=154') -or
    $regionLogText.Contains('Regional tile coverage mismatch')) {
    throw 'Packaged regional capture did not load the complete configured tile grid.'
}
if ($regionLogText.Contains('missing usage flag') -or $regionLogText.Contains('Default Material will be used in game')) {
    throw 'Packaged regional capture substituted Unreal default materials; inspect material usage flags.'
}
Write-Host 'Packaged regional smoke passed: center town, west expansion, waterfront, farmland, highlands, and tropics rendered from deterministic viewpoints.'
Write-Host "Regional log: $regionLog"
Write-Host "Regional screenshots: $regionScreenshotFolder"

$environmentArguments = @(
    '-RenderOffscreen',
    '-Windowed',
    '-ResX=1920',
    '-ResY=1080',
    '-unattended',
    '-nosplash',
    '-UEGT1SmokeResetSettings',
    "-UEGT1EnvironmentCaptureFolder=$environmentScreenshotFolder",
    "-abslog=$environmentLog"
)
Write-Host 'Running rendered day/night and full-island map smoke'
$environmentProcess = Start-Process -FilePath $Executable -ArgumentList $environmentArguments -WorkingDirectory (Split-Path -Parent $Executable) -WindowStyle Hidden -PassThru
if (-not $environmentProcess.WaitForExit($TimeoutSeconds * 1000)) {
    Stop-Process -Id $environmentProcess.Id -Force
    throw "Packaged day/night and map capture did not finish within $TimeoutSeconds seconds."
}
if ($environmentProcess.ExitCode -ne 0 -or -not (Test-Path -LiteralPath $environmentLog -PathType Leaf)) {
    throw "Packaged day/night and map capture failed with exit code $($environmentProcess.ExitCode). Inspect $environmentLog."
}
foreach ($environmentScreenshot in $environmentScreenshots) {
    if (-not (Test-Path -LiteralPath $environmentScreenshot -PathType Leaf)) {
        throw "Packaged day/night and map capture did not create $environmentScreenshot."
    }
}
$environmentHashes = $environmentScreenshots | ForEach-Object { (Get-FileHash -LiteralPath $_ -Algorithm SHA256).Hash } | Select-Object -Unique
if ($environmentHashes.Count -ne $environmentScreenshots.Count) {
    throw 'Packaged day/night and map capture produced duplicate frames; the renderer did not reflect every requested environment state.'
}
$environmentLogText = Get-Content -LiteralPath $environmentLog -Raw
foreach ($view in @('Dawn', 'Noon', 'GoldenHour', 'Midnight', 'WorldMap')) {
    if (-not $environmentLogText.Contains("Automated environment screenshot requested: View=$view")) {
        throw "Packaged environment capture is missing the $view view."
    }
}
foreach ($phase in @('Dawn', 'Day', 'Dusk', 'Night')) {
    if (-not $environmentLogText.Contains("Day/night phase: $phase")) {
        throw "Packaged environment capture is missing the $phase lighting phase."
    }
}
if (-not $environmentLogText.Contains('World map opened.')) {
    throw 'Packaged environment capture did not open the full-island world map.'
}
if ($environmentLogText.Contains('Sun=None') -or $environmentLogText.Contains('Sky=None') -or
    $environmentLogText.Contains('Fog=None') -or $environmentLogText.Contains('Exposure=None')) {
    throw 'Packaged environment capture did not resolve every authored lighting actor.'
}
Write-Host 'Packaged environment smoke passed: dawn, noon, golden hour, midnight, and the service-indexed world map rendered.'
Write-Host "Environment log: $environmentLog"
Write-Host "Environment screenshots: $environmentScreenshotFolder"

$techDemoArguments = @(
    '-RenderOffscreen',
    '-Windowed',
    '-ResX=1920',
    '-ResY=1080',
    '-unattended',
    '-nosplash',
    '-UEGT1SmokeResetSettings',
    '-UEGT1SmokeSelectTechDemo',
    '-UEGT1DevMode',
    '-UEGT1DevFlight',
    "-UEGT1TechDemoCaptureFolder=$techDemoScreenshotFolder",
    "-abslog=$techDemoLog"
)
Write-Host 'Running menu-travel, developer-mode, and three-view Lumen Wilds smoke'
$techDemoProcess = Start-Process -FilePath $Executable -ArgumentList $techDemoArguments -WorkingDirectory (Split-Path -Parent $Executable) -WindowStyle Hidden -PassThru
if (-not $techDemoProcess.WaitForExit($TimeoutSeconds * 1000)) {
    Stop-Process -Id $techDemoProcess.Id -Force
    throw "Packaged Lumen Wilds smoke did not finish within $TimeoutSeconds seconds."
}
if ($techDemoProcess.ExitCode -ne 0 -or -not (Test-Path -LiteralPath $techDemoLog -PathType Leaf)) {
    throw "Packaged Lumen Wilds smoke failed with exit code $($techDemoProcess.ExitCode). Inspect $techDemoLog."
}
foreach ($techDemoScreenshot in $techDemoScreenshots) {
    if (-not (Test-Path -LiteralPath $techDemoScreenshot -PathType Leaf)) {
        throw "Packaged Lumen Wilds smoke did not create $techDemoScreenshot."
    }
}
$techDemoLogText = Get-Content -LiteralPath $techDemoLog -Raw
foreach ($signal in @(
    'Developer mode initialized: Invincible=true FastTravel=true Flight=true',
    'Automated menu travel to Lumen Wilds scheduled.',
    'Game menu opened: Initial=true',
    'Level selected from menu: TechDemo',
    'Lumen Wilds showcase ready: TerrainVertices=16641',
    'Automated Lumen Wilds screenshot requested: View=ValleyApproach',
    'Automated Lumen Wilds screenshot requested: View=LakeOverlook',
    'Automated Lumen Wilds screenshot requested: View=CanopyFlight',
    'Automated gameplay smoke completed.'
)) {
    if (-not $techDemoLogText.Contains($signal)) {
        throw "Packaged Lumen Wilds smoke is missing expected log signal: $signal"
    }
}
if ($techDemoLogText.Contains('No authored biome tiles found') -or
    $techDemoLogText.Contains('missing usage flag') -or
    $techDemoLogText.Contains('Default Material will be used in game')) {
    throw 'Packaged Lumen Wilds smoke used a fallback world or substituted default materials.'
}
Write-Host 'Packaged Lumen Wilds smoke passed: menu travel, invincibility/fast-flight state, terrain load, and three rendered views verified.'
Write-Host "Lumen Wilds log: $techDemoLog"
Write-Host "Lumen Wilds screenshots: $techDemoScreenshotFolder"
