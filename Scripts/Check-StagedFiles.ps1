[CmdletBinding()]
param()

$ErrorActionPreference = 'Stop'
Set-StrictMode -Version Latest

$blockedDirectoryPattern = '(^|/)(Binaries|DerivedDataCache|Intermediate|Saved|\.vs)(/|$)'
$credentialNamePattern = '(^|/)(\.env($|\.)|.*\.(pfx|p12|pem|key)$)'
$maxFileBytes = 100MB
$failures = [System.Collections.Generic.List[string]]::new()
$stagedFiles = @(git diff --cached --name-only --diff-filter=ACMR)

foreach ($relativePath in $stagedFiles) {
    $normalizedPath = $relativePath -replace '\\', '/'
    if ($normalizedPath -match $blockedDirectoryPattern) {
        $failures.Add("Generated path must not be committed: $normalizedPath")
    }
    if ($normalizedPath -match $credentialNamePattern) {
        $failures.Add("Potential credential file must not be committed: $normalizedPath")
    }

    $fullPath = Join-Path (git rev-parse --show-toplevel) $relativePath
    if ((Test-Path -LiteralPath $fullPath -PathType Leaf) -and (Get-Item -LiteralPath $fullPath).Length -gt $maxFileBytes) {
        $failures.Add("File exceeds the 100 MiB GitHub limit: $normalizedPath")
    }
}

if ($failures.Count -gt 0) {
    $failures | ForEach-Object { Write-Error $_ }
    exit 1
}

& (Join-Path (git rev-parse --show-toplevel) 'Scripts\Validate-Repository.ps1')

