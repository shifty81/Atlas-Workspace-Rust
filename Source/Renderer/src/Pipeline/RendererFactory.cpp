#include "RendererFactory.h"
#include "NullRendererBackend.h"

namespace atlas::render {

std::unique_ptr<RendererBackend> RendererFactory::Create(RenderAPI api) {
    switch (api) {
        case RenderAPI::Null:
            return std::make_unique<NullRendererBackend>();
        case RenderAPI::OpenGL:
        case RenderAPI::Vulkan:
        case RenderAPI::DX11:
            // GPU backends require platform libraries; return nullptr in
            // environments where those are not linked.
            return nullptr;
        case RenderAPI::None:
        default:
            return nullptr;
    }
}

} // namespace atlas::render
