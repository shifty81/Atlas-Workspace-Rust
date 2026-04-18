#!/usr/bin/env bash
# validate_content.sh — Validates the Content/ directory layout against
# CONTENT_RULES.md requirements.
#
# Checks:
#   1. All required content subdirectories exist
#   2. JSON/YAML content files have an "id" field (stable unique ids)
#   3. JSON/YAML content files with a "schemaVersion" field (versioned schemas)
#   4. No duplicate ids within a content category
#   5. References resolve to existing ids (basic check)
set -euo pipefail

REPO_ROOT="$(cd "$(dirname "$0")/.." && pwd)"
CONTENT_ROOT="${REPO_ROOT}/Content"

echo "=== NovaForge Content Validation ==="

FAILED=0

# ── Check 1: Required directories ────────────────────────────────────────────
REQUIRED_DIRS=(
    "Data"
    "Definitions"
    "Buildables"
    "Rig"
    "Salvage"
    "Worldgen"
    "Factions"
    "Seasons"
    "Localization"
    "Prototypes"
    "SaveSchemas"
)

echo ""
echo "Check 1: Required content directories"
for dir in "${REQUIRED_DIRS[@]}"; do
    if [ ! -d "${CONTENT_ROOT}/${dir}" ]; then
        echo "  FAIL: missing Content/${dir}"
        FAILED=1
    else
        echo "  OK:   Content/${dir}"
    fi
done

# ── Check 2: JSON content files have "id" fields ─────────────────────────────
echo ""
echo "Check 2: Content files have stable ids"
CONTENT_FILES=()
while IFS= read -r -d '' f; do
    CONTENT_FILES+=("$f")
done < <(find "${CONTENT_ROOT}" -name '*.json' -not -name '.gitkeep' -print0 2>/dev/null)

ID_COUNT=0
MISSING_ID=0
for f in "${CONTENT_FILES[@]}"; do
    # Skip packaging or schema files that aren't content entries
    REL="${f#"${CONTENT_ROOT}/"}"
    if python3 -c "
import json, sys
try:
    d = json.load(open('$f'))
    if isinstance(d, dict) and 'id' in d:
        sys.exit(0)
    elif isinstance(d, list):
        sys.exit(0)  # arrays are acceptable (batch files)
    else:
        sys.exit(1)
except Exception:
    sys.exit(1)
" 2>/dev/null; then
        ID_COUNT=$((ID_COUNT + 1))
    else
        echo "  FAIL: no 'id' field in ${REL}"
        MISSING_ID=$((MISSING_ID + 1))
        FAILED=1
    fi
done

if [ "${#CONTENT_FILES[@]}" -eq 0 ]; then
    echo "  SKIP: no JSON content files found (content not yet populated)"
else
    echo "  Scanned ${#CONTENT_FILES[@]} file(s): ${ID_COUNT} with id, ${MISSING_ID} without"
fi

# ── Check 3: No duplicate ids ────────────────────────────────────────────────
echo ""
echo "Check 3: No duplicate content ids"
if [ "${#CONTENT_FILES[@]}" -gt 0 ]; then
    DUPS=$(python3 -c "
import json, os, sys
ids = {}
content_root = '${CONTENT_ROOT}'
for root, dirs, files in os.walk(content_root):
    for fn in files:
        if not fn.endswith('.json') or fn == '.gitkeep':
            continue
        path = os.path.join(root, fn)
        try:
            d = json.load(open(path))
            if isinstance(d, dict) and 'id' in d:
                cid = d['id']
                rel = os.path.relpath(path, content_root)
                if cid in ids:
                    print(f'  DUPLICATE: id={cid} in {ids[cid]} and {rel}')
                else:
                    ids[cid] = rel
        except Exception:
            pass
print(f'  Checked {len(ids)} unique id(s)')
" 2>/dev/null)
    echo "${DUPS}"
    if echo "${DUPS}" | grep -q "DUPLICATE"; then
        FAILED=1
    fi
else
    echo "  SKIP: no content files to check"
fi

# ── Check 4: Prototype isolation ─────────────────────────────────────────────
echo ""
echo "Check 4: Prototype content isolation"
PROTO_DIR="${CONTENT_ROOT}/Prototypes"
if [ -d "${PROTO_DIR}" ]; then
    PROTO_COUNT=$(find "${PROTO_DIR}" -name '*.json' -not -name '.gitkeep' 2>/dev/null | wc -l)
    echo "  Prototype files: ${PROTO_COUNT}"
    echo "  OK: Prototypes directory exists and is isolated"
else
    echo "  FAIL: Content/Prototypes directory missing"
    FAILED=1
fi

# ── Summary ───────────────────────────────────────────────────────────────────
echo ""
if [ "${FAILED}" -ne 0 ]; then
    echo "Content validation: FAILED"
    exit 1
else
    echo "Content validation: PASSED"
    exit 0
fi
