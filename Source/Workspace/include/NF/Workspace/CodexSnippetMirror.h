#pragma once
// NF::Editor — Codex snippet mirror: snippet library with tags, search, and sync state
#include "NF/Core/Core.h"
#include <algorithm>
#include <cstdint>
#include <string>
#include <vector>

namespace NF {

// ── Snippet Language ──────────────────────────────────────────────

enum class SnippetLanguage : uint8_t {
    Any,
    Cpp,
    CSharp,
    Lua,
    Python,
    GLSL,
    HLSL,
    Json,
    Yaml,
    Markdown,
};

inline const char* snippetLanguageName(SnippetLanguage l) {
    switch (l) {
        case SnippetLanguage::Any:      return "Any";
        case SnippetLanguage::Cpp:      return "C++";
        case SnippetLanguage::CSharp:   return "C#";
        case SnippetLanguage::Lua:      return "Lua";
        case SnippetLanguage::Python:   return "Python";
        case SnippetLanguage::GLSL:     return "GLSL";
        case SnippetLanguage::HLSL:     return "HLSL";
        case SnippetLanguage::Json:     return "JSON";
        case SnippetLanguage::Yaml:     return "YAML";
        case SnippetLanguage::Markdown: return "Markdown";
    }
    return "Unknown";
}

// ── Snippet Sync State ────────────────────────────────────────────

enum class SnippetSyncState : uint8_t {
    Local,      // not synced with codex
    Synced,     // matches codex
    Modified,   // locally modified after sync
    Conflict,   // conflicting changes
    Pending,    // awaiting sync
};

inline const char* snippetSyncStateName(SnippetSyncState s) {
    switch (s) {
        case SnippetSyncState::Local:    return "Local";
        case SnippetSyncState::Synced:   return "Synced";
        case SnippetSyncState::Modified: return "Modified";
        case SnippetSyncState::Conflict: return "Conflict";
        case SnippetSyncState::Pending:  return "Pending";
    }
    return "Unknown";
}

// ── Snippet ───────────────────────────────────────────────────────

struct CodexSnippet {
    uint32_t            id       = 0;
    std::string         title;
    std::string         body;
    std::string         description;
    SnippetLanguage     language = SnippetLanguage::Any;
    std::vector<std::string> tags;
    SnippetSyncState    syncState = SnippetSyncState::Local;
    std::string         codexId;   // remote codex identifier
    bool                pinned    = false;

    [[nodiscard]] bool isValid()    const { return id != 0 && !title.empty() && !body.empty(); }
    [[nodiscard]] bool isSynced()   const { return syncState == SnippetSyncState::Synced;    }
    [[nodiscard]] bool hasConflict()const { return syncState == SnippetSyncState::Conflict;  }

    bool hasTag(const std::string& tag) const {
        return std::find(tags.begin(), tags.end(), tag) != tags.end();
    }
    void addTag(const std::string& tag) {
        if (!hasTag(tag)) tags.push_back(tag);
    }
    bool removeTag(const std::string& tag) {
        auto it = std::find(tags.begin(), tags.end(), tag);
        if (it == tags.end()) return false;
        tags.erase(it); return true;
    }
};

// ── Insert Target ─────────────────────────────────────────────────
// Describes where to insert a snippet (panel, cursor position).

struct SnippetInsertTarget {
    std::string panelId;
    int         line     = 0;
    int         column   = 0;
    bool        replaceSelection = false;

    [[nodiscard]] bool isValid() const { return !panelId.empty(); }
};

// ── Codex Snippet Mirror ──────────────────────────────────────────
// Maintains a local mirror of codex snippets with search and sync tracking.

class CodexSnippetMirror {
public:
    static constexpr size_t MAX_SNIPPETS = 2048;

    bool addSnippet(const CodexSnippet& s) {
        if (!s.isValid()) return false;
        if (m_snippets.size() >= MAX_SNIPPETS) return false;
        for (const auto& e : m_snippets) if (e.id == s.id) return false;
        m_snippets.push_back(s);
        return true;
    }

    bool removeSnippet(uint32_t id) {
        for (auto it = m_snippets.begin(); it != m_snippets.end(); ++it) {
            if (it->id == id) { m_snippets.erase(it); return true; }
        }
        return false;
    }

