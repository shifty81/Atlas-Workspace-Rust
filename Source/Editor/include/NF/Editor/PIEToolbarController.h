#pragma once
// NF::Editor — PIEToolbarController: Play-In-Editor toolbar button controller.
//
// Wraps PIEService with named toolbar actions (pressPlay, pressPause,
// pressStop, pressStep) that map directly to toolbar button presses.
// Also maintains a read-only gate that editor panels can query to suppress
// edits while a PIE session is active.
//
// Phase F — PIE toolbar wiring:
//   pressPlay()    — enter PIE or resume from pause
//   pressPause()   — pause an active session
//   pressStop()    — exit the current session
//   pressStep()    — advance one tick while paused
//   isReadOnly()   — true whenever PIE is Playing or Paused
//   canPlay()      — true when the toolbar "Play" button should be enabled
//   canPause()     — true when the toolbar "Pause" button should be enabled
//   canStop()      — true when the toolbar "Stop" button should be enabled
//   canStep()      — true when the toolbar "Step" button should be enabled

#include "NF/Editor/PIEService.h"
#include <functional>

namespace NF {

// ── PIEToolbarAction ──────────────────────────────────────────────────────────

enum class PIEToolbarAction : uint8_t {
    Play,   ///< enter session or resume from pause
    Pause,  ///< pause the running session
    Stop,   ///< exit the session entirely
    Step,   ///< advance one tick while paused
};

inline const char* pieToolbarActionName(PIEToolbarAction a) {
    switch (a) {
    case PIEToolbarAction::Play:  return "Play";
    case PIEToolbarAction::Pause: return "Pause";
    case PIEToolbarAction::Stop:  return "Stop";
    case PIEToolbarAction::Step:  return "Step";
    }
    return "Unknown";
}

// ── PIEToolbarController ──────────────────────────────────────────────────────

class PIEToolbarController {
public:
    using ActionCallback = std::function<void(PIEToolbarAction)>;

    /// Construct with a reference to the workspace PIEService.
    /// The PIEService must outlive this controller.
    explicit PIEToolbarController(PIEService& pie) : m_pie(pie) {}

    // ── Toolbar button presses ────────────────────────────────────────────

    /// Press the Play button.
    /// Behaviour:
    ///   Stopped → enter() (start session)
    ///   Paused  → resume() (continue session)
    ///   Playing → no-op
    bool pressPlay() {
        bool acted = false;
        if (m_pie.isStopped())       acted = m_pie.enter();
        else if (m_pie.isPaused())   acted = m_pie.resume();
        if (acted) notify(PIEToolbarAction::Play);
        return acted;
    }

    /// Press the Pause button.
    /// Behaviour: Playing → pause(). No-op otherwise.
    bool pressPause() {
        bool acted = m_pie.pause();
        if (acted) notify(PIEToolbarAction::Pause);
        return acted;
    }

    /// Press the Stop button.
    /// Behaviour: exits the session regardless of Playing/Paused state.
    bool pressStop() {
        bool acted = m_pie.exit();
        if (acted) notify(PIEToolbarAction::Stop);
        return acted;
    }

    /// Press the Step button.
    /// Behaviour: Paused → step(). No-op otherwise.
    bool pressStep() {
        bool acted = m_pie.step();
        if (acted) notify(PIEToolbarAction::Step);
        return acted;
    }

    // ── Button enable state ───────────────────────────────────────────────

    /// True when pressing Play would do something useful.
    [[nodiscard]] bool canPlay() const {
        return m_pie.isStopped() || m_pie.isPaused();
    }

    /// True when pressing Pause would do something useful.
    [[nodiscard]] bool canPause() const {
        return m_pie.isPlaying();
    }

    /// True when pressing Stop would do something useful.
    [[nodiscard]] bool canStop() const {
        return !m_pie.isStopped();
    }

    /// True when pressing Step would do something useful.
    [[nodiscard]] bool canStep() const {
        return m_pie.isPaused();
    }

    // ── Read-only gate ────────────────────────────────────────────────────

    /// True while PIE is active (Playing or Paused).
    /// Editor panels must query this to suppress authoring edits during PIE.
    [[nodiscard]] bool isReadOnly() const {
        return !m_pie.isStopped();
    }

    // ── State passthrough ─────────────────────────────────────────────────

    [[nodiscard]] PIEState pieState() const { return m_pie.state(); }

    // ── Observer ──────────────────────────────────────────────────────────

    /// Register a callback that fires each time a toolbar action is dispatched.
    void setOnAction(ActionCallback cb) { m_onAction = std::move(cb); }

    [[nodiscard]] uint32_t actionCount() const { return m_actionCount; }

private:
    PIEService&   m_pie;
    ActionCallback m_onAction;
    uint32_t       m_actionCount = 0;

    void notify(PIEToolbarAction a) {
        ++m_actionCount;
        if (m_onAction) m_onAction(a);
    }
};

} // namespace NF
