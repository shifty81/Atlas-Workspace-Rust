# Nova Forge — Design Ideas Extraction

> **Extracted from**: `docs/archive/ideas and improvements.txt`
> **Date**: March 16, 2026
> **Purpose**: Curated, viable ideas organized by domain for staged implementation.
> All ideas below have been validated against the project's design enforcement rules:
> Atlas-fit · Simulation-first · Lore-consistent.

---

## 1. Cinematic Warp System (PVE-Optimized)

### 1.1 Warp Tunnel Shader Stack (5 Layers)

| Layer | Description | Budget |
|-------|-------------|--------|
| Space Compression Field | Screen-space radial distortion, center-weighted, slow oscillation (0.05–0.1 Hz) | ≤ 0.3 ms |
| Starfield Velocity Bloom | Stars stretch into lines, length proportional to warp speed, chromatic split at tips | ≤ 0.2 ms |
| Warp Tunnel Skin | Procedural noise shell (gravitational lensing, not a literal tube) | ≤ 0.4 ms |
| Ship Silhouette Anchor | Ship visible with rim-light and sub-pixel camera-relative bob | included |
| Vignette + Peripheral Falloff | Darken edges 10–18%, soft blur at periphery, center crisp | ≤ 0.2 ms |

**Total visual budget**: ≤ 1.2 ms at 1080p/60fps.

### 1.2 Adaptive Warp Audio System

| Audio Layer | Description |
|-------------|-------------|
| Engine Core (Sub Bass) | 30–60 Hz sine, constant during warp, felt not heard |
| Warp Field Harmonics | Mid-frequency shimmer, phase-shifted stereo drift, slow LFO |
| Environmental Shimmer | Randomized one-shots at distance milestones (not time) |
| Meditation Layer | Sustained pads (no melody/rhythm), fades in after ~15s, out before exit |

Audio progression: 0–5s tension → 5–15s stabilize → 15–40s ambient bloom → 40s+ meditative state.

### 1.3 Dynamic Warp Intensity (Ship Mass–Driven)

- `warpIntensity = smoothstep(0, 4, t) * lerp(0.6, 1.4, massNorm)`
- Heavier ships: more distortion, deeper bass, slower tunnel oscillation, longer exit bloom
- Capital ships: warp "takes longer to settle" — feels inevitable, not fast

### 1.4 Warp Anomalies & Rare Mid-Warp Events

| Category | Frequency | Examples |
|----------|-----------|----------|
| Visual-Only (Common) | 1 per 3–5 long warps | Tunnel eddies, color shifts, phantom star clusters |
| Sensory Distortions (Uncommon) | 1 per 10–15 long warps | Bass deepens, harmonics detune, tunnel thickens |
| Spacetime Shear (Rare) | 1 per 50–100 warps | Fracture visuals, distant silhouettes, ghost systems |
| Legendary Events (Ultra-Rare) | Opt-in/seeded | Collapsed gate scars, ancient battlefield echoes |

**Core rule**: Anomalies never interrupt control, never cause failure, never require reaction.

### 1.5 Accessibility Settings

- Motion Intensity slider (0–100%)
- Bass Intensity slider
- Peripheral Blur scalar
- Tunnel Geometry toggle (off = star streaks only)
- Auto-comfort: reduce oscillation on FPS drop, clamp distortion on ultrawide

---

## 2. Fleet Personality & Social Systems

### 2.1 Captain Personality Axes

```
aggression:      cautious ↔ bold
sociability:     quiet ↔ talkative
optimism:        grim ↔ hopeful
professionalism: formal ↔ casual
```

No archetype labels shown to player — personality is inferred over time through behavior and dialogue.

### 2.2 Activity-Aware Fleet Chatter

Chatter occurs during all activities, not just combat:

| Activity | Chatter Style |
|----------|--------------|
| Warp (long) | Reminiscing, speculation, philosophical drift |
| Mining/Salvage | Yield comments, monotony complaints, casual life talk |
| Combat | Short/sharp warnings, praise, frustration, panic (low morale) |
| Exploration | Lore hints, wonder, map speculation |
| Idle/Holding | Minimal chatter, occasional check-ins, intentional silence |

