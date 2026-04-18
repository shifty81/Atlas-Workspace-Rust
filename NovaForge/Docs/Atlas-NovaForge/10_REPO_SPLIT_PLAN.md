# Atlas — Game Module Architecture

EveOffline ships permanently inside this repository as a first-party game
example. There is no plan to split it into a separate repo. Both the engine
and EveOffline are actively developed side-by-side, and once the engine is
complete EveOffline will serve as the shipped reference game that
demonstrates every engine feature end-to-end.

## Current Layout

```
Atlas/                        (single repo)
├─ engine/                    ← AtlasEngine static lib (core, ecs, net, sim, assets, graphs)
├─ runtime/                   ← AtlasRuntime entry that loads a game module
├─ client/                    ← AtlasClient links AtlasEngine + AtlasGameplay + module
├─ server/                    ← AtlasServer links AtlasEngine + AtlasGameplay + module
├─ editor/                    ← AtlasEditor links AtlasEngine + AtlasGameplay + module
├─ modules/
│  └─ atlas_gameplay/         ← AtlasGameplay static lib (factions, economy, combat base, AI schedulers)
├─ projects/
│  ├─ eveoffline/             ← Shipped game example (permanent, not split out)
│  │  └─ module/              ← EveOfflineModule (IGameModule implementation)
│  └─ arena2d/                ← 2D arena reference project
└─ CMakeLists.txt             ← adds AtlasEngine, AtlasGameplay, AtlasRuntime/Client/Server/Editor
```

Third-party games that want to build against the Atlas SDK can still use the
module interface and `find_package(AtlasEngine)` / `find_package(AtlasGameplay)`
after an SDK install, but EveOffline itself is not going external.

## Game Module Interface

The module interface keeps game logic cleanly separated from engine internals,
even though everything lives in one repo.

```cpp
namespace atlas::module {
struct GameModuleDesc {
    const char* name;
    uint32_t version;
};

struct GameModuleContext {
    ecs::World& world;
    net::NetContext& net;
    net::ReplicationManager& replication;
    rules::ServerRules& rules;
    assets::AssetRegistry& assets;
    project::ProjectDescriptor project;
};

class IGameModule {
public:
    virtual GameModuleDesc Describe() const = 0;
    virtual void RegisterTypes(GameModuleContext& ctx) = 0;          // components, systems, assets
    virtual void ConfigureReplication(GameModuleContext& ctx) {}     // add ReplicationRule entries
    virtual void ConfigureServerRules(GameModuleContext& ctx) {}     // populate ServerRules
    virtual void OnStart(GameModuleContext& ctx) {}                  // called after project load
    virtual void OnTick(GameModuleContext& ctx, float dt) {}         // per-tick hook
    virtual void OnShutdown(GameModuleContext& ctx) {}               // cleanup
    virtual ~IGameModule() = default;
};

using CreateGameModuleFn = std::unique_ptr<IGameModule>(*)();
} // namespace atlas::module

// Factory symbol exported by each game module (dll/so):
extern "C" std::unique_ptr<atlas::module::IGameModule> CreateGameModule();
```

## Runtime Loading Flow

`ServerRuntime` / `ClientRuntime` (in Atlas repo) should:
1. Resolve module name from CLI or project file (e.g., `project.modules.content`).
2. Locate the shared library (`libEveOfflineModule.so`, `EveOfflineModule.dll`) or a statically linked fallback.
3. Load with `dlopen` / `GetProcAddress` (or a compile-time registry in tests).
4. Call `CreateGameModule()` to get the module instance.
5. Build `GameModuleContext` with `Engine` subsystems (world, net, replication, rules, assets, project descriptor).
6. Invoke `RegisterTypes`, `ConfigureReplication`, `ConfigureServerRules`, then `OnStart`.
7. On each tick, forward to `OnTick` with `dt`.
8. On shutdown, call `OnShutdown` and unload the library.

Failure policy: if the module cannot be loaded or `Describe().name` mismatches the requested project, log + exit with a clear message.

## Completed Steps

1) ✅ **Freeze engine-facing APIs**: finalized `ecs::World`, `net::NetContext`, `net::ReplicationManager`, `rules::ServerRules`, and `assets::AssetRegistry` headers as the public surface.
2) ✅ **Carve AtlasGameplay**: moved reusable gameplay frameworks (FactionSystem, CombatFramework, EconomySystem) under `modules/atlas_gameplay` and exposed them via `AtlasGameplay` target; no game-specific names.
3) ✅ **Add module loader**: implemented the `IGameModule` interface header in `engine/module/`, added `ModuleLoader` with `dlopen`/`LoadLibrary` dynamic loading and `SetStaticModule()` for static linking during tests. Runtime/server/client entry points support `--module` flag.
4) ✅ **EveOffline as in-repo game module**: created `projects/eveoffline/module/` with `EveOfflineModule` implementing `IGameModule`; registers factions, economy, replication rules, and server rules. Ships permanently as the reference game example.
5) ✅ **CI/build updates**: extended `build.sh` with `engine` target (builds `AtlasEngine` + `AtlasGameplay` libraries) and `--install` flag to publish SDK artifacts (headers + static libs + CMake configs); external third-party modules can `find_package(AtlasEngine)` and `find_package(AtlasGameplay)` cleanly.
