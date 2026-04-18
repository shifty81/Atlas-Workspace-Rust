#pragma once
// NF::Editor — AssetDependencyViewer: dependency graph viewer for the Asset Editor.
//
// Takes an AssetDocument's dependency list and projects it into a flat or
// tree view suitable for a panel.  Supports depth-limited expansion, type
// filtering, and a "highlight reverse dependencies" mode (which assets depend
// on this one).
//
// Phase G.2 — Asset Editor: asset dependency viewer

#include "NF/Editor/AssetDocument.h"
#include <cstdint>
#include <functional>
#include <string>
#include <vector>

namespace NF {

// ── DependencyViewMode ────────────────────────────────────────────────────────

enum class DependencyViewMode : uint8_t {
    Flat,   ///< flat list of all direct dependencies
    Tree,   ///< hierarchical tree (depth-limited)
};

inline const char* dependencyViewModeName(DependencyViewMode m) {
    switch (m) {
    case DependencyViewMode::Flat: return "Flat";
    case DependencyViewMode::Tree: return "Tree";
    }
    return "Unknown";
}

// ── DependencyViewEntry ───────────────────────────────────────────────────────

struct DependencyViewEntry {
    std::string guid;
    std::string relationship;
    uint32_t    depth       = 0;
    bool        isHighlighted = false;

    [[nodiscard]] bool isValid() const { return !guid.empty(); }
};

// ── AssetDependencyViewer ─────────────────────────────────────────────────────

class AssetDependencyViewer {
public:
    static constexpr uint32_t kDefaultMaxDepth = 4;

    AssetDependencyViewer() = default;

    // ── Source binding ────────────────────────────────────────────────────

    /// Populate the viewer from an AssetDocument's dependency list.
    void loadFromDocument(const AssetDocument& doc) {
        m_entries.clear();
        m_rootGuid = doc.guid();

        for (const auto& dep : doc.dependencies()) {
            DependencyViewEntry e;
            e.guid         = dep.guid;
            e.relationship = dep.relationship;
            e.depth        = 0;
            m_entries.push_back(std::move(e));
        }

        m_dirty = false;
        if (m_onReload) m_onReload();
    }

    // ── Configuration ─────────────────────────────────────────────────────

    void setMode(DependencyViewMode mode) { m_mode = mode; }
    [[nodiscard]] DependencyViewMode mode() const { return m_mode; }

    void setMaxDepth(uint32_t d) { m_maxDepth = d; }
    [[nodiscard]] uint32_t maxDepth() const { return m_maxDepth; }

    void setRelationshipFilter(const std::string& rel) { m_relationshipFilter = rel; }
    void clearRelationshipFilter() { m_relationshipFilter.clear(); }
    [[nodiscard]] const std::string& relationshipFilter() const { return m_relationshipFilter; }

    // ── Entries (post-filter) ─────────────────────────────────────────────

    [[nodiscard]] std::vector<DependencyViewEntry> visibleEntries() const {
        std::vector<DependencyViewEntry> result;
        for (const auto& e : m_entries) {
            if (!m_relationshipFilter.empty() && e.relationship != m_relationshipFilter)
                continue;
            if (m_mode == DependencyViewMode::Tree && e.depth > m_maxDepth)
                continue;
            result.push_back(e);
        }
        return result;
    }

    [[nodiscard]] uint32_t totalCount() const {
        return static_cast<uint32_t>(m_entries.size());
    }

    [[nodiscard]] uint32_t visibleCount() const {
        return static_cast<uint32_t>(visibleEntries().size());
    }

    // ── Highlight (reverse dependency) ────────────────────────────────────

    /// Mark entries whose GUID matches `guid` as highlighted.
    /// Used for "who depends on this?" reverse lookup.
    uint32_t highlightByGuid(const std::string& guid) {
        uint32_t count = 0;
        for (auto& e : m_entries) {
            e.isHighlighted = (e.guid == guid);
            if (e.isHighlighted) ++count;
        }
        return count;
    }

    void clearHighlights() {
        for (auto& e : m_entries) e.isHighlighted = false;
    }

    [[nodiscard]] uint32_t highlightedCount() const {
        uint32_t n = 0;
        for (const auto& e : m_entries) if (e.isHighlighted) ++n;
        return n;
    }

    // ── State ─────────────────────────────────────────────────────────────

    [[nodiscard]] bool          isDirty()   const { return m_dirty; }
    [[nodiscard]] const std::string& rootGuid() const { return m_rootGuid; }

    // ── Observer ──────────────────────────────────────────────────────────

    using ReloadCallback = std::function<void()>;
    void setOnReload(ReloadCallback cb) { m_onReload = std::move(cb); }

private:
    DependencyViewMode             m_mode    = DependencyViewMode::Flat;
    uint32_t                       m_maxDepth = kDefaultMaxDepth;
    std::string                    m_rootGuid;
    std::string                    m_relationshipFilter;
    std::vector<DependencyViewEntry> m_entries;
    bool                           m_dirty = false;
    ReloadCallback                 m_onReload;
};

} // namespace NF