**Timing**: One line every 20–45s max. No chatter during warp entry/exit. Lines can be cut off by warp exit.

### 2.3 Interruptible Chatter System

- Each line has `priority` and `interruptible` flag
- Higher-priority events (combat, anomalies, warp exit) cut lower-priority chatter
- Interrupted lines may resume later or never finish (memory decay)
- Reactions to interruptions: `"—and that was the third time I—" "Did anyone else just feel that?"`

### 2.4 Positional Audio in Warp Tunnel

- Each captain's voice originates from their ship's formation position
- Volume/clarity affected by distance and formation spread
- Warp: slight echo, phase-shifted reverb, pitch warble
- Combat: cleaner, sharper, less reverb

### 2.5 Fleet Memory & Morale

- Persistent per-captain: missions together, wins, losses, ships lost, saves given/received
- Morale formula: `wins*1.0 - losses*1.5 - shipsLost*2.0 + timesSavedByPlayer*1.2` (clamped -100 to +100)
- High morale: more chatter, confidence, tighter formations
- Low morale: hesitation, silence, looser formations
- Critical morale: captains may refuse risky missions

### 2.6 Captain Social Graph

- Bidirectional asymmetric `affinity` (-100 grudge → +100 bond) between captains
- Modifiers: saved in combat (+10), abandoned (-20), shared warp/mining time (+passive), kill credit stolen (-8)
- Friends: fly closer, back each other faster, reference each other in chatter
- Grudges: fly wider, delay responses, sarcastic/clipped comms

### 2.7 Emotional Arcs Across Campaigns

- Per-captain: `confidence`, `trustInPlayer`, `fatigue`, `hope`
- Updated slowly (not per-frame)
- Emergent arcs: The Optimist (starts hopeful, quiets after losses), The Survivor (quiet early, mentors later)

### 2.8 Captain Departures & Transfers

- Triggers: long-term losing streak, repeated near-death, personality mismatch
- Progression: argue in chatter → request changes → ask for transfer → leave
- High morale captains may request bigger ship or lead position
- Low morale captains may ask for escort-only roles or temporary leave
- Splinter fleets possible if grudges + low morale combine

### 2.9 Chatter Reacting to Player Silence

- Track `timeSinceLastCommand` and `timeSinceLastSpeech`
- Short silence = normal; Long silence = focused; Very long = detached/stressed
- High-trust captains ask ("You alright up there?"), low-trust assume

### 2.10 Four Faction Personality Profiles

| Faction | Style | Morale Driver | Example Line |
|---------|-------|---------------|-------------|
| Industrial/Pragmatic | Low combat chatter, more mining/warp talk | Efficiency | "Cargo's clean. That's what matters." |
| Militaristic/Proud | Competitive, tracks kills obsessively | Victory | "We should've hit harder." |
| Nomadic/Superstitious | Heavy rumor belief, emotional anomaly reactions | Exploration | "That wasn't empty space." |
| Corporate/Detached | Formal speech, leaves politely when unhappy | Success metrics | "This operation no longer aligns." |

---

## 3. Fleet-as-Civilization Model

### 3.1 Fleet Cargo Pool

- Each ship has `ShipCargoComponent` (capacity, items, `contributesToFleetPool` flag)
- `FleetCargoAggregatorComponent` pools opted-in ships' spare cargo
- Items physically live on ships — loss of a ship = real cargo loss
- At titan scale, inventory is effectively bottomless; constraint shifts from space → logistics/organization

### 3.2 Fleet Composition (25-Ship Endgame)

| Phase | Ships | Wings | Key Mechanics |
|-------|-------|-------|--------------|
| Core Fleet (v0.1–0.2) | 5 | 1 | Player + 4 captains, basic personality/chatter |
| Wing System (Midgame) | 15 | 3×5 | Wing commanders, role specialization (mining/combat/logistics) |
| Full Doctrine (Endgame) | 25 | 5×5 | Ideology, fracture mechanics, station deployment ships |

