# Atlas Engine — Naming Conventions

This document is canonical for all Atlas Engine development and Atlas-based games, including EVE-Offline.

## 1. Component Naming

Components are pure data structs. They inherit from `ecs::Component` and use the `COMPONENT_TYPE()` macro.

**Rules:**
- PascalCase for struct names
- No prefix for game-level components (e.g., `CaptainPersonality`, `FleetMorale`)
- No `Manager`, `Controller`, or `Handler` suffixes
- Components contain NO logic — only data fields
- Components must be serializable

**Examples:**
```cpp
struct CaptainPersonality : ecs::Component {
    COMPONENT_TYPE(CaptainPersonality)
    float aggression = 0.5f;
    float loyalty = 0.5f;
};

struct SectorTension : ecs::Component {
    COMPONENT_TYPE(SectorTension)
    float resourceStress = 0.0f;
    float piratePressure = 0.0f;
};
```

## 2. System Naming

Systems are stateless logic processors. They are deterministic and scheduled by the ECS.

**Rules:**
- PascalCase with `System` suffix
- Named by domain + action (e.g., `FleetMoraleSystem`, `SectorTensionSystem`)
- Systems contain NO persistent state — only process components
- File names use snake_case: `fleet_morale_system.cpp`

**Examples:**
```cpp
class FleetMoraleSystem { ... };       // fleet_morale_system.cpp
class SectorTensionSystem { ... };     // sector_tension_system.cpp
class TitanAssemblySystem { ... };     // titan_assembly_system.cpp
```

## 3. File Naming

| Type | Convention | Example |
|------|-----------|---------|
| Header files | `snake_case.h` | `fleet_morale_system.h` |
| Source files | `snake_case.cpp` | `fleet_morale_system.cpp` |
| Data files | `snake_case.json` | `chatter_templates.json` |
| Documentation | `UPPER_SNAKE_CASE.md` | `ATLAS_NAMING_CONVENTIONS.md` |

## 4. Namespace

All engine and game code lives in the `atlas` namespace:

```cpp
namespace atlas {
    struct CaptainPersonality : ecs::Component { ... };
    class FleetMoraleSystem { ... };
}
```

## 5. Directory Structure

```
cpp_server/
  include/
    components/game_components.h    # All ECS components
    systems/*.h                     # System headers
    data/*.h                        # Data loaders
  src/
    systems/*.cpp                   # System implementations
    data/*.cpp                      # Data implementations
```

## 6. Events & Signals

Events are explicit structs, immutable, and passed through system interfaces:

```cpp
// Events use the pattern: Domain + Event suffix
struct FleetMoraleShiftEvent {
    uint64_t fleetId;
    float delta;
};
```

No anonymous callbacks. No lambdas for event handling.

## 7. Enumerations

- Use `enum class` (not plain `enum`)
- PascalCase for enum names and values

```cpp
enum class WarpPhase { None, Align, Accelerate, Cruise, Decelerate };
enum class MoraleState { Inspired, Steady, Doubtful, Disengaged };
enum class PirateDoctrine { Accumulate, Conceal, Disrupt, Defend, PrepareLaunch };
```

## 8. Data Constants

- Use `constexpr` or `static const` for compile-time constants
- ALL_CAPS with underscores for constant names

```cpp
constexpr float MAX_MORALE = 100.0f;
constexpr float MIN_MORALE = -100.0f;
constexpr int MAX_FLEET_SIZE = 25;
```

## 9. Design Enforcement

Every new system must answer YES to all:

1. **Simulation-first**: Does it function without UI or player input?
2. **Lore-consistent**: Would captains believe this exists?
3. **Deterministic**: Does it produce the same output given the same input?

If NO to any → it does not ship.
