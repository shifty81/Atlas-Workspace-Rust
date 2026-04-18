# Project Chat Summary

## Workspace direction
Atlas Workspace is the shell, tooling environment, and build/release environment. It should expose major functions as app-like tools through a unified Atlas shell and Broker AI flow. AtlasUI is the standard shared UI framework and Win32 is the baseline platform target.

## UI direction
- Native C++ editor path is the active direction.
- WPF shell work is deferred.
- AtlasUI should standardize dock tabs, panel chrome, buttons, hover/pressed states, rounded corners, tooltips, and theming.
- Game development tasks are intentionally pushed behind framework/tooling standardization in roadmap priority.

## Repo direction
- NovaForge is the main hosted project developed inside Workspace.
- It stays logically detachable and should not be built by default as part of workspace builds.
- Workspace should support GitHub integration and project loading.
- Zip drops in root should be audited automatically.
- Loose txt/md files in root should be archived into the archive system.

## Core game direction
- Voxel truth layer. Low-poly visual wrap.
- 3D FPS first. Fleet gameplay later.
- R.I.G. is the central player progression system.
- Surface escape loop starts underground and teaches survival, excavation, processing, repair, and power.

## R.I.G. direction
- Starter state is backpack + deployable helmet only.
- Backpack is root authority for slots, power, storage, data routing, and HUD capability.
- Attachments unlock further attachments.
- Equipment menu unlocks later through backpack progression.
- Palm interface port gates piloting, terminals, station services, and hacking.
- Embedded AI handles alerts, scanning, mapping, diagnostics, drone support, and later nano-healing.

## Systems repeatedly identified as missing or needing hard contracts
- module system contract
- resource conversion contract
- power and resource graph
- voxel material and collapse rules
- interface link state machine
- AI event and feature contract
- skills and research progression
- environmental survival rules
- construction and repair workflow
- vehicle and station interface contract

## Roadmap implication
Front-load:
- AtlasUI
- workspace shell standards
- command/shortcut system
- theme/token system
- validation and enforcement
- repo drift prevention

Defer unless selected:
- broader game content expansion
- complex fleet systems
- late-game mechanics
