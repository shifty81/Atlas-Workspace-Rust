#pragma once
#include <cstdint>
#include <string>

namespace atlas::render {

/// Centralized GPU capability reporting filled once during backend Init().
struct RendererCapabilities {
    bool bindlessTextures = false;
    bool computeShaders   = false;
    bool rayTracing       = false;

    uint32_t maxMSAASamples   = 1;
    bool     hdrSwapchain     = false;

    uint32_t maxTextureSize   = 0;
    uint32_t maxUniformBuffers = 0;

    std::string deviceName;
    std::string driverVersion;
};

} // namespace atlas::render
