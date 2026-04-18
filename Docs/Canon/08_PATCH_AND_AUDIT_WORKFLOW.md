# Patch and Audit Workflow

## Patch Application

- Patch application must be deterministic
- Uninstall must exist for every shell integration
- No one-way registry changes
- Context menu registration paired with removal scripts

## Audit Workflow

- Audit snapshots must not include `.git/`, `build/`, `.vs/`, generated artifacts
- Each audit should verify current README and canon docs are up to date
- Cleanup progress must be measurable against inventory docs

## Repo Audit Tool

The audit/refactor utility should classify:

- Dead code
- Reusable snippets
- Project-specific code
- Universalizable code
- Stale naming
- Backend stubs

Outputs should map to the real workspace cleanup plan.

## Scripts

| Script | Purpose |
|--------|---------|
| `Scripts/build.sh` | Linux/macOS build |
| `Scripts/build_all.sh` | Full build |
| `Scripts/build_win.cmd` | Windows build |
| `Scripts/generate_vs_solution.bat` | VS solution generation |
| `Scripts/generate_vs_solution.ps1` | VS solution generation (PowerShell) |
| `Scripts/validate_project.sh` | Project validation |
| `build.cmd` | Root Windows build with MSBuild |
