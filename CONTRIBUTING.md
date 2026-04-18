# Contributing to Atlas Workspace

Atlas Workspace is a **Rust + Vulkan** project. All new systems are written in Rust.

## Prerequisites

- **rustup** — https://rustup.rs (installs rustc + cargo)
- **Vulkan SDK** — https://vulkan.lunarg.com/sdk/home (for `glslc` shader compiler)
- **Git**

```bash
# Verify your setup
rustc --version
cargo --version
glslc --version  # optional, only needed for shader compilation
```

## Quick Start

```bash
# Clone
git clone https://github.com/shifty81/Atlas-Workspace-Rust.git
cd Atlas-Workspace-Rust

# Build everything
cargo build --workspace

# Run all tests
cargo test --workspace

# Run the workspace editor
cargo run --bin atlas-workspace

# Run the standalone game
cargo run --bin atlas-game
```

## Workspace Structure

```
crates/
  atlas-core/       Foundation types, logging, string IDs
  atlas-math/       Math primitives (glam-backed)
  atlas-ecs/        Entity-Component-System
  atlas-pcg/        PCG world-gen pipeline
  atlas-world/      Universe-scale world generation
  atlas-workspace/  Main binary (cargo run --bin atlas-workspace)
  atlas-renderer/   Vulkan rendering backend
  atlas-editor/     egui editor app
  atlas-game/       Standalone game binary (cargo run --bin atlas-game)
  atlas-input/      Input system (stub)
  atlas-physics/    Physics (stub)
  atlas-sim/        Simulation (stub)
  atlas-script/     Scripting (stub)
  atlas-animation/  Animation (stub)
  atlas-ai/         AtlasAI broker (stub)
  atlas-sound/      Audio (stub)
  atlas-graphvm/    Node graph VM (stub)
  atlas-net/        Networking (stub)
  atlas-schema/     JSON schema types (stub)
  atlas-abi/        Plugin ABI (stub)
  atlas-asset/      Asset registry (stub)
  atlas-ui/         egui widget extensions (stub)
```

## Development Workflow

```bash
# Fast check (no binary output)
bash Scripts/check_rust.sh
# or: cargo check --workspace

# Clippy (lint)
cargo clippy --workspace -- -D warnings

# Format check
cargo fmt --all -- --check

# Format fix
cargo fmt --all

# Run tests
bash Scripts/test_rust.sh
# or: cargo test --workspace

# Full build with tests + clippy
bash Scripts/build_rust.sh --test --clippy

# Compile SPIR-V shaders
bash Scripts/build_shaders.sh

# Or use the Makefile
make check
make clippy
make test
make build
```

## How to Add a New System

### Extend an existing crate

Add new modules inside the relevant `crates/<name>/src/` directory and export from `lib.rs`.

### Add a new crate

1. Create the directory: `crates/<name>/`
2. Create `crates/<name>/Cargo.toml` and `crates/<name>/src/lib.rs`
3. Add `"crates/<name>"` to the `[workspace]` members list in the root `Cargo.toml`
4. Update `Docs/Canon/00_PROJECT_STATUS.md` with the new crate status
5. Update `README.md` crate table and workspace layout

### Add tests

Write unit tests in the same file using `#[cfg(test)]` blocks. Integration tests go in `crates/<name>/tests/`.

```rust
#[cfg(test)]
mod tests {
    use super::*;
    #[test]
    fn test_example() { /* ... */ }
}
```

## Commit Style

Use conventional commit prefixes:

| Prefix | When to use |
|--------|-------------|
| `feat(atlas-renderer):` | New feature in a specific crate |
| `fix(atlas-ecs):` | Bug fix |
| `docs:` | Documentation changes only |
| `test(atlas-pcg):` | Tests only |
| `refactor(atlas-math):` | Refactoring without behavior change |
| `chore:` | Build scripts, CI, Makefile |

Examples:
```
feat(atlas-renderer): wire Vulkan surface via winit + ash-window
fix(atlas-ecs): correct component store index on entity removal
docs: update Phase 0 milestone progress in roadmap
test(atlas-pcg): add determinism regression tests for all 16 domains
```

## PR Checklist

Before opening a PR:

- [ ] `cargo fmt --all -- --check` passes (no format errors)
- [ ] `cargo clippy --workspace -- -D warnings` passes (no clippy errors)
- [ ] `cargo test --workspace` passes (no failing tests)
- [ ] New public APIs have rustdoc comments
- [ ] `Docs/Roadmap/00_MASTER_ROADMAP.md` milestone updated if applicable
- [ ] `Docs/Canon/00_PROJECT_STATUS.md` crate status updated if applicable

## Legacy C++ Note

**Do not submit C++ code PRs.**

The C++ code in `Source/` and `NovaForge/` is **reference/archive only** — it serves as the specification for the Rust port. It is not actively developed. CMake is not the primary build system.

If you need to reference the C++ implementation:
```bash
bash Scripts/build_cpp_legacy.sh Debug
```
