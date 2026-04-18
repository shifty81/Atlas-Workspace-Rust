#!/usr/bin/env bash
# verify_build.sh — Verifies that the NovaForge CMake project configures and
# builds end-to-end.
#
# Usage:
#   ./Scripts/verify_build.sh [BuildType]
#
# BuildType defaults to "Development" if not specified.
set -euo pipefail

REPO_ROOT="$(cd "$(dirname "$0")/.." && pwd)"
BUILD_TYPE="${1:-Development}"
BUILD_DIR="${REPO_ROOT}/build/verify-${BUILD_TYPE}"

echo "=== NovaForge Build Verification ==="
echo "Build type: ${BUILD_TYPE}"
echo "Build dir:  ${BUILD_DIR}"

# Map profile names to CMake build types
case "${BUILD_TYPE}" in
    Debug)       CMAKE_BUILD_TYPE="Debug" ;;
    Development) CMAKE_BUILD_TYPE="RelWithDebInfo" ;;
    Release)     CMAKE_BUILD_TYPE="Release" ;;
    *)           CMAKE_BUILD_TYPE="${BUILD_TYPE}" ;;
esac

# ── Step 1: Configure ────────────────────────────────────────────────────────
echo ""
echo "Step 1: CMake configure..."
cmake -S "${REPO_ROOT}/NovaForge" \
      -B "${BUILD_DIR}" \
      -DCMAKE_BUILD_TYPE="${CMAKE_BUILD_TYPE}" \
      -DBUILD_TESTING=ON

echo "  Configure: OK"

# ── Step 2: Build ────────────────────────────────────────────────────────────
echo ""
echo "Step 2: CMake build..."
cmake --build "${BUILD_DIR}" --parallel

echo "  Build: OK"

# ── Step 3: Run tests ────────────────────────────────────────────────────────
echo ""
echo "Step 3: CTest..."
TEST_RESULT=0
if ctest --test-dir "${BUILD_DIR}" --output-on-failure; then
    echo "  Tests: OK"
else
    echo "  Tests: FAILED"
    TEST_RESULT=1
fi

echo ""
if [ "${TEST_RESULT}" -ne 0 ]; then
    echo "Build verification complete (with test failures)."
    exit 1
else
    echo "Build verification complete."
fi
