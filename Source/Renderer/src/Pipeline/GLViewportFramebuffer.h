#pragma once
#include "EditorViewportFramebuffer.h"

namespace atlas::render {

/// OpenGL-based offscreen framebuffer for rendering the game scene into a
/// texture that the editor viewport panel can display.
///
/// On creation the FBO, color attachment texture, and depth renderbuffer are
/// allocated.  Resize destroys and recreates the attachments at the new
/// resolution.  Bind/Unbind redirect rendering into/out of the FBO.
///
/// GL extension function pointers (glGenFramebuffers, etc.) are resolved at
/// runtime via glXGetProcAddress / wglGetProcAddress so that the class
/// compiles against the base GL/gl.h header.
class GLViewportFramebuffer : public EditorViewportFramebuffer {
public:
    GLViewportFramebuffer() = default;
    ~GLViewportFramebuffer() override;

    /// Create the framebuffer with the given dimensions.  Returns true on
    /// success.  Safe to call multiple times (destroys previous resources).
    bool Create(uint32_t width, uint32_t height);

    void Resize(uint32_t width, uint32_t height) override;
    void Bind() override;
    void Unbind() override;

    uint32_t GetColorAttachment() const override { return m_colorTexture; }
    uint32_t GetWidth() const override { return m_width; }
    uint32_t GetHeight() const override { return m_height; }
    bool IsValid() const override { return m_fbo != 0 && m_width > 0 && m_height > 0; }

private:
    void Destroy();
    bool ResolveExtensions();

    uint32_t m_fbo = 0;
    uint32_t m_colorTexture = 0;
    uint32_t m_depthRbo = 0;
    uint32_t m_width = 0;
    uint32_t m_height = 0;
    bool m_extensionsResolved = false;
};

} // namespace atlas::render
