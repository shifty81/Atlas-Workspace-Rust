# Game HUD Contract v1

## Goal
Define every HUD layer, unlock rule, state source, and visibility condition.

## HUD Layer Stack
1. reticle and hit marker layer
2. core vitals layer
3. contextual interaction layer
4. objective and mission layer
5. compass / navigation / waypoint layer
6. notification layer
7. comms / dialogue layer
8. vehicle overlay layer
9. ship / station / hangar overlay layer
10. debug / developer overlay layer

## Core Rules
- HUD begins in minimal mode at starter R.I.G. state
- HUD elements unlock through R.I.G. modules and progression
- player may hide or reposition non-critical widgets where allowed
- critical survival warnings override minimal mode

## Minimal HUD at Start
- health
- oxygen
- suit power
- damage direction indicator
- interaction prompt when target valid

## Expanded HUD Unlocks
| Unlock | Adds |
|---|---|
| scanner | scan readout, material tag, target info |
| wrist terminal | objective feed, map shortcuts, quick panels |
| helmet camera overlay | environment warnings, camera telemetry |
| neural link | fleet panel, command markers |
| vehicle interface port | vehicle speed, power, hull, module alerts |
| ship interface port | ship attitude, reactor, nav, dock state |

## Required Data Contracts
- `RigHUDState`
- `WorldInteractionPrompt`
- `ObjectiveFeedState`
- `NavigationOverlayState`
- `VehicleOverlayState`
- `ShipOverlayState`
- `NotificationFeedState`

## Visual Behavior
- compact and readable in FPS
- low-poly / stylized visual language
- clear state colors via shared token system
- no clutter at tier 0

## Failure and Safety Behavior
- if an unlocked subsystem goes offline, related HUD panel enters degraded state instead of vanishing instantly
- critical warnings can pin on screen even if user minimized the corresponding module view
