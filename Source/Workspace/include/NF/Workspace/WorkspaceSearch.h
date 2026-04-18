#pragma once
// NF::Workspace — Phase 17: Workspace Search and Indexing
//
// Workspace-level search and content indexing infrastructure:
//   SearchScope         — search scope classification
//   SearchResultType    — result type classification
//   SearchMatchKind     — how a result matched
//   SearchQuery         — typed search query with filters and scope
//   SearchResult        — ranked result with match context
//   SearchIndex         — in-memory content index with add/remove/query
//   SearchEngine        — register indices and execute cross-index queries

#include <algorithm>
#include <cstdint>
#include <functional>
#include <string>
#include <vector>

namespace NF {

// ═════════════════════════════════════════════════════════════════
// SearchScope — where to search
// ═════════════════════════════════════════════════════════════════

enum class SearchScope : uint8_t {
    All         = 0,   // Search everywhere
    Project     = 1,   // Current project files
    Assets      = 2,   // Asset catalog
    Tools       = 3,   // Tool registry
    Panels      = 4,   // Panel registry
    Commands    = 5,   // Command palette
    Settings    = 6,   // Preferences/settings
    Plugins     = 7,   // Plugin registry
    Scripts     = 8,   // Script bindings
    Custom      = 9,   // User-defined
};

inline const char* searchScopeName(SearchScope s) {
    switch (s) {
        case SearchScope::All:      return "All";
        case SearchScope::Project:  return "Project";
        case SearchScope::Assets:   return "Assets";
        case SearchScope::Tools:    return "Tools";
        case SearchScope::Panels:   return "Panels";
        case SearchScope::Commands: return "Commands";
        case SearchScope::Settings: return "Settings";
        case SearchScope::Plugins:  return "Plugins";
        case SearchScope::Scripts:  return "Scripts";
        case SearchScope::Custom:   return "Custom";
        default:                    return "Unknown";
    }
}

// ═════════════════════════════════════════════════════════════════
// SearchResultType — what was found
// ═════════════════════════════════════════════════════════════════

enum class SearchResultType : uint8_t {
    File        = 0,
    Asset       = 1,
    Tool        = 2,
    Panel       = 3,
    Command     = 4,
    Setting     = 5,
    Plugin      = 6,
    Script      = 7,
    Text        = 8,
    Symbol      = 9,
    Custom      = 10,
};

inline const char* searchResultTypeName(SearchResultType t) {
    switch (t) {
        case SearchResultType::File:    return "File";
        case SearchResultType::Asset:   return "Asset";
        case SearchResultType::Tool:    return "Tool";
        case SearchResultType::Panel:   return "Panel";
        case SearchResultType::Command: return "Command";
        case SearchResultType::Setting: return "Setting";
        case SearchResultType::Plugin:  return "Plugin";
        case SearchResultType::Script:  return "Script";
        case SearchResultType::Text:    return "Text";
        case SearchResultType::Symbol:  return "Symbol";
        case SearchResultType::Custom:  return "Custom";
        default:                        return "Unknown";
    }
}

// ═════════════════════════════════════════════════════════════════
// SearchMatchKind — how a result matched
// ═════════════════════════════════════════════════════════════════

enum class SearchMatchKind : uint8_t {
    Exact       = 0,   // Exact string match
    Prefix      = 1,   // Starts with query
    Contains    = 2,   // Contains query substring
    Fuzzy       = 3,   // Fuzzy/approximate match
};

inline const char* searchMatchKindName(SearchMatchKind k) {
    switch (k) {
        case SearchMatchKind::Exact:    return "Exact";
        case SearchMatchKind::Prefix:   return "Prefix";
        case SearchMatchKind::Contains: return "Contains";
        case SearchMatchKind::Fuzzy:    return "Fuzzy";
        default:                        return "Unknown";
    }
}

// ═════════════════════════════════════════════════════════════════
// SearchQuery — typed search query with filters and scope
// ═════════════════════════════════════════════════════════════════

class SearchQuery {
public:
    SearchQuery() = default;

    explicit SearchQuery(const std::string& text,
                          SearchScope scope = SearchScope::All)
        : text_(text), scope_(scope) {}

    // ── Identity ─────────────────────────────────────────────────
    const std::string& text()   const { return text_; }
    SearchScope        scope()  const { return scope_; }
    bool isValid() const { return !text_.empty(); }

    // ── Filters ──────────────────────────────────────────────────
    void setScope(SearchScope s)               { scope_ = s; }
    void setCaseSensitive(bool v)              { caseSensitive_ = v; }
    bool caseSensitive() const                 { return caseSensitive_; }

    void setMaxResults(size_t n)               { maxResults_ = n; }
    size_t maxResults() const                  { return maxResults_; }

