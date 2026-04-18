#include "WorldGraphPanel.h"
#include <sstream>

namespace atlas::editor {

void WorldGraphPanel::Draw() {
    m_nodeInfos.clear();
    if (!m_graph) return;

    auto nodeIDs = m_graph->GetNodeIDs();
    for (auto id : nodeIDs) {
        const auto* node = m_graph->GetNode(id);
        if (!node) continue;

        GraphNodeInfo info;
        info.id = id;
        info.name = node->GetName();
        info.category = node->GetCategory();
        info.inputs = node->Inputs();
        info.outputs = node->Outputs();

        m_nodeInfos.push_back(std::move(info));
    }
}

bool WorldGraphPanel::CompileGraph() {
    if (!m_graph) return false;
    return m_graph->Compile();
}

bool WorldGraphPanel::ExecutePreview(const world::WorldGenContext& ctx) {
    if (!m_graph || !m_graph->IsCompiled()) return false;
    return m_graph->Execute(ctx);
}

bool WorldGraphPanel::IsCompiled() const {
    if (!m_graph) return false;
    return m_graph->IsCompiled();
}

std::string WorldGraphPanel::Summary() const {
    std::ostringstream oss;
    oss << "Nodes: " << m_nodeInfos.size();
    oss << ", Compiled: " << (IsCompiled() ? "yes" : "no");
    return oss.str();
}

}
