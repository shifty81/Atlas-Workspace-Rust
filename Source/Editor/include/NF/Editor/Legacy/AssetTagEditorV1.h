#pragma once
// NF::Editor — Asset tag editor v1: tag authoring, color coding, and filter presets
#include "NF/Core/Core.h"
#include <algorithm>
#include <cstdint>
#include <functional>
#include <string>
#include <vector>

namespace NF {

enum class Atv1TagCategory  : uint8_t { Type, Status, Priority, Team, Custom };
enum class Atv1TagScope     : uint8_t { Asset, Folder, Project, Global };

inline const char* atv1TagCategoryName(Atv1TagCategory c) {
    switch (c) {
        case Atv1TagCategory::Type:     return "Type";
        case Atv1TagCategory::Status:   return "Status";
        case Atv1TagCategory::Priority: return "Priority";
        case Atv1TagCategory::Team:     return "Team";
        case Atv1TagCategory::Custom:   return "Custom";
    }
    return "Unknown";
}

inline const char* atv1TagScopeName(Atv1TagScope s) {
    switch (s) {
        case Atv1TagScope::Asset:   return "Asset";
        case Atv1TagScope::Folder:  return "Folder";
        case Atv1TagScope::Project: return "Project";
        case Atv1TagScope::Global:  return "Global";
    }
    return "Unknown";
}

struct Atv1Tag {
    uint64_t        id       = 0;
    std::string     name;
    Atv1TagCategory category = Atv1TagCategory::Custom;
    Atv1TagScope    scope    = Atv1TagScope::Asset;
    uint32_t        color    = 0xFFFFFFFF; // RRGGBBAA
    bool            pinned   = false;

    [[nodiscard]] bool isValid() const { return id != 0 && !name.empty(); }
};

struct Atv1TaggedAsset {
    std::string              assetPath;
    std::vector<uint64_t>    tagIds;

    [[nodiscard]] bool hasTag(uint64_t tagId) const {
        return std::find(tagIds.begin(), tagIds.end(), tagId) != tagIds.end();
    }
    bool addTag(uint64_t tagId) {
        if (hasTag(tagId)) return false;
        tagIds.push_back(tagId);
        return true;
    }
    bool removeTag(uint64_t tagId) {
        auto it = std::find(tagIds.begin(), tagIds.end(), tagId);
        if (it == tagIds.end()) return false;
        tagIds.erase(it);
        return true;
    }
};

using Atv1ChangeCallback = std::function<void()>;

class AssetTagEditorV1 {
public:
    static constexpr size_t MAX_TAGS   = 512;
    static constexpr size_t MAX_ASSETS = 4096;

    bool addTag(const Atv1Tag& tag) {
        if (!tag.isValid()) return false;
        for (const auto& t : m_tags) if (t.id == tag.id) return false;
        if (m_tags.size() >= MAX_TAGS) return false;
        m_tags.push_back(tag);
        if (m_onChange) m_onChange();
        return true;
    }

    bool removeTag(uint64_t id) {
        for (auto it = m_tags.begin(); it != m_tags.end(); ++it) {
            if (it->id == id) { m_tags.erase(it); if (m_onChange) m_onChange(); return true; }
        }
        return false;
    }

    [[nodiscard]] const Atv1Tag* findTag(uint64_t id) const {
        for (const auto& t : m_tags) if (t.id == id) return &t;
        return nullptr;
    }

    bool tagAsset(const std::string& assetPath, uint64_t tagId) {
        if (!findTag(tagId)) return false;
        for (auto& a : m_assets) {
            if (a.assetPath == assetPath) return a.addTag(tagId);
        }
        if (m_assets.size() >= MAX_ASSETS) return false;
        Atv1TaggedAsset ta; ta.assetPath = assetPath; ta.addTag(tagId);
        m_assets.push_back(std::move(ta));
        if (m_onChange) m_onChange();
        return true;
    }

    bool untagAsset(const std::string& assetPath, uint64_t tagId) {
        for (auto& a : m_assets) {
            if (a.assetPath == assetPath) {
                bool ok = a.removeTag(tagId);
                if (ok && m_onChange) m_onChange();
                return ok;
            }
        }
        return false;
    }

    [[nodiscard]] size_t tagCount()   const { return m_tags.size(); }
    [[nodiscard]] size_t assetCount() const { return m_assets.size(); }
    [[nodiscard]] size_t countByCategory(Atv1TagCategory cat) const {
        size_t c = 0; for (const auto& t : m_tags) if (t.category == cat) ++c; return c;
    }
    [[nodiscard]] size_t countByScope(Atv1TagScope scope) const {
        size_t c = 0; for (const auto& t : m_tags) if (t.scope == scope) ++c; return c;
    }
    [[nodiscard]] size_t pinnedCount() const {
        size_t c = 0; for (const auto& t : m_tags) if (t.pinned) ++c; return c;
    }

    void setOnChange(Atv1ChangeCallback cb) { m_onChange = std::move(cb); }

private:
    std::vector<Atv1Tag>         m_tags;
    std::vector<Atv1TaggedAsset> m_assets;
    Atv1ChangeCallback           m_onChange;
};

} // namespace NF
