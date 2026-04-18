#pragma once
// NF::Editor — Shadow caster editor v1: per-mesh shadow settings and cascade authoring
#include "NF/Core/Core.h"
#include <algorithm>
#include <cstdint>
#include <functional>
#include <string>
#include <vector>

namespace NF {

enum class Scv1ShadowMode   : uint8_t { Off, TwoSided, OneSided, ShadowOnly };
enum class Scv1CascadeMode  : uint8_t { None, TwoCascade, FourCascade, Manual };
enum class Scv1FilterMode   : uint8_t { Hard, PCF, PCSS, VSM };

inline const char* scv1ShadowModeName(Scv1ShadowMode m) {
    switch (m) {
        case Scv1ShadowMode::Off:        return "Off";
        case Scv1ShadowMode::TwoSided:   return "TwoSided";
        case Scv1ShadowMode::OneSided:   return "OneSided";
        case Scv1ShadowMode::ShadowOnly: return "ShadowOnly";
    }
    return "Unknown";
}

inline const char* scv1CascadeModeName(Scv1CascadeMode m) {
    switch (m) {
        case Scv1CascadeMode::None:       return "None";
        case Scv1CascadeMode::TwoCascade: return "TwoCascade";
        case Scv1CascadeMode::FourCascade:return "FourCascade";
        case Scv1CascadeMode::Manual:     return "Manual";
    }
    return "Unknown";
}

inline const char* scv1FilterModeName(Scv1FilterMode f) {
    switch (f) {
        case Scv1FilterMode::Hard: return "Hard";
        case Scv1FilterMode::PCF:  return "PCF";
        case Scv1FilterMode::PCSS: return "PCSS";
        case Scv1FilterMode::VSM:  return "VSM";
    }
    return "Unknown";
}

struct Scv1CasterEntry {
    uint64_t        id            = 0;
    std::string     meshName;
    Scv1ShadowMode  shadowMode    = Scv1ShadowMode::TwoSided;
    Scv1CascadeMode cascadeMode   = Scv1CascadeMode::FourCascade;
    Scv1FilterMode  filterMode    = Scv1FilterMode::PCF;
    float           bias          = 0.001f;
    float           normalBias    = 0.02f;
    uint16_t        resolution    = 1024;
    bool            selfShadow    = true;
    bool            receiveShadow = true;

    [[nodiscard]] bool isValid() const { return id != 0 && !meshName.empty() && resolution > 0; }
    [[nodiscard]] bool castsShadow() const { return shadowMode != Scv1ShadowMode::Off; }
};

using Scv1ChangeCallback = std::function<void(uint64_t)>;

class ShadowCasterEditorV1 {
public:
    static constexpr size_t MAX_ENTRIES = 512;

    bool addEntry(const Scv1CasterEntry& entry) {
        if (!entry.isValid()) return false;
        for (const auto& e : m_entries) if (e.id == entry.id) return false;
        if (m_entries.size() >= MAX_ENTRIES) return false;
        m_entries.push_back(entry);
        return true;
    }

    bool removeEntry(uint64_t id) {
        for (auto it = m_entries.begin(); it != m_entries.end(); ++it) {
            if (it->id == id) { m_entries.erase(it); return true; }
        }
        return false;
    }

    [[nodiscard]] Scv1CasterEntry* findEntry(uint64_t id) {
        for (auto& e : m_entries) if (e.id == id) return &e;
        return nullptr;
    }

    bool setShadowMode(uint64_t id, Scv1ShadowMode mode) {
        auto* e = findEntry(id);
        if (!e) return false;
        e->shadowMode = mode;
        if (m_onChange) m_onChange(id);
        return true;
    }

    bool setCascadeMode(uint64_t id, Scv1CascadeMode mode) {
        auto* e = findEntry(id);
        if (!e) return false;
        e->cascadeMode = mode;
        if (m_onChange) m_onChange(id);
        return true;
    }

    bool setFilterMode(uint64_t id, Scv1FilterMode mode) {
        auto* e = findEntry(id);
        if (!e) return false;
        e->filterMode = mode;
        if (m_onChange) m_onChange(id);
        return true;
    }

    [[nodiscard]] size_t entryCount()       const { return m_entries.size(); }
    [[nodiscard]] size_t castingCount()     const {
        size_t c = 0; for (const auto& e : m_entries) if (e.castsShadow()) ++c; return c;
    }
    [[nodiscard]] size_t countByMode(Scv1ShadowMode m) const {
        size_t c = 0; for (const auto& e : m_entries) if (e.shadowMode == m) ++c; return c;
    }
    [[nodiscard]] size_t countByFilter(Scv1FilterMode f) const {
        size_t c = 0; for (const auto& e : m_entries) if (e.filterMode == f) ++c; return c;
    }

    void setOnChange(Scv1ChangeCallback cb) { m_onChange = std::move(cb); }

private:
    std::vector<Scv1CasterEntry> m_entries;
    Scv1ChangeCallback           m_onChange;
};

} // namespace NF
