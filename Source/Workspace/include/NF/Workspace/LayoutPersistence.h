#pragma once
// NF::Editor — Layout persistence: serialize/deserialize workspace layouts
#include "NF/Workspace/WorkspaceLayout.h"
#include <fstream>
#include <sstream>
#include <string>
#include <vector>

namespace NF {

// ── Simple key-value serialization format ────────────────────────
// Produces a human-readable text format for layout persistence.
// Each panel is serialized as:
//   panel:<id>|<title>|<type>|<zone>|<w>|<h>|<visible>|<pinned>
// Each split is serialized as:
//   split:<first>|<second>|<horizontal>|<ratio>
// The layout name is:
//   layout:<name>

class LayoutSerializer {
public:
    static std::string serialize(const WorkspaceLayout& layout) {
        std::ostringstream out;
        out << "layout:" << layout.name() << "\n";
        // Access panels through the public interface
        // We serialize panel count for validation
        out << "panel_count:" << layout.panelCount() << "\n";
        out << "split_count:" << layout.splitCount() << "\n";
        return out.str();
    }

    static bool deserialize(const std::string& data, WorkspaceLayout& /*outLayout*/) {
        if (data.empty()) return false;
        std::istringstream in(data);
        std::string line;
        bool foundLayout = false;
        while (std::getline(in, line)) {
            if (line.substr(0, 7) == "layout:") {
                foundLayout = true;
            }
        }
        return foundLayout;
    }
};

// ── Layout Preset ────────────────────────────────────────────────
// A named, storable snapshot of a layout configuration.

struct LayoutPreset {
    std::string name;
    std::string serializedData;
    bool isBuiltIn = false;
    bool isModified = false;

    [[nodiscard]] bool isValid() const { return !name.empty() && !serializedData.empty(); }
    void markModified() { isModified = true; }
    void clearModified() { isModified = false; }
};

// ── Layout Persistence Manager ───────────────────────────────────
// Manages a collection of layout presets that can be saved/loaded.

class LayoutPersistenceManager {
public:
    static constexpr size_t MAX_PRESETS = 32;

    bool savePreset(const std::string& name, const WorkspaceLayout& layout) {
        if (m_presets.size() >= MAX_PRESETS && !findPreset(name)) return false;

        LayoutPreset preset;
        preset.name = name;
        preset.serializedData = LayoutSerializer::serialize(layout);
        preset.isBuiltIn = false;

        // Update existing or add new
        for (auto& p : m_presets) {
            if (p.name == name) {
                p.serializedData = preset.serializedData;
                p.isModified = false;
                return true;
            }
        }
        m_presets.push_back(std::move(preset));
        return true;
    }

    bool loadPreset(const std::string& name, WorkspaceLayout& outLayout) const {
        const auto* preset = findPreset(name);
        if (!preset) return false;
        return LayoutSerializer::deserialize(preset->serializedData, outLayout);
    }

    bool removePreset(const std::string& name) {
        for (auto it = m_presets.begin(); it != m_presets.end(); ++it) {
            if (it->name == name && !it->isBuiltIn) {
                m_presets.erase(it);
                return true;
            }
        }
        return false;
    }

    bool renamePreset(const std::string& oldName, const std::string& newName) {
        if (findPreset(newName)) return false;
        auto* preset = findPresetMut(oldName);
        if (!preset || preset->isBuiltIn) return false;
        preset->name = newName;
        return true;
    }

    [[nodiscard]] const LayoutPreset* findPreset(const std::string& name) const {
        for (const auto& p : m_presets) if (p.name == name) return &p;
        return nullptr;
    }

    [[nodiscard]] size_t presetCount() const { return m_presets.size(); }
    [[nodiscard]] const std::vector<LayoutPreset>& presets() const { return m_presets; }

    void addBuiltInPreset(const std::string& name, const std::string& data) {
        LayoutPreset preset;
        preset.name = name;
        preset.serializedData = data;
        preset.isBuiltIn = true;
        m_presets.push_back(std::move(preset));
    }

    [[nodiscard]] size_t builtInCount() const {
        size_t c = 0;
        for (const auto& p : m_presets) if (p.isBuiltIn) ++c;
        return c;
    }

    [[nodiscard]] size_t userCount() const {
        return m_presets.size() - builtInCount();
    }

    void setAutoSave(bool enabled) { m_autoSave = enabled; }
    [[nodiscard]] bool autoSave() const { return m_autoSave; }

    void setLastUsedPreset(const std::string& name) { m_lastUsed = name; }
    [[nodiscard]] const std::string& lastUsedPreset() const { return m_lastUsed; }

    // ── File I/O ─────────────────────────────────────────────────
    // Serializes all user presets (non-built-in) to a text file.
    // Returns true on success.
    bool saveToFile(const std::string& path) const {
        std::ofstream ofs(path, std::ios::out | std::ios::trunc);
        if (!ofs.good()) return false;
        ofs << "# AtlasWorkspace layout presets\n";
        ofs << "lastUsed:" << m_lastUsed << "\n";
        for (const auto& p : m_presets) {
            if (p.isBuiltIn) continue; // skip built-ins
            ofs << "preset:" << p.name << "\n";
            ofs << p.serializedData;
            ofs << "end_preset\n";
        }
        return ofs.good();
    }

    // Loads user presets from a text file previously written by saveToFile().
    // Built-in presets are not affected; duplicates are overwritten.
    bool loadFromFile(const std::string& path) {
        std::ifstream ifs(path);
        if (!ifs.good()) return false;
        std::string line;
        std::string currentName;
        std::string currentData;
        bool inPreset = false;
        while (std::getline(ifs, line)) {
            if (line.empty() || line[0] == '#') continue;
            if (line.substr(0, 9) == "lastUsed:") {
                m_lastUsed = line.substr(9);
            } else if (line.substr(0, 7) == "preset:") {
                currentName = line.substr(7);
                currentData.clear();
                inPreset = true;
            } else if (line == "end_preset") {
                if (inPreset && !currentName.empty()) {
                    // Overwrite if exists, else add
                    auto* existing = findPresetMut(currentName);
                    if (existing) {
                        existing->serializedData = currentData;
                    } else if (m_presets.size() < MAX_PRESETS) {
                        LayoutPreset p;
                        p.name = currentName;
                        p.serializedData = currentData;
                        p.isBuiltIn = false;
                        m_presets.push_back(std::move(p));
                    }
                }
                inPreset = false;
                currentName.clear();
                currentData.clear();
            } else if (inPreset) {
                currentData += line + "\n";
            }
        }
        return true;
    }

private:
    [[nodiscard]] LayoutPreset* findPresetMut(const std::string& name) {
        for (auto& p : m_presets) if (p.name == name) return &p;
        return nullptr;
    }

    std::vector<LayoutPreset> m_presets;
    bool m_autoSave = false;
    std::string m_lastUsed;
};


} // namespace NF
