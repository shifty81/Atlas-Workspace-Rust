#include "EnvironmentControlTool.h"
#include <algorithm>

namespace atlas::tools {

void EnvironmentControlTool::Activate() {
    m_active = true;
    m_elapsed = 0.0f;
}

void EnvironmentControlTool::Deactivate() {
    m_active = false;
}

void EnvironmentControlTool::Update(float deltaTime) {
    if (!m_active) return;
    m_elapsed += deltaTime;
}

void EnvironmentControlTool::SetGravity(float x, float y, float z) {
    m_gravityX = x; m_gravityY = y; m_gravityZ = z;
}

void EnvironmentControlTool::SetWind(float x, float y, float z) {
    m_windX = x; m_windY = y; m_windZ = z;
}

void EnvironmentControlTool::SetAtmosphereDensity(float density) {
    m_atmosphereDensity = std::max(0.0f, std::min(density, 10.0f));
}

void EnvironmentControlTool::SetTimeOfDay(float hours) {
    // Wrap to 0–24 range
    while (hours < 0.0f) hours += 24.0f;
    while (hours >= 24.0f) hours -= 24.0f;
    m_timeOfDay = hours;
}

} // namespace atlas::tools
