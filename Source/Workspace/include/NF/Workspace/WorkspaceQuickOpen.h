#pragma once
// NF::Workspace — Phase 50: Workspace Quick-Open Palette
//
// Cmd+P / Ctrl+P style quick-open: fuzzy-match items from multiple providers
// and deliver a selection result to the caller without touching the UI.
//
//   QuickOpenItemKind  — File / Tool / Command / Symbol / Custom
//   QuickOpenItem      — id + label + detail + kind + score; isValid()
//   QuickOpenQuery     — query string + kind filter + max results;
//                        matches(item) — case-insensitive substring match;
//                        score(item) — Exact(100) > Prefix(60) > Contains(30)
//   QuickOpenProvider  — named source; populate(query) → vector<QuickOpenItem>
//   QuickOpenSession   — holds active query + result set; open/close/submit;
//                        addProvider/removeProvider/hasProvider; isOpen; results
//   QuickOpenManager   — session registry (MAX_SESSIONS=8); createSession/
//                        removeSession/findSession; observer callbacks on submit
//                        (MAX_OBSERVERS=16)

#include <algorithm>
#include <cstdint>
#include <functional>
#include <string>
#include <vector>

namespace NF {

// ═════════════════════════════════════════════════════════════════════════
// QuickOpenItemKind
// ═════════════════════════════════════════════════════════════════════════

enum class QuickOpenItemKind : uint8_t {
    File,
    Tool,
    Command,
    Symbol,
    Custom,
};

inline const char* quickOpenItemKindName(QuickOpenItemKind k) {
    switch (k) {
        case QuickOpenItemKind::File:    return "File";
        case QuickOpenItemKind::Tool:    return "Tool";
        case QuickOpenItemKind::Command: return "Command";
        case QuickOpenItemKind::Symbol:  return "Symbol";
        case QuickOpenItemKind::Custom:  return "Custom";
        default:                         return "Unknown";
    }
}

// ═════════════════════════════════════════════════════════════════════════
// QuickOpenItem
// ═════════════════════════════════════════════════════════════════════════

struct QuickOpenItem {
    std::string      id;
    std::string      label;
    std::string      detail;      // secondary info (path, description, …)
    QuickOpenItemKind kind  = QuickOpenItemKind::Custom;
    int              score  = 0;  // higher = more relevant

    [[nodiscard]] bool isValid() const { return !id.empty() && !label.empty(); }

    bool operator==(const QuickOpenItem& o) const { return id == o.id; }
    bool operator!=(const QuickOpenItem& o) const { return id != o.id; }
};

// ═════════════════════════════════════════════════════════════════════════
// QuickOpenQuery
// ═════════════════════════════════════════════════════════════════════════

struct QuickOpenQuery {
    static constexpr int kScoreExact    = 100;
    static constexpr int kScorePrefix   = 60;
    static constexpr int kScoreContains = 30;
    static constexpr int kScoreNone     = -1;

    std::string       text;
    QuickOpenItemKind filterKind   = QuickOpenItemKind::Custom; // Custom = no filter
    bool              filterByKind = false;
    size_t            maxResults   = 32;

    // Returns true if the item should be included (no text = include all).
    [[nodiscard]] bool matches(const QuickOpenItem& item) const {
        if (filterByKind && item.kind != filterKind) return false;
        if (text.empty()) return true;
        // Case-insensitive label match
        std::string lbl = toLower(item.label);
        std::string q   = toLower(text);
        return lbl.find(q) != std::string::npos;
    }

    // Score: Exact=100 > Prefix=60 > Contains=30 > -1 (no match).
    [[nodiscard]] int score(const QuickOpenItem& item) const {
        if (!matches(item)) return kScoreNone;
        if (text.empty()) return kScoreContains; // all items match at base score
        std::string lbl = toLower(item.label);
        std::string q   = toLower(text);
        if (lbl == q)                       return kScoreExact;
        if (lbl.substr(0, q.size()) == q)   return kScorePrefix;
        return kScoreContains;
    }

private:
    static std::string toLower(const std::string& s) {
        std::string out(s);
        for (char& c : out) c = static_cast<char>(std::tolower(static_cast<unsigned char>(c)));
        return out;
    }
};

// ═════════════════════════════════════════════════════════════════════════
// QuickOpenProvider — pluggable item source
// ═════════════════════════════════════════════════════════════════════════

using QuickOpenPopulateFn = std::function<std::vector<QuickOpenItem>(const QuickOpenQuery&)>;

struct QuickOpenProvider {
    std::string         id;
    std::string         name;
    QuickOpenPopulateFn populate;

    [[nodiscard]] bool isValid() const {
        return !id.empty() && !name.empty() && static_cast<bool>(populate);
    }
};

// ═════════════════════════════════════════════════════════════════════════
// QuickOpenSession — open/close/query/submit lifecycle
// ═════════════════════════════════════════════════════════════════════════

class QuickOpenSession {
public:
    static constexpr size_t MAX_PROVIDERS = 8;

    explicit QuickOpenSession(const std::string& id) : m_id(id) {}

    [[nodiscard]] const std::string& id()     const { return m_id; }
    [[nodiscard]] bool               isOpen() const { return m_open; }

    // ── Provider management ────────────────────────────────────────

    bool addProvider(const QuickOpenProvider& p) {
        if (!p.isValid()) return false;
        if (m_providers.size() >= MAX_PROVIDERS) return false;
        for (const auto& existing : m_providers)
            if (existing.id == p.id) return false;
        m_providers.push_back(p);
        return true;
    }

