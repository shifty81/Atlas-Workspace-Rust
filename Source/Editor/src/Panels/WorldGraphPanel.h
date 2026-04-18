#pragma once
#include "../ui/EditorPanel.h"
#include "../../engine/world/WorldGraph.h"
#include <string>
#include <vector>

namespace atlas::editor {

struct GraphNodeInfo {
    world::NodeID id = 0;
    std::string name;
    std::string category;
    std::vector<world::NodePort> inputs;
    std::vector<world::NodePort> outputs;
};

class WorldGraphPanel : public EditorPanel {
public:
    const char* Name() const override { return "World Graph Editor"; }
    void Draw() override;

    void SetGraph(world::WorldGraph* graph) { m_graph = graph; }
    world::WorldGraph* GetGraph() { return m_graph; }

    bool CompileGraph();
    bool ExecutePreview(const world::WorldGenContext& ctx);

    void SelectNode(world::NodeID id) { m_selectedNode = id; }
    world::NodeID SelectedNode() const { return m_selectedNode; }

    // Node catalog snapshot populated by Draw()
    const std::vector<GraphNodeInfo>& NodeInfos() const { return m_nodeInfos; }

    // Compiled status
    bool IsCompiled() const;

    std::string Summary() const;

private:
    world::WorldGraph* m_graph = nullptr;
    world::NodeID m_selectedNode = 0;
    std::vector<GraphNodeInfo> m_nodeInfos;
};

}
