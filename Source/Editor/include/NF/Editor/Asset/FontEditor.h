#pragma once
// NF::Editor — Font asset + editor
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

enum class FontStyle : uint8_t {
    Normal, Italic, Oblique, Inherit
};

inline const char* fontStyleName(FontStyle s) {
    switch (s) {
        case FontStyle::Normal:  return "Normal";
        case FontStyle::Italic:  return "Italic";
        case FontStyle::Oblique: return "Oblique";
        case FontStyle::Inherit: return "Inherit";
    }
    return "Unknown";
}

enum class FontWeight : uint8_t {
    Thin, ExtraLight, Light, Regular, Medium, SemiBold, Bold, ExtraBold, Black
};

inline const char* fontWeightName(FontWeight w) {
    switch (w) {
        case FontWeight::Thin:       return "Thin";
        case FontWeight::ExtraLight: return "ExtraLight";
        case FontWeight::Light:      return "Light";
        case FontWeight::Regular:    return "Regular";
        case FontWeight::Medium:     return "Medium";
        case FontWeight::SemiBold:   return "SemiBold";
        case FontWeight::Bold:       return "Bold";
        case FontWeight::ExtraBold:  return "ExtraBold";
        case FontWeight::Black:      return "Black";
    }
    return "Unknown";
}

enum class FontVariant : uint8_t {
    Normal, SmallCaps, AllSmallCaps, PetiteCaps
};

inline const char* fontVariantName(FontVariant v) {
    switch (v) {
        case FontVariant::Normal:       return "Normal";
        case FontVariant::SmallCaps:    return "SmallCaps";
        case FontVariant::AllSmallCaps: return "AllSmallCaps";
        case FontVariant::PetiteCaps:   return "PetiteCaps";
    }
    return "Unknown";
}

class FontAsset {
public:
    explicit FontAsset(const std::string& family,
                       float              size = 12.0f)
        : m_family(family), m_size(size) {}

    void setStyle(FontStyle s)    { m_style   = s; }
    void setWeight(FontWeight w)  { m_weight  = w; }
    void setVariant(FontVariant v){ m_variant = v; }
    void setSize(float s)         { m_size    = s; }
    void setLineHeight(float lh)  { m_lineHeight = lh; }
    void setLetterSpacing(float ls){ m_letterSpacing = ls; }
    void setEmbedded(bool e)      { m_embedded = e; }
    void setDirty(bool d)         { m_dirty    = d; }

    [[nodiscard]] FontStyle   style()         const { return m_style;         }
    [[nodiscard]] FontWeight  weight()        const { return m_weight;        }
    [[nodiscard]] FontVariant variant()       const { return m_variant;       }
    [[nodiscard]] float       size()          const { return m_size;          }
    [[nodiscard]] float       lineHeight()    const { return m_lineHeight;    }
    [[nodiscard]] float       letterSpacing() const { return m_letterSpacing; }
    [[nodiscard]] bool        isEmbedded()    const { return m_embedded;      }
    [[nodiscard]] bool        isDirty()       const { return m_dirty;         }
    [[nodiscard]] const std::string& family() const { return m_family;        }

    [[nodiscard]] bool isBold()   const { return m_weight == FontWeight::Bold || m_weight == FontWeight::Medium; }
    [[nodiscard]] bool isItalic() const { return m_style  == FontStyle::Italic || m_style == FontStyle::Oblique; }

private:
    std::string m_family;
    float       m_size          = 12.0f;
    float       m_lineHeight    = 1.2f;
    float       m_letterSpacing = 0.0f;
    FontStyle   m_style         = FontStyle::Normal;
    FontWeight  m_weight        = FontWeight::Regular;
    FontVariant m_variant       = FontVariant::Normal;
    bool        m_embedded      = false;
    bool        m_dirty         = false;
};

class FontEditor {
public:
    static constexpr size_t MAX_FONTS = 128;

    [[nodiscard]] bool addFont(const FontAsset& font) {
        for (auto& f : m_fonts) if (f.family() == font.family()) return false;
        if (m_fonts.size() >= MAX_FONTS) return false;
        m_fonts.push_back(font);
        return true;
    }

    [[nodiscard]] bool removeFont(const std::string& family) {
        for (auto it = m_fonts.begin(); it != m_fonts.end(); ++it) {
            if (it->family() == family) {
                if (m_activeFont == family) m_activeFont.clear();
                m_fonts.erase(it);
                return true;
            }
        }
        return false;
    }

    [[nodiscard]] FontAsset* findFont(const std::string& family) {
        for (auto& f : m_fonts) if (f.family() == family) return &f;
        return nullptr;
    }

    [[nodiscard]] bool setActiveFont(const std::string& family) {
        for (auto& f : m_fonts) {
            if (f.family() == family) { m_activeFont = family; return true; }
        }
        return false;
    }

    [[nodiscard]] size_t fontCount()     const { return m_fonts.size(); }
    [[nodiscard]] const std::string& activeFont() const { return m_activeFont; }

    [[nodiscard]] size_t dirtyCount() const {
        size_t c = 0; for (auto& f : m_fonts) if (f.isDirty()) ++c; return c;
    }
    [[nodiscard]] size_t embeddedCount() const {
        size_t c = 0; for (auto& f : m_fonts) if (f.isEmbedded()) ++c; return c;
    }
    [[nodiscard]] size_t boldCount() const {
        size_t c = 0; for (auto& f : m_fonts) if (f.isBold()) ++c; return c;
    }
    [[nodiscard]] size_t italicCount() const {
        size_t c = 0; for (auto& f : m_fonts) if (f.isItalic()) ++c; return c;
    }

private:
    std::vector<FontAsset> m_fonts;
    std::string            m_activeFont;
};

// ── S37 — Icon Editor ─────────────────────────────────────────────


} // namespace NF
