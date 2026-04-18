# ── AtlasWorkspace — Generate Visual Studio Solution ────────────────────
# Usage:
#   .\Scripts\generate_vs_solution.ps1              # VS 2022 by default
#   .\Scripts\generate_vs_solution.ps1 -VSVersion 2019
#
# Output: Builds/vs20XX/AtlasWorkspace.sln

param(
    [ValidateSet("2022", "2019")]
    [string]$VSVersion = "2022"
)

$ErrorActionPreference = "Stop"

$preset = "vs$VSVersion"

Write-Host ""
Write-Host "══════════════════════════════════════════════════════════" -ForegroundColor Cyan
Write-Host "  AtlasWorkspace — Generating VS $VSVersion Solution" -ForegroundColor Cyan
Write-Host "══════════════════════════════════════════════════════════" -ForegroundColor Cyan
Write-Host ""

Push-Location (Split-Path $PSScriptRoot)

try {
    cmake --preset $preset
    if ($LASTEXITCODE -ne 0) {
        throw "CMake configure failed with exit code $LASTEXITCODE"
    }

    $slnPath = Join-Path "Builds" $preset "AtlasWorkspace.sln"
    Write-Host ""
    Write-Host "══════════════════════════════════════════════════════════" -ForegroundColor Green
    Write-Host "  Solution generated successfully!" -ForegroundColor Green
    Write-Host "  Open: $slnPath" -ForegroundColor Green
    Write-Host "══════════════════════════════════════════════════════════" -ForegroundColor Green
    Write-Host ""
}
finally {
    Pop-Location
}
