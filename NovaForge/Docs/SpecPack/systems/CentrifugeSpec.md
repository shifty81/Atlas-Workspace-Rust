# Centrifuge Specification

## Overview
The centrifuge is the critical resource processing gate. No raw material
can be used directly — all must pass through centrifuge processing.

## Design Rules
1. One input slot, one output slot
2. Processing takes real time (not instant)
3. Processing consumes power continuously
4. Queue up to 8 jobs
5. Power loss pauses processing (does not destroy)
6. Tier upgrades reduce processing time

## Centrifuge Tiers

| Tier | Speed Multiplier | Queue Size | Power Draw |
|------|-----------------|------------|------------|
| 1 (Basic) | 1.0× | 4 | 3 kW |
| 2 (Improved) | 1.5× | 6 | 5 kW |
| 3 (Advanced) | 2.5× | 8 | 8 kW |

## State Machine
```
IDLE → LOADING → PROCESSING → COMPLETE → IDLE
                    ↓
              POWER_STALL → (power restored) → PROCESSING
```

## C++ Contract
```cpp
enum class CentrifugeState : uint8_t {
    Idle, Loading, Processing, Complete, PowerStall
};

struct CentrifugeJob {
    std::string inputMaterial;
    int inputQuantity;
    std::string outputMaterial;
    int outputQuantity;
    float processingTime;   // seconds
    float elapsed;          // progress
    float powerRequired;    // kW
};

class CentrifugeSystem {
    void addJob(CentrifugeJob job);
    void tick(float dt, float availablePower);
    CentrifugeState state() const;
    float progress() const;       // 0.0 to 1.0
    size_t queueSize() const;
    size_t maxQueueSize() const;
    const CentrifugeJob* currentJob() const;
};
```

## Dependency
- Requires: Power system active
- Requires: Raw materials in inventory
- Unlocks: All processed materials
- Unlocks: Construction system
- Unlocks: R.I.G. module upgrades

## Implementation Status
**NOT IMPLEMENTED** — this is a Tier 1 blocker.
