# Launch-to-Play Flow Spec v1

## Goal
Define the canonical player journey from executable launch to active gameplay.

## Scope
- title boot
- profile/session init
- world selection or save selection
- pre-spawn validation
- spawn sequence
- first-control handoff
- pause/return loops

## Flow
1. **Executable Launch**
   - initialize platform systems
   - load config, bindings, graphics preset, accessibility profile
   - validate content manifests and required save metadata
2. **Studio / Project Identity Splash**
   - skippable after first successful boot
3. **Title Screen**
   - Continue
   - Load World
   - New World
   - Settings
   - Mods
   - Credits
   - Quit
4. **World Entry Decision**
   - Continue: open last valid save
   - Load World: open world/save browser
   - New World: open seed/start-setup flow
5. **Pre-Spawn Pipeline**
   - validate save version
   - validate required content packs
   - resolve player loadout state
   - resolve R.I.G. baseline state
   - stream initial zone
6. **Spawn Intro State**
   - freeze high-risk input
   - play spawn animation or wake-up sequence
   - deploy minimal HUD only
   - inject onboarding prompts based on current progression tier
7. **Control Handoff**
   - enable movement
   - enable look input
   - enable contextual interaction prompt system
   - enable objective feed if unlocked
8. **Active Gameplay**
   - FPS mode is primary
   - third-person is optional when supported by current vehicle or camera unlock rules
9. **Pause Layer**
   - Resume
   - Save
   - Load
   - Settings
   - Return to Title
   - Quit to Desktop

## First-Time New World Flow
- difficulty / rules preset
- seed entry or random seed
- starting biome/site
- optional tutorial flag
- confirm character baseline
- initialize world and spawn intro

## Failure States
- corrupted save → recovery prompt + backup options
- missing content pack → disable load with clear dependency message
- incompatible version → migration prompt if supported
- failed streaming/bootstrap → return to title with logged error ID

## Implementation Notes
- the flow must be data-driven where possible
- UI transitions must be AtlasUI-compliant for tooling and mirrored game GUI standards
- gameplay handoff must allow modular future hooks for cutscenes, intro events, and landing/hangar starts
