#pragma once
#include "StateHasher.h"
#include <cstdint>
#include <functional>
#include <string>
#include <vector>

namespace atlas::sim {

/// Abstract simulation interface for mirror-mode verification
class ISimulation {
public:
    virtual ~ISimulation() = default;

    /// Advance the simulation by one tick with the given input
    virtual void Step(const std::vector<uint8_t>& inputFrame) = 0;

    /// Compute a deterministic hash of the current world state
    virtual uint64_t WorldHash() const = 0;

    /// Current tick number
    virtual uint64_t CurrentTick() const = 0;
};

/// Records a desync detected by the mirror controller
struct MirrorDesyncEvent {
    uint64_t tick = 0;
    uint64_t serverHash = 0;
    uint64_t clientHash = 0;
};

/// Runs two simulations in lockstep and compares their state hashes
/// to verify determinism between server and client.
class SimMirrorController {
public:
    /// Set the two simulations to mirror
    void SetServer(ISimulation* server);
    void SetClient(ISimulation* client);

    /// Step both simulations with the same input and compare hashes
    /// Returns true if hashes match, false on desync
    bool Step(const std::vector<uint8_t>& inputFrame);

    /// Run N frames with the same input per frame
    /// Returns the tick of the first desync, or 0 if all match
    uint64_t RunFrames(const std::vector<std::vector<uint8_t>>& inputs);

    /// Whether a desync has been detected
    bool HasDesync() const;

    /// Get all desyncs detected so far
    const std::vector<MirrorDesyncEvent>& Desyncs() const;

    /// Get the first desync event (if any)
    const MirrorDesyncEvent* FirstDesync() const;

    /// Total frames stepped
    uint64_t FrameCount() const;

    /// Clear desync history
    void Reset();

    /// Callback when desync is detected
    using DesyncCallback = std::function<void(const MirrorDesyncEvent&)>;
    void SetDesyncCallback(DesyncCallback cb);

    /// Enable/disable mirror mode
    bool IsEnabled() const;
    void SetEnabled(bool enabled);

private:
    ISimulation* m_server = nullptr;
    ISimulation* m_client = nullptr;
    std::vector<MirrorDesyncEvent> m_desyncs;
    uint64_t m_frameCount = 0;
    bool m_enabled = true;
    DesyncCallback m_desyncCallback;
};

}  // namespace atlas::sim
