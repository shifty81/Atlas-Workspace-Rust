#pragma once
// NF::Workspace — Phase 57: Workspace Snippet Library
//
// Reusable code/content snippet storage and retrieval with tagging,
// folder organization, and search.
//
//   SnippetLanguage — None / Cpp / HLSL / GLSL / Python / Lua / JSON / XML / Custom
//   SnippetEntry    — id + title + body + language + tags + createdMs + modifiedMs; isValid()
//   SnippetFolder   — named folder of snippets (MAX_SNIPPETS=256)
//   SnippetLibrary  — folder registry (MAX_FOLDERS=32); add/remove/search;
//                     observer callbacks; serialize()/deserialize()

#include <algorithm>
#include <cstdint>
#include <functional>
#include <string>
#include <vector>

namespace NF {

// ═════════════════════════════════════════════════════════════════
// SnippetLanguage
// ═════════════════════════════════════════════════════════════════

enum class SnippetLanguage : uint8_t {
    None    = 0,
    Cpp     = 1,
    HLSL    = 2,
    GLSL    = 3,
    Python  = 4,
    Lua     = 5,
    JSON    = 6,
    XML     = 7,
    Custom  = 8,
};

inline const char* snippetLanguageName(SnippetLanguage l) {
    switch (l) {
        case SnippetLanguage::None:   return "None";
        case SnippetLanguage::Cpp:    return "C++";
        case SnippetLanguage::HLSL:   return "HLSL";
        case SnippetLanguage::GLSL:   return "GLSL";
        case SnippetLanguage::Python: return "Python";
        case SnippetLanguage::Lua:    return "Lua";
        case SnippetLanguage::JSON:   return "JSON";
        case SnippetLanguage::XML:    return "XML";
        case SnippetLanguage::Custom: return "Custom";
        default:                      return "Unknown";
    }
}

// ═════════════════════════════════════════════════════════════════
// SnippetEntry
// ═════════════════════════════════════════════════════════════════

struct SnippetEntry {
    std::string              id;
    std::string              title;
    std::string              body;
    SnippetLanguage          language   = SnippetLanguage::None;
    std::vector<std::string> tags;       // e.g. {"math", "utility"}
    std::string              description;
    uint64_t                 createdMs  = 0;
    uint64_t                 modifiedMs = 0;

    static constexpr int MAX_TAGS = 16;

    [[nodiscard]] bool isValid() const { return !id.empty() && !title.empty(); }

    bool addTag(const std::string& tag) {
        if (tag.empty()) return false;
        if (hasTag(tag)) return false;
        if (static_cast<int>(tags.size()) >= MAX_TAGS) return false;
        tags.push_back(tag);
        return true;
    }

    bool removeTag(const std::string& tag) {
        auto it = std::find(tags.begin(), tags.end(), tag);
        if (it == tags.end()) return false;
        tags.erase(it);
        return true;
    }

    [[nodiscard]] bool hasTag(const std::string& tag) const {
        return std::find(tags.begin(), tags.end(), tag) != tags.end();
    }

    bool operator==(const SnippetEntry& o) const { return id == o.id; }
    bool operator!=(const SnippetEntry& o) const { return id != o.id; }
};

// ═════════════════════════════════════════════════════════════════
// SnippetFolder
// ═════════════════════════════════════════════════════════════════

class SnippetFolder {
public:
    static constexpr int MAX_SNIPPETS = 256;

    std::string id;
    std::string name;

    [[nodiscard]] bool isValid() const { return !id.empty() && !name.empty(); }

    bool addSnippet(const SnippetEntry& snippet) {
        if (!snippet.isValid()) return false;
        if (findSnippet(snippet.id)) return false;
        if (static_cast<int>(m_snippets.size()) >= MAX_SNIPPETS) return false;
        m_snippets.push_back(snippet);
        return true;
    }

    bool removeSnippet(const std::string& snippetId) {
        auto it = std::find_if(m_snippets.begin(), m_snippets.end(),
                               [&](const SnippetEntry& s) { return s.id == snippetId; });
        if (it == m_snippets.end()) return false;
        m_snippets.erase(it);
        return true;
    }

