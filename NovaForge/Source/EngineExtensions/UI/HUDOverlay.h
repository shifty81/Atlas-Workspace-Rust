#pragma once
#include <cstdint>
#include <string>
#include <vector>

namespace atlas::ui {

enum class WarningLevel : uint8_t {
    None,
    Info,
    Warning,
    Critical
};

struct HUDWarning {
    std::string message;
    WarningLevel level = WarningLevel::None;
    uint32_t tick = 0;
    double timestamp = 0.0;
};

struct TimeControlState {
    bool paused = false;
    double speed = 1.0;
    uint32_t targetTick = 0;
};

class HUDOverlay {
public:
    void SetCurrentTick(uint32_t tick);
    uint32_t GetCurrentTick() const;

    void SetSimulationTime(double time);
    double GetSimulationTime() const;

    void SetTickRate(uint32_t hz);
    uint32_t GetTickRate() const;

    // Warning system
    void AddWarning(const std::string& message, WarningLevel level, uint32_t tick);
    void ClearWarnings();
    const std::vector<HUDWarning>& ActiveWarnings() const;
    size_t WarningCount() const;
    bool HasCriticalWarning() const;
    void SetMaxWarnings(size_t max);

    // Time controls
    void SetPaused(bool paused);
    bool IsPaused() const;
    void SetSpeed(double speed);
    double GetSpeed() const;
    void StepForward();
    void StepBackward();
    const TimeControlState& GetTimeControl() const;

    // WorldTime dilation controls
    void SetWorldDilation(double dilation);
    double GetWorldDilation() const;
    void SetWorldPaused(bool paused);
    bool IsWorldPaused() const;

    // Determinism display
    void SetStateHash(uint64_t hash);
    uint64_t GetStateHash() const;
    void SetDeterminismStatus(bool ok);
    bool IsDeterminismOk() const;

    // Visibility
    void SetVisible(bool visible);
    bool IsVisible() const;

private:
    uint32_t m_currentTick = 0;
    double m_simulationTime = 0.0;
    uint32_t m_tickRate = 30;
    std::vector<HUDWarning> m_warnings;
    size_t m_maxWarnings = 50;
    TimeControlState m_timeControl;
    uint64_t m_stateHash = 0;
    bool m_determinismOk = true;
    bool m_visible = true;
    double m_worldDilation = 1.0;
    bool m_worldPaused = false;
};

} // namespace atlas::ui
