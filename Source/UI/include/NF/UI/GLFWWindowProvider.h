#pragma once
// NF::GLFWWindowProvider — GLFW window abstraction for compat / OpenGL path.
//
// ⚠️  COMPAT ONLY: Part of the OpenGL compatibility backend.
//     Not used by the primary D3D11 / Win32 window path.
//     Include via: NF/UI/Compat/CompatBackends.h
//
// Provides window creation, event polling, and frame timing.
// Compiles as a stub without GLFW headers; real implementation requires
// linking against GLFW and a valid display server.
#include "NF/Core/Core.h"
#include <string>

namespace NF {

class GLFWWindowProvider {
public:
    struct WindowConfig {
        int width = 1280;
        int height = 800;
        std::string title = "Atlas Editor";
        bool vsync = true;
        bool maximized = false;
        bool decorated = true;
    };

    bool init(const WindowConfig& config) {
        m_config = config;
        m_width  = config.width;
        m_height = config.height;
        m_initialized = true;
        m_pollCount = 0;
        NF_LOG_INFO("UI", "GLFWWindowProvider initialized (stub) " +
                    std::to_string(m_width) + "x" + std::to_string(m_height) +
                    " title=\"" + config.title + "\"");
        // Real implementation would:
        //   1. glfwInit()
        //   2. glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3) etc.
        //   3. glfwWindowHint(GLFW_MAXIMIZED, config.maximized)
        //   4. glfwWindowHint(GLFW_DECORATED, config.decorated)
        //   5. m_window = glfwCreateWindow(width, height, title, nullptr, nullptr)
        //   6. glfwMakeContextCurrent(m_window)
        //   7. glfwSwapInterval(config.vsync ? 1 : 0)
        return true;
    }

    void shutdown() {
        m_initialized = false;
        m_window = nullptr;
        // Real implementation: glfwDestroyWindow(m_window); glfwTerminate();
        NF_LOG_INFO("UI", "GLFWWindowProvider shutdown");
    }

    [[nodiscard]] bool shouldClose() const {
        // Stub: returns false until pollEvents() has been called at least once.
        // Real implementation: return glfwWindowShouldClose(m_window);
        return m_pollCount > 0;
    }

    void pollEvents() {
        ++m_pollCount;
        // Real implementation: glfwPollEvents();
        // Also update frame timing:
        //   double now = glfwGetTime();
        //   m_frameTime = now - m_lastFrameTime;
        //   m_lastFrameTime = now;
        m_frameTime = 0.016;
    }

    void swapBuffers() {
        // Real implementation: glfwSwapBuffers(m_window);
    }

    [[nodiscard]] int width() const { return m_width; }
    [[nodiscard]] int height() const { return m_height; }

    void setTitle(const std::string& title) {
        m_config.title = title;
        // Real implementation: glfwSetWindowTitle(m_window, title.c_str());
    }

    [[nodiscard]] bool isInitialized() const { return m_initialized; }

    [[nodiscard]] void* nativeHandle() const { return m_window; }

    // Frame timing
    [[nodiscard]] double frameTime() const { return m_frameTime; }
    [[nodiscard]] double fps() const {
        return (m_frameTime > 0.0) ? (1.0 / m_frameTime) : 0.0;
    }

private:
    WindowConfig m_config;
    bool m_initialized = false;
    void* m_window = nullptr; // GLFWwindow* when GLFW is linked
    int m_width = 0;
    int m_height = 0;
    double m_frameTime = 0.016;
    double m_lastFrameTime = 0.0;
    int m_pollCount = 0;
};

} // namespace NF
