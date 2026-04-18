# cmake/WindowsToolchain.cmake
# ─────────────────────────────────────────────────────────────────────────────
# Windows-specific CMake settings.  Included automatically by the Windows
# configure presets via CMAKE_TOOLCHAIN_FILE, or manually:
#   cmake -DCMAKE_TOOLCHAIN_FILE=cmake/WindowsToolchain.cmake ...
#
# This file does NOT hard-code paths to MSVC or Windows SDK — CMake discovers
# those automatically.  Instead it sets safe, cross-compiler-compatible
# defaults that apply when targeting Windows.
# ─────────────────────────────────────────────────────────────────────────────

if(NOT WIN32 AND NOT CMAKE_SYSTEM_NAME STREQUAL "Windows")
    return()
endif()

# Prefer Unicode Win32 API (W variants over A variants).
add_compile_definitions(UNICODE _UNICODE)

# Exclude rarely-used Windows headers to speed up compilation.
add_compile_definitions(WIN32_LEAN_AND_MEAN VC_EXTRALEAN NOMINMAX)

# Silence MSVC deprecation warnings for standard C library functions.
add_compile_definitions(_CRT_SECURE_NO_WARNINGS _SCL_SECURE_NO_WARNINGS)

# Windows 10 / Server 2019 minimum (0x0A00).
add_compile_definitions(_WIN32_WINNT=0x0A00 WINVER=0x0A00)
