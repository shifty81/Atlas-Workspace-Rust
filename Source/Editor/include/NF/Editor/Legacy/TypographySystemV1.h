#pragma once
// NF::Editor — Typography system v1: font entry and type scale management
#include "NF/Core/Core.h"
#include <algorithm>
#include <cstdint>
#include <functional>
#include <string>
#include <vector>

namespace NF {

enum class Typv1FontWeight : uint8_t { Thin, Light, Regular, Medium, Bold, Black };
enum class Typv1TypeRole   : uint8_t { Display, Heading, Body, Label, Caption, Code };

inline const char* typv1FontWeightName(Typv1FontWeight w) {
    switch (w) {
        case Typv1FontWeight::Thin:    return "Thin";
        case Typv1FontWeight::Light:   return "Light";
        case Typv1FontWeight::Regular: return "Regular";
        case Typv1FontWeight::Medium:  return "Medium";
        case Typv1FontWeight::Bold:    return "Bold";
        case Typv1FontWeight::Black:   return "Black";
    }
    return "Unknown";
}

inline const char* typv1TypeRoleName(Typv1TypeRole r) {
    switch (r) {
        case Typv1TypeRole::Display: return "Display";
        case Typv1TypeRole::Heading: return "Heading";
        case Typv1TypeRole::Body:    return "Body";
        case Typv1TypeRole::Label:   return "Label";
        case Typv1TypeRole::Caption: return "Caption";
        case Typv1TypeRole::Code:    return "Code";
    }
    return "Unknown";
}

struct Typv1FontEntry {
    uint64_t         id     = 0;
    std::string      family;
    Typv1FontWeight  weight = Typv1FontWeight::Regular;
    Typv1TypeRole    role   = Typv1TypeRole::Body;

    [[nodiscard]] bool isValid()  const { return id != 0 && !family.empty(); }
    [[nodiscard]] bool isBold()   const { return weight == Typv1FontWeight::Bold ||
                                                 weight == Typv1FontWeight::Black; }
};

struct Typv1Scale {
    uint64_t      id         = 0;
    uint64_t      entryId    = 0;
    float         sizePx     = 16.f;
    float         lineHeight = 1.5f;

    [[nodiscard]] bool isValid() const { return id != 0 && entryId != 0 && sizePx > 0.f; }
};

using Typv1ChangeCallback = std::function<void(uint64_t)>;

class TypographySystemV1 {
public:
    static constexpr size_t MAX_FONTS  = 256;
    static constexpr size_t MAX_SCALES = 512;

    bool addFont(const Typv1FontEntry& font) {
        if (!font.isValid()) return false;
        for (const auto& f : m_fonts) if (f.id == font.id) return false;
        if (m_fonts.size() >= MAX_FONTS) return false;
        m_fonts.push_back(font);
        if (m_onChange) m_onChange(font.id);
        return true;
    }

    bool removeFont(uint64_t id) {
        for (auto it = m_fonts.begin(); it != m_fonts.end(); ++it) {
            if (it->id == id) { m_fonts.erase(it); return true; }
        }
        return false;
    }

    [[nodiscard]] Typv1FontEntry* findFont(uint64_t id) {
        for (auto& f : m_fonts) if (f.id == id) return &f;
        return nullptr;
    }

    bool addScale(const Typv1Scale& scale) {
        if (!scale.isValid()) return false;
        for (const auto& s : m_scales) if (s.id == scale.id) return false;
        if (m_scales.size() >= MAX_SCALES) return false;
        m_scales.push_back(scale);
        if (m_onChange) m_onChange(scale.entryId);
        return true;
    }

    bool removeScale(uint64_t id) {
        for (auto it = m_scales.begin(); it != m_scales.end(); ++it) {
            if (it->id == id) { m_scales.erase(it); return true; }
        }
        return false;
    }

    [[nodiscard]] size_t fontCount()  const { return m_fonts.size(); }
    [[nodiscard]] size_t scaleCount() const { return m_scales.size(); }

    [[nodiscard]] size_t boldCount() const {
        size_t c = 0; for (const auto& f : m_fonts) if (f.isBold()) ++c; return c;
    }
    [[nodiscard]] size_t countByRole(Typv1TypeRole role) const {
        size_t c = 0; for (const auto& f : m_fonts) if (f.role == role) ++c; return c;
    }
    [[nodiscard]] size_t countByWeight(Typv1FontWeight weight) const {
        size_t c = 0; for (const auto& f : m_fonts) if (f.weight == weight) ++c; return c;
    }

    void setOnChange(Typv1ChangeCallback cb) { m_onChange = std::move(cb); }

private:
    std::vector<Typv1FontEntry> m_fonts;
    std::vector<Typv1Scale>     m_scales;
    Typv1ChangeCallback         m_onChange;
};

} // namespace NF