    void addTypeFilter(SearchResultType t) {
        for (auto f : typeFilters_) if (f == t) return;
        typeFilters_.push_back(t);
    }
    bool hasTypeFilter(SearchResultType t) const {
        for (auto f : typeFilters_) if (f == t) return true;
        return false;
    }
    bool hasTypeFilters() const { return !typeFilters_.empty(); }
    const std::vector<SearchResultType>& typeFilters() const { return typeFilters_; }
    void clearTypeFilters() { typeFilters_.clear(); }

    // ── Source filter ────────────────────────────────────────────
    void setSourceFilter(const std::string& s) { sourceFilter_ = s; }
    const std::string& sourceFilter() const    { return sourceFilter_; }

    bool operator==(const SearchQuery& o) const {
        return text_ == o.text_ && scope_ == o.scope_;
    }
    bool operator!=(const SearchQuery& o) const { return !(*this == o); }

private:
    std::string                   text_;
    SearchScope                   scope_         = SearchScope::All;
    bool                          caseSensitive_ = false;
    size_t                        maxResults_    = 100;
    std::vector<SearchResultType> typeFilters_;
    std::string                   sourceFilter_;
};

// ═════════════════════════════════════════════════════════════════
// SearchResult — ranked result with match context
// ═════════════════════════════════════════════════════════════════

struct SearchResult {
    std::string      id;            // Unique result identifier
    std::string      title;         // Display title
    std::string      description;   // Brief description
    std::string      source;        // Where the result came from (index name)
    std::string      context;       // Match context (snippet)
    SearchResultType type       = SearchResultType::Text;
    SearchMatchKind  matchKind  = SearchMatchKind::Contains;
    float            score      = 0.0f;   // Relevance score (higher=better)
    size_t           matchStart = 0;      // Start offset of match in context
    size_t           matchLen   = 0;      // Length of match

    bool isValid() const { return !id.empty() && !title.empty(); }

    bool operator<(const SearchResult& o) const { return score > o.score; } // Higher score first
    bool operator==(const SearchResult& o) const { return id == o.id && source == o.source; }
    bool operator!=(const SearchResult& o) const { return !(*this == o); }
};

// ═════════════════════════════════════════════════════════════════
// SearchIndex — in-memory content index
// ═════════════════════════════════════════════════════════════════

class SearchIndex {
public:
    static constexpr size_t MAX_ENTRIES = 16384;

    SearchIndex() = default;
    explicit SearchIndex(const std::string& name, SearchScope scope = SearchScope::All)
        : name_(name), scope_(scope) {}

    // ── Identity ─────────────────────────────────────────────────
    const std::string& name()  const { return name_; }
    SearchScope        scope() const { return scope_; }
    bool isValid() const { return !name_.empty(); }

    // ── Entry management ─────────────────────────────────────────
    struct Entry {
        std::string      id;
        std::string      title;
        std::string      content;     // Searchable text content
        std::string      description;
        SearchResultType type = SearchResultType::Text;

        bool isValid() const { return !id.empty() && !title.empty(); }
    };

    bool addEntry(const Entry& entry) {
        if (!entry.isValid()) return false;
        for (auto& e : entries_) if (e.id == entry.id) return false;
        if (entries_.size() >= MAX_ENTRIES) return false;
        entries_.push_back(entry);
        return true;
    }

    bool removeEntry(const std::string& id) {
        for (auto it = entries_.begin(); it != entries_.end(); ++it) {
            if (it->id == id) { entries_.erase(it); return true; }
        }
        return false;
    }

    bool updateEntry(const Entry& entry) {
        for (auto& e : entries_) {
            if (e.id == entry.id) {
                e = entry;
                return true;
            }
        }
        return false;
    }

    const Entry* findEntry(const std::string& id) const {
        for (auto& e : entries_) if (e.id == id) return &e;
        return nullptr;
    }

    size_t entryCount() const { return entries_.size(); }
    bool   isEmpty()    const { return entries_.empty(); }
    const std::vector<Entry>& entries() const { return entries_; }

