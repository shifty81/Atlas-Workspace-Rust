#include "PlayInEditor.h"

namespace atlas::editor {

bool PlayInEditor::StartSimulation(Engine& engine, const PIEConfig& config) {
    if (m_mode != PIEMode::Stopped) return false;

    m_config = config;
    m_ticksSimulated = 0;
    m_possessedEntity = 0;
    m_loopbackActive = false;

    // Save ECS snapshot for later restore
    m_preSimSnapshot = engine.GetWorld().Serialize();
    m_preSimTick = engine.GetTimeModel().Context().sim.tick;

    // Apply PIE tick rate
    if (config.tickRate > 0) {
        engine.GetScheduler().SetTickRate(config.tickRate);
    }

    // Auto-possess if requested
    if (config.autoPossessEntity != 0) {
        m_possessedEntity = config.autoPossessEntity;
    }

    // Enable loopback if requested
    if (config.loopback) {
        EnableLoopback(engine);
    }

    SetMode(PIEMode::Simulating);
    return true;
}

bool PlayInEditor::Pause() {
    if (m_mode != PIEMode::Simulating && m_mode != PIEMode::Possessed) return false;
    SetMode(PIEMode::Paused);
    return true;
}

bool PlayInEditor::Resume() {
    if (m_mode != PIEMode::Paused) return false;
    SetMode(m_possessedEntity != 0 ? PIEMode::Possessed : PIEMode::Simulating);
    return true;
}

bool PlayInEditor::StepTick(Engine& engine) {
    if (m_mode != PIEMode::Paused) return false;

    engine.GetTimeModel().AdvanceTick();
    const auto& timeCtx = engine.GetTimeModel().Context();
    engine.GetWorld().Update(timeCtx.sim.fixedDeltaTime);
    m_ticksSimulated++;

    return true;
}

bool PlayInEditor::StopSimulation(Engine& engine) {
    if (m_mode == PIEMode::Stopped) return false;

    // Restore pre-simulation state
    if (!m_preSimSnapshot.empty()) {
        engine.GetWorld().Deserialize(m_preSimSnapshot);
        engine.GetTimeModel().SetTick(m_preSimTick);
    }

    m_possessedEntity = 0;
    m_loopbackActive = false;
    m_preSimSnapshot.clear();

    SetMode(PIEMode::Stopped);
    return true;
}

bool PlayInEditor::PossessEntity(uint32_t entityId) {
    if (m_mode != PIEMode::Simulating && m_mode != PIEMode::Paused) return false;
    if (entityId == 0) return false;

    m_possessedEntity = entityId;
    if (m_mode == PIEMode::Simulating) {
        SetMode(PIEMode::Possessed);
    }
    return true;
}

bool PlayInEditor::Unpossess() {
    if (m_possessedEntity == 0) return false;
    m_possessedEntity = 0;
    if (m_mode == PIEMode::Possessed) {
        SetMode(PIEMode::Simulating);
    }
    return true;
}

bool PlayInEditor::EnableLoopback(Engine& engine) {
    if (m_loopbackActive) return false;

    // Switch net context to loopback mode for local client-server testing
    engine.GetNet().Init(net::NetMode::Server);
    engine.GetNet().SetWorld(&engine.GetWorld());
    m_loopbackActive = true;
    return true;
}

void PlayInEditor::DisableLoopback() {
    m_loopbackActive = false;
}

PIEMode PlayInEditor::Mode() const {
    return m_mode;
}

uint32_t PlayInEditor::PossessedEntity() const {
    return m_possessedEntity;
}

bool PlayInEditor::IsLoopbackActive() const {
    return m_loopbackActive;
}

uint64_t PlayInEditor::TicksSimulated() const {
    return m_ticksSimulated;
}

void PlayInEditor::SetModeCallback(ModeCallback cb) {
    m_modeCallback = std::move(cb);
}

const PIEConfig& PlayInEditor::Config() const {
    return m_config;
}

void PlayInEditor::SetMode(PIEMode newMode) {
    PIEMode oldMode = m_mode;
    if (oldMode == newMode) return;
    m_mode = newMode;
    if (m_modeCallback) {
        m_modeCallback(oldMode, newMode);
    }
}

}
