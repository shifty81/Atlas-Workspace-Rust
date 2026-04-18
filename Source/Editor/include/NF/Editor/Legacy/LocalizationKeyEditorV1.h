#pragma once
// NF::Editor — Localization key editor v1: localization key management and translation status tracking
#include "NF/Core/Core.h"
#include <algorithm>
#include <cstdint>
#include <functional>
#include <string>
#include <vector>

namespace NF {

enum class Lokv1TranslationState : uint8_t { Missing, Draft, Review, Approved, Exported };
enum class Lokv1KeyType          : uint8_t { UI, Dialogue, Subtitle, Audio, Metadata };

inline const char* lokv1TranslationStateName(Lokv1TranslationState s) {
    switch (s) {
        case Lokv1TranslationState::Missing:  return "Missing";
        case Lokv1TranslationState::Draft:    return "Draft";
        case Lokv1TranslationState::Review:   return "Review";
        case Lokv1TranslationState::Approved: return "Approved";
        case Lokv1TranslationState::Exported: return "Exported";
    }
    return "Unknown";
}

inline const char* lokv1KeyTypeName(Lokv1KeyType t) {
    switch (t) {
        case Lokv1KeyType::UI:       return "UI";
        case Lokv1KeyType::Dialogue: return "Dialogue";
        case Lokv1KeyType::Subtitle: return "Subtitle";
        case Lokv1KeyType::Audio:    return "Audio";
        case Lokv1KeyType::Metadata: return "Metadata";
    }
    return "Unknown";
}

struct Lokv1Translation {
    std::string           locale;
    std::string           text;
    Lokv1TranslationState state = Lokv1TranslationState::Missing;

    [[nodiscard]] bool isValid()    const { return !locale.empty(); }
    [[nodiscard]] bool isApproved() const { return state == Lokv1TranslationState::Approved; }
    [[nodiscard]] bool isMissing()  const { return state == Lokv1TranslationState::Missing; }
};

struct Lokv1Key {
    uint64_t                     id      = 0;
    std::string                  name;
    Lokv1KeyType                 type    = Lokv1KeyType::UI;
    std::vector<Lokv1Translation> translations;

    [[nodiscard]] bool isValid() const { return id != 0 && !name.empty(); }

    bool addTranslation(const Lokv1Translation& t) {
        if (!t.isValid()) return false;
        for (const auto& tr : translations) if (tr.locale == t.locale) return false;
        translations.push_back(t);
        return true;
    }
};

using Lokv1ChangeCallback = std::function<void(uint64_t)>;

class LocalizationKeyEditorV1 {
public:
    static constexpr size_t MAX_KEYS = 65536;

    bool addKey(const Lokv1Key& key) {
        if (!key.isValid()) return false;
        for (const auto& k : m_keys) if (k.id == key.id) return false;
        if (m_keys.size() >= MAX_KEYS) return false;
        m_keys.push_back(key);
        return true;
    }

    bool removeKey(uint64_t id) {
        for (auto it = m_keys.begin(); it != m_keys.end(); ++it) {
            if (it->id == id) { m_keys.erase(it); return true; }
        }
        return false;
    }

    [[nodiscard]] Lokv1Key* findKey(uint64_t id) {
        for (auto& k : m_keys) if (k.id == id) return &k;
        return nullptr;
    }

    bool addTranslation(uint64_t keyId, const Lokv1Translation& t) {
        auto* k = findKey(keyId);
        if (!k) return false;
        bool ok = k->addTranslation(t);
        if (ok && m_onChange) m_onChange(keyId);
        return ok;
    }

    [[nodiscard]] size_t keyCount() const { return m_keys.size(); }

    [[nodiscard]] size_t approvedCount() const {
        size_t c = 0;
        for (const auto& k : m_keys)
            for (const auto& t : k.translations) if (t.isApproved()) ++c;
        return c;
    }
    [[nodiscard]] size_t missingCount() const {
        size_t c = 0;
        for (const auto& k : m_keys)
            for (const auto& t : k.translations) if (t.isMissing()) ++c;
        return c;
    }
    [[nodiscard]] size_t countByKeyType(Lokv1KeyType type) const {
        size_t c = 0; for (const auto& k : m_keys) if (k.type == type) ++c; return c;
    }
    [[nodiscard]] size_t countByState(Lokv1TranslationState state) const {
        size_t c = 0;
        for (const auto& k : m_keys)
            for (const auto& t : k.translations) if (t.state == state) ++c;
        return c;
    }

    void setOnChange(Lokv1ChangeCallback cb) { m_onChange = std::move(cb); }

private:
    std::vector<Lokv1Key>  m_keys;
    Lokv1ChangeCallback    m_onChange;
};

} // namespace NF
