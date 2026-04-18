#!/usr/bin/env bash
# =================================================================
# Atlas Build Dependency Verification
# =================================================================
#
# Verifies that Atlas executables follow dependency rules:
# - AtlasServer must not link graphics libraries
# - Module boundaries are respected
# - Engine is self-contained
#
# Usage: ./tools/verify_dependencies.sh

set -e

SCRIPT_DIR="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)"
PROJECT_ROOT="$(cd "${SCRIPT_DIR}/.." && pwd)"
BUILD_DIR="${PROJECT_ROOT}/build"

GREEN='\033[0;32m'
RED='\033[0;31m'
YELLOW='\033[1;33m'
NC='\033[0m' # No Color

echo "═══════════════════════════════════════════════════"
echo "  Atlas Dependency Verification"
echo "═══════════════════════════════════════════════════"
echo ""

# Check if build directory exists
if [ ! -d "$BUILD_DIR" ]; then
    echo -e "${RED}✗${NC} Build directory not found: $BUILD_DIR"
    echo "  Run './build.sh' first"
    exit 1
fi

SUCCESS_COUNT=0
TOTAL_CHECKS=0

# =================================================================
# Check 1: AtlasServer must not link graphics libraries
# =================================================================
echo -n "Checking AtlasServer (headless)... "
TOTAL_CHECKS=$((TOTAL_CHECKS + 1))

SERVER_BIN="${BUILD_DIR}/server/AtlasServer"
if [ ! -f "$SERVER_BIN" ]; then
    echo -e "${YELLOW}SKIP${NC} (not built)"
else
    FORBIDDEN_LIBS=$(ldd "$SERVER_BIN" 2>&1 | grep -i -E "(vulkan|opengl|x11|xcb|wayland|glfw|sdl)" || true)
    
    if [ -z "$FORBIDDEN_LIBS" ]; then
        echo -e "${GREEN}✓${NC} No graphics libraries"
        SUCCESS_COUNT=$((SUCCESS_COUNT + 1))
    else
        echo -e "${RED}✗${NC} Graphics libraries found:"
        echo "$FORBIDDEN_LIBS"
    fi
fi

# =================================================================
# Check 2: AtlasEngine library exists
# =================================================================
echo -n "Checking AtlasEngine library... "
TOTAL_CHECKS=$((TOTAL_CHECKS + 1))

ENGINE_LIB="${BUILD_DIR}/engine/libAtlasEngine.a"
if [ -f "$ENGINE_LIB" ]; then
    echo -e "${GREEN}✓${NC} Found"
    SUCCESS_COUNT=$((SUCCESS_COUNT + 1))
else
    echo -e "${RED}✗${NC} Not found"
fi

# =================================================================
# Check 3: AtlasGameplay library exists
# =================================================================
echo -n "Checking AtlasGameplay library... "
TOTAL_CHECKS=$((TOTAL_CHECKS + 1))

GAMEPLAY_LIB="${BUILD_DIR}/modules/atlas_gameplay/libAtlasGameplay.a"
if [ -f "$GAMEPLAY_LIB" ]; then
    echo -e "${GREEN}✓${NC} Found"
    SUCCESS_COUNT=$((SUCCESS_COUNT + 1))
else
    echo -e "${RED}✗${NC} Not found"
fi

# =================================================================
# Check 4: Client executable exists
# =================================================================
echo -n "Checking AtlasClient... "
TOTAL_CHECKS=$((TOTAL_CHECKS + 1))

CLIENT_BIN="${BUILD_DIR}/client/AtlasClient"
if [ -f "$CLIENT_BIN" ]; then
    echo -e "${GREEN}✓${NC} Found"
    SUCCESS_COUNT=$((SUCCESS_COUNT + 1))
else
    echo -e "${YELLOW}SKIP${NC} (not built)"
fi

# =================================================================
# Check 5: Runtime executable exists
# =================================================================
echo -n "Checking AtlasRuntime... "
TOTAL_CHECKS=$((TOTAL_CHECKS + 1))

RUNTIME_BIN="${BUILD_DIR}/runtime/AtlasRuntime"
if [ -f "$RUNTIME_BIN" ]; then
    echo -e "${GREEN}✓${NC} Found"
    SUCCESS_COUNT=$((SUCCESS_COUNT + 1))
else
    echo -e "${YELLOW}SKIP${NC} (not built)"
fi

# =================================================================
# Check 6: Editor executable exists
# =================================================================
echo -n "Checking AtlasEditor... "
TOTAL_CHECKS=$((TOTAL_CHECKS + 1))

EDITOR_BIN="${BUILD_DIR}/editor/AtlasEditor"
if [ -f "$EDITOR_BIN" ]; then
    echo -e "${GREEN}✓${NC} Found"
    SUCCESS_COUNT=$((SUCCESS_COUNT + 1))
else
    echo -e "${YELLOW}SKIP${NC} (not built)"
fi

# =================================================================
# Check 7: No simulation code uses forbidden APIs
# =================================================================
echo -n "Checking contract violations... "
TOTAL_CHECKS=$((TOTAL_CHECKS + 1))

cd "$PROJECT_ROOT"
if python3 tools/contract_scan.py --path engine > /dev/null 2>&1; then
    echo -e "${GREEN}✓${NC} No violations"
    SUCCESS_COUNT=$((SUCCESS_COUNT + 1))
else
    echo -e "${RED}✗${NC} Contract violations found"
    echo "  Run: python3 tools/contract_scan.py --path engine"
fi

# =================================================================
# Summary
# =================================================================
echo ""
echo "═══════════════════════════════════════════════════"
echo "  Results: ${SUCCESS_COUNT}/${TOTAL_CHECKS} checks passed"
echo "═══════════════════════════════════════════════════"

if [ $SUCCESS_COUNT -eq $TOTAL_CHECKS ]; then
    echo -e "${GREEN}✓ All dependency rules verified${NC}"
    exit 0
else
    FAILED=$((TOTAL_CHECKS - SUCCESS_COUNT))
    echo -e "${RED}✗ ${FAILED} check(s) failed${NC}"
    exit 1
fi
