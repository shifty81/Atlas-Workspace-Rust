#pragma once
// NF::PIEInputRouter — Input routing switch for Play-In-Editor.
//
// When PIE is active, all input events that would normally drive editor
// controls (camera pan, tool selection, etc.) are redirected to the
// game/runtime input system instead.  On PIE exit the router restores
// editor input mode cleanly.
//
// Phase F.2 — PIE Input Mode
//   routeToGame()   — switch to game input (called on PIE enter)
//   routeToEditor() — switch to editor input (called on PIE exit)
//   isGameMode()    — true while in game-input routing mode
//   processKey()    — dispatch a key event to the active sink
//   processMouseButton() / processMouseMove() — mouse dispatch

#include "NF/Core/Core.h"
#include <cstdint>
#include <functional>
#include <string>

namespace NF {

// ── PIEInputMode ──────────────────────────────────────────────────────────────

enum class PIEInputMode : uint8_t {
    Editor, ///< Normal editor controls active
    Game,   ///< Game runtime controls active during PIE
};

inline const char* pieInputModeName(PIEInputMode m) {
    switch (m) {
    case PIEInputMode::Editor: return "Editor";
    case PIEInputMode::Game:   return "Game";
    }
    return "Unknown";
}

// ── PIEKeyEvent ───────────────────────────────────────────────────────────────

struct PIEKeyEvent {
    uint32_t    keyCode   = 0;
    bool        pressed   = true;
    bool        shift     = false;
    bool        ctrl      = false;
    bool        alt       = false;
};

// ── PIEMouseEvent ─────────────────────────────────────────────────────────────

struct PIEMouseEvent {
    float    x         = 0.f;
    float    y         = 0.f;
    uint8_t  button    = 0;    ///< 0=left, 1=right, 2=middle
    bool     pressed   = true;
};

struct PIEMouseMoveEvent {
    float x = 0.f;
    float y = 0.f;
    float dx = 0.f;
    float dy = 0.f;
};

// ── PIEInputRouter ────────────────────────────────────────────────────────────

class PIEInputRouter {
public:
    PIEInputRouter()  = default;
    ~PIEInputRouter() = default;

    // ── Mode switching ────────────────────────────────────────────────────

    void routeToGame() {
        if (m_mode == PIEInputMode::Game) return;
        m_mode = PIEInputMode::Game;
        ++m_modeSwitchCount;
        if (m_onModeChange) m_onModeChange(m_mode);
    }

    void routeToEditor() {
        if (m_mode == PIEInputMode::Editor) return;
        m_mode = PIEInputMode::Editor;
        ++m_modeSwitchCount;
        if (m_onModeChange) m_onModeChange(m_mode);
    }

    [[nodiscard]] PIEInputMode mode()        const { return m_mode; }
    [[nodiscard]] bool         isGameMode()  const { return m_mode == PIEInputMode::Game; }
    [[nodiscard]] bool         isEditorMode() const { return m_mode == PIEInputMode::Editor; }
    [[nodiscard]] uint32_t     modeSwitchCount() const { return m_modeSwitchCount; }

    // ── Key dispatch ──────────────────────────────────────────────────────

    bool processKey(const PIEKeyEvent& evt) {
        ++m_keyEventCount;
        if (m_mode == PIEInputMode::Game) {
            if (m_gameKeySink)   { m_gameKeySink(evt);   return true; }
        } else {
            if (m_editorKeySink) { m_editorKeySink(evt); return true; }
        }
        return false; // no sink registered
    }

    // ── Mouse dispatch ────────────────────────────────────────────────────

    bool processMouseButton(const PIEMouseEvent& evt) {
        ++m_mouseEventCount;
        if (m_mode == PIEInputMode::Game) {
            if (m_gameMouseSink)   { m_gameMouseSink(evt);   return true; }
        } else {
            if (m_editorMouseSink) { m_editorMouseSink(evt); return true; }
        }
        return false;
    }

    bool processMouseMove(const PIEMouseMoveEvent& evt) {
        ++m_mouseMoveCount;
        if (m_mode == PIEInputMode::Game) {
            if (m_gameMouseMoveSink)   { m_gameMouseMoveSink(evt);   return true; }
        } else {
            if (m_editorMouseMoveSink) { m_editorMouseMoveSink(evt); return true; }
        }
        return false;
    }

    // ── Escape key handling ───────────────────────────────────────────────

    /// Check if a key event is the escape key used to exit PIE.
    /// By default escape (keyCode == 0x1B) exits game mode.
    [[nodiscard]] bool isExitKey(const PIEKeyEvent& evt) const {
        return m_mode == PIEInputMode::Game && evt.keyCode == m_exitKeyCode && evt.pressed;
    }

    void setExitKeyCode(uint32_t code) { m_exitKeyCode = code; }
    [[nodiscard]] uint32_t exitKeyCode() const { return m_exitKeyCode; }

    // ── Event counters ────────────────────────────────────────────────────

    [[nodiscard]] uint32_t keyEventCount()    const { return m_keyEventCount; }
    [[nodiscard]] uint32_t mouseEventCount()  const { return m_mouseEventCount; }
    [[nodiscard]] uint32_t mouseMoveCount()   const { return m_mouseMoveCount; }

    // ── Sink registration ─────────────────────────────────────────────────

    using KeySink       = std::function<void(const PIEKeyEvent&)>;
    using MouseSink     = std::function<void(const PIEMouseEvent&)>;
    using MouseMoveSink = std::function<void(const PIEMouseMoveEvent&)>;
    using ModeChangeCb  = std::function<void(PIEInputMode)>;

    void setGameKeySink(KeySink s)             { m_gameKeySink        = std::move(s); }
    void setEditorKeySink(KeySink s)           { m_editorKeySink      = std::move(s); }
    void setGameMouseSink(MouseSink s)         { m_gameMouseSink      = std::move(s); }
    void setEditorMouseSink(MouseSink s)       { m_editorMouseSink    = std::move(s); }
    void setGameMouseMoveSink(MouseMoveSink s) { m_gameMouseMoveSink  = std::move(s); }
    void setEditorMouseMoveSink(MouseMoveSink s){ m_editorMouseMoveSink = std::move(s); }
    void setOnModeChange(ModeChangeCb cb)      { m_onModeChange       = std::move(cb); }

private:
    PIEInputMode  m_mode            = PIEInputMode::Editor;
    uint32_t      m_modeSwitchCount = 0;
    uint32_t      m_keyEventCount   = 0;
    uint32_t      m_mouseEventCount = 0;
    uint32_t      m_mouseMoveCount  = 0;
    uint32_t      m_exitKeyCode     = 0x1B; // VK_ESCAPE

    KeySink       m_gameKeySink;
    KeySink       m_editorKeySink;
    MouseSink     m_gameMouseSink;
    MouseSink     m_editorMouseSink;
    MouseMoveSink m_gameMouseMoveSink;
    MouseMoveSink m_editorMouseMoveSink;
    ModeChangeCb  m_onModeChange;
};

} // namespace NF
