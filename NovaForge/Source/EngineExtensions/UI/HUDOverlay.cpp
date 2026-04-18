#include "HUDOverlay.h"

namespace atlas::ui {

void HUDOverlay::SetCurrentTick(uint32_t tick) {
    m_currentTick = tick;
}

uint32_t HUDOverlay::GetCurrentTick() const {
    return m_currentTick;
}

void HUDOverlay::SetSimulationTime(double time) {
    m_simulationTime = time;
}

double HUDOverlay::GetSimulationTime() const {
    return m_simulationTime;
}

void HUDOverlay::SetTickRate(uint32_t hz) {
    m_tickRate = hz;
}

uint32_t HUDOverlay::GetTickRate() const {
    return m_tickRate;
}

void HUDOverlay::AddWarning(const std::string& message, WarningLevel level, uint32_t tick) {
    if (m_warnings.size() >= m_maxWarnings) {
        m_warnings.erase(m_warnings.begin());
    }

    HUDWarning warning;
    warning.message = message;
    warning.level = level;
    warning.tick = tick;
    warning.timestamp = m_simulationTime;
    m_warnings.push_back(warning);
}

void HUDOverlay::ClearWarnings() {
    m_warnings.clear();
}

const std::vector<HUDWarning>& HUDOverlay::ActiveWarnings() const {
    return m_warnings;
}

size_t HUDOverlay::WarningCount() const {
    return m_warnings.size();
}

bool HUDOverlay::HasCriticalWarning() const {
    for (const auto& w : m_warnings) {
        if (w.level == WarningLevel::Critical) {
            return true;
        }
    }
    return false;
}

void HUDOverlay::SetMaxWarnings(size_t max) {
    m_maxWarnings = max;
    while (m_warnings.size() > m_maxWarnings) {
        m_warnings.erase(m_warnings.begin());
    }
}

void HUDOverlay::SetPaused(bool paused) {
    m_timeControl.paused = paused;
}

bool HUDOverlay::IsPaused() const {
    return m_timeControl.paused;
}

void HUDOverlay::SetSpeed(double speed) {
    if (speed < 0.0) speed = 0.0;
    m_timeControl.speed = speed;
}

double HUDOverlay::GetSpeed() const {
    return m_timeControl.speed;
}

void HUDOverlay::StepForward() {
    m_timeControl.paused = true;
    m_timeControl.targetTick = m_currentTick + 1;
}

void HUDOverlay::StepBackward() {
    m_timeControl.paused = true;
    if (m_currentTick > 0) {
        m_timeControl.targetTick = m_currentTick - 1;
    } else {
        m_timeControl.targetTick = 0;
    }
}

const TimeControlState& HUDOverlay::GetTimeControl() const {
    return m_timeControl;
}

void HUDOverlay::SetStateHash(uint64_t hash) {
    m_stateHash = hash;
}

uint64_t HUDOverlay::GetStateHash() const {
    return m_stateHash;
}

void HUDOverlay::SetDeterminismStatus(bool ok) {
    m_determinismOk = ok;
}

bool HUDOverlay::IsDeterminismOk() const {
    return m_determinismOk;
}

void HUDOverlay::SetVisible(bool visible) {
    m_visible = visible;
}

bool HUDOverlay::IsVisible() const {
    return m_visible;
}

void HUDOverlay::SetWorldDilation(double dilation) {
    if (dilation < 0.0) dilation = 0.0;
    m_worldDilation = dilation;
}

double HUDOverlay::GetWorldDilation() const {
    return m_worldDilation;
}

void HUDOverlay::SetWorldPaused(bool paused) {
    m_worldPaused = paused;
}

bool HUDOverlay::IsWorldPaused() const {
    return m_worldPaused;
}

} // namespace atlas::ui
