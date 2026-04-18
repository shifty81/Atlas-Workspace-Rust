# Snippet: MSVC Runtime Library Fix (CMakeLists.txt)

Prevents LNK4098 by enforcing consistent MSVC runtime across all targets.
Apply this at the top of the root CMakeLists.txt BEFORE the `project()` call.

**Problem:** When mixing static libs that use `/MT` with libs that use `/MD`,
the linker emits `LNK4098: defaultlib 'LIBCMT' conflicts with use of other libs`.
This is common when vcpkg-built dependencies use a different runtime than the
project default.

**Solution:** Lock all targets to `MultiThreadedDLL` (release) / `MultiThreadedDebugDLL`
(debug) via CMake policy CMP0091.

```cmake
cmake_minimum_required(VERSION 3.20)

# Must be set before project() to take effect via CMP0091
cmake_policy(SET CMP0091 NEW)

project(NovaForge VERSION 0.1.0 LANGUAGES CXX)

# Enforce a consistent MSVC runtime library across all targets to avoid LNK4098
if(MSVC)
    set(CMAKE_MSVC_RUNTIME_LIBRARY "MultiThreaded$<$<CONFIG:Debug>:Debug>DLL")
endif()
```

**What this does:**
- `CMP0091 NEW` enables the `CMAKE_MSVC_RUNTIME_LIBRARY` variable
- The generator expression selects `/MDd` for Debug and `/MD` for Release
- Applies to ALL targets in the project, including those added via FetchContent
- Must appear before `project()` so the policy is in effect when CMake starts
  processing target definitions

**When to apply:** Every root CMakeLists.txt that uses MSVC and links against
vcpkg packages or FetchContent dependencies.
