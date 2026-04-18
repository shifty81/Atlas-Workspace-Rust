#!/usr/bin/env bash
# ╔══════════════════════════════════════════════════════════════════╗
# ║  Atlas Workspace — Rust Check & Clippy                          ║
# ║  Fast cargo check then cargo clippy. Exits 1 if errors.         ║
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
ARROW='→'

LOG_DIR="$ROOT_DIR/Logs"
LOG_FILE="$LOG_DIR/rust_check.log"

mkdir -p "$LOG_DIR"
echo "=== Atlas Workspace Rust Check Log ===" > "$LOG_FILE"
echo "Started: $(date '+%Y-%m-%d %H:%M:%S')" >> "$LOG_FILE"
echo "" >> "$LOG_FILE"

echo ""
echo -e "${BOLD}${CYAN}╔══════════════════════════════════════════════════════════╗${RESET}"
echo -e "${BOLD}${CYAN}║${RESET}  ${BOLD}Atlas Workspace — Rust Check & Clippy${RESET}                   ${BOLD}${CYAN}║${RESET}"
echo -e "${BOLD}${CYAN}╚══════════════════════════════════════════════════════════╝${RESET}"
echo ""

cd "$ROOT_DIR"

# ── Step 1: cargo check ───────────────────────────────────────────
echo -e "${CYAN}[$ARROW]${RESET} Running ${BOLD}cargo check --workspace${RESET}..."
CHECK_ERRORS=0

if cargo check --workspace 2>&1 | tee -a "$LOG_FILE"; then
    echo -e "  ${GREEN}${CHECK} cargo check clean${RESET}"
else
    echo -e "  ${RED}${CROSS} cargo check found errors${RESET}"
    CHECK_ERRORS=1
fi
echo ""

# ── Step 2: cargo clippy ──────────────────────────────────────────
echo -e "${CYAN}[$ARROW]${RESET} Running ${BOLD}cargo clippy --workspace -- -D warnings${RESET}..."
CLIPPY_ERRORS=0

CLIPPY_OUTPUT=$(cargo clippy --workspace -- -D warnings 2>&1) || CLIPPY_ERRORS=1
echo "$CLIPPY_OUTPUT" | tee -a "$LOG_FILE"

WARN_COUNT=$(echo "$CLIPPY_OUTPUT" | grep -c "^warning" || true)
ERR_COUNT=$(echo "$CLIPPY_OUTPUT"  | grep -c "^error"   || true)

echo ""
if [[ $CLIPPY_ERRORS -eq 0 ]]; then
    echo -e "  ${GREEN}${CHECK} clippy clean${RESET} (${WARN_COUNT} warnings)"
else
    echo -e "  ${RED}${CROSS} clippy errors: ${ERR_COUNT} errors, ${WARN_COUNT} warnings${RESET}"
fi
echo ""

echo "check errors:  $CHECK_ERRORS"  >> "$LOG_FILE"
echo "clippy errors: $CLIPPY_ERRORS" >> "$LOG_FILE"
echo "Finished: $(date '+%Y-%m-%d %H:%M:%S')" >> "$LOG_FILE"

echo "  Log: $LOG_FILE"
echo ""

if [[ $CHECK_ERRORS -ne 0 || $CLIPPY_ERRORS -ne 0 ]]; then
    exit 1
fi
exit 0
