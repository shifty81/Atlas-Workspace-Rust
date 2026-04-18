#pragma once
// NF::Editor — multi-device control scheme management editor
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

enum class CtrlSchemeDevice : uint8_t { Keyboard, Gamepad, Mobile, VR, Custom };
inline const char* ctrlSchemeDeviceName(CtrlSchemeDevice v) {
    switch (v) {
        case CtrlSchemeDevice::Keyboard: return "Keyboard";
        case CtrlSchemeDevice::Gamepad:  return "Gamepad";
        case CtrlSchemeDevice::Mobile:   return "Mobile";
        case CtrlSchemeDevice::VR:       return "VR";
        case CtrlSchemeDevice::Custom:   return "Custom";
    }
    return "Unknown";
}

enum class CtrlSchemeState : uint8_t { Inactive, Active, Conflicted, Disabled };
inline const char* ctrlSchemeSateName(CtrlSchemeState v) {
    switch (v) {
        case CtrlSchemeState::Inactive:   return "Inactive";
        case CtrlSchemeState::Active:     return "Active";
        case CtrlSchemeState::Conflicted: return "Conflicted";
        case CtrlSchemeState::Disabled:   return "Disabled";
    }
    return "Unknown";
}

class ControlScheme {
public:
    explicit ControlScheme(uint32_t id, const std::string& name,
                            CtrlSchemeDevice device, CtrlSchemeState state)
        : m_id(id), m_name(name), m_device(device), m_state(state) {}

    void setPriority(uint32_t v)  { m_priority  = v; }
    void setIsDefault(bool v)     { m_isDefault = v; }
    void setIsEnabled(bool v)     { m_isEnabled = v; }

    [[nodiscard]] uint32_t           id()        const { return m_id;        }
    [[nodiscard]] const std::string& name()      const { return m_name;      }
    [[nodiscard]] CtrlSchemeDevice   device()    const { return m_device;    }
    [[nodiscard]] CtrlSchemeState    state()     const { return m_state;     }
    [[nodiscard]] uint32_t           priority()  const { return m_priority;  }
    [[nodiscard]] bool               isDefault() const { return m_isDefault; }
    [[nodiscard]] bool               isEnabled() const { return m_isEnabled; }

private:
    uint32_t        m_id;
    std::string     m_name;
    CtrlSchemeDevice m_device;
    CtrlSchemeState  m_state;
    uint32_t        m_priority  = 0u;
    bool            m_isDefault = false;
    bool            m_isEnabled = true;
};

class ControlSchemeEditor {
public:
    void setIsShowDisabled(bool v)      { m_isShowDisabled      = v; }
    void setIsGroupByDevice(bool v)     { m_isGroupByDevice     = v; }
    void setAllowMultipleActive(bool v) { m_allowMultipleActive = v; }

    bool addScheme(const ControlScheme& s) {
        for (auto& x : m_schemes) if (x.id() == s.id()) return false;
        m_schemes.push_back(s); return true;
    }
    bool removeScheme(uint32_t id) {
        auto it = std::find_if(m_schemes.begin(), m_schemes.end(),
            [&](const ControlScheme& s){ return s.id() == id; });
        if (it == m_schemes.end()) return false;
        m_schemes.erase(it); return true;
    }
    [[nodiscard]] ControlScheme* findScheme(uint32_t id) {
        for (auto& s : m_schemes) if (s.id() == id) return &s;
        return nullptr;
    }

    [[nodiscard]] bool   isShowDisabled()      const { return m_isShowDisabled;      }
    [[nodiscard]] bool   isGroupByDevice()     const { return m_isGroupByDevice;     }
    [[nodiscard]] bool   allowMultipleActive() const { return m_allowMultipleActive; }
    [[nodiscard]] size_t schemeCount()         const { return m_schemes.size();      }

    [[nodiscard]] size_t countByDevice(CtrlSchemeDevice d) const {
        size_t n = 0; for (auto& s : m_schemes) if (s.device() == d) ++n; return n;
    }
    [[nodiscard]] size_t countByState(CtrlSchemeState st) const {
        size_t n = 0; for (auto& s : m_schemes) if (s.state() == st) ++n; return n;
    }
    [[nodiscard]] size_t countEnabled() const {
        size_t n = 0; for (auto& s : m_schemes) if (s.isEnabled()) ++n; return n;
    }

private:
    std::vector<ControlScheme> m_schemes;
    bool m_isShowDisabled      = false;
    bool m_isGroupByDevice     = true;
    bool m_allowMultipleActive = false;
};

} // namespace NF
