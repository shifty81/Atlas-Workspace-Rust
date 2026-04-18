#!/usr/bin/env bash
# fetch_novaforge_assets.sh
#
# Downloads Nova-Forge assets (including Git LFS blobs) from
# https://github.com/shifty81/Nova-Forge and places them into
# novaforge-assets/ in the repository root.
#
# Requirements:
#   - git (with git-lfs extension installed)
#   - Internet access to github.com
#
# Usage:
#   bash Scripts/fetch_novaforge_assets.sh [--force]
#
# Options:
#   --force    Re-clone even if the tmp clone already exists.
#
# The assets are NOT committed to this repository. This script must be
# run once per developer machine (or after a Nova-Forge upstream update).
# ---------------------------------------------------------------------------
set -euo pipefail

SCRIPT_DIR="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)"
REPO_ROOT="$(dirname "$SCRIPT_DIR")"
DEST_DIR="$REPO_ROOT/novaforge-assets"
TMP_DIR="/tmp/nova-forge-lfs-clone"
NOVA_FORGE_REPO="https://github.com/shifty81/Nova-Forge.git"

FORCE=false
for arg in "$@"; do
    case "$arg" in
        --force) FORCE=true ;;
        *) echo "Unknown argument: $arg"; exit 1 ;;
    esac
done

# ── Preflight checks ────────────────────────────────────────────────────────

if ! command -v git &>/dev/null; then
    echo "ERROR: git is not installed." >&2
    exit 1
fi

if ! git lfs version &>/dev/null; then
    echo "ERROR: git-lfs is not installed." >&2
    echo "  Install with: sudo apt install git-lfs  OR  brew install git-lfs" >&2
    exit 1
fi

# ── Clone / update ──────────────────────────────────────────────────────────

if [ -d "$TMP_DIR" ] && [ "$FORCE" = false ]; then
    echo "[fetch-assets] Using existing clone at $TMP_DIR"
    echo "  (pass --force to re-clone)"
    cd "$TMP_DIR"
    echo "[fetch-assets] Fetching latest changes + LFS objects..."
    git fetch origin
    git checkout origin/HEAD -- assets/
    git lfs pull
else
    if [ -d "$TMP_DIR" ]; then
        echo "[fetch-assets] Removing existing clone (--force)..."
        rm -rf "$TMP_DIR"
    fi

    echo "[fetch-assets] Cloning Nova-Forge (sparse, LFS enabled)..."
    git clone \
        --filter=blob:none \
        --no-checkout \
        --depth=1 \
        "$NOVA_FORGE_REPO" \
        "$TMP_DIR"

    cd "$TMP_DIR"
    git lfs install
    # Sparse checkout: only the assets/ subtree
    git sparse-checkout init --cone
    git sparse-checkout set assets
    git checkout HEAD
    echo "[fetch-assets] Fetching LFS objects..."
    git lfs pull
fi

# ── Sync into novaforge-assets/ ─────────────────────────────────────────────

echo "[fetch-assets] Copying assets → $DEST_DIR ..."
mkdir -p "$DEST_DIR"

# Use rsync if available, otherwise fall back to cp
if command -v rsync &>/dev/null; then
    rsync -a --delete \
        --exclude="README.md" \
        "$TMP_DIR/assets/" \
        "$DEST_DIR/"
else
    cp -r "$TMP_DIR/assets/." "$DEST_DIR/"
fi

# Restore the .gitkeep files that git would have deleted
for subdir in common voxygen world server; do
    touch "$DEST_DIR/$subdir/.gitkeep" 2>/dev/null || true
done

echo "[fetch-assets] Done. Assets are in: $DEST_DIR"
echo ""
echo "  Set NOVAFORGE_ASSETS_DIR=$DEST_DIR in your environment"
echo "  (or leave unset — atlas-asset searches there by default)."
