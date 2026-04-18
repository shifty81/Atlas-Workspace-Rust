#!/usr/bin/env bash
# ╔══════════════════════════════════════════════════════════════════╗
# ║  AtlasWorkspace — Build System with Progress Bars & Log Mirroring    ║
# ║  All output is shown on screen AND mirrored to Logs/build.log   ║
# ╚══════════════════════════════════════════════════════════════════╝
set -euo pipefail

SCRIPT_DIR="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)"
ROOT_DIR="$(cd "$SCRIPT_DIR/.." && pwd)"

# ── Arguments ─────────────────────────────────────────────────────
BUILD_TYPE="${1:-Debug}"
RUN_TESTS=false
TARGET=""
shift || true
while [[ $# -gt 0 ]]; do
    case "$1" in
        --test)     RUN_TESTS=true ;;
        --target)   shift; TARGET="$1" ;;
        --editor)   TARGET="AtlasWorkspaceEditor" ;;
        --game)     TARGET="AtlasWorkspaceGame" ;;
        --server)   TARGET="AtlasWorkspaceServer" ;;
        *)          echo "Unknown option: $1"; exit 1 ;;
    esac
    shift
done

BUILD_DIR="$ROOT_DIR/Builds/$BUILD_TYPE"
LOG_DIR="$ROOT_DIR/Logs"
LOG_FILE="$LOG_DIR/build.log"
TEST_LOG_FILE="$LOG_DIR/test.log"

# ── Colors & Symbols ─────────────────────────────────────────────
RED='\033[0;31m'
GREEN='\033[0;32m'
YELLOW='\033[1;33m'
BLUE='\033[0;34m'
CYAN='\033[0;36m'
BOLD='\033[1m'
DIM='\033[2m'
RESET='\033[0m'
CHECK='✓'
CROSS='✗'
ARROW='→'
GEAR='⚙'

# ── Helpers ───────────────────────────────────────────────────────
timestamp() { date '+%Y-%m-%d %H:%M:%S'; }

log() {
    local level="$1"; shift
    local msg="$*"
    local ts; ts=$(timestamp)
    case "$level" in
        INFO)  echo -e "${CYAN}[$ts]${RESET} ${GREEN}[INFO]${RESET}  $msg" ;;
        WARN)  echo -e "${CYAN}[$ts]${RESET} ${YELLOW}[WARN]${RESET}  $msg" ;;
        ERROR) echo -e "${CYAN}[$ts]${RESET} ${RED}[ERROR]${RESET} $msg" ;;
        STEP)  echo -e "${CYAN}[$ts]${RESET} ${BLUE}${BOLD}[$GEAR]${RESET}     $msg" ;;
    esac
    # Mirror to log file (strip ANSI codes)
    echo "[$ts] [$level] $msg" | sed 's/\x1b\[[0-9;]*m//g' >> "$LOG_FILE"
}

progress_bar() {
    local current=$1
    local total=$2
    local label="${3:-Building}"
    local width=40
    local pct=$((current * 100 / total))
    local filled=$((current * width / total))
    local empty=$((width - filled))
    local bar=""
    for ((i=0; i<filled; i++)); do bar+="█"; done
    for ((i=0; i<empty; i++)); do bar+="░"; done
    printf "\r  ${BLUE}%s${RESET} [${GREEN}%s${RESET}] ${BOLD}%3d%%${RESET} (%d/%d)" \
        "$label" "$bar" "$pct" "$current" "$total"
}

separator() {
    echo -e "${DIM}────────────────────────────────────────────────────────────${RESET}"
    echo "────────────────────────────────────────────────────────────" >> "$LOG_FILE"
}

# ── Setup ─────────────────────────────────────────────────────────
mkdir -p "$LOG_DIR"
mkdir -p "$BUILD_DIR"

# Start fresh log for this build
echo "=== AtlasWorkspace Build Log ===" > "$LOG_FILE"
echo "Started: $(timestamp)" >> "$LOG_FILE"
echo "Build Type: $BUILD_TYPE" >> "$LOG_FILE"
echo "Build Dir: $BUILD_DIR" >> "$LOG_FILE"
echo "" >> "$LOG_FILE"

# ── Banner ────────────────────────────────────────────────────────
echo ""
echo -e "${BOLD}${CYAN}╔══════════════════════════════════════════════════════════╗${RESET}"
echo -e "${BOLD}${CYAN}║${RESET}  ${BOLD}AtlasWorkspace Build System v0.1.0${RESET}                           ${BOLD}${CYAN}║${RESET}"
echo -e "${BOLD}${CYAN}╠══════════════════════════════════════════════════════════╣${RESET}"
echo -e "${BOLD}${CYAN}║${RESET}  Build Type:  ${YELLOW}$BUILD_TYPE${RESET}"
echo -e "${BOLD}${CYAN}║${RESET}  Build Dir:   ${DIM}$BUILD_DIR${RESET}"
echo -e "${BOLD}${CYAN}║${RESET}  Log File:    ${DIM}$LOG_FILE${RESET}"
if [[ -n "$TARGET" ]]; then
echo -e "${BOLD}${CYAN}║${RESET}  Target:      ${YELLOW}$TARGET${RESET}"
fi
if $RUN_TESTS; then
echo -e "${BOLD}${CYAN}║${RESET}  Run Tests:   ${GREEN}Yes${RESET}"
fi
echo -e "${BOLD}${CYAN}╚══════════════════════════════════════════════════════════╝${RESET}"
echo ""

