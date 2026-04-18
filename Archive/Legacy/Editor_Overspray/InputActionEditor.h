#pragma once
// NF::Editor — input action definition management editor
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

enum class InputActType : uint8_t { Button, Axis, Gesture, Touch, Custom };
inline const char* inputActTypeName(InputActType v) {
    switch (v) {
        case InputActType::Button:  return "Button";
        case InputActType::Axis:    return "Axis";
        case InputActType::Gesture: return "Gesture";
        case InputActType::Touch:   return "Touch";
        case InputActType::Custom:  return "Custom";
    }
    return "Unknown";
}

enum class InputActionTrigger : uint8_t { Pressed, Released, Held, DoubleTap, LongPress };
inline const char* inputActionTriggerName(InputActionTrigger v) {
    switch (v) {
        case InputActionTrigger::Pressed:    return "Pressed";
        case InputActionTrigger::Released:   return "Released";
        case InputActionTrigger::Held:       return "Held";
        case InputActionTrigger::DoubleTap:  return "DoubleTap";
        case InputActionTrigger::LongPress:  return "LongPress";
    }
    return "Unknown";
}

class InputActionDef {
public:
    explicit InputActionDef(uint32_t id, const std::string& name,
                             InputActType type, InputActionTrigger trigger)
        : m_id(id), m_name(name), m_type(type), m_trigger(trigger) {}

    void setConsumeInput(bool v)    { m_consumeInput = v; }
    void setRepeatDelay(float v)    { m_repeatDelay  = v; }
    void setIsEnabled(bool v)       { m_isEnabled    = v; }

    [[nodiscard]] uint32_t            id()           const { return m_id;           }
    [[nodiscard]] const std::string&  name()         const { return m_name;         }
    [[nodiscard]] InputActType        type()         const { return m_type;         }
    [[nodiscard]] InputActionTrigger  trigger()      const { return m_trigger;      }
    [[nodiscard]] bool                consumeInput() const { return m_consumeInput; }
    [[nodiscard]] float               repeatDelay()  const { return m_repeatDelay;  }
    [[nodiscard]] bool                isEnabled()    const { return m_isEnabled;    }

private:
    uint32_t          m_id;
    std::string       m_name;
    InputActType      m_type;
    InputActionTrigger m_trigger;
    bool              m_consumeInput = true;
    float             m_repeatDelay  = 0.5f;
    bool              m_isEnabled    = true;
};

class InputActionEditor {
public:
    void setIsShowDisabled(bool v)       { m_isShowDisabled    = v; }
    void setIsGroupByType(bool v)        { m_isGroupByType     = v; }
    void setDefaultRepeatDelay(float v)  { m_defaultRepeatDelay = v; }

    bool addAction(const InputActionDef& a) {
        for (auto& x : m_actions) if (x.id() == a.id()) return false;
        m_actions.push_back(a); return true;
    }
    bool removeAction(uint32_t id) {
        auto it = std::find_if(m_actions.begin(), m_actions.end(),
            [&](const InputActionDef& a){ return a.id() == id; });
        if (it == m_actions.end()) return false;
        m_actions.erase(it); return true;
    }
    [[nodiscard]] InputActionDef* findAction(uint32_t id) {
        for (auto& a : m_actions) if (a.id() == id) return &a;
        return nullptr;
    }

    [[nodiscard]] bool  isShowDisabled()     const { return m_isShowDisabled;     }
    [[nodiscard]] bool  isGroupByType()      const { return m_isGroupByType;      }
    [[nodiscard]] float defaultRepeatDelay() const { return m_defaultRepeatDelay; }
    [[nodiscard]] size_t actionCount()       const { return m_actions.size();     }

    [[nodiscard]] size_t countByType(InputActType t) const {
        size_t n = 0; for (auto& a : m_actions) if (a.type() == t) ++n; return n;
    }
    [[nodiscard]] size_t countByTrigger(InputActionTrigger t) const {
        size_t n = 0; for (auto& a : m_actions) if (a.trigger() == t) ++n; return n;
    }
    [[nodiscard]] size_t countEnabled() const {
        size_t n = 0; for (auto& a : m_actions) if (a.isEnabled()) ++n; return n;
    }

private:
    std::vector<InputActionDef> m_actions;
    bool  m_isShowDisabled     = false;
    bool  m_isGroupByType      = true;
    float m_defaultRepeatDelay = 0.3f;
};

} // namespace NF
