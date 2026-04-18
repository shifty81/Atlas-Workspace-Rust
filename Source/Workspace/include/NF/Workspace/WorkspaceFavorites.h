#pragma once
// NF::Workspace — Phase 55: Workspace Favorites System
//
// A workspace-wide favorites system for starring/bookmarking items
// (assets, tools, scenes, files) for quick access.
//
//   FavoriteItemKind  — Asset / Tool / Scene / File / Panel / Custom
//   FavoriteItem      — id + label + kind + path + iconKey + addedMs; isValid()
//   FavoriteFolder    — named folder of favorites (MAX_ITEMS=128); add/remove/find/reorder
//   FavoritesManager  — folder registry (MAX_FOLDERS=32); addFolder/removeFolder/findFolder;
//                       addItem/removeItem shorthand; globalFavorites() (merge all, sorted by addedMs);
//                       observer callbacks (MAX_OBSERVERS=16); serialize()/deserialize()

#include <algorithm>
#include <cstdint>
#include <functional>
#include <string>
#include <vector>

namespace NF {

// ═════════════════════════════════════════════════════════════════
// FavoriteItemKind
// ═════════════════════════════════════════════════════════════════

enum class FavoriteItemKind : uint8_t {
    Asset   = 0,
    Tool    = 1,
    Scene   = 2,
    File    = 3,
    Panel   = 4,
    Custom  = 5,
};

inline const char* favoriteItemKindName(FavoriteItemKind k) {
    switch (k) {
        case FavoriteItemKind::Asset:  return "Asset";
        case FavoriteItemKind::Tool:   return "Tool";
        case FavoriteItemKind::Scene:  return "Scene";
        case FavoriteItemKind::File:   return "File";
        case FavoriteItemKind::Panel:  return "Panel";
        case FavoriteItemKind::Custom: return "Custom";
        default:                       return "Unknown";
    }
}

// ═════════════════════════════════════════════════════════════════
// FavoriteItem
// ═════════════════════════════════════════════════════════════════

struct FavoriteItem {
    std::string      id;
    std::string      label;
    FavoriteItemKind kind    = FavoriteItemKind::Custom;
    std::string      path;     // optional resource path
    std::string      iconKey;  // optional icon identifier
    uint64_t         addedMs = 0;

    [[nodiscard]] bool isValid() const { return !id.empty() && !label.empty(); }

    bool operator==(const FavoriteItem& o) const { return id == o.id; }
    bool operator!=(const FavoriteItem& o) const { return id != o.id; }
};

// ═════════════════════════════════════════════════════════════════
// FavoriteFolder
// ═════════════════════════════════════════════════════════════════

class FavoriteFolder {
public:
    static constexpr int MAX_ITEMS = 128;

    std::string id;
    std::string name;

    [[nodiscard]] bool isValid() const { return !id.empty() && !name.empty(); }

    bool addItem(const FavoriteItem& item) {
        if (!item.isValid()) return false;
        if (findItem(item.id)) return false;
        if (static_cast<int>(m_items.size()) >= MAX_ITEMS) return false;
        m_items.push_back(item);
        return true;
    }

    bool removeItem(const std::string& itemId) {
        auto it = std::find_if(m_items.begin(), m_items.end(),
                               [&](const FavoriteItem& i) { return i.id == itemId; });
        if (it == m_items.end()) return false;
        m_items.erase(it);
        return true;
    }

    const FavoriteItem* findItem(const std::string& itemId) const {
        auto it = std::find_if(m_items.begin(), m_items.end(),
                               [&](const FavoriteItem& i) { return i.id == itemId; });
        return it != m_items.end() ? &(*it) : nullptr;
    }

    bool containsItem(const std::string& itemId) const { return findItem(itemId) != nullptr; }

    bool moveItem(const std::string& itemId, int newIndex) {
        if (newIndex < 0 || newIndex >= static_cast<int>(m_items.size())) return false;
        auto it = std::find_if(m_items.begin(), m_items.end(),
                               [&](const FavoriteItem& i) { return i.id == itemId; });
        if (it == m_items.end()) return false;
        FavoriteItem copy = *it;
        m_items.erase(it);
        m_items.insert(m_items.begin() + newIndex, copy);
        return true;
    }

    [[nodiscard]] int                             count() const { return static_cast<int>(m_items.size()); }
    [[nodiscard]] bool                            empty() const { return m_items.empty(); }
    [[nodiscard]] const std::vector<FavoriteItem>& items() const { return m_items; }

    void clear() { m_items.clear(); }

private:
    std::vector<FavoriteItem> m_items;
};

// ═════════════════════════════════════════════════════════════════
// FavoritesManager
// ═════════════════════════════════════════════════════════════════

class FavoritesManager {
public:
    using FavoritesObserver = std::function<void(const FavoriteItem&, bool added)>;

    static constexpr int MAX_FOLDERS   = 32;
    static constexpr int MAX_OBSERVERS = 16;
    static constexpr int MAX_GLOBAL    = 64;

    // Folder management ────────────────────────────────────────