    const SnippetEntry* findSnippet(const std::string& snippetId) const {
        auto it = std::find_if(m_snippets.begin(), m_snippets.end(),
                               [&](const SnippetEntry& s) { return s.id == snippetId; });
        return it != m_snippets.end() ? &(*it) : nullptr;
    }

    SnippetEntry* findSnippetMut(const std::string& snippetId) {
        auto it = std::find_if(m_snippets.begin(), m_snippets.end(),
                               [&](const SnippetEntry& s) { return s.id == snippetId; });
        return it != m_snippets.end() ? &(*it) : nullptr;
    }

    bool containsSnippet(const std::string& snippetId) const { return findSnippet(snippetId) != nullptr; }

    [[nodiscard]] int                              count()    const { return static_cast<int>(m_snippets.size()); }
    [[nodiscard]] bool                             empty()    const { return m_snippets.empty(); }
    [[nodiscard]] const std::vector<SnippetEntry>& snippets() const { return m_snippets; }

    void clear() { m_snippets.clear(); }

private:
    std::vector<SnippetEntry> m_snippets;
};

// ═════════════════════════════════════════════════════════════════
// SnippetLibrary
// ═════════════════════════════════════════════════════════════════

class SnippetLibrary {
public:
    using SnippetObserver = std::function<void(const SnippetEntry&, bool added)>;

    static constexpr int MAX_FOLDERS   = 32;
    static constexpr int MAX_OBSERVERS = 16;

    // Folder management ────────────────────────────────────────

    bool addFolder(const SnippetFolder& folder) {
        if (!folder.isValid()) return false;
        if (findFolder(folder.id)) return false;
        if (static_cast<int>(m_folders.size()) >= MAX_FOLDERS) return false;
        m_folders.push_back(folder);
        return true;
    }

    bool removeFolder(const std::string& folderId) {
        auto it = findFolderIt(folderId);
        if (it == m_folders.end()) return false;
        m_folders.erase(it);
        return true;
    }

    SnippetFolder* findFolder(const std::string& folderId) {
        auto it = findFolderIt(folderId);
        return it != m_folders.end() ? &(*it) : nullptr;
    }

    const SnippetFolder* findFolder(const std::string& folderId) const {
        auto it = std::find_if(m_folders.begin(), m_folders.end(),
                               [&](const SnippetFolder& f) { return f.id == folderId; });
        return it != m_folders.end() ? &(*it) : nullptr;
    }

    bool hasFolder(const std::string& folderId) const { return findFolder(folderId) != nullptr; }

    [[nodiscard]] int folderCount() const { return static_cast<int>(m_folders.size()); }

    [[nodiscard]] const std::vector<SnippetFolder>& folders() const { return m_folders; }

    // Snippet shortcuts ────────────────────────────────────────

    bool addSnippet(const std::string& folderId, const SnippetEntry& snippet) {
        auto* folder = findFolder(folderId);
        if (!folder) return false;
        if (!folder->addSnippet(snippet)) return false;
        notifyObservers(snippet, true);
        return true;
    }

    bool removeSnippet(const std::string& folderId, const std::string& snippetId) {
        auto* folder = findFolder(folderId);
        if (!folder) return false;
        const auto* snippet = folder->findSnippet(snippetId);
        if (!snippet) return false;
        SnippetEntry copy = *snippet;
        if (!folder->removeSnippet(snippetId)) return false;
        notifyObservers(copy, false);
        return true;
    }

    // Search ───────────────────────────────────────────────────

    [[nodiscard]] std::vector<const SnippetEntry*> searchByTag(const std::string& tag) const {
        std::vector<const SnippetEntry*> results;
        for (const auto& folder : m_folders) {
            for (const auto& snippet : folder.snippets()) {
                if (snippet.hasTag(tag)) {
                    results.push_back(&snippet);
                }
            }
        }
        return results;
    }

