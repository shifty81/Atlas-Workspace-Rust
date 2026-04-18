# Atlas Engine — Networking

## Implementation Status

| Feature | Status | Notes |
|---------|--------|-------|
| NetContext API | ✅ Implemented | Mode selection, peer management, packet send/receive |
| Dedicated server loop | ✅ Implemented | Headless mode with tick processing |
| P2P support | ✅ Implemented | Host/peer roles with RTT tracking |
| Loopback mode | ✅ Implemented | Local testing without network |
| Lockstep sync | ✅ Implemented | ECS state serialized into snapshots |
| Rollback/replay | ✅ Implemented | ECS state restore + input frame replay |
| Replication rules | ✅ Implemented | Rule-based dirty tracking, delta collection/application |
| Production hardening | ✅ Implemented | Connection timeouts, reconnection, bandwidth throttling, heartbeat monitoring |

## Unified Networking Model

Atlas supports multiple networking modes through a single abstraction:

| Mode           | Description                    |
|----------------|--------------------------------|
| Standalone     | No networking                  |
| Client         | Client connecting to server    |
| Server         | Dedicated authoritative server |
| P2P_Host       | Peer-to-peer (host)            |
| P2P_Peer       | Peer-to-peer (peer)            |

## NetContext API

```cpp
enum class NetMode {
    Standalone,
    Client,
    Server,
    P2P_Host,
    P2P_Peer
};

struct NetPeer {
    uint32_t id;
    float rtt;
    bool connected;
};

class NetContext {
public:
    void Init(NetMode mode);
    void Shutdown();
    void Poll();
    void Send(uint32_t peerID, const Packet& pkt);
    void Broadcast(const Packet& pkt);
    void Flush();

    NetMode Mode() const;
    const std::vector<NetPeer>& Peers() const;
    bool IsAuthority() const;
};
```

## Lockstep + Rollback

The lockstep and rollback methods are implemented in `NetContext` using
ECS state serialization. `SaveSnapshot()` captures the full ECS state,
`RollbackTo()` restores it, and `ReplayFrom()` re-applies recorded
input frames.

Design:

- Fixed tick rate (e.g., 30 Hz)
- Inputs buffered by tick
- Graph VM deterministic
- ECS state snapshot per tick

### Rollback

```cpp
// Methods are implemented with ECS state serialization
void OnLateInput(InputFrame f) {
    if (f.tick < currentTick) {
        RollbackTo(f.tick);   // Restores ECS state from snapshot
        ReplayFrom(f.tick);   // Re-applies input frames from tick onward
    }
}
```

## Lockstep Frame Semantics

Atlas networking uses a lockstep input frame model:

| Concept | Detail |
|---------|--------|
| Frame unit | Fixed simulation tick (default 60 Hz) |
| Input frame | `InputFrame { tick, playerID, moveX, moveY }` |
| Authority | Server is authoritative for state |
| Prediction | Client predicts locally, server corrects |

### Input Transport

Each tick, clients send an `InputFrame` to the server. The server collects
all input frames for a tick before advancing simulation. This guarantees
that all machines process identical inputs in identical order.

### Rollback Window

The engine maintains a configurable snapshot history (default: 60 snapshots
at 30 Hz = ~2 seconds). When a desync is detected:

1. Server identifies the divergence tick via hash ladder comparison
2. Both sides roll back to the last known-good snapshot
3. Simulation replays forward with corrected inputs

### Save-Tick Broadcast

When the server performs an autosave, it broadcasts the save tick and state
hash to all clients via `NetContext::BroadcastSaveTick()`. Clients can
verify their local state hash matches the server's, catching desyncs at
save boundaries.

## Replication Rules

Atlas uses a rule-based replication system (`engine/net/Replication.h`)
to synchronize ECS component state between server and clients.

### ReplicationManager

```cpp
enum class ReplicateFrequency { EveryTick, OnChange, Manual };
enum class ReplicateDirection { ServerToClient, ClientToServer, Bidirectional };

struct ReplicationRule {
    uint32_t typeTag;
    std::string componentName;
    ReplicateFrequency frequency;  // default: OnChange
    ReplicateDirection direction;  // default: ServerToClient
    bool reliable;                 // default: true
    uint8_t priority;              // 0–255, default: 128
};
```

### Dirty Tracking

Components are marked dirty via `MarkDirty(typeTag, entityID)`. The
`OnChange` frequency mode only includes entities that have been marked
dirty since the last collection. `EveryTick` rules always include all
entities with that component.

### Delta Format

`CollectDelta(tick)` produces a compact binary payload:

```
[tick : 4 bytes]
[ruleCount : 4 bytes]
  for each rule:
    [typeTag : 4 bytes]
    [entityCount : 4 bytes]
      for each entity:
        [entityID : 4 bytes]
        [dataSize : 4 bytes]
        [componentData : dataSize bytes]
```

`ApplyDelta(data)` parses the payload and deserializes each component
back into the local ECS world.

### Data Flow

| Data     | Direction            |
|----------|----------------------|
| Input    | Client → Authority   |
| State    | Authority → Clients  |
| Events   | Authority → All      |
| Assets   | Editor → Disk → Runtime |

### Authoritative State
- ECS component state
- Simulation ticks
- Graph events

### Client-Only State
- Camera
- UI
- Prediction

## Production Hardening

The `NetHardening` class (`engine/net/NetHardening.h`) provides resilience
and monitoring on top of the core networking layer.

### Configuration

| Parameter | Default | Description |
|-----------|---------|-------------|
| `connectionTimeoutMs` | 10000 | Max time to establish connection |
| `maxReconnectAttempts` | 5 | Retry limit before giving up |
| `reconnectDelayMs` | 2000 | Delay between reconnection attempts |
| `maxBandwidthBytesPerSec` | 0 (unlimited) | Bandwidth throttle |
| `maxPacketSize` | 1400 | Maximum packet size in bytes |
| `heartbeatIntervalMs` | 1000 | Heartbeat send interval |
| `heartbeatMissThreshold` | 5 | Missed heartbeats before timeout |

### Connection State Machine

```
Disconnected → Connecting → Connected
                    ↓              ↓
              Reconnecting    TimedOut / Kicked
                    ↓
              Connecting (retry)
```

- `Connect()` transitions to Connecting and starts timeout timer
- `Update(deltaTimeMs)` drives the state machine each frame
- Receiving any packet while Connecting auto-transitions to Connected
- Heartbeat misses beyond threshold trigger timeout handling
- Reconnection attempts increment a counter; exceeding the limit
  transitions to TimedOut

### Telemetry

`ConnectionStats` tracks:
- Bytes sent / received
- Packets sent / received / dropped
- Average RTT (exponential moving average: 90% old + 10% new)
- Peak RTT
- Reconnection count
- Current bandwidth usage (resets per second)
