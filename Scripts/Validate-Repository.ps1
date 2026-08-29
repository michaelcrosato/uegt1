[CmdletBinding()]
param()

$ErrorActionPreference = 'Stop'
Set-StrictMode -Version Latest

$projectRoot = Split-Path -Parent $PSScriptRoot
$requiredFiles = @(
    '.gitignore',
    '.gitattributes',
    '.vsconfig',
    'UEGT1.uproject',
    'Config\DefaultEngine.ini',
    'Config\DefaultGameUserSettings.ini',
    'Config\DefaultInput.ini',
    'Content\Maps\Main.umap',
    'Content\Maps\TechDemo.umap',
    'Content\ArchVis\SampleScene\Tree\HillTree_02.uasset',
    'Content\ArchVis\SampleScene\Tree\Materials\M_HillTree_01_Leaves.uasset',
    'Content\ArchVis\SampleScene\Tree\Textures\T_HillTree_01_Atlas.uasset',
    'Content\SimSandbox\Meshes\Rock.uasset',
    'Content\SimSandbox\Materials\MI_PrototypeGrid_TopBlue.uasset',
    'Content\Materials\M_SignalGlow.uasset',
    'Content\Materials\M_SignalSurface.uasset',
    'Content\Materials\M_SignalWater.uasset',
    'Content\Materials\M_TechFoliage.uasset',
    'Content\Materials\M_TechSurface.uasset',
    'Content\Materials\M_TechTerrain.uasset',
    'Content\Materials\M_TechWater.uasset',
    'Docs\Architecture.md',
    'Docs\Lumen-Wilds.md',
    'Docs\Playtest-0.1.md',
    'Docs\Region-Authoring.md',
	'Docs\Town-Simulation.md',
    'Source\UEGT1.Target.cs',
    'Source\UEGT1Editor.Target.cs',
    'Source\UEGT1\UEGT1.Build.cs',
    'Source\UEGT1\Public\Settings\UEGT1GameUserSettings.h',
	'Source\UEGT1\Public\Simulation\UEGT1TownSimulationModel.h',
	'Source\UEGT1\Public\Simulation\UEGT1TownSimulationSettings.h',
	'Source\UEGT1\Public\Simulation\UEGT1TownSimulationSubsystem.h',
	'Source\UEGT1\Public\Simulation\UEGT1TownSimulationTypes.h',
    'Source\UEGT1\Public\Development\UEGT1DeveloperModeSubsystem.h',
    'Source\UEGT1\Public\Gameplay\UEGT1SessionSubsystem.h',
    'Source\UEGT1\Public\World\UEGT1TechDemoEnvironment.h',
    'Source\UEGT1\Public\World\UEGT1RegionSettings.h',
    'Source\UEGT1\Public\World\UEGT1RegionTypes.h',
    'Source\UEGT1\Public\World\UEGT1Town.h',
    'Source\UEGT1\Public\World\UEGT1VisualMaterials.h',
    'Source\UEGT1\Private\World\UEGT1RegionSettings.cpp',
    'Source\UEGT1\Private\World\UEGT1RegionTypes.cpp',
    'Source\UEGT1\Private\World\UEGT1Town.cpp',
    'Source\UEGT1\Private\World\UEGT1VisualMaterials.cpp',
    'Source\UEGT1\Private\Development\UEGT1DeveloperModeSubsystem.cpp',
    'Source\UEGT1\Private\World\UEGT1TechDemoEnvironment.cpp',
    'Source\UEGT1\Private\World\UEGT1WorldLayout.cpp',
    'Source\UEGT1\Private\Settings\UEGT1GameUserSettings.cpp',
	'Source\UEGT1\Private\Simulation\UEGT1TownSimulationModel.cpp',
	'Source\UEGT1\Private\Simulation\UEGT1TownSimulationSubsystem.cpp',
	'Source\UEGT1\Private\Tests\UEGT1TownSimulationTest.cpp',
    'Source\UEGT1\Private\UI\SUEGT1Menu.h',
    'Source\UEGT1\Private\UI\SUEGT1Menu.cpp',
    'Scripts\Build.ps1',
    'Scripts\Create-TechDemoContent.ps1',
    'Scripts\CreateTechDemoContent.py',
    'Scripts\Package.ps1',
    'Scripts\Smoke-Gameplay.ps1',
    'Scripts\Smoke-PackagedBuild.ps1',
    'Scripts\Test.ps1'
)

