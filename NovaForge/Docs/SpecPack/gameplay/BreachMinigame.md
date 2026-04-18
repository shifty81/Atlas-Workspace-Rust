# Breach Minigame Specification

## Overview
Breach is a snake-style grid minigame used to override locked or secured systems.
It is the gameplay mechanic for hacking and forced access.

## Trigger Conditions
- Player has interface port linked to secured target
- Target requires override (locked terminal, enemy ship, etc.)
- Breach difficulty set by target type

## Grid Rules
- Grid size: 6×6 (basic), 8×8 (advanced), 10×10 (expert)
- Player controls a trace line from entry point
- Must reach exit point while collecting data nodes
- Avoid ICE (defense) nodes
- Time limit based on difficulty
- Power consumption during breach

## Difficulty Scaling

| Target Type | Grid | Time (s) | ICE Nodes | Data Nodes |
|-------------|------|----------|-----------|------------|
| Station terminal | 6×6 | 30 | 3 | 2 |
| Vehicle port | 6×6 | 25 | 4 | 2 |
| Enemy terminal | 8×8 | 45 | 8 | 4 |
| Enemy ship | 10×10 | 60 | 12 | 6 |
| Ancient tech | 8×8 | 90 | 6 | 8 |

## Outcomes
- **Success**: System unlocked, data nodes grant bonus intel
- **Failure**: Lockout timer (30s), security alert on target
- **Partial**: Some data nodes collected, system partially accessible

## State Machine
```
INACTIVE → INITIATING → ACTIVE → {SUCCESS | FAILURE | PARTIAL} → COOLDOWN → INACTIVE
```

## C++ Contract
```cpp
enum class BreachState : uint8_t {
    Inactive, Initiating, Active, Success, Failure, Partial, Cooldown
};

struct BreachGrid {
    int width, height;
    float timeLimit;
    int iceNodeCount;
    int dataNodeCount;
};

class BreachMinigame {
    void initiate(const BreachGrid& grid);
    void tick(float dt);
    void moveTrace(int dx, int dy);
    BreachState state() const;
    float timeRemaining() const;
    int dataCollected() const;
};
```

## Missing
- Grid generation algorithm
- ICE node placement rules
- Visual representation (AtlasUI panel)
- Sound design hooks
- Skill modifiers (reduce time, reveal ICE)
