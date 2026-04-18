#pragma once
// NF::Editor — analog axis mapping management editor
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

enum class AxisMappingSource : uint8_t { Gamepad, Mouse, Keyboard, Touch, VR };
inline const char* axisMappingSourceName(AxisMappingSource v) {
    switch (v) {
        case AxisMappingSource::Gamepad:  return "Gamepad";
        case AxisMappingSource::Mouse:    return "Mouse";
        case AxisMappingSource::Keyboard: return "Keyboard";
        case AxisMappingSource::Touch:    return "Touch";
        case AxisMappingSource::VR:       return "VR";
    }
    return "Unknown";
}

enum class AxisMappingMode : uint8_t { Raw, Normalized, Deadzone, Smooth, Inverted };
inline const char* axisMappingModeName(AxisMappingMode v) {
    switch (v) {
        case AxisMappingMode::Raw:        return "Raw";
        case AxisMappingMode::Normalized: return "Normalized";
        case AxisMappingMode::Deadzone:   return "Deadzone";
        case AxisMappingMode::Smooth:     return "Smooth";
        case AxisMappingMode::Inverted:   return "Inverted";
    }
    return "Unknown";
}

class AxisMapping {
public:
    explicit AxisMapping(uint32_t id, const std::string& name,
                          AxisMappingSource source, AxisMappingMode mode)
        : m_id(id), m_name(name), m_source(source), m_mode(mode) {}

    void setScale(float v)      { m_scale     = v; }
    void setDeadzone(float v)   { m_deadzone  = v; }
    void setIsEnabled(bool v)   { m_isEnabled = v; }

    [[nodiscard]] uint32_t           id()        const { return m_id;        }
    [[nodiscard]] const std::string& name()      const { return m_name;      }
    [[nodiscard]] AxisMappingSource  source()    const { return m_source;    }
    [[nodiscard]] AxisMappingMode    mode()      const { return m_mode;      }
    [[nodiscard]] float              scale()     const { return m_scale;     }
    [[nodiscard]] float              deadzone()  const { return m_deadzone;  }
    [[nodiscard]] bool               isEnabled() const { return m_isEnabled; }

private:
    uint32_t         m_id;
    std::string      m_name;
    AxisMappingSource m_source;
    AxisMappingMode   m_mode;
    float            m_scale     = 1.0f;
    float            m_deadzone  = 0.1f;
    bool             m_isEnabled = true;
};

class AxisMappingEditor {
public:
    void setIsShowDisabled(bool v)      { m_isShowDisabled    = v; }
    void setIsGroupBySource(bool v)     { m_isGroupBySource   = v; }
    void setDefaultDeadzone(float v)    { m_defaultDeadzone   = v; }

    bool addMapping(const AxisMapping& m) {
        for (auto& x : m_mappings) if (x.id() == m.id()) return false;
        m_mappings.push_back(m); return true;
    }
    bool removeMapping(uint32_t id) {
        auto it = std::find_if(m_mappings.begin(), m_mappings.end(),
            [&](const AxisMapping& m){ return m.id() == id; });
        if (it == m_mappings.end()) return false;
        m_mappings.erase(it); return true;
    }
    [[nodiscard]] AxisMapping* findMapping(uint32_t id) {
        for (auto& m : m_mappings) if (m.id() == id) return &m;
        return nullptr;
    }

    [[nodiscard]] bool  isShowDisabled()  const { return m_isShowDisabled;  }
    [[nodiscard]] bool  isGroupBySource() const { return m_isGroupBySource; }
    [[nodiscard]] float defaultDeadzone() const { return m_defaultDeadzone; }
    [[nodiscard]] size_t mappingCount()   const { return m_mappings.size(); }

    [[nodiscard]] size_t countBySource(AxisMappingSource s) const {
        size_t n = 0; for (auto& m : m_mappings) if (m.source() == s) ++n; return n;
    }
    [[nodiscard]] size_t countByMode(AxisMappingMode mo) const {
        size_t n = 0; for (auto& m : m_mappings) if (m.mode() == mo) ++n; return n;
    }
    [[nodiscard]] size_t countEnabled() const {
        size_t n = 0; for (auto& m : m_mappings) if (m.isEnabled()) ++n; return n;
    }

private:
    std::vector<AxisMapping> m_mappings;
    bool  m_isShowDisabled  = false;
    bool  m_isGroupBySource = false;
    float m_defaultDeadzone = 0.15f;
};

} // namespace NF