    [[nodiscard]] const CodexSnippet* findSnippet(uint32_t id) const {
        for (const auto& s : m_snippets) if (s.id == id) return &s;
        return nullptr;
    }
    CodexSnippet* findSnippetMut(uint32_t id) {
        for (auto& s : m_snippets) if (s.id == id) return &s;
        return nullptr;
    }

    bool updateBody(uint32_t id, const std::string& newBody) {
        auto* s = findSnippetMut(id);
        if (!s || newBody.empty()) return false;
        s->body = newBody;
        if (s->syncState == SnippetSyncState::Synced)
            s->syncState = SnippetSyncState::Modified;
        return true;
    }

    bool markSynced(uint32_t id, const std::string& codexId) {
        auto* s = findSnippetMut(id);
        if (!s) return false;
        s->syncState = SnippetSyncState::Synced;
        s->codexId   = codexId;
        ++m_syncCount;
        return true;
    }

    bool markConflict(uint32_t id) {
        auto* s = findSnippetMut(id);
        if (!s) return false;
        s->syncState = SnippetSyncState::Conflict;
        return true;
    }

    bool resolveConflict(uint32_t id, bool keepLocal) {
        auto* s = findSnippetMut(id);
        if (!s || s->syncState != SnippetSyncState::Conflict) return false;
        s->syncState = keepLocal ? SnippetSyncState::Modified : SnippetSyncState::Synced;
        return true;
    }

    // Search by query substring in title, description, tags, or body
    [[nodiscard]] std::vector<uint32_t> search(const std::string& query) const {
        std::vector<uint32_t> ids;
        for (const auto& s : m_snippets) {
            if (s.title.find(query)       != std::string::npos ||
                s.description.find(query) != std::string::npos ||
                s.body.find(query)        != std::string::npos  ||
                s.hasTag(query)) {
                ids.push_back(s.id);
            }
        }
        return ids;
    }

    [[nodiscard]] std::vector<uint32_t> filterByLanguage(SnippetLanguage lang) const {
        std::vector<uint32_t> ids;
        for (const auto& s : m_snippets) {
            if (lang == SnippetLanguage::Any || s.language == lang || s.language == SnippetLanguage::Any)
                ids.push_back(s.id);
        }
        return ids;
    }

    [[nodiscard]] std::vector<uint32_t> filterByTag(const std::string& tag) const {
        std::vector<uint32_t> ids;
        for (const auto& s : m_snippets) if (s.hasTag(tag)) ids.push_back(s.id);
        return ids;
    }

    [[nodiscard]] std::vector<uint32_t> pendingSync() const {
        std::vector<uint32_t> ids;
        for (const auto& s : m_snippets)
            if (s.syncState == SnippetSyncState::Pending || s.syncState == SnippetSyncState::Modified)
                ids.push_back(s.id);
        return ids;
    }

    // Register insert target (where snippets can be inserted)
    void registerInsertTarget(const SnippetInsertTarget& target) {
        for (auto& t : m_insertTargets) {
            if (t.panelId == target.panelId) { t = target; return; }
        }
        m_insertTargets.push_back(target);
    }

    bool unregisterInsertTarget(const std::string& panelId) {
        for (auto it = m_insertTargets.begin(); it != m_insertTargets.end(); ++it) {
            if (it->panelId == panelId) { m_insertTargets.erase(it); return true; }
        }
        return false;
    }

    [[nodiscard]] size_t snippetCount()        const { return m_snippets.size();       }
    [[nodiscard]] size_t syncCount()           const { return m_syncCount;             }
    [[nodiscard]] size_t insertTargetCount()   const { return m_insertTargets.size();  }
    [[nodiscard]] const std::vector<CodexSnippet>& snippets() const { return m_snippets; }

    [[nodiscard]] size_t conflictCount() const {
        size_t n = 0;
        for (const auto& s : m_snippets) if (s.hasConflict()) ++n;
        return n;
    }

private:
    std::vector<CodexSnippet>        m_snippets;
    std::vector<SnippetInsertTarget> m_insertTargets;
    size_t                           m_syncCount = 0;
};

} // namespace NF
