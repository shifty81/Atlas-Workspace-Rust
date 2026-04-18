#!/usr/bin/env python3
"""
Atlas Project Scaffolder — Constitution-Compliant Project Creator

Creates a new Atlas project that is compliant with the Atlas Core Contract
from day one. The generated project includes determinism traits, schema
versioning, replay/migration directories, and a .atlas manifest with
determinism_profile and binary_abi declarations.

This tool differs from atlas_init.py (which creates a full game module
scaffold with Plugin.toml and CMake integration). create_atlas_project
focuses on contract compliance and is intended for projects that will
be opened inside the Atlas Editor.

Usage:
    python tools/create_atlas_project.py <ProjectName> [output_dir]

Example:
    python tools/create_atlas_project.py EveOffline projects/

See: docs/ATLAS_CORE_CONTRACT.md
     docs/ATLAS_SDK.md
     docs/ATLAS_LOCKDOWN_CHECKLIST.md
"""

import json
import os
import sys
from pathlib import Path


def die(msg: str) -> None:
    print(f"❌ {msg}", file=sys.stderr)
    sys.exit(1)


def create_project(project_name: str, output_dir: Path) -> Path:
    """Create a constitution-compliant Atlas project.

    Returns the path to the created project root.
    """
    project_root = output_dir / project_name

    if project_root.exists():
        die(f"Directory already exists: {project_root}")

    print(f"📦 Creating Atlas project: {project_name}")

    # --- Folder Structure ---
    folders = [
        "src",
        "src/systems",
        "src/components",
        "scripts",
        "assets",
        "replays",
        "migrations",
        "config",
    ]

    for f in folders:
        (project_root / f).mkdir(parents=True, exist_ok=True)

    # --- .atlas Project File ---
    atlas_project = {
        "project": {
            "name": project_name,
            "version": "0.1.0",
            "binary_abi": "atlas_abi_v1",
            "determinism_profile": "fixed32_v1",
            "requires": {
                "replay_support": True,
                "migration_support": True,
            },
        }
    }

    with open(project_root / f"{project_name}.atlas", "w") as f:
        json.dump(atlas_project, f, indent=2)
        f.write("\n")

    # --- Stub System ---
    system_cpp = """\
#include <cstdint>

// Atlas Core Contract: every system must declare determinism traits.
// See docs/ATLAS_CORE_CONTRACT.md

#ifndef ATLAS_SYSTEM_TRAITS
#define ATLAS_SYSTEM_TRAITS(cls, ...) \\
    static constexpr auto cls##_traits = __VA_ARGS__
#endif

struct SystemTraits {
    const char* name;
    bool deterministic;
    bool serializable;
    bool replayable;
    const char* source_file;
};

class ExampleSystem {
public:
    void Update() {
        // deterministic logic only
    }
};

ATLAS_SYSTEM_TRAITS(ExampleSystem,
    (SystemTraits{ "ExampleSystem", true, true, true, __FILE__ })
);
"""
    with open(project_root / "src" / "systems" / "ExampleSystem.cpp", "w") as f:
        f.write(system_cpp)

    # --- Stub Component ---
    component_h = """\
#pragma once
#include <cstdint>

// Atlas Core Contract: all components must declare a schema version
// for migration support. See docs/ATLAS_CORE_CONTRACT.md

struct ExampleComponent {
    int value = 0;
    static constexpr uint32_t SchemaVersion = 1;
};
"""
    with open(project_root / "src" / "components" / "ExampleComponent.h", "w") as f:
        f.write(component_h)

    # --- README ---
    readme = f"""\
# {project_name}

Atlas project created with `create_atlas_project`.

## Rules
- Built inside Atlas Editor only
- Deterministic systems only
- Migrations required for schema changes
- Replays are mandatory

## Entry Points
- `src/systems/` — Simulation systems (deterministic only)
- `src/components/` — ECS components with schema versioning

## Directory Structure
- `assets/` — Project assets (meshes, textures, sounds)
- `config/` — Runtime configuration
- `migrations/` — Schema migration scripts
- `replays/` — Golden replay files for CI verification
- `scripts/` — Gameplay scripts (Lua/Wren, sandboxed)
"""
    with open(project_root / "README.md", "w") as f:
        f.write(readme)

    print(f"✅ Project created successfully")
    print(f"👉 Open in Atlas Editor: {project_root}")

    return project_root


def main() -> None:
    if len(sys.argv) < 2:
        die("Usage: create_atlas_project.py <ProjectName> [output_dir]")

    project_name = sys.argv[1]
    output_dir = Path(sys.argv[2]) if len(sys.argv) > 2 else Path.cwd()

    create_project(project_name, output_dir)


if __name__ == "__main__":
    main()
