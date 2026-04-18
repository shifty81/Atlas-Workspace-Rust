# Nova-Forge-Expeditions — Usable Snippets

Patterns and code structures extracted from the Nova-Forge-Expeditions audit
that serve as reference for future development.

## Game System Pattern (cpp_server/src/systems/)

All 449 game systems follow one of these patterns:

### SingleComponentSystem Template
```cpp
template <typename C>
class SingleComponentSystem : public ISystem {
public:
    void tick(float dt) override {
        for (auto& [id, comp] : m_world->components<C>()) {
            process(id, comp, dt);
        }
    }
protected:
    virtual void process(EntityID id, C& comp, float dt) = 0;
};
```

### StateMachine System
```cpp
template <typename C, typename PhaseEnum>
class StateMachineSystem : public SingleComponentSystem<C> {
protected:
    void process(EntityID id, C& comp, float dt) override {
        switch (comp.phase) {
            case PhaseEnum::Idle:    onIdle(id, comp, dt); break;
            case PhaseEnum::Active:  onActive(id, comp, dt); break;
            case PhaseEnum::Cooldown: onCooldown(id, comp, dt); break;
        }
    }
    virtual void onIdle(EntityID, C&, float) {}
    virtual void onActive(EntityID, C&, float) {}
    virtual void onCooldown(EntityID, C&, float) {}
};
```

## PCG Framework Pattern (engine/procedural/)

```cpp
class PCGManager {
    DeterministicRNG m_rng;
    ConstraintSolver m_solver;
    BuildQueue m_queue;
public:
    void generateHull(const HullParams& params);
    void generateInterior(const InteriorParams& params);
    bool verify(const PCGResult& result);
};
```

## Editor Tool Interface (engine/tools/)

```cpp
class ITool {
public:
    virtual ~ITool() = default;
    virtual const char* name() const = 0;
    virtual void activate() = 0;
    virtual void deactivate() = 0;
    virtual void update(float dt) = 0;
    virtual void render(RenderContext& ctx) = 0;
};
```

## Blender Addon Ship Presets

JSON preset files for procedural ship generation:
- `fighter.json` — small, fast hull
- `frigate.json` — medium hull with turret hardpoints
- `cruiser.json` — large hull with module bays
- `battleship.json` — heavy hull with armor plating
- `capital.json` — massive hull with hangar bays

## Message Handler Pattern

```cpp
class IMessageHandler {
public:
    virtual ~IMessageHandler() = default;
    virtual void handle(const Message& msg, GameSession& session) = 0;
};

class CombatHandler : public IMessageHandler {
    void handle(const Message& msg, GameSession& session) override;
};
```
