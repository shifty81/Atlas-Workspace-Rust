#pragma once
// NF::Editor — Shared curve asset library
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

enum class CurveLibraryCategory : uint8_t {
    Animation, Material, Gameplay, Audio, UI, Custom
};

inline const char* curveLibraryCategoryName(CurveLibraryCategory c) {
    switch (c) {
        case CurveLibraryCategory::Animation: return "Animation";
        case CurveLibraryCategory::Material:  return "Material";
        case CurveLibraryCategory::Gameplay:  return "Gameplay";
        case CurveLibraryCategory::Audio:     return "Audio";
        case CurveLibraryCategory::UI:        return "UI";
        case CurveLibraryCategory::Custom:    return "Custom";
    }
    return "Unknown";
}

enum class CurveLibraryInterp : uint8_t {
    Linear, Cubic, Step, EaseIn, EaseOut, EaseInOut
};

inline const char* curveLibraryInterpName(CurveLibraryInterp i) {
    switch (i) {
        case CurveLibraryInterp::Linear:    return "Linear";
        case CurveLibraryInterp::Cubic:     return "Cubic";
        case CurveLibraryInterp::Step:      return "Step";
        case CurveLibraryInterp::EaseIn:    return "EaseIn";
        case CurveLibraryInterp::EaseOut:   return "EaseOut";
        case CurveLibraryInterp::EaseInOut: return "EaseInOut";
    }
    return "Unknown";
}

enum class CurveLibraryScope : uint8_t {
    Project, Global, Package, Local
};

inline const char* curveLibraryScopeName(CurveLibraryScope s) {
    switch (s) {
        case CurveLibraryScope::Project: return "Project";
        case CurveLibraryScope::Global:  return "Global";
        case CurveLibraryScope::Package: return "Package";
        case CurveLibraryScope::Local:   return "Local";
    }
    return "Unknown";
}

class CurveAssetEntry {
public:
    explicit CurveAssetEntry(const std::string& name,
                              CurveLibraryCategory category,
                              CurveLibraryInterp   interp)
        : m_name(name), m_category(category), m_interp(interp) {}

    void setScope(CurveLibraryScope s) { m_scope   = s; }
    void setKeyCount(uint16_t k)       { m_keyCount = k; }
    void setLooped(bool v)             { m_looped   = v; }
    void setReadOnly(bool v)           { m_readOnly = v; }

    [[nodiscard]] const std::string&   name()      const { return m_name;     }
    [[nodiscard]] CurveLibraryCategory category()  const { return m_category; }
    [[nodiscard]] CurveLibraryInterp   interp()    const { return m_interp;   }
    [[nodiscard]] CurveLibraryScope    scope()     const { return m_scope;    }
    [[nodiscard]] uint16_t             keyCount()  const { return m_keyCount; }
    [[nodiscard]] bool                 isLooped()  const { return m_looped;   }
    [[nodiscard]] bool                 isReadOnly()const { return m_readOnly; }

private:
    std::string          m_name;
    CurveLibraryCategory m_category;
    CurveLibraryInterp   m_interp;
    CurveLibraryScope    m_scope     = CurveLibraryScope::Project;
    uint16_t             m_keyCount  = 0;
    bool                 m_looped    = false;
    bool                 m_readOnly  = false;
};

class CurveLibrary {
public:
    static constexpr size_t MAX_CURVES = 1024;

    [[nodiscard]] bool addCurve(const CurveAssetEntry& curve) {
        for (auto& c : m_curves) if (c.name() == curve.name()) return false;
        if (m_curves.size() >= MAX_CURVES) return false;
        m_curves.push_back(curve);
        return true;
    }

    [[nodiscard]] bool removeCurve(const std::string& name) {
        for (auto it = m_curves.begin(); it != m_curves.end(); ++it) {
            if (it->name() == name) {
                if (m_selectedCurve == name) m_selectedCurve.clear();
                m_curves.erase(it);
                return true;
            }
        }
        return false;
    }

    [[nodiscard]] CurveAssetEntry* findCurve(const std::string& name) {
        for (auto& c : m_curves) if (c.name() == name) return &c;
        return nullptr;
    }

    [[nodiscard]] bool selectCurve(const std::string& name) {
        for (auto& c : m_curves) if (c.name() == name) { m_selectedCurve = name; return true; }
        return false;
    }

    [[nodiscard]] const std::string& selectedCurve() const { return m_selectedCurve; }
    [[nodiscard]] size_t curveCount()  const { return m_curves.size(); }

    [[nodiscard]] size_t countByCategory(CurveLibraryCategory cat) const {
        size_t c = 0; for (auto& curve : m_curves) if (curve.category() == cat) ++c; return c;
    }
    [[nodiscard]] size_t countByInterp(CurveLibraryInterp i) const {
        size_t c = 0; for (auto& curve : m_curves) if (curve.interp() == i) ++c; return c;
    }
    [[nodiscard]] size_t loopedCount() const {
        size_t c = 0; for (auto& curve : m_curves) if (curve.isLooped()) ++c; return c;
    }
    [[nodiscard]] size_t readOnlyCount() const {
        size_t c = 0; for (auto& curve : m_curves) if (curve.isReadOnly()) ++c; return c;
    }

private:
    std::vector<CurveAssetEntry> m_curves;
    std::string                  m_selectedCurve;
};

} // namespace NF
