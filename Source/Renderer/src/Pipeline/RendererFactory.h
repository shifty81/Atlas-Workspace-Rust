#pragma once
#include "RenderAPI.h"
#include "RendererBackend.h"
#include <memory>

namespace atlas::render {

/// Creates the appropriate RendererBackend for the requested API.
/// Returns nullptr if the API is None or unrecognised.
class RendererFactory {
public:
    static std::unique_ptr<RendererBackend> Create(RenderAPI api);
};

} // namespace atlas::render
