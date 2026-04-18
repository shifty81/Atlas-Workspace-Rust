# Atlas Engine — Architecture

## Core Runtimes

Atlas ships as three executables, all linked against the same engine core:

| Binary         | Purpose                          |
|----------------|----------------------------------|
| `atlas_editor` | Full editor + runtime            |
| `atlas_client` | Player runtime                   |
| `atlas_server` | Headless authoritative server    |

## Engine Layers

```
Platform
└─ Core (math, memory, jobs, time, logging)
   └─ ECS (entity/component/system)
      └─ Graph VM (deterministic gameplay logic)
         └─ Gameplay Systems (data-driven)
            └─ Rendering / Physics / Audio
```

## Runtime Roles

```cpp
enum class RuntimeRole {
    Editor,
    Client,
    Server
};
```

### Capability Mask

| Capability      | Editor | Client | Server |
|-----------------|--------|--------|--------|
| Asset Write     | ✅     | ❌     | ❌     |
| Rendering       | ✅     | ✅     | ❌     |
| Physics         | ✅     | ✅     | ✅     |
| Graph Edit      | ✅     | ❌     | ❌     |
| Graph Execute   | ✅     | ✅     | ✅     |
| Net Authority   | ⚠️     | ❌     | ✅     |
| Hot Reload      | ✅     | ⚠️     | ⚠️     |

## Boot Flow

```cpp
int main() {
    EngineConfig cfg;
    cfg.mode = EngineMode::Editor; // or Client, Server

    Engine engine(cfg);
    engine.InitCore();      // time, logging
    engine.InitRender();    // OpenGL/Vulkan
    engine.InitUI();        // Atlas UI
    engine.InitECS();       // empty world
    engine.InitNetworking();// idle or local
    engine.InitEditor();    // inspector, console (editor only)

    engine.Run();
}
```

## Core Engine Loop

```cpp
while (engine.Running()) {
    engine.Time.BeginFrame();
    engine.Net.Poll();
    engine.ECS.Tick(engine.Time.Delta());
    engine.UI.Update();
    engine.Render.Draw();
    engine.Time.EndFrame();
}
```
