# Source Import Policy

## Allowed
- NovaForge-only runtime code
- project-specific content
- project-specific data schemas
- project-specific validation logic
- smoke-test and dev-world assets that are clearly project-local

## Forbidden
- Atlas Suite shell code
- generic tooling
- repo refactor engines
- workspace services
- generic editor hosts
- unrelated archived source trees

## Import Decisions
Each candidate file should be classified as:
- KEEP_AS_BASE
- MERGE
- CHERRY_PICK
- ARCHIVE_ONLY
- DROP
