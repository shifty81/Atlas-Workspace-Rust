#!/usr/bin/env bash
# ╔══════════════════════════════════════════════════════════════════╗
# ║  Atlas Workspace — Unified Build & Clean Script                  ║
# ║  Single entry-point for clean, build, and rebuild operations.    ║
# ║  ⚠ NEVER modifies novaforge-assets/ — assets are always safe.   ║
# ║  All output is mirrored to Logs/atlas.log                        ║
# ╚══════════════════════════════════════════════════════════════════╝
#
# Usage:
#   ./atlas.sh build   [debug|release] [--workspace|--game|--editor]
#                      [--test] [--clippy] [--fmt-check] [--doc]
#   ./atlas.sh clean
#   ./atlas.sh rebuild [debug|release] [--workspace|--game|--editor]
#                      [--test] [--clippy] [--fmt-check] [--doc]
#
# Subcommands:
#   build     Compile the workspace or a specific target.
#   clean     Remove cargo build artefacts (never touches novaforge-assets/).
#   rebuild   clean → build in one step.
#
# Build flags:
#   debug | release   Build profile (default: debug)
#   --workspace       Build all workspace crates (default when no target given)
#   --game            Build the atlas-game binary only
#   --editor          Build the atlas-workspace editor binary only
#   --test            Run tests after a successful build
#   --clippy          Run cargo clippy before the build step
#   --fmt-check       Run cargo fmt --check before the build step
#   --doc             Build rustdoc after a successful build

set -euo pipefail

# ── Paths ─────────────────────────────────────────────────────────
SCRIPT_DIR="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)"
ROOT_DIR="$SCRIPT_DIR"
ASSETS_DIR="$ROOT_DIR/novaforge-assets"
LOG_DIR="$ROOT_DIR/Logs"
LOG_FILE="$LOG_DIR/atlas.log"

# ── Colors & symbols ──────────────────────────────────────────────
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
        STEP)  echo -e "${CYAN}[$ts]${RESET} ${BLUE}${BOLD}[${GEAR}]${RESET}     $msg" ;;
    esac
    echo "[$ts] [$level] $msg" | sed 's/\x1b\[[0-9;]*m//g' >> "$LOG_FILE"
}

separator() {
    echo -e "${DIM}────────────────────────────────────────────────────────────${RESET}"
    echo "────────────────────────────────────────────────────────────" >> "$LOG_FILE"
}

die() {
    log ERROR "$*"
    exit 1
}

usage() {
    echo ""
    echo -e "${BOLD}Usage:${RESET}"
    echo "  ./atlas.sh build   [debug|release] [--workspace|--game|--editor]"
    echo "                     [--test] [--clippy] [--fmt-check] [--doc]"
    echo "  ./atlas.sh clean"
    echo "  ./atlas.sh rebuild [debug|release] [--workspace|--game|--editor]"
    echo "                     [--test] [--clippy] [--fmt-check] [--doc]"
    echo ""
    echo -e "${BOLD}Subcommands:${RESET}"
    echo "  build     Compile the workspace or a specific target"
    echo "  clean     Remove cargo build artefacts only (assets are never touched)"
    echo "  rebuild   clean → build in one step"
    echo ""
    echo -e "${BOLD}Build flags:${RESET}"
    echo "  debug | release   Build profile             (default: debug)"
    echo "  --workspace       Build all workspace crates (default)"
    echo "  --game            Build atlas-game binary only"
    echo "  --editor          Build atlas-workspace editor binary only"
    echo "  --test            Run tests after build"
    echo "  --clippy          Run clippy before build"
    echo "  --fmt-check       Run cargo fmt --check before build"
    echo "  --doc             Build rustdoc after build"
    echo ""
}

# ── Argument parsing ──────────────────────────────────────────────
if [[ $# -eq 0 ]]; then
    usage
    exit 0
fi

SUBCOMMAND="$1"; shift

BUILD_TYPE="debug"
TARGET_BIN=""
RUN_TESTS=false
RUN_CLIPPY=false
FMT_CHECK=false
RUN_DOC=false

while [[ $# -gt 0 ]]; do
    case "$1" in
        debug|release)  BUILD_TYPE="$1" ;;
        --workspace)    TARGET_BIN="" ;;           # explicit full-workspace (default)
        --game)         TARGET_BIN="atlas-game" ;;
        --editor)       TARGET_BIN="atlas-workspace" ;;
        --test)         RUN_TESTS=true ;;
        --clippy)       RUN_CLIPPY=true ;;
        --fmt-check)    FMT_CHECK=true ;;
        --doc)          RUN_DOC=true ;;
        -h|--help)      usage; exit 0 ;;
        *)              die "Unknown option: $1  (run ./atlas.sh --help)" ;;
    esac
    shift
done