$errors = [System.Collections.Generic.List[string]]::new()
foreach ($relativePath in $requiredFiles) {
    if (-not (Test-Path -LiteralPath (Join-Path $projectRoot $relativePath) -PathType Leaf)) {
        $errors.Add("Missing required file: $relativePath")
    }
}

$project = Get-Content -LiteralPath (Join-Path $projectRoot 'UEGT1.uproject') -Raw | ConvertFrom-Json
if ($project.EngineAssociation -ne '5.8') {
    $errors.Add("UEGT1.uproject must target Unreal Engine 5.8; found '$($project.EngineAssociation)'.")
}
if ('UEGT1' -notin $project.Modules.Name) {
    $errors.Add('UEGT1.uproject does not declare the UEGT1 runtime module.')
}
if ('ProceduralMeshComponent' -notin $project.Plugins.Name) {
    $errors.Add('UEGT1.uproject must enable ProceduralMeshComponent for the Lumen Wilds collision terrain.')
}

$engineConfig = Get-Content -LiteralPath (Join-Path $projectRoot 'Config\DefaultEngine.ini') -Raw
if ($engineConfig -notmatch '(?m)^DefaultGraphicsRHI=DefaultGraphicsRHI_DX12\r?$') {
    $errors.Add('DefaultEngine.ini does not make DirectX 12 the default Windows RHI.')
}
if ($engineConfig -notmatch '(?m)^GameDefaultMap=/Game/Maps/Main\r?$') {
    $errors.Add('DefaultEngine.ini does not use /Game/Maps/Main as the game map.')
}
if ($engineConfig -notmatch '(?m)^GameUserSettingsClassName=/Script/UEGT1\.UEGT1GameUserSettings\r?$') {
    $errors.Add('DefaultEngine.ini does not select the persistent UEGT1 graphics settings class.')
}
if ($engineConfig -match '(?im)^\s*SecurityToken\s*=\s*\S+') {
    $errors.Add('DefaultEngine.ini contains a non-empty security token.')
}

$inputConfig = Get-Content -LiteralPath (Join-Path $projectRoot 'Config\DefaultInput.ini') -Raw
if ($inputConfig -notmatch '(?m)^DefaultPlayerInputClass=/Script/EnhancedInput\.EnhancedPlayerInput\r?$' -or
    $inputConfig -notmatch '(?m)^DefaultInputComponentClass=/Script/EnhancedInput\.EnhancedInputComponent\r?$') {
    $errors.Add('DefaultInput.ini must keep Enhanced Input enabled for the legacy mapping bridge used by the C++ bindings.')
}
if ($inputConfig -notmatch 'ActionName="PauseMenu"[^\r\n]*Key=Escape' -or
    $inputConfig -notmatch 'ActionName="PauseMenu"[^\r\n]*Key=Gamepad_Special_Right') {
    $errors.Add('DefaultInput.ini must bind the game menu to Escape and the gamepad Menu/Start button.')
}
if ($inputConfig -notmatch 'ActionName="ToggleDeveloperMode"[^\r\n]*Key=F8' -or
    $inputConfig -notmatch 'ActionName="ToggleDeveloperFlight"[^\r\n]*Key=F9' -or
    $inputConfig -notmatch 'ActionName="DeveloperDescend"[^\r\n]*Key=LeftControl') {
    $errors.Add('DefaultInput.ini must bind the complete F8/F9 developer movement controls.')
}
if ($inputConfig -notmatch 'ActionName="CycleSimulationInspector"[^\r\n]*Key=F4' -or
    $inputConfig -notmatch 'ActionName="SaveTownSimulation"[^\r\n]*Key=F5' -or
    $inputConfig -notmatch 'ActionName="LoadTownSimulation"[^\r\n]*Key=F6') {
    $errors.Add('DefaultInput.ini must bind the F4 inspector and F5/F6 simulation persistence controls.')
}

