#pragma once
// NF::Editor — Typography system: font families, weights, scales, and line height rules
#include "NF/Core/Core.h"
#include "NF/UI/UI.h"
#include "NF/Editor/Asset/FontEditor.h"
#include <cstdint>
#include <string>
#include <vector>

namespace NF {
// Semantic roles in the design-system type scale.

enum class TypeRole : uint8_t {
    Display,      // Hero text / large headings
    Heading1,
    Heading2,
    Heading3,
    Body,         // Default reading text
    BodySmall,
    Label,        // Form labels, tag text
    Caption,      // Supplementary, fine print
    Code,         // Monospace code blocks
    Tooltip,      // Tooltip copy
};

inline const char* typeRoleName(TypeRole r) {
    switch (r) {
        case TypeRole::Display:   return "Display";
        case TypeRole::Heading1:  return "Heading1";
        case TypeRole::Heading2:  return "Heading2";
        case TypeRole::Heading3:  return "Heading3";
        case TypeRole::Body:      return "Body";
        case TypeRole::BodySmall: return "BodySmall";
        case TypeRole::Label:     return "Label";
        case TypeRole::Caption:   return "Caption";
        case TypeRole::Code:      return "Code";
        case TypeRole::Tooltip:   return "Tooltip";
    }
    return "Unknown";
}

// ── Type Style ───────────────────────────────────────────────────
// Fully resolved typographic style for one semantic role.

struct TypeStyle {
    TypeRole   role       = TypeRole::Body;
    float      sizePx     = 13.f;  // font size in pixels
    float      lineHeight = 1.5f;  // multiplier
    float      letterSpacing = 0.f; // em units
    FontWeight weight     = FontWeight::Regular;
    FontStyle  style      = FontStyle::Normal;
    std::string fontFamily;

    [[nodiscard]] float lineHeightPx() const { return sizePx * lineHeight; }
};

// ── Typography Scale ─────────────────────────────────────────────
// Registry of resolved TypeStyle per semantic role.

class TypographyScale {
public:
    void setStyle(const TypeStyle& s) {
        for (auto& existing : m_styles) {
            if (existing.role == s.role) { existing = s; return; }
        }
        m_styles.push_back(s);
    }

    [[nodiscard]] const TypeStyle* style(TypeRole role) const {
        for (const auto& s : m_styles) {
            if (s.role == role) return &s;
        }
        return nullptr;
    }

    [[nodiscard]] size_t count() const { return m_styles.size(); }
    [[nodiscard]] const std::vector<TypeStyle>& styles() const { return m_styles; }

    void loadAtlasDarkDefaults() {
        auto add = [&](TypeRole role, float size, FontWeight w, float lh) {
            TypeStyle s;
            s.role = role; s.sizePx = size; s.weight = w; s.lineHeight = lh;
            s.fontFamily = "AtlasUI";
            setStyle(s);
        };
        add(TypeRole::Display,   32.f, FontWeight::Bold,     1.2f);
        add(TypeRole::Heading1,  24.f, FontWeight::Bold,     1.3f);
        add(TypeRole::Heading2,  20.f, FontWeight::SemiBold, 1.35f);
        add(TypeRole::Heading3,  16.f, FontWeight::SemiBold, 1.4f);
        add(TypeRole::Body,      13.f, FontWeight::Regular,  1.5f);
        add(TypeRole::BodySmall, 11.f, FontWeight::Regular,  1.5f);
        add(TypeRole::Label,     12.f, FontWeight::Medium,   1.4f);
        add(TypeRole::Caption,   10.f, FontWeight::Regular,  1.4f);
        add(TypeRole::Code,      13.f, FontWeight::Regular,  1.6f);
        add(TypeRole::Tooltip,   11.f, FontWeight::Regular,  1.3f);
    }

private:
    std::vector<TypeStyle> m_styles;
};

// ── Typography System ─────────────────────────────────────────────
// Global registry: manages scale, active theme name, and baseline font.

class TypographySystem {
public:
    void initialize(const std::string& themeName = "AtlasDark") {
        m_themeName = themeName;
        m_scale.loadAtlasDarkDefaults();
        m_initialized = true;
    }

    [[nodiscard]] bool isInitialized() const { return m_initialized; }
    [[nodiscard]] const std::string& themeName() const { return m_themeName; }

    [[nodiscard]] const TypeStyle* get(TypeRole role) const {
        return m_scale.style(role);
    }

    [[nodiscard]] TypographyScale& scale() { return m_scale; }
    [[nodiscard]] const TypographyScale& scale() const { return m_scale; }

    void setBaseFontFamily(const std::string& family) { m_baseFontFamily = family; }
    [[nodiscard]] const std::string& baseFontFamily() const { return m_baseFontFamily; }

    [[nodiscard]] float resolvedSizePx(TypeRole role) const {
        const auto* s = m_scale.style(role);
        return s ? s->sizePx : 13.f;
    }

    [[nodiscard]] float resolvedLineHeightPx(TypeRole role) const {
        const auto* s = m_scale.style(role);
        return s ? s->lineHeightPx() : 19.5f;
    }

private:
    TypographyScale m_scale;
    std::string     m_themeName;
    std::string     m_baseFontFamily = "AtlasUI";
    bool            m_initialized    = false;
};

} // namespace NF