    [[nodiscard]] std::vector<const SnippetEntry*> searchByLanguage(SnippetLanguage lang) const {
        std::vector<const SnippetEntry*> results;
        for (const auto& folder : m_folders) {
            for (const auto& snippet : folder.snippets()) {
                if (snippet.language == lang) {
                    results.push_back(&snippet);
                }
            }
        }
        return results;
    }

    [[nodiscard]] std::vector<const SnippetEntry*> searchByText(const std::string& query) const {
        std::vector<const SnippetEntry*> results;
        if (query.empty()) return results;
        std::string lowerQuery = toLower(query);
        for (const auto& folder : m_folders) {
            for (const auto& snippet : folder.snippets()) {
                if (toLower(snippet.title).find(lowerQuery) != std::string::npos
                    || toLower(snippet.body).find(lowerQuery) != std::string::npos
                    || toLower(snippet.description).find(lowerQuery) != std::string::npos) {
                    results.push_back(&snippet);
                }
            }
        }
        return results;
    }

    [[nodiscard]] int totalSnippets() const {
        int total = 0;
        for (const auto& folder : m_folders) total += folder.count();
        return total;
    }

    // Observers ────────────────────────────────────────────────

    bool addObserver(SnippetObserver cb) {
        if (!cb) return false;
        if (static_cast<int>(m_observers.size()) >= MAX_OBSERVERS) return false;
        m_observers.push_back(std::move(cb));
        return true;
    }

    void clearObservers() { m_observers.clear(); }

    // Serialization ────────────────────────────────────────────

    [[nodiscard]] std::string serialize() const {
        std::string out;
        for (const auto& folder : m_folders) {
            out += "[" + esc(folder.id) + "|" + esc(folder.name) + "]\n";
            for (const auto& snippet : folder.snippets()) {
                // id|title|language|description|createdMs|modifiedMs|tagCount|tag1|...|bodyLineCount
                out += esc(snippet.id) + "|" + esc(snippet.title) + "|"
                     + std::to_string(static_cast<int>(snippet.language)) + "|"
                     + esc(snippet.description) + "|"
                     + std::to_string(snippet.createdMs) + "|"
                     + std::to_string(snippet.modifiedMs) + "|"
                     + std::to_string(snippet.tags.size());
                for (const auto& tag : snippet.tags) {
                    out += "|" + esc(tag);
                }
                // Body: count lines, write each as BODY:line
                auto lines = splitLines(snippet.body);
                out += "|" + std::to_string(lines.size()) + "\n";
                for (const auto& line : lines) {
                    out += "BODY:" + esc(line) + "\n";
                }
            }
        }
        return out;
    }

    bool deserialize(const std::string& text) {
        m_folders.clear();
        if (text.empty()) return true;

        SnippetFolder* current = nullptr;
        size_t pos = 0;
        while (pos < text.size()) {
            size_t eol = text.find('\n', pos);
            if (eol == std::string::npos) eol = text.size();
            std::string line = text.substr(pos, eol - pos);
            pos = eol + 1;
            if (line.empty()) continue;

            if (line.front() == '[' && line.back() == ']') {
                std::string inner = line.substr(1, line.size() - 2);
                auto sep = findPipe(inner, 0);
                if (sep == std::string::npos) continue;
                SnippetFolder folder;
                folder.id   = unesc(inner.substr(0, sep));
                folder.name = unesc(inner.substr(sep + 1));
                m_folders.push_back(std::move(folder));
                current = &m_folders.back();
            } else if (current && line.substr(0, 5) != "BODY:") {
                // Parse snippet header
                auto fields = splitPipe(line);
                // id|title|lang|desc|created|modified|tagCount|tags...|bodyLineCount
                if (fields.size() < 8) continue;
                SnippetEntry snippet;
                snippet.id          = unesc(fields[0]);
                snippet.title       = unesc(fields[1]);
                snippet.language    = static_cast<SnippetLanguage>(std::stoi(fields[2]));
                snippet.description = unesc(fields[3]);
                snippet.createdMs   = std::stoull(fields[4]);
                snippet.modifiedMs  = std::stoull(fields[5]);
                int tagCount        = std::stoi(fields[6]);
                for (int i = 0; i < tagCount && (7 + i) < static_cast<int>(fields.size()); ++i) {
                    snippet.addTag(unesc(fields[7 + i]));
                }
                int bodyLineCountIdx = 7 + tagCount;
                int bodyLineCount = 0;
                if (bodyLineCountIdx < static_cast<int>(fields.size())) {
                    bodyLineCount = std::stoi(fields[bodyLineCountIdx]);
                }
                // Read body lines
                std::string body;
                for (int b = 0; b < bodyLineCount && pos < text.size(); ++b) {
                    size_t beol = text.find('\n', pos);
                    if (beol == std::string::npos) beol = text.size();
                    std::string bline = text.substr(pos, beol - pos);
                    pos = beol + 1;
                    if (bline.substr(0, 5) == "BODY:") {
                        if (!body.empty()) body += "\n";
                        body += unesc(bline.substr(5));
                    }
                }
                snippet.body = body;
                current->addSnippet(snippet);
            }
        }
        return true;
    }

