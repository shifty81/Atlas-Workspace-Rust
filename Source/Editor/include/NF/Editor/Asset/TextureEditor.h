#pragma once
// NF::Editor — Texture asset + editor
#include "NF/Core/Core.h"
#include "NF/Engine/Engine.h"
#include "NF/Renderer/Renderer.h"
#include "NF/UI/UI.h"
#include "NF/GraphVM/GraphVM.h"
#include "NF/Input/Input.h"
#include "NF/UI/UIWidgets.h"
#include <filesystem>
#include <fstream>
#include <set>
#include <map>
#include <deque>
#include <unordered_map>

namespace NF {

enum class TextureFormat : uint8_t {
    R8, RG8, RGB8, RGBA8, R16F, RG16F, RGB16F, RGBA16F
};

inline const char* textureFormatName(TextureFormat f) {
    switch (f) {
        case TextureFormat::R8:      return "R8";
        case TextureFormat::RG8:     return "RG8";
        case TextureFormat::RGB8:    return "RGB8";
        case TextureFormat::RGBA8:   return "RGBA8";
        case TextureFormat::R16F:    return "R16F";
        case TextureFormat::RG16F:   return "RG16F";
        case TextureFormat::RGB16F:  return "RGB16F";
        case TextureFormat::RGBA16F: return "RGBA16F";
    }
    return "Unknown";
}

enum class TextureFilter : uint8_t {
    Nearest, Linear, NearestMipmapNearest, LinearMipmapLinear
};

inline const char* textureFilterName(TextureFilter f) {
    switch (f) {
        case TextureFilter::Nearest:               return "Nearest";
        case TextureFilter::Linear:                return "Linear";
        case TextureFilter::NearestMipmapNearest:  return "NearestMipmapNearest";
        case TextureFilter::LinearMipmapLinear:    return "LinearMipmapLinear";
    }
    return "Unknown";
}

enum class TextureWrapMode : uint8_t {
    Repeat, MirroredRepeat, ClampToEdge, ClampToBorder
};

inline const char* textureWrapModeName(TextureWrapMode w) {
    switch (w) {
        case TextureWrapMode::Repeat:          return "Repeat";
        case TextureWrapMode::MirroredRepeat:  return "MirroredRepeat";
        case TextureWrapMode::ClampToEdge:     return "ClampToEdge";
        case TextureWrapMode::ClampToBorder:   return "ClampToBorder";
    }
    return "Unknown";
}

class TextureAsset {
public:
    explicit TextureAsset(const std::string& name,
                          uint32_t width  = 1,
                          uint32_t height = 1)
        : m_name(name), m_width(width), m_height(height) {}

    void setFormat(TextureFormat f)       { m_format = f; }
    void setFilter(TextureFilter f)       { m_filter = f; }
    void setWrapMode(TextureWrapMode w)   { m_wrap   = w; }
    void setMipmapsEnabled(bool v)        { m_mipmaps = v; }
    void setDirty(bool d)                 { m_dirty   = d; }
    void resize(uint32_t w, uint32_t h)   { m_width = w; m_height = h; }

    [[nodiscard]] TextureFormat   format()        const { return m_format; }
    [[nodiscard]] TextureFilter   filter()        const { return m_filter; }
    [[nodiscard]] TextureWrapMode wrapMode()      const { return m_wrap;   }
    [[nodiscard]] bool            mipmapsEnabled()const { return m_mipmaps; }
    [[nodiscard]] bool            isDirty()       const { return m_dirty;   }
    [[nodiscard]] uint32_t        width()         const { return m_width;   }
    [[nodiscard]] uint32_t        height()        const { return m_height;  }
    [[nodiscard]] uint64_t        pixelCount()    const { return static_cast<uint64_t>(m_width) * m_height; }
    [[nodiscard]] const std::string& name()       const { return m_name;    }

    [[nodiscard]] bool isHDR() const {
        return m_format == TextureFormat::R16F   ||
               m_format == TextureFormat::RG16F  ||
               m_format == TextureFormat::RGB16F ||
               m_format == TextureFormat::RGBA16F;
    }

private:
    std::string       m_name;
    uint32_t          m_width   = 1;
    uint32_t          m_height  = 1;
    TextureFormat     m_format  = TextureFormat::RGBA8;
    TextureFilter     m_filter  = TextureFilter::Linear;
    TextureWrapMode   m_wrap    = TextureWrapMode::Repeat;
    bool              m_mipmaps = false;
    bool              m_dirty   = false;
};

class TextureEditor {
public:
    static constexpr size_t MAX_TEXTURES = 256;

    [[nodiscard]] bool addTexture(const TextureAsset& tex) {
        for (auto& t : m_textures) if (t.name() == tex.name()) return false;
        if (m_textures.size() >= MAX_TEXTURES) return false;
        m_textures.push_back(tex);
        return true;
    }

    [[nodiscard]] bool removeTexture(const std::string& name) {
        for (auto it = m_textures.begin(); it != m_textures.end(); ++it) {
            if (it->name() == name) {
                if (m_activeTexture == name) m_activeTexture.clear();
                m_textures.erase(it);
                return true;
            }
        }
        return false;
    }

    [[nodiscard]] TextureAsset* findTexture(const std::string& name) {
        for (auto& t : m_textures) if (t.name() == name) return &t;
        return nullptr;
    }

    [[nodiscard]] bool setActiveTexture(const std::string& name) {
        for (auto& t : m_textures) {
            if (t.name() == name) { m_activeTexture = name; return true; }
        }
        return false;
    }

    [[nodiscard]] size_t textureCount() const { return m_textures.size(); }
    [[nodiscard]] const std::string& activeTexture() const { return m_activeTexture; }
    [[nodiscard]] size_t dirtyCount() const {
        size_t c = 0; for (auto& t : m_textures) if (t.isDirty()) ++c; return c;
    }
    [[nodiscard]] size_t hdrCount() const {
        size_t c = 0; for (auto& t : m_textures) if (t.isHDR()) ++c; return c;
    }
    [[nodiscard]] size_t mipmapCount() const {
        size_t c = 0; for (auto& t : m_textures) if (t.mipmapsEnabled()) ++c; return c;
    }

private:
    std::vector<TextureAsset> m_textures;
    std::string               m_activeTexture;
};

// ── S36 — Font Editor ─────────────────────────────────────────────


} // namespace NF
