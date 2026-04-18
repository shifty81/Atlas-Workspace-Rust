# Story + Game Systems Audit

## What is working
The story now has a real progression spine:
- buried bunker start
- blocked shaft to the surface
- sand and debris excavation
- centrifuge-driven material recovery
- freight elevator restoration
- backpack-first R.I.G. growth
- helmet-gated HUD
- palm port-gated advanced interaction
- embedded AI and drones later
- surface unlock into broader survival and expansion

That is a coherent early-to-mid-game onboarding path.

## What is locked
- 3D FPS survival sandbox
- modular ship and base construction
- voxel-backed structure, mining, damage, and PCG
- low-poly presentation layer
- R.I.G. as the main player progression platform
- editor/runtime pipeline parity

## Major unresolved gaps
### Contracts
- module schema
- material schema
- attachment/slot schema
- recipe schema
- interface target schema
- injury and scan data schemas

### Simulation
- power generation, storage, routing, and failure
- oxygen consumption and refill rules
- sand collapse and support propagation
- environmental exposure rules
- healing consumption and limits

### State machines
- freight elevator repair and activation
- interface port handshake and link
- breach minigame lifecycle
- AI activation and feature degradation
- drone task execution
- healing supervision

## Highest-value next specs
1. Centrifuge + Resource Conversion Spec v1
2. R.I.G. System Contract v1
3. Attachment Graph + Slot Dependency Spec v1
4. Interface Port + Link System Spec v1
5. R.I.G. AI Core Spec v1
6. Power + Resource Graph Spec v1
7. Voxel Material + Collapse Rules Spec v1

## R.I.G. conclusions
The R.I.G. is now central enough that almost every advanced interaction depends on it:
- backpack is root authority
- helmet is required for HUD
- backpack tiers unlock slots
- attachments unlock more attachments
- right palm interface port gates advanced systems
- AI lives in backpack core
- drones and later healing are service layers on top

This is a good architecture. It now needs strict contracts.
