#pragma once
#include "RenderAPI.h"
#include "RendererCapabilities.h"
#include <cstdint>

namespace atlas::render {

/// Abstract base class for all rendering backends.
/// Concrete implementations (OpenGL, Vulkan, DX11, Null) derive from this.
class RendererBackend {
public:
    virtual ~RendererBackend() = default;

    virtual void Init() = 0;
    virtual void Shutdown() = 0;

    virtual void BeginFrame() = 0;
    virtual void EndFrame() = 0;

    virtual void SetViewport(int32_t width, int32_t height) = 0;

    /// Returns the API this backend represents.
    virtual RenderAPI GetAPI() const = 0;

    /// Returns the capabilities filled during Init().
    const RendererCapabilities& GetCapabilities() const { return m_capabilities; }

protected:
    RendererCapabilities m_capabilities;
};

} // namespace atlas::render
