# Atlas Engine – Project-Oriented Architecture

Atlas is a game-agnostic simulation engine. The engine binaries (runtime, client, server, editor) never contain gameplay. All gameplay lives in standalone game modules located under `projects/`.

## Design Goals
- Engine code stays reusable and independent of any specific game.
- Server, client, and editor share the same deterministic simulation core.
- Game logic is provided through loadable modules; swapping projects does not require rebuilding the engine.
- Multiplayer, rollback, replay, and editor time-scrubbing rely on the same infrastructure.

## Repository Layout (no gameplay in engine)
```
engine/      # Core engine only (ECS, net, replication, rollback, serialization, math, runtime, debug, module)
gameplay/    # Optional generic helpers, still game-agnostic
projects/    # Game modules; add your projects here
tools/       # Tooling (e.g., project scaffolding)
docs/        # Architecture and workflow docs
```

### Engine Responsibilities
- Deterministic ECS simulation and tick scheduling.
- Networking, replication, rollback/snapshots, and replay/desync diagnostics.
- Runtime front-ends (`ServerRuntime`, `ClientRuntime`, `EditorRuntime`) that load a game module and run it.
- Module interface contracts only; no project-specific logic.

### Game Module Contract (engine-owned)
Modules implement the engine-facing interface and export a factory symbol:
```
class IGameModule {
public:
    virtual ~IGameModule() = default;
    virtual const char* GetGameName() const = 0;
    virtual uint32_t GetVersion() const = 0;
    virtual void RegisterSystems(SystemRegistry&) {}
    virtual void RegisterAssets(AssetRegistry&) {}
    virtual void RegisterReplication(ReplicationRegistry&) {}
    virtual void InitializeWorld(World&) {}
};

extern "C" IGameModule* CreateGameModule();
```
- Engine loads the module shared library from `projects/<ProjectName>/`.
- Modules register systems, assets, and replication rules, then build initial world state.
- Optional extensions: world serialization/migration hooks for hot reload and rollback.

### Runtime Behavior
- **ServerRuntime**: authoritative simulation, builds replication deltas, records replays; never predicts or rolls back.
- **ClientRuntime**: prediction + rollback with snapshot store, visual smoothing for corrections.
- **EditorRuntime**: same world model with pause/step/scrub controls and debug overlays.

## Projects Folder
- Each project is a self-contained module under `projects/<ProjectName>/`.
- Minimal example module (`ExampleProject`) can be copied to start new projects.
- Expected layout:
```
projects/MyGame/
├─ project/           # Module source (CreateGameModule)
├─ data/              # Game data/assets
├─ tests/             # Project-specific tests
└─ CMakeLists.txt     # Builds the module as a shared library
```

## Determinism & Networking Rules
- Simulation math should be deterministic (fixed-point or vetted math utilities).
- ECS iteration order is stable and predictable.
- Replication is component-level and explicitly registered.
- Inputs are buffered with fixed delay; client-side rollback uses snapshots.
- Replays, saves, and desync repros all rely on the same snapshot/recording pipeline.

## Workflow to Add a Project
1) Copy the example module under `projects/` and rename it.
2) Implement `CreateGameModule()` and the module interface methods.
3) Register components/systems/replication; initialize world data.
4) Build via CMake; the engine front-ends load the resulting module without engine changes.

