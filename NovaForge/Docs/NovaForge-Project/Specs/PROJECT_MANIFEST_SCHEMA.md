# Project Manifest Schema

## Filename
`novaforge.project.json`

## Version
Current manifest version: **2**

## Minimum Shape
```json
{
  "projectId": "novaforge",
  "displayName": "NovaForge",
  "projectType": "game",
  "manifestVersion": 2,
  "atlasSuiteCompatibility": {
    "minimumVersion": "0.1.0",
    "projectContractVersion": 2
  },
  "targets": {
    "client": "NovaForge.Client",
    "server": "NovaForge.Server",
    "tests": "NovaForge.Tests",
    "validation": "NovaForge.Validation"
  },
  "paths": {
    "repoRoot": ".",
    "projectRoot": "NovaForge",
    "sourceRoot": "NovaForge",
    "clientRoot": "NovaForge/Client",
    "serverRoot": "NovaForge/Server",
    "contentRoot": "Content",
    "configRoot": "Config",
    "scriptsRoot": "Scripts",
    "testsRoot": "Tests",
    "packagingRoot": "Packaging",
    "releaseRoot": "Release",
    "docsRoot": "Docs",
    "integrationsRoot": "Integrations",
    "aiAdapterRoot": "AtlasAI/ProjectAdapters/NovaForge"
  },
  "bridge": {
    "transport": "http",
    "baseUrl": "http://127.0.0.1:8005",
    "timeoutMs": 8000
  },
  "capabilities": [
    "project.info",
    "build.run",
    "files.open",
    "analysis.lint"
  ]
}
```

## Bridge Protocol
Atlas Suite connects to NovaForge via the bridge config. No shared assemblies — all communication is over the bridge protocol.

## Capabilities
The `capabilities` array declares what operations NovaForge supports when hosted by Atlas Suite.
