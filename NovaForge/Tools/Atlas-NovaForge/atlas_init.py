#!/usr/bin/env python3
"""Atlas Project Scaffolding Tool.

Creates a new game project from the AtlasForge template with proper
namespace isolation, module scaffolding, and build configuration.

Usage:
    python tools/atlas_init.py --name MyGame --namespace mygame [--dir /path/to/output]
"""

import argparse
import json
import os
import sys


def create_project_file(name, namespace, version="0.1.0"):
    """Return the .atlas project file content."""
    return json.dumps({
        "schema": "atlas.project.v1",
        "name": name.upper(),
        "version": version,
        "description": f"{name} — built on Atlas Engine",
        "modules": {
            "worldGraph": "worlds/main.worldgraph",
            "tileGraphs": "worlds/",
            "strategyGraphs": "strategy/",
            "conversationGraphs": "conversations/",
            "behaviorGraphs": "ai/",
            "ai": True,
            "content": "data/"
        },
        "runtime": {
            "entryWorld": "worlds/main.worldgraph",
            "tickRate": 30,
            "maxPlayers": 20
        },
        "assets": {
            "root": "assets"
        },
        "config": "config/runtime.json"
    }, indent=4) + "\n"


def create_plugin_toml(name, namespace, version="0.1.0"):
    """Return the Plugin.toml content."""
    return f"""\
[plugin]
id = "{namespace}"
name = "{name}"
version = "{version}"
type = "game"

[engine]
required_version = ">=0.1.0"
determinism = true
rollback = true
server_authoritative = true

[capabilities]
uses_graphs = true
uses_schemas = true
uses_assetgraph = true
uses_networking = true
uses_ai_assist = false

[content]
schemas_path = "Schemas"
graphs_path = "worlds"
strategy_path = "strategy"
conversations_path = "conversations"
assets_path = "assets"
data_path = "data"
config_path = "config"

[code]
adapters_path = "Code/Adapters"
entry_point = "Code/GameInit.cpp"

[editor]
show_in_launcher = true
default_project = true

[testing]
test_path = "Tests"
golden_required = true
"""


def create_root_cmakelists(name):
    """Return the top-level CMakeLists.txt content."""
    return f"""\
cmake_minimum_required(VERSION 3.20)
project({name} LANGUAGES CXX)
set(CMAKE_CXX_STANDARD 17)

find_package(AtlasEngine REQUIRED)
find_package(AtlasGameplay REQUIRED)

add_subdirectory(module)
"""


def create_module_cmakelists(name):
    """Return the module CMakeLists.txt content."""
    return f"""\
add_library({name}Module SHARED
    {name}Module.cpp
)

target_include_directories({name}Module PUBLIC ${{CMAKE_CURRENT_SOURCE_DIR}})
target_link_libraries({name}Module PRIVATE Atlas::AtlasEngine Atlas::AtlasGameplay)
"""


def create_module_header(name, namespace):
    """Return the IGameModule header content."""
    return f"""\
#pragma once
#include <engine/module/IGameModule.h>

namespace {namespace} {{

class {name}Module : public atlas::module::IGameModule {{
public:
    atlas::module::GameModuleDesc Describe() const override;
    void RegisterTypes(atlas::module::GameModuleContext& ctx) override;
    void ConfigureReplication(atlas::module::GameModuleContext& ctx) override;
    void ConfigureServerRules(atlas::module::GameModuleContext& ctx) override;
    void OnStart(atlas::module::GameModuleContext& ctx) override;
    void OnTick(atlas::module::GameModuleContext& ctx, float dt) override;
    void OnShutdown(atlas::module::GameModuleContext& ctx) override;

private:
    bool m_started = false;
    uint32_t m_tickCount = 0;
}};

}} // namespace {namespace}
"""


