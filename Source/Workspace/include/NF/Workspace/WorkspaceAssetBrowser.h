#pragma once
// NF::WorkspaceAssetBrowser — Query-driven asset browser model (Phase 67).
//
// Provides a filterable, searchable view over an AssetCatalog that editor
// tools can use to browse, select, and preview assets.  This is the model
// layer only; the UI chrome lives in the AtlasUI panel layer.
//
// Components:
//   AssetBrowserSortMode  — sort order for the results list
//   AssetBrowserFilter    — type mask, name pattern, path prefix, import state mask
//   AssetBrowserEntry     — a single result item (id, displayName, catalogPath, typeTag)
//   AssetBrowserState     — current filter + result set + selection
//   AssetBrowser          — owns an AssetBrowserState; references a catalog;
//                           applyFilter() / refresh() / select() / clearSelection()

#include "NF/Workspace/AssetCatalog.h"
#include <algorithm>
#include <cctype>
#include <cstdint>
#include <string>
#include <vector>

namespace NF {

// ── AssetBrowserSortMode ─────────────────────────────────────────────────────

enum class AssetBrowserSortMode : uint8_t {
    ByName,        // alphabetical by displayName
    ByType,        // by type tag, then name
    ByPath,        // alphabetical by catalogPath
    ByImportTime,  // newest imported first
    BySize,        // largest source file first
};

inline const char* assetBrowserSortModeName(AssetBrowserSortMode m) {
    switch (m) {
    case AssetBrowserSortMode::ByName:       return "ByName";
    case AssetBrowserSortMode::ByType:       return "ByType";
    case AssetBrowserSortMode::ByPath:       return "ByPath";
    case AssetBrowserSortMode::ByImportTime: return "ByImportTime";
    case AssetBrowserSortMode::BySize:       return "BySize";
    }
    return "Unknown";
}

// ── AssetBrowserFilter ───────────────────────────────────────────────────────
// Describes which assets should appear in the result set.

struct AssetBrowserFilter {
    // If non-empty, only assets whose displayName or catalogPath contains this
    // string (case-insensitive) are included.
    std::string namePattern;

    // If non-empty, only assets whose catalogPath starts with this prefix
    // are included.  Use "/" for the entire catalog root.
    std::string pathPrefix;

    // Type filter bit-mask.  Each bit corresponds to (1 << AssetTypeTag value).
    // 0 = all types accepted.
    uint32_t typeMask = 0u;

    // Import state filter bit-mask.  Each bit corresponds to
    // (1 << AssetImportState value).  0 = all states accepted.
    uint32_t importStateMask = 0u;

    // Maximum number of results to return.  0 = no limit (up to MAX_RESULTS).
    uint32_t maxResults = 0u;

    // Sort order for the result list.
    AssetBrowserSortMode sortMode = AssetBrowserSortMode::ByName;

    // ── Helpers ───────────────────────────────────────────────────────────────

    [[nodiscard]] bool acceptsType(AssetTypeTag tag) const {
        if (typeMask == 0u) return true;
        return (typeMask & (1u << static_cast<uint8_t>(tag))) != 0u;
    }

    [[nodiscard]] bool acceptsImportState(AssetImportState state) const {
        if (importStateMask == 0u) return true;
        return (importStateMask & (1u << static_cast<uint8_t>(state))) != 0u;
    }

    [[nodiscard]] bool isEmpty() const {
        return namePattern.empty()
            && pathPrefix.empty()
            && typeMask == 0u
            && importStateMask == 0u;
    }

    void clear() {
        namePattern.clear();
        pathPrefix.clear();
        typeMask        = 0u;
        importStateMask = 0u;
        maxResults      = 0u;
        sortMode        = AssetBrowserSortMode::ByName;
    }
};

// ── AssetBrowserEntry ────────────────────────────────────────────────────────
// A single result item returned by the browser.

struct AssetBrowserEntry {
    AssetId      id          = INVALID_ASSET_ID;
    std::string  displayName;
    std::string  catalogPath;
    AssetTypeTag typeTag    = AssetTypeTag::Unknown;
    AssetImportState importState = AssetImportState::Unknown;

    // Opaque thumbnail cookie.  Non-zero signals that a GPU texture or
    // platform-specific surface exists for preview purposes.
    uint32_t thumbnailCookie = 0u;

    [[nodiscard]] bool isValid() const {
        return id != INVALID_ASSET_ID && !catalogPath.empty();
    }
};

// ── AssetBrowserState ────────────────────────────────────────────────────────
// Current filter + result set + selection for one browser instance.

struct AssetBrowserState {
    static constexpr size_t MAX_RESULTS = 2048;

