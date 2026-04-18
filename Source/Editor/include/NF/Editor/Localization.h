#pragma once
// NF::Editor — Translation table, locale manager, localization system
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

enum class LocaleId : uint8_t {
    English,
    Spanish,
    French,
    German,
    Japanese,
    Chinese,
    Korean,
    Russian
};

inline const char* localeIdName(LocaleId id) noexcept {
    switch (id) {
        case LocaleId::English:  return "English";
        case LocaleId::Spanish:  return "Spanish";
        case LocaleId::French:   return "French";
        case LocaleId::German:   return "German";
        case LocaleId::Japanese: return "Japanese";
        case LocaleId::Chinese:  return "Chinese";
        case LocaleId::Korean:   return "Korean";
        case LocaleId::Russian:  return "Russian";
        default:                 return "Unknown";
    }
}

struct LocalizedString {
    std::string key;
    std::string value;
    LocaleId locale = LocaleId::English;
    std::string context;
    bool verified = false;

    [[nodiscard]] bool isValid() const { return !key.empty() && !value.empty(); }
    [[nodiscard]] bool isVerified() const { return verified; }
    void verify() { verified = true; }
};

struct TranslationEntry {
    std::string key;
    std::string context;
    std::unordered_map<uint8_t, std::string> translations; // LocaleId -> text

    void set(LocaleId locale, const std::string& text) {
        translations[static_cast<uint8_t>(locale)] = text;
    }

    [[nodiscard]] const std::string* get(LocaleId locale) const {
        auto it = translations.find(static_cast<uint8_t>(locale));
        return (it != translations.end()) ? &it->second : nullptr;
    }

    [[nodiscard]] bool has(LocaleId locale) const {
        return translations.count(static_cast<uint8_t>(locale)) > 0;
    }

    [[nodiscard]] size_t localeCount() const { return translations.size(); }
};

class TranslationTable {
public:
    explicit TranslationTable(const std::string& tableName)
        : m_name(tableName) {}

    bool addEntry(const std::string& key, const std::string& context = "") {
        if (m_entries.size() >= kMaxEntries) return false;
        for (const auto& e : m_entries) {
            if (e.key == key) return false;
        }
        TranslationEntry entry;
        entry.key = key;
        entry.context = context;
        m_entries.push_back(entry);
        return true;
    }

    bool removeEntry(const std::string& key) {
        for (auto it = m_entries.begin(); it != m_entries.end(); ++it) {
            if (it->key == key) {
                m_entries.erase(it);
                return true;
            }
        }
        return false;
    }

    [[nodiscard]] TranslationEntry* findEntry(const std::string& key) {
        for (auto& e : m_entries) {
            if (e.key == key) return &e;
        }
        return nullptr;
    }

    [[nodiscard]] const TranslationEntry* findEntry(const std::string& key) const {
        for (const auto& e : m_entries) {
            if (e.key == key) return &e;
        }
        return nullptr;
    }

    bool setTranslation(const std::string& key, LocaleId locale, const std::string& text) {
        auto* entry = findEntry(key);
        if (!entry) return false;
        entry->set(locale, text);
        return true;
    }

    [[nodiscard]] const std::string* lookup(const std::string& key, LocaleId locale) const {
        const auto* entry = findEntry(key);
        if (!entry) return nullptr;
        return entry->get(locale);
    }

    [[nodiscard]] size_t entryCount() const { return m_entries.size(); }
    [[nodiscard]] const std::string& name() const { return m_name; }
    [[nodiscard]] const std::vector<TranslationEntry>& entries() const { return m_entries; }

    [[nodiscard]] size_t translatedCount(LocaleId locale) const {
        size_t count = 0;
        for (const auto& e : m_entries) {
            if (e.has(locale)) ++count;
        }
        return count;
    }

    [[nodiscard]] float completionRate(LocaleId locale) const {
        if (m_entries.empty()) return 0.f;
        return static_cast<float>(translatedCount(locale)) / static_cast<float>(m_entries.size());
    }

