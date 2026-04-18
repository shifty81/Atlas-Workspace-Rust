#pragma once
// NF::GLFWInputAdapter — Bridges GLFW input events to the InputSystem.
//
// ⚠️  COMPAT ONLY: Part of the OpenGL compatibility backend.
//     Not used by the primary Win32 input path.
//     Include via: NF/UI/Compat/CompatBackends.h
//
// Installs GLFW callbacks that translate key/mouse/scroll events into
// InputSystem calls.  Compiles as a stub without GLFW headers; real
// implementation would use glfwSetKeyCallback, glfwSetCursorPosCallback, etc.
#include "NF/Core/Core.h"
#include "NF/Input/Input.h"
#include "NF/UI/GLFWWindowProvider.h"

namespace NF {

class GLFWInputAdapter {
public:
    explicit GLFWInputAdapter(InputSystem& input)
        : m_input(input) {}

    void attach(GLFWWindowProvider& window) {
        if (!window.isInitialized()) return;
        m_attached = true;
        NF_LOG_INFO("Input", "GLFWInputAdapter attached to window");
        // Real implementation would:
        //   glfwSetWindowUserPointer(window.nativeHandle(), this);
        //   glfwSetKeyCallback(window.nativeHandle(), keyCallback);
        //   glfwSetCursorPosCallback(window.nativeHandle(), cursorCallback);
        //   glfwSetMouseButtonCallback(window.nativeHandle(), mouseButtonCallback);
        //   glfwSetScrollCallback(window.nativeHandle(), scrollCallback);
    }

    void detach() {
        m_attached = false;
        // Real implementation:
        //   glfwSetKeyCallback(window, nullptr);
        //   glfwSetCursorPosCallback(window, nullptr);
        //   glfwSetMouseButtonCallback(window, nullptr);
        //   glfwSetScrollCallback(window, nullptr);
        NF_LOG_INFO("Input", "GLFWInputAdapter detached");
    }

    [[nodiscard]] bool isAttached() const { return m_attached; }

    // Manual event injection for testing
    void injectKeyEvent(KeyCode key, bool pressed) {
        if (pressed)
            m_input.setKeyDown(key);
        else
            m_input.setKeyUp(key);
        ++m_eventCount;
    }

    void injectMouseMove(float x, float y) {
        m_input.setMousePosition(x, y);
        ++m_eventCount;
    }

    void injectMouseButton(int button, bool pressed) {
        // Map GLFW-style button index (0=left, 1=right, 2=middle) to KeyCode
        KeyCode key = KeyCode::Unknown;
        switch (button) {
            case 0: key = KeyCode::Mouse1; break;
            case 1: key = KeyCode::Mouse2; break;
            case 2: key = KeyCode::Mouse3; break;
            case 3: key = KeyCode::Mouse4; break;
            case 4: key = KeyCode::Mouse5; break;
            default: break;
        }
        if (pressed)
            m_input.setKeyDown(key);
        else
            m_input.setKeyUp(key);
        ++m_eventCount;
    }

    void injectScroll(float xOffset, float yOffset) {
        (void)xOffset;
        m_input.setScrollDelta(yOffset);
        ++m_eventCount;
    }

    [[nodiscard]] size_t eventCount() const { return m_eventCount; }

private:
    InputSystem& m_input;
    bool m_attached = false;
    size_t m_eventCount = 0;
};

} // namespace NF
