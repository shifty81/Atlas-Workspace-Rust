#pragma once
// NF::Editor — Key binding editor v1: key mapping and chord management
#include "NF/Core/Core.h"
#include <algorithm>
#include <cstdint>
#include <functional>
#include <string>
#include <vector>

namespace NF {

enum class Kbev1KeyCategory : uint8_t { Movement, Combat, UI, Camera, Debug, Custom };
enum class Kbev1ChordState  : uint8_t { Unbound, Bound, Conflicting, Disabled };

inline const char* kbev1KeyCategoryName(Kbev1KeyCategory c) {
    switch (c) {
        case Kbev1KeyCategory::Movement: return "Movement";
        case Kbev1KeyCategory::Combat:   return "Combat";
        case Kbev1KeyCategory::UI:       return "UI";
        case Kbev1KeyCategory::Camera:   return "Camera";
        case Kbev1KeyCategory::Debug:    return "Debug";
        case Kbev1KeyCategory::Custom:   return "Custom";
    }
    return "Unknown";
}

inline const char* kbev1ChordStateName(Kbev1ChordState s) {
    switch (s) {
        case Kbev1ChordState::Unbound:     return "Unbound";
        case Kbev1ChordState::Bound:       return "Bound";
        case Kbev1ChordState::Conflicting: return "Conflicting";
        case Kbev1ChordState::Disabled:    return "Disabled";
    }
    return "Unknown";
}

struct Kbev1KeyBinding {
    uint64_t          id       = 0;
    std::string       action;
    std::string       keyChord;
    Kbev1KeyCategory  category = Kbev1KeyCategory::Custom;
    Kbev1ChordState   state    = Kbev1ChordState::Unbound;

    [[nodiscard]] bool isValid()       const { return id != 0 && !action.empty(); }
    [[nodiscard]] bool isBound()       const { return state == Kbev1ChordState::Bound; }
    [[nodiscard]] bool isConflicting() const { return state == Kbev1ChordState::Conflicting; }
    [[nodiscard]] bool isDisabled()    const { return state == Kbev1ChordState::Disabled; }
};

struct Kbev1BindingSet {
    uint64_t    id   = 0;
    std::string name;

    [[nodiscard]] bool isValid() const { return id != 0 && !name.empty(); }
};

using Kbev1ChangeCallback = std::function<void(uint64_t)>;

class KeyBindingEditorV1 {
public:
    static constexpr size_t MAX_BINDINGS    = 512;
    static constexpr size_t MAX_BINDING_SETS = 64;

    bool addBinding(const Kbev1KeyBinding& binding) {
        if (!binding.isValid()) return false;
        for (const auto& b : m_bindings) if (b.id == binding.id) return false;
        if (m_bindings.size() >= MAX_BINDINGS) return false;
        m_bindings.push_back(binding);
        if (m_onChange) m_onChange(binding.id);
        return true;
    }

    bool removeBinding(uint64_t id) {
        for (auto it = m_bindings.begin(); it != m_bindings.end(); ++it) {
            if (it->id == id) { m_bindings.erase(it); return true; }
        }
        return false;
    }

    [[nodiscard]] Kbev1KeyBinding* findBinding(uint64_t id) {
        for (auto& b : m_bindings) if (b.id == id) return &b;
        return nullptr;
    }

    bool addBindingSet(const Kbev1BindingSet& set) {
        if (!set.isValid()) return false;
        for (const auto& s : m_sets) if (s.id == set.id) return false;
        if (m_sets.size() >= MAX_BINDING_SETS) return false;
        m_sets.push_back(set);
        return true;
    }

    bool removeBindingSet(uint64_t id) {
        for (auto it = m_sets.begin(); it != m_sets.end(); ++it) {
            if (it->id == id) { m_sets.erase(it); return true; }
        }
        return false;
    }

    [[nodiscard]] size_t bindingCount()    const { return m_bindings.size(); }
    [[nodiscard]] size_t bindingSetCount() const { return m_sets.size(); }

    [[nodiscard]] size_t boundCount() const {
        size_t c = 0; for (const auto& b : m_bindings) if (b.isBound()) ++c; return c;
    }
    [[nodiscard]] size_t conflictingCount() const {
        size_t c = 0; for (const auto& b : m_bindings) if (b.isConflicting()) ++c; return c;
    }
    [[nodiscard]] size_t countByCategory(Kbev1KeyCategory cat) const {
        size_t c = 0; for (const auto& b : m_bindings) if (b.category == cat) ++c; return c;
    }

    void setOnChange(Kbev1ChangeCallback cb) { m_onChange = std::move(cb); }

private:
    std::vector<Kbev1KeyBinding>  m_bindings;
    std::vector<Kbev1BindingSet>  m_sets;
    Kbev1ChangeCallback           m_onChange;
};

} // namespace NF
