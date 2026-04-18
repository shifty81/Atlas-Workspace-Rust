#pragma once
#include "ITool.h"
#include <cstdint>

namespace atlas::tools {

/// In-client overlay tool for controlling simulation stepping.
///
/// Provides frame-step, pause, resume, and speed control from within
/// the running client — without switching to the full editor.
class SimulationStepTool : public ITool {
public:
    std::string Name() const override { return "Simulation Step"; }
    void Activate() override;
    void Deactivate() override;
    void Update(float deltaTime) override;
    bool IsActive() const override { return m_active; }

    /// Request a single simulation step (takes effect on next Update).
    void RequestStep();

    /// Pause the simulation overlay (tool stays active but doesn't step).
    void SetPaused(bool paused);
    bool IsPaused() const { return m_paused; }

    /// Set simulation speed multiplier (1.0 = normal).
    void SetSpeedMultiplier(float mult);
    float SpeedMultiplier() const { return m_speedMultiplier; }

    uint64_t StepCount() const { return m_stepCount; }
    float AccumulatedTime() const { return m_accumulatedTime; }

private:
    bool m_active = false;
    bool m_paused = false;
    bool m_stepRequested = false;
    float m_speedMultiplier = 1.0f;
    uint64_t m_stepCount = 0;
    float m_accumulatedTime = 0.0f;
};

} // namespace atlas::tools