    void clear() {
        m_folders.clear();
        m_observers.clear();
    }

private:
    std::vector<SnippetFolder>    m_folders;
    std::vector<SnippetObserver>  m_observers;

    std::vector<SnippetFolder>::iterator findFolderIt(const std::string& id) {
        return std::find_if(m_folders.begin(), m_folders.end(),
                            [&](const SnippetFolder& f) { return f.id == id; });
    }

    void notifyObservers(const SnippetEntry& snippet, bool added) {
        for (auto& cb : m_observers) {
            if (cb) cb(snippet, added);
        }
    }

    static std::string esc(const std::string& s) {
        std::string out;
        out.reserve(s.size());
        for (char c : s) {
            if (c == '|')       out += "\\P";
            else if (c == '\\') out += "\\\\";
            else if (c == '\n') out += "\\N";
            else                out += c;
        }
        return out;
    }

    static std::string unesc(const std::string& s) {
        std::string out;
        out.reserve(s.size());
        for (size_t i = 0; i < s.size(); ++i) {
            if (s[i] == '\\' && i + 1 < s.size()) {
                if (s[i + 1] == 'P')  { out += '|'; ++i; }
                else if (s[i + 1] == '\\') { out += '\\'; ++i; }
                else if (s[i + 1] == 'N')  { out += '\n'; ++i; }
                else out += s[i];
            } else {
                out += s[i];
            }
        }
        return out;
    }

    static size_t findPipe(const std::string& s, size_t start) {
        for (size_t i = start; i < s.size(); ++i) {
            if (s[i] == '\\' && i + 1 < s.size()) { ++i; continue; }
            if (s[i] == '|') return i;
        }
        return std::string::npos;
    }

    static std::vector<std::string> splitPipe(const std::string& s) {
        std::vector<std::string> fields;
        size_t start = 0;
        while (start <= s.size()) {
            auto p = findPipe(s, start);
            if (p == std::string::npos) {
                fields.push_back(s.substr(start));
                break;
            }
            fields.push_back(s.substr(start, p - start));
            start = p + 1;
        }
        return fields;
    }

    static std::vector<std::string> splitLines(const std::string& s) {
        std::vector<std::string> lines;
        if (s.empty()) { lines.push_back(""); return lines; }
        size_t start = 0;
        while (start <= s.size()) {
            auto p = s.find('\n', start);
            if (p == std::string::npos) {
                lines.push_back(s.substr(start));
                break;
            }
            lines.push_back(s.substr(start, p - start));
            start = p + 1;
        }
        return lines;
    }

    static std::string toLower(const std::string& s) {
        std::string out = s;
        for (auto& c : out) c = static_cast<char>(std::tolower(static_cast<unsigned char>(c)));
        return out;
    }
};

} // namespace NF
