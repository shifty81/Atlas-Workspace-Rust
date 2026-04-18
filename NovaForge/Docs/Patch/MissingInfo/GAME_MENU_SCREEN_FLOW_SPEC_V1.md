# Game Menu + Screen Flow Spec v1

## Primary Screen Graph
- Title Screen
- Save/World Browser
- New World Setup
- Settings
- Credits
- In-Game Pause
- Inventory
- Equipment
- Crafting
- Map
- Journal / Missions
- Codex / Knowledge
- Vehicle Interface
- Ship Interface
- Hangar Management
- Service Screens

## Open Rules
- pause-independent overlays: notifications, comms, warnings
- pause-blocking screens: settings, save/load, title-return confirmation
- gameplay overlays: inventory, equipment, map, journal, crafting
- docked-location screens: hangar management, ship service, vehicle service

## Input Rules
- keyboard/controller routing must respect current top modal screen
- ESC or equivalent backs out one layer unless in critical confirmation dialog
- quick access bindings are gated by unlock state and current context

## Canonical In-Game Quick Menu Set
- Inventory
- Equipment
- Crafting
- Map
- Journal
- Rig Terminal
- Squad/Fleet if unlocked
- Build/Repair when relevant

## Modal Priorities
1. hard-failure dialog
2. confirmation dialog
3. service screen modal
4. standard gameplay screen
5. background HUD