$settingsConfig = Get-Content -LiteralPath (Join-Path $projectRoot 'Config\DefaultGameUserSettings.ini') -Raw
if ($settingsConfig -notmatch '(?m)^\[/Script/UEGT1\.UEGT1GameUserSettings\]\r?$' -or
    $settingsConfig -notmatch '(?m)^ResolutionSizeX=1920\r?$' -or
    $settingsConfig -notmatch '(?m)^bFoliageEnabled=True\r?$') {
    $errors.Add('DefaultGameUserSettings.ini does not define the recommended persistent UEGT1 profile.')
}

$gameConfig = Get-Content -LiteralPath (Join-Path $projectRoot 'Config\DefaultGame.ini') -Raw
if ($gameConfig -notmatch '(?m)^\[/Script/UEGT1\.UEGT1RegionSettings\]\r?$' -or
    $gameConfig -notmatch '(?m)^TileRadius=5\r?$' -or
	$gameConfig -notmatch '(?m)^WestTileExtension=3\r?$' -or
	$gameConfig -notmatch '(?m)^TownWestExtension=13000\.000000\r?$' -or
    $gameConfig -notmatch '(?m)^CoastStart=3600\.000000\r?$' -or
    $gameConfig -notmatch '(?m)^MountainMaxElevation=2600\.000000\r?$') {
    $errors.Add('DefaultGame.ini does not define the v0.5 regional generation profile.')
}
if ($gameConfig -notmatch '(?m)^\+DirectoriesToAlwaysCook=\(Path="/Game/Materials"\)\r?$') {
    $errors.Add('DefaultGame.ini must cook the authored Signal Grove materials used by runtime-generated instances.')
}
if ($gameConfig -notmatch '(?m)^\[/Script/UEGT1\.UEGT1TownSimulationSettings\]\r?$' -or
    $gameConfig -notmatch '(?m)^NPCCount=100\r?$' -or
	$gameConfig -notmatch '(?m)^TownSeed=7319\r?$' -or
	$gameConfig -notmatch 'CitizenWalkSpeedCentimetersPerSecond=440\.000000' -or
	$gameConfig -notmatch '(?m)^\+ActionDefinitions=\(Action=Eat,VenueType=Home,[^\r\n]*Cost=1\.000000' -or
	$gameConfig -notmatch '(?m)^\+ActionDefinitions=\(Action=Work,[^\r\n]*DurationMinutes=60\.000000[^\r\n]*Earnings=0\.000000' -or
	([regex]::Matches($gameConfig, '(?m)^\+JobDefinitions=')).Count -ne 20) {
	$errors.Add('DefaultGame.ini does not define the town population, $1 groceries, hourly work, and twenty-job catalog.')
}
foreach ($map in @('Main', 'TechDemo')) {
    $mapPattern = '(?m)^\+MapsToCook=\(FilePath="/Game/Maps/{0}"\)\r?$' -f $map
    if ($gameConfig -notmatch $mapPattern) {
        $errors.Add("DefaultGame.ini must explicitly cook /Game/Maps/$map for menu travel.")
    }
}

$externalActorFolder = Join-Path $projectRoot 'Content\__ExternalActors__\Maps\Main'
$externalActorCount = if (Test-Path -LiteralPath $externalActorFolder -PathType Container) {
    (Get-ChildItem -LiteralPath $externalActorFolder -Recurse -File -Filter '*.uasset').Count
} else {
    0
}
if ($externalActorCount -lt 160) {
	$errors.Add("The Main World Partition is incomplete; expected at least 160 west-expanded regional external actor packages, found $externalActorCount.")
}

if ($errors.Count -gt 0) {
    $errors | ForEach-Object { Write-Error $_ }
    exit 1
}

Write-Host 'Repository structure and project configuration are valid.'
