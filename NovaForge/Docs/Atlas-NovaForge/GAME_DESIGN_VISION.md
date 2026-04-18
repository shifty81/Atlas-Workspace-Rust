# Atlas NovaForge — Game Design Vision

> **Status**: Active Design  
> **Date**: March 2026  
> **Scope**: Consolidated game design vision from planning sessions, to be implemented as the project matures

---

## Identity

Atlas NovaForge is a **procedural legend engine** — a tri-modal (FPS + Flight + Fleet Command) PvE space simulator where player behavior shapes the universe through emergent legends, faction pressure, and world bias.

**North Star**:  
> NovaForge is a system that turns player behavior into legend, legend into world bias, and world bias into new stories — without ever scripting the outcome.

---

## Three-Layer Experience Stack

### Layer 1 — Sandbox (Moment-to-Moment Truth)

The lowest layer and the most important. AI reacts to room + biome + reputation. Physics, damage, breaches, and zero-G resolve honestly. Player improvisation creates chaos. **This layer never lies** — no scripting, no fake outcomes.

### Layer 2 — Campaign Engine (Structure & Intent)

The flow graph + mission system. Gives pacing, prevents randomness soup, makes runs feel authored. Campaigns frame the sandbox so memorable events can emerge — but they don't dictate outcomes.

### Layer 3 — Roguelike Legend System (Memory & Myth)

The long game. Player reputation persists, enemy tactics evolve, bosses mutate across runs, future missions bias generation rules. Legends never punish the player — they reshape expectations.

**Feedback loop**: Sandbox creates events → Campaign frames meaning → Legend system remembers patterns → Future campaigns bias generation → New sandbox moments emerge.

---

## Player Modes (Hard Boundaries)

The game features three distinct player modes with hard walls between them.

```cpp
enum class PlayerMode {
    FPS,
    FLIGHT,
    FLEET_COMMAND
};
```

### Mode Ownership Matrix

| System | FPS | Flight | Fleet Command |
|--------|-----|--------|---------------|
| Input | ✓ | ✓ | ✓ |
| Camera | ✓ | ✓ | ✓ |
| UI Stack | ✓ | ✓ | ✓ |
| Time Scale | 1.0 | 1.0 | 0.1–0.25 |
| Entity Authority | Pawn | Ship | Fleet AI |
| Player Body | Physical | Abstracted | Abstracted |

### Mode Transition Rules

Every transition must:
1. Fade / dissolve control
2. Transfer authority
3. Swap camera
4. Swap input map
5. Swap UI stack
6. Restore context on exit

No shortcuts. All transitions go through the `PlayerModeController`.

---

## Fleet Command (RTS Layer)

### Command Vocabulary (6 orders)

| Command | Meaning |
|---------|---------|
| ENGAGE | Seek & fight enemies |
| DEFEND | Hold position, intercept threats |
| ESCORT | Protect target |
| RETREAT | Disengage intelligently |
| HOLD | Freeze formation |
| REGROUP | Reform fleet |

Orders are **intent**, not control. Fleet AI translates intent into ship behavior.

```cpp
struct FleetOrder {
    FleetIntent intent;
    EntityID target;
    float aggression;  // 0.0 – 1.0
};
```

### RTS Mode Rules

- Time scale: 0.15x
- Weapons disabled
- Thrusters disabled
- Camera = tactical only
- Exit returns to exact prior flight state

### AI Authority Layers

| Tier | Scope |
|------|-------|
| Individual | FPS enemies |
| Crew | Turrets, repairs |
| Ship AI | Piloting, targeting |
| Fleet AI | Positioning, doctrine |

RTS mode never controls ships directly — it controls fleet intent.

---

## FPS Systems

### Combat State Machine

```
EXPLORING → ALERT → ENGAGED → WOUNDED → DOWNED
                                ↓
                            BREACHED (hull breach)
```

- Movement speed −15% in WOUNDED
- Breathing audio + vignette in WOUNDED
- Aim sway increased in BREACHED
- No healing during ENGAGED unless injector used

### Enemy Archetypes

| Type | Role | Behavior |
|------|------|----------|
| Raider Grunt | Pressure | Advance → cover → fire |
| Breacher | Flush | Push aggressively, break stalemates |
| Technician | Support | Deploy turret, repair, retreat when shot |

### Squad Roles

| Role | Behavior |
|------|----------|
| Breacher | Rushes doors, throws opening attack |
| Suppressor | Anchors sightlines, forces cover |
| Flanker | Delayed entry, punishes tunnel vision |

### Interior Generation Rules

Procedural interiors obey FPS grammar:

```
AIRLOCK → CHOKE → PRIMARY COMBAT → OPTIONAL SIDE ROOM → OBJECTIVE
```

