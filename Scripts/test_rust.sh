#!/usr/bin/env bash
# ╔══════════════════════════════════════════════════════════════════╗
# ║  Atlas Workspace — Rust Test Runner                             ║
# ║  Run cargo tests, parse per-crate pass/fail, log output         ║
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
DIM='\033[2m'
RESET='\033[0m'
CHECK='✓'
CROSS='✗'

# ── Arguments ─────────────────────────────────────────────────────
TARGET_CRATE=""
VERBOSE=false
LIB_ONLY=false
DOC_TESTS=false

while [[ $# -gt 0 ]]; do
    case "$1" in
        --crate)   shift; TARGET_CRATE="$1" ;;
        --verbose) VERBOSE=true ;;
        --lib)     LIB_ONLY=true ;;
        --doc)     DOC_TESTS=true ;;
        *)         echo "Unknown option: $1"; exit 1 ;;
    esac
    shift
done

LOG_DIR="$ROOT_DIR/Logs"
LOG_FILE="$LOG_DIR/rust_test.log"

mkdir -p "$LOG_DIR"
echo "=== Atlas Workspace Rust Test Log ===" > "$LOG_FILE"
echo "Started: $(date '+%Y-%m-%d %H:%M:%S')" >> "$LOG_FILE"
echo "" >> "$LOG_FILE"

# ── Banner ────────────────────────────────────────────────────────
echo ""
echo -e "${BOLD}${CYAN}╔══════════════════════════════════════════════════════════╗${RESET}"
echo -e "${BOLD}${CYAN}║${RESET}  ${BOLD}Atlas Workspace — Rust Test Runner${RESET}                       ${BOLD}${CYAN}║${RESET}"
echo -e "${BOLD}${CYAN}╚══════════════════════════════════════════════════════════╝${RESET}"
echo ""

# ── Build test command ────────────────────────────────────────────
cd "$ROOT_DIR"

TEST_ARGS=()
if [[ -n "$TARGET_CRATE" ]]; then
    TEST_ARGS+=(--package "$TARGET_CRATE")
    echo -e "  Crate: ${YELLOW}$TARGET_CRATE${RESET}"
else
    TEST_ARGS+=(--workspace)
    echo -e "  Scope: ${YELLOW}all workspace crates${RESET}"
fi

$LIB_ONLY  && TEST_ARGS+=(--lib)
$DOC_TESTS && TEST_ARGS+=(--doc)
$VERBOSE   && TEST_ARGS+=(-- --nocapture)

echo ""

# ── Run tests ─────────────────────────────────────────────────────
TOTAL_PASSED=0
TOTAL_FAILED=0

RUN_OUTPUT=$(cargo test "${TEST_ARGS[@]}" 2>&1) || true
echo "$RUN_OUTPUT" | tee -a "$LOG_FILE"

# Parse per-crate results
while IFS= read -r line; do
    if [[ "$line" =~ ^test\ result:\ ok\.\ ([0-9]+)\ passed ]]; then
        N=${BASH_REMATCH[1]}
        TOTAL_PASSED=$((TOTAL_PASSED + N))
        echo -e "  ${GREEN}${CHECK}${RESET} $N passed"
    elif [[ "$line" =~ ^test\ result:\ FAILED\..*([0-9]+)\ failed ]]; then
        F=${BASH_REMATCH[1]}
        TOTAL_FAILED=$((TOTAL_FAILED + F))
        echo -e "  ${RED}${CROSS}${RESET} $F FAILED"
    fi
done <<< "$RUN_OUTPUT"

echo ""
echo "────────────────────────────────────────────────────────────"
if [[ $TOTAL_FAILED -eq 0 ]]; then
    echo -e "  ${GREEN}${BOLD}${CHECK} All tests passed: ${TOTAL_PASSED}${RESET}"
else
    echo -e "  ${RED}${BOLD}${CROSS} FAILED: ${TOTAL_PASSED} passed, ${TOTAL_FAILED} failed${RESET}"
fi
echo ""
echo "  Log: $LOG_FILE"
echo ""

echo "Total passed: $TOTAL_PASSED" >> "$LOG_FILE"
echo "Total failed: $TOTAL_FAILED" >> "$LOG_FILE"
echo "Finished: $(date '+%Y-%m-%d %H:%M:%S')" >> "$LOG_FILE"

[[ $TOTAL_FAILED -gt 0 ]] && exit 1
exit 0