case "$SUBCOMMAND" in
    build|clean|rebuild) ;;
    -h|--help) usage; exit 0 ;;
    *) die "Unknown subcommand: '$SUBCOMMAND'  (expected: build | clean | rebuild)" ;;
esac

RELEASE_FLAG=""
[[ "$BUILD_TYPE" == "release" ]] && RELEASE_FLAG="--release"

# ── Log initialisation ────────────────────────────────────────────
mkdir -p "$LOG_DIR"
{
    echo "=== Atlas Workspace Build Log ==="
    echo "Started: $(timestamp)"
    echo "Command: $SUBCOMMAND  build_type=$BUILD_TYPE"
    [[ -n "$TARGET_BIN" ]] && echo "Target: $TARGET_BIN"
    echo ""
} > "$LOG_FILE"

# ── Prerequisite checks ───────────────────────────────────────────
for tool in rustup rustc cargo; do
    if ! command -v "$tool" &>/dev/null; then
        die "'$tool' not found — install rustup from https://rustup.rs"
    fi
done

# ── Assets guard ──────────────────────────────────────────────────
# Verify novaforge-assets/ will not be touched by any operation.
# This function is intentionally a no-op today (cargo clean / cargo build
# never enter that directory) but documents the invariant explicitly.
assert_assets_safe() {
    if [[ -d "$ASSETS_DIR" ]]; then
        log INFO "novaforge-assets/ present — will not be touched by this operation"
    fi
}

# ── Banner ────────────────────────────────────────────────────────
print_banner() {
    local target_label="${TARGET_BIN:-workspace (all crates)}"
    echo ""
    echo -e "${BOLD}${CYAN}╔══════════════════════════════════════════════════════════╗${RESET}"
    echo -e "${BOLD}${CYAN}║${RESET}  ${BOLD}Atlas Workspace — Unified Build System${RESET}                 ${BOLD}${CYAN}║${RESET}"
    echo -e "${BOLD}${CYAN}╠══════════════════════════════════════════════════════════╣${RESET}"
    echo -e "${BOLD}${CYAN}║${RESET}  Subcommand:  ${YELLOW}$SUBCOMMAND${RESET}"
    echo -e "${BOLD}${CYAN}║${RESET}  Build Type:  ${YELLOW}$BUILD_TYPE${RESET}"
    echo -e "${BOLD}${CYAN}║${RESET}  Target:      ${YELLOW}$target_label${RESET}"
    echo -e "${BOLD}${CYAN}║${RESET}  rustc:       $(rustc --version)"
    echo -e "${BOLD}${CYAN}║${RESET}  Log File:    ${DIM}$LOG_FILE${RESET}"
    $RUN_CLIPPY  && echo -e "${BOLD}${CYAN}║${RESET}  Clippy:      ${GREEN}Yes${RESET}"
    $FMT_CHECK   && echo -e "${BOLD}${CYAN}║${RESET}  Fmt-Check:   ${GREEN}Yes${RESET}"
    $RUN_TESTS   && echo -e "${BOLD}${CYAN}║${RESET}  Tests:       ${GREEN}Yes${RESET}"
    $RUN_DOC     && echo -e "${BOLD}${CYAN}║${RESET}  Docs:        ${GREEN}Yes${RESET}"
    echo -e "${BOLD}${CYAN}╚══════════════════════════════════════════════════════════╝${RESET}"
    echo ""
}

# ── Step: clean ───────────────────────────────────────────────────
do_clean() {
    separator
    log STEP "${ARROW} Cleaning build artefacts (cargo clean)..."
    log INFO "novaforge-assets/ will not be touched"
    cd "$ROOT_DIR"
    if cargo clean 2>&1 | tee -a "$LOG_FILE"; then
        log INFO "${CHECK} Clean complete"
    else
        die "${CROSS} Clean failed"
    fi
    echo ""
}

# ── Step: fmt-check ───────────────────────────────────────────────
do_fmt_check() {
    separator
    log STEP "${ARROW} Checking code formatting..."
    cd "$ROOT_DIR"
    if cargo fmt --all -- --check 2>&1 | tee -a "$LOG_FILE"; then
        log INFO "${CHECK} Formatting OK"
    else
        die "${CROSS} Formatting issues found — run: cargo fmt --all"
    fi
    echo ""
}

# ── Step: clippy ──────────────────────────────────────────────────
do_clippy() {
    separator
    log STEP "${ARROW} Running clippy..."
    cd "$ROOT_DIR"
    if cargo clippy --workspace -- -D warnings 2>&1 | tee -a "$LOG_FILE"; then
        log INFO "${CHECK} Clippy clean"
    else
        die "${CROSS} Clippy found errors"
    fi
    echo ""
}

# ── Step: build ───────────────────────────────────────────────────
TESTS_PASSED=0
TESTS_FAILED=0

