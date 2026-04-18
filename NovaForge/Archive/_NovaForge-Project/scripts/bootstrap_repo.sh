#!/usr/bin/env bash
set -euo pipefail

REPO_NAME="${1:-NovaForge}"
REMOTE_URL="${2:-}"

echo "NovaForge bootstrap starting"
echo "Repo name: ${REPO_NAME}"

if [ ! -d ".git" ]; then
  git init >/dev/null 2>&1
  echo "Initialized git repository"
else
  echo "Git repository already initialized"
fi

git add .

if git commit -m "Initial NovaForge spec + scaffold bootstrap" >/dev/null 2>&1; then
  echo "Initial commit created"
else
  echo "Initial commit not created. This usually means git user identity is not configured or no staged changes were present."
fi

if [ -n "${REMOTE_URL}" ]; then
  if git remote | grep -q "^origin$"; then
    git remote set-url origin "${REMOTE_URL}"
    echo "Updated origin remote"
  else
    git remote add origin "${REMOTE_URL}"
    echo "Added origin remote"
  fi
fi

echo
echo "Next suggested commands:"
echo "  git status"
echo "  git branch -M main"
if [ -n "${REMOTE_URL}" ]; then
  echo "  git push -u origin main"
fi
