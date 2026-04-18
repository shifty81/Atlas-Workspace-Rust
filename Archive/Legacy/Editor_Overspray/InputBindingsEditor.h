#pragma once
// NF::Editor — Input bindings editor
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

enum class InputActionType : uint8_t {
    Button, Axis1D, Axis2D, Analog, Gesture
};

inline const char* inputActionTypeName(InputActionType t) {
    switch (t) {
        case InputActionType::Button:  return "Button";
        case InputActionType::Axis1D:  return "Axis1D";
        case InputActionType::Axis2D:  return "Axis2D";
        case InputActionType::Analog:  return "Analog";
        case InputActionType::Gesture: return "Gesture";
    }
    return "Unknown";
}

enum class InputDevice : uint8_t {
    Keyboard, Mouse, Gamepad, Touch, Joystick, VR
};

inline const char* inputDeviceName(InputDevice d) {
    switch (d) {
        case InputDevice::Keyboard: return "Keyboard";
        case InputDevice::Mouse:    return "Mouse";
        case InputDevice::Gamepad:  return "Gamepad";
        case InputDevice::Touch:    return "Touch";
        case InputDevice::Joystick: return "Joystick";
        case InputDevice::VR:       return "VR";
    }
    return "Unknown";
}

enum class InputBindingConflict : uint8_t {
    None, Warning, Error
};

inline const char* inputBindingConflictName(InputBindingConflict c) {
    switch (c) {
        case InputBindingConflict::None:    return "None";
        case InputBindingConflict::Warning: return "Warning";
        case InputBindingConflict::Error:   return "Error";
    }
    return "Unknown";
}

class InputActionBinding {
public:
    explicit InputActionBinding(const std::string& actionName,
                                 InputActionType type,
                                 InputDevice device,
                                 const std::string& key)
        : m_actionName(actionName), m_type(type), m_device(device), m_key(key) {}

    void setScale(float s)                    { m_scale    = s; }
    void setConflict(InputBindingConflict c)  { m_conflict = c; }
    void setEnabled(bool v)                   { m_enabled  = v; }

    [[nodiscard]] const std::string&   actionName() const { return m_actionName; }
    [[nodiscard]] InputActionType      type()       const { return m_type;       }
    [[nodiscard]] InputDevice          device()     const { return m_device;     }
    [[nodiscard]] const std::string&   key()        const { return m_key;        }
    [[nodiscard]] float                scale()      const { return m_scale;      }
    [[nodiscard]] InputBindingConflict conflict()   const { return m_conflict;   }
    [[nodiscard]] bool                 isEnabled()  const { return m_enabled;    }

    [[nodiscard]] bool hasConflict() const { return m_conflict != InputBindingConflict::None; }

private:
    std::string          m_actionName;
    InputActionType      m_type;
    InputDevice          m_device;
    std::string          m_key;
    float                m_scale    = 1.0f;
    InputBindingConflict m_conflict = InputBindingConflict::None;
    bool                 m_enabled  = true;
};

class InputBindingsEditor {
public:
    static constexpr size_t MAX_BINDINGS = 512;

    [[nodiscard]] bool addBinding(const InputActionBinding& b) {
        if (m_bindings.size() >= MAX_BINDINGS) return false;
        for (auto& e : m_bindings)
            if (e.actionName() == b.actionName() && e.device() == b.device()) return false;
        m_bindings.push_back(b);
        return true;
    }

    [[nodiscard]] bool removeBinding(const std::string& actionName) {
        for (auto it = m_bindings.begin(); it != m_bindings.end(); ++it) {
            if (it->actionName() == actionName) { m_bindings.erase(it); return true; }
        }
        return false;
    }

    [[nodiscard]] InputActionBinding* findBinding(const std::string& actionName) {
        for (auto& b : m_bindings) if (b.actionName() == actionName) return &b;
        return nullptr;
    }

    [[nodiscard]] size_t bindingCount()   const { return m_bindings.size(); }
    [[nodiscard]] size_t conflictCount()  const {
        size_t c = 0; for (auto& b : m_bindings) if (b.hasConflict()) ++c; return c;
    }
    [[nodiscard]] size_t countByDevice(InputDevice d) const {
        size_t c = 0; for (auto& b : m_bindings) if (b.device() == d) ++c; return c;
    }
    [[nodiscard]] size_t countByType(InputActionType t) const {
        size_t c = 0; for (auto& b : m_bindings) if (b.type() == t) ++c; return c;
    }
    [[nodiscard]] size_t enabledCount() const {
        size_t c = 0; for (auto& b : m_bindings) if (b.isEnabled()) ++c; return c;
    }

private:
    std::vector<InputActionBinding> m_bindings;
};

} // namespace NF
