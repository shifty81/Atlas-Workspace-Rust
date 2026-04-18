#pragma once
// NF::Editor — Font editor v1: font config, atlas generation, glyph management (v1 to avoid collision)
#include "NF/Core/Core.h"
#include <cstdint>
#include <functional>
#include <string>
#include <vector>

namespace NF {

enum class Fev1RenderMode : uint8_t { Bitmap, SDF, MSDF };

inline const char* fev1RenderModeName(Fev1RenderMode m) {
    switch(m){
        case Fev1RenderMode::Bitmap: return "Bitmap";
        case Fev1RenderMode::SDF:    return "SDF";
        case Fev1RenderMode::MSDF:   return "MSDF";
    }
    return "Unknown";
}

struct Fev1GlyphInfo {
    uint32_t codepoint = 0;
    float    advance   = 0.f;
    float    bearing   = 0.f;
    [[nodiscard]] bool isValid() const { return codepoint != 0; }
};

struct Fev1FontConfig {
    uint32_t        id          = 0;
    std::string     familyName;
    float           size        = 16.f;
    Fev1RenderMode  mode        = Fev1RenderMode::SDF;
    bool            antialiased = true;
    std::vector<Fev1GlyphInfo> glyphs;
    [[nodiscard]] bool isValid() const { return id != 0 && !familyName.empty(); }
};

using Fev1GenerateCallback = std::function<void(uint32_t, bool)>;

class FontEditorV1 {
public:
    bool addConfig(const Fev1FontConfig& cfg) {
        if (!cfg.isValid()) return false;
        for (const auto& c : m_configs) if (c.id == cfg.id) return false;
        m_configs.push_back(cfg);
        return true;
    }

    bool removeConfig(uint32_t id) {
        for (auto it = m_configs.begin(); it != m_configs.end(); ++it) {
            if (it->id == id) { m_configs.erase(it); return true; }
        }
        return false;
    }

    bool generateAtlas(uint32_t id) {
        Fev1FontConfig* c = findConfig_(id);
        if (!c) return false;
        bool ok = !c->familyName.empty();
        if (m_onGenerate) m_onGenerate(id, ok);
        return ok;
    }

    bool addGlyph(uint32_t configId, const Fev1GlyphInfo& glyph) {
        Fev1FontConfig* c = findConfig_(configId);
        if (!c || !glyph.isValid()) return false;
        for (const auto& g : c->glyphs) if (g.codepoint == glyph.codepoint) return false;
        c->glyphs.push_back(glyph);
        return true;
    }

    const Fev1FontConfig* findConfig(const std::string& name) const {
        for (const auto& c : m_configs) if (c.familyName == name) return &c;
        return nullptr;
    }

    [[nodiscard]] size_t configCount() const { return m_configs.size(); }

    [[nodiscard]] size_t totalGlyphCount() const {
        size_t n = 0;
        for (const auto& c : m_configs) n += c.glyphs.size();
        return n;
    }

    void setOnGenerate(Fev1GenerateCallback cb) { m_onGenerate = std::move(cb); }

private:
    Fev1FontConfig* findConfig_(uint32_t id) {
        for (auto& c : m_configs) if (c.id == id) return &c;
        return nullptr;
    }

    std::vector<Fev1FontConfig>  m_configs;
    Fev1GenerateCallback         m_onGenerate;
};

} // namespace NF
