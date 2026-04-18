#pragma once
// NF::Editor — Hotkey registry v1: binding management and dispatch
#include "NF/Core/Core.h"
#include <cstdint>
#include <functional>
#include <string>
#include <vector>

namespace NF {

enum class HkModifier : uint8_t {
    None, Ctrl, Shift, Alt, CtrlShift, CtrlAlt, ShiftAlt, CtrlShiftAlt
};

inline const char* hkModifierName(HkModifier m) {
    switch(m){
        case HkModifier::None:         return "None";
        case HkModifier::Ctrl:         return "Ctrl";
        case HkModifier::Shift:        return "Shift";
        case HkModifier::Alt:          return "Alt";
        case HkModifier::CtrlShift:    return "CtrlShift";
        case HkModifier::CtrlAlt:      return "CtrlAlt";
        case HkModifier::ShiftAlt:     return "ShiftAlt";
        case HkModifier::CtrlShiftAlt: return "CtrlShiftAlt";
    }
    return "Unknown";
}

struct HkBinding {
    uint32_t    id       = 0;
    std::string actionId;
    std::string keyCode;
    HkModifier  modifier = HkModifier::None;
    bool        enabled  = true;

    [[nodiscard]] bool isValid() const { return id != 0 && !actionId.empty() && !keyCode.empty(); }
    [[nodiscard]] std::string toString() const {
        std::string s = hkModifierName(modifier);
        if (modifier != HkModifier::None) s += "+";
        return s + keyCode;
    }
};

using HkDispatchCallback = std::function<void(const HkBinding&)>;

class HotkeyRegistryV1 {
public:
    static constexpr size_t MAX_BINDINGS = 256;

    bool addBinding(const HkBinding& b) {
        if (!b.isValid()) return false;
        if (m_bindings.size() >= MAX_BINDINGS) return false;
        for (const auto& x : m_bindings) if (x.id == b.id) return false;
        m_bindings.push_back(b);
        return true;
    }

    bool removeBinding(uint32_t id) {
        for (auto it = m_bindings.begin(); it != m_bindings.end(); ++it) {
            if (it->id == id) { m_bindings.erase(it); return true; }
        }
        return false;
    }

    const HkBinding* findByKey(const std::string& keyCode, HkModifier mod) const {
        for (const auto& b : m_bindings)
            if (b.enabled && b.keyCode == keyCode && b.modifier == mod) return &b;
        return nullptr;
    }

    const HkBinding* findByAction(const std::string& actionId) const {
        for (const auto& b : m_bindings)
            if (b.actionId == actionId) return &b;
        return nullptr;
    }

    bool dispatch(const std::string& keyCode, HkModifier mod) {
        const HkBinding* b = findByKey(keyCode, mod);
        if (!b) return false;
        if (m_onDispatch) m_onDispatch(*b);
        return true;
    }

    void setOnDispatch(HkDispatchCallback cb) { m_onDispatch = std::move(cb); }

    [[nodiscard]] size_t bindingCount() const { return m_bindings.size(); }

private:
    std::vector<HkBinding> m_bindings;
    HkDispatchCallback     m_onDispatch;
};

} // namespace NF
