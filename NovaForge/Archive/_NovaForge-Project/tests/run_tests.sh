#!/usr/bin/env bash
# run_tests.sh — Starter test lane for NovaForge project-level tests.
# Runs all project-level validation and test steps in sequence.
#
# Usage:
#   ./Tests/NovaForge.Tests/run_tests.sh
set -euo pipefail

REPO_ROOT="$(cd "$(dirname "$0")/../.." && pwd)"
TESTS_DIR="$(cd "$(dirname "$0")" && pwd)"

PASSED=0
FAILED=0
SKIPPED=0

run_step() {
    local name="$1"
    local cmd="$2"
    echo ""
    echo "────────────────────────────────────────"
    echo "Step: ${name}"
    echo "────────────────────────────────────────"
    if eval "${cmd}"; then
        echo "  Result: PASSED"
        PASSED=$((PASSED + 1))
    else
        echo "  Result: FAILED"
        FAILED=$((FAILED + 1))
    fi
}

echo "=== NovaForge Test Lane ==="

# ── 1. Project structure validation ──────────────────────────────────────────
run_step "Project structure validation" \
    "bash '${REPO_ROOT}/Scripts/validate_project.sh'"

# ── 2. Manifest validation ───────────────────────────────────────────────────
run_step "Manifest validation" \
    "bash '${TESTS_DIR}/test_manifest.sh'"

# ── 3. Content schema validation ─────────────────────────────────────────────
run_step "Content schema validation" \
    "bash '${REPO_ROOT}/Scripts/validate_content.sh'"

# ── 4. Bridge roundtrip test ─────────────────────────────────────────────────
run_step "Bridge roundtrip test" \
    "bash '${TESTS_DIR}/test_bridge_roundtrip.sh'"

# ── Summary ───────────────────────────────────────────────────────────────────
TOTAL=$((PASSED + FAILED + SKIPPED))
echo ""
echo "========================================"
echo "Test Lane Summary"
echo "========================================"
echo "  Total:   ${TOTAL}"
echo "  Passed:  ${PASSED}"
echo "  Failed:  ${FAILED}"
echo "  Skipped: ${SKIPPED}"
echo ""

if [ "${FAILED}" -gt 0 ]; then
    echo "Test lane: FAILED"
    exit 1
else
    echo "Test lane: PASSED"
    exit 0
fi
