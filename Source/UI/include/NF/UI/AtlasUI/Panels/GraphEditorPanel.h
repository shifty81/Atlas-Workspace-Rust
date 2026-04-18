#pragma once

#include "NF/UI/AtlasUI/PanelBase.h"
#include "NF/UI/AtlasUI/WidgetTheme.h"
#include "NF/UI/AtlasUI/WidgetHelpers.h"
#include <string>
#include <vector>

namespace NF::UI::AtlasUI {

/// AtlasUI GraphEditorPanel — visual graph/node editing shell.
/// Replaces the legacy NF::Editor::GraphEditorPanel for the AtlasUI framework.
/// This is a shell implementation; full graph editing will be wired during
/// the GraphVM integration phase.
class GraphEditorPanel final : public PanelBase {
public:
    GraphEditorPanel()
        : PanelBase("atlas.graph_editor", "Graph Editor") {}

    void paint(IPaintContext& context) override;

    struct GraphNode {
        int id = 0;
        std::string name;
        float x = 0.f;
        float y = 0.f;
        bool selected = false;
    };

    struct GraphLink {
        int sourceNode = 0;
        int targetNode = 0;
    };

    void clearGraph() {
        m_nodes.clear();
        m_links.clear();
        m_selectedNodeId = -1;
    }

    int addNode(const std::string& name, float x, float y) {
        int id = m_nextNodeId++;
        m_nodes.push_back({id, name, x, y, false});
        return id;
    }

    bool removeNode(int nodeId) {
        for (auto it = m_nodes.begin(); it != m_nodes.end(); ++it) {
            if (it->id == nodeId) {
                m_nodes.erase(it);
                if (m_selectedNodeId == nodeId) m_selectedNodeId = -1;
                return true;
            }
        }
        return false;
    }

    void addLink(int src, int dst) {
        m_links.push_back({src, dst});
    }

    void selectNode(int id) { m_selectedNodeId = id; }
    void clearSelection() { m_selectedNodeId = -1; }
    [[nodiscard]] int selectedNodeId() const { return m_selectedNodeId; }

    [[nodiscard]] const std::vector<GraphNode>& nodes() const { return m_nodes; }
    [[nodiscard]] const std::vector<GraphLink>& links() const { return m_links; }
    [[nodiscard]] size_t nodeCount() const { return m_nodes.size(); }
    [[nodiscard]] size_t linkCount() const { return m_links.size(); }

    void setGraphName(const std::string& name) { m_graphName = name; }
    [[nodiscard]] const std::string& graphName() const { return m_graphName; }
    [[nodiscard]] bool hasOpenGraph() const { return !m_graphName.empty(); }

private:
    std::vector<GraphNode> m_nodes;
    std::vector<GraphLink> m_links;
    std::string m_graphName;
    int m_selectedNodeId = -1;
    int m_nextNodeId = 1;
};

} // namespace NF::UI::AtlasUI
