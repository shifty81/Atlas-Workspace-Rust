#pragma once
// NF::Editor — modding toolkit
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

enum class ModType : uint8_t {
    Content, Gameplay, UI, Audio, Visual, Script, Total, Patch
};

inline const char* modTypeName(ModType t) {
    switch (t) {
        case ModType::Content:  return "Content";
        case ModType::Gameplay: return "Gameplay";
        case ModType::UI:       return "UI";
        case ModType::Audio:    return "Audio";
        case ModType::Visual:   return "Visual";
        case ModType::Script:   return "Script";
        case ModType::Total:    return "Total";
        case ModType::Patch:    return "Patch";
    }
    return "Unknown";
}

enum class ModLoadOrder : uint8_t {
    Early, Normal, Late, Override
};

inline const char* modLoadOrderName(ModLoadOrder o) {
    switch (o) {
        case ModLoadOrder::Early:    return "Early";
        case ModLoadOrder::Normal:   return "Normal";
        case ModLoadOrder::Late:     return "Late";
        case ModLoadOrder::Override: return "Override";
    }
    return "Unknown";
}

enum class ModCompatibility : uint8_t {
    Compatible, RequiresBase, Incompatible, Unknown
};

inline const char* modCompatibilityName(ModCompatibility c) {
    switch (c) {
        case ModCompatibility::Compatible:    return "Compatible";
        case ModCompatibility::RequiresBase:  return "RequiresBase";
        case ModCompatibility::Incompatible:  return "Incompatible";
        case ModCompatibility::Unknown:       return "Unknown";
    }
    return "Unknown";
}

class ModDef {
public:
    explicit ModDef(uint32_t id, const std::string& name, ModType type)
        : m_id(id), m_name(name), m_type(type) {}

    void setLoadOrder(ModLoadOrder v)      { m_loadOrder     = v; }
    void setCompatibility(ModCompatibility v) { m_compatibility = v; }
    void setVersion(const std::string& v)  { m_version       = v; }
    void setIsEnabled(bool v)              { m_isEnabled     = v; }
    void setIsRequired(bool v)             { m_isRequired    = v; }

    [[nodiscard]] uint32_t             id()            const { return m_id;            }
    [[nodiscard]] const std::string&   name()          const { return m_name;          }
    [[nodiscard]] ModType              type()          const { return m_type;          }
    [[nodiscard]] ModLoadOrder         loadOrder()     const { return m_loadOrder;     }
    [[nodiscard]] ModCompatibility     compatibility() const { return m_compatibility; }
    [[nodiscard]] const std::string&   version()       const { return m_version;       }
    [[nodiscard]] bool                 isEnabled()     const { return m_isEnabled;     }
    [[nodiscard]] bool                 isRequired()    const { return m_isRequired;    }

private:
    uint32_t         m_id;
    std::string      m_name;
    ModType          m_type;
    ModLoadOrder     m_loadOrder     = ModLoadOrder::Normal;
    ModCompatibility m_compatibility = ModCompatibility::Unknown;
    std::string      m_version       = "1.0.0";
    bool             m_isEnabled     = true;
    bool             m_isRequired    = false;
};

class ModdingToolkit {
public:
    void setShowDisabled(bool v)    { m_showDisabled  = v; }
    void setShowConflicts(bool v)   { m_showConflicts = v; }
    void setAutoResolve(bool v)     { m_autoResolve   = v; }

    bool addMod(const ModDef& m) {
        for (auto& e : m_mods) if (e.id() == m.id()) return false;
        m_mods.push_back(m); return true;
    }
    bool removeMod(uint32_t id) {
        auto it = std::find_if(m_mods.begin(), m_mods.end(),
            [&](const ModDef& e){ return e.id() == id; });
        if (it == m_mods.end()) return false;
        m_mods.erase(it); return true;
    }
    [[nodiscard]] ModDef* findMod(uint32_t id) {
        for (auto& e : m_mods) if (e.id() == id) return &e;
        return nullptr;
    }

    [[nodiscard]] bool   isShowDisabled()  const { return m_showDisabled;  }
    [[nodiscard]] bool   isShowConflicts() const { return m_showConflicts; }
    [[nodiscard]] bool   isAutoResolve()   const { return m_autoResolve;   }
    [[nodiscard]] size_t modCount()        const { return m_mods.size();   }

    [[nodiscard]] size_t countByType(ModType t) const {
        size_t n = 0; for (auto& e : m_mods) if (e.type() == t) ++n; return n;
    }
    [[nodiscard]] size_t countByCompatibility(ModCompatibility c) const {
        size_t n = 0; for (auto& e : m_mods) if (e.compatibility() == c) ++n; return n;
    }
    [[nodiscard]] size_t countEnabled() const {
        size_t n = 0; for (auto& e : m_mods) if (e.isEnabled()) ++n; return n;
    }

private:
    std::vector<ModDef> m_mods;
    bool m_showDisabled  = false;
    bool m_showConflicts = true;
    bool m_autoResolve   = false;
};

} // namespace NF
