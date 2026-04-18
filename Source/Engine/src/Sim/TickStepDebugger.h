#pragma once
#include "StateHasher.h"
#include <cstdint>
#include <vector>
#include <functional>
#include <string>

namespace atlas::sim {

/// Breakpoint that triggers when a condition is met during stepping
struct TickBreakpoint {
    uint32_t id = 0;
    uint64_t tick = 0;          // Break at specific tick (0 = any)
    uint64_t hashMismatch = 0;  // Break when hash doesn't match this value (0 = disabled)
    bool enabled = true;
    std::string label;
};

/// Manages tick-step debugging: step forward/backward, breakpoints, state inspection
class TickStepDebugger {
public:
    /// Set the state hasher for hash-based breakpoints
    void SetHasher(const StateHasher* hasher);

    /// Current tick position
    uint64_t CurrentTick() const;
    void SetCurrentTick(uint64_t tick);

    /// Step forward by N ticks (default 1)
    void StepForward(uint64_t count = 1);

    /// Step backward by N ticks (default 1, cannot go below 0)
    void StepBackward(uint64_t count = 1);

    /// Jump to a specific tick
    void JumpToTick(uint64_t tick);

    /// Breakpoint management
    uint32_t AddBreakpoint(const TickBreakpoint& bp);
    bool RemoveBreakpoint(uint32_t bpId);
    bool EnableBreakpoint(uint32_t bpId, bool enable);
    const std::vector<TickBreakpoint>& Breakpoints() const;
    uint32_t BreakpointCount() const;
    void ClearBreakpoints();

    /// Check if current tick hits any breakpoint
    bool CheckBreakpoints() const;

    /// Get the ID of the triggered breakpoint (0 if none)
    uint32_t TriggeredBreakpointId() const;

    /// Add a hash-mismatch breakpoint that fires when the state hash
    /// at any tick doesn't match the expected hash
    uint32_t AddHashMismatchBreakpoint(uint64_t expectedHash, const std::string& label = "");

    /// Paused state
    bool IsPaused() const;
    void SetPaused(bool paused);

    /// Callback when a breakpoint is hit
    using BreakpointCallback = std::function<void(uint32_t breakpointId, uint64_t tick)>;
    void SetBreakpointCallback(BreakpointCallback cb);

private:
    uint64_t m_currentTick = 0;
    bool m_paused = false;
    const StateHasher* m_hasher = nullptr;
    std::vector<TickBreakpoint> m_breakpoints;
    uint32_t m_nextBreakpointId = 1;
    mutable uint32_t m_triggeredBp = 0;
    BreakpointCallback m_breakpointCallback;
};

}  // namespace atlas::sim
