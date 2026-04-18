# EVE-Offline — PVE-Only Space MMO Simulation

## 1. Project Vision

EVE-Offline is a single-player / cooperative PVE space simulation inspired by EVE Online, rebuilt from the ground up in C++ with OpenGL, using the Atlas Engine as its core framework.

The goal is to preserve EVE's depth, clarity, and systemic interactions while:

- Removing PVP entirely
- Allowing offline play
- Supporting peer-to-peer hosting
- Optionally running a dedicated server (up to ~75 players)
- Making AI the primary driver of conflict, economy, and movement

This is a systems-first simulation, not an arcade space shooter.

## 2. Hard Non-Goals (Locked)

- ❌ No PVP (ever)
- ❌ No twitch / FPS-style aiming
- ❌ No cinematic camera shake or "immersion blur"
- ❌ No handcrafted missions as primary content
- ❌ No ImGui in final UI

## 3. Core Gameplay Pillars

**Spatial Decision-Making**
Positioning, range, velocity, transversal, alignment

**Information Density**
Overview, brackets, ranges, UI clarity over spectacle

**Systemic AI**
AI ships obey the exact same rules as players

**Economy as Cause, Not Reward**
Mining, hauling, production, destruction feed each other

**Persistence**
Everything leaves traces: wrecks, shortages, responses

## 4. Camera & Player Interaction

- Third-person, orbital camera
- Mouse-driven: click in space to move, click object to interact
- No WASD flight
- Ship motion is vector-based, inertia-aware, governed by align time and max velocity
- Navigation modes: Align, Orbit, Keep at Range, Warp

## 5. Weapon & Combat Model (Fully Deterministic)

Core formula inputs: optimal range, falloff, tracking speed, target angular velocity, signature radius, velocity.

Damage layers: Shields → Armor → Hull, each with resist profiles and different visual feedback.

Weapon types: Beam-style (continuous), Projectile-style (volley), Missile-style (time-of-flight), Drone-style (AI sub-entities).

## 6. AI Philosophy & Architecture

AI is rule-following behavior, not "smart scripting."

- AI uses the same navigation, weapons, targeting, and cooldowns as players
- Decisions are probabilistic + role-based
- No cheating stats
- Difficulty comes from numbers, coordination, and role composition

## 7. Economy & Simulation Loops

Core actors: AI miners, AI haulers, AI pirates, AI industrial producers, players.

The economy exists even without the player. Resources are extracted, moved, produced, destroyed. Shortages emerge. AI reacts.

## 8. Pirate Factions & the Titan Rumor

At the outer edges of the galaxy, pirate factions behave differently. They are more coordinated, more secretive, more resource-hungry. The reason is never explicitly stated.

Evidence leaks through: warp anomalies, missing convoys, escalating protection, unusual fleet movements, economic distortion.

A pirate coalition may be attempting to assemble a Titan-class vessel. This threat is not a quest, not a UI bar, not guaranteed to succeed. It exists first as belief and fear, not fact.

## 9. Transitional States (Warp, Travel, Downtime)

Warp and long-distance travel are intentional gameplay states, not loading screens.

- No loss of control or information
- HUD remains present with optional soft edge treatment
- Fleet members warp alongside the player in formation
- AI chatter continues during warp, travel, mining, and combat
- Warp is a social and psychological space where fleets bond or fracture

## 10. Layered System Architecture

**Layer 1 — Core Simulation**: Economy, threat levels, NPC activity, logistics. Pure math, no lore.

**Layer 2 — Evidence & Anomalies**: Warp instability, resource irregularities, route changes, wreck density. Effects, not explanations.

**Layer 3 — Social Interpretation**: Fleet chatter, captain beliefs, morale, rumors. Meaning emerges socially.

**Layer 4 — Meta-Threat Systems (Late Game)**: Titan assembly pressure, pirate coalition doctrine, galactic response curves. Applies pressure downward, never narrates upward.

## 11. Technical Architecture

- C++20 (engine), C++17 (client/server)
- OpenGL rendering
- ECS model: components are data only, systems are deterministic
- Server authoritative, clients send intent not state
- Supports P2P and dedicated server (~75 players)

## 12. Design Non-Negotiables

- ✔ Accurate spatial systems
- ✔ Passive, truthful UI
- ✔ Deterministic simulation
- ✔ Slow, monumental progression
- ✔ Emergent narrative
- ❌ Clickable tactical overlays
- ❌ Fake distances
- ❌ Scripted reveals
- ❌ Power without cost
- ❌ Theme-park content

## 13. What This Project Is

A spacefaring civilization simulator where scale is earned, truth is visible, and the universe slowly reacts to forces it barely understands — including the player.
