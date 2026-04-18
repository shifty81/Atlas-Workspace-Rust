# Runtime Gameplay Spec — Deep Expansion

## Purpose

`Runtime/` owns all project-specific playable behavior for NovaForge.  
The Engine remains generic; Runtime binds Core, Engine, Builder, PCG, UI, save/load, and mission systems into a playable loop.

Primary loop:

```text
Ship Interior
   ↓
Airlock Cycle
   ↓
EVA / Tethered Traversal
   ↓
Scan / Salvage / Repair / Loot
   ↓
Return to Ship
   ↓
Mission Resolution / Economy / Faction Update
   ↓
Save / Persist / Continue
Design goals
Always-first-person gameplay.

The player uses a rig as the wearable gameplay platform.

Runtime systems must be deterministic where PCG, salvage state, or mission state depend on seeds or replay.

Runtime must be replication-aware, even before full multiplayer.

Runtime UI must be distinct from Atlas Suite editor UI.

Runtime systems should expose debug hooks for Atlas Suite Runtime Debug Mode.

Runtime module responsibility map
Runtime
├── Session
│   ├── boot flow
│   ├── scenario load
│   ├── checkpoint restore
│   └── play session orchestration
├── Gameplay
│   ├── rules
│   ├── state
│   ├── mission phase transitions
│   └── win/fail conditions
├── Player
│   ├── rig controller
│   ├── locomotion
│   ├── interaction
│   ├── vitals
│   └── equipment
├── Inventory / Crafting / Equipment
├── BuilderRuntime / Salvage / Repair
├── Combat / Damage / Hazards
├── Quest / Economy / Faction
├── SaveLoad
└── UI
Vertical-slice scenario
Scenario name
VerticalSlice_StarterDerelict

Initial conditions
Player spawns in starter ship interior.

Starter ship contains at least:

cockpit

corridor

airlock

cargo access

one repairable subsystem

Nearby derelict contains:

one breach or hazard

one mission-critical salvage item

one optional scrap source

One mission is active at session start.

Slice success path
Boot session.

Spawn rig in ship.

Review mission.

Cycle airlock.

EVA with tether attached.

Reach derelict.

Scan components.

Detach or collect mission-critical item.

Survive or resolve one hazard.

Return item to ship or mission receptacle.

Resolve reward.

Save and reload state.

System contracts
SessionBootstrap
Owns:

game mode creation

scenario selection

save restore selection

deterministic seed injection

startup mission injection

spawn routing

VerticalSliceGameMode
Owns:

scenario rules

subsystem registration

fail/success conditions

runtime phase transitions

event hooks to UI and save/load

RigController
Owns:

first-person possession

movement mode switching

interaction focus

equipment activation

hand/tool state coordination

SalvageRuntimeService
Owns:

salvage target registration

scan results

tool requirement validation

detach/cut action processing

loot generation

wreck state delta generation

MissionRuntimeComponent
Owns:

objective state

condition checks

mission event handling

reward dispatch

handoff to faction/economy systems

SaveCoordinator
Owns:

domain save requests

ordered serialization

version stamping

deterministic restore

error reporting

Gameplay state model
SessionState
├── Booting
├── InShip
├── AirlockCycling
├── EVA
├── SalvageActive
├── HazardResponse
├── ReturnTransit
├── MissionComplete
├── Debrief
└── SavePending
Transition rules
Booting -> InShip when scenario and player spawn complete.

InShip -> AirlockCycling when player initiates airlock use.

AirlockCycling -> EVA when outer door opens and pressure rules are satisfied.

EVA -> SalvageActive when player enters salvage interaction range.

SalvageActive -> HazardResponse when fire/breach/threat event triggers.

HazardResponse -> ReturnTransit when required objective is secured and player leaves encounter zone.

ReturnTransit -> MissionComplete when objective handoff validates.

MissionComplete -> Debrief when economy/faction rewards finalize.

Debrief -> SavePending on explicit or automatic checkpoint.

SavePending -> InShip after successful commit.

Core gameplay components
Player / Rig
RigVitalsComponent

oxygen

battery/power

suit integrity

pressure exposure

injury reserve

RigEquipmentComponent

equipped tool slots

quick-use mapping

tier/quality metadata

RigInventoryComponent

carried stacks

mission items

salvage storage limits

RigInteractionComponent

focus trace

interaction prompts

use/hold/progress actions

RigMovementZeroGComponent

push-off

tether stabilization

drift damping

impact handling

Airlock
inner door state

chamber pressure

outer door state

safety lockouts

emergency override state

atmospheric audio/FX hooks

Tether
host source id

oxygen feed state

power feed state

max length

reel speed

obstruction check

emergency disconnect

failover to internal reserves

Salvage
scanable target metadata

material class

structural dependency state

cut points / detach rules

loot table

mission relevance tag

hazard risk tag

Repair
repairable subsystem id

fault state

required parts/tools

staged repair progress

restoration effects

dependency restoration cascade

Hazard model
Required slice hazards
hull breach

electrical fire

Optional first threat
hostile maintenance drone

Breach effects
local pressure loss

oxygen drain acceleration

debris motion / suction hinting

alarm state

door lock or route change effects

Fire effects
local damage over time

visibility degradation

subsystem disable chance

heat damage to nearby nodes

repair urgency escalation

Inventory / item categories
Items
├── Materials
│   ├── scrap metal
│   ├── circuitry
│   ├── sealant
│   └── polymer
├── Consumables
│   ├── oxygen refill
│   ├── battery cell
│   └── repair patch
├── Tools
│   ├── cutter
│   ├── scanner
│   ├── repair tool
│   └── tether tool
├── Mission Items
└── Installed Parts
Mission archetypes for first slice
recover item

repair subsystem

scan target

retrieve and return black box

secure hazardous component

Economy integration
Mission completion can award:

credits

materials

faction standing

unlock tags

repair cost offset

Economy in the first slice should remain intentionally narrow.
It needs only enough value resolution to test:

salvage value

contract reward

repair expenditure

inventory persistence

Faction integration
First-slice faction model:

FactionId

standing value

trust tier

contract eligibility tags

Mission resolution must be able to produce:

standing increase/decrease

unlock of follow-up mission tags

UI debrief text hooks

Save/load domains
Profile
user settings

control mappings

display preferences

Campaign
season profile

long-term faction standing

unlocked tech flags

Session
current mission

rig state

inventory

ship subsystem state

current location

WorldInstance
encounter seed

wreck modifications

detached parts

hazard state

depleted loot nodes

Debug requirements
Atlas Suite Runtime Debug Mode must expose:

spawn mission override

item injection

oxygen/power override

trigger breach

ignite fire

complete objective

checkpoint save/load

encounter seed swap

subsystem repair state toggle

Telemetry / analytics events
Minimum events:

session_started

airlock_cycle_started

eva_started

salvage_scan_completed

salvage_detach_completed

mission_item_secured

breach_triggered

fire_triggered

subsystem_repaired

mission_completed

save_created

save_loaded

Performance and scaling notes
Keep mission/event buses lightweight.

Avoid hard references from every salvage node to UI systems.

Prefer event dispatch and service registries.

Store deterministic deltas instead of whole-world state blobs.

Build replication-safe payload shapes now even if offline-first.

Definition of done
The Runtime Gameplay slice is complete when a player can:

boot into a controlled scenario

leave ship through a functioning airlock

EVA with tether behavior

salvage at least one mission-critical object

encounter and resolve one hazard

complete mission resolution

receive economy/faction results

save and reload without losing encounter state
