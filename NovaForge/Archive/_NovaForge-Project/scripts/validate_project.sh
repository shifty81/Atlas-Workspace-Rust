#!/usr/bin/env bash
# validate_project.sh — Validates the NovaForge project structure.
# Bash equivalent of Scripts/validate_project.ps1.
set -euo pipefail

REPO_ROOT="$(cd "$(dirname "$0")/.." && pwd)"
cd "${REPO_ROOT}"

REQUIRED=(
    "novaforge.project.json"
    "README.md"
    "PROJECT_RULES.md"
    "BUILD_RULES.md"
    "ARCHITECTURE.md"
    "Docs/Specs/HOSTED_PROJECT_CONTRACT.md"
    "NovaForge/CMakeLists.txt"
    "NovaForge/Client"
    "NovaForge/Server"
    "NovaForge/Gameplay"
    "NovaForge/World"
    "Integrations/AtlasSuite/Runtime"
    "Integrations/AtlasSuite/Adapter"
    "AtlasAI/ProjectAdapters/NovaForge"
    "Tests/NovaForge.Tests"
)

FAILED=0
MISSING=()

for item in "${REQUIRED[@]}"; do
    if [ ! -e "${item}" ]; then
        MISSING+=("${item}")
        FAILED=1
    fi
done

if [ "${FAILED}" -ne 0 ]; then
    echo "Validation failed. Missing required items:"
    for m in "${MISSING[@]}"; do
        echo "  - ${m}"
    done
    exit 1
fi

echo "Validation passed."
exit 0