do_build() {
    separator
    cd "$ROOT_DIR"

    if [[ -n "$TARGET_BIN" ]]; then
        log STEP "${ARROW} Building binary: $TARGET_BIN ($BUILD_TYPE)..."
        BUILD_CMD=(cargo build --bin "$TARGET_BIN" $RELEASE_FLAG)
    else
        log STEP "${ARROW} Building workspace ($BUILD_TYPE)..."
        BUILD_CMD=(cargo build --workspace $RELEASE_FLAG)
    fi

    if "${BUILD_CMD[@]}" 2>&1 | tee -a "$LOG_FILE"; then
        log INFO "${CHECK} Build complete"
    else
        die "${CROSS} Build failed"
    fi
    echo ""

    # Binary locations summary
    separator
    log STEP "${ARROW} Binary locations:"
    local TARGET_DIR="$ROOT_DIR/target/$BUILD_TYPE"
    for bin in atlas-workspace atlas-game; do
        local BIN_PATH="$TARGET_DIR/$bin"
        if [[ -f "$BIN_PATH" ]]; then
            local SIZE
            SIZE=$(du -h "$BIN_PATH" | cut -f1)
            echo -e "  ${GREEN}${CHECK}${RESET} $BIN_PATH ${DIM}($SIZE)${RESET}"
            echo "  [BIN] $bin ($SIZE)" >> "$LOG_FILE"
        fi
    done
    echo ""
}

# ── Step: tests ───────────────────────────────────────────────────
do_tests() {
    separator
    log STEP "${ARROW} Running tests..."
    cd "$ROOT_DIR"

    local TEST_CMD
    if [[ -n "$TARGET_BIN" ]]; then
        TEST_CMD=(cargo test --bin "$TARGET_BIN" $RELEASE_FLAG)
    else
        TEST_CMD=(cargo test --workspace $RELEASE_FLAG)
    fi

    local TEST_OUTPUT
    TEST_OUTPUT=$("${TEST_CMD[@]}" 2>&1 | tee -a "$LOG_FILE") || true
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
        log INFO "${CHECK} Tests: ${TESTS_PASSED} passed, 0 failed"
    else
        log ERROR "${CROSS} Tests: ${TESTS_PASSED} passed, ${TESTS_FAILED} FAILED"
    fi
    echo ""
}

# ── Step: docs ────────────────────────────────────────────────────
do_doc() {
    separator
    log STEP "${ARROW} Building documentation..."
    cd "$ROOT_DIR"
    cargo doc --workspace --no-deps 2>&1 | tee -a "$LOG_FILE"
    log INFO "${CHECK} Docs built: target/doc/"
    echo ""
}

# ── Summary footer ────────────────────────────────────────────────
print_summary() {
    local elapsed="$1"
    separator
    echo ""
    echo -e "${BOLD}${GREEN}╔══════════════════════════════════════════════════════════╗${RESET}"
    echo -e "${BOLD}${GREEN}║${RESET}  ${BOLD}Done!${RESET}                                                  ${BOLD}${GREEN}║${RESET}"
    echo -e "${BOLD}${GREEN}╠══════════════════════════════════════════════════════════╣${RESET}"
    echo -e "${BOLD}${GREEN}║${RESET}  Subcommand:    ${YELLOW}$SUBCOMMAND${RESET}"
    echo -e "${BOLD}${GREEN}║${RESET}  Build Type:    ${YELLOW}$BUILD_TYPE${RESET}"
    echo -e "${BOLD}${GREEN}║${RESET}  Total Time:    ${BOLD}${elapsed}s${RESET}"
    $RUN_TESTS && echo -e "${BOLD}${GREEN}║${RESET}  Tests:         ${GREEN}${TESTS_PASSED} passed${RESET}"
    echo -e "${BOLD}${GREEN}║${RESET}  Log File:      ${DIM}$LOG_FILE${RESET}"
    echo -e "${BOLD}${GREEN}╚══════════════════════════════════════════════════════════╝${RESET}"
    echo ""
    {
        echo "Finished: $(timestamp)"
        echo "Total time: ${elapsed}s"
    } >> "$LOG_FILE"
}

# ── Main dispatcher ───────────────────────────────────────────────
assert_assets_safe
print_banner

START=$(date +%s)

case "$SUBCOMMAND" in
    clean)
        do_clean
        ;;
    build)
        $FMT_CHECK  && do_fmt_check
        $RUN_CLIPPY && do_clippy
        do_build
        $RUN_TESTS && do_tests
        $RUN_DOC   && do_doc
        ;;
    rebuild)
        do_clean
        $FMT_CHECK  && do_fmt_check
        $RUN_CLIPPY && do_clippy
        do_build
        $RUN_TESTS && do_tests
        $RUN_DOC   && do_doc
        ;;
esac

END=$(date +%s)
ELAPSED=$((END - START))

if [[ "${TESTS_FAILED:-0}" -gt 0 ]]; then
    print_summary "$ELAPSED"
    exit 1
fi

print_summary "$ELAPSED"
