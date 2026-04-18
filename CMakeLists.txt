cmake_minimum_required(VERSION 3.20)

# Must be set before project() to take effect via CMP0091
cmake_policy(SET CMP0091 NEW)

project(AtlasWorkspace VERSION 0.1.0 LANGUAGES CXX)

# Enforce a consistent MSVC runtime library across all targets to avoid LNK4098
if(MSVC)
    set(CMAKE_MSVC_RUNTIME_LIBRARY "MultiThreaded$<$<CONFIG:Debug>:Debug>DLL")
endif()

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

# ── CMake helper modules ──────────────────────────────────────────
list(APPEND CMAKE_MODULE_PATH "${CMAKE_CURRENT_SOURCE_DIR}/cmake")

# Apply Windows-specific compile definitions when targeting Windows.
if(WIN32 OR CMAKE_SYSTEM_NAME STREQUAL "Windows")
    include(WindowsToolchain)
endif()

# ── Global compile settings ───────────────────────────────────────
# Applied per-target via a common interface library to avoid polluting
# FetchContent dependencies with our warning flags.
add_library(NF_CompileOptions INTERFACE)
target_compile_options(NF_CompileOptions INTERFACE
    $<$<CXX_COMPILER_ID:MSVC>:/W4 /permissive- /Zc:__cplusplus /utf-8 /MP>
    $<$<NOT:$<CXX_COMPILER_ID:MSVC>>:-Wall -Wextra -Wpedantic>
)

# Output directories — applications land in bin/, tests in bin/Tests/
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/bin)
set(CMAKE_LIBRARY_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/lib)
set(CMAKE_ARCHIVE_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/lib)

# Per-config output directories (Visual Studio / multi-config generators)
foreach(CFG Debug Release RelWithDebInfo MinSizeRel)
    string(TOUPPER ${CFG} CFG_UPPER)
    set(CMAKE_RUNTIME_OUTPUT_DIRECTORY_${CFG_UPPER} ${CMAKE_BINARY_DIR}/bin/${CFG})
    set(CMAKE_LIBRARY_OUTPUT_DIRECTORY_${CFG_UPPER} ${CMAKE_BINARY_DIR}/lib/${CFG})
    set(CMAKE_ARCHIVE_OUTPUT_DIRECTORY_${CFG_UPPER} ${CMAKE_BINARY_DIR}/lib/${CFG})
endforeach()

# ── Build options ─────────────────────────────────────────────────
option(ATLAS_BUILD_TESTS        "Build workspace test suite"                OFF)
option(ATLAS_BUILD_WORKSPACE    "Build AtlasWorkspace executable"           ON)
option(ATLAS_ENABLE_ONLINE_DEPS "Allow online dependency fetches (Catch2)" OFF)

# ── Atlas Engine subsystems (static libraries) ────────────────────
# The flagship engine that powers all projects loaded in Atlas Workspace.
add_subdirectory(Source/Core)
add_subdirectory(Source/Engine)
add_subdirectory(Source/Renderer)
add_subdirectory(Source/Physics)
add_subdirectory(Source/Audio)
add_subdirectory(Source/Animation)
add_subdirectory(Source/Input)
add_subdirectory(Source/Networking)

# ── Workspace platform modules (static libraries) ─────────────────
add_subdirectory(Source/UI)
add_subdirectory(Source/GraphVM)
add_subdirectory(Source/AI)
add_subdirectory(Source/Pipeline)
add_subdirectory(Source/Workspace)
add_subdirectory(Source/Editor)

if(ATLAS_BUILD_WORKSPACE)
    add_subdirectory(Source/Programs/AtlasWorkspace)
endif()

# ── Standalone child executables ──────────────────────────────────
# Builds NovaForgeEditor, NovaForgeGame, and NovaForgeServer into the
# same bin/ directory as AtlasWorkspace so the launch service can find them.
option(ATLAS_BUILD_PROGRAMS "Build standalone child executables (NovaForgeEditor, NovaForgeGame, NovaForgeServer)" ON)

if(ATLAS_BUILD_PROGRAMS)
    # NovaForge game-specific modules (EditorAdapter, Game, World, Patch).
    # These depend only on the Atlas engine subsystems already built above.
    add_subdirectory(NovaForge/Source/EditorAdapter)
    add_subdirectory(NovaForge/Source/Game)
    add_subdirectory(NovaForge/Source/World)
    add_subdirectory(NovaForge/Source/Patch)

    # Enable all three child executables, then include the programs directory
    # that already knows how to build them.
    set(NF_BUILD_EDITOR ON)
    set(NF_BUILD_GAME   ON)
    set(NF_BUILD_SERVER ON)
    add_subdirectory(NovaForge/Source/Programs)
endif()

if(ATLAS_BUILD_TESTS)
    enable_testing()
    add_subdirectory(Tests)
endif()

message(STATUS "")
message(STATUS "=== AtlasWorkspace ${PROJECT_VERSION} ===")
message(STATUS "  Build Workspace: ${ATLAS_BUILD_WORKSPACE}")
message(STATUS "  Build Programs:  ${ATLAS_BUILD_PROGRAMS}")
message(STATUS "  Build Tests:     ${ATLAS_BUILD_TESTS}")
message(STATUS "")
