#pragma once
#include "../../engine/core/Engine.h"
#include <cstdint>
#include <functional>
#include <string>
#include <vector>

namespace atlas::editor {

enum class PIEMode {
    Stopped,        ///< Editor is in edit mode, not simulating.
    Simulating,     ///< Simulation is running inside the editor.
    Paused,         ///< Simulation paused (can step).
    Possessed       ///< Simulating with a possessed entity (player control).
};

struct PIEConfig {
    /// Tick rate for the simulation (defaults to engine config).
    uint32_t tickRate = 60;
    /// Whether to use client-server loopback (spawn a local server).
    bool loopback = false;
    /// Entity ID to auto-possess when entering PIE. 0 = none.
    uint32_t autoPossessEntity = 0;
};

/// Play-In-Editor controller.
/// Manages the simulate/play/pause/stop lifecycle within the editor,
/// supporting entity possession and client-server loopback.
class PlayInEditor {
public:
    using ModeCallback = std::function<void(PIEMode oldMode, PIEMode newMode)>;

    /// Start simulation in the editor.
    /// Saves a snapshot of the current ECS state for later restore.
    bool StartSimulation(Engine& engine, const PIEConfig& config = {});

    /// Pause the running simulation.
    bool Pause();

    /// Resume a paused simulation.
    bool Resume();

    /// Step one tick while paused.
    bool StepTick(Engine& engine);

    /// Stop simulation and restore pre-simulation state.
    bool StopSimulation(Engine& engine);

    /// Possess an entity (enter first-person control).
    bool PossessEntity(uint32_t entityId);

    /// Release possession of the current entity.
    bool Unpossess();

    /// Enable client-server loopback mode.
    /// Creates a local server context alongside the client.
    bool EnableLoopback(Engine& engine);

    /// Disable loopback mode.
    void DisableLoopback();

    /// Current PIE mode.
    PIEMode Mode() const;

    /// Currently possessed entity ID (0 if none).
    uint32_t PossessedEntity() const;

    /// Whether loopback is active.
    bool IsLoopbackActive() const;

    /// Number of ticks simulated in this PIE session.
    uint64_t TicksSimulated() const;

    /// Set a callback for mode changes.
    void SetModeCallback(ModeCallback cb);

    /// Get the PIE configuration.
    const PIEConfig& Config() const;

private:
    void SetMode(PIEMode newMode);

    PIEMode m_mode = PIEMode::Stopped;
    PIEConfig m_config;
    ModeCallback m_modeCallback;
    uint32_t m_possessedEntity = 0;
    bool m_loopbackActive = false;
    uint64_t m_ticksSimulated = 0;

    /// Snapshot of ECS state before simulation started (for restore).
    std::vector<uint8_t> m_preSimSnapshot;
    uint64_t m_preSimTick = 0;
};

}
