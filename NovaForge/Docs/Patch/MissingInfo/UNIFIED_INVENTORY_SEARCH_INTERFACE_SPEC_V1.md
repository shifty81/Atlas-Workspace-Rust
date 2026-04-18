# Unified Inventory / Search Interface Spec v1

## Goal
One searchable interface for all discovered and accessible storage domains.

## Core Features
- global search by name, tag, material class, quality, source, distance
- filter by reachable network tier
- show quantity, location, reservation state, and route source
- jump from search result to owning screen or logistics action
- support player inventory, crafting pulls, service pulls, and automation reservations

## Result Fields
- item id
- display name
- quantity available
- reserved quantity
- storage node
- distance / access tier
- transfer mode
- allowed actions

## Actions
- move
- reserve
- craft-from
- service-from
- route-to
- inspect source
