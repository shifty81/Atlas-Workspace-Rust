#pragma once
// NF::Editor — Gamepad configurator and layout viewer
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

enum class GamepadLayout : uint8_t {
    Xbox, PlayStation, Switch, Generic, Custom
};

inline const char* gamepadLayoutName(GamepadLayout l) {
    switch (l) {
        case GamepadLayout::Xbox:        return "Xbox";
        case GamepadLayout::PlayStation: return "PlayStation";
        case GamepadLayout::Switch:      return "Switch";
        case GamepadLayout::Generic:     return "Generic";
        case GamepadLayout::Custom:      return "Custom";
    }
    return "Unknown";
}

enum class GamepadButton : uint8_t {
    A, B, X, Y, LB, RB, LT, RT, LS, RS, Start, Select, DPadUp, DPadDown, DPadLeft, DPadRight
};

inline const char* gamepadButtonName(GamepadButton b) {
    switch (b) {
        case GamepadButton::A:         return "A";
        case GamepadButton::B:         return "B";
        case GamepadButton::X:         return "X";
        case GamepadButton::Y:         return "Y";
        case GamepadButton::LB:        return "LB";
        case GamepadButton::RB:        return "RB";
        case GamepadButton::LT:        return "LT";
        case GamepadButton::RT:        return "RT";
        case GamepadButton::LS:        return "LS";
        case GamepadButton::RS:        return "RS";
        case GamepadButton::Start:     return "Start";
        case GamepadButton::Select:    return "Select";
        case GamepadButton::DPadUp:    return "DPadUp";
        case GamepadButton::DPadDown:  return "DPadDown";
        case GamepadButton::DPadLeft:  return "DPadLeft";
        case GamepadButton::DPadRight: return "DPadRight";
    }
    return "Unknown";
}

enum class AnalogDeadzone : uint8_t {
    None, Radial, Axial, Cross, Circle
};

inline const char* analogDeadzoneName(AnalogDeadzone d) {
    switch (d) {
        case AnalogDeadzone::None:   return "None";
        case AnalogDeadzone::Radial: return "Radial";
        case AnalogDeadzone::Axial:  return "Axial";
        case AnalogDeadzone::Cross:  return "Cross";
        case AnalogDeadzone::Circle: return "Circle";
    }
    return "Unknown";
}

class GamepadProfile {
public:
    explicit GamepadProfile(const std::string& name, GamepadLayout layout)
        : m_name(name), m_layout(layout) {}

    void setDeadzone(AnalogDeadzone d)  { m_deadzone   = d;    }
    void setDeadzoneSize(float s)       { m_deadzoneSize = s;  }
    void setVibration(bool v)           { m_vibration  = v;    }
    void setLookSensitivity(float s)    { m_lookSensitivity = s; }

    [[nodiscard]] const std::string& name()             const { return m_name;             }
    [[nodiscard]] GamepadLayout      layout()           const { return m_layout;           }
    [[nodiscard]] AnalogDeadzone     deadzone()         const { return m_deadzone;         }
    [[nodiscard]] float              deadzoneSize()     const { return m_deadzoneSize;     }
    [[nodiscard]] bool               vibrationEnabled() const { return m_vibration;        }
    [[nodiscard]] float              lookSensitivity()  const { return m_lookSensitivity;  }

private:
    std::string    m_name;
    GamepadLayout  m_layout;
    AnalogDeadzone m_deadzone         = AnalogDeadzone::Radial;
    float          m_deadzoneSize     = 0.1f;
    float          m_lookSensitivity  = 1.0f;
    bool           m_vibration        = true;
};

class GamepadConfiguratorPanel {
public:
    static constexpr size_t MAX_PROFILES = 32;

    [[nodiscard]] bool addProfile(const GamepadProfile& profile) {
        for (auto& p : m_profiles) if (p.name() == profile.name()) return false;
        if (m_profiles.size() >= MAX_PROFILES) return false;
        m_profiles.push_back(profile);
        return true;
    }

    [[nodiscard]] bool removeProfile(const std::string& name) {
        for (auto it = m_profiles.begin(); it != m_profiles.end(); ++it) {
            if (it->name() == name) {
                if (m_activeProfile == name) m_activeProfile.clear();
                m_profiles.erase(it);
                return true;
            }
        }
        return false;
    }

    [[nodiscard]] GamepadProfile* findProfile(const std::string& name) {
        for (auto& p : m_profiles) if (p.name() == name) return &p;
        return nullptr;
    }

    [[nodiscard]] bool setActiveProfile(const std::string& name) {
        for (auto& p : m_profiles) if (p.name() == name) { m_activeProfile = name; return true; }
        return false;
    }

    [[nodiscard]] const std::string& activeProfile() const { return m_activeProfile; }
    [[nodiscard]] size_t             profileCount()  const { return m_profiles.size(); }
    [[nodiscard]] size_t countByLayout(GamepadLayout l) const {
        size_t c = 0; for (auto& p : m_profiles) if (p.layout() == l) ++c; return c;
    }

private:
    std::vector<GamepadProfile> m_profiles;
    std::string                 m_activeProfile;
};

} // namespace NF
