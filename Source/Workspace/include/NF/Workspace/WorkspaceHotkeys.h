#pragma once
// NF::Workspace — Phase 23: Workspace Hotkey Manager
//
// Workspace-level keyboard shortcut management:
//   ModifierFlags  — bitmask enum for modifier keys (Ctrl/Alt/Shift/Meta)
//   HotkeyChord    — modifier+key string chord; toString(), isValid(), equality
//   HotkeyBinding  — chord → command id mapping, scoped and enable-toggled
//   HotkeyConflict — overlapping binding pair within the same scope
//   HotkeyManager  — binding registry with conflict detection and observers

#include <algorithm>
#include <cstdint>
#include <functional>
#include <sstream>
#include <string>
#include <vector>

namespace NF {

// ═════════════════════════════════════════════════════════════════
// ModifierFlags — bitmask for keyboard modifier keys
// ═════════════════════════════════════════════════════════════════

enum class ModifierFlags : uint8_t {
    None  = 0x00,
    Ctrl  = 0x01,
    Alt   = 0x02,
    Shift = 0x04,
    Meta  = 0x08,
};

inline ModifierFlags operator|(ModifierFlags a, ModifierFlags b) {
    return static_cast<ModifierFlags>(static_cast<uint8_t>(a) | static_cast<uint8_t>(b));
}
inline ModifierFlags operator&(ModifierFlags a, ModifierFlags b) {
    return static_cast<ModifierFlags>(static_cast<uint8_t>(a) & static_cast<uint8_t>(b));
}
inline bool hasModifier(ModifierFlags flags, ModifierFlags check) {
    return (static_cast<uint8_t>(flags) & static_cast<uint8_t>(check)) != 0;
}

inline std::string modifierFlagsString(ModifierFlags f) {
    if (f == ModifierFlags::None) return "None";
    std::string s;
    if (hasModifier(f, ModifierFlags::Ctrl))  { if (!s.empty()) s += "+"; s += "Ctrl"; }
    if (hasModifier(f, ModifierFlags::Alt))   { if (!s.empty()) s += "+"; s += "Alt";  }
    if (hasModifier(f, ModifierFlags::Shift)) { if (!s.empty()) s += "+"; s += "Shift";}
    if (hasModifier(f, ModifierFlags::Meta))  { if (!s.empty()) s += "+"; s += "Meta"; }
    return s;
}

// ═════════════════════════════════════════════════════════════════
// HotkeyChord — modifier + key string
// ═════════════════════════════════════════════════════════════════

struct HotkeyChord {
    ModifierFlags modifiers = ModifierFlags::None;
    std::string   key;

    bool isValid() const { return !key.empty(); }

    std::string toString() const {
        if (modifiers == ModifierFlags::None) return key;
        return modifierFlagsString(modifiers) + "+" + key;
    }

    bool operator==(const HotkeyChord& o) const {
        return modifiers == o.modifiers && key == o.key;
    }
    bool operator!=(const HotkeyChord& o) const { return !(*this == o); }
};

// ═════════════════════════════════════════════════════════════════
// HotkeyBinding — chord → command mapping with scope
// ═════════════════════════════════════════════════════════════════

struct HotkeyBinding {
    std::string   id;         // unique binding id
    HotkeyChord   chord;
    std::string   commandId;  // target command
    std::string   scopeId;    // "" = global
    bool          enabled = true;

    bool isValid() const {
        return !id.empty() && chord.isValid() && !commandId.empty();
    }

    bool operator==(const HotkeyBinding& o) const { return id == o.id; }
};

// ═════════════════════════════════════════════════════════════════
// HotkeyConflict — two bindings sharing a chord within a scope
// ═════════════════════════════════════════════════════════════════

struct HotkeyConflict {
    std::string bindingIdA;
    std::string bindingIdB;
    HotkeyChord chord;
    std::string scopeId;

    bool isValid() const {
        return !bindingIdA.empty() && !bindingIdB.empty() && chord.isValid();
    }
};

// ═════════════════════════════════════════════════════════════════
// HotkeyManager — binding registry with conflict detection
// ═════════════════════════════════════════════════════════════════

class HotkeyManager {
public:
    using Observer = std::function<void(const HotkeyBinding&)>;

    static constexpr int MAX_BINDINGS  = 512;
    static constexpr int MAX_OBSERVERS = 16;

