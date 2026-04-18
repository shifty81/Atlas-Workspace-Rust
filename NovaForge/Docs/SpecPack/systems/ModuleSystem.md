# Module System Specification

## Overview
Modules are installable components that extend R.I.G., ship, and base capabilities.
All modules consume power and occupy slots.

## Existing Implementation
- Schema: `Schemas/module.schema.json` — full validation contract
- System: `module_system.h`, `module_power_grid_system.h`
- Subsystems: `module_cascading_failure_system.h`, `module_overheat_system.h`, `module_capability_system.h`
- Data: `Data/Modules/` — capital_modules, defensive, drones, engine, faction, utility
- Components: `RigModule` in fps_components.h

## Module Types (Existing Schema)
Weapon, Shield, Engine, Reactor, Cargo, Mining, Scanner, Repair, Armor, Utility

## R.I.G. Module Types (Existing Code)
LifeSupport, PowerCore, JetpackTank, Sensor, Shield, EnvironFilter,
ToolMount, WeaponMount, DroneController, ScannerSuite, CargoPod,
BatteryPack, SolarPanel

## Module Installation Contract
```
1. Check: slot available on target (R.I.G. / ship / base)
2. Check: module tier <= target tier
3. Check: power budget has capacity for module.power_cost
4. Install: occupy slot, add to power grid
5. Activate: module goes online if power available
```

## Missing Contracts
- R.I.G. slot schema (which slots accept which module types)
- Module dependency graph (some modules require others)
- Installation/removal animation states
- Module damage and repair states
- Upgrade paths (module tier progression)

## Schema File
→ `Schemas/module.schema.json` (EXISTS)
