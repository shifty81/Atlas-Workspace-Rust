# Interface Port Specification

## Overview
The interface port is a right-palm hardware module that enables all
linked and secured interactions. Without it, the player cannot control
vehicles, access terminals, or override locked systems.

## Requirement
- Arm module slot available (Tier 2+ R.I.G.)
- R.I.G. Tier >= 2 for basic port
- R.I.G. Tier >= 3 for advanced port (secured interactions)

## States
```
IDLE → CONTACT → LINKING → LINKED → CONTROL
                    ↓
                LINK_FAILED → IDLE
```

| State | Description | Transition |
|-------|-------------|-----------|
| IDLE | Port installed, not in use | Approach target → CONTACT |
| CONTACT | Near valid target | Press interact → LINKING |
| LINKING | Authentication handshake | Success → LINKED, Fail → LINK_FAILED |
| LINKED | Data connection active | Enter control → CONTROL |
| CONTROL | Full control of target | Release → LINKED, Disconnect → IDLE |
| LINK_FAILED | Auth rejected | Retry or breach → CONTACT |

## Interaction Targets
| Target | Auth | Breach Possible |
|--------|------|----------------|
| Player terminal | None | No |
| Station terminal | Basic | Yes |
| Vehicle port | Basic | Yes |
| Elevator control | Basic | No |
| Enemy terminal | Secured | Yes |
| Enemy ship | Secured | Yes |
| Ancient tech | Advanced | No (requires scan) |

## Data Flow
```
Interface Port → Link → Target System → Data → Helmet HUD
```

## C++ Contract
```cpp
enum class LinkState : uint8_t {
    Idle, Contact, Linking, Linked, Control, LinkFailed
};

class InterfacePort {
    void beginContact(const std::string& targetId);
    void attemptLink();
    void enterControl();
    void disconnect();
    LinkState state() const;
    const std::string& currentTarget() const;
    float linkQuality() const;    // 0.0 to 1.0
    bool isLinked() const;
    bool hasControl() const;
};
```

## Implementation Status
**NOT IMPLEMENTED** — see `Source/Game/include/NF/Game/Game.h` for integration point.

## Related Existing Code
- `RigLinkSystem` — links R.I.G. to ship port (stat bonuses)
- `RigLinkState` — component in fps_components.h
- These provide the *ship-link* variant but not the general-purpose interface port