Combat room rules:
- 2–3 entry angles max
- Cover every 4–6 meters
- One elevation change only
- Clear sightline priority

---

## Low-Poly Visual Style (Procedural Enforcement)

### Geometry Rules

- Flat shading at mesh level (duplicate vertices per face, assign face normals)
- Hard edges only — no bevels
- Poly budget: corridor 12–24 faces, room 30–60 faces, door 6–12 faces

### Allowed Primitives

Box, Wedge, Trapezoid, Hex prism (rare).

**Forbidden**: Cylinders (unless 6–8 sided), spheres, curves.

### Color & Material

- 6–10 total colors per biome palette
- No roughness maps, no normal maps
- Unlit or very simple lit
- Vertex color gradients only (optional)
- Lighting creates contrast, not materials

### Style Validation

```cpp
bool ValidateLowPolyStyle(Mesh& m) {
    return m.faceCount < MaxFaces &&
           m.hasFlatNormals &&
           m.materialCount <= 2 &&
           m.noBevelEdges();
}
```

If validation fails → regenerate, simplify, or reject.

---

## Legend & Reputation System

### Memory Taxonomy

| Tier | Persistence | Examples |
|------|-------------|---------|
| **Persistent Legend** | Never forgotten | Signature playstyle, boss archetypes defeated, campaign-ending choices |
| **Campaign Memory** | Fades slowly | Faction treatment, mission success patterns, lethal biomes |
| **Run Memory** | Ephemeral | Camping, weapon overuse, retreat patterns |

### Explicitly Forgotten (By Design)

Individual deaths, exact room layouts, specific weapon loadouts, RNG outcomes, momentary mistakes. Forgetting keeps legends mythic.

### Promotion Rules

A memory becomes a Legend only if it:
- Appears across multiple runs
- Survives different biomes
- Meaningfully affects difficulty

### Boss Mutation

Bosses mutate based on legend pressure:

| Legend Trait | Boss Response |
|-------------|---------------|
| High explosives | Shield phasing |
| High precision | Moving weakpoints |
| High stealth | Area scans |
| Aggression | Counter-rush modules |

Max 2 mutations per boss. Never remove all weaknesses. Bosses telegraph changes visually.

---

## Campaign Structure

Campaigns are 3–5 chapters, each with a theme:

| Chapter | Purpose |
|---------|---------|
| Incursion | Orientation, simple flows |
| Escalation | Tension, set-pieces appear |
| Fracture | Instability, breaches, zero-G |
| Confrontation | Boss encounter, legend expression |
| Aftermath (optional) | Memory formation, seeds future legends |

---

## Procedural Solar System Density

Systems are generated from seed with variable density:

- **Stations**: 1–7 (rolled per system)
- **Mining Belts**: 2–8 (biased by security level)
- **Pirate Zone**: 1 (weighted by security)
- **Anomalies**: 3–5 (already supported)
- **AI Traffic**: Scaled to station + belt count

Station types include: Trade Hub, Refinery, Mining Outpost, Research Array, Military Patrol Base, Black Market Node.

---

## World Simulation (Faction Ecosystem)

### Sector Model

```cpp
struct Sector {
    Faction controllingFaction;
    float stability;       // 0.0 = chaotic, 1.0 = stable
    float hostility;       // toward player and outsiders
    float resourceWealth;  // loot and supply potential
};
```

### Heat Propagation

Player actions emit "heat" — pressure that propagates through sectors:

```
EmitHeat(systemID, factionID, magnitude)
```

Heat propagates to neighbors each tick. Sector heat averages into galaxy pressure.

### Drift Stabilizers

- **Faction Fatigue**: Dominant factions spawn internal instability
- **Economic Elasticity**: Markets drift toward equilibrium price
- **Power Vacuum**: Destroyed stations create temporary pirate nodes
- **Era Resets**: Periodic rebalancing of dominant factions and resources

---

## Golden Path Mission: "Derelict Intercept"

The vertical slice proving all three modes work:

1. Accept contract
2. Fly to system (Flight)
3. Enemy fleet detected
4. Enter Fleet Command → issue ENGAGE + FLANK
5. Resume Flight → combat
6. Disable enemy ship
7. Dock / EVA
8. FPS boarding combat
9. Loot core module
10. Escape
11. Debrief + rewards

**If this mission works flawlessly, the game works.**

---

## Design Rules (Non-Negotiable)

1. **No scripted outcomes** — systems resolve honestly
2. **No hand-authored hero moments** — emergence only
3. **No permanent player penalties from legends** — bias, not punishment
4. **Prefer data-driven bias** over certainty
5. **Prefer systems that interact**, not override
6. If a feature doesn't serve legend formation, world bias, or emergent storytelling → it doesn't belong
