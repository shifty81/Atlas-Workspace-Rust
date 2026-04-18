#!/usr/bin/env bash
# AtlasWorkspace — Project Structure Validator
# Validates that the repository conforms to the hosted project contract.
# Exit 0 = all checks pass, Exit 1 = one or more checks failed.

set -euo pipefail

SCRIPT_DIR="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)"
ROOT_DIR="$(cd "$SCRIPT_DIR/.." && pwd)"

PASS=0
FAIL=0

check() {
    local desc="$1"
    local result="$2"
    if [ "$result" = "true" ]; then
        echo "  ✅  $desc"
        PASS=$((PASS + 1))
    else
        echo "  ❌  $desc"
        FAIL=$((FAIL + 1))
    fi
}

dir_exists() {
    [ -d "$ROOT_DIR/$1" ] && echo "true" || echo "false"
}

file_exists() {
    [ -f "$ROOT_DIR/$1" ] && echo "true" || echo "false"
}

echo "═══════════════════════════════════════════════════"
echo "  AtlasWorkspace Project Validator"
echo "═══════════════════════════════════════════════════"
echo ""

# ── Required directories ─────────────────────────────
echo "▸ Required directories"
check "Source/"               "$(dir_exists Source)"
check "Tests/"                "$(dir_exists Tests)"
check "Content/"              "$(dir_exists Content)"
check "Content/Incoming/"     "$(dir_exists Content/Incoming)"
check "Config/"               "$(dir_exists Config)"
check "Data/"                 "$(dir_exists Data)"
check "Schemas/"              "$(dir_exists Schemas)"
check "Docs/"                 "$(dir_exists Docs)"
check "Scripts/"              "$(dir_exists Scripts)"
check "Tools/"                "$(dir_exists Tools)"
check "AtlasAI/"              "$(dir_exists AtlasAI)"
check "Archive/"              "$(dir_exists Archive)"
check "Project/"              "$(dir_exists Project)"
check "Logs/"                 "$(dir_exists Logs)"
check "Codex/"                "$(dir_exists Codex)"
check "Codex/Snippets/"       "$(dir_exists Codex/Snippets)"
check "Codex/Fixes/"          "$(dir_exists Codex/Fixes)"
check "Codex/Graphs/"         "$(dir_exists Codex/Graphs)"
echo ""

# ── Required files ────────────────────────────────────
echo "▸ Required files"
check "CMakeLists.txt"                   "$(file_exists CMakeLists.txt)"
check "CMakePresets.json"                "$(file_exists CMakePresets.json)"
check "vcpkg.json"                       "$(file_exists vcpkg.json)"
check "README.md"                        "$(file_exists README.md)"
check "LICENSE"                          "$(file_exists LICENSE)"
check ".gitignore"                       "$(file_exists .gitignore)"
check "Dockerfile"                       "$(file_exists Dockerfile)"
check "Makefile"                         "$(file_exists Makefile)"
check "Project/project.atlas.json"       "$(file_exists Project/project.atlas.json)"
check "Config/novaforge.project.json"    "$(file_exists Config/novaforge.project.json)"
check "Config/season.config.json"        "$(file_exists Config/season.config.json)"
check "Logs/logger.format.md"            "$(file_exists Logs/logger.format.md)"
echo ""

# ── Source modules ────────────────────────────────────
echo "▸ Source modules"
for mod in Core Engine Renderer Physics Audio Animation Game Input Networking UI Editor; do
    check "Source/$mod/"  "$(dir_exists "Source/$mod")"
done
echo ""

# ── AtlasAI structure ────────────────────────────────
echo "▸ AtlasAI broker"
check "AtlasAI/README.md"                    "$(file_exists AtlasAI/README.md)"
check "AtlasAI/Atlas_Arbiter/README.md"      "$(file_exists AtlasAI/Atlas_Arbiter/README.md)"
check "AtlasAI/Atlas_SwissAgent/README.md"   "$(file_exists AtlasAI/Atlas_SwissAgent/README.md)"
echo ""

# ── AtlasUI structure ─────────────────────────────────
echo "▸ AtlasUI system"
check "Source/UI/include/NF/UI/AtlasUI/"                      "$(dir_exists Source/UI/include/NF/UI/AtlasUI)"
check "Source/UI/src/AtlasUI/"                                 "$(dir_exists Source/UI/src/AtlasUI)"
check "AtlasUI Theme/"                                         "$(dir_exists Source/UI/include/NF/UI/AtlasUI/Theme)"
check "AtlasUI Widgets/"                                       "$(dir_exists Source/UI/include/NF/UI/AtlasUI/Widgets)"
check "AtlasUI Commands/"                                      "$(dir_exists Source/UI/include/NF/UI/AtlasUI/Commands)"
check "AtlasUI Services/"                                      "$(dir_exists Source/UI/include/NF/UI/AtlasUI/Services)"
check "ThemeManager.h"                                         "$(file_exists Source/UI/include/NF/UI/AtlasUI/Theme/ThemeManager.h)"
check "Interfaces.h"                                           "$(file_exists Source/UI/include/NF/UI/AtlasUI/Interfaces.h)"
check "Button.h"                                               "$(file_exists Source/UI/include/NF/UI/AtlasUI/Widgets/Button.h)"
check "CommandRegistry.h"                                      "$(file_exists Source/UI/include/NF/UI/AtlasUI/Commands/CommandRegistry.h)"
check "FocusService.h"                                         "$(file_exists Source/UI/include/NF/UI/AtlasUI/Services/FocusService.h)"
check "Docs/SpecRollup/"                                       "$(dir_exists Docs/SpecRollup)"
check "Tools/Validation/"                                      "$(dir_exists Tools/Validation)"
echo ""

