# R.I.G. Mech Transformation Spec v1

## Purpose
Late-game progression path that expands the player's upgraded R.I.G. into a deployable mech platform.

## Design Rules
- mech deployment is an extension of the existing R.I.G., not a separate progression tree
- equipped armor/modules influence available mech states
- deployment requires a dedicated transformation-capable frame and power threshold
- nano-tech consolidation can merge multiple late-game modules into composite systems

## State Machine
`IdleRig -> ArmedForDeploy -> Transforming -> MechActive -> MechDamaged -> Retracting -> IdleRig`

## Preconditions
- minimum rig tier reached
- mech core installed
- power reserve above threshold
- deployment volume valid
- heavy damage or hard-disable can block transformation

## Slot Remap Rules
- backpack core -> mech spine/core
- arm modules -> mech arm hardpoints
- propulsion modules -> boost/jump systems
- shield modules -> mech barrier matrix
- drone/control modules -> support bay or shoulder mount

## Failure Rules
- partial deployment creates vulnerable transition state
- destroyed remapped module reduces mech subsystem capability rather than silently substituting

## Required Runtime Data
- transformation charge
- mech hull state
- mech heat
- subsystem remap table
- pilot exposure / emergency eject flags
