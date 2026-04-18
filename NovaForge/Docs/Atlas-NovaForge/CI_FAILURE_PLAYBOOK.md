# Atlas CI Failure Playbook

This document explains what broke, why it broke, and how to fix it.

CI failures are not obstacles — they are Atlas telling you the truth.
If CI fails, the platform is protecting you.

---

## ❌ "System missing ATLAS_SYSTEM_TRAITS"

**Cause:**
A system was added without a determinism contract.

**Fix:**
Add the traits declaration to your system:

```cpp
ATLAS_SYSTEM_TRAITS(MySystem,
    (SystemTraits{ "MySystem", true, true, true, __FILE__ })
);
```

---

## ❌ "Determinism lint failed"

**Cause:**
Forbidden API used in a deterministic system. The contract scanner
(`tools/contract_scan.py`) detected a violation.

**Common Fixes:**

| Violation | Replacement |
|-----------|-------------|
| `std::unordered_map` | `std::map` |
| `std::unordered_set` | `std::set` |
| `rand()` | `DeterministicRNG` |
| `std::chrono` | `AtlasTick` |
| `float == float` | Epsilon comparison or fixed-point |
| `__m128` / `_mm_` | Atlas math wrappers |

**Full rule set:** See `tools/determinism_rules.yaml`

---

## ❌ "Replay hash mismatch"

**Cause:**
Golden replay hashes differ between runs. This indicates nondeterministic
logic, a missing migration, or a script hash change.

**Fix:**

1. Run the replay locally in the Editor
2. Open the Replay Inspector panel to find the divergence frame
3. Use the State Hash Diff panel for per-system hash comparison
4. Fix the nondeterministic logic OR add a schema migration
5. Re-run the replay and update the golden hash

**Tools:**
- `tools/replay_inspector.py` — Inspect replay contents
- `tools/replay_minimizer.py` — Reduce to minimal failing replay
- `tools/state_diff_viewer.py` — Compare state snapshots

---

## ❌ "Contract scanner violation"

**Cause:**
Source code contains a forbidden API or banned library usage. The
contract scanner runs against all simulation directories and the
entire codebase for banned UI libraries.

**Fix:**

1. Read the violation message — it tells you which file and pattern
2. Replace the forbidden API with the Atlas-approved alternative
3. If the usage is legitimate (e.g. tooling code), move it to an
   exempt directory or add the file to `SKIP_FILES` in `contract_scan.py`

**Reference:** `docs/ATLAS_CORE_CONTRACT.md`

---

## ❌ "Project contract validation failed"

**Cause:**
The `.atlas` project file requires guarantees the engine does not provide.

**Fix:**

- Update the `.atlas` project's `requires` section to match engine capabilities
- OR upgrade the engine to provide the required determinism profile

---

## ❌ "ABI validation failed"

**Cause:**
Binary compatibility was broken between the project and engine.

**Fix:**

- Restore the ABI layout
- OR increment the ABI version in the `.atlas` project file
- OR mark the change as non-compatible and update all dependent projects

---

## ❌ "Cross-platform replay divergence"

**Cause:**
The same replay produces different hashes on different platforms. This
usually means platform-dependent floating-point behavior or SIMD
intrinsics leaked into simulation code.

**Fix:**

1. Check for platform-specific math (`__m128`, `_mm_` intrinsics)
2. Check for floating-point mode differences (FP strict vs relaxed)
3. Ensure all simulation math uses Atlas deterministic wrappers
4. Run the hash comparison dashboard in CI artifacts

---

## ❌ "Build failed"

**Cause:**
Compilation error. Most common in Atlas:

- Missing include for a contract header
- ODR (One Definition Rule) violation from duplicate type names
- Missing source file in `CMakeLists.txt`

**Fix:**

1. Read the compiler error carefully
2. Check for duplicate struct/class names across headers
3. Ensure new `.cpp` files are listed in the appropriate `CMakeLists.txt`
4. Build locally with `cmake -B build && cmake --build build`

---

## ❌ "Test failure"

**Cause:**
One or more tests failed. Common causes:

- Logic bug in new code
- API change broke existing test expectations
- Non-deterministic test (timing-dependent)

**Fix:**

1. Run the failing test locally: `./build/tests/AtlasTests`
2. Check if the test is timing-dependent (should not be in Atlas)
3. Fix the logic or update the test expectation
4. Verify all tests pass before pushing

---

## ❌ "Crash reporter triggered"

**Cause:**
The CI workflow detected a crash during headless execution.

**Fix:**

1. Download the `.atlascrash` bundle from CI artifacts
2. The bundle contains: state snapshot, replay up to crash, hash ladder
3. Use `tools/crash_reporter.py` to inspect the bundle
4. Fix the crash and add a regression test

---

## General Debugging Workflow

```
1. Read the CI log carefully
2. Reproduce locally
3. Use Atlas tools (replay inspector, state diff viewer)
4. Fix the root cause (not the symptom)
5. Add a test to prevent regression
6. Verify CI passes
```

---

## Important Rule

> CI failures are not obstacles.
> They are Atlas telling you the truth.
>
> If CI fails, the platform is protecting you.

See also:
- `docs/ATLAS_CORE_CONTRACT.md`
- `docs/ATLAS_DETERMINISM_ENFORCEMENT.md`
- `docs/ATLAS_LOCKDOWN_CHECKLIST.md`
