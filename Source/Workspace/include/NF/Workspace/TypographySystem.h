#pragma once
// NF::TypographySystem — Workspace-wide typography enforcement.
//
// The typography system defines the canonical font stack, size scale,
// and text role mappings for all workspace UI. All AtlasUI widgets
// and panels must source their text metrics from this system — never
// from hardcoded pixel sizes or arbitrary font names.
//
// Roles map to concrete (family, size, weight) triples, allowing the
// entire workspace to retheme its typography by changing one registry
// entry rather than touching every widget.
//
// Design:
//   TextRole          — semantic role enum (Label, Heading, Caption, etc.)
//   TypefaceDescriptor — (family, size, weight, italic)
//   TypographyScale   — ordered collection of TypefaceDescriptors
//   TypographyRegistry — workspace singleton: role → TypefaceDescriptor

#include "NF/Core/Core.h"
#include <cstdint>
#include <string>
#include <unordered_map>
#include <vector>

namespace NF {

// ── Font Weight ───────────────────────────────────────────────────

enum class FontWeight : uint16_t {
    Thin       = 100,
    Light      = 300,
    Regular    = 400,
    Medium     = 500,
    SemiBold   = 600,
    Bold       = 700,
    ExtraBold  = 800,
    Black      = 900,
};

inline const char* fontWeightName(FontWeight w) {
    switch (w) {
    case FontWeight::Thin:      return "Thin";
    case FontWeight::Light:     return "Light";
    case FontWeight::Regular:   return "Regular";
    case FontWeight::Medium:    return "Medium";
    case FontWeight::SemiBold:  return "SemiBold";
    case FontWeight::Bold:      return "Bold";
    case FontWeight::ExtraBold: return "ExtraBold";
    case FontWeight::Black:     return "Black";
    }
    return "Unknown";
}

// ── Text Role ─────────────────────────────────────────────────────
// Semantic roles that map to concrete typeface descriptors.

enum class TextRole : uint8_t {
    // Structural
    Heading1,     // Primary panel/tool heading
    Heading2,     // Section heading
    Heading3,     // Sub-section heading

    // Body
    Body,         // General readable text
    BodySmall,    // Compact body text (densely packed panels)

    // UI chrome
    Label,        // Widget label (property name, menu item)
    LabelSmall,   // Compact widget label
    Caption,      // Annotation, tooltip, status bar

    // Monospace (code / data)
    Code,         // Source code, scripts
    CodeSmall,    // Compact code (console output)
    Data,         // Tabular data values

    // Special
    Icon,         // Icon glyph (icon font)
    Badge,        // Badge/count overlays
};

inline const char* textRoleName(TextRole r) {
    switch (r) {
    case TextRole::Heading1:   return "Heading1";
    case TextRole::Heading2:   return "Heading2";
    case TextRole::Heading3:   return "Heading3";
    case TextRole::Body:       return "Body";
    case TextRole::BodySmall:  return "BodySmall";
    case TextRole::Label:      return "Label";
    case TextRole::LabelSmall: return "LabelSmall";
    case TextRole::Caption:    return "Caption";
    case TextRole::Code:       return "Code";
    case TextRole::CodeSmall:  return "CodeSmall";
    case TextRole::Data:       return "Data";
    case TextRole::Icon:       return "Icon";
    case TextRole::Badge:      return "Badge";
    }
    return "Unknown";
}

// ── TypefaceDescriptor ────────────────────────────────────────────
// Concrete specification for how a TextRole should be rendered.

struct TypefaceDescriptor {
    std::string family  = "Segoe UI";
    float       size    = 12.f;       // logical pixel size
    FontWeight  weight  = FontWeight::Regular;
    bool        italic  = false;

    // Derived metrics (set by the system from backend measurement)
    float lineHeight    = 0.f;  // 0 = auto (size * 1.2)
    float letterSpacing = 0.f;  // extra px between characters (0 = default)

    [[nodiscard]] bool isValid() const {
        return !family.empty() && size > 0.f;
    }

