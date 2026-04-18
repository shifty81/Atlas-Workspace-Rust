#include "TickStepDebugger.h"

namespace atlas::sim {

void TickStepDebugger::SetHasher(const StateHasher* hasher) {
    m_hasher = hasher;
}

uint64_t TickStepDebugger::CurrentTick() const {
    return m_currentTick;
}

void TickStepDebugger::SetCurrentTick(uint64_t tick) {
    m_currentTick = tick;
}

void TickStepDebugger::StepForward(uint64_t count) {
    m_currentTick += count;
}

void TickStepDebugger::StepBackward(uint64_t count) {
    if (count > m_currentTick) {
        m_currentTick = 0;
    } else {
        m_currentTick -= count;
    }
}

void TickStepDebugger::JumpToTick(uint64_t tick) {
    m_currentTick = tick;
}

uint32_t TickStepDebugger::AddBreakpoint(const TickBreakpoint& bp) {
    TickBreakpoint b = bp;
    b.id = m_nextBreakpointId++;
    m_breakpoints.push_back(b);
    return b.id;
}

bool TickStepDebugger::RemoveBreakpoint(uint32_t bpId) {
    for (auto it = m_breakpoints.begin(); it != m_breakpoints.end(); ++it) {
        if (it->id == bpId) {
            m_breakpoints.erase(it);
            return true;
        }
    }
    return false;
}

bool TickStepDebugger::EnableBreakpoint(uint32_t bpId, bool enable) {
    for (auto& bp : m_breakpoints) {
        if (bp.id == bpId) {
            bp.enabled = enable;
            return true;
        }
    }
    return false;
}

const std::vector<TickBreakpoint>& TickStepDebugger::Breakpoints() const {
    return m_breakpoints;
}

uint32_t TickStepDebugger::BreakpointCount() const {
    return static_cast<uint32_t>(m_breakpoints.size());
}

void TickStepDebugger::ClearBreakpoints() {
    m_breakpoints.clear();
}

bool TickStepDebugger::CheckBreakpoints() const {
    m_triggeredBp = 0;
    for (const auto& bp : m_breakpoints) {
        if (!bp.enabled) continue;

        bool triggered = false;

        // Check tick match
        if (bp.tick != 0 && bp.tick == m_currentTick) {
            triggered = true;
        }

        // Check hash mismatch
        if (bp.hashMismatch != 0 && m_hasher && m_hasher->CurrentHash() != bp.hashMismatch) {
            triggered = true;
        }

        if (triggered) {
            m_triggeredBp = bp.id;
            if (m_breakpointCallback) {
                m_breakpointCallback(bp.id, m_currentTick);
            }
            return true;
        }
    }
    return false;
}

uint32_t TickStepDebugger::TriggeredBreakpointId() const {
    return m_triggeredBp;
}

uint32_t TickStepDebugger::AddHashMismatchBreakpoint(uint64_t expectedHash, const std::string& label) {
    TickBreakpoint bp;
    bp.hashMismatch = expectedHash;
    bp.label = label.empty() ? "hash_mismatch" : label;
    return AddBreakpoint(bp);
}

bool TickStepDebugger::IsPaused() const {
    return m_paused;
}

void TickStepDebugger::SetPaused(bool paused) {
    m_paused = paused;
}

void TickStepDebugger::SetBreakpointCallback(BreakpointCallback cb) {
    m_breakpointCallback = std::move(cb);
}

}  // namespace atlas::sim
