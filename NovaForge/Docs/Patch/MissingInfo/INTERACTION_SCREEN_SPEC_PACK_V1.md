# Interaction Screen Spec Pack v1

## Screen Families
- terminal screen
- crafting screen
- processing screen
- storage screen
- service screen
- docking / hangar screen
- research screen
- communications screen
- build / repair screen

## Shared Frame Contract
Every interaction screen should expose:
- header identity
- context breadcrumb
- primary status strip
- main workspace panel
- side detail panel
- action bar
- error/warning strip

## Shared Controller Inputs
- current actor
- source entity id
- distance / access validity
- required permissions or installed module checks
- power/network connectivity state
- data model snapshot

## Shared Output Events
- close
- commit action
- cancel action
- queue action
- open linked screen
- open codex/help

## Required First Implementations
1. centrifuge interaction screen
2. storage search screen
3. equipment screen
4. hangar management screen
5. vehicle service screen
6. ship service screen