    static constexpr size_t kMaxEntries = 4096;

private:
    std::string m_name;
    std::vector<TranslationEntry> m_entries;
};

class LocaleManager {
public:
    explicit LocaleManager(LocaleId activeLocale = LocaleId::English)
        : m_active(activeLocale), m_fallback(LocaleId::English) {}

    void setActive(LocaleId locale) { m_active = locale; m_switchCount++; }
    void setFallback(LocaleId locale) { m_fallback = locale; }

    [[nodiscard]] LocaleId active() const { return m_active; }
    [[nodiscard]] LocaleId fallback() const { return m_fallback; }
    [[nodiscard]] size_t switchCount() const { return m_switchCount; }

    [[nodiscard]] const std::string* resolve(const TranslationTable& table, const std::string& key) const {
        const auto* result = table.lookup(key, m_active);
        if (result) return result;
        if (m_active != m_fallback) {
            return table.lookup(key, m_fallback);
        }
        return nullptr;
    }

private:
    LocaleId m_active;
    LocaleId m_fallback;
    size_t m_switchCount = 0;
};

struct LocalizationConfig {
    LocaleId defaultLocale = LocaleId::English;
    LocaleId fallbackLocale = LocaleId::English;
    size_t maxTables = 16;
    bool autoDetectLocale = false;
};

class LocalizationSystem {
public:
    void init(const LocalizationConfig& config = {}) {
        m_config = config;
        m_manager = LocaleManager(config.defaultLocale);
        m_manager.setFallback(config.fallbackLocale);
        m_initialized = true;
    }

    void shutdown() {
        m_tables.clear();
        m_initialized = false;
        m_tickCount = 0;
    }

    [[nodiscard]] bool isInitialized() const { return m_initialized; }

    int createTable(const std::string& tableName) {
        if (!m_initialized) return -1;
        if (m_tables.size() >= m_config.maxTables) return -1;
        for (const auto& t : m_tables) {
            if (t.name() == tableName) return -1;
        }
        m_tables.emplace_back(tableName);
        return static_cast<int>(m_tables.size()) - 1;
    }

    [[nodiscard]] TranslationTable* table(int index) {
        if (index < 0 || index >= static_cast<int>(m_tables.size())) return nullptr;
        return &m_tables[static_cast<size_t>(index)];
    }

    [[nodiscard]] TranslationTable* tableByName(const std::string& name) {
        for (auto& t : m_tables) {
            if (t.name() == name) return &t;
        }
        return nullptr;
    }

    [[nodiscard]] const std::string* translate(const std::string& tableName, const std::string& key) const {
        for (const auto& t : m_tables) {
            if (t.name() == tableName) {
                return m_manager.resolve(t, key);
            }
        }
        return nullptr;
    }

    void setLocale(LocaleId locale) { m_manager.setActive(locale); }
    [[nodiscard]] LocaleId activeLocale() const { return m_manager.active(); }

    void tick(float /*dt*/) {
        if (!m_initialized) return;
        m_tickCount++;
    }

    [[nodiscard]] size_t tableCount() const { return m_tables.size(); }
    [[nodiscard]] size_t tickCount() const { return m_tickCount; }
    [[nodiscard]] const LocalizationConfig& config() const { return m_config; }
    [[nodiscard]] LocaleManager& localeManager() { return m_manager; }
    [[nodiscard]] const LocaleManager& localeManager() const { return m_manager; }

    [[nodiscard]] size_t totalEntries() const {
        size_t count = 0;
        for (const auto& t : m_tables) count += t.entryCount();
        return count;
    }

    [[nodiscard]] size_t totalTranslated(LocaleId locale) const {
        size_t count = 0;
        for (const auto& t : m_tables) count += t.translatedCount(locale);
        return count;
    }

private:
    LocalizationConfig m_config;
    std::vector<TranslationTable> m_tables;
    LocaleManager m_manager;
    bool m_initialized = false;
    size_t m_tickCount = 0;
};

// ─────────────────────────────────────────────────────────────────────────────
// S14 — Plugin System
// ─────────────────────────────────────────────────────────────────────────────


} // namespace NF
