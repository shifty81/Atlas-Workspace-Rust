#pragma once
// NF::InputRouter — centralised per-frame input ownership for the workspace.
//
// InputRouter sits between the platform input adapter (InputSystem / Win32)
// and the individual consumers (WorkspaceRenderer, hosted tools, shared
// panels, text-input widgets).  A single instance is owned by WorkspaceShell.
//
// Usage (called once per frame, outside WM_PAINT):
//
//   shell.inputRouter().beginFrame(inputSystem.state());
//
// Usage (inside WM_PAINT / render path):
//
//   UIMouseState mouse = shell.inputRouter().mouseState();
//   wsRenderer.render(ui, w, h, shell, mouse, &launchSvc);
//
// Focus model:
//   The router tracks a lightweight focus target (WorkspaceChrome, ActiveTool,
//   SharedPanel, TextInput) so that future patches can route typed text and
//   key events to the correct consumer without each layer guessing.  For now
//   focus management is advisory — nothing is enforced yet.

#include "NF/Input/Input.h"
#include "NF/UI/UIWidgets.h"
#include <string>

namespace NF {

// ── InputFocusTarget ─────────────────────────────────────────────
// Advisory focus target.  Only one target "owns" keyboard/typed input
// at a time.  The router does not enforce this yet — it is used by
// consumers to decide whether to process typed text.

enum class InputFocusTarget : uint8_t {
    None,
    WorkspaceChrome,   // toolbar, menus, sidebar
    ActiveTool,        // the currently active IHostedTool surface
    SharedPanel,       // a docked shared panel
    TextInput          // a focused TextInput widget
};

// ── InputRouter ───────────────────────────────────────────────────

class InputRouter {
public:
    // Call once per frame BEFORE rendering, from the main loop.
    // |state| is the InputSystem frame snapshot.  Mouse button state and typed
    // text are extracted from |state| directly — no extra parameters needed.
    void beginFrame(const InputState& state) {
        const bool currLeftDown = state.keys[static_cast<size_t>(KeyCode::Mouse1)];
        m_mouse.x           = state.mouse.x;
        m_mouse.y           = state.mouse.y;
        m_mouse.scrollDelta = state.mouse.scrollDelta;
        m_mouse.leftDown    = currLeftDown;
        m_mouse.leftPressed  = !m_prevLeftDown && currLeftDown;
        m_mouse.leftReleased =  m_prevLeftDown && !currLeftDown;
        // state.textInput is cleared by InputSystem::update() at the start of
        // each frame, so typedText is always fresh and scoped to one frame.
        m_mouse.typedText    = state.textInput;
        m_prevLeftDown = currLeftDown;
    }

    // Call once per frame AFTER rendering (optional cleanup hook).
    void endFrame() {
        // Reserved for future per-frame cleanup (e.g. consumed-input tracking).
    }

    // Returns the UIMouseState built from the last beginFrame() call.
    // Safe to call multiple times per frame (same value returned each time).
    [[nodiscard]] const UIMouseState& mouseState() const { return m_mouse; }

    // ── Focus management ─────────────────────────────────────────

    void setFocus(InputFocusTarget target, std::string id = {}) {
        m_focusTarget = target;
        m_focusId     = std::move(id);
    }

    void clearFocus() {
        m_focusTarget = InputFocusTarget::WorkspaceChrome;
        m_focusId.clear();
    }

    [[nodiscard]] InputFocusTarget focusTarget() const { return m_focusTarget; }
    [[nodiscard]] const std::string& focusId()   const { return m_focusId;    }

    [[nodiscard]] bool focusedOn(InputFocusTarget target) const {
        return m_focusTarget == target;
    }

    // Convenience: returns true if the current focus would allow typed text
    // to reach a TextInput widget (i.e. focus is TextInput or None).
    [[nodiscard]] bool textInputActive() const {
        return m_focusTarget == InputFocusTarget::TextInput;
    }

private:
    UIMouseState      m_mouse;
    bool              m_prevLeftDown = false;
    InputFocusTarget  m_focusTarget  = InputFocusTarget::WorkspaceChrome;
    std::string       m_focusId;
};

} // namespace NF
