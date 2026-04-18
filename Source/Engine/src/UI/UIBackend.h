#pragma once
#include "UIRenderer.h"  // UIRect, UIColor

namespace atlas::ui {

class UIDrawList;  // forward declare

/// Abstract rendering backend interface for the UI system.
/// Concrete implementations translate UIDrawList commands into
/// API-specific draw calls (OpenGL, Vulkan, DX11, or Null).
class UIBackend {
public:
    virtual ~UIBackend() = default;

    /// Called once per frame before any draw commands.
    virtual void BeginFrame() = 0;

    /// Execute all draw commands in the given draw list.
    virtual void Draw(const UIDrawList& drawList) = 0;

    /// Called once per frame after all draw commands.
    virtual void EndFrame() = 0;

    /// Returns a human-readable name for this backend.
    virtual const char* Name() const = 0;
};

/// Null UIBackend — silently counts frames.  Used in headless / test mode.
class NullUIBackend : public UIBackend {
public:
    void BeginFrame() override { m_frameCount++; }
    void Draw(const UIDrawList& /*drawList*/) override { m_drawCalls++; }
    void EndFrame() override {}

    const char* Name() const override { return "Null"; }

    uint32_t FrameCount() const { return m_frameCount; }
    uint32_t DrawCalls() const { return m_drawCalls; }

    void ResetCounters() { m_frameCount = 0; m_drawCalls = 0; }

private:
    uint32_t m_frameCount = 0;
    uint32_t m_drawCalls = 0;
};

} // namespace atlas::ui
