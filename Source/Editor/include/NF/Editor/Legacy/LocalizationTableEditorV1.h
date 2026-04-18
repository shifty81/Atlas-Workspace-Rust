#pragma once
// NF::Editor — Localization table editor v1: localization key and translation management
#include "NF/Core/Core.h"
#include <algorithm>
#include <cstdint>
#include <functional>
#include <string>
#include <vector>

namespace NF {

enum class Ltev1EntryState : uint8_t { Draft, Translated, Verified, Deprecated };
enum class Ltev1TextContext : uint8_t { UI, Dialogue, Tutorial, Item, Quest, System };

inline const char* ltev1EntryStateName(Ltev1EntryState s) {
    switch (s) {
        case Ltev1EntryState::Draft:      return "Draft";
        case Ltev1EntryState::Translated: return "Translated";
        case Ltev1EntryState::Verified:   return "Verified";
        case Ltev1EntryState::Deprecated: return "Deprecated";
    }
    return "Unknown";
}

inline const char* ltev1TextContextName(Ltev1TextContext c) {
    switch (c) {
        case Ltev1TextContext::UI:       return "UI";
        case Ltev1TextContext::Dialogue: return "Dialogue";
        case Ltev1TextContext::Tutorial: return "Tutorial";
        case Ltev1TextContext::Item:     return "Item";
        case Ltev1TextContext::Quest:    return "Quest";
        case Ltev1TextContext::System:   return "System";
    }
    return "Unknown";
}

struct Ltev1LocalizationEntry {
    uint64_t         id          = 0;
    std::string      key;
    std::string      sourceText;
    Ltev1EntryState  state       = Ltev1EntryState::Draft;
    Ltev1TextContext context     = Ltev1TextContext::UI;

    [[nodiscard]] bool isValid()      const { return id != 0 && !key.empty(); }
    [[nodiscard]] bool isTranslated() const { return state == Ltev1EntryState::Translated; }
    [[nodiscard]] bool isVerified()   const { return state == Ltev1EntryState::Verified; }
    [[nodiscard]] bool isDeprecated() const { return state == Ltev1EntryState::Deprecated; }
};

struct Ltev1Translation {
    uint64_t    id      = 0;
    uint64_t    entryId = 0;
    std::string locale;
    std::string text;

    [[nodiscard]] bool isValid() const { return id != 0 && entryId != 0 && !locale.empty(); }
};

using Ltev1ChangeCallback = std::function<void(uint64_t)>;

class LocalizationTableEditorV1 {
public:
    static constexpr size_t MAX_ENTRIES      = 65536;
    static constexpr size_t MAX_TRANSLATIONS = 262144;

    bool addEntry(const Ltev1LocalizationEntry& entry) {
        if (!entry.isValid()) return false;
        for (const auto& e : m_entries) if (e.id == entry.id) return false;
        if (m_entries.size() >= MAX_ENTRIES) return false;
        m_entries.push_back(entry);
        if (m_onChange) m_onChange(entry.id);
        return true;
    }

    bool removeEntry(uint64_t id) {
        for (auto it = m_entries.begin(); it != m_entries.end(); ++it) {
            if (it->id == id) { m_entries.erase(it); return true; }
        }
        return false;
    }

    [[nodiscard]] Ltev1LocalizationEntry* findEntry(uint64_t id) {
        for (auto& e : m_entries) if (e.id == id) return &e;
        return nullptr;
    }

    bool addTranslation(const Ltev1Translation& trans) {
        if (!trans.isValid()) return false;
        for (const auto& t : m_translations) if (t.id == trans.id) return false;
        if (m_translations.size() >= MAX_TRANSLATIONS) return false;
        m_translations.push_back(trans);
        if (m_onChange) m_onChange(trans.id);
        return true;
    }

    bool removeTranslation(uint64_t id) {
        for (auto it = m_translations.begin(); it != m_translations.end(); ++it) {
            if (it->id == id) { m_translations.erase(it); return true; }
        }
        return false;
    }

    [[nodiscard]] size_t entryCount()      const { return m_entries.size(); }
    [[nodiscard]] size_t translationCount() const { return m_translations.size(); }

    [[nodiscard]] size_t translatedCount() const {
        size_t c = 0; for (const auto& e : m_entries) if (e.isTranslated()) ++c; return c;
    }
    [[nodiscard]] size_t verifiedCount() const {
        size_t c = 0; for (const auto& e : m_entries) if (e.isVerified()) ++c; return c;
    }
    [[nodiscard]] size_t countByContext(Ltev1TextContext context) const {
        size_t c = 0; for (const auto& e : m_entries) if (e.context == context) ++c; return c;
    }
    [[nodiscard]] size_t translationsForEntry(uint64_t entryId) const {
        size_t c = 0; for (const auto& t : m_translations) if (t.entryId == entryId) ++c; return c;
    }

    void setOnChange(Ltev1ChangeCallback cb) { m_onChange = std::move(cb); }

private:
    std::vector<Ltev1LocalizationEntry> m_entries;
    std::vector<Ltev1Translation>       m_translations;
    Ltev1ChangeCallback                 m_onChange;
};

} // namespace NF
