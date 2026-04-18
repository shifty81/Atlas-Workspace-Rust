#pragma once
#include "RendererBackend.h"

namespace atlas::render {

/// Headless renderer backend for CI, servers, and tests.
/// All draw calls are silently counted — no GPU resources are touched.
class NullRendererBackend final : public RendererBackend {
public:
    void Init() override;
    void Shutdown() override;

    void BeginFrame() override;
    void EndFrame() override;

    void SetViewport(int32_t width, int32_t height) override;

    RenderAPI GetAPI() const override { return RenderAPI::Null; }

    /// Returns number of frames rendered since Init().
    uint32_t FrameCount() const { return m_frameCount; }

    /// Returns true if currently between BeginFrame/EndFrame.
    bool IsFrameActive() const { return m_frameActive; }

    int32_t ViewportWidth() const { return m_viewportWidth; }
    int32_t ViewportHeight() const { return m_viewportHeight; }

private:
    int32_t m_viewportWidth = 0;
    int32_t m_viewportHeight = 0;
    uint32_t m_frameCount = 0;
    bool m_frameActive = false;
    bool m_initialized = false;
};

} // namespace atlas::render
