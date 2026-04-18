#include "BehaviorGraph.h"
#include <algorithm>
#include <queue>
#include <cstring>

namespace atlas::ai {

BehaviorNodeID BehaviorGraph::AddNode(std::unique_ptr<BehaviorNode> node) {
    BehaviorNodeID id = m_nextID++;
    m_nodes[id] = std::move(node);
    m_compiled = false;
    return id;
}

void BehaviorGraph::RemoveNode(BehaviorNodeID id) {
    m_nodes.erase(id);
    m_edges.erase(
        std::remove_if(m_edges.begin(), m_edges.end(),
            [id](const BehaviorEdge& e) {
                return e.fromNode == id || e.toNode == id;
            }),
        m_edges.end()
    );
    m_compiled = false;
}

void BehaviorGraph::AddEdge(const BehaviorEdge& edge) {
    m_edges.push_back(edge);
    m_compiled = false;
}

void BehaviorGraph::RemoveEdge(const BehaviorEdge& edge) {
    m_edges.erase(
        std::remove_if(m_edges.begin(), m_edges.end(),
            [&edge](const BehaviorEdge& e) {
                return e.fromNode == edge.fromNode &&
                       e.fromPort == edge.fromPort &&
                       e.toNode == edge.toNode &&
                       e.toPort == edge.toPort;
            }),
        m_edges.end()
    );
    m_compiled = false;
}

bool BehaviorGraph::HasCycle() const {
    std::unordered_map<BehaviorNodeID, int> inDegree;
    for (auto& [id, _] : m_nodes) {
        inDegree[id] = 0;
    }
    for (auto& e : m_edges) {
        if (inDegree.count(e.toNode)) {
            inDegree[e.toNode]++;
        }
    }

    std::queue<BehaviorNodeID> q;
    for (auto& [id, deg] : inDegree) {
        if (deg == 0) q.push(id);
    }

    int visited = 0;
    while (!q.empty()) {
        BehaviorNodeID n = q.front();
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

bool BehaviorGraph::ValidateEdgeTypes() const {
    for (auto& e : m_edges) {
        auto fromIt = m_nodes.find(e.fromNode);
        auto toIt = m_nodes.find(e.toNode);
        if (fromIt == m_nodes.end() || toIt == m_nodes.end()) return false;

        auto fromOutputs = fromIt->second->Outputs();
        auto toInputs = toIt->second->Inputs();

        if (e.fromPort >= fromOutputs.size()) return false;
        if (e.toPort >= toInputs.size()) return false;

        if (fromOutputs[e.fromPort].type != toInputs[e.toPort].type) return false;
    }
    return true;
}

bool BehaviorGraph::Compile() {
    m_compiled = false;
    m_executionOrder.clear();

    if (HasCycle()) return false;
    if (!ValidateEdgeTypes()) return false;

    std::unordered_map<BehaviorNodeID, int> inDegree;
    for (auto& [id, _] : m_nodes) {
        inDegree[id] = 0;
    }
    for (auto& e : m_edges) {
        inDegree[e.toNode]++;
    }

    std::queue<BehaviorNodeID> q;
    for (auto& [id, deg] : inDegree) {
        if (deg == 0) q.push(id);
    }

    while (!q.empty()) {
        BehaviorNodeID n = q.front();
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

bool BehaviorGraph::Execute(const AIContext& ctx) {
    if (!m_compiled) return false;

    m_outputs.clear();

    for (BehaviorNodeID id : m_executionOrder) {
        auto it = m_nodes.find(id);
        if (it == m_nodes.end()) return false;

        BehaviorNode* node = it->second.get();
        auto inputDefs = node->Inputs();

        std::vector<BehaviorValue> inputs(inputDefs.size());
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
        std::vector<BehaviorValue> outputs(outputDefs.size());
        node->Evaluate(ctx, inputs, outputs);

        for (BehaviorPortID p = 0; p < outputs.size(); ++p) {
            uint64_t key = (static_cast<uint64_t>(id) << 32) | p;
            m_outputs[key] = std::move(outputs[p]);
        }
    }

    return true;
}

const BehaviorValue* BehaviorGraph::GetOutput(BehaviorNodeID node, BehaviorPortID port) const {
    uint64_t key = (static_cast<uint64_t>(node) << 32) | port;
    auto it = m_outputs.find(key);
    if (it != m_outputs.end()) return &it->second;
    return nullptr;
}

size_t BehaviorGraph::NodeCount() const {
    return m_nodes.size();
}

bool BehaviorGraph::IsCompiled() const {
    return m_compiled;
}

std::vector<uint8_t> BehaviorGraph::SerializeState() const {
    std::vector<uint8_t> buf;

    auto writeU8 = [&](uint8_t v) { buf.push_back(v); };
    auto writeU32 = [&](uint32_t v) {
        size_t pos = buf.size();
        buf.resize(pos + sizeof(uint32_t));
        std::memcpy(buf.data() + pos, &v, sizeof(uint32_t));
    };
    auto writeU64 = [&](uint64_t v) {
        size_t pos = buf.size();
        buf.resize(pos + sizeof(uint64_t));
        std::memcpy(buf.data() + pos, &v, sizeof(uint64_t));
    };
    auto writeFloat = [&](float v) {
        size_t pos = buf.size();
        buf.resize(pos + sizeof(float));
        std::memcpy(buf.data() + pos, &v, sizeof(float));
    };

    // Compiled flag
    writeU8(m_compiled ? 1 : 0);

    // Execution order
    writeU32(static_cast<uint32_t>(m_executionOrder.size()));
    for (auto id : m_executionOrder) {
        writeU32(id);
    }

    // Outputs map
    writeU32(static_cast<uint32_t>(m_outputs.size()));
    for (const auto& [key, val] : m_outputs) {
        writeU64(key);
        writeU8(static_cast<uint8_t>(val.type));
        writeU32(static_cast<uint32_t>(val.data.size()));
        for (float f : val.data) {
            writeFloat(f);
        }
    }

    return buf;
}

bool BehaviorGraph::DeserializeState(const std::vector<uint8_t>& data) {
    size_t pos = 0;

    auto readU8 = [&](uint8_t& v) -> bool {
        if (pos + 1 > data.size()) return false;
        v = data[pos++];
        return true;
    };
    auto readU32 = [&](uint32_t& v) -> bool {
        if (pos + sizeof(uint32_t) > data.size()) return false;
        std::memcpy(&v, data.data() + pos, sizeof(uint32_t));
        pos += sizeof(uint32_t);
        return true;
    };
    auto readU64 = [&](uint64_t& v) -> bool {
        if (pos + sizeof(uint64_t) > data.size()) return false;
        std::memcpy(&v, data.data() + pos, sizeof(uint64_t));
        pos += sizeof(uint64_t);
        return true;
    };
    auto readFloat = [&](float& v) -> bool {
        if (pos + sizeof(float) > data.size()) return false;
        std::memcpy(&v, data.data() + pos, sizeof(float));
        pos += sizeof(float);
        return true;
    };

    uint8_t compiledFlag = 0;
    if (!readU8(compiledFlag)) return false;
    m_compiled = (compiledFlag != 0);

    uint32_t orderSize = 0;
    if (!readU32(orderSize)) return false;
    if (orderSize > 1000000) return false;  // sanity limit
    m_executionOrder.resize(orderSize);
    for (uint32_t i = 0; i < orderSize; ++i) {
        if (!readU32(m_executionOrder[i])) return false;
    }

    uint32_t outputCount = 0;
    if (!readU32(outputCount)) return false;
    if (outputCount > 1000000) return false;  // sanity limit
    m_outputs.clear();
    for (uint32_t i = 0; i < outputCount; ++i) {
        uint64_t key = 0;
        if (!readU64(key)) return false;
        uint8_t typeVal = 0;
        if (!readU8(typeVal)) return false;
        uint32_t dataSize = 0;
        if (!readU32(dataSize)) return false;
        BehaviorValue val;
        val.type = static_cast<BehaviorPinType>(typeVal);
        val.data.resize(dataSize);
        for (uint32_t j = 0; j < dataSize; ++j) {
            if (!readFloat(val.data[j])) return false;
        }
        m_outputs[key] = std::move(val);
    }

    return true;
}

}
