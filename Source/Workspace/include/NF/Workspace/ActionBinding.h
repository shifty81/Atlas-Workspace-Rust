#pragma once
// NF::Workspace — ActionBinding: bind workspace commands to UI gestures.
//
// An ActionBinding maps a logical WorkspaceCommand to one or more UI trigger
// sources:
//   - KeyboardShortcut — modifier+key string (e.g. "Ctrl+S", "Shift+Z")
//   - ToolbarSlot      — toolbar id + slot index
//   - MenuItem         — menu path (e.g. "File/Save As...")
//
// ActionMap is the authoritative store of ActionBindings for a workspace
// session. It supports:
//   - Register/unregister bindings by command id
//   - Lookup command id by any gesture source
//   - Multiple bindings per command (e.g. Ctrl+Z *and* menu "Edit/Undo")
//   - Serialization to/from text (one binding record per line)
//
// ActionMap does NOT execute commands — callers look up the command id and
// dispatch it through CommandRegistry.

#include "NF/Workspace/WorkspaceCommand.h"
#include <algorithm>
#include <cstdint>
#include <sstream>
#include <string>
#include <vector>

namespace NF {

// ── Gesture Source ─────────────────────────────────────────────────

enum class GestureType : uint8_t {
    Keyboard,  // keyboard shortcut string
    Toolbar,   // toolbar id + slot index
    MenuItem,  // menu path string
};

inline const char* gestureTypeName(GestureType g) {
    switch (g) {
    case GestureType::Keyboard: return "keyboard";
    case GestureType::Toolbar:  return "toolbar";
    case GestureType::MenuItem: return "menu";
    }
    return "unknown";
}

// ── Action Binding ─────────────────────────────────────────────────
// One gesture-to-command binding.

struct ActionBinding {
    std::string commandId;   // the command this binding triggers
    GestureType gestureType;
    std::string gestureKey;  // shortcut string, menu path, or "toolbarId:slot"

    [[nodiscard]] bool isValid() const {
        return !commandId.empty() && !gestureKey.empty();
    }

    [[nodiscard]] bool operator==(const ActionBinding& o) const {
        return commandId == o.commandId
            && gestureType == o.gestureType
            && gestureKey == o.gestureKey;
    }
};

// ── Action Map ─────────────────────────────────────────────────────

class ActionMap {
public:
    static constexpr size_t MAX_BINDINGS = 512;

    // ── Register ──────────────────────────────────────────────────

    // Returns false if binding is invalid, duplicate, or map is full.
    bool addBinding(const ActionBinding& b) {
        if (!b.isValid()) return false;
        if (m_bindings.size() >= MAX_BINDINGS) return false;
        // Reject exact duplicates (same commandId + gestureType + gestureKey)
        for (const auto& existing : m_bindings)
            if (existing == b) return false;
        m_bindings.push_back(b);
        return true;
    }

    bool addKeyboardBinding(const std::string& commandId, const std::string& shortcut) {
        return addBinding({commandId, GestureType::Keyboard, shortcut});
    }

    bool addMenuBinding(const std::string& commandId, const std::string& menuPath) {
        return addBinding({commandId, GestureType::MenuItem, menuPath});
    }

    bool addToolbarBinding(const std::string& commandId,
                           const std::string& toolbarId, int slot) {
        return addBinding({commandId, GestureType::Toolbar,
                           toolbarId + ":" + std::to_string(slot)});
    }

    // ── Remove ────────────────────────────────────────────────────

    // Remove all bindings for a command.
    size_t removeBindingsForCommand(const std::string& commandId) {
        size_t before = m_bindings.size();
        m_bindings.erase(
            std::remove_if(m_bindings.begin(), m_bindings.end(),
                [&](const ActionBinding& b) { return b.commandId == commandId; }),
            m_bindings.end());
        return before - m_bindings.size();
    }

    // Remove one specific binding.
    bool removeBinding(const ActionBinding& b) {
        for (auto it = m_bindings.begin(); it != m_bindings.end(); ++it) {
            if (*it == b) { m_bindings.erase(it); return true; }
        }
        return false;
    }

    // ── Lookup ────────────────────────────────────────────────────

    // Returns the first command id bound to this exact gesture key and type.
    [[nodiscard]] std::string resolveGesture(GestureType type,
                                              const std::string& key) const {
        for (const auto& b : m_bindings)
            if (b.gestureType == type && b.gestureKey == key)
                return b.commandId;
        return {};
    }

    [[nodiscard]] std::string resolveKeyboard(const std::string& shortcut) const {
        return resolveGesture(GestureType::Keyboard, shortcut);
    }

    [[nodiscard]] std::string resolveMenu(const std::string& menuPath) const {
        return resolveGesture(GestureType::MenuItem, menuPath);
    }

    [[nodiscard]] std::string resolveToolbar(const std::string& toolbarId, int slot) const {
        return resolveGesture(GestureType::Toolbar, toolbarId + ":" + std::to_string(slot));
    }

    // All bindings for a given command.
    [[nodiscard]] std::vector<ActionBinding> bindingsForCommand(const std::string& id) const {
        std::vector<ActionBinding> out;
        for (const auto& b : m_bindings)
            if (b.commandId == id) out.push_back(b);
        return out;
    }

    // All bindings for a given gesture type.
    [[nodiscard]] std::vector<ActionBinding> bindingsByType(GestureType type) const {
        std::vector<ActionBinding> out;
        for (const auto& b : m_bindings)
            if (b.gestureType == type) out.push_back(b);
        return out;
    }

    [[nodiscard]] bool hasBinding(const std::string& commandId) const {
        for (const auto& b : m_bindings)
            if (b.commandId == commandId) return true;
        return false;
    }

    [[nodiscard]] size_t count() const { return m_bindings.size(); }
    [[nodiscard]] bool   empty() const { return m_bindings.empty(); }

    [[nodiscard]] const std::vector<ActionBinding>& all() const { return m_bindings; }

    // ── Serialization ─────────────────────────────────────────────
    // Wire format: one binding per line:
    //   <gestureTypeName>|<commandId>|<gestureKey>

    [[nodiscard]] std::string serialize() const {
        std::ostringstream out;
        for (const auto& b : m_bindings) {
            out << gestureTypeName(b.gestureType)
                << "|" << b.commandId
                << "|" << b.gestureKey
                << "\n";
        }
        return out.str();
    }

    static bool deserialize(const std::string& data, ActionMap& out) {
        if (data.empty()) return false;
        std::istringstream in(data);
        std::string line;
        size_t loaded = 0;
        while (std::getline(in, line)) {
            if (line.empty()) continue;
            // Split on '|' (exactly 2 separators expected)
            auto p1 = line.find('|');
            if (p1 == std::string::npos) continue;
            auto p2 = line.find('|', p1 + 1);
            if (p2 == std::string::npos) continue;

            std::string typeName  = line.substr(0, p1);
            std::string commandId = line.substr(p1 + 1, p2 - p1 - 1);
            std::string key       = line.substr(p2 + 1);

            GestureType type = GestureType::Keyboard;
            if      (typeName == "toolbar") type = GestureType::Toolbar;
            else if (typeName == "menu")    type = GestureType::MenuItem;

            ActionBinding b{commandId, type, key};
            if (b.isValid()) { out.addBinding(b); ++loaded; }
        }
        return loaded > 0;
    }

    void clear() { m_bindings.clear(); }

private:
    std::vector<ActionBinding> m_bindings;
};

} // namespace NF
