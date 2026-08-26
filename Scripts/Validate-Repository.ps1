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
    'Docs\Architecture.md',
    'Docs\Playtest-0.1.md',
    'Docs\Region-Authoring.md',
    'Source\UEGT1.Target.cs',
    'Source\UEGT1Editor.Target.cs',
    'Source\UEGT1\UEGT1.Build.cs',
    'Source\UEGT1\Public\Settings\UEGT1GameUserSettings.h',
    'Source\UEGT1\Public\World\UEGT1RegionSettings.h',
    'Source\UEGT1\Public\World\UEGT1RegionTypes.h',
    'Source\UEGT1\Public\World\UEGT1Town.h',
    'Source\UEGT1\Private\World\UEGT1RegionSettings.cpp',
    'Source\UEGT1\Private\World\UEGT1RegionTypes.cpp',
    'Source\UEGT1\Private\World\UEGT1Town.cpp',
    'Source\UEGT1\Private\World\UEGT1WorldLayout.cpp',
    'Source\UEGT1\Private\Settings\UEGT1GameUserSettings.cpp',
    'Source\UEGT1\Private\UI\SUEGT1Menu.h',
    'Source\UEGT1\Private\UI\SUEGT1Menu.cpp',
    'Scripts\Build.ps1',
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

$settingsConfig = Get-Content -LiteralPath (Join-Path $projectRoot 'Config\DefaultGameUserSettings.ini') -Raw
if ($settingsConfig -notmatch '(?m)^\[/Script/UEGT1\.UEGT1GameUserSettings\]\r?$' -or
    $settingsConfig -notmatch '(?m)^ResolutionSizeX=1920\r?$' -or
    $settingsConfig -notmatch '(?m)^bFoliageEnabled=True\r?$') {
    $errors.Add('DefaultGameUserSettings.ini does not define the recommended persistent UEGT1 profile.')
}

$gameConfig = Get-Content -LiteralPath (Join-Path $projectRoot 'Config\DefaultGame.ini') -Raw
if ($gameConfig -notmatch '(?m)^\[/Script/UEGT1\.UEGT1RegionSettings\]\r?$' -or
    $gameConfig -notmatch '(?m)^TileRadius=5\r?$' -or
    $gameConfig -notmatch '(?m)^CoastStart=3600\.000000\r?$' -or
    $gameConfig -notmatch '(?m)^MountainMaxElevation=2600\.000000\r?$') {
    $errors.Add('DefaultGame.ini does not define the v0.3 regional generation profile.')
}

$externalActorFolder = Join-Path $projectRoot 'Content\__ExternalActors__\Maps\Main'
$externalActorCount = if (Test-Path -LiteralPath $externalActorFolder -PathType Container) {
    (Get-ChildItem -LiteralPath $externalActorFolder -Recurse -File -Filter '*.uasset').Count
} else {
    0
}
if ($externalActorCount -lt 130) {
    $errors.Add("The Main World Partition is incomplete; expected at least 130 regional external actor packages, found $externalActorCount.")
}

if ($errors.Count -gt 0) {
    $errors | ForEach-Object { Write-Error $_ }
    exit 1
}

Write-Host 'Repository structure and project configuration are valid.'
