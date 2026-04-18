$ErrorActionPreference = "Stop"

$required = @(
    "novaforge.project.json",
    "README.md",
    "PROJECT_RULES.md",
    "BUILD_RULES.md",
    "ARCHITECTURE.md",
    "Docs/Specs/HOSTED_PROJECT_CONTRACT.md",
    "NovaForge/CMakeLists.txt",
    "NovaForge/Client",
    "NovaForge/Server",
    "NovaForge/Gameplay",
    "NovaForge/World",
    "Integrations/AtlasSuite/Runtime",
    "Integrations/AtlasSuite/Adapter",
    "AtlasAI/ProjectAdapters/NovaForge",
    "Tests/NovaForge.Tests"
)

$missing = @()
foreach ($item in $required) {
    if (-not (Test-Path $item)) {
        $missing += $item
    }
}

if ($missing.Count -gt 0) {
    Write-Host "Validation failed. Missing required items:"
    $missing | ForEach-Object { Write-Host " - $_" }
    exit 1
}

Write-Host "Validation passed."
exit 0