def create_module_source(name, namespace):
    """Return the IGameModule implementation source content."""
    return f"""\
#include "{name}Module.h"

namespace {namespace} {{

atlas::module::GameModuleDesc {name}Module::Describe() const {{
    return {{"{name}", 1}};
}}

void {name}Module::RegisterTypes(atlas::module::GameModuleContext& ctx) {{
    (void)ctx;
}}

void {name}Module::ConfigureReplication(atlas::module::GameModuleContext& ctx) {{
    (void)ctx;
}}

void {name}Module::ConfigureServerRules(atlas::module::GameModuleContext& ctx) {{
    (void)ctx;
}}

void {name}Module::OnStart(atlas::module::GameModuleContext& ctx) {{
    (void)ctx;
    m_started = true;
}}

void {name}Module::OnTick(atlas::module::GameModuleContext& ctx, float dt) {{
    (void)ctx;
    (void)dt;
    ++m_tickCount;
}}

void {name}Module::OnShutdown(atlas::module::GameModuleContext& ctx) {{
    (void)ctx;
    m_started = false;
    m_tickCount = 0;
}}

}} // namespace {namespace}

// Factory function for dynamic loading
extern "C" atlas::module::IGameModule* CreateGameModule() {{
    return new {namespace}::{name}Module();
}}
"""


def create_runtime_json():
    """Return the default runtime configuration."""
    return json.dumps({
        "schema": "atlas.config.v1",
        "tickRate": 30,
        "maxPlayers": 20,
        "networkMode": "client_server",
        "deterministic": True,
        "serverRules": {}
    }, indent=4) + "\n"


def main():
    parser = argparse.ArgumentParser(
        description="Create a new Atlas game project from the template."
    )
    parser.add_argument(
        "--name", required=True,
        help="Project display name (e.g. MyGame)"
    )
    parser.add_argument(
        "--namespace", required=True,
        help="C++ namespace for game code (e.g. mygame)"
    )
    parser.add_argument(
        "--dir", default=".",
        help="Parent directory for the new project (default: current directory)"
    )
    args = parser.parse_args()

    name = args.name
    namespace = args.namespace
    project_dir = os.path.join(args.dir, name)

    if os.path.exists(project_dir):
        print(f"Error: Directory already exists: {project_dir}", file=sys.stderr)
        sys.exit(1)

    # Create directory structure
    dirs = [
        project_dir,
        os.path.join(project_dir, "module"),
        os.path.join(project_dir, "assets"),
        os.path.join(project_dir, "data"),
        os.path.join(project_dir, "config"),
    ]
    for d in dirs:
        os.makedirs(d, exist_ok=True)

    # Generate files
    files = {
        os.path.join(project_dir, f"{name}.atlas"): create_project_file(name, namespace),
        os.path.join(project_dir, "Plugin.toml"): create_plugin_toml(name, namespace),
        os.path.join(project_dir, "CMakeLists.txt"): create_root_cmakelists(name),
        os.path.join(project_dir, "module", "CMakeLists.txt"): create_module_cmakelists(name),
        os.path.join(project_dir, "module", f"{name}Module.h"): create_module_header(name, namespace),
        os.path.join(project_dir, "module", f"{name}Module.cpp"): create_module_source(name, namespace),
        os.path.join(project_dir, "config", "runtime.json"): create_runtime_json(),
    }

    for path, content in files.items():
        with open(path, "w") as f:
            f.write(content)

    print(f"Created project '{name}' in {os.path.abspath(project_dir)}")
    print()
    print("Project structure:")
    print(f"  {name}/")
    print(f"  ├── {name}.atlas")
    print(f"  ├── Plugin.toml")
    print(f"  ├── CMakeLists.txt")
    print(f"  ├── module/")
    print(f"  │   ├── CMakeLists.txt")
    print(f"  │   ├── {name}Module.h")
    print(f"  │   └── {name}Module.cpp")
    print(f"  ├── assets/")
    print(f"  ├── data/")
    print(f"  └── config/")
    print(f"      └── runtime.json")
    print()
    print("Next steps:")
    print(f"  cd {name}")
    print(f"  mkdir build && cd build")
    print(f"  cmake .. -DCMAKE_PREFIX_PATH=<path-to-atlas-install>")
    print(f"  cmake --build .")


if __name__ == "__main__":
    main()
