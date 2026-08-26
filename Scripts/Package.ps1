[CmdletBinding()]
param(
    [ValidateSet('DebugGame', 'Development', 'Shipping', 'Test')]
    [string] $Configuration = 'Development',
    [string] $EngineRoot,
    [string] $OutputDirectory
)

$ErrorActionPreference = 'Stop'
Set-StrictMode -Version Latest

$projectRoot = Split-Path -Parent $PSScriptRoot
$projectFile = Join-Path $projectRoot 'UEGT1.uproject'
$resolvedEngine = & (Join-Path $PSScriptRoot 'Resolve-Engine.ps1') -EngineRoot $EngineRoot
$automationTool = Join-Path $resolvedEngine 'Engine\Build\BatchFiles\RunUAT.bat'
if (-not $OutputDirectory) {
    $OutputDirectory = Join-Path $projectRoot "LocalBuilds\Windows-$Configuration"
}
$OutputDirectory = [IO.Path]::GetFullPath($OutputDirectory)

Write-Host "Packaging UEGT1 Win64 $Configuration to $OutputDirectory"
& $automationTool BuildCookRun "-project=$projectFile" -noP4 -utf8output -platform=Win64 "-clientconfig=$Configuration" -build -cook -stage -pak -package -archive "-archivedirectory=$OutputDirectory"
if ($LASTEXITCODE -ne 0) {
    throw "Unreal packaging failed with exit code $LASTEXITCODE."
}

$executable = Get-ChildItem -LiteralPath $OutputDirectory -Recurse -File -Filter 'UEGT1.exe' -ErrorAction SilentlyContinue | Select-Object -First 1
if (-not $executable) {
    throw "Packaging completed without producing UEGT1.exe under $OutputDirectory."
}
Write-Host "Packaged executable: $($executable.FullName)"
