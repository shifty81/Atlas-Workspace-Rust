#include "NullRendererBackend.h"

namespace atlas::render {

void NullRendererBackend::Init() {
    m_capabilities = {};  // everything false / zero
    m_capabilities.deviceName = "NullRenderer";
    m_capabilities.driverVersion = "0.0.0";
    m_initialized = true;
    m_frameCount = 0;
    m_frameActive = false;
}

void NullRendererBackend::Shutdown() {
    m_initialized = false;
    m_frameActive = false;
}

void NullRendererBackend::BeginFrame() {
    m_frameActive = true;
}

void NullRendererBackend::EndFrame() {
    m_frameActive = false;
    m_frameCount++;
}

void NullRendererBackend::SetViewport(int32_t width, int32_t height) {
    m_viewportWidth = width;
    m_viewportHeight = height;
}

} // namespace atlas::render