    AssetBrowserFilter         filter;
    std::vector<AssetBrowserEntry> results;

    // Selected entry (INVALID_ASSET_ID = nothing selected).
    AssetId selectedId = INVALID_ASSET_ID;

    // True when a refresh() call is pending (filter changed but not yet applied).
    bool dirty = false;

    [[nodiscard]] bool hasSelection() const {
        return selectedId != INVALID_ASSET_ID;
    }

    [[nodiscard]] const AssetBrowserEntry* selectedEntry() const {
        if (!hasSelection()) return nullptr;
        for (const auto& e : results)
            if (e.id == selectedId) return &e;
        return nullptr;
    }

    void clearSelection() { selectedId = INVALID_ASSET_ID; }

    void clear() {
        filter.clear();
        results.clear();
        selectedId = INVALID_ASSET_ID;
        dirty      = false;
    }
};

// ── AssetBrowser ─────────────────────────────────────────────────────────────
// Queries an AssetCatalog and maintains an AssetBrowserState.
//
// Ownership model:
//   - The browser holds a NON-OWNING pointer to an AssetCatalog.
//   - The catalog must outlive all browsers that reference it.
//
// Typical usage:
//   AssetBrowser browser;
//   browser.setCatalog(&catalog);
//   browser.setFilter(f);
//   browser.refresh();
//   const auto* entry = browser.state().selectedEntry();

class AssetBrowser {
public:
    static constexpr size_t MAX_OBSERVERS = 8;

    // ── Catalog binding ───────────────────────────────────────────────────────

    void setCatalog(const AssetCatalog* catalog) {
        m_catalog = catalog;
        m_state.dirty = true;
    }

    [[nodiscard]] const AssetCatalog* catalog() const { return m_catalog; }

    // ── Filter ────────────────────────────────────────────────────────────────

    void setFilter(const AssetBrowserFilter& f) {
        m_state.filter = f;
        m_state.dirty  = true;
    }

    [[nodiscard]] const AssetBrowserFilter& filter() const { return m_state.filter; }

    void clearFilter() {
        m_state.filter.clear();
        m_state.dirty = true;
    }

    // ── Refresh ───────────────────────────────────────────────────────────────
    // Re-runs the filter over the catalog and repopulates state.results.
    // Clears the current selection if the selected asset is no longer in results.
    // Returns the number of results produced.
    uint32_t refresh() {
        m_state.results.clear();
        m_state.dirty = false;

        if (!m_catalog) return 0u;

        const AssetBrowserFilter& f = m_state.filter;

        // Helper: case-insensitive contains
        auto containsCI = [](const std::string& haystack, const std::string& needle) {
            if (needle.empty()) return true;
            auto it = std::search(haystack.begin(), haystack.end(),
                                  needle.begin(), needle.end(),
                                  [](char a, char b) {
                                      return std::tolower(static_cast<unsigned char>(a))
                                          == std::tolower(static_cast<unsigned char>(b));
                                  });
            return it != haystack.end();
        };

        const auto allAssets = m_catalog->all();
        for (const auto* descPtr : allAssets) {
            if (!descPtr) continue;
            const AssetDescriptor& desc = *descPtr;
            if (m_state.results.size() >= AssetBrowserState::MAX_RESULTS) break;

            // Type filter
            if (!f.acceptsType(desc.typeTag)) continue;

            // Import state filter
            if (!f.acceptsImportState(desc.importState)) continue;

            // Path prefix filter
            if (!f.pathPrefix.empty()) {
                if (desc.catalogPath.compare(0, f.pathPrefix.size(), f.pathPrefix) != 0)
                    continue;
            }

            // Name pattern filter (matches displayName or catalogPath)
            if (!f.namePattern.empty()) {
                if (!containsCI(desc.displayName, f.namePattern) &&
                    !containsCI(desc.catalogPath, f.namePattern))
                    continue;
            }

            AssetBrowserEntry entry;
            entry.id           = desc.id;
            entry.displayName  = desc.displayName.empty() ? desc.catalogPath : desc.displayName;
            entry.catalogPath  = desc.catalogPath;
            entry.typeTag      = desc.typeTag;
            entry.importState  = desc.importState;
            m_state.results.push_back(std::move(entry));
        }

        // Apply max results cap
        if (f.maxResults > 0 && m_state.results.size() > f.maxResults) {
            m_state.results.resize(f.maxResults);
        }

        // Sort
        sortResults(f.sortMode);

        // Deselect if selected asset is no longer visible
        if (m_state.hasSelection()) {
            bool found = false;
            for (const auto& e : m_state.results)
                if (e.id == m_state.selectedId) { found = true; break; }
            if (!found) m_state.selectedId = INVALID_ASSET_ID;
        }

        uint32_t count = static_cast<uint32_t>(m_state.results.size());
        notifyRefresh(count);
        return count;
    }

