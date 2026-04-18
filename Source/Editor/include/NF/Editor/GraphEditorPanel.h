#pragma once
// NF::Editor — Graph editor panel
#include "NF/Core/Core.h"
#include "NF/Engine/Engine.h"
#include "NF/Renderer/Renderer.h"
#include "NF/UI/UI.h"
#include "NF/GraphVM/GraphVM.h"
#include "NF/Input/Input.h"
#include "NF/UI/UIWidgets.h"
#include <filesystem>
#include <fstream>
#include <set>
#include <map>
#include <deque>
#include <unordered_map>
#include "NF/Editor/EditorPanel.h"

namespace NF {

class GraphEditorPanel : public EditorPanel {
public:
    explicit GraphEditorPanel(GraphVM* vm = nullptr) : m_graphVM(vm) {}

    const std::string& name() const override { return m_name; }
    DockSlot slot() const override { return DockSlot::Center; }
    void update(float /*dt*/) override {}

    void render(UIRenderer& ui, const Rect& bounds,
                const EditorTheme& theme) override {
        // Background
        ui.drawRect(bounds, theme.panelBackground);

        if (!m_currentGraph) {
            ui.drawText(bounds.x + 8.f, bounds.y + 8.f, "No graph open", theme.panelText);
            return;
        }

        // Draw nodes as simple labelled rectangles
        for (const auto& node : m_currentGraph->nodes()) {
            Rect nr{node.position.x + bounds.x, node.position.y + bounds.y, 120.f, 60.f};
            uint32_t nodeColor = (m_selectedNodeId >= 0 && node.id == static_cast<uint32_t>(m_selectedNodeId))
                                 ? theme.selectionHighlight : theme.toolbarBackground;
            ui.drawRect(nr, nodeColor);
            ui.drawRectOutline(nr, theme.panelText, 1.f);
            ui.drawText(nr.x + 4.f, nr.y + 4.f, node.name, theme.panelText);
        }

        // Draw link count annotation
        float ly = bounds.y + bounds.h - 20.f;
        ui.drawText(bounds.x + 4.f, ly,
                           std::to_string(m_currentGraph->links().size()) + " link(s)",
                           theme.propertyLabel);
    }

    void setGraphVM(GraphVM* vm) { m_graphVM = vm; }
    GraphVM* graphVM() const { return m_graphVM; }

    // Create a brand-new Graph and make it the current one.
    void newGraph(GraphType type, const std::string& graphName) {
        m_ownedGraph = std::make_unique<Graph>();
        m_currentGraph = m_ownedGraph.get();
        m_currentGraphName = graphName;
        m_currentGraphType = type;
        m_nextNodeId = 1;
        m_selectedNodeId = -1;
        NF_LOG_INFO("Editor", "GraphEditorPanel: new graph '" + graphName + "'");
    }

    // Open an externally-owned graph (e.g. loaded from disk).
    bool openGraph(const std::string& graphName) {
        if (!m_graphVM) return false;
        m_currentGraphName = graphName;
        NF_LOG_INFO("Editor", "GraphEditorPanel: opened graph '" + graphName + "'");
        return true;
    }

    // Add a node to the current graph; returns the assigned node ID or -1 on failure.
    int addNode(const std::string& nodeName, GraphType type = GraphType::World,
                float x = 0.f, float y = 0.f) {
        if (!m_currentGraph) return -1;
        GraphNode n;
        n.id       = m_nextNodeId++;
        n.name     = nodeName;
        n.type     = type;
        n.position = {x, y};
        m_currentGraph->addNode(n);
        NF_LOG_INFO("Editor", "GraphEditorPanel: added node '" + nodeName + "' id=" +
                    std::to_string(n.id));
        return n.id;
    }

    // Remove a node (and its connected links) from the current graph.
    bool removeNode(int nodeId) {
        if (!m_currentGraph) return false;
        if (m_currentGraph->findNode(nodeId) == nullptr) return false;
        m_currentGraph->removeNode(nodeId);
        if (m_selectedNodeId == nodeId) m_selectedNodeId = -1;
        return true;
    }

    // Add a link between two node ports.
    bool addLink(int srcNode, uint32_t srcPort, int dstNode, uint32_t dstPort) {
        if (!m_currentGraph) return false;
        GraphLink lk;
        lk.sourceNode = srcNode;
        lk.sourcePort = srcPort;
        lk.targetNode = dstNode;
        lk.targetPort = dstPort;
        m_currentGraph->addLink(lk);
        return true;
    }

    // Compile the current graph and load the resulting program into the VM.
    bool compileAndLoad() {
        if (!m_currentGraph || !m_graphVM) return false;
        auto prog = GraphCompiler::compile(*m_currentGraph);
        m_graphVM->loadProgram(prog);
        NF_LOG_INFO("Editor", "GraphEditorPanel: compiled graph '" + m_currentGraphName + "'");
        return true;
    }

    [[nodiscard]] Graph* currentGraph() { return m_currentGraph; }
    [[nodiscard]] const Graph* currentGraph() const { return m_currentGraph; }
    const std::string& currentGraphName() const { return m_currentGraphName; }
    bool hasOpenGraph() const { return m_currentGraph != nullptr; }

    int selectedNodeId() const { return m_selectedNodeId; }
    void selectNode(int id) { m_selectedNodeId = id; }
    void clearSelection() { m_selectedNodeId = -1; }

    int nodeCount() const { return m_currentGraph ? (int)m_currentGraph->nodes().size() : 0; }
    int linkCount() const { return m_currentGraph ? (int)m_currentGraph->links().size() : 0; }

private:
    std::string m_name = "GraphEditor";
    GraphVM* m_graphVM = nullptr;
    std::unique_ptr<Graph> m_ownedGraph;
    Graph* m_currentGraph = nullptr;
    std::string m_currentGraphName;
    GraphType m_currentGraphType = GraphType::World;
    int m_selectedNodeId = -1;
    int m_nextNodeId = 1;
};

// ── Tool Window Manager ──────────────────────────────────────────


} // namespace NF
