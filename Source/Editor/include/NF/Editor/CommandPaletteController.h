#pragma once
// NF::Editor — Phase H.4: Command Palette Controller
//
// Wraps CommandPalette with UX-level features: open/close toggle (Ctrl+P),
// context-aware filtering, fuzzy search, recent commands display, and
// keyboard navigation support.
//
//   CommandPaletteContext  — current tool / panel / scope for context filtering
//   CommandPaletteController —
//       open() / close() / toggle() / isOpen()
//       setQuery(text)           — update search text
//       query()                  — current search text
//       results()                — filtered CommandEntry list (fuzzy, context-aware)
//       recentResults()          — recent command ids from history
//       setContext(context)      — update current context for filtering
//       executeSelected()        — execute the currently highlighted result
//       moveSelectionUp/Down()   — keyboard navigation within results
//       selectedIndex()          — currently highlighted result index
//       selectIndex(n)           — set highlighted result programmatically
//       registerCommand(entry)   — delegate to underlying CommandPalette
//       palette()                — access underlying CommandPalette

#include "NF/Workspace/WorkspaceCommandPalette.h"
#include <algorithm>
#include <cctype>
#include <functional>
#include <string>
#include <vector>

namespace NF {

// ═════════════════════════════════════════════════════════════════
// CommandPaletteContext
// ═════════════════════════════════════════════════════════════════

struct CommandPaletteContext {
    std::string activeTool;    // e.g. "scene_editor", "material_editor"
    std::string activePanel;   // e.g. "inspector", "content_browser"
    std::string scope;         // e.g. "global", "project", "selection"

    [[nodiscard]] bool isEmpty() const {
        return activeTool.empty() && activePanel.empty() && scope.empty();
    }
};

// ═════════════════════════════════════════════════════════════════
// CommandPaletteController
// ═════════════════════════════════════════════════════════════════

class CommandPaletteController {
public:
    static constexpr int MAX_RESULTS = 64;

    // ── Open / close / toggle ─────────────────────────────────────

    void open() {
        m_isOpen        = true;
        m_selectedIndex = 0;
        rebuildResults();
    }

    void close() {
        m_isOpen = false;
        m_query.clear();
        m_results.clear();
        m_selectedIndex = 0;
    }

    void toggle() {
        if (m_isOpen) close();
        else          open();
    }

    [[nodiscard]] bool isOpen() const { return m_isOpen; }

    // ── Query / search ────────────────────────────────────────────

    void setQuery(const std::string& query) {
        m_query         = query;
        m_selectedIndex = 0;
        rebuildResults();
    }

    [[nodiscard]] const std::string& query() const { return m_query; }

    // ── Context ───────────────────────────────────────────────────

    void setContext(const CommandPaletteContext& ctx) {
        m_context = ctx;
        rebuildResults();
    }

    [[nodiscard]] const CommandPaletteContext& context() const { return m_context; }

    // ── Results ───────────────────────────────────────────────────

    [[nodiscard]] const std::vector<const CommandEntry*>& results() const { return m_results; }

    [[nodiscard]] int resultCount() const { return static_cast<int>(m_results.size()); }

    // ── Recent commands ───────────────────────────────────────────

    [[nodiscard]] std::vector<const CommandEntry*> recentResults() const {
        std::vector<const CommandEntry*> recents;
        for (const auto& id : m_palette.history().entries()) {
            const CommandEntry* e = m_palette.find(id);
            if (e) recents.push_back(e);
        }
        return recents;
    }

    // ── Keyboard navigation ───────────────────────────────────────

    void moveSelectionUp() {
        if (m_results.empty()) return;
        if (m_selectedIndex > 0) --m_selectedIndex;
    }

    void moveSelectionDown() {
        if (m_results.empty()) return;
        if (m_selectedIndex < resultCount() - 1) ++m_selectedIndex;
    }

    void selectIndex(int index) {
        if (index >= 0 && index < resultCount())
            m_selectedIndex = index;
    }

    [[nodiscard]] int selectedIndex() const { return m_selectedIndex; }

