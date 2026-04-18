#pragma once
#include "../ui/EditorPanel.h"
#include "../../engine/ui/UIDrawList.h"
#include <string>
#include <vector>
#include <cstdint>

namespace atlas::editor {

struct QuestNode {
    uint64_t id = 0;
    std::string name;
    std::string description;
    std::string nodeType; // "objective", "dialogue", "branch", "reward"
    bool completed = false;
    std::vector<uint64_t> nextNodes; // connections to other nodes
};

class QuestEditorPanel : public EditorPanel {
public:
    const char* Name() const override { return "Quest Editor"; }
    void Draw() override;

    uint64_t AddNode(const QuestNode& node);
    bool RemoveNode(uint64_t id);
    const QuestNode* GetNode(uint64_t id) const;
    std::vector<QuestNode> ListNodes() const;
    size_t NodeCount() const;

    bool ConnectNodes(uint64_t fromId, uint64_t toId);
    bool DisconnectNodes(uint64_t fromId, uint64_t toId);
    std::vector<uint64_t> GetConnections(uint64_t nodeId) const;

    void SelectNode(uint64_t id);
    uint64_t SelectedNodeID() const;

    void SetPreviewMode(bool enabled);
    bool IsPreviewMode() const;

    std::string ExportQuest() const;
    bool ImportQuest(const std::string& json);

    void Clear();

    const atlas::ui::UIDrawList& GetDrawList() const { return m_drawList; }

private:
    std::vector<QuestNode> m_nodes;
    uint64_t m_nextID = 1;
    uint64_t m_selectedID = 0;
    bool m_previewMode = false;
    atlas::ui::UIDrawList m_drawList;
};

} // namespace atlas::editor
