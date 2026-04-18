#pragma once
// NF::Workspace — Phase 51: Workspace Keymap System
//
// Layered keyboard-shortcut configuration. UI-agnostic data model only.
//
//   KeyModifiers     — bitmask: Ctrl/Shift/Alt/Meta
//   KeyChord         — key name + modifiers; toString(); isValid(); equality
//   KeyAction        — id + chord + description + context (tool/panel id or "");
//                      isValid(); equality by id
//   KeymapLayer      — named layer + priority; addAction/removeAction/findAction/
//                      findByChord/contains; MAX_ACTIONS=128; enabled flag
//   KeymapManager    — ordered layer stack (highest priority first);
//                      addLayer/removeLayer/findLayer/hasLayer; lookup(chord) →
//                      first matching KeyAction* across enabled layers;
//                      lookupAll(chord) → all matches across all enabled layers;
//                      registerAction/unregisterAction (global default layer);
//                      observer callbacks on any change (MAX_OBSERVERS=16);
//                      serialize() / deserialize() — tab-separated text

#include <algorithm>
#include <cstdint>
#include <functional>
#include <string>
#include <vector>

namespace NF {

// ═════════════════════════════════════════════════════════════════════════
// KeyModifiers — bitmask
// ═════════════════════════════════════════════════════════════════════════

struct KeyModifiers {
    static constexpr uint8_t None  = 0;
    static constexpr uint8_t Ctrl  = 1 << 0;
    static constexpr uint8_t Shift = 1 << 1;
    static constexpr uint8_t Alt   = 1 << 2;
    static constexpr uint8_t Meta  = 1 << 3;

    uint8_t bits = None;

    KeyModifiers() = default;
    explicit KeyModifiers(uint8_t b) : bits(b) {}

    [[nodiscard]] bool hasCtrl()  const { return (bits & Ctrl)  != 0; }
    [[nodiscard]] bool hasShift() const { return (bits & Shift) != 0; }
    [[nodiscard]] bool hasAlt()   const { return (bits & Alt)   != 0; }
    [[nodiscard]] bool hasMeta()  const { return (bits & Meta)  != 0; }
    [[nodiscard]] bool none()     const { return bits == None; }

    bool operator==(const KeyModifiers& o) const { return bits == o.bits; }
    bool operator!=(const KeyModifiers& o) const { return bits != o.bits; }

    [[nodiscard]] std::string toString() const {
        std::string out;
        if (hasCtrl())  out += "Ctrl+";
        if (hasShift()) out += "Shift+";
        if (hasAlt())   out += "Alt+";
        if (hasMeta())  out += "Meta+";
        return out;
    }
};

// ═════════════════════════════════════════════════════════════════════════
// KeyChord
// ═════════════════════════════════════════════════════════════════════════

struct KeyChord {
    std::string  key;          // e.g. "P", "F5", "Delete", "Tab"
    KeyModifiers modifiers;

    KeyChord() = default;
    KeyChord(const std::string& k, KeyModifiers m = KeyModifiers{}) : key(k), modifiers(m) {}

    [[nodiscard]] bool isValid() const { return !key.empty(); }

    [[nodiscard]] std::string toString() const {
        return modifiers.toString() + key;
    }

    bool operator==(const KeyChord& o) const {
        return key == o.key && modifiers == o.modifiers;
    }
    bool operator!=(const KeyChord& o) const { return !(*this == o); }
};

// ═════════════════════════════════════════════════════════════════════════
// KeyAction
// ═════════════════════════════════════════════════════════════════════════

struct KeyAction {
    std::string id;
    KeyChord    chord;
    std::string description;
    std::string context;       // tool/panel id or "" = global

    [[nodiscard]] bool isValid()  const { return !id.empty() && chord.isValid(); }

    bool operator==(const KeyAction& o) const { return id == o.id; }
    bool operator!=(const KeyAction& o) const { return id != o.id; }
};

// ═════════════════════════════════════════════════════════════════════════
// KeymapLayer — named layer with priority
// ═════════════════════════════════════════════════════════════════════════

class KeymapLayer {
public:
    static constexpr size_t MAX_ACTIONS = 128;

    KeymapLayer() = default;
    KeymapLayer(const std::string& id, const std::string& name, int priority)
        : m_id(id), m_name(name), m_priority(priority) {}

    [[nodiscard]] const std::string& id()       const { return m_id; }
    [[nodiscard]] const std::string& name()     const { return m_name; }
    [[nodiscard]] int                priority() const { return m_priority; }
    [[nodiscard]] bool               enabled()  const { return m_enabled; }
    [[nodiscard]] size_t             count()    const { return m_actions.size(); }
    [[nodiscard]] bool               empty()    const { return m_actions.empty(); }

