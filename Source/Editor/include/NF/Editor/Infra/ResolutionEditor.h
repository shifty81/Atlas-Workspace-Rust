#pragma once
// NF::Editor — resolution profile management editor
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

enum class ResolutionScale : uint8_t { Native, Half, Quarter, Dynamic, Custom };
inline const char* resolutionScaleName(ResolutionScale v) {
    switch (v) {
        case ResolutionScale::Native:  return "Native";
        case ResolutionScale::Half:    return "Half";
        case ResolutionScale::Quarter: return "Quarter";
        case ResolutionScale::Dynamic: return "Dynamic";
        case ResolutionScale::Custom:  return "Custom";
    }
    return "Unknown";
}

enum class ResolutionAspect : uint8_t { Wide16x9, Ultra21x9, Square4x3, Tall9x16, Custom };
inline const char* resolutionAspectName(ResolutionAspect v) {
    switch (v) {
        case ResolutionAspect::Wide16x9:  return "Wide16x9";
        case ResolutionAspect::Ultra21x9: return "Ultra21x9";
        case ResolutionAspect::Square4x3: return "Square4x3";
        case ResolutionAspect::Tall9x16:  return "Tall9x16";
        case ResolutionAspect::Custom:    return "Custom";
    }
    return "Unknown";
}

class ResolutionProfile {
public:
    explicit ResolutionProfile(uint32_t id, const std::string& name,
                                ResolutionScale scale, ResolutionAspect aspect)
        : m_id(id), m_name(name), m_scale(scale), m_aspect(aspect) {}

    void setWidth(uint32_t v)    { m_width     = v; }
    void setHeight(uint32_t v)   { m_height    = v; }
    void setIsEnabled(bool v)    { m_isEnabled = v; }

    [[nodiscard]] uint32_t           id()        const { return m_id;        }
    [[nodiscard]] const std::string& name()      const { return m_name;      }
    [[nodiscard]] ResolutionScale    scale()     const { return m_scale;     }
    [[nodiscard]] ResolutionAspect   aspect()    const { return m_aspect;    }
    [[nodiscard]] uint32_t           width()     const { return m_width;     }
    [[nodiscard]] uint32_t           height()    const { return m_height;    }
    [[nodiscard]] bool               isEnabled() const { return m_isEnabled; }

private:
    uint32_t        m_id;
    std::string     m_name;
    ResolutionScale  m_scale;
    ResolutionAspect m_aspect;
    uint32_t        m_width     = 1920u;
    uint32_t        m_height    = 1080u;
    bool            m_isEnabled = true;
};

class ResolutionEditor {
public:
    void setIsShowDisabled(bool v)    { m_isShowDisabled  = v; }
    void setIsGroupByAspect(bool v)   { m_isGroupByAspect = v; }
    void setDefaultWidth(uint32_t v)  { m_defaultWidth    = v; }

    bool addProfile(const ResolutionProfile& p) {
        for (auto& x : m_profiles) if (x.id() == p.id()) return false;
        m_profiles.push_back(p); return true;
    }
    bool removeProfile(uint32_t id) {
        auto it = std::find_if(m_profiles.begin(), m_profiles.end(),
            [&](const ResolutionProfile& p){ return p.id() == id; });
        if (it == m_profiles.end()) return false;
        m_profiles.erase(it); return true;
    }
    [[nodiscard]] ResolutionProfile* findProfile(uint32_t id) {
        for (auto& p : m_profiles) if (p.id() == id) return &p;
        return nullptr;
    }

    [[nodiscard]] bool     isShowDisabled()  const { return m_isShowDisabled;  }
    [[nodiscard]] bool     isGroupByAspect() const { return m_isGroupByAspect; }
    [[nodiscard]] uint32_t defaultWidth()    const { return m_defaultWidth;    }
    [[nodiscard]] size_t   profileCount()    const { return m_profiles.size(); }

    [[nodiscard]] size_t countByScale(ResolutionScale s) const {
        size_t n = 0; for (auto& p : m_profiles) if (p.scale() == s) ++n; return n;
    }
    [[nodiscard]] size_t countByAspect(ResolutionAspect a) const {
        size_t n = 0; for (auto& p : m_profiles) if (p.aspect() == a) ++n; return n;
    }
    [[nodiscard]] size_t countEnabled() const {
        size_t n = 0; for (auto& p : m_profiles) if (p.isEnabled()) ++n; return n;
    }

private:
    std::vector<ResolutionProfile> m_profiles;
    bool     m_isShowDisabled  = false;
    bool     m_isGroupByAspect = false;
    uint32_t m_defaultWidth    = 1280u;
};

} // namespace NF
