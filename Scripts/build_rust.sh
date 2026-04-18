#!/usr/bin/env bash
# ╔══════════════════════════════════════════════════════════════════╗
# ║  Atlas Workspace — Rust Build System                            ║
# ║  Primary build script for cargo-based Rust workspace            ║
# ║  All output is shown on screen AND mirrored to Logs/rust_build.log ║
# ╚══════════════════════════════════════════════════════════════════╝
set -euo pipefail

SCRIPT_DIR="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)"
ROOT_DIR="$(cd "$SCRIPT_DIR/.." && pwd)"

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

# ── Arguments ─────────────────────────────────────────────────────
BUILD_TYPE="debug"
RUN_TESTS=false
RUN_CLIPPY=false
FMT_CHECK=false
RUN_DOC=false
TARGET_BIN=""
TARGET_CRATE=""
TARGET_PACKAGE=""

while [[ $# -gt 0 ]]; do
    case "$1" in
        debug|release)  BUILD_TYPE="$1" ;;
        --test)         RUN_TESTS=true ;;
        --clippy)       RUN_CLIPPY=true ;;
        --fmt-check)    FMT_CHECK=true ;;
        --doc)          RUN_DOC=true ;;
        --editor)       TARGET_BIN="atlas-workspace" ;;
        --game)         TARGET_BIN="atlas-game" ;;
        --workspace)    TARGET_PACKAGE="atlas-workspace" ;;
        --crate)        shift; TARGET_CRATE="$1"; TARGET_PACKAGE="$1" ;;
        *)              echo "Unknown option: $1"; exit 1 ;;
    esac
    shift
done

RELEASE_FLAG=""
[[ "$BUILD_TYPE" == "release" ]] && RELEASE_FLAG="--release"

LOG_DIR="$ROOT_DIR/Logs"
LOG_FILE="$LOG_DIR/rust_build.log"

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
    echo "[$ts] [$level] $msg" | sed 's/\x1b\[[0-9;]*m//g' >> "$LOG_FILE"
}

separator() {
    echo -e "${DIM}────────────────────────────────────────────────────────────${RESET}"
    echo "────────────────────────────────────────────────────────────" >> "$LOG_FILE"
}

# ── Setup ─────────────────────────────────────────────────────────
mkdir -p "$LOG_DIR"
echo "=== Atlas Workspace Rust Build Log ===" > "$LOG_FILE"
echo "Started: $(timestamp)" >> "$LOG_FILE"
echo "Build Type: $BUILD_TYPE" >> "$LOG_FILE"
echo "" >> "$LOG_FILE"

# ── Check prerequisites ───────────────────────────────────────────
for tool in rustup rustc cargo; do
    if ! command -v "$tool" &>/dev/null; then
        echo -e "${RED}${CROSS} '$tool' not found. Install rustup from https://rustup.rs${RESET}"
        exit 1
    fi
done

# ── Banner ────────────────────────────────────────────────────────
echo ""
echo -e "${BOLD}${CYAN}╔══════════════════════════════════════════════════════════╗${RESET}"
echo -e "${BOLD}${CYAN}║${RESET}  ${BOLD}Atlas Workspace — Rust Build System${RESET}                      ${BOLD}${CYAN}║${RESET}"
echo -e "${BOLD}${CYAN}╠══════════════════════════════════════════════════════════╣${RESET}"
echo -e "${BOLD}${CYAN}║${RESET}  Build Type:  ${YELLOW}$BUILD_TYPE${RESET}"
echo -e "${BOLD}${CYAN}║${RESET}  rustc:       $(rustc --version)"
echo -e "${BOLD}${CYAN}║${RESET}  cargo:       $(cargo --version)"
echo -e "${BOLD}${CYAN}║${RESET}  Log File:    ${DIM}$LOG_FILE${RESET}"
[[ -n "$TARGET_CRATE" ]] && echo -e "${BOLD}${CYAN}║${RESET}  Crate:       ${YELLOW}$TARGET_CRATE${RESET}"
[[ -n "$TARGET_BIN"   ]] && echo -e "${BOLD}${CYAN}║${RESET}  Binary:      ${YELLOW}$TARGET_BIN${RESET}"
$RUN_TESTS  && echo -e "${BOLD}${CYAN}║${RESET}  Tests:       ${GREEN}Yes${RESET}"
$RUN_CLIPPY && echo -e "${BOLD}${CYAN}║${RESET}  Clippy:      ${GREEN}Yes${RESET}"
$FMT_CHECK  && echo -e "${BOLD}${CYAN}║${RESET}  Fmt-Check:   ${GREEN}Yes${RESET}"
echo -e "${BOLD}${CYAN}╚══════════════════════════════════════════════════════════╝${RESET}"
echo ""

BUILD_START=$(date +%s)

# ── Step 1: fmt check (optional) ──────────────────────────────────
if $FMT_CHECK; then
    separator
    log STEP "${ARROW} Checking code formatting..."
    cd "$ROOT_DIR"
    if cargo fmt --all -- --check 2>&1 | tee -a "$LOG_FILE"; then
        log INFO "${CHECK} Formatting OK"
    else
        log ERROR "${CROSS} Formatting issues found. Run: cargo fmt --all"
        exit 1
    fi
    echo ""
fi