    // Registration ─────────────────────────────────────────────

    bool registerBinding(const HotkeyBinding& binding) {
        if (!binding.isValid()) return false;
        if (findById(binding.id)) return false;
        if ((int)m_bindings.size() >= MAX_BINDINGS) return false;
        m_bindings.push_back(binding);
        return true;
    }

    bool unregisterBinding(const std::string& id) {
        auto it = std::find_if(m_bindings.begin(), m_bindings.end(),
            [&](const HotkeyBinding& b) { return b.id == id; });
        if (it == m_bindings.end()) return false;
        m_bindings.erase(it);
        return true;
    }

    bool isRegistered(const std::string& id) const {
        return findById(id) != nullptr;
    }

    const HotkeyBinding* findById(const std::string& id) const {
        for (auto& b : m_bindings)
            if (b.id == id) return &b;
        return nullptr;
    }

    // Lookup ───────────────────────────────────────────────────

    const HotkeyBinding* findByChord(const HotkeyChord& chord,
                                      const std::string& scopeId = "") const {
        // Prefer scope-exact match over global
        for (auto& b : m_bindings)
            if (b.enabled && b.chord == chord && b.scopeId == scopeId)
                return &b;
        // Fall back to global bindings when searching a specific scope
        if (!scopeId.empty()) {
            for (auto& b : m_bindings)
                if (b.enabled && b.chord == chord && b.scopeId.empty())
                    return &b;
        }
        return nullptr;
    }

    std::vector<const HotkeyBinding*> findByCommand(const std::string& commandId) const {
        std::vector<const HotkeyBinding*> result;
        for (auto& b : m_bindings)
            if (b.commandId == commandId)
                result.push_back(&b);
        return result;
    }

    // Conflict detection ───────────────────────────────────────

    std::vector<HotkeyConflict> detectConflicts() const {
        std::vector<HotkeyConflict> conflicts;
        for (int i = 0; i < (int)m_bindings.size(); ++i) {
            for (int j = i + 1; j < (int)m_bindings.size(); ++j) {
                auto& a = m_bindings[i];
                auto& b = m_bindings[j];
                if (a.chord == b.chord && a.scopeId == b.scopeId) {
                    conflicts.push_back({a.id, b.id, a.chord, a.scopeId});
                }
            }
        }
        return conflicts;
    }

    // Enable / disable ─────────────────────────────────────────

    bool enableBinding(const std::string& id) {
        for (auto& b : m_bindings)
            if (b.id == id) { b.enabled = true; return true; }
        return false;
    }

    bool disableBinding(const std::string& id) {
        for (auto& b : m_bindings)
            if (b.id == id) { b.enabled = false; return true; }
        return false;
    }

    // Activate (dispatch) ──────────────────────────────────────

    bool activate(const HotkeyChord& chord, const std::string& scopeId = "") {
        const HotkeyBinding* b = findByChord(chord, scopeId);
        if (!b) return false;
        notifyObservers(*b);
        return true;
    }

    // Bulk access ──────────────────────────────────────────────

    std::vector<std::string> allBindingIds() const {
        std::vector<std::string> ids;
        ids.reserve(m_bindings.size());
        for (auto& b : m_bindings) ids.push_back(b.id);
        return ids;
    }

    int bindingCount() const { return (int)m_bindings.size(); }

    void clear() { m_bindings.clear(); }

    // Observers ────────────────────────────────────────────────

    uint32_t addObserver(Observer cb) {
        if (!cb || (int)m_observers.size() >= MAX_OBSERVERS) return 0;
        uint32_t id = ++m_nextObserverId;
        m_observers.push_back({id, std::move(cb)});
        return id;
    }

    void removeObserver(uint32_t id) {
        m_observers.erase(
            std::remove_if(m_observers.begin(), m_observers.end(),
                [id](const ObserverEntry& e) { return e.id == id; }),
            m_observers.end());
    }

    void clearObservers() { m_observers.clear(); }

private:
    struct ObserverEntry { uint32_t id; Observer cb; };

    void notifyObservers(const HotkeyBinding& b) {
        for (auto& e : m_observers) e.cb(b);
    }

    std::vector<HotkeyBinding>  m_bindings;
    uint32_t                    m_nextObserverId = 0;
    std::vector<ObserverEntry>  m_observers;
};

} // namespace NF
