#include "ConversationGraph.h"
#include <algorithm>
#include <queue>

namespace atlas::conversation {

ConversationNodeID ConversationGraph::AddNode(std::unique_ptr<ConversationNode> node) {
    ConversationNodeID id = m_nextID++;
    m_nodes[id] = std::move(node);
    m_compiled = false;
    return id;
}

void ConversationGraph::RemoveNode(ConversationNodeID id) {
    m_nodes.erase(id);
    m_edges.erase(
        std::remove_if(m_edges.begin(), m_edges.end(),
            [id](const ConversationEdge& e) {
                return e.fromNode == id || e.toNode == id;
            }),
        m_edges.end()
    );
    m_compiled = false;
}

void ConversationGraph::AddEdge(const ConversationEdge& edge) {
    m_edges.push_back(edge);
    m_compiled = false;
}

void ConversationGraph::RemoveEdge(const ConversationEdge& edge) {
    m_edges.erase(
        std::remove_if(m_edges.begin(), m_edges.end(),
            [&edge](const ConversationEdge& e) {
                return e.fromNode == edge.fromNode &&
                       e.fromPort == edge.fromPort &&
                       e.toNode == edge.toNode &&
                       e.toPort == edge.toPort;
            }),
        m_edges.end()
    );
    m_compiled = false;
}

bool ConversationGraph::HasCycle() const {
    std::unordered_map<ConversationNodeID, int> inDegree;
    for (auto& [id, _] : m_nodes) {
        inDegree[id] = 0;
    }
    for (auto& e : m_edges) {
        if (inDegree.count(e.toNode)) {
            inDegree[e.toNode]++;
        }
    }

    std::queue<ConversationNodeID> q;
    for (auto& [id, deg] : inDegree) {
        if (deg == 0) q.push(id);
    }

    int visited = 0;
    while (!q.empty()) {
        ConversationNodeID n = q.front();
        q.pop();
        visited++;
        for (auto& e : m_edges) {
            if (e.fromNode == n) {
                if (--inDegree[e.toNode] == 0) {
                    q.push(e.toNode);
                }
            }
        }
    }

    return visited != static_cast<int>(m_nodes.size());
}

bool ConversationGraph::ValidateEdgeTypes() const {
    for (auto& e : m_edges) {
        auto fromIt = m_nodes.find(e.fromNode);
        auto toIt = m_nodes.find(e.toNode);
        if (fromIt == m_nodes.end() || toIt == m_nodes.end()) return false;

        auto fromOutputs = fromIt->second->Outputs();
        auto toInputs = toIt->second->Inputs();

        if (e.fromPort >= fromOutputs.size()) return false;
        if (e.toPort >= toInputs.size()) return false;
    }
    return true;
}

bool ConversationGraph::Compile() {
    m_compiled = false;
    m_executionOrder.clear();

    if (HasCycle()) return false;
    if (!ValidateEdgeTypes()) return false;

    std::unordered_map<ConversationNodeID, int> inDegree;
    for (auto& [id, _] : m_nodes) {
        inDegree[id] = 0;
    }
    for (auto& e : m_edges) {
        inDegree[e.toNode]++;
    }

    std::queue<ConversationNodeID> q;
    for (auto& [id, deg] : inDegree) {
        if (deg == 0) q.push(id);
    }

    while (!q.empty()) {
        ConversationNodeID n = q.front();
        q.pop();
        m_executionOrder.push_back(n);
        for (auto& e : m_edges) {
            if (e.fromNode == n) {
                if (--inDegree[e.toNode] == 0) {
                    q.push(e.toNode);
                }
            }
        }
    }

    m_compiled = (m_executionOrder.size() == m_nodes.size());
    return m_compiled;
}

bool ConversationGraph::Execute(const ConversationContext& ctx) {
    if (!m_compiled) return false;

    m_outputs.clear();

    for (ConversationNodeID id : m_executionOrder) {
        auto it = m_nodes.find(id);
        if (it == m_nodes.end()) return false;

        ConversationNode* node = it->second.get();
        auto inputDefs = node->Inputs();

        std::vector<ConversationValue> inputs(inputDefs.size());
        for (auto& e : m_edges) {
            if (e.toNode == id && e.toPort < inputs.size()) {
                uint64_t key = (static_cast<uint64_t>(e.fromNode) << 32) | e.fromPort;
                auto outIt = m_outputs.find(key);
                if (outIt != m_outputs.end()) {
                    inputs[e.toPort] = outIt->second;
                }
            }
        }

        auto outputDefs = node->Outputs();
        std::vector<ConversationValue> outputs(outputDefs.size());
        node->Evaluate(ctx, inputs, outputs);

        for (ConversationPortID p = 0; p < static_cast<ConversationPortID>(outputs.size()); ++p) {
            uint64_t key = (static_cast<uint64_t>(id) << 32) | p;
            m_outputs[key] = std::move(outputs[p]);
        }
    }

    return true;
}

const ConversationValue* ConversationGraph::GetOutput(ConversationNodeID node, ConversationPortID port) const {
    uint64_t key = (static_cast<uint64_t>(node) << 32) | port;
    auto it = m_outputs.find(key);
    if (it != m_outputs.end()) return &it->second;
    return nullptr;
}

size_t ConversationGraph::NodeCount() const {
    return m_nodes.size();
}

bool ConversationGraph::IsCompiled() const {
    return m_compiled;
}

}
