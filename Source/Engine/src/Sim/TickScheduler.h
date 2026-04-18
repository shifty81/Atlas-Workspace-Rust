#pragma once
#include <cstdint>
#include <chrono>
#include <functional>
#include "../core/contract/SimulationGuard.h"

namespace atlas::sim {

class TickScheduler {
public:
    void SetTickRate(uint32_t hz);
    uint32_t TickRate() const;

    float FixedDeltaTime() const;

    void Tick(const std::function<void(float)>& callback);
    uint64_t CurrentTick() const;

    // Frame pacing control
    void SetFramePacing(bool enabled);
    bool FramePacingEnabled() const;

    /// Lock the tick rate so it cannot be changed after initialization.
    void LockTickRate();
    bool IsTickRateLocked() const;

private:
    uint32_t m_tickRate = 30;
    uint64_t m_currentTick = 0;
    bool m_framePacing = true;
    bool m_tickRateLocked = false;
    std::chrono::steady_clock::time_point m_nextTick{};
    bool m_firstTick = true;
};

}
