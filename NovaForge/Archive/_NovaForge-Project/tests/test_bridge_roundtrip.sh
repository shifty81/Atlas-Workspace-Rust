#!/usr/bin/env bash
# test_bridge_roundtrip.sh — Validates the bridge configuration roundtrip:
# Suite reads manifest → resolves bridge config → can reach bridge endpoint.
#
# This test validates the configuration chain without requiring a live bridge
# server. When the bridge is running, it also performs a connectivity check.
set -euo pipefail

REPO_ROOT="$(cd "$(dirname "$0")/../.." && pwd)"
MANIFEST="${REPO_ROOT}/novaforge.project.json"

echo "=== Bridge Roundtrip Test ==="

if [ ! -f "${MANIFEST}" ]; then
    echo "FAIL: manifest not found"
    exit 1
fi

ERRORS=0

# ── Step 1: Read bridge config from manifest ─────────────────────────────────
echo ""
echo "Step 1: Parse bridge config from manifest"
BRIDGE_CONFIG=$(python3 -c "
import json, sys
m = json.load(open('${MANIFEST}'))
b = m.get('bridge', {})
if not b:
    print('ERROR: no bridge config in manifest', file=sys.stderr)
    sys.exit(1)
print(f'transport={b.get(\"transport\",\"\")}')
print(f'baseUrl={b.get(\"baseUrl\",\"\")}')
print(f'timeoutMs={b.get(\"timeoutMs\",\"\")}')
")

eval "${BRIDGE_CONFIG}"
echo "  transport: ${transport}"
echo "  baseUrl:   ${baseUrl}"
echo "  timeoutMs: ${timeoutMs}"

if [ -z "${transport}" ] || [ -z "${baseUrl}" ] || [ -z "${timeoutMs}" ]; then
    echo "  FAIL: incomplete bridge config"
    ERRORS=$((ERRORS + 1))
else
    echo "  OK: bridge config complete"
fi

# ── Step 2: Validate capabilities chain ──────────────────────────────────────
echo ""
echo "Step 2: Validate capabilities match bridge protocol"
python3 -c "
import json, sys
m = json.load(open('${MANIFEST}'))
caps = m.get('capabilities', [])
bridge = m.get('bridge', {})

# Verify bridge is HTTP (required by HOSTED_PROJECT_CONTRACT.md)
if bridge.get('transport') != 'http':
    print('  FAIL: bridge transport must be http')
    sys.exit(1)

# Verify minimum capabilities are declared
min_caps = ['project.info', 'build.run']
missing = [c for c in min_caps if c not in caps]
if missing:
    print(f'  FAIL: missing capabilities for bridge: {missing}')
    sys.exit(1)

print(f'  OK: {len(caps)} capabilities declared, minimum set present')
" || ERRORS=$((ERRORS + 1))

# ── Step 3: Validate safety settings ─────────────────────────────────────────
echo ""
echo "Step 3: Validate safety settings"
python3 -c "
import json, sys
m = json.load(open('${MANIFEST}'))
safety = m.get('safety', {})
if not safety:
    print('  FAIL: no safety settings')
    sys.exit(1)

if safety.get('allowWriteActions') is not False:
    print('  FAIL: allowWriteActions should be false')
    sys.exit(1)

if safety.get('requireReviewForCodegen') is not True:
    print('  FAIL: requireReviewForCodegen should be true')
    sys.exit(1)

whitelist = safety.get('whitelistedToolActions', [])
print(f'  OK: safety settings valid (writeActions=false, reviewRequired=true, {len(whitelist)} whitelisted actions)')
" || ERRORS=$((ERRORS + 1))

# ── Step 4: Connectivity check (non-fatal) ───────────────────────────────────
echo ""
echo "Step 4: Bridge connectivity check"
TIMEOUT_SECS=$((timeoutMs / 1000))
[ "${TIMEOUT_SECS}" -lt 1 ] && TIMEOUT_SECS=1

if command -v curl >/dev/null 2>&1; then
    if curl -sf --max-time "${TIMEOUT_SECS}" "${baseUrl}" >/dev/null 2>&1; then
        echo "  OK: bridge endpoint reachable at ${baseUrl}"
    else
        echo "  SKIP: bridge endpoint not reachable (server not running — expected in CI)"
    fi
else
    echo "  SKIP: curl not available"
fi

# ── Summary ───────────────────────────────────────────────────────────────────
echo ""
if [ "${ERRORS}" -gt 0 ]; then
    echo "Bridge roundtrip test: FAILED (${ERRORS} error(s))"
    exit 1
else
    echo "Bridge roundtrip test: PASSED"
    exit 0
fi
