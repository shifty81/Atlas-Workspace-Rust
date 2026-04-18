# Storage Schema + Access Topology Spec v1

## Purpose
Define how dense compressed storage containers, local inventories, base inventories, vehicles, ships, and remote logistics expose a single searchable access model.

## Storage Node Types
- personal rig inventory
- local container
- compressed bulk material tank/container
- station/base storage
- vehicle cargo
- ship cargo
- remote logistics hub
- orbital logistics node
- inter-system relay node

## Access Tiers
1. local only
2. structure-wide
3. base-wide
4. regional/planetary
5. orbital
6. current solar system

## Canonical Record
```json
{
  "node_id": "storage.base.alpha.bulk01",
  "node_type": "bulk_container",
  "access_tier": 3,
  "network_id": "alpha_base",
  "supported_classes": ["ore", "ingot", "gas", "biomass"],
  "capacity_units": 500000,
  "current_units": 182400,
  "power_required": true,
  "search_visible": true
}
```