### 3.3 Station Deployment Ships

- A ship class that deploys into a station
- Attach modules to upgrade station features
- Upgrades affect stats of the solar system the fleet settles in
- Stations do NOT persist across worlds — only fleet/character/skills transfer

---

## 4. Tactical Overlay (Strategy View)

### 4.1 Core Design

- Passive 2.5D spatial overlay — flattens 3D space into readable tactical plane
- Shows truthful distances and positioning
- Not a minimap, not a radar, not interactive
- Toggle: `V` hotkey (configurable), icon near ship HUD cluster

### 4.2 Visual Elements

- **Distance Rings**: Concentric circles at fixed world-unit radii (5/10/20/30/50/100), thin, translucent
- **Tool Range Ring**: Dynamic ring for active tool, thicker stroke, color-coded
- **Entity Projection**: Radial distance preserved, vertical (Z) compressed with tick encoding
- **Entity Rendering**: Small icon per type, color from standings, muted asteroids, high-contrast hostiles

### 4.3 Implementation Stages

| Stage | Features |
|-------|----------|
| Skeleton | Toggle + distance rings only (validates projection math) |
| Entity Projection | Flat projection + vertical ticks + shared Overview filters |
| Tool Awareness | Active tool range ring, color coding |
| Fleet Extensions | Anchor rings (leader only), wing distance bands |

---

## 5. Living Galaxy Simulation (EDNMS Integration)

### 5.1 Star System State Model

Per-system normalized floats: `economicHealth`, `tradeVolume`, `securityLevel`, `threatLevel`, `trafficLevel`, `stability`, `factionInfluence[]`, `notoriety`.

### 5.2 Background Simulation Loop

- Runs on system entry, fixed intervals, and save/load
- Economy drifts with trade vs threat
- Security responds to faction presence
- Threat grows inversely to security
- Traffic reacts to economy vs threat
- Threshold-based events: pirate surge (threat > 0.7), shortages (economy < 0.3), lockdown (security < 0.2)

### 5.3 NPC Intent-Driven Behavior

- Intent enum: Idle, Trade, Patrol, Hunt, Explore, Salvage, Flee
- Intent scoring from system state × NPC personality traits
- Behavior trees per intent (Trader: check threat → navigate → dock → trade → choose next)

### 5.4 Ambient Life

- Civilian shuttles, mining drones, distress beacons, salvage fields
- Non-combat events: nav beacon malfunction, station lockdowns, radiation storms
- Discoveries attract NPC traffic and trigger faction interest

---

## 6. Pirate Titan Meta-Threat

### 6.1 Distributed Assembly Model

- Titan composed of 6 macro assemblies (Superstructure, Reactor, Warp Projector, Command, Armor, Weapons)
- Each tracks: `completion`, `resourceDebt`, `workforceSkill`, `concealment`, `currentSystem`
- Assemblies move between systems via deadspace pockets and masked convoys
- Player cannot "stop the Titan" — only slow, damage, or destabilize

### 6.2 Pirate Coalition AI Doctrine

- Doctrine states: Accumulate → Conceal → Disrupt → Defend → PrepareLaunch
- Shifts driven by: Titan completion %, discovery risk, resource scarcity, player proximity
- Early: hit-and-fade, protect industrial assets
- Mid: coordinated fleets, decoy raids, escort-heavy logistics
- Late: system denial, gate camps with purpose, strategic retreat

### 6.3 Warp Anomaly Evidence

- Macro assemblies emit non-standard warp residue (mass echoes, temporal shear, vector noise)
- Player experiences: longer warp exit, fleet chatter interrupts, audio bass phasing
- Repeated anomalies increase certainty, reduce deniability, affect AI behavior

### 6.4 Fleet Chatter Tied to Titan Progress

| Titan % | Phase | Chatter Themes |
|---------|-------|----------------|
| 0–20% | Rumor | Jokes, denial, gallows humor |
| 20–50% | Unease | Arguments, tactical disagreements, hesitation |
| 50–80% | Fear | Nightmares, leave requests, philosophical debates |
| 80–100% | Acceptance | Resignation or resolve, loyalty tests, potential mutiny |

