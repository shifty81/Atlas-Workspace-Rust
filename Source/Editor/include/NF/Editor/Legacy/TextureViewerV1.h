#pragma once
// NF::Editor — Texture viewer v1: texture loading, zoom, mip level, format inspection
#include "NF/Core/Core.h"
#include <algorithm>
#include <cstdint>
#include <string>
#include <vector>

namespace NF {

enum class TvvFormat : uint8_t { R8, RG8, RGB8, RGBA8, RGBA16F, R32F, BC1, BC3, BC5, BC7 };
enum class TvvMipFilter : uint8_t { None, Point, Linear, Trilinear };

struct TvvTextureInfo {
    uint32_t    id        = 0;
    std::string name;
    uint32_t    width     = 0;
    uint32_t    height    = 0;
    uint32_t    mipLevels = 1;
    TvvFormat   format    = TvvFormat::RGBA8;
    [[nodiscard]] bool isValid() const { return id != 0 && !name.empty() && width > 0 && height > 0; }
    [[nodiscard]] size_t estimatedBytes() const {
        size_t bpp = 4;
        switch(format){
            case TvvFormat::R8:    bpp = 1; break;
            case TvvFormat::RG8:   bpp = 2; break;
            case TvvFormat::RGB8:  bpp = 3; break;
            case TvvFormat::RGBA8: bpp = 4; break;
            default: bpp = 8; break;
        }
        return static_cast<size_t>(width) * height * bpp;
    }
};

class TextureViewerV1 {
public:
    bool loadTexture(const TvvTextureInfo& info) {
        if (!info.isValid()) return false;
        for (const auto& t : m_textures) if (t.id == info.id) return false;
        m_textures.push_back(info);
        if (m_current == nullptr || m_textures.size() == 1)
            m_current = &m_textures.back();
        return true;
    }

    bool unloadTexture(uint32_t id) {
        for (auto it = m_textures.begin(); it != m_textures.end(); ++it) {
            if (it->id == id) {
                if (m_current && m_current->id == id) m_current = nullptr;
                m_textures.erase(it);
                if (!m_textures.empty() && !m_current) m_current = &m_textures.back();
                return true;
            }
        }
        return false;
    }

    const TvvTextureInfo* findTexture(const std::string& name) const {
        for (const auto& t : m_textures) if (t.name == name) return &t;
        return nullptr;
    }

    void     setZoom(float z)     { m_zoom = std::max(0.01f, z); }
    float    getZoom()      const { return m_zoom; }

    void     setMipLevel(uint32_t l) { m_mipLevel = l; }
    uint32_t getMipLevel()     const { return m_mipLevel; }

    void     setMipFilter(TvvMipFilter f) { m_mipFilter = f; }
    TvvMipFilter getMipFilter() const { return m_mipFilter; }

    const TvvTextureInfo* currentTexture() const { return m_current; }

    [[nodiscard]] size_t textureCount() const { return m_textures.size(); }

private:
    std::vector<TvvTextureInfo> m_textures;
    const TvvTextureInfo*       m_current   = nullptr;
    float                       m_zoom      = 1.f;
    uint32_t                    m_mipLevel  = 0;
    TvvMipFilter                m_mipFilter = TvvMipFilter::Linear;
};

} // namespace NF