    [[nodiscard]] float effectiveLineHeight() const {
        return lineHeight > 0.f ? lineHeight : size * 1.2f;
    }
};

// ── TypographyScale ───────────────────────────────────────────────
// Named set of TypefaceDescriptors (e.g. "compact", "comfortable", "large").

struct TypographyScale {
    std::string name;
    float       baseSize = 12.f;  // reference body size; all other sizes scale relative to this

    // Scale factors relative to baseSize:
    //   Heading1 = baseSize * 2.0
    //   Heading2 = baseSize * 1.5
    //   Heading3 = baseSize * 1.25
    //   Body     = baseSize * 1.0
    //   Label    = baseSize * 0.9
    //   Caption  = baseSize * 0.75
    //   Code     = baseSize * 0.95 (monospace)

    [[nodiscard]] bool isValid() const { return !name.empty() && baseSize > 0.f; }
};

// ── TypographyRegistry ────────────────────────────────────────────
// Workspace-owned registry mapping TextRole → TypefaceDescriptor.
// Initialized with sensible defaults; can be overridden for branding.

class TypographyRegistry {
public:
    // Load default workspace typography (Segoe UI + Consolas).
    void loadDefaults() {
        static constexpr float base = 12.f;
        setRole(TextRole::Heading1,   {"Segoe UI", base * 2.0f,  FontWeight::Bold});
        setRole(TextRole::Heading2,   {"Segoe UI", base * 1.5f,  FontWeight::SemiBold});
        setRole(TextRole::Heading3,   {"Segoe UI", base * 1.25f, FontWeight::Medium});
        setRole(TextRole::Body,       {"Segoe UI", base * 1.0f,  FontWeight::Regular});
        setRole(TextRole::BodySmall,  {"Segoe UI", base * 0.85f, FontWeight::Regular});
        setRole(TextRole::Label,      {"Segoe UI", base * 0.9f,  FontWeight::Regular});
        setRole(TextRole::LabelSmall, {"Segoe UI", base * 0.75f, FontWeight::Regular});
        setRole(TextRole::Caption,    {"Segoe UI", base * 0.75f, FontWeight::Light});
        setRole(TextRole::Code,       {"Consolas",  base * 0.95f, FontWeight::Regular});
        setRole(TextRole::CodeSmall,  {"Consolas",  base * 0.80f, FontWeight::Regular});
        setRole(TextRole::Data,       {"Consolas",  base * 0.85f, FontWeight::Regular});
        setRole(TextRole::Icon,       {"Segoe MDL2 Assets", base, FontWeight::Regular});
        setRole(TextRole::Badge,      {"Segoe UI", base * 0.7f,  FontWeight::Bold});
        m_loaded = true;
    }

    // Override a role's descriptor.
    void setRole(TextRole role, TypefaceDescriptor desc) {
        m_map[static_cast<uint8_t>(role)] = std::move(desc);
    }

    // Look up the descriptor for a role. Returns a default descriptor if not found.
    [[nodiscard]] const TypefaceDescriptor& getRole(TextRole role) const {
        auto it = m_map.find(static_cast<uint8_t>(role));
        if (it != m_map.end()) return it->second;
        return m_defaultDesc;
    }

    [[nodiscard]] bool hasRole(TextRole role) const {
        return m_map.count(static_cast<uint8_t>(role)) > 0;
    }

    [[nodiscard]] bool isLoaded()  const { return m_loaded; }
    [[nodiscard]] size_t roleCount() const { return m_map.size(); }

    // Apply a scale adjustment to all registered roles.
    // Multiplies each role's size by the given factor.
    void applyScale(float factor) {
        if (factor <= 0.f) return;
        for (auto& [k, v] : m_map) {
            v.size = std::max(6.f, v.size * factor);
        }
    }

    // Enumerate all registered roles.
    [[nodiscard]] std::vector<TextRole> registeredRoles() const {
        std::vector<TextRole> roles;
        roles.reserve(m_map.size());
        for (const auto& [k, _] : m_map)
            roles.push_back(static_cast<TextRole>(k));
        return roles;
    }