    [[nodiscard]] const CommandEntry* selectedEntry() const {
        if (m_results.empty() || m_selectedIndex < 0
            || m_selectedIndex >= resultCount()) return nullptr;
        return m_results[static_cast<size_t>(m_selectedIndex)];
    }

    // ── Execute ───────────────────────────────────────────────────

    bool executeSelected() {
        const CommandEntry* entry = selectedEntry();
        if (!entry) return false;
        bool ok = m_palette.execute(entry->id);
        if (m_closeOnExecute) close();
        return ok;
    }

    bool executeById(const std::string& id) {
        return m_palette.execute(id);
    }

    void setCloseOnExecute(bool v) { m_closeOnExecute = v; }
    [[nodiscard]] bool closeOnExecute() const { return m_closeOnExecute; }

    // ── Command registration ──────────────────────────────────────

    bool registerCommand(CommandEntry entry) {
        return m_palette.registerCommand(std::move(entry));
    }

    bool unregisterCommand(const std::string& id) {
        return m_palette.unregisterCommand(id);
    }

    // ── Underlying palette ────────────────────────────────────────

    [[nodiscard]] CommandPalette&       palette()       { return m_palette; }
    [[nodiscard]] const CommandPalette& palette() const { return m_palette; }

private:
    CommandPalette          m_palette;
    CommandPaletteContext   m_context;
    std::string             m_query;
    bool                    m_isOpen        = false;
    int                     m_selectedIndex = 0;
    bool                    m_closeOnExecute = true;
    std::vector<const CommandEntry*> m_results;

    // ── Fuzzy match helper ────────────────────────────────────────
    // Returns a score > 0 if query matches label (case-insensitive subsequence).
    // Higher score = better match.

    static int fuzzyScore(const std::string& query, const std::string& label) {
        if (query.empty()) return 1; // empty query matches everything with low score
        std::string lq = toLower(query);
        std::string ll = toLower(label);
        // Exact substring — highest score
        if (ll.find(lq) != std::string::npos) return 100;
        // Subsequence match
        size_t qi = 0;
        for (size_t li = 0; li < ll.size() && qi < lq.size(); ++li) {
            if (ll[li] == lq[qi]) ++qi;
        }
        if (qi == lq.size()) return 50;
        return 0;
    }

    static std::string toLower(const std::string& s) {
        std::string r = s;
        for (auto& c : r) c = static_cast<char>(std::tolower(static_cast<unsigned char>(c)));
        return r;
    }

    // ── Context filter ────────────────────────────────────────────
    // Commands tagged with a shortcut matching the active tool are promoted.
    // For now: all commands pass — real UX context filtering would use
    // CommandEntry metadata once tags are added.

    bool passesContextFilter(const CommandEntry& entry) const {
        (void)entry;
        return true; // all commands visible; context promotes, not excludes
    }

    void rebuildResults() {
        m_results.clear();

        struct Scored { const CommandEntry* entry; int score; };
        std::vector<Scored> scored;

        for (const auto& cmd : m_palette.commands()) {
            if (!passesContextFilter(cmd)) continue;
            int s = fuzzyScore(m_query, cmd.label);
            if (s == 0 && !m_query.empty()) {
                // also try description
                s = fuzzyScore(m_query, cmd.description);
            }
            if (s > 0 || m_query.empty()) {
                scored.push_back({&cmd, s});
            }
        }

        // Sort: higher score first, then by label
        std::stable_sort(scored.begin(), scored.end(),
            [](const Scored& a, const Scored& b) {
                if (a.score != b.score) return a.score > b.score;
                return a.entry->label < b.entry->label;
            });

        int count = 0;
        for (const auto& s : scored) {
            if (count >= MAX_RESULTS) break;
            m_results.push_back(s.entry);
            ++count;
        }

        // Clamp selection
        if (m_selectedIndex >= resultCount()) {
            m_selectedIndex = resultCount() > 0 ? resultCount() - 1 : 0;
        }
    }
};

} // namespace NF
