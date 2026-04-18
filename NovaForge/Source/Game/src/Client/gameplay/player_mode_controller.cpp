/**
 * @file player_mode_controller.cpp
 * @brief PlayerModeController implementation.
 */

#include "gameplay/player_mode_controller.h"
#include <stdexcept>

namespace atlas::gameplay {

static int ModeIndex(PlayerMode mode) {
    switch (mode) {
        case PlayerMode::FPS:           return 0;
        case PlayerMode::FLIGHT:        return 1;
        case PlayerMode::FLEET_COMMAND: return 2;
    }
    return -1;
}

PlayerModeController::PlayerModeController()
    : m_currentMode(PlayerMode::FLIGHT)
{}

void PlayerModeController::RegisterMode(std::unique_ptr<IPlayerMode> mode) {
    if (!mode) return;
    int idx = ModeIndex(mode->Mode());
    if (idx < 0) return;
    m_modes[idx] = std::move(mode);
}

bool PlayerModeController::TransitionTo(PlayerMode next) {
    if (m_currentMode == next && m_initialized) return false;

    IPlayerMode* nextMode = FindMode(next);
    if (!nextMode) return false;

    m_transitioning = true;

    // Step 1–5: exit current, enter next
    if (m_initialized) {
        IPlayerMode* curMode = FindMode(m_currentMode);
        if (curMode) curMode->OnExit();

        if (m_transitionCallback) {
            m_transitionCallback({ m_currentMode, next });
        }
    }

    PlayerMode prev = m_currentMode;
    m_currentMode   = next;
    m_initialized   = true;
    ++m_transitionCount;

    nextMode->OnEnter();

    // Step 6: transition complete
    m_transitioning = false;
    (void)prev; // suppress unused-variable warning in release
    return true;
}

void PlayerModeController::Tick(float deltaTime) {
    if (!m_initialized) return;
    IPlayerMode* cur = FindMode(m_currentMode);
    if (cur) cur->Tick(deltaTime);
}

bool PlayerModeController::HasMode(PlayerMode mode) const {
    return FindMode(mode) != nullptr;
}

float PlayerModeController::CurrentTimeScale() const {
    IPlayerMode* cur = FindMode(m_currentMode);
    if (!cur) return 1.0f;
    return cur->Ownership().timeScale;
}

ModeOwnership PlayerModeController::CurrentOwnership() const {
    IPlayerMode* cur = FindMode(m_currentMode);
    if (!cur) return ModeOwnership{};
    return cur->Ownership();
}

IPlayerMode* PlayerModeController::FindMode(PlayerMode mode) const {
    int idx = ModeIndex(mode);
    if (idx < 0) return nullptr;
    return m_modes[idx].get();
}

} // namespace atlas::gameplay
