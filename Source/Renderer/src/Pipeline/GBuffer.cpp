#include "GBuffer.h"
#include <algorithm>

namespace atlas::render {

GBufferConfig GBuffer::DefaultPBRConfig(uint32_t w, uint32_t h) {
    GBufferConfig config;
    config.width = w;
    config.height = h;
    config.enableDepth = true;
    config.enableStencil = false;

    config.attachments.push_back({"position", GBufferFormat::RGBA16F, w, h, 0});
    config.attachments.push_back({"normal", GBufferFormat::RGBA16F, w, h, 0});
    config.attachments.push_back({"albedo", GBufferFormat::RGBA8, w, h, 0});
    config.attachments.push_back({"metallic_roughness", GBufferFormat::RG16F, w, h, 0});
    config.attachments.push_back({"depth", GBufferFormat::Depth32F, w, h, 0});

    return config;
}

void GBuffer::Init(const GBufferConfig& config) {
    if (m_initialized) {
        Shutdown();
    }

    m_width = config.width;
    m_height = config.height;
    m_attachments.clear();
    m_nextId = 1;

    for (const auto& att : config.attachments) {
        GBufferAttachment a = att;
        a.width = m_width;
        a.height = m_height;
        a.id = m_nextId++;
        m_attachments.push_back(a);
    }

    m_initialized = true;
    m_bound = false;
}

void GBuffer::Shutdown() {
    m_attachments.clear();
    m_width = 0;
    m_height = 0;
    m_initialized = false;
    m_bound = false;
}

void GBuffer::Bind() {
    if (m_initialized) {
        m_bound = true;
    }
}

void GBuffer::Unbind() {
    m_bound = false;
}

void GBuffer::Resize(uint32_t width, uint32_t height) {
    if (!m_initialized) return;

    m_width = width;
    m_height = height;

    for (auto& att : m_attachments) {
        att.width = width;
        att.height = height;
    }
}

const GBufferAttachment* GBuffer::GetAttachment(const std::string& name) const {
    auto it = std::find_if(m_attachments.begin(), m_attachments.end(),
        [&name](const GBufferAttachment& a) { return a.name == name; });
    if (it != m_attachments.end()) {
        return &(*it);
    }
    return nullptr;
}

uint32_t GBuffer::AttachmentCount() const {
    return static_cast<uint32_t>(m_attachments.size());
}

} // namespace atlas::render
