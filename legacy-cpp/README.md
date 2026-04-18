# Legacy C++ Reference

This folder contains the original C++ build system and project files from the pre-Rust
version of Atlas Workspace. They are kept here as a **reference only** and are **not part
of the active build**.

## Active build system

The project has been rewritten in Rust. Use Cargo for all development:

```sh
# Build the editor
cargo run --bin atlas-workspace

# Build the game
cargo build --bin atlas-game

# Run all tests
cargo test --workspace

# Quick check + clippy
bash Scripts/check_rust.sh
```

Or use the top-level `Makefile` targets: `make build`, `make test`, `make check`, etc.

## Contents

| Path | Description |
|------|-------------|
| `CMakeLists.txt` | CMake root build file for the C++ project |
| `CMakePresets.json` | CMake presets (vs2022, vs2019, debug, release, …) |
| `cmake/` | CMake helper modules (compiler settings, Windows toolchain) |
| `vcpkg.json` | vcpkg C++ dependency manifest (glfw3, vulkan-headers, spdlog, …) |
| `build.cmd` | Windows batch script to build with MSBuild / Visual Studio |
| `Directory.Build.props` | MSBuild auto-import guard (ensures correct VS MSBuild is used) |
| `Scripts/build_all.sh` | Full CMake configure + build + optional ctest run |
| `Scripts/build_cpp_legacy.sh` | Thin wrapper that invokes the CMake build |
| `Scripts/generate_vs_solution.bat` | Generates a VS 2022/2019 `.sln` via CMake presets |
| `Scripts/generate_vs_solution.ps1` | PowerShell equivalent of the above |

## Why keep these?

- Serves as a reference for the port — useful when comparing C++ and Rust implementations.
- Preserves the original build infrastructure in case any logic needs to be re-examined.
