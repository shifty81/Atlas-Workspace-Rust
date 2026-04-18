#include "SceneBookmarkManager.h"
#include <algorithm>

namespace atlas::tools {

uint32_t SceneBookmarkManager::Save(const SceneBookmark& bm) {
    SceneBookmark copy = bm;
    copy.id = m_nextId++;
    m_bookmarks.push_back(std::move(copy));
    return m_bookmarks.back().id;
}

const SceneBookmark* SceneBookmarkManager::Get(uint32_t id) const {
    for (const auto& b : m_bookmarks) {
        if (b.id == id) return &b;
    }
    return nullptr;
}

const SceneBookmark* SceneBookmarkManager::GetByName(const std::string& name) const {
    for (const auto& b : m_bookmarks) {
        if (b.name == name) return &b;
    }
    return nullptr;
}

bool SceneBookmarkManager::Remove(uint32_t id) {
    auto it = std::remove_if(m_bookmarks.begin(), m_bookmarks.end(),
                              [id](const SceneBookmark& b) { return b.id == id; });
    if (it == m_bookmarks.end()) return false;
    m_bookmarks.erase(it, m_bookmarks.end());
    return true;
}

const std::vector<SceneBookmark>& SceneBookmarkManager::All() const {
    return m_bookmarks;
}

size_t SceneBookmarkManager::Count() const {
    return m_bookmarks.size();
}

void SceneBookmarkManager::Clear() {
    m_bookmarks.clear();
    m_nextId = 1;
}

} // namespace atlas::tools
