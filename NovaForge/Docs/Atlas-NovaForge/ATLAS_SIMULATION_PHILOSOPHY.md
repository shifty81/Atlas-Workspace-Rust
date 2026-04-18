# Atlas Engine — Simulation Philosophy

## Core Principle

The Atlas Engine simulates a universe. It does not script experiences.

Every system in Atlas follows one rule: **the simulation runs whether or not the player is watching.**

## Layered Architecture

The simulation is strictly layered to prevent narrative pollution:

### Layer 1 — Core Simulation (Always On)
- Economy, threat levels, NPC activity, logistics
- Pure math and rules
- Deterministic, save-file authoritative
- Does not "know" about the player as special

### Layer 2 — Evidence & Anomalies
- Warp instability, resource irregularities, route changes, wreck density
- This layer reacts to upstream pressure from Layer 1
- Produces observable effects, not explanations
- Player discovers patterns, not facts

### Layer 3 — Social Interpretation
- Fleet chatter, captain beliefs, morale, rumors, disagreements
- This layer interprets Layer 2
- Meaning emerges through people, not UI popups
- Two players in the same galaxy may interpret events differently

### Layer 4 — Meta-Threat Systems (Late Game, Optional)
- Titan assembly pressure, pirate coalition doctrine, galactic response curves
- Applies pressure downward into Layers 1–3
- Never narrates upward
- Can be disabled entirely for early versions without breaking anything

## Tick-Based Determinism

All simulation systems are tick-based and deterministic:

- Fixed delta time per tick
- Same inputs always produce same outputs
- No randomness that isn't seed-based
- Enables networking, replay, and debugging

## System Update Order

Systems execute in a strict order each tick:

1. **BackgroundSimulationSystem** — Galaxy moves first
2. **SectorTensionSystem** — Per-system resource/threat updates
3. **TradeFlowSystem** — Economic supply/demand
4. **PirateDoctrineSystem** — Pirate coalition behavior
5. **TitanAssemblySystem** — Meta-threat pressure (if enabled)
6. **GalacticResponseSystem** — Faction AI reactions
7. **NPCIntentSystem** — Individual NPC decisions
8. **OperationalWearSystem** — Logistics fatigue
9. **RumorPropagationSystem** — Information spreading
10. **Social Systems** — Morale, chatter, emotional arcs

This order ensures no circular dependencies and no narrative cheating.

## Economy as Cause

The economy is not a reward dispenser. It is a causal system:

- Resources are extracted → moved → produced → destroyed
- Shortages emerge naturally
- AI reacts to shortages by changing behavior
- Player actions create ripples, not quests

## AI Equality

AI entities obey the exact same rules as players:

- Same navigation, weapons, targeting, cooldowns
- No hidden stat bonuses
- Difficulty comes from numbers and coordination, not cheating
- AI intent is probabilistic and role-based

## What Atlas Never Does

- ❌ Spawns "content" on demand
- ❌ Uses quest flags or script triggers
- ❌ Gives AI information the player can't access
- ❌ Makes the player "special" mechanically
- ❌ Creates events that only exist when observed

## What Atlas Always Does

- ✔ Runs the simulation continuously
- ✔ Produces deterministic, replayable results
- ✔ Lets consequences propagate through systems
- ✔ Makes the universe feel older than the player
- ✔ Treats every entity equally under the rules
