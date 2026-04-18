#!/usr/bin/env bash
# package.sh — Packaging entry point for NovaForge.
# Reads a packaging profile from Packaging/Profiles/ and runs the corresponding
# CMake build, then gathers artifacts into the output directory.
#
# Usage:
#   ./Scripts/package.sh <ProfileId>
#
# Example:
#   ./Scripts/package.sh ClientDev
#   ./Scripts/package.sh ClientRelease
#   ./Scripts/package.sh ServerRelease
set -euo pipefail

REPO_ROOT="$(cd "$(dirname "$0")/.." && pwd)"
PROFILE_ID="${1:-}"

if [ -z "${PROFILE_ID}" ]; then
    echo "Usage: $0 <ProfileId>"
    echo "Available profiles:"
    for f in "${REPO_ROOT}/Packaging/Profiles/"*.json; do
        basename "$f" .json
    done
    exit 1
fi

PROFILE_PATH="${REPO_ROOT}/Packaging/Profiles/${PROFILE_ID}.json"

if [ ! -f "${PROFILE_PATH}" ]; then
    echo "Error: packaging profile not found: ${PROFILE_PATH}"
    exit 1
fi

echo "=== NovaForge Packaging ==="
echo "Profile: ${PROFILE_ID}"
echo "Profile path: ${PROFILE_PATH}"

# ── Parse profile fields ──────────────────────────────────────────────────────
# Uses python3 for JSON parsing (available on all CI and dev environments).
read_field() {
    python3 -c "import json,sys; d=json.load(open('${PROFILE_PATH}')); print(d.get('$1',''))"
}

read_array() {
    python3 -c "
import json
d = json.load(open('${PROFILE_PATH}'))
for v in d.get('$1', []):
    print(v)
"
}

BUILD_PROFILE="$(read_field buildProfile)"
OUTPUT_DIR="${REPO_ROOT}/$(read_field outputDir)"
INCLUDE_SYMBOLS="$(read_field includeSymbols)"

echo "Build profile: ${BUILD_PROFILE}"
echo "Output dir:    ${OUTPUT_DIR}"

# ── CMake configure + build ───────────────────────────────────────────────────
BUILD_DIR="${REPO_ROOT}/build/${PROFILE_ID}"
CMAKE_ARGS=()
while IFS= read -r arg; do
    [ -n "$arg" ] && CMAKE_ARGS+=("$arg")
done < <(read_array cmakeArgs)

echo ""
echo "Configuring CMake (${BUILD_DIR})..."
cmake -S "${REPO_ROOT}/NovaForge" -B "${BUILD_DIR}" "${CMAKE_ARGS[@]}"

echo ""
echo "Building..."
cmake --build "${BUILD_DIR}" --parallel

# ── Gather artifacts ──────────────────────────────────────────────────────────
echo ""
echo "Collecting artifacts into ${OUTPUT_DIR}..."
mkdir -p "${OUTPUT_DIR}"

while IFS= read -r artifact; do
    [ -z "$artifact" ] && continue
    SRC="${BUILD_DIR}/${artifact}"
    if [ -e "${SRC}" ]; then
        cp -r "${SRC}" "${OUTPUT_DIR}/"
        echo "  copied: ${artifact}"
    else
        echo "  warning: artifact not found: ${SRC}"
    fi
done < <(read_array artifacts)

# ── Copy content directories ─────────────────────────────────────────────────
INCLUDE_CONTENT="$(read_field includeContentRoot)"
if [ "${INCLUDE_CONTENT}" = "True" ] || [ "${INCLUDE_CONTENT}" = "true" ]; then
    CONTENT_OUT="${OUTPUT_DIR}/Content"
    mkdir -p "${CONTENT_OUT}"
    while IFS= read -r cdir; do
        [ -z "$cdir" ] && continue
        SRC="${REPO_ROOT}/${cdir}"
        if [ -d "${SRC}" ]; then
            DEST="${CONTENT_OUT}/$(basename "$cdir")"
            mkdir -p "${DEST}"
            cp -r "${SRC}/." "${DEST}/"
            echo "  content: ${cdir}"
        fi
    done < <(read_array contentDirs)
fi

# ── Copy version metadata ────────────────────────────────────────────────────
if [ -f "${REPO_ROOT}/Release/version.json" ]; then
    cp "${REPO_ROOT}/Release/version.json" "${OUTPUT_DIR}/version.json"
    echo "  metadata: version.json"
fi

echo ""
echo "Packaging complete: ${PROFILE_ID}"
echo "Output: ${OUTPUT_DIR}"