    bool addFolder(const FavoriteFolder& folder) {
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

    FavoriteFolder* findFolder(const std::string& folderId) {
        auto it = findFolderIt(folderId);
        return it != m_folders.end() ? &(*it) : nullptr;
    }

    const FavoriteFolder* findFolder(const std::string& folderId) const {
        auto it = std::find_if(m_folders.begin(), m_folders.end(),
                               [&](const FavoriteFolder& f) { return f.id == folderId; });
        return it != m_folders.end() ? &(*it) : nullptr;
    }

    bool hasFolder(const std::string& folderId) const { return findFolder(folderId) != nullptr; }

    [[nodiscard]] int folderCount() const { return static_cast<int>(m_folders.size()); }

    [[nodiscard]] const std::vector<FavoriteFolder>& folders() const { return m_folders; }

    // Item shortcuts (operate on a specific folder) ────────────

    bool addItem(const std::string& folderId, const FavoriteItem& item) {
        auto* folder = findFolder(folderId);
        if (!folder) return false;
        if (!folder->addItem(item)) return false;
        notifyObservers(item, true);
        return true;
    }

    bool removeItem(const std::string& folderId, const std::string& itemId) {
        auto* folder = findFolder(folderId);
        if (!folder) return false;
        const auto* item = folder->findItem(itemId);
        if (!item) return false;
        FavoriteItem copy = *item;
        if (!folder->removeItem(itemId)) return false;
        notifyObservers(copy, false);
        return true;
    }

    // Global view ──────────────────────────────────────────────

    [[nodiscard]] std::vector<FavoriteItem> globalFavorites() const {
        std::vector<FavoriteItem> all;
        for (const auto& folder : m_folders) {
            for (const auto& item : folder.items()) {
                // Dedup by id
                bool dup = false;
                for (const auto& existing : all) {
                    if (existing.id == item.id) { dup = true; break; }
                }
                if (!dup) all.push_back(item);
            }
        }
        std::sort(all.begin(), all.end(), [](const FavoriteItem& a, const FavoriteItem& b) {
            return a.addedMs > b.addedMs; // newest first
        });
        if (static_cast<int>(all.size()) > MAX_GLOBAL) {
            all.resize(MAX_GLOBAL);
        }
        return all;
    }

    // Observers ────────────────────────────────────────────────

    bool addObserver(FavoritesObserver cb) {
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
            out += "[" + escPipe(folder.id) + "|" + escPipe(folder.name) + "]\n";
            for (const auto& item : folder.items()) {
                out += escPipe(item.id) + "|" + escPipe(item.label) + "|"
                     + std::to_string(static_cast<int>(item.kind)) + "|"
                     + escPipe(item.path) + "|" + escPipe(item.iconKey) + "|"
                     + std::to_string(item.addedMs) + "\n";
            }
        }
        return out;
    }

    bool deserialize(const std::string& text) {
        m_folders.clear();
        if (text.empty()) return true;

        FavoriteFolder* current = nullptr;
        size_t pos = 0;
        while (pos < text.size()) {
            size_t eol = text.find('\n', pos);
            if (eol == std::string::npos) eol = text.size();
            std::string line = text.substr(pos, eol - pos);
            pos = eol + 1;
            if (line.empty()) continue;

            if (line.front() == '[' && line.back() == ']') {
                std::string inner = line.substr(1, line.size() - 2);
                auto sep = findUnescapedPipe(inner, 0);
                if (sep == std::string::npos) continue;
                FavoriteFolder folder;
                folder.id   = unescPipe(inner.substr(0, sep));
                folder.name = unescPipe(inner.substr(sep + 1));
                m_folders.push_back(std::move(folder));
                current = &m_folders.back();
            } else if (current) {
                // Parse item: id|label|kind|path|iconKey|addedMs
                std::vector<std::string> fields;
                size_t start = 0;
                while (fields.size() < 6) {
                    auto p = findUnescapedPipe(line, start);
                    if (p == std::string::npos) {
                        fields.push_back(line.substr(start));
                        break;
                    }
                    fields.push_back(line.substr(start, p - start));
                    start = p + 1;
                }
                if (fields.size() >= 6) {
                    FavoriteItem item;
                    item.id      = unescPipe(fields[0]);
                    item.label   = unescPipe(fields[1]);
                    item.kind    = static_cast<FavoriteItemKind>(std::stoi(fields[2]));
                    item.path    = unescPipe(fields[3]);
                    item.iconKey = unescPipe(fields[4]);
                    item.addedMs = std::stoull(fields[5]);
                    current->addItem(item);
                }
            }
        }
        return true;
    }

    // Utility ──────────────────────────────────────────────────

    void clear() {
        m_folders.clear();
        m_observers.clear();
    }

private:
    std::vector<FavoriteFolder>    m_folders;
    std::vector<FavoritesObserver> m_observers;

    std::vector<FavoriteFolder>::iterator findFolderIt(const std::string& id) {
        return std::find_if(m_folders.begin(), m_folders.end(),
                            [&](const FavoriteFolder& f) { return f.id == id; });
    }

    void notifyObservers(const FavoriteItem& item, bool added) {
        for (auto& cb : m_observers) {
            if (cb) cb(item, added);
        }
    }

    static std::string escPipe(const std::string& s) {
        std::string out;
        out.reserve(s.size());
        for (char c : s) {
            if (c == '|')  out += "\\P";
            else if (c == '\\') out += "\\\\";
            else           out += c;
        }
        return out;
    }

    static std::string unescPipe(const std::string& s) {
        std::string out;
        out.reserve(s.size());
        for (size_t i = 0; i < s.size(); ++i) {
            if (s[i] == '\\' && i + 1 < s.size()) {
                if (s[i + 1] == 'P') { out += '|'; ++i; }
                else if (s[i + 1] == '\\') { out += '\\'; ++i; }
                else out += s[i];
            } else {
                out += s[i];
            }
        }
        return out;
    }

    static size_t findUnescapedPipe(const std::string& s, size_t start) {
        for (size_t i = start; i < s.size(); ++i) {
            if (s[i] == '\\' && i + 1 < s.size()) { ++i; continue; }
            if (s[i] == '|') return i;
        }
        return std::string::npos;
    }
};

} // namespace NF