# ── Step 1: Configure ─────────────────────────────────────────────
separator
log STEP "${ARROW} Step 1/3: Configuring CMake..."

CONFIGURE_START=$(date +%s)
cmake -B "$BUILD_DIR" -S "$ROOT_DIR" \
    -DCMAKE_BUILD_TYPE="$BUILD_TYPE" \
    -DCMAKE_EXPORT_COMPILE_COMMANDS=ON \
     \
     \
     \
    -DATLAS_BUILD_TESTS=ON \
    2>&1 | tee -a "$LOG_FILE" | while IFS= read -r line; do
        echo -e "  ${DIM}${line}${RESET}"
    done
CONFIGURE_END=$(date +%s)
CONFIGURE_ELAPSED=$((CONFIGURE_END - CONFIGURE_START))
log INFO "${CHECK} Configure complete (${CONFIGURE_ELAPSED}s)"
echo ""

# ── Step 2: Build ─────────────────────────────────────────────────
separator
NPROC=$(nproc 2>/dev/null || sysctl -n hw.ncpu 2>/dev/null || echo 4)

if [[ -n "$TARGET" ]]; then
    log STEP "${ARROW} Step 2/3: Building target ${BOLD}$TARGET${RESET} (${NPROC} threads)..."
    BUILD_CMD="cmake --build $BUILD_DIR --target $TARGET --parallel $NPROC"
else
    log STEP "${ARROW} Step 2/3: Building all targets (${NPROC} threads)..."
    BUILD_CMD="cmake --build $BUILD_DIR --parallel $NPROC"
fi

BUILD_START=$(date +%s)

# Count total build steps by doing a dry run if possible, otherwise estimate
TOTAL_STEPS=0
CURRENT_STEP=0

# Stream build output with progress tracking
$BUILD_CMD 2>&1 | tee -a "$LOG_FILE" | while IFS= read -r line; do
    # Parse Ninja/Make progress: [N/M] ...
    if [[ "$line" =~ ^\[([0-9]+)/([0-9]+)\] ]]; then
        CURRENT_STEP="${BASH_REMATCH[1]}"
        TOTAL_STEPS="${BASH_REMATCH[2]}"
        progress_bar "$CURRENT_STEP" "$TOTAL_STEPS" "Building"
    else
        # Show non-progress lines (warnings, errors, linking info)
        if [[ "$line" == *"warning:"* ]]; then
            echo -e "\n  ${YELLOW}⚠ ${line}${RESET}"
        elif [[ "$line" == *"error:"* ]]; then
            echo -e "\n  ${RED}✗ ${line}${RESET}"
        fi
    fi
done

echo ""  # newline after progress bar
BUILD_END=$(date +%s)
BUILD_ELAPSED=$((BUILD_END - BUILD_START))
log INFO "${CHECK} Build complete (${BUILD_ELAPSED}s)"
echo ""

# ── Build output summary ─────────────────────────────────────────
separator
log STEP "${ARROW} Build Output:"

# List executables
if [[ -d "$BUILD_DIR/bin" ]]; then
    echo -e "  ${BOLD}Executables:${RESET}"
    for exe in "$BUILD_DIR/bin/"*; do
        if [[ -f "$exe" && -x "$exe" && ! -d "$exe" ]]; then
            SIZE=$(du -h "$exe" | cut -f1)
            echo -e "    ${GREEN}${CHECK}${RESET} $(basename "$exe") ${DIM}($SIZE)${RESET}"
            echo "  [EXE] $(basename "$exe") ($SIZE)" >> "$LOG_FILE"
        fi
    done

    # List test executables
    if [[ -d "$BUILD_DIR/bin/Tests" ]]; then
        echo ""
        echo -e "  ${BOLD}Test Executables:${RESET}"
        TEST_COUNT=0
        for exe in "$BUILD_DIR/bin/Tests/"*; do
            if [[ -f "$exe" && -x "$exe" ]]; then
                SIZE=$(du -h "$exe" | cut -f1)
                echo -e "    ${BLUE}${CHECK}${RESET} $(basename "$exe") ${DIM}($SIZE)${RESET}"
                echo "  [TEST] $(basename "$exe") ($SIZE)" >> "$LOG_FILE"
                TEST_COUNT=$((TEST_COUNT + 1))
            fi
        done
        echo -e "  ${DIM}Total: $TEST_COUNT test executables${RESET}"
    fi

    # Check per-config directories (VS/multi-config)
    for CFG in Debug Release RelWithDebInfo; do
        if [[ -d "$BUILD_DIR/bin/$CFG" ]]; then
            echo ""
            echo -e "  ${BOLD}$CFG Executables:${RESET}"
            for exe in "$BUILD_DIR/bin/$CFG/"*; do
                if [[ -f "$exe" && -x "$exe" && ! -d "$exe" ]]; then
                    SIZE=$(du -h "$exe" | cut -f1)
                    echo -e "    ${GREEN}${CHECK}${RESET} $(basename "$exe") ${DIM}($SIZE)${RESET}"
                fi
            done
            if [[ -d "$BUILD_DIR/bin/$CFG/Tests" ]]; then
                echo -e "  ${BOLD}$CFG Test Executables:${RESET}"
                for exe in "$BUILD_DIR/bin/$CFG/Tests/"*; do
                    if [[ -f "$exe" && -x "$exe" ]]; then
                        SIZE=$(du -h "$exe" | cut -f1)
                        echo -e "    ${BLUE}${CHECK}${RESET} $(basename "$exe") ${DIM}($SIZE)${RESET}"
                    fi
                done
            fi
        fi
    done
