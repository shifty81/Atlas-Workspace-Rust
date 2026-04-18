# R.I.G. AI Core Specification

## Overview
The R.I.G. AI is embedded in the backpack. It provides contextual assistance,
monitoring, and late-game autonomous capabilities.

## Feature Flags (Unlocked by modules)

| Feature | Unlock Condition | Function |
|---------|-----------------|----------|
| vitals_monitoring | Always (Tier 0) | Health/O2/energy warnings |
| scanning | Scanner module (Tier 1) | Object/material identification |
| mapping | Scanner module (Tier 1) | Area mapping and waypoints |
| navigation | Wrist terminal (Tier 2) | Pathfinding assistance |
| drone_control | Drone controller (Tier 2) | Command deployed drones |
| healing_control | Medical module (Tier 3) | Auto-triage, stim injection |
| fleet_command | Neural link (Tier 3) | AI fleet coordination |

## Event Inputs
```cpp
enum class RigAIEvent : uint8_t {
    PowerChanged,       // power level shifted
    HealthChanged,      // damage taken or healed
    OxygenLow,          // O2 below 20%
    ScanResult,         // scan completed
    ThreatDetected,     // hostile in range
    EnvironmentChanged, // entered new hazard zone
    ModuleInstalled,    // new module added
    ModuleRemoved,      // module removed
    DroneStatus,        // drone state changed
    SystemFailure       // critical system offline
};
```

## Event Outputs
- HUD alerts (text, icon, severity)
- Recommendations (suggested actions)
- Drone commands (if drone_control enabled)
- Auto-healing (if healing_control enabled)
- Fleet orders (if fleet_command enabled)

## C++ Contract
```cpp
struct RigAIFeatures {
    bool vitals = true;
    bool scanning = false;
    bool mapping = false;
    bool navigation = false;
    bool droneControl = false;
    bool healingControl = false;
    bool fleetCommand = false;
};

class RigAICore {
    void init(const RigAIFeatures& features);
    void onEvent(RigAIEvent event, float value = 0.f);
    void tick(float dt);
    void enableFeature(const std::string& feature);
    void disableFeature(const std::string& feature);
    const RigAIFeatures& features() const;
    size_t pendingAlerts() const;
};
```

## Existing Code
- AtlasAI broker system (workspace-level AI)
- `ai_system.h` — server-side AI
- `Docs/AI/` — AI architecture documentation

## Missing
- R.I.G.-specific AI event bus
- Feature flag struct as runtime component
- Power scaling (AI features consume power)
- Priority queue for AI recommendations