### 6.5 Galactic Response Curves

| Titan % | Faction Behavior |
|---------|-----------------|
| 10% | Ignore rumors |
| 25% | Border patrol increases |
| 40% | Capital production prioritized |
| 60% | Outer systems abandoned |
| 75% | Emergency doctrines activated |
| 90% | Pre-emptive strikes |
| 100% | Galaxy permanently altered |

---

## 7. 4-Layer System Architecture

| Layer | Responsibility | Examples |
|-------|---------------|----------|
| **1 — Core Simulation** | Pure math, no lore | Economy, threat, NPC activity, logistics |
| **2 — Evidence & Anomalies** | Effects, not explanations | Warp instability, resource irregularities, wreck density |
| **3 — Social Interpretation** | Meaning emerges socially | Fleet chatter, captain beliefs, morale, rumors |
| **4 — Meta-Threat** | Pressure downward, never narrates upward | Titan assembly, pirate doctrine, galactic response |

Layer 4 can be disabled entirely for v0.1 without breaking anything.

---

## 8. Additional Viable Features

### 8.1 Imperfect Information & Diegetic Knowledge

- Long-range scans return confidence ranges, not exact values
- AI may misidentify ship classes
- Old intel decays over time
- Knowledge progression: early "Likely mining vessel" → late "Tri-beam extractor, low-grade shield"

### 8.2 Captain Backgrounds

- Lightweight data-driven origins: former miner, ex-military, corporate defector, salvager-raised
- Affects preferred tactics, chatter tone, morale thresholds

### 8.3 Fleet Norms (Emergent)

- Fleets develop habits: "We don't retreat", "We always salvage", "We avoid that faction's space"
- Breaking norms causes tension; following them builds loyalty
- Communicated through chatter, not UI

### 8.4 Persistent Space Scars & Unofficial Landmarks

- Old wreck fields, burnt-out stations, failed colonies
- AI references them: "That's where the convoy disappeared last cycle."
- Players and AI name places organically; names propagate via chatter and saves

### 8.5 Operational Wear

- Long deployments increase fuel inefficiency, repair time, crew stress
- Encourages rotations, downtime, returning "home"
- Field fixes: faster/cheaper but leave hidden penalties; proper dock repairs clear all

### 8.6 Behavioral Reputation

- AI tracks player behavior: abandon wrecks? rescue disabled ships? overcommit allies? hoard resources?
- Changes dialogue tone, recruitment willingness, AI response to risky orders

### 8.7 Sector Tension & Slow-Burn Mysteries

- Per-system: resource stress, pirate pressure, industrial output, faction influence
- Causes migration, wars, collapse, opportunity over time
- Anomalies that take dozens of hours to resolve (strange warp echoes, vanishing ships)

### 8.8 Soft Failure States

- No "game over" — fleets fracture, influence wanes, systems turn hostile, allies leave
- Recovery becomes a chapter, not a reload

### 8.9 Post-Event Analysis

- After major events: fleet discusses what went wrong
- Different captains blame different causes
- Replaces tutorial popups with organic learning

### 8.10 Data as Gameplay

- Export combat logs, view engagement summaries, study economic graphs
- Power-user feature for the EVE audience

---

## 9. Atlas Engine Naming Discipline

All new systems from this extraction must follow:

| Scope | Prefix | Example |
|-------|--------|---------|
| Engine-wide | `Atlas` | `AtlasDeterministicScheduler` |
| Simulation | `Sim` | `SimFleetMoraleComponent` |
| Rendering | `AtlasRender` | `AtlasRenderTacticalOverlay` |
| UI | `UI_` | `UI_TacticalOverlayRing` |
| Game-specific | `EVO_` | `EVO_PirateCoalitionDoctrine` |

**Forbidden**: Generic names (`Manager`, `Controller`, `Handler`), engine-agnostic names (`InventorySystem`).

---

*This document is the authoritative extraction from the brainstorming archive.*
*For implementation scheduling, see [ROADMAP.md](../ROADMAP.md).*