    void setEnabled(bool e) { m_enabled = e; }

    // Add action. Rejects invalid, duplicate id, or at capacity.
    bool addAction(const KeyAction& action) {
        if (!action.isValid()) return false;
        if (m_actions.size() >= MAX_ACTIONS) return false;
        for (const auto& a : m_actions)
            if (a.id == action.id) return false;
        m_actions.push_back(action);
        return true;
    }

    bool removeAction(const std::string& id) {
        auto it = std::find_if(m_actions.begin(), m_actions.end(),
                               [&](const KeyAction& a){ return a.id == id; });
        if (it == m_actions.end()) return false;
        m_actions.erase(it);
        return true;
    }

    [[nodiscard]] const KeyAction* findAction(const std::string& id) const {
        for (const auto& a : m_actions)
            if (a.id == id) return &a;
        return nullptr;
    }

    // First action whose chord matches (within optional context).
    [[nodiscard]] const KeyAction* findByChord(const KeyChord& chord,
                                               const std::string& context = "") const {
        for (const auto& a : m_actions) {
            if (a.chord != chord) continue;
            // Match if action is global or context matches
            if (a.context.empty() || context.empty() || a.context == context)
                return &a;
        }
        return nullptr;
    }

    [[nodiscard]] bool contains(const std::string& id) const {
        return findAction(id) != nullptr;
    }

    [[nodiscard]] const std::vector<KeyAction>& actions() const { return m_actions; }
    void clear() { m_actions.clear(); }

private:
    std::string             m_id;
    std::string             m_name;
    int                     m_priority = 0;
    bool                    m_enabled  = true;
    std::vector<KeyAction>  m_actions;
};

// ═════════════════════════════════════════════════════════════════════════
// KeymapManager
// ═════════════════════════════════════════════════════════════════════════

using KeymapObserver = std::function<void()>;

class KeymapManager {
public:
    static constexpr size_t MAX_LAYERS    = 16;
    static constexpr size_t MAX_OBSERVERS = 16;

    KeymapManager() {
        // Create a built-in "default" layer at lowest priority
        m_layers.emplace_back("default", "Default", 0);
    }

    // ── Layer management ──────────────────────────────────────────

    // Add a new layer. Returns nullptr if id is duplicate or at capacity.
    KeymapLayer* addLayer(const std::string& id, const std::string& name,
                          int priority) {
        if (id.empty() || m_layers.size() >= MAX_LAYERS) return nullptr;
        for (const auto& l : m_layers)
            if (l.id() == id) return nullptr;
        m_layers.emplace_back(id, name, priority);
        sortLayers();
        notify();
        return findLayer(id);
    }

    bool removeLayer(const std::string& id) {
        if (id == "default") return false; // cannot remove default
        auto it = std::find_if(m_layers.begin(), m_layers.end(),
                               [&](const KeymapLayer& l){ return l.id() == id; });
        if (it == m_layers.end()) return false;
        m_layers.erase(it);
        notify();
        return true;
    }

    [[nodiscard]] KeymapLayer* findLayer(const std::string& id) {
        for (auto& l : m_layers)
            if (l.id() == id) return &l;
        return nullptr;
    }

    [[nodiscard]] const KeymapLayer* findLayer(const std::string& id) const {
        for (const auto& l : m_layers)
            if (l.id() == id) return &l;
        return nullptr;
    }

    [[nodiscard]] bool   hasLayer(const std::string& id) const {
        return findLayer(id) != nullptr;
    }
    [[nodiscard]] size_t layerCount() const { return m_layers.size(); }

    [[nodiscard]] const std::vector<KeymapLayer>& layers() const { return m_layers; }

    // Enable/disable a layer.
    bool setLayerEnabled(const std::string& id, bool enabled) {
        auto* l = findLayer(id);
        if (!l) return false;
        l->setEnabled(enabled);
        notify();
        return true;
    }

    // ── Action management on default layer ─────────────────────────

    bool registerAction(const KeyAction& action) {
        bool ok = defaultLayer().addAction(action);
        if (ok) notify();
        return ok;
    }

    bool unregisterAction(const std::string& id) {
        bool ok = defaultLayer().removeAction(id);
        if (ok) notify();
        return ok;
    }

    // ── Lookup ────────────────────────────────────────────────────

