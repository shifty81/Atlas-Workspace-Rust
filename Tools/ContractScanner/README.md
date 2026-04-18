# ContractScanner — Code Analysis Tool

`Tools/ContractScanner/` — C++ static analysis binary.

ContractScanner scans C++ source files for common defensive-coding
violations: unguarded pointer dereferences, ignored return values, raw
C-style casts, and missing null checks on `optional`/pointer results.
Findings are written to the pipeline so the editor can surface them in a
"Contract Issues" panel with one-click jump-to-definition.

---

## Tool Roadmap

| ID   | Milestone | Goal |
|------|-----------|------|
| CS-1 | Single-file scan | Detect missing null checks, ignored return values, C-style casts in one `.cpp` |
| CS-2 | Project-wide scan | Reads source index from pipeline; scans all files in the project |
| CS-3 | Pipeline output | Findings written to `pipeline/changes/contractscanner_ContractIssue_<ts>.change.json` |
| CS-4 | Editor panel | "Contract Issues" list in editor; click → jump to file+line via `CodeNavigator` |
| CS-5 | CI gate | Exit code 1 if any critical-severity issue is found; wired into GitHub Actions |

---

## Pipeline integration

- Reads: source file paths from `manifest.json` or `pipeline/` source-index
- Writes: `pipeline/changes/<timestamp>_contractscanner_ContractIssue.change.json`

### Finding JSON schema

```json
{
  "tool": "ContractScanner",
  "event_type": "ContractIssue",
  "path": "Source/Game/src/Game.cpp",
  "timestamp": 1712282858000,
  "metadata": "line=42 severity=warning rule=unguarded_ptr message=pointer dereferenced without null check"
}
```

---

## Usage (CS-1 / CS-2 target)

```
contractscanner scan Source/Game/src/Game.cpp
contractscanner scan-project --manifest .novaforge/manifest.json
```
