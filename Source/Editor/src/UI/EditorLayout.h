#pragma once
#include "DockNode.h"
#include "EditorMenuBar.h"
#include <vector>
#include <memory>
#include <string>

namespace atlas::editor {

class EditorLayout {
public:
    void RegisterPanel(EditorPanel* panel);
    void Draw();

    /// Attempt to remove a panel from the layout.  Returns false if the
    /// panel is marked as non-closable.
    bool ClosePanel(const std::string& name);

    /// Find a registered panel by name.
    EditorPanel* FindPanel(const std::string& name) const;

    DockNode& Root() { return m_root; }
    const DockNode& Root() const { return m_root; }
    const std::vector<EditorPanel*>& Panels() const { return m_panels; }

    EditorMenuBar& MenuBar() { return m_menuBar; }
    const EditorMenuBar& MenuBar() const { return m_menuBar; }

    std::string SerializeToJSON() const;
    bool DeserializeFromJSON(const std::string& json);

    bool SaveToFile(const std::string& path) const;
    bool LoadFromFile(const std::string& path);

private:
    DockNode m_root;
    std::vector<EditorPanel*> m_panels;
    EditorMenuBar m_menuBar;

    static void DrawNode(DockNode& node);
};

}
