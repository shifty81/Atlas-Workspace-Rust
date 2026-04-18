#pragma once
// NF::Editor — Menu bar and status bar
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

struct MenuItem {
    std::string name;
    std::string command;    // command name to execute via EditorCommandRegistry
    std::string hotkey;
    bool enabled = true;
    bool isSeparator = false;
    std::vector<MenuItem> children;

    static MenuItem separator() {
        MenuItem m;
        m.isSeparator = true;
        return m;
    }
};

struct MenuCategory {
    std::string name;
    std::vector<MenuItem> items;

    void addItem(const std::string& itemName, const std::string& command,
                 const std::string& hotkey = "", bool enabled = true) {
        MenuItem m;
        m.name = itemName;
        m.command = command;
        m.hotkey = hotkey;
        m.enabled = enabled;
        items.push_back(std::move(m));
    }

    void addSeparator() { items.push_back(MenuItem::separator()); }
};

class MenuBar {
public:
    MenuCategory& addCategory(const std::string& name) {
        m_categories.push_back(MenuCategory{name, {}});
        return m_categories.back();
    }

    MenuCategory* findCategory(const std::string& name) {
        for (auto& c : m_categories) if (c.name == name) return &c;
        return nullptr;
    }

    const MenuCategory* findCategory(const std::string& name) const {
        for (auto& c : m_categories) if (c.name == name) return &c;
        return nullptr;
    }

    const std::vector<MenuCategory>& categories() const { return m_categories; }
    size_t categoryCount() const { return m_categories.size(); }

private:
    std::vector<MenuCategory> m_categories;
};

// ── EditorStatusBar ──────────────────────────────────────────────

struct StatusBarState {
    std::string modeName;
    std::string worldPath;
    bool isDirty = false;
    int selectionCount = 0;
    float fps = 0.f;
    std::string statusMessage;
};

class EditorStatusBar {
public:
    void update(const std::string& mode, const std::string& worldPath,
                bool dirty, int selection, float fps,
                const std::string& msg = "") {
        m_state.modeName = mode;
        m_state.worldPath = worldPath;
        m_state.isDirty = dirty;
        m_state.selectionCount = selection;
        m_state.fps = fps;
        m_state.statusMessage = msg;
    }

    const StatusBarState& state() const { return m_state; }

    std::string buildText() const {
        std::string s = m_state.modeName;
        if (!m_state.worldPath.empty()) {
            s += "  |  " + m_state.worldPath;
            if (m_state.isDirty) s += " *";
        }
        if (m_state.selectionCount > 0)
            s += "  |  " + std::to_string(m_state.selectionCount) + " selected";
        s += "  |  " + std::to_string(static_cast<int>(m_state.fps)) + " FPS";
        if (!m_state.statusMessage.empty())
            s += "  |  " + m_state.statusMessage;
        return s;
    }

private:
    StatusBarState m_state;
};

// ── Notification System ──────────────────────────────────────────


} // namespace NF
