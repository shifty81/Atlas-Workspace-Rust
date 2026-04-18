#pragma once
#include <cstdint>
#include <string>
#include <vector>

namespace atlas::render {

/// Texture format for G-Buffer attachments.
enum class GBufferFormat : uint8_t {
    RGBA8,
    RGBA16F,
    RGBA32F,
    Depth24Stencil8,
    Depth32F,
    RG16F,
    R8
};

/// Describes a single G-Buffer attachment.
struct GBufferAttachment {
    std::string name;
    GBufferFormat format = GBufferFormat::RGBA8;
    uint32_t width = 0;
    uint32_t height = 0;
    uint32_t id = 0;
};

/// Configuration for creating a G-Buffer.
struct GBufferConfig {
    uint32_t width = 0;
    uint32_t height = 0;
    std::vector<GBufferAttachment> attachments;
    bool enableDepth = true;
    bool enableStencil = false;
};

/// G-Buffer for deferred rendering geometry pass.
/// Manages a set of render-target attachments (position, normal, albedo, etc.)
/// that store per-pixel geometry data for later lighting passes.
class GBuffer {
public:
    /// Create a standard PBR G-Buffer configuration.
    static GBufferConfig DefaultPBRConfig(uint32_t w, uint32_t h);

    void Init(const GBufferConfig& config);
    void Shutdown();

    void Bind();
    void Unbind();

    void Resize(uint32_t width, uint32_t height);

    /// Get an attachment by name. Returns nullptr if not found.
    const GBufferAttachment* GetAttachment(const std::string& name) const;

    uint32_t AttachmentCount() const;
    uint32_t Width() const { return m_width; }
    uint32_t Height() const { return m_height; }

    bool IsInitialized() const { return m_initialized; }
    bool IsBound() const { return m_bound; }

private:
    std::vector<GBufferAttachment> m_attachments;
    uint32_t m_width = 0;
    uint32_t m_height = 0;
    bool m_initialized = false;
    bool m_bound = false;
    uint32_t m_nextId = 1;
};

} // namespace atlas::render