    // ── Selection ─────────────────────────────────────────────────────────────

    // Select an asset by id.  Returns true if the id is present in the current results.
    bool select(AssetId id) {
        if (id == INVALID_ASSET_ID) {
            m_state.selectedId = INVALID_ASSET_ID;
            return false;
        }
        for (const auto& e : m_state.results) {
            if (e.id == id) {
                m_state.selectedId = id;
                notifySelection(id);
                return true;
            }
        }
        return false;
    }

    void clearSelection() {
        m_state.selectedId = INVALID_ASSET_ID;
    }

    [[nodiscard]] AssetId selectedId() const { return m_state.selectedId; }

    [[nodiscard]] const AssetBrowserEntry* selectedEntry() const {
        return m_state.selectedEntry();
    }

    // ── State access ───────────────────────────────────────────────────────────

    [[nodiscard]] const AssetBrowserState& state() const { return m_state; }

    [[nodiscard]] size_t  resultCount() const { return m_state.results.size(); }
    [[nodiscard]] bool    isDirty()     const { return m_state.dirty; }
    [[nodiscard]] bool    hasResults()  const { return !m_state.results.empty(); }

    // ── Observers ─────────────────────────────────────────────────────────────
    /// Fired after each `refresh()` call with the number of results produced.
    /// Receives: (uint32_t resultCount) — zero when the catalog is empty or null.
    using RefreshCallback   = std::function<void(uint32_t resultCount)>;

    /// Fired after each successful `select()` call.
    /// Receives: (AssetId id) — the id that was just selected.
    using SelectionCallback = std::function<void(AssetId id)>;

    bool addRefreshObserver(RefreshCallback cb) {
        if (!cb || m_refreshObservers.size() >= MAX_OBSERVERS) return false;
        m_refreshObservers.push_back(std::move(cb));
        return true;
    }

    bool addSelectionObserver(SelectionCallback cb) {
        if (!cb || m_selectionObservers.size() >= MAX_OBSERVERS) return false;
        m_selectionObservers.push_back(std::move(cb));
        return true;
    }

    void clearObservers() {
        m_refreshObservers.clear();
        m_selectionObservers.clear();
    }

    // ── Reset ─────────────────────────────────────────────────────────────────

    void reset() {
        m_state.clear();
        m_catalog = nullptr;
        clearObservers();
    }

private:
    const AssetCatalog* m_catalog = nullptr;
    AssetBrowserState   m_state;

    std::vector<RefreshCallback>   m_refreshObservers;
    std::vector<SelectionCallback> m_selectionObservers;

    void notifyRefresh(uint32_t count) {
        for (auto& cb : m_refreshObservers) cb(count);
    }

    void notifySelection(AssetId id) {
        for (auto& cb : m_selectionObservers) cb(id);
    }

    void sortResults(AssetBrowserSortMode mode) {
        switch (mode) {
        case AssetBrowserSortMode::ByName:
            std::sort(m_state.results.begin(), m_state.results.end(),
                [](const AssetBrowserEntry& a, const AssetBrowserEntry& b) {
                    return a.displayName < b.displayName;
                });
            break;
        case AssetBrowserSortMode::ByType:
            std::sort(m_state.results.begin(), m_state.results.end(),
                [](const AssetBrowserEntry& a, const AssetBrowserEntry& b) {
                    if (a.typeTag != b.typeTag)
                        return static_cast<uint8_t>(a.typeTag)
                             < static_cast<uint8_t>(b.typeTag);
                    return a.displayName < b.displayName;
                });
            break;
        case AssetBrowserSortMode::ByPath:
            std::sort(m_state.results.begin(), m_state.results.end(),
                [](const AssetBrowserEntry& a, const AssetBrowserEntry& b) {
                    return a.catalogPath < b.catalogPath;
                });
            break;
        case AssetBrowserSortMode::ByImportTime:
        case AssetBrowserSortMode::BySize:
            // These require data from the catalog that we don't carry in
            // AssetBrowserEntry; fall through to ByPath as a stable fallback.
            std::sort(m_state.results.begin(), m_state.results.end(),
                [](const AssetBrowserEntry& a, const AssetBrowserEntry& b) {
                    return a.catalogPath < b.catalogPath;
                });
            break;
        }
    }
};

} // namespace NF