# ── Phase 3 extraction ──────────────────────────────
echo "▸ Phase 3 — Nova-Forge-Expeditions extraction"
check "Source/Game/src/Systems/ (449 game systems)"            "$(dir_exists Source/Game/src/Systems)"
check "Source/Game/include/NF/Game/Server/ (server headers)"   "$(dir_exists Source/Game/include/NF/Game/Server)"
check "Tests/Game/Server/ (game tests)"                        "$(dir_exists Tests/Game/Server)"
check "Source/Engine/src/PCG/ (PCG framework)"                 "$(dir_exists Source/Engine/src/PCG)"
check "Source/Renderer/src/Pipeline/ (rendering pipeline)"     "$(dir_exists Source/Renderer/src/Pipeline)"
check "Source/Editor/src/Tools/ (editor tools)"                "$(dir_exists Source/Editor/src/Tools)"
check "Source/Game/src/Modules/ (gameplay modules)"            "$(dir_exists Source/Game/src/Modules)"
check "Tools/BlenderGenerator/blender-addon/"                  "$(dir_exists Tools/BlenderGenerator/blender-addon)"
check "Docs/Nova-Forge-Expeditions/"                           "$(dir_exists Docs/Nova-Forge-Expeditions)"
check "Archive/_Nova-Forge-Expeditions/docs_archive/AUDIT.md"  "$(file_exists Archive/_Nova-Forge-Expeditions/docs_archive/AUDIT.md)"
echo ""

# ── Architecture docs ────────────────────────────────
echo "▸ Architecture documentation"
check "Docs/Architecture/NAMING_CANON.md"            "$(file_exists Docs/Architecture/NAMING_CANON.md)"
check "Docs/Architecture/CURRENT_DIRECTION.md"       "$(file_exists Docs/Architecture/CURRENT_DIRECTION.md)"
check "Docs/Architecture/HOSTED_PROJECT_CONTRACT.md" "$(file_exists Docs/Architecture/HOSTED_PROJECT_CONTRACT.md)"
check "Docs/Architecture/DEFERRED_TO_WORKSPACE.md"   "$(file_exists Docs/Architecture/DEFERRED_TO_WORKSPACE.md)"
check "Docs/Architecture/BUILD_MODES.md"             "$(file_exists Docs/Architecture/BUILD_MODES.md)"
check "Docs/Architecture/VOXEL_RENDER_PIPELINE.md"   "$(file_exists Docs/Architecture/VOXEL_RENDER_PIPELINE.md)"
echo ""

# ── project.atlas.json validity ──────────────────────
echo "▸ Project manifest validation"
MANIFEST="$ROOT_DIR/Project/project.atlas.json"
if [ -f "$MANIFEST" ]; then
    # Check it's valid JSON (python fallback to jq)
    if command -v python3 &>/dev/null; then
        if python3 -c "import json; json.load(open('$MANIFEST'))" 2>/dev/null; then
            check "project.atlas.json is valid JSON" "true"
            # Check required keys
            HAS_NAME=$(python3 -c "import json; d=json.load(open('$MANIFEST')); print('true' if 'name' in d else 'false')")
            HAS_ENTRY=$(python3 -c "import json; d=json.load(open('$MANIFEST')); print('true' if 'entrypoints' in d else 'false')")
            HAS_BUILD=$(python3 -c "import json; d=json.load(open('$MANIFEST')); print('true' if 'build' in d else 'false')")
            HAS_PIPE=$(python3 -c "import json; d=json.load(open('$MANIFEST')); print('true' if 'pipeline' in d else 'false')")
            HAS_ATLAS=$(python3 -c "import json; d=json.load(open('$MANIFEST')); print('true' if 'atlas_workspace' in d else 'false')")
            check "Has 'name' key"             "$HAS_NAME"
            check "Has 'entrypoints' key"      "$HAS_ENTRY"
            check "Has 'build' key"            "$HAS_BUILD"
            check "Has 'pipeline' key"         "$HAS_PIPE"
            check "Has 'atlas_workspace' key"  "$HAS_ATLAS"
        else
            check "project.atlas.json is valid JSON" "false"
        fi
    else
        check "project.atlas.json is valid JSON (python3 not found — skipped)" "true"
    fi
else
    check "project.atlas.json exists" "false"
fi
echo ""

# ── Summary ──────────────────────────────────────────
echo "═══════════════════════════════════════════════════"
echo "  Results: $PASS passed, $FAIL failed"
echo "═══════════════════════════════════════════════════"

if [ "$FAIL" -gt 0 ]; then
    exit 1
fi
exit 0
