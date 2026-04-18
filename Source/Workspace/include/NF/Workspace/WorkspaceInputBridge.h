#pragma once
// NF::WorkspaceInputBridge
// Syncs NF::InputSystem state into an AtlasUI BasicInputContext each frame.
// This bridges the Win32/platform input pipeline to the AtlasUI widget system.
//
// The bridge is stateful: it remembers the previous frame's primary-button
// state so it can compute the one-shot primaryPressed flag used by panels
// for click detection (avoids repeated firing while the button is held).

#include "NF/Input/Input.h"
#include "NF/UI/AtlasUI/Contexts.h"

namespace NF {

class WorkspaceInputBridge {
public:
    /// Copy current InputSystem state into a BasicInputContext.
    /// Call once per frame before routing input to panels.
    void sync(const InputSystem& input, UI::AtlasUI::BasicInputContext& ctx) {
        const InputState& state = input.state();
        ctx.setMousePosition({state.mouse.x, state.mouse.y});

        const bool currPrimary = state.keys[static_cast<size_t>(KeyCode::Mouse1)];
        ctx.setPrimaryDown(currPrimary);
        ctx.setPrimaryPressed(!m_prevPrimaryDown && currPrimary);
        m_prevPrimaryDown = currPrimary;

        ctx.setSecondaryDown(state.keys[static_cast<size_t>(KeyCode::Mouse2)]);
        ctx.setScrollDelta(state.mouse.scrollDelta);
        ctx.setTypedText(state.textInput);
        // Mirror a representative set of keyboard keys used by widgets.
        for (size_t k = 0; k < static_cast<size_t>(KeyCode::COUNT); ++k) {
            ctx.setKeyDown(static_cast<int>(k), state.keys[k]);
        }
    }

private:
    bool m_prevPrimaryDown = false;
};

} // namespace NF
