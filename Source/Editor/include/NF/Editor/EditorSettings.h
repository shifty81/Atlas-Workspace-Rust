#pragma once
// NF::Editor — Editor settings, hotkey dispatcher
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
#include "NF/Editor/EditorCamera.h"
#include "NF/Editor/ProjectServices.h"

namespace NF {

struct EditorSettings {
    bool        darkMode             = true;
    SnapSettings snap;
    bool        showGrid             = true;
    bool        showGizmos           = true;
    float       cameraSpeed          = 10.f;
    bool        autosave             = true;
    float       autosaveIntervalSecs = 300.f;
    int         undoHistorySize      = 100;
};

class EditorSettingsService {
public:
    void reset() { m_settings = EditorSettings{}; }

    EditorSettings& settings() { return m_settings; }
    const EditorSettings& settings() const { return m_settings; }

    void applyTheme(EditorTheme& theme) const {
        if (m_settings.darkMode) theme = EditorTheme::dark();
        else                     theme = EditorTheme::light();
    }

    void setDarkMode(bool dark) { m_settings.darkMode = dark; }
    void setShowGrid(bool show) { m_settings.showGrid = show; }
    void setSnapEnabled(bool on) { m_settings.snap.enabled = on; }
    void setCameraSpeed(float s) { m_settings.cameraSpeed = s; }

private:
    EditorSettings m_settings;
};

// ── Hotkey Dispatcher ────────────────────────────────────────────

struct HotkeyBinding {
    std::string hotkey;      // e.g. "Ctrl+Z", "F12", "Ctrl+Shift+S"
    std::string commandName;
};

class HotkeyDispatcher {
public:
    void bind(const std::string& hotkey, const std::string& commandName) {
        m_bindings.push_back({hotkey, commandName});
    }

    void unbind(const std::string& hotkey) {
        m_bindings.erase(
            std::remove_if(m_bindings.begin(), m_bindings.end(),
                           [&](const HotkeyBinding& b){ return b.hotkey == hotkey; }),
            m_bindings.end());
    }

    // Returns command name matched for a given hotkey string (empty if none)
    std::string findCommand(const std::string& hotkey) const {
        for (auto& b : m_bindings)
            if (b.hotkey == hotkey) return b.commandName;
        return {};
    }

    // Dispatch all matching bindings given a pressed hotkey string.
    // Returns number of commands dispatched.
    int dispatch(const std::string& hotkey, EditorCommandRegistry& commands) {
        int count = 0;
        for (auto& b : m_bindings) {
            if (b.hotkey == hotkey) {
                commands.executeCommand(b.commandName);
                ++count;
            }
        }
        return count;
    }

    void loadDefaults(EditorCommandRegistry& commands) {
        // Mirror the hotkeys already registered on commands
        auto names = commands.allCommandNames();
        for (auto& n : names) {
            if (auto* info = commands.findCommand(n)) {
                if (!info->hotkey.empty())
                    bind(info->hotkey, n);
            }
        }
    }

    const std::vector<HotkeyBinding>& bindings() const { return m_bindings; }
    int bindingCount() const { return (int)m_bindings.size(); }

private:
    std::vector<HotkeyBinding> m_bindings;
};

// ── Graph Editor Panel ───────────────────────────────────────────
// DEPRECATED: Use NF::UI::AtlasUI::GraphEditorPanel instead (U6).


} // namespace NF
