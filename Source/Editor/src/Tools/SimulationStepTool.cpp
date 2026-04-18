#include "SimulationStepTool.h"

namespace atlas::tools {

void SimulationStepTool::Activate() {
    m_active = true;
    m_paused = false;
    m_stepRequested = false;
    m_stepCount = 0;
    m_accumulatedTime = 0.0f;
    m_speedMultiplier = 1.0f;
}

void SimulationStepTool::Deactivate() {
    m_active = false;
    m_paused = false;
    m_stepRequested = false;
}

void SimulationStepTool::Update(float deltaTime) {
    if (!m_active) return;

    if (m_paused && !m_stepRequested) return;

    m_accumulatedTime += deltaTime * m_speedMultiplier;
    m_stepCount++;
    m_stepRequested = false;
}

void SimulationStepTool::RequestStep() {
    m_stepRequested = true;
}

void SimulationStepTool::SetPaused(bool paused) {
    m_paused = paused;
}

void SimulationStepTool::SetSpeedMultiplier(float mult) {
    if (mult > 0.0f) m_speedMultiplier = mult;
}

} // namespace atlas::tools