    // First matching action across all enabled layers (highest priority first).
    [[nodiscard]] const KeyAction* lookup(const KeyChord& chord,
                                          const std::string& context = "") const {
        for (const auto& layer : m_layers) {
            if (!layer.enabled()) continue;
            if (const auto* a = layer.findByChord(chord, context)) return a;
        }
        return nullptr;
    }

    // All matching actions across all enabled layers.
    [[nodiscard]] std::vector<const KeyAction*> lookupAll(const KeyChord& chord,
                                                          const std::string& context = "") const {
        std::vector<const KeyAction*> out;
        for (const auto& layer : m_layers) {
            if (!layer.enabled()) continue;
            for (const auto& a : layer.actions()) {
                if (a.chord != chord) continue;
                if (a.context.empty() || context.empty() || a.context == context)
                    out.push_back(&a);
            }
        }
        return out;
    }

    // Find any action by id across all layers.
    [[nodiscard]] const KeyAction* findAction(const std::string& id) const {
        for (const auto& layer : m_layers) {
            if (const auto* a = layer.findAction(id)) return a;
        }
        return nullptr;
    }

    // ── Serialization ─────────────────────────────────────────────
    // Format per action (one line):
    //   <layerId>\t<actionId>\t<context>\t<modBits>\t<key>\t<description>

    [[nodiscard]] std::string serialize() const {
        std::string out;
        for (const auto& layer : m_layers) {
            for (const auto& a : layer.actions()) {
                out += layer.id()                           + "\t";
                out += a.id                                 + "\t";
                out += a.context                            + "\t";
                out += std::to_string(a.chord.modifiers.bits) + "\t";
                out += a.chord.key                          + "\t";
                out += a.description                        + "\n";
            }
        }
        return out;
    }

    // Deserialize: clears all non-default layers, replaces default layer content.
    // Returns count of actions loaded.
    int deserialize(const std::string& text) {
        // Clear non-default layers and reset default
        m_layers.erase(
            std::remove_if(m_layers.begin(), m_layers.end(),
                           [](const KeymapLayer& l){ return l.id() != "default"; }),
            m_layers.end());
        defaultLayer().clear();

        int loaded = 0;
        size_t pos = 0;
        while (pos < text.size()) {
            size_t nl   = text.find('\n', pos);
            std::string line = text.substr(pos, nl == std::string::npos
                                              ? text.size() - pos : nl - pos);
            pos = (nl == std::string::npos) ? text.size() : nl + 1;
            if (line.empty()) continue;

            // Split on tab, 6 fields
            std::vector<std::string> f;
            size_t start = 0;
            for (int i = 0; i < 5; ++i) {
                size_t tab = line.find('\t', start);
                if (tab == std::string::npos) break;
                f.push_back(line.substr(start, tab - start));
                start = tab + 1;
            }
            f.push_back(line.substr(start));
            if (f.size() != 6) continue;

            const std::string& layerId = f[0];
            KeymapLayer* layer = findLayer(layerId);
            if (!layer) {
                // Create layer with priority = current count
                layer = addLayer(layerId, layerId,
                                 static_cast<int>(m_layers.size()));
                if (!layer) continue;
            }

            KeyAction action;
            action.id          = f[1];
            action.context     = f[2];
            action.chord.modifiers = KeyModifiers(static_cast<uint8_t>(std::stoul(f[3])));
            action.chord.key   = f[4];
            action.description = f[5];
            if (layer->addAction(action)) ++loaded;
        }
        return loaded;
    }

    // ── Observers ─────────────────────────────────────────────────

    bool addObserver(KeymapObserver obs) {
        if (m_observers.size() >= MAX_OBSERVERS) return false;
        m_observers.push_back(std::move(obs));
        return true;
    }

    void clearObservers() { m_observers.clear(); }

    void clear() {
        m_layers.clear();
        m_layers.emplace_back("default", "Default", 0);
        notify();
    }

private:
    void notify() {
        for (auto& obs : m_observers) obs();
    }

    void sortLayers() {
        // Sort descending by priority so highest priority is first in lookup
        std::stable_sort(m_layers.begin(), m_layers.end(),
                         [](const KeymapLayer& a, const KeymapLayer& b){
                             return a.priority() > b.priority();
                         });
    }

    KeymapLayer& defaultLayer() {
        for (auto& l : m_layers)
            if (l.id() == "default") return l;
        return m_layers.back(); // fallback (should not happen)
    }

    std::vector<KeymapLayer>    m_layers;
    std::vector<KeymapObserver> m_observers;
};

} // namespace NF
