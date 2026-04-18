#pragma once
#include <cstdint>
#include <string>
#include <vector>

namespace atlas::tools {

/// A saved camera / scene checkpoint that can be restored later.
struct SceneBookmark {
    uint32_t id = 0;
    std::string name;

    // Camera state
    float posX = 0.0f, posY = 0.0f, posZ = 0.0f;
    float yaw = 0.0f, pitch = 0.0f;
    float zoom = 1.0f;

    // Optional metadata
    std::string description;
    uint64_t timestamp = 0;  ///< tick or wall-clock when created
};

/// Manages a collection of scene bookmarks (camera + scene checkpoints).
///
/// Bookmarks are stored in insertion order and can be looked up by id or
/// by name.  The manager does NOT own the camera — callers must copy
/// camera state in/out of SceneBookmark themselves.
class SceneBookmarkManager {
public:
    /// Save a new bookmark and return its id.
    uint32_t Save(const SceneBookmark& bm);

    /// Retrieve a bookmark by id (nullptr if not found).
    const SceneBookmark* Get(uint32_t id) const;

    /// Retrieve by name (first match; nullptr if not found).
    const SceneBookmark* GetByName(const std::string& name) const;

    /// Remove a bookmark by id.  Returns true if found & removed.
    bool Remove(uint32_t id);

    /// All bookmarks (order of creation).
    const std::vector<SceneBookmark>& All() const;

    /// Number of stored bookmarks.
    size_t Count() const;

    /// Remove all bookmarks.
    void Clear();

private:
    std::vector<SceneBookmark> m_bookmarks;
    uint32_t m_nextId = 1;
};

} // namespace atlas::tools
