#pragma once
#include "ToolModuleRegistry.h"
#include <cstdint>

namespace atlas::tools {

/// Runtime-toggleable tooling overlay.
///
/// The ToolingLayer wraps a ToolModuleRegistry and adds activation /
/// deactivation semantics so that tools can be toggled on or off at
/// runtime (e.g. via a hotkey in the game client).  When inactive the
/// layer performs no work — tools are not updated or rendered.
///
/// This is the first step toward phasing out the standalone editor
/// executable: the game client can embed a ToolingLayer and provide
/// the same editing experience as an in-game overlay.
class ToolingLayer {
public:
    /// Access the underlying tool module registry.
    ToolModuleRegistry& Registry();
    const ToolModuleRegistry& Registry() const;

    /// Activate the tooling overlay.
    void Activate();

    /// Deactivate the tooling overlay.
    void Deactivate();

    /// Toggle the overlay on/off.
    void Toggle();

    /// Whether the overlay is currently active.
    bool IsActive() const;

    /// Process an input event.  When inactive this is a no-op.
    /// Returns true if a tool consumed the event.
    bool HandleInput(uint32_t keyCode, bool pressed);

    /// Per-frame update.  When inactive this is a no-op.
    void Update(float deltaTime);

    /// Render tool content.  When inactive this is a no-op.
    void Render();

    /// Set the hotkey that toggles the overlay (e.g. F1).
    void SetToggleKey(uint32_t keyCode);

    /// Get the current toggle key.
    uint32_t ToggleKey() const;

private:
    ToolModuleRegistry m_registry;
    bool m_active = false;
    uint32_t m_toggleKey = 0;
};

} // namespace atlas::tools
