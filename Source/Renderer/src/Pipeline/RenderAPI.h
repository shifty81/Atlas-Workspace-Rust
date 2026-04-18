#pragma once
#include <cstdint>

namespace atlas::render {

enum class RenderAPI : uint8_t {
    None = 0,
    OpenGL,
    Vulkan,
    DX11,
    Null
};

} // namespace atlas::render