    bool removeProvider(const std::string& pid) {
        auto it = std::find_if(m_providers.begin(), m_providers.end(),
                               [&](const QuickOpenProvider& p){ return p.id == pid; });
        if (it == m_providers.end()) return false;
        m_providers.erase(it);
        return true;
    }

    [[nodiscard]] bool hasProvider(const std::string& pid) const {
        for (const auto& p : m_providers)
            if (p.id == pid) return true;
        return false;
    }

    [[nodiscard]] size_t providerCount() const { return m_providers.size(); }

    // ── Lifecycle ──────────────────────────────────────────────────

    // Open the palette. Clears previous results.
    void open() {
        m_open     = true;
        m_results.clear();
        m_submitted.reset();
    }

    // Close without submitting.
    void close() {
        m_open = false;
    }

    // Execute query: collect from all providers, score, sort, cap.
    // Returns result count. Does nothing if not open.
    size_t query(const QuickOpenQuery& q) {
        if (!m_open) return 0;
        m_results.clear();
        for (const auto& provider : m_providers) {
            auto items = provider.populate(q);
            for (auto& item : items) {
                int s = q.score(item);
                if (s >= 0) {
                    item.score = s;
                    m_results.push_back(std::move(item));
                }
            }
        }
        // Sort: higher score first; ties broken by label alphabetically
        std::stable_sort(m_results.begin(), m_results.end(),
                         [](const QuickOpenItem& a, const QuickOpenItem& b){
                             if (a.score != b.score) return a.score > b.score;
                             return a.label < b.label;
                         });
        if (m_results.size() > q.maxResults)
            m_results.resize(q.maxResults);
        return m_results.size();
    }

    // Submit item by id. Returns false if not open or id not in results.
    bool submit(const std::string& itemId) {
        if (!m_open) return false;
        for (const auto& item : m_results) {
            if (item.id == itemId) {
                m_submitted = item;
                m_open      = false;
                return true;
            }
        }
        return false;
    }

    [[nodiscard]] const std::vector<QuickOpenItem>& results()    const { return m_results; }
    [[nodiscard]] bool                               hasSubmit()  const { return m_submitted.has_value(); }
    [[nodiscard]] const QuickOpenItem*               submitted()  const {
        return m_submitted.has_value() ? &*m_submitted : nullptr;
    }
    void clearSubmit() { m_submitted.reset(); }

private:
    std::string                    m_id;
    std::vector<QuickOpenProvider> m_providers;
    std::vector<QuickOpenItem>     m_results;
    bool                           m_open = false;

    // Use optional-like pattern without <optional>
    struct OptItem {
        bool           valid = false;
        QuickOpenItem  item;
        bool   has_value() const { return valid; }
        void   reset()           { valid = false; }
        const QuickOpenItem& operator*() const { return item; }
        OptItem& operator=(const QuickOpenItem& i) { item = i; valid = true; return *this; }
    };
    OptItem m_submitted;
};

// ═════════════════════════════════════════════════════════════════════════
// QuickOpenManager — session registry with submit observers
// ═════════════════════════════════════════════════════════════════════════

using QuickOpenObserver = std::function<void(const std::string& /*sessionId*/,
                                             const QuickOpenItem& /*submitted*/)>;

class QuickOpenManager {
public:
    static constexpr size_t MAX_SESSIONS  = 8;
    static constexpr size_t MAX_OBSERVERS = 16;

    // ── Session management ────────────────────────────────────────

    QuickOpenSession* createSession(const std::string& id) {
        if (id.empty() || m_sessions.size() >= MAX_SESSIONS) return nullptr;
        for (const auto& s : m_sessions)
            if (s.id() == id) return nullptr;
        m_sessions.emplace_back(id);
        return &m_sessions.back();
    }

    bool removeSession(const std::string& id) {
        auto it = std::find_if(m_sessions.begin(), m_sessions.end(),
                               [&](const QuickOpenSession& s){ return s.id() == id; });
        if (it == m_sessions.end()) return false;
        m_sessions.erase(it);
        return true;
    }

    [[nodiscard]] QuickOpenSession* findSession(const std::string& id) {
        for (auto& s : m_sessions)
            if (s.id() == id) return &s;
        return nullptr;
    }

    [[nodiscard]] const QuickOpenSession* findSession(const std::string& id) const {
        for (const auto& s : m_sessions)
            if (s.id() == id) return &s;
        return nullptr;
    }

    [[nodiscard]] bool   hasSession(const std::string& id) const {
        return findSession(id) != nullptr;
    }
    [[nodiscard]] size_t sessionCount() const { return m_sessions.size(); }
    [[nodiscard]] bool   empty()        const { return m_sessions.empty(); }

    // ── Wired submit — fire observers ────────────────────────────────

    // Call this after session.submit() to notify observers.
    void notifySubmit(const std::string& sessionId, const QuickOpenItem& item) {
        for (auto& obs : m_observers)
            obs(sessionId, item);
    }

    // ── Observers ─────────────────────────────────────────────────

    bool addObserver(QuickOpenObserver obs) {
        if (m_observers.size() >= MAX_OBSERVERS) return false;
        m_observers.push_back(std::move(obs));
        return true;
    }

    void clearObservers() { m_observers.clear(); }

    // ── Clear ─────────────────────────────────────────────────────

    void clear() {
        m_sessions.clear();
        m_observers.clear();
    }

private:
    std::vector<QuickOpenSession>   m_sessions;
    std::vector<QuickOpenObserver>  m_observers;
};

} // namespace NF
