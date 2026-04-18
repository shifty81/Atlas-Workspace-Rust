#!/usr/bin/env bash
# AtlasWorkspace — Build all targets
set -euo pipefail

SCRIPT_DIR="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)"
ROOT_DIR="$(cd "$SCRIPT_DIR/.." && pwd)"

BUILD_TYPE="${1:-Debug}"
BUILD_DIR="$ROOT_DIR/Builds/$BUILD_TYPE"
LOG_DIR="$ROOT_DIR/Logs"
LOG_FILE="$LOG_DIR/build.log"
TEST_LOG_FILE="$LOG_DIR/test.log"

# ── Setup logging ──────────────────────────────────────────────────
mkdir -p "$LOG_DIR"
echo "=== AtlasWorkspace Build Log ===" > "$LOG_FILE"
echo "Started: $(date '+%Y-%m-%d %H:%M:%S')" >> "$LOG_FILE"
echo "Build Type: $BUILD_TYPE" >> "$LOG_FILE"
echo "Build Dir:  $BUILD_DIR" >> "$LOG_FILE"
echo "" >> "$LOG_FILE"

log() {
    local ts; ts=$(date '+%Y-%m-%d %H:%M:%S')
    echo "[$ts] $*"
    echo "[$ts] $*" >> "$LOG_FILE"
}

echo "╔══════════════════════════════════════════╗"
echo "║       AtlasWorkspace Build System             ║"
echo "╠══════════════════════════════════════════╣"
echo "║  Build Type:  $BUILD_TYPE"
echo "║  Build Dir:   $BUILD_DIR"
echo "║  Log File:    $LOG_FILE"
echo "╚══════════════════════════════════════════╝"

# ── Configure ─────────────────────────────────────────────────────
log "[STEP] Configuring CMake..."
cmake -B "$BUILD_DIR" -S "$ROOT_DIR" \
    -DCMAKE_BUILD_TYPE="$BUILD_TYPE" \
    -DATLAS_BUILD_TESTS=ON \
    2>&1 | tee -a "$LOG_FILE"
log "[INFO] Configure complete."

# ── Build ─────────────────────────────────────────────────────────
NPROC=$(nproc 2>/dev/null || sysctl -n hw.ncpu 2>/dev/null || echo 4)
log "[STEP] Building all targets (${NPROC} threads)..."
cmake --build "$BUILD_DIR" --parallel "$NPROC" \
    2>&1 | tee -a "$LOG_FILE"
log "[INFO] Build complete. Executables in: $BUILD_DIR/bin/"

echo "Finished: $(date '+%Y-%m-%d %H:%M:%S')" >> "$LOG_FILE"

# ── Optionally run tests ───────────────────────────────────────────
if [[ "${2:-}" == "--test" ]]; then
    log "[STEP] Running tests..."
    echo "" > "$TEST_LOG_FILE"
    ctest --test-dir "$BUILD_DIR" --output-on-failure \
        2>&1 | tee -a "$LOG_FILE" | tee -a "$TEST_LOG_FILE"
    log "[INFO] Tests complete. See $TEST_LOG_FILE"
fi
