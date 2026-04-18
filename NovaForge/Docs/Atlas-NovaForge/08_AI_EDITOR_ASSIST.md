# Atlas Engine — AI Editor Assist

## Overview

The AI system is an engine service that produces validated assets.
It operates inside the engine, not externally.

## Capabilities

- Context-aware asset generation
- Graph creation from prompts
- Procedural rules generation
- Validation of generated content
- Refactoring suggestions

## Architecture

```
Editor Request
  ↓
AIContextBuilder
  ↓
AIAggregator
  ├─ Local LLM
  ├─ Remote LLM A
  ├─ Remote LLM B
  ↓
Response Comparator
  ↓
Engine Validator
  ↓
Asset Generator
  ↓
Preview → Apply
```

## AI Request Types

- Graph Generation
- World Generation
- Code Assist
- Analysis

## Safety Rules

The AI system:
- Cannot touch core engine code
- Cannot write arbitrary disk files
- Cannot modify network rules without approval
- All generated assets must pass engine validation
- Dry-run generation mode available

## Validation Layer

All AI-generated content goes through:
1. Schema validation
2. Determinism checks
3. Network safety validation
4. Preview before application
