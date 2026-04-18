#pragma once
// NF::Editor — Shortcut context + manager
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

enum class ShortcutCategory : uint8_t {
    File, Edit, View, Navigate, Select, Debug, Tool, Custom
};

[[nodiscard]] inline const char* shortcutCategoryName(ShortcutCategory c) {
    switch (c) {
        case ShortcutCategory::File:     return "File";
        case ShortcutCategory::Edit:     return "Edit";
        case ShortcutCategory::View:     return "View";
        case ShortcutCategory::Navigate: return "Navigate";
        case ShortcutCategory::Select:   return "Select";
        case ShortcutCategory::Debug:    return "Debug";
        case ShortcutCategory::Tool:     return "Tool";
        case ShortcutCategory::Custom:   return "Custom";
    }
    return "Unknown";
}

enum class ShortcutState : uint8_t {
    Inactive, Active, Pressed, Blocked
};

[[nodiscard]] inline const char* shortcutStateName(ShortcutState s) {
    switch (s) {
        case ShortcutState::Inactive: return "Inactive";
        case ShortcutState::Active:   return "Active";
        case ShortcutState::Pressed:  return "Pressed";
        case ShortcutState::Blocked:  return "Blocked";
    }
    return "Unknown";
}

struct ShortcutBinding {
    std::string      id;
    std::string      name;
    ShortcutCategory category  = ShortcutCategory::File;
    std::string      key;
    uint8_t          modifiers = 0;
    bool             enabled   = true;
    ShortcutState    state     = ShortcutState::Inactive;

    void enable()   { enabled = true; }
    void disable()  { enabled = false; }
    void trigger()  { state = ShortcutState::Pressed; }
    void reset()    { state = ShortcutState::Inactive; }

    [[nodiscard]] bool isEnabled()  const { return enabled; }
    [[nodiscard]] bool isActive()   const { return state == ShortcutState::Pressed; }
    [[nodiscard]] bool hasKey()     const { return !key.empty(); }
};

class ShortcutContext {
public:
    explicit ShortcutContext(const std::string& name) : m_name(name) {}

    [[nodiscard]] const std::string& name()         const { return m_name; }
    [[nodiscard]] size_t             bindingCount() const { return m_bindings.size(); }

    bool addBinding(const ShortcutBinding& b) {
        for (auto& existing : m_bindings) if (existing.id == b.id) return false;
        m_bindings.push_back(b);
        return true;
    }

    bool removeBinding(const std::string& id) {
        for (auto it = m_bindings.begin(); it != m_bindings.end(); ++it) {
            if (it->id == id) { m_bindings.erase(it); return true; }
        }
        return false;
    }

    [[nodiscard]] ShortcutBinding* findBinding(const std::string& id) {
        for (auto& b : m_bindings) if (b.id == id) return &b;
        return nullptr;
    }

    void enableAll()  { for (auto& b : m_bindings) b.enable(); }
    void disableAll() { for (auto& b : m_bindings) b.disable(); }

    [[nodiscard]] size_t activeCount() const {
        size_t c = 0;
        for (auto& b : m_bindings) if (b.isEnabled()) c++;
        return c;
    }

    [[nodiscard]] const std::vector<ShortcutBinding>& bindings() const { return m_bindings; }

private:
    std::string                  m_name;
    std::vector<ShortcutBinding> m_bindings;
};

class ShortcutManager {
public:
    static constexpr size_t MAX_CONTEXTS = 32;

    ShortcutContext* createContext(const std::string& name) {
        if (m_contexts.size() >= MAX_CONTEXTS) return nullptr;
        for (auto& c : m_contexts) if (c.name() == name) return nullptr;
        m_contexts.emplace_back(name);
        return &m_contexts.back();
    }

    bool removeContext(const std::string& name) {
        for (auto it = m_contexts.begin(); it != m_contexts.end(); ++it) {
            if (it->name() == name) {
                if (m_activeName == name) m_activeName.clear();
                m_contexts.erase(it);
                return true;
            }
        }
        return false;
    }

    [[nodiscard]] ShortcutContext* findContext(const std::string& name) {
        for (auto& c : m_contexts) if (c.name() == name) return &c;
        return nullptr;
    }

    bool setActiveContext(const std::string& name) {
        if (!findContext(name)) return false;
        m_activeName = name;
        return true;
    }

    [[nodiscard]] ShortcutContext* activeContext() { return findContext(m_activeName); }
    [[nodiscard]] const std::string& activeName()  const { return m_activeName; }
    [[nodiscard]] bool               hasActive()   const { return !m_activeName.empty(); }
    [[nodiscard]] size_t             contextCount() const { return m_contexts.size(); }

private:
    std::vector<ShortcutContext> m_contexts;
    std::string                  m_activeName;
};

// ============================================================
// S24 — Notification System
// ============================================================


} // namespace NF
