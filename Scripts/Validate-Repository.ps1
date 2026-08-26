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
    'Source\UEGT1.Target.cs',
    'Source\UEGT1Editor.Target.cs',
    'Source\UEGT1\UEGT1.Build.cs',
    'Scripts\Build.ps1',
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
if ($engineConfig -notmatch '(?m)^DefaultGraphicsRHI=DefaultGraphicsRHI_DX12$') {
    $errors.Add('DefaultEngine.ini does not make DirectX 12 the default Windows RHI.')
}
if ($engineConfig -notmatch '(?m)^GameDefaultMap=/Game/Maps/Main$') {
    $errors.Add('DefaultEngine.ini does not use /Game/Maps/Main as the game map.')
}

if ($errors.Count -gt 0) {
    $errors | ForEach-Object { Write-Error $_ }
    exit 1
}

Write-Host 'Repository structure and project configuration are valid.'

