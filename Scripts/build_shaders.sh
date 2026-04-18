#!/usr/bin/env bash
# ╔══════════════════════════════════════════════════════════════════╗
# ║  Atlas Workspace — SPIR-V Shader Compiler                       ║
# ║  Compiles .vert/.frag/.comp under crates/atlas-renderer/shaders/ ║
# ╚══════════════════════════════════════════════════════════════════╝
set -euo pipefail

SCRIPT_DIR="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)"
ROOT_DIR="$(cd "$SCRIPT_DIR/.." && pwd)"

# ── Colors ────────────────────────────────────────────────────────
RED='\033[0;31m'
GREEN='\033[0;32m'
YELLOW='\033[1;33m'
CYAN='\033[0;36m'
BOLD='\033[1m'
RESET='\033[0m'
CHECK='✓'
CROSS='✗'
ARROW='→'

SHADER_SRC="$ROOT_DIR/crates/atlas-renderer/shaders"
SHADER_OUT="$ROOT_DIR/target/shaders"
LOG_DIR="$ROOT_DIR/Logs"
LOG_FILE="$LOG_DIR/shader_build.log"

mkdir -p "$SHADER_SRC"
mkdir -p "$SHADER_OUT"
mkdir -p "$LOG_DIR"

echo "=== Atlas Workspace Shader Build Log ===" > "$LOG_FILE"
echo "Started: $(date '+%Y-%m-%d %H:%M:%S')" >> "$LOG_FILE"
echo "" >> "$LOG_FILE"

echo ""
echo -e "${BOLD}${CYAN}╔══════════════════════════════════════════════════════════╗${RESET}"
echo -e "${BOLD}${CYAN}║${RESET}  ${BOLD}Atlas Workspace — SPIR-V Shader Compiler${RESET}                 ${BOLD}${CYAN}║${RESET}"
echo -e "${BOLD}${CYAN}╚══════════════════════════════════════════════════════════╝${RESET}"
echo ""
echo -e "  Source: ${CYAN}$SHADER_SRC${RESET}"
echo -e "  Output: ${CYAN}$SHADER_OUT${RESET}"
echo ""

# ── Check for glslc ──────────────────────────────────────────────
if ! command -v glslc &>/dev/null; then
    echo -e "${YELLOW}⚠  glslc not found.${RESET}"
    echo ""
    echo "  To install glslc, install the Vulkan SDK:"
    echo "    Linux:   sudo apt-get install glslang-tools"
    echo "    macOS:   brew install glslang"
    echo "    Windows: https://vulkan.lunarg.com/sdk/home"
    echo ""
    echo "  Alternatively, install the full Vulkan SDK:"
    echo "    https://vulkan.lunarg.com/sdk/home"
    echo ""
    echo -e "${YELLOW}  Shader compilation skipped.${RESET}"
    echo "glslc not found — skipping" >> "$LOG_FILE"
    exit 0
fi

echo -e "  glslc: $(glslc --version 2>&1 | head -1)"
echo ""

# ── Compile shaders ───────────────────────────────────────────────
COMPILED=0
FAILED=0

while IFS= read -r -d '' shader; do
    rel="${shader#$SHADER_SRC/}"
    outfile="$SHADER_OUT/${rel//\//_}.spv"
    echo -e "  ${CYAN}[$ARROW]${RESET} Compiling: $rel"
    echo "Compiling: $rel -> $outfile" >> "$LOG_FILE"

    if glslc "$shader" -o "$outfile" 2>&1 | tee -a "$LOG_FILE"; then
        echo -e "    ${GREEN}${CHECK} -> $(basename "$outfile")${RESET}"
        COMPILED=$((COMPILED + 1))
    else
        echo -e "    ${RED}${CROSS} FAILED${RESET}"
        FAILED=$((FAILED + 1))
    fi
done < <(find "$SHADER_SRC" -type f \( -name "*.vert" -o -name "*.frag" -o -name "*.comp" \) -print0 2>/dev/null)

echo ""
if [[ $COMPILED -eq 0 && $FAILED -eq 0 ]]; then
    echo -e "  ${YELLOW}No shader files found in $SHADER_SRC${RESET}"
    echo "  Create .vert/.frag/.comp files in crates/atlas-renderer/shaders/"
else
    echo -e "  Compiled: ${GREEN}$COMPILED${RESET}  Failed: ${RED}$FAILED${RESET}"
fi
echo ""
echo "  Log: $LOG_FILE"
echo ""

echo "Compiled: $COMPILED" >> "$LOG_FILE"
echo "Failed: $FAILED" >> "$LOG_FILE"
echo "Finished: $(date '+%Y-%m-%d %H:%M:%S')" >> "$LOG_FILE"

[[ $FAILED -gt 0 ]] && exit 1
exit 0
