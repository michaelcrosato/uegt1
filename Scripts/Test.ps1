[CmdletBinding()]
param(
    [string] $EngineRoot,
    [switch] $SkipBuild
)

$ErrorActionPreference = 'Stop'
Set-StrictMode -Version Latest

$projectRoot = Split-Path -Parent $PSScriptRoot
$projectFile = Join-Path $projectRoot 'UEGT1.uproject'
$resolvedEngine = & (Join-Path $PSScriptRoot 'Resolve-Engine.ps1') -EngineRoot $EngineRoot

if (-not $SkipBuild) {
    & (Join-Path $PSScriptRoot 'Build.ps1') -EngineRoot $resolvedEngine
}

$editorCommand = Join-Path $resolvedEngine 'Engine\Binaries\Win64\UnrealEditor-Cmd.exe'
$reportFolder = Join-Path $projectRoot 'Saved\TestReports'
New-Item -ItemType Directory -Path $reportFolder -Force | Out-Null

Write-Host 'Running UEGT1 automation tests headlessly'
& $editorCommand $projectFile -unattended -nop4 -nosplash -NullRHI '-ExecCmds=Automation RunTests UEGT1; Quit' '-TestExit=Automation Test Queue Empty' "-ReportExportPath=$reportFolder" -log
if ($LASTEXITCODE -ne 0) {
    throw "Unreal automation tests failed with exit code $LASTEXITCODE. See Saved\Logs and Saved\TestReports."
}

$report = Join-Path $reportFolder 'index.json'
if (-not (Test-Path -LiteralPath $report -PathType Leaf)) {
    throw 'Unreal exited successfully but did not create the expected automation report.'
}
$results = Get-Content -LiteralPath $report -Raw | ConvertFrom-Json
if (($results.failed -gt 0) -or ($results.notRun -gt 0) -or ($results.inProcess -gt 0) -or ($results.succeeded -lt 1)) {
    throw "Automation report is not clean: succeeded=$($results.succeeded), failed=$($results.failed), notRun=$($results.notRun), inProcess=$($results.inProcess)."
}
Write-Host "Automation passed: $($results.succeeded) succeeded, $($results.succeededWithWarnings) with warnings, $($results.failed) failed."
Write-Host "Automation report: $report"
