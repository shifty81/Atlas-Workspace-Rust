cmake_minimum_required(VERSION 3.20)

# Must be set before project() to take effect via CMP0091
cmake_policy(SET CMP0091 NEW)

project(NovaForge VERSION 0.1.0 LANGUAGES CXX)

# Enforce a consistent MSVC runtime library across all targets to avoid LNK4098
if(MSVC)
    set(CMAKE_MSVC_RUNTIME_LIBRARY "MultiThreaded$<$<CONFIG:Debug>:Debug>DLL")
endif()

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

# ── CMake helper modules ──────────────────────────────────────────
list(APPEND CMAKE_MODULE_PATH "${CMAKE_CURRENT_SOURCE_DIR}/../cmake")

# Apply Windows-specific compile definitions when targeting Windows.
if(WIN32 OR CMAKE_SYSTEM_NAME STREQUAL "Windows")
    include(WindowsToolchain)
endif()

# ── Global compile settings ───────────────────────────────────────
add_library(NF_CompileOptions INTERFACE)
target_compile_options(NF_CompileOptions INTERFACE
    $<$<CXX_COMPILER_ID:MSVC>:/W4 /permissive- /Zc:__cplusplus /utf-8 /MP>
    $<$<NOT:$<CXX_COMPILER_ID:MSVC>>:-Wall -Wextra -Wpedantic>
)

# Output directories
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/bin)
set(CMAKE_LIBRARY_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/lib)
set(CMAKE_ARCHIVE_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/lib)

foreach(CFG Debug Release RelWithDebInfo MinSizeRel)
    string(TOUPPER ${CFG} CFG_UPPER)
    set(CMAKE_RUNTIME_OUTPUT_DIRECTORY_${CFG_UPPER} ${CMAKE_BINARY_DIR}/bin/${CFG})
    set(CMAKE_LIBRARY_OUTPUT_DIRECTORY_${CFG_UPPER} ${CMAKE_BINARY_DIR}/lib/${CFG})
    set(CMAKE_ARCHIVE_OUTPUT_DIRECTORY_${CFG_UPPER} ${CMAKE_BINARY_DIR}/lib/${CFG})
endforeach()

# ── Build mode ────────────────────────────────────────────────────
option(NF_STANDALONE "Standalone build (all targets enabled by default)" ON)
option(NF_HOSTED     "Workspace-hosted build (targets disabled by default, Workspace controls)" OFF)

if(NF_HOSTED AND NOT NF_STANDALONE)
    set(_NF_DEFAULT_BUILD_TARGETS OFF)
else()
    set(_NF_DEFAULT_BUILD_TARGETS ON)
endif()

# Build options
option(NF_BUILD_EDITOR  "Build NovaForge Editor"             ${_NF_DEFAULT_BUILD_TARGETS})
option(NF_BUILD_GAME    "Build NovaForge Game Runtime"       ${_NF_DEFAULT_BUILD_TARGETS})
option(NF_BUILD_SERVER  "Build NovaForge Dedicated Server"   ${_NF_DEFAULT_BUILD_TARGETS})
option(NF_BUILD_TESTS   "Build test suite"                   OFF)

# ── Workspace platform modules (from parent) ──────────────────────
# In hosted mode, these are provided by AtlasWorkspace.
# In standalone mode, build them from the parent Source directory.
if(NF_STANDALONE)
    # Atlas Engine — flagship engine (Core + Engine + subsystems)
    add_subdirectory(${CMAKE_CURRENT_SOURCE_DIR}/../Source/Core        ${CMAKE_BINARY_DIR}/ws_Core)
    add_subdirectory(${CMAKE_CURRENT_SOURCE_DIR}/../Source/Engine      ${CMAKE_BINARY_DIR}/ws_Engine)
    add_subdirectory(${CMAKE_CURRENT_SOURCE_DIR}/../Source/Renderer    ${CMAKE_BINARY_DIR}/ws_Renderer)
    add_subdirectory(${CMAKE_CURRENT_SOURCE_DIR}/../Source/Physics     ${CMAKE_BINARY_DIR}/ws_Physics)
    add_subdirectory(${CMAKE_CURRENT_SOURCE_DIR}/../Source/Audio       ${CMAKE_BINARY_DIR}/ws_Audio)
    add_subdirectory(${CMAKE_CURRENT_SOURCE_DIR}/../Source/Animation   ${CMAKE_BINARY_DIR}/ws_Animation)
    add_subdirectory(${CMAKE_CURRENT_SOURCE_DIR}/../Source/Input       ${CMAKE_BINARY_DIR}/ws_Input)
    add_subdirectory(${CMAKE_CURRENT_SOURCE_DIR}/../Source/Networking  ${CMAKE_BINARY_DIR}/ws_Networking)
    # Workspace platform modules
    add_subdirectory(${CMAKE_CURRENT_SOURCE_DIR}/../Source/UI          ${CMAKE_BINARY_DIR}/ws_UI)
    add_subdirectory(${CMAKE_CURRENT_SOURCE_DIR}/../Source/GraphVM     ${CMAKE_BINARY_DIR}/ws_GraphVM)
    add_subdirectory(${CMAKE_CURRENT_SOURCE_DIR}/../Source/AI          ${CMAKE_BINARY_DIR}/ws_AI)
    add_subdirectory(${CMAKE_CURRENT_SOURCE_DIR}/../Source/Pipeline    ${CMAKE_BINARY_DIR}/ws_Pipeline)
    add_subdirectory(${CMAKE_CURRENT_SOURCE_DIR}/../Source/Workspace   ${CMAKE_BINARY_DIR}/ws_Workspace)
    add_subdirectory(${CMAKE_CURRENT_SOURCE_DIR}/../Source/Editor      ${CMAKE_BINARY_DIR}/ws_Editor)
endif()

# ── NovaForge game modules ────────────────────────────────────────
add_subdirectory(Source/EditorAdapter)
add_subdirectory(Source/Game)
add_subdirectory(Source/World)
add_subdirectory(Source/Patch)

# Executables
add_subdirectory(Source/Programs)

if(NF_BUILD_TESTS)
    enable_testing()
    add_subdirectory(Tests)
endif()

message(STATUS "")
message(STATUS "=== NovaForge ${PROJECT_VERSION} ===")
message(STATUS "  Standalone:    ${NF_STANDALONE}")
message(STATUS "  Hosted:        ${NF_HOSTED}")
message(STATUS "  Build Editor:  ${NF_BUILD_EDITOR}")
message(STATUS "  Build Game:    ${NF_BUILD_GAME}")
message(STATUS "  Build Server:  ${NF_BUILD_SERVER}")
message(STATUS "  Build Tests:   ${NF_BUILD_TESTS}")
message(STATUS "")
