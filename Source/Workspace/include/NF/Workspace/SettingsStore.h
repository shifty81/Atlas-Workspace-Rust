#pragma once
// NF::Workspace — SettingsStore: layered typed key-value store.
//
// Settings are stored in three named layers with descending precedence:
//   User    — per-user overrides (highest precedence)
//   Project — project-scoped settings
//   Default — built-in defaults (lowest precedence; read-only externally)
//
// A read for a key walks User → Project → Default and returns the first hit.
// Writes go to the specified layer (User by default).
//
// All values are stored as strings. Typed accessors (getBool, getInt32, etc.)
// parse on read and stringify on write. A missing key with no default returns
// an empty string.
//
// Integration: SettingsStore reads/writes via a WorkspaceProjectFile section.
// Each layer maps to a named section in the project file:
//   "Settings.User", "Settings.Project", "Settings.Default"

#include "NF/Workspace/WorkspaceProjectFile.h"
#include <cstdint>
#include <fstream>
#include <functional>
#include <string>
#include <vector>

namespace NF {

// ── Settings Layer ─────────────────────────────────────────────────

enum class SettingsLayer : uint8_t {
    Default,   // lowest precedence — read-only (use setDefault to populate)
    Project,   // project-scoped
    User,      // per-user override (highest precedence)
};

inline const char* settingsLayerName(SettingsLayer l) {
    switch (l) {
    case SettingsLayer::Default: return "Default";
    case SettingsLayer::Project: return "Project";
    case SettingsLayer::User:    return "User";
    }
    return "Unknown";
}

// ── Change Notification ────────────────────────────────────────────
using SettingsChangeCallback = std::function<void(const std::string& key,
                                                   const std::string& newValue,
                                                   SettingsLayer layer)>;

// ── Settings Store ─────────────────────────────────────────────────

class SettingsStore {
public:
    static constexpr size_t MAX_KEYS_PER_LAYER = 512;
    static constexpr size_t MAX_OBSERVERS      = 64;

    // Section names used in WorkspaceProjectFile
    static constexpr const char* SECTION_USER    = "Settings.User";
    static constexpr const char* SECTION_PROJECT = "Settings.Project";
    static constexpr const char* SECTION_DEFAULT = "Settings.Default";

    // ── Write ──────────────────────────────────────────────────────

    void set(const std::string& key, const std::string& value,
             SettingsLayer layer = SettingsLayer::User) {
        layerData(layer).set(key, value);
        notifyChange(key, value, layer);
    }

    void setBool(const std::string& key, bool value,
                 SettingsLayer layer = SettingsLayer::User) {
        set(key, value ? "true" : "false", layer);
    }

    void setInt32(const std::string& key, int32_t value,
                  SettingsLayer layer = SettingsLayer::User) {
        set(key, std::to_string(value), layer);
    }

    void setFloat(const std::string& key, float value,
                  SettingsLayer layer = SettingsLayer::User) {
        // 6 significant digits
        char buf[32];
        std::snprintf(buf, sizeof(buf), "%.6g", static_cast<double>(value));
        set(key, buf, layer);
    }

    // Populate the Default layer (used at startup to register built-in defaults).
    void setDefault(const std::string& key, const std::string& value) {
        set(key, value, SettingsLayer::Default);
    }

    // ── Read ───────────────────────────────────────────────────────

    // Walk User → Project → Default; return first hit or empty string.
    [[nodiscard]] std::string get(const std::string& key) const {
        for (auto layer : {SettingsLayer::User, SettingsLayer::Project, SettingsLayer::Default}) {
            const auto* v = layerData(layer).get(key);
            if (v) return *v;
        }
        return {};
    }

    [[nodiscard]] std::string getOr(const std::string& key,
                                    const std::string& fallback) const {
        auto v = get(key);
        return v.empty() ? fallback : v;
    }

    [[nodiscard]] bool getBool(const std::string& key, bool fallback = false) const {
        auto v = get(key);
        if (v == "true"  || v == "1") return true;
        if (v == "false" || v == "0") return false;
        return fallback;
    }

    [[nodiscard]] int32_t getInt32(const std::string& key, int32_t fallback = 0) const {
        auto v = get(key);
        if (v.empty()) return fallback;
        try { return static_cast<int32_t>(std::stol(v)); }
        catch (...) { return fallback; }
    }

    [[nodiscard]] float getFloat(const std::string& key, float fallback = 0.f) const {
        auto v = get(key);
        if (v.empty()) return fallback;
        try { return std::stof(v); }
        catch (...) { return fallback; }
    }

    // Read from a specific layer only (no cascade).
    [[nodiscard]] const std::string* getFromLayer(const std::string& key,
                                                   SettingsLayer layer) const {
        return layerData(layer).get(key);
    }

