#pragma once
#include "EditorLayout.h"
#include <string>

namespace atlas::editor {

/// Saves and restores editor panel layouts to/from JSON files.
class LayoutPersistence {
public:
    /// Save the current layout to a JSON file.
    /// Returns true on success.
    bool SaveLayout(const EditorLayout& layout, const std::string& path);

    /// Load a layout from a JSON file and apply to the layout.
    /// Restores panel visibility and dock tree structure.
    /// Returns true on success.
    bool LoadLayout(EditorLayout& layout, const std::string& path);

    /// Serialize the layout to a JSON string.
    std::string Serialize(const EditorLayout& layout) const;

    /// Apply a JSON string to the layout.
    bool Deserialize(EditorLayout& layout, const std::string& json);

    /// Get the last error message.
    const std::string& LastError() const { return m_lastError; }

private:
    std::string SerializeDockNode(const DockNode& node, int depth) const;
    bool DeserializeDockNode(DockNode& node, EditorLayout& layout,
                             const std::string& json, size_t& pos);

    std::string m_lastError;
};

}
