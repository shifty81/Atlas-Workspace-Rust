#pragma once
// NF::Input — Keyboard, mouse, gamepad, action mappings
#include "NF/Core/Core.h"
#include <unordered_set>

namespace NF {

// ── Key Codes ────────────────────────────────────────────────────

enum class KeyCode : uint16_t {
    Unknown = 0,
    // Letters
    A, B, C, D, E, F, G, H, I, J, K, L, M,
    N, O, P, Q, R, S, T, U, V, W, X, Y, Z,
    // Numbers
    Num0, Num1, Num2, Num3, Num4, Num5, Num6, Num7, Num8, Num9,
    // Function keys
    F1, F2, F3, F4, F5, F6, F7, F8, F9, F10, F11, F12,
    // Navigation
    Left, Right, Up, Down,
    // Modifiers
    Space, Escape, Enter, Tab, Backspace, Delete,
    LShift, RShift, LCtrl, RCtrl, LAlt, RAlt,
    // Mouse buttons
    Mouse1, Mouse2, Mouse3, Mouse4, Mouse5,

    COUNT
};

} // namespace NF

// Specialize std::hash for KeyCode before it is used in unordered containers
template<>
struct std::hash<NF::KeyCode> {
    size_t operator()(NF::KeyCode k) const noexcept {
        return static_cast<size_t>(k);
    }
};

namespace NF {

// ── Mouse state ──────────────────────────────────────────────────

struct MouseState {
    float x = 0.f, y = 0.f;
    float deltaX = 0.f, deltaY = 0.f;
    float scrollDelta = 0.f;
};

// ── Gamepad state ────────────────────────────────────────────────

struct GamepadState {
    bool connected = false;
    float leftStickX  = 0.f, leftStickY  = 0.f;
    float rightStickX = 0.f, rightStickY = 0.f;
    float leftTrigger = 0.f, rightTrigger = 0.f;
    bool buttons[16] = {};
};

// ── Input State (aggregate) ──────────────────────────────────────

struct InputState {
    bool keys[static_cast<size_t>(KeyCode::COUNT)] = {};
    MouseState mouse;
    GamepadState gamepad;
    std::string textInput; // characters typed this frame (from WM_CHAR); '\b'=Backspace, '\r'=Enter
};

// ── Action Mapping ───────────────────────────────────────────────

struct ActionBinding {
    StringID  actionName;
    KeyCode   key = KeyCode::Unknown;
    bool      ctrl  = false;
    bool      shift = false;
    bool      alt   = false;
};

enum class ActionPhase : uint8_t {
    Pressed,    // key just pressed this frame
    Held,       // key is being held
    Released    // key just released this frame
};

struct ActionEvent {
    StringID    actionName;
    ActionPhase phase = ActionPhase::Pressed;
    float       value = 1.f;
};

// ── Input System ─────────────────────────────────────────────────

class InputSystem {
public:
    void init() {
        NF_LOG_INFO("Input", "Input system initialized");
    }

    void shutdown() {
        m_bindings.clear();
        m_pendingEvents.clear();
        NF_LOG_INFO("Input", "Input system shutdown");
    }

    // Append a character from the platform's text-input event (WM_CHAR).
    // '\b' = Backspace, '\r' = Enter, printable ASCII otherwise.
    void appendTextInput(char c) { m_state.textInput += c; }

    void update() {
        m_state.textInput.clear();
        m_pendingEvents.clear();

        for (auto& binding : m_bindings) {
            bool isDown = isKeyDown(binding.key);
            bool wasDown = m_prevKeys.count(binding.key) > 0;

            if (binding.ctrl  && !isKeyDown(KeyCode::LCtrl) && !isKeyDown(KeyCode::RCtrl))  continue;
            if (binding.shift && !isKeyDown(KeyCode::LShift) && !isKeyDown(KeyCode::RShift)) continue;
            if (binding.alt   && !isKeyDown(KeyCode::LAlt) && !isKeyDown(KeyCode::RAlt))     continue;

            if (isDown && !wasDown) {
                m_pendingEvents.push_back({binding.actionName, ActionPhase::Pressed, 1.f});
            } else if (isDown && wasDown) {
                m_pendingEvents.push_back({binding.actionName, ActionPhase::Held, 1.f});
            } else if (!isDown && wasDown) {
                m_pendingEvents.push_back({binding.actionName, ActionPhase::Released, 0.f});
            }
        }

        // Snapshot current key state for next frame comparison
        m_prevKeys.clear();
        for (size_t i = 0; i < static_cast<size_t>(KeyCode::COUNT); ++i) {
            if (m_state.keys[i]) {
                m_prevKeys.insert(static_cast<KeyCode>(i));
            }
        }
    }

    // Key state
    void setKeyDown(KeyCode key) {
        if (key != KeyCode::Unknown)
            m_state.keys[static_cast<size_t>(key)] = true;
    }

    void setKeyUp(KeyCode key) {
        if (key != KeyCode::Unknown)
            m_state.keys[static_cast<size_t>(key)] = false;
    }

    [[nodiscard]] bool isKeyDown(KeyCode key) const {
        if (key == KeyCode::Unknown) return false;
        return m_state.keys[static_cast<size_t>(key)];
    }

    // Mouse
    void setMousePosition(float x, float y) {
        m_state.mouse.deltaX = x - m_state.mouse.x;
        m_state.mouse.deltaY = y - m_state.mouse.y;
        m_state.mouse.x = x;
        m_state.mouse.y = y;
    }

    void setScrollDelta(float delta) { m_state.mouse.scrollDelta = delta; }

    // Gamepad
    void setGamepadConnected(bool connected) { m_state.gamepad.connected = connected; }

    void setGamepadAxis(float lx, float ly, float rx, float ry) {
        m_state.gamepad.leftStickX  = lx;
        m_state.gamepad.leftStickY  = ly;
        m_state.gamepad.rightStickX = rx;
        m_state.gamepad.rightStickY = ry;
    }

    void setGamepadTriggers(float left, float right) {
        m_state.gamepad.leftTrigger  = left;
        m_state.gamepad.rightTrigger = right;
    }

    // Action bindings
    void bindAction(const ActionBinding& binding) {
        m_bindings.push_back(binding);
    }

    void clearBindings() { m_bindings.clear(); }

    [[nodiscard]] const std::vector<ActionEvent>& actionEvents() const {
        return m_pendingEvents;
    }

    [[nodiscard]] bool isActionActive(StringID actionName) const {
        for (auto& evt : m_pendingEvents) {
            if (evt.actionName == actionName &&
                (evt.phase == ActionPhase::Pressed || evt.phase == ActionPhase::Held))
                return true;
        }
        return false;
    }

    // Raw state access
    [[nodiscard]] const InputState& state() const { return m_state; }
    InputState& mutableState() { return m_state; }

private:
    InputState m_state;
    std::vector<ActionBinding> m_bindings;
    std::vector<ActionEvent> m_pendingEvents;
    std::unordered_set<KeyCode> m_prevKeys;
};

} // namespace NF