    [[nodiscard]] bool has(const std::string& key) const {
        return !get(key).empty();
    }

    [[nodiscard]] bool hasInLayer(const std::string& key, SettingsLayer layer) const {
        return layerData(layer).get(key) != nullptr;
    }

    // ── Remove ─────────────────────────────────────────────────────

    bool remove(const std::string& key, SettingsLayer layer = SettingsLayer::User) {
        return layerData(layer).remove(key);
    }

    void clearLayer(SettingsLayer layer) {
        layerData(layer).clear();
    }

    // ── Stats ──────────────────────────────────────────────────────

    [[nodiscard]] size_t countInLayer(SettingsLayer layer) const {
        return layerData(layer).count();
    }

    [[nodiscard]] size_t totalCount() const {
        return m_user.count() + m_project.count() + m_default.count();
    }

    // ── Observers ──────────────────────────────────────────────────

    bool addObserver(SettingsChangeCallback cb) {
        if (!cb || m_observers.size() >= MAX_OBSERVERS) return false;
        m_observers.push_back(std::move(cb));
        return true;
    }

    void clearObservers() { m_observers.clear(); }

    // ── Serialization ──────────────────────────────────────────────

    void serializeLayer(SettingsLayer layer, WorkspaceProjectFile& file) const {
        const char* secName = sectionName(layer);
        auto& sec = file.section(secName);
        for (const auto& [k, v] : layerData(layer).entries()) {
            sec.set(k, v);
        }
    }

    bool deserializeLayer(SettingsLayer layer, const WorkspaceProjectFile& file) {
        const char* secName = sectionName(layer);
        const auto* sec = file.findSection(secName);
        if (!sec) return false;
        for (const auto& [k, v] : sec->entries()) {
            layerData(layer).set(k, v);
        }
        return true;
    }

    void serializeAll(WorkspaceProjectFile& file) const {
        serializeLayer(SettingsLayer::User,    file);
        serializeLayer(SettingsLayer::Project, file);
        serializeLayer(SettingsLayer::Default, file);
    }

    void deserializeAll(const WorkspaceProjectFile& file) {
        deserializeLayer(SettingsLayer::User,    file);
        deserializeLayer(SettingsLayer::Project, file);
        deserializeLayer(SettingsLayer::Default, file);
    }

    // ── File I/O ──────────────────────────────────────────────────
    // Saves the User and Project layers to a settings file.
    // The Default layer is never persisted (it is always rebuilt from code).
    bool saveToFile(const std::string& path) const {
        WorkspaceProjectFile pf;
        serializeLayer(SettingsLayer::User,    pf);
        serializeLayer(SettingsLayer::Project, pf);
        std::string text = pf.serialize();
        std::ofstream ofs(path, std::ios::out | std::ios::trunc);
        if (!ofs.good()) return false;
        ofs << text;
        return ofs.good();
    }

    // Loads User and Project layers from a settings file previously written by saveToFile().
    // The Default layer is not affected.
    bool loadFromFile(const std::string& path) {
        std::ifstream ifs(path);
        if (!ifs.good()) return false;
        std::string text((std::istreambuf_iterator<char>(ifs)),
                          std::istreambuf_iterator<char>());
        WorkspaceProjectFile pf;
        if (!WorkspaceProjectFile::parse(text, pf)) return false;
        deserializeLayer(SettingsLayer::User,    pf);
        deserializeLayer(SettingsLayer::Project, pf);
        return true;
    }

private:
    // Simple layer-backed store using ProjectFileSection for the k/v container.
    ProjectFileSection m_user    {"User"};
    ProjectFileSection m_project {"Project"};
    ProjectFileSection m_default {"Default"};

    std::vector<SettingsChangeCallback> m_observers;

    ProjectFileSection& layerData(SettingsLayer l) {
        switch (l) {
        case SettingsLayer::User:    return m_user;
        case SettingsLayer::Project: return m_project;
        default:                     return m_default;
        }
    }

    [[nodiscard]] const ProjectFileSection& layerData(SettingsLayer l) const {
        switch (l) {
        case SettingsLayer::User:    return m_user;
        case SettingsLayer::Project: return m_project;
        default:                     return m_default;
        }
    }

    static const char* sectionName(SettingsLayer l) {
        switch (l) {
        case SettingsLayer::User:    return SECTION_USER;
        case SettingsLayer::Project: return SECTION_PROJECT;
        default:                     return SECTION_DEFAULT;
        }
    }

    void notifyChange(const std::string& key, const std::string& value, SettingsLayer layer) {
        for (auto& cb : m_observers) {
            if (cb) cb(key, value, layer);
        }
    }
};

} // namespace NF
