#pragma once
// NF::Editor — PIEConsoleBridge: wires PIEExternalLaunch stdout → ConsolePanel.
//
// When the external game process is running, its stdout lines are captured by
// PIEExternalLaunch via pushStdoutLine / setOnStdoutLine.  PIEConsoleBridge
// binds the two objects so every stdout line appears in the ConsolePanel as an
// Info message, making game process output visible inside the editor.
//
// Phase F — PIE console output routing:
//   bind(launch, console) — connect stdout routing; call once at startup
//   unbind()              — disconnect routing (e.g. before destroying panels)
//   lineCount()           — total stdout lines routed since last bind()
//   isBound()             — true while a binding is active

#include "NF/Editor/ConsolePanel.h"
#include "NF/Editor/PIEExternalLaunch.h"

namespace NF {

class PIEConsoleBridge {
public:
    PIEConsoleBridge() = default;
    ~PIEConsoleBridge() { unbind(); }

    // ── Binding ───────────────────────────────────────────────────────────

    /// Connect the stdout callback of `launch` to `console`.
    /// Each line pushed via launch.pushStdoutLine() will appear in the
    /// ConsolePanel as a ConsoleMessageLevel::Info entry at timestamp 0.
    /// Calling bind() again replaces any previous binding.
    void bind(PIEExternalLaunch& launch, ConsolePanel& console) {
        m_launch  = &launch;
        m_console = &console;
        m_lineCount = 0;

        launch.setOnStdoutLine([this](const std::string& line) {
            if (m_console) {
                m_console->addMessage(line, ConsoleMessageLevel::Info, 0.f);
            }
            ++m_lineCount;
        });
    }

    /// Disconnect the stdout callback.  Subsequent pushStdoutLine calls on the
    /// previously bound launch object will not reach the ConsolePanel.
    void unbind() {
        if (m_launch) {
            m_launch->setOnStdoutLine(nullptr);
            m_launch = nullptr;
        }
        m_console = nullptr;
    }

    // ── State ─────────────────────────────────────────────────────────────

    [[nodiscard]] bool     isBound()    const { return m_launch && m_console; }
    [[nodiscard]] uint32_t lineCount()  const { return m_lineCount; }

private:
    PIEExternalLaunch* m_launch  = nullptr;
    ConsolePanel*      m_console = nullptr;
    uint32_t           m_lineCount = 0;
};

} // namespace NF