# ── Step 2: clippy (optional) ─────────────────────────────────────
if $RUN_CLIPPY; then
    separator
    log STEP "${ARROW} Running clippy..."
    cd "$ROOT_DIR"
    if cargo clippy --workspace -- -D warnings 2>&1 | tee -a "$LOG_FILE"; then
        log INFO "${CHECK} Clippy clean"
    else
        log ERROR "${CROSS} Clippy found errors"
        exit 1
    fi
    echo ""
fi

# ── Step 3: Build ─────────────────────────────────────────────────
separator

if [[ -n "$TARGET_BIN" ]]; then
    log STEP "${ARROW} Building binary: $TARGET_BIN ($BUILD_TYPE)..."
    BUILD_CMD="cargo build --bin $TARGET_BIN $RELEASE_FLAG"
elif [[ -n "$TARGET_PACKAGE" ]]; then
    log STEP "${ARROW} Building package: $TARGET_PACKAGE ($BUILD_TYPE)..."
    BUILD_CMD="cargo build --package $TARGET_PACKAGE $RELEASE_FLAG"
else
    log STEP "${ARROW} Building workspace ($BUILD_TYPE)..."
    BUILD_CMD="cargo build --workspace $RELEASE_FLAG"
fi

cd "$ROOT_DIR"
if $BUILD_CMD 2>&1 | tee -a "$LOG_FILE"; then
    log INFO "${CHECK} Build complete"
else
    log ERROR "${CROSS} Build failed"
    exit 1
fi
echo ""

# ── Binary locations ──────────────────────────────────────────────
separator
log STEP "${ARROW} Binary locations:"
TARGET_DIR="$ROOT_DIR/target/$BUILD_TYPE"
for bin in atlas-workspace atlas-game; do
    BIN_PATH="$TARGET_DIR/$bin"
    if [[ -f "$BIN_PATH" ]]; then
        SIZE=$(du -h "$BIN_PATH" | cut -f1)
        echo -e "  ${GREEN}${CHECK}${RESET} $BIN_PATH ${DIM}($SIZE)${RESET}"
        echo "  [BIN] $bin ($SIZE)" >> "$LOG_FILE"
    fi
done
echo ""

# ── Step 4: Tests (optional) ─────────────────────────────────────
if $RUN_TESTS; then
    separator
    log STEP "${ARROW} Running tests..."
    cd "$ROOT_DIR"

    TESTS_PASSED=0
    TESTS_FAILED=0

    if [[ -n "$TARGET_PACKAGE" ]]; then
        TEST_CMD="cargo test --package $TARGET_PACKAGE $RELEASE_FLAG"
    else
        TEST_CMD="cargo test --workspace $RELEASE_FLAG"
    fi

    TEST_OUTPUT=$($TEST_CMD 2>&1 | tee -a "$LOG_FILE")
    echo "$TEST_OUTPUT"

    while IFS= read -r line; do
        if [[ "$line" =~ test\ result:\ ok\.\ ([0-9]+)\ passed ]]; then
            TESTS_PASSED=$((TESTS_PASSED + BASH_REMATCH[1]))
        elif [[ "$line" =~ test\ result:\ FAILED\..*([0-9]+)\ failed ]]; then
            TESTS_FAILED=$((TESTS_FAILED + BASH_REMATCH[1]))
        fi
    done <<< "$TEST_OUTPUT"

    echo ""
    if [[ $TESTS_FAILED -eq 0 ]]; then
        log INFO "${CHECK} Tests: ${TESTS_PASSED} passed, ${TESTS_FAILED} failed"
    else
        log ERROR "${CROSS} Tests: ${TESTS_PASSED} passed, ${TESTS_FAILED} FAILED"
        exit 1
    fi
    echo ""
fi

# ── Step 5: Docs (optional) ───────────────────────────────────────
if $RUN_DOC; then
    separator
    log STEP "${ARROW} Building documentation..."
    cd "$ROOT_DIR"
    cargo doc --workspace --no-deps 2>&1 | tee -a "$LOG_FILE"
    log INFO "${CHECK} Docs built: target/doc/"
    echo ""
fi

# ── Summary ───────────────────────────────────────────────────────
separator
BUILD_END=$(date +%s)
TOTAL_ELAPSED=$((BUILD_END - BUILD_START))
echo ""
echo -e "${BOLD}${GREEN}╔══════════════════════════════════════════════════════════╗${RESET}"
echo -e "${BOLD}${GREEN}║${RESET}  ${BOLD}Build Complete!${RESET}                                        ${BOLD}${GREEN}║${RESET}"
echo -e "${BOLD}${GREEN}╠══════════════════════════════════════════════════════════╣${RESET}"
echo -e "${BOLD}${GREEN}║${RESET}  Total Time:    ${BOLD}${TOTAL_ELAPSED}s${RESET}"
echo -e "${BOLD}${GREEN}║${RESET}  Build Type:    ${YELLOW}$BUILD_TYPE${RESET}"
$RUN_TESTS  && echo -e "${BOLD}${GREEN}║${RESET}  Tests:         ${GREEN}${TESTS_PASSED:-0} passed${RESET}"
echo -e "${BOLD}${GREEN}║${RESET}  Log File:      ${DIM}$LOG_FILE${RESET}"
echo -e "${BOLD}${GREEN}╚══════════════════════════════════════════════════════════╝${RESET}"
echo ""
echo "Finished: $(timestamp)" >> "$LOG_FILE"
echo "Total time: ${TOTAL_ELAPSED}s" >> "$LOG_FILE"