fi
echo ""

# ── Step 3: Tests (optional) ─────────────────────────────────────
if $RUN_TESTS; then
    separator
    log STEP "${ARROW} Step 3/3: Running tests..."
    echo "" > "$TEST_LOG_FILE"

    TEST_START=$(date +%s)
    TESTS_PASSED=0
    TESTS_FAILED=0
    TESTS_TOTAL=0

    ctest --test-dir "$BUILD_DIR" --output-on-failure 2>&1 | tee -a "$TEST_LOG_FILE" | while IFS= read -r line; do
        # Mirror to build log too
        echo "$line" >> "$LOG_FILE"

        # Parse ctest output for progress
        if [[ "$line" =~ ^[[:space:]]*([0-9]+)/([0-9]+) ]]; then
            CURRENT="${BASH_REMATCH[1]}"
            TOTAL="${BASH_REMATCH[2]}"
            progress_bar "$CURRENT" "$TOTAL" "Testing "
        fi

        # Show pass/fail
        if [[ "$line" == *"Passed"* ]]; then
            : # handled by progress bar
        elif [[ "$line" == *"Failed"* || "$line" == *"FAILED"* ]]; then
            echo -e "\n  ${RED}${CROSS} $line${RESET}"
        fi

        # Final summary
        if [[ "$line" =~ ([0-9]+)%" tests passed" ]]; then
            echo ""
            echo -e "  $line"
        fi
    done

    echo ""
    TEST_END=$(date +%s)
    TEST_ELAPSED=$((TEST_END - TEST_START))
    log INFO "${CHECK} Tests complete (${TEST_ELAPSED}s) — see $TEST_LOG_FILE"
    echo ""
else
    log INFO "Skipping tests (use --test to run)"
fi

# ── Atlas Workspace scaffolding ───────────────────────────────────
log INFO "${ARROW} Scaffolding Atlas Workspace..."
mkdir -p "$ROOT_DIR/Atlas/Workspace/Sessions"
mkdir -p "$ROOT_DIR/.novaforge/pipeline/changes"
mkdir -p "$ROOT_DIR/.novaforge/pipeline/assets"
mkdir -p "$ROOT_DIR/.novaforge/pipeline/worlds"
mkdir -p "$ROOT_DIR/.novaforge/pipeline/scripts"
mkdir -p "$ROOT_DIR/.novaforge/pipeline/animations"
mkdir -p "$ROOT_DIR/.novaforge/pipeline/sessions"
log INFO "${CHECK} Atlas Workspace ready"
echo ""

# ── Summary ───────────────────────────────────────────────────────
separator
TOTAL_END=$(date +%s)
TOTAL_ELAPSED=$((TOTAL_END - CONFIGURE_START))
echo ""
echo -e "${BOLD}${GREEN}╔══════════════════════════════════════════════════════════╗${RESET}"
echo -e "${BOLD}${GREEN}║${RESET}  ${BOLD}Build Complete!${RESET}                                        ${BOLD}${GREEN}║${RESET}"
echo -e "${BOLD}${GREEN}╠══════════════════════════════════════════════════════════╣${RESET}"
echo -e "${BOLD}${GREEN}║${RESET}  Total Time:    ${BOLD}${TOTAL_ELAPSED}s${RESET}"
echo -e "${BOLD}${GREEN}║${RESET}  Configure:     ${CONFIGURE_ELAPSED}s"
echo -e "${BOLD}${GREEN}║${RESET}  Build:         ${BUILD_ELAPSED}s"
echo -e "${BOLD}${GREEN}║${RESET}  Build Type:    ${YELLOW}$BUILD_TYPE${RESET}"
echo -e "${BOLD}${GREEN}║${RESET}  Log File:      ${DIM}$LOG_FILE${RESET}"
echo -e "${BOLD}${GREEN}╚══════════════════════════════════════════════════════════╝${RESET}"
echo ""
echo "Finished: $(timestamp)" >> "$LOG_FILE"
echo "Total time: ${TOTAL_ELAPSED}s" >> "$LOG_FILE"
