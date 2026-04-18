#!/usr/bin/env bash
# test_manifest.sh — Validates novaforge.project.json against the manifest
# schema requirements defined in Docs/Specs/PROJECT_MANIFEST_SCHEMA.md.
set -euo pipefail

REPO_ROOT="$(cd "$(dirname "$0")/../.." && pwd)"
MANIFEST="${REPO_ROOT}/novaforge.project.json"

echo "Validating project manifest: ${MANIFEST}"

if [ ! -f "${MANIFEST}" ]; then
    echo "FAIL: manifest file not found"
    exit 1
fi

# Validate using python3 (available in CI and dev environments)
python3 -c "
import json, sys

manifest = json.load(open('${MANIFEST}'))
errors = []

# Required top-level fields
for field in ['projectId', 'displayName', 'projectType', 'manifestVersion',
              'atlasSuiteCompatibility', 'targets', 'paths', 'bridge', 'capabilities']:
    if field not in manifest:
        errors.append(f'missing required field: {field}')

# Manifest version
if manifest.get('manifestVersion') != 2:
    errors.append(f'expected manifestVersion 2, got {manifest.get(\"manifestVersion\")}')

# Project identity
if manifest.get('projectId') != 'novaforge':
    errors.append(f'expected projectId \"novaforge\", got \"{manifest.get(\"projectId\")}\"')

# Compatibility block
compat = manifest.get('atlasSuiteCompatibility', {})
if 'minimumVersion' not in compat:
    errors.append('atlasSuiteCompatibility.minimumVersion missing')
if 'projectContractVersion' not in compat:
    errors.append('atlasSuiteCompatibility.projectContractVersion missing')

# Targets
targets = manifest.get('targets', {})
for t in ['client', 'server', 'tests', 'validation']:
    if t not in targets:
        errors.append(f'missing target: {t}')

# Paths
paths = manifest.get('paths', {})
for p in ['repoRoot', 'projectRoot', 'sourceRoot', 'clientRoot', 'serverRoot',
          'contentRoot', 'configRoot', 'scriptsRoot', 'testsRoot', 'packagingRoot',
          'releaseRoot', 'docsRoot', 'integrationsRoot', 'aiAdapterRoot']:
    if p not in paths:
        errors.append(f'missing path: {p}')

# Bridge
bridge = manifest.get('bridge', {})
if bridge.get('transport') != 'http':
    errors.append(f'expected bridge transport \"http\", got \"{bridge.get(\"transport\")}\"')
if 'baseUrl' not in bridge:
    errors.append('bridge.baseUrl missing')
if 'timeoutMs' not in bridge:
    errors.append('bridge.timeoutMs missing')

# Capabilities (minimum set)
capabilities = manifest.get('capabilities', [])
for cap in ['project.info', 'build.run', 'files.open', 'analysis.lint']:
    if cap not in capabilities:
        errors.append(f'missing minimum capability: {cap}')

if errors:
    print('Manifest validation FAILED:')
    for e in errors:
        print(f'  - {e}')
    sys.exit(1)
else:
    print('Manifest validation PASSED')
    print(f'  projectId:       {manifest[\"projectId\"]}')
    print(f'  manifestVersion: {manifest[\"manifestVersion\"]}')
    print(f'  capabilities:    {len(capabilities)}')
    print(f'  paths:           {len(paths)}')
    sys.exit(0)
"