    // Validate that all canonical roles are registered.
    [[nodiscard]] bool validate() const {
        static const TextRole kAll[] = {
            TextRole::Heading1, TextRole::Heading2, TextRole::Heading3,
            TextRole::Body, TextRole::BodySmall,
            TextRole::Label, TextRole::LabelSmall, TextRole::Caption,
            TextRole::Code, TextRole::CodeSmall, TextRole::Data,
            TextRole::Icon, TextRole::Badge,
        };
        for (auto r : kAll) if (!hasRole(r)) return false;
        return true;
    }

    void clear() { m_map.clear(); m_loaded = false; }

private:
    std::unordered_map<uint8_t, TypefaceDescriptor> m_map;
    TypefaceDescriptor m_defaultDesc; // fallback: empty family, size 12
    bool m_loaded = false;
};

// ── TypographyEnforcementReport ───────────────────────────────────
// Result of validating that all registered roles have valid descriptors.

struct TypographyEnforcementReport {
    struct Violation {
        TextRole    role;
        std::string reason;
    };

    std::vector<Violation> violations;
    bool                   passed = false;

    void addViolation(TextRole role, const std::string& reason) {
        violations.push_back({role, reason});
        passed = false;
    }

    [[nodiscard]] bool hasViolations() const { return !violations.empty(); }
    [[nodiscard]] size_t count() const { return violations.size(); }
};

// ── TypographyEnforcer ────────────────────────────────────────────
// Validates a TypographyRegistry against workspace rules.

class TypographyEnforcer {
public:
    // Minimum readable size for any non-icon role.
    static constexpr float kMinBodySize   = 9.f;
    // Heading roles must be strictly larger than Body.
    static constexpr float kMinHeadingSize = 14.f;

    [[nodiscard]] static TypographyEnforcementReport enforce(
        const TypographyRegistry& reg)
    {
        TypographyEnforcementReport report;
        report.passed = true;

        // All roles must be present
        if (!reg.validate()) {
            report.addViolation(TextRole::Body, "Registry is incomplete — missing roles");
            return report;
        }

        // Body text must meet minimum size
        const auto& body = reg.getRole(TextRole::Body);
        if (body.size < kMinBodySize) {
            report.addViolation(TextRole::Body,
                "Body size " + std::to_string(body.size) + " < minimum " +
                std::to_string(kMinBodySize));
        }

        // Headings must be larger than body
        for (auto r : {TextRole::Heading1, TextRole::Heading2, TextRole::Heading3}) {
            const auto& h = reg.getRole(r);
            if (h.size < kMinHeadingSize) {
                report.addViolation(r,
                    std::string(textRoleName(r)) + " size " +
                    std::to_string(h.size) + " < minimum heading size " +
                    std::to_string(kMinHeadingSize));
            }
            if (h.size <= body.size) {
                report.addViolation(r,
                    std::string(textRoleName(r)) + " size must be > Body size");
            }
        }

        // Heading1 must be larger than Heading2
        if (reg.getRole(TextRole::Heading1).size <= reg.getRole(TextRole::Heading2).size) {
            report.addViolation(TextRole::Heading1, "Heading1 must be larger than Heading2");
        }
        // Heading2 must be larger than Heading3
        if (reg.getRole(TextRole::Heading2).size <= reg.getRole(TextRole::Heading3).size) {
            report.addViolation(TextRole::Heading2, "Heading2 must be larger than Heading3");
        }

        // Code and Data roles should use monospace families
        for (auto r : {TextRole::Code, TextRole::CodeSmall, TextRole::Data}) {
            const auto& d = reg.getRole(r);
            if (d.family.find("Consolas") == std::string::npos &&
                d.family.find("Mono")     == std::string::npos &&
                d.family.find("Courier")  == std::string::npos) {
                report.addViolation(r,
                    std::string(textRoleName(r)) + " family '" + d.family +
                    "' does not look monospace");
            }
        }

        if (report.violations.empty()) report.passed = true;
        return report;
    }
};

} // namespace NF