    // ── Search ───────────────────────────────────────────────────
    std::vector<SearchResult> query(const SearchQuery& q) const {
        std::vector<SearchResult> results;
        if (!q.isValid()) return results;

        std::string queryText = q.caseSensitive() ? q.text() : toLower(q.text());

        for (auto& entry : entries_) {
            // Type filter check
            if (q.hasTypeFilters() && !q.hasTypeFilter(entry.type)) continue;

            SearchResult result;
            result.id     = entry.id;
            result.title  = entry.title;
            result.description = entry.description;
            result.source = name_;
            result.type   = entry.type;

            std::string titleLower = q.caseSensitive() ? entry.title : toLower(entry.title);
            std::string contentLower = q.caseSensitive() ? entry.content : toLower(entry.content);

            // Check title match
            if (titleLower == queryText) {
                result.matchKind  = SearchMatchKind::Exact;
                result.score      = 100.0f;
                result.context    = entry.title;
                result.matchStart = 0;
                result.matchLen   = queryText.size();
            } else if (titleLower.find(queryText) == 0) {
                result.matchKind  = SearchMatchKind::Prefix;
                result.score      = 80.0f;
                result.context    = entry.title;
                result.matchStart = 0;
                result.matchLen   = queryText.size();
            } else if (titleLower.find(queryText) != std::string::npos) {
                size_t pos = titleLower.find(queryText);
                result.matchKind  = SearchMatchKind::Contains;
                result.score      = 60.0f;
                result.context    = entry.title;
                result.matchStart = pos;
                result.matchLen   = queryText.size();
            } else if (contentLower.find(queryText) != std::string::npos) {
                size_t pos = contentLower.find(queryText);
                result.matchKind  = SearchMatchKind::Contains;
                result.score      = 40.0f;
                // Extract context snippet
                size_t start = (pos > 20) ? pos - 20 : 0;
                size_t end   = std::min(pos + queryText.size() + 20, entry.content.size());
                result.context    = entry.content.substr(start, end - start);
                result.matchStart = pos - start;
                result.matchLen   = queryText.size();
            } else {
                continue; // No match
            }

            results.push_back(result);
            if (results.size() >= q.maxResults()) break;
        }

        // Sort by score descending
        std::sort(results.begin(), results.end());
        return results;
    }

    // ── Lifecycle ────────────────────────────────────────────────
    void clear() { entries_.clear(); }

private:
    static std::string toLower(const std::string& s) {
        std::string r = s;
        for (auto& c : r) {
            if (c >= 'A' && c <= 'Z') c = static_cast<char>(c + 32);
        }
        return r;
    }

    std::string          name_;
    SearchScope          scope_ = SearchScope::All;
    std::vector<Entry>   entries_;
};

// ═════════════════════════════════════════════════════════════════
// SearchEngine — register indices and execute cross-index queries
// ═════════════════════════════════════════════════════════════════

class SearchEngine {
public:
    static constexpr size_t MAX_INDICES = 64;

    // ── Index management ─────────────────────────────────────────
    bool registerIndex(const SearchIndex& index) {
        if (!index.isValid()) return false;
        for (auto& idx : indices_) if (idx.name() == index.name()) return false;
        if (indices_.size() >= MAX_INDICES) return false;
        indices_.push_back(index);
        return true;
    }

    bool unregisterIndex(const std::string& name) {
        for (auto it = indices_.begin(); it != indices_.end(); ++it) {
            if (it->name() == name) { indices_.erase(it); return true; }
        }
        return false;
    }

    bool isRegistered(const std::string& name) const {
        for (auto& idx : indices_) if (idx.name() == name) return true;
        return false;
    }

    SearchIndex* findIndex(const std::string& name) {
        for (auto& idx : indices_) if (idx.name() == name) return &idx;
        return nullptr;
    }

    const SearchIndex* findIndex(const std::string& name) const {
        for (auto& idx : indices_) if (idx.name() == name) return &idx;
        return nullptr;
    }

    size_t indexCount() const { return indices_.size(); }
    const std::vector<SearchIndex>& allIndices() const { return indices_; }

    // ── Cross-index search ───────────────────────────────────────
    std::vector<SearchResult> search(const SearchQuery& query) const {
        std::vector<SearchResult> allResults;
        if (!query.isValid()) return allResults;

        for (auto& idx : indices_) {
            // Scope filter: if query scope is not All, only search matching indices
            if (query.scope() != SearchScope::All && idx.scope() != query.scope()) continue;

            // Source filter
            if (!query.sourceFilter().empty() && idx.name() != query.sourceFilter()) continue;

            auto indexResults = idx.query(query);
            allResults.insert(allResults.end(), indexResults.begin(), indexResults.end());
        }

        // Re-sort all results by score
        std::sort(allResults.begin(), allResults.end());

        // Enforce max results
        if (allResults.size() > query.maxResults()) {
            allResults.resize(query.maxResults());
        }

        ++totalSearches_;
        totalResults_ += allResults.size();
        return allResults;
    }

    // ── Statistics ───────────────────────────────────────────────
    size_t totalSearches() const { return totalSearches_; }
    size_t totalResults()  const { return totalResults_; }

    size_t totalEntries() const {
        size_t n = 0;
        for (auto& idx : indices_) n += idx.entryCount();
        return n;
    }

    // ── Lifecycle ────────────────────────────────────────────────
    void clear() {
        indices_.clear();
        totalSearches_ = 0;
        totalResults_  = 0;
    }

private:
    std::vector<SearchIndex> indices_;
    mutable size_t           totalSearches_ = 0;
    mutable size_t           totalResults_  = 0;
};

} // namespace NF
