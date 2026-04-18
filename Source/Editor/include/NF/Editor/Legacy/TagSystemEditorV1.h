#pragma once
// NF::Editor — Tag system editor v1: named, coloured, categorised tag authoring
#include "NF/Core/Core.h"
#include <cstdint>
#include <functional>
#include <string>
#include <vector>

namespace NF {

struct Tsv1Tag {
    uint64_t    id       = 0;
    std::string name;
    uint32_t    color    = 0xFFFFFFFFu;
    std::string category;

    [[nodiscard]] bool isValid() const { return id != 0 && !name.empty(); }
};

using Tsv1ChangeCallback = std::function<void(uint64_t)>;

class TagSystemEditorV1 {
public:
    static constexpr size_t MAX_TAGS = 512;

    bool addTag(const Tsv1Tag& tag) {
        if (!tag.isValid()) return false;
        for (const auto& t : m_tags) if (t.id == tag.id) return false;
        if (m_tags.size() >= MAX_TAGS) return false;
        m_tags.push_back(tag);
        if (m_onChange) m_onChange(tag.id);
        return true;
    }

    bool removeTag(uint64_t id) {
        for (auto it = m_tags.begin(); it != m_tags.end(); ++it) {
            if (it->id == id) { m_tags.erase(it); return true; }
        }
        return false;
    }

    [[nodiscard]] Tsv1Tag* findTag(uint64_t id) {
        for (auto& t : m_tags) if (t.id == id) return &t;
        return nullptr;
    }

    bool setCategory(uint64_t id, const std::string& category) {
        auto* t = findTag(id);
        if (!t) return false;
        t->category = category;
        if (m_onChange) m_onChange(id);
        return true;
    }

    bool setColor(uint64_t id, uint32_t color) {
        auto* t = findTag(id);
        if (!t) return false;
        t->color = color;
        if (m_onChange) m_onChange(id);
        return true;
    }

    [[nodiscard]] bool hasTag(const std::string& name) const {
        for (const auto& t : m_tags) if (t.name == name) return true;
        return false;
    }

    [[nodiscard]] size_t tagCount() const { return m_tags.size(); }

    [[nodiscard]] size_t countByCategory(const std::string& category) const {
        size_t c = 0;
        for (const auto& t : m_tags) if (t.category == category) ++c;
        return c;
    }

    void setOnChange(Tsv1ChangeCallback cb) { m_onChange = std::move(cb); }

private:
    std::vector<Tsv1Tag> m_tags;
    Tsv1ChangeCallback   m_onChange;
};

} // namespace NF
