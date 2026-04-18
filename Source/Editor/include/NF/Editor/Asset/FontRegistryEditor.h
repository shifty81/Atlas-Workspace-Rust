#pragma once
// NF::Editor — Font registry editor: register, preview, and manage font assets
#include "NF/Editor/TypographySystem.h"
#include <algorithm>
#include <cstdint>
#include <string>
#include <vector>

namespace NF {

// ── Font Asset Format ─────────────────────────────────────────────

enum class FontAssetFormat : uint8_t { TTF, OTF, WOFF, BDF };

inline const char* fontAssetFormatName(FontAssetFormat f) {
    switch (f) {
        case FontAssetFormat::TTF:  return "TTF";
        case FontAssetFormat::OTF:  return "OTF";
        case FontAssetFormat::WOFF: return "WOFF";
        case FontAssetFormat::BDF:  return "BDF";
    }
    return "Unknown";
}

// ── Font Face Descriptor ──────────────────────────────────────────

struct FontFaceDescriptor {
    uint32_t    id          = 0;
    std::string familyName;
    FontWeight  weight      = FontWeight::Regular;
    FontStyle   style       = FontStyle::Normal;
    FontAssetFormat format  = FontAssetFormat::TTF;
    std::string assetPath;
    bool        isEmbedded  = false;  // bundled with AtlasUI
    bool        loaded      = false;

    [[nodiscard]] bool isValid() const {
        return id != 0 && !familyName.empty() && !assetPath.empty();
    }

    [[nodiscard]] std::string displayLabel() const {
        return familyName + " " + fontWeightName(weight) + " "
             + fontStyleName(style);
    }
};

// ── Font Registry Editor ──────────────────────────────────────────

class FontRegistryEditor {
public:
    static constexpr size_t MAX_FACES = 128;

    bool registerFace(const FontFaceDescriptor& face) {
        if (!face.isValid()) return false;
        if (m_faces.size() >= MAX_FACES) return false;
        for (const auto& f : m_faces) {
            if (f.id == face.id) return false;
        }
        m_faces.push_back(face);
        return true;
    }

    bool unregisterFace(uint32_t id) {
        for (auto it = m_faces.begin(); it != m_faces.end(); ++it) {
            if (it->id == id) { m_faces.erase(it); return true; }
        }
        return false;
    }

    [[nodiscard]] const FontFaceDescriptor* find(uint32_t id) const {
        for (const auto& f : m_faces) if (f.id == id) return &f;
        return nullptr;
    }

    [[nodiscard]] const FontFaceDescriptor* findByFamily(
            const std::string& family, FontWeight weight = FontWeight::Regular,
            FontStyle style = FontStyle::Normal) const {
        for (const auto& f : m_faces) {
            if (f.familyName == family && f.weight == weight && f.style == style)
                return &f;
        }
        return nullptr;
    }

    bool markLoaded(uint32_t id, bool loaded = true) {
        for (auto& f : m_faces) {
            if (f.id == id) { f.loaded = loaded; return true; }
        }
        return false;
    }

    [[nodiscard]] std::vector<FontFaceDescriptor> byFamily(const std::string& family) const {
        std::vector<FontFaceDescriptor> result;
        for (const auto& f : m_faces) {
            if (f.familyName == family) result.push_back(f);
        }
        return result;
    }

    [[nodiscard]] std::vector<std::string> families() const {
        std::vector<std::string> names;
        for (const auto& f : m_faces) {
            bool found = false;
            for (const auto& n : names) if (n == f.familyName) { found = true; break; }
            if (!found) names.push_back(f.familyName);
        }
        return names;
    }

    [[nodiscard]] size_t faceCount() const { return m_faces.size(); }
    [[nodiscard]] const std::vector<FontFaceDescriptor>& faces() const { return m_faces; }

    [[nodiscard]] size_t loadedCount() const {
        size_t n = 0;
        for (const auto& f : m_faces) if (f.loaded) ++n;
        return n;
    }

    // Load Atlas embedded faces (UI system font)
    void loadAtlasEmbedded() {
        auto add = [&](uint32_t id, const char* family, FontWeight w,
                       FontStyle style, const char* path) {
            FontFaceDescriptor d;
            d.id = id; d.familyName = family; d.weight = w;
            d.style = style; d.format = FontAssetFormat::TTF;
            d.assetPath = path; d.isEmbedded = true; d.loaded = true;
            registerFace(d);
        };
        add(1,  "AtlasUI", FontWeight::Regular,  FontStyle::Normal,  "Embedded/AtlasUI-Regular.ttf");
        add(2,  "AtlasUI", FontWeight::Medium,   FontStyle::Normal,  "Embedded/AtlasUI-Medium.ttf");
        add(3,  "AtlasUI", FontWeight::Bold,     FontStyle::Normal,  "Embedded/AtlasUI-Bold.ttf");
        add(4,  "AtlasUI", FontWeight::Regular,  FontStyle::Italic,  "Embedded/AtlasUI-Italic.ttf");
        add(5,  "AtlasMono", FontWeight::Regular, FontStyle::Normal, "Embedded/AtlasMono-Regular.ttf");
        add(6,  "AtlasMono", FontWeight::Bold,    FontStyle::Normal, "Embedded/AtlasMono-Bold.ttf");
    }

    // Preview state
    void setPreviewText(const std::string& text)  { m_previewText = text; }
    void setPreviewSize(float sizePx)              { m_previewSizePx = sizePx; }
    void setSelectedFaceId(uint32_t id)            { m_selectedFaceId = id; }

    [[nodiscard]] const std::string& previewText()  const { return m_previewText;     }
    [[nodiscard]] float              previewSizePx() const { return m_previewSizePx;   }
    [[nodiscard]] uint32_t           selectedFaceId() const { return m_selectedFaceId; }

private:
    std::vector<FontFaceDescriptor> m_faces;
    std::string m_previewText    = "The quick brown fox jumps over the lazy dog";
    float       m_previewSizePx  = 13.f;
    uint32_t    m_selectedFaceId = 0;
};

} // namespace NF
