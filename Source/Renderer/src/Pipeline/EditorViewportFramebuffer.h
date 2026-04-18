#pragma once
#include <cstdint>

namespace atlas::render {

/// Abstraction for an offscreen framebuffer used to render the game scene
/// into a texture that the editor viewport panel can display.
///
/// This addresses the root cause of the GUI rendering issue where the scene
/// was rendered directly to the swapchain backbuffer, appearing behind the
/// editor UI instead of inside the viewport panel.
///
/// Each render backend (OpenGL, Vulkan, etc.) provides a concrete
/// implementation.  The editor render loop uses this interface to:
///   1. Bind the framebuffer before scene rendering
///   2. Render the scene into the offscreen texture
///   3. Unbind and draw the resulting texture inside the viewport panel
class EditorViewportFramebuffer {
public:
    virtual ~EditorViewportFramebuffer() = default;

    /// Resize the offscreen framebuffer.  Only call when the viewport panel
    /// size actually changes — do NOT call every frame.
    virtual void Resize(uint32_t width, uint32_t height) = 0;

    /// Bind this framebuffer as the active render target.
    virtual void Bind() = 0;

    /// Unbind and restore the default (swapchain) render target.
    virtual void Unbind() = 0;

    /// Returns a handle to the color attachment texture.  The UI renderer
    /// draws this as a textured quad inside the viewport panel.
    virtual uint32_t GetColorAttachment() const = 0;

    virtual uint32_t GetWidth() const = 0;
    virtual uint32_t GetHeight() const = 0;

    /// Returns true if the framebuffer has been successfully created.
    virtual bool IsValid() const = 0;
};

/// Null implementation for headless / CI / server builds where no GPU is
/// available.  All operations are safe no-ops.
class NullViewportFramebuffer : public EditorViewportFramebuffer {
public:
    explicit NullViewportFramebuffer(uint32_t width = 0, uint32_t height = 0)
        : m_width(width), m_height(height) {}

    void Resize(uint32_t width, uint32_t height) override {
        m_width = width;
        m_height = height;
    }
    void Bind() override {}
    void Unbind() override {}
    uint32_t GetColorAttachment() const override { return 0; }
    uint32_t GetWidth() const override { return m_width; }
    uint32_t GetHeight() const override { return m_height; }
    bool IsValid() const override { return m_width > 0 && m_height > 0; }

private:
    uint32_t m_width = 0;
    uint32_t m_height = 0;
};

} // namespace atlas::render
