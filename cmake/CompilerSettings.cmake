# cmake/CompilerSettings.cmake
# ─────────────────────────────────────────────────────────────────────────────
# Shared compiler warning / hardening flags for all NovaForge targets.
# Include via:
#   include(${CMAKE_SOURCE_DIR}/cmake/CompilerSettings.cmake)
# or apply the interface target:
#   target_link_libraries(MyTarget PRIVATE NF_CompilerSettings)
# ─────────────────────────────────────────────────────────────────────────────

if(TARGET NF_CompilerSettings)
    return()  # Already included
endif()

add_library(NF_CompilerSettings INTERFACE)

target_compile_options(NF_CompilerSettings INTERFACE
    # ── MSVC (Visual Studio / cl.exe) ────────────────────────────────────────
    $<$<CXX_COMPILER_ID:MSVC>:
        /W4             # High warning level
        /permissive-    # Strict standard conformance
        /Zc:__cplusplus # Report correct __cplusplus value
        /utf-8          # Source and execution charset = UTF-8
        /MP             # Multi-processor compilation
    >

    # ── GCC ──────────────────────────────────────────────────────────────────
    $<$<CXX_COMPILER_ID:GNU>:
        -Wall
        -Wextra
        -Wpedantic
        -Wshadow
        -Wconversion
    >

    # ── Clang (any platform, including clang-cl on Windows) ──────────────────
    $<$<CXX_COMPILER_ID:Clang>:
        -Wall
        -Wextra
        -Wpedantic
    >
)

# C++20 minimum on all targets that link this interface.
target_compile_features(NF_CompilerSettings INTERFACE cxx_std_20)
