#include "ProceduralMaterialGraph.h"
#include "ProceduralMaterialNodes.h"
#include <algorithm>
#include <queue>
#include <cstdlib>
#include <stdexcept>

namespace atlas::procedural {

static float SafeStof(const std::string& s, float def) {
    try { return s.empty() ? def : std::stof(s); }
    catch (...) { return def; }
}

static int SafeStoi(const std::string& s, int def) {
    try { return s.empty() ? def : std::stoi(s); }
    catch (...) { return def; }
}

static uint64_t SafeStoull(const std::string& s, uint64_t def) {
    try { return s.empty() ? def : std::stoull(s); }
    catch (...) { return def; }
}

uint32_t ProceduralMaterialGraph::AddNode(MaterialNodeType type) {
    uint32_t id = m_nextID++;
    MaterialNode node;
    node.id = id;
    node.type = type;
    m_nodes[id] = std::move(node);
    m_compiled = false;
    return id;
}

void ProceduralMaterialGraph::SetNodeProperty(uint32_t id, const std::string& key, const std::string& value) {
    auto it = m_nodes.find(id);
    if (it == m_nodes.end()) return;
    for (auto& p : it->second.properties) {
        if (p.first == key) {
            p.second = value;
            m_compiled = false;
            return;
        }
    }
    it->second.properties.push_back({key, value});
    m_compiled = false;
}

void ProceduralMaterialGraph::AddEdge(const MaterialEdge& edge) {
    m_edges.push_back(edge);
    m_compiled = false;
}

void ProceduralMaterialGraph::RemoveNode(uint32_t id) {
    m_nodes.erase(id);
    m_edges.erase(
        std::remove_if(m_edges.begin(), m_edges.end(),
            [id](const MaterialEdge& e) {
                return e.fromNode == id || e.toNode == id;
            }),
        m_edges.end()
    );
    m_compiled = false;
}

bool ProceduralMaterialGraph::HasCycle() const {
    std::unordered_map<uint32_t, int> inDegree;
    for (auto& [id, _] : m_nodes) {
        inDegree[id] = 0;
    }
    for (auto& e : m_edges) {
        if (inDegree.count(e.toNode)) {
            inDegree[e.toNode]++;
        }
    }

    std::queue<uint32_t> q;
    for (auto& [id, deg] : inDegree) {
        if (deg == 0) q.push(id);
    }

    int visited = 0;
    while (!q.empty()) {
        uint32_t n = q.front();
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

bool ProceduralMaterialGraph::Compile() {
    m_compiled = false;
    m_executionOrder.clear();

    if (HasCycle()) return false;

    // Topological sort using Kahn's algorithm
    std::unordered_map<uint32_t, int> inDegree;
    for (auto& [id, _] : m_nodes) {
        inDegree[id] = 0;
    }
    for (auto& e : m_edges) {
        inDegree[e.toNode]++;
    }

    std::queue<uint32_t> q;
    for (auto& [id, deg] : inDegree) {
        if (deg == 0) q.push(id);
    }

    while (!q.empty()) {
        uint32_t n = q.front();
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

void ProceduralMaterialGraph::ExecuteNode(const MaterialNode& node) {
    // Collect input materials from upstream edges (port 0 = primary, port 1 = secondary)
    MaterialData input0, input1;
    for (auto& e : m_edges) {
        if (e.toNode == node.id) {
            uint64_t key = (static_cast<uint64_t>(e.fromNode) << 16) | e.fromPort;
            auto outIt = m_outputs.find(key);
            if (outIt != m_outputs.end()) {
                if (e.toPort == 0) input0 = outIt->second;
                else if (e.toPort == 1) input1 = outIt->second;
            }
        }
    }

    MaterialData result;

    switch (node.type) {
        case MaterialNodeType::SolidColor: {
            uint32_t w = static_cast<uint32_t>(SafeStoi(node.GetProperty("width"), 64));
            uint32_t h = static_cast<uint32_t>(SafeStoi(node.GetProperty("height"), 64));
            float r = SafeStof(node.GetProperty("r"), 1.0f);
            float g = SafeStof(node.GetProperty("g"), 1.0f);
            float b = SafeStof(node.GetProperty("b"), 1.0f);
            float a = SafeStof(node.GetProperty("a"), 1.0f);
            result = GenerateSolidColor(w, h, r, g, b, a);
            break;
        }
        case MaterialNodeType::Noise: {
            uint32_t w = static_cast<uint32_t>(SafeStoi(node.GetProperty("width"), 64));
            uint32_t h = static_cast<uint32_t>(SafeStoi(node.GetProperty("height"), 64));
            uint64_t seed = SafeStoull(node.GetProperty("seed"), 0);
            float scale = SafeStof(node.GetProperty("scale"), 1.0f);
            result = GenerateNoiseTexture(w, h, seed, scale);
            break;
        }
        case MaterialNodeType::Checker: {
            uint32_t w = static_cast<uint32_t>(SafeStoi(node.GetProperty("width"), 64));
            uint32_t h = static_cast<uint32_t>(SafeStoi(node.GetProperty("height"), 64));
            uint32_t ts = static_cast<uint32_t>(SafeStoi(node.GetProperty("tileSize"), 8));
            float r1 = SafeStof(node.GetProperty("r1"), 1.0f);
            float g1 = SafeStof(node.GetProperty("g1"), 1.0f);
            float b1 = SafeStof(node.GetProperty("b1"), 1.0f);
            float r2 = SafeStof(node.GetProperty("r2"), 0.0f);
            float g2 = SafeStof(node.GetProperty("g2"), 0.0f);
            float b2 = SafeStof(node.GetProperty("b2"), 0.0f);
            result = GenerateCheckerboard(w, h, ts, r1, g1, b1, r2, g2, b2);
            break;
        }
        case MaterialNodeType::Blend: {
            float factor = SafeStof(node.GetProperty("factor"), 0.5f);
            result = BlendMaterials(input0, input1, factor);
            break;
        }
        case MaterialNodeType::NormalMap: {
            float strength = SafeStof(node.GetProperty("strength"), 1.0f);
            result = ComputeNormalMap(input0, strength);
            break;
        }
        case MaterialNodeType::Output: {
            result = input0;
            break;
        }
    }

    uint64_t key = (static_cast<uint64_t>(node.id) << 16) | 0;
    m_outputs[key] = std::move(result);
}

bool ProceduralMaterialGraph::Execute() {
    if (!m_compiled) return false;

    m_outputs.clear();

    for (uint32_t id : m_executionOrder) {
        auto it = m_nodes.find(id);
        if (it == m_nodes.end()) return false;
        ExecuteNode(it->second);
    }

    return true;
}

const MaterialData* ProceduralMaterialGraph::GetOutput() const {
    // Find the Output node and return its result
    for (auto& [id, node] : m_nodes) {
        if (node.type == MaterialNodeType::Output) {
            uint64_t key = (static_cast<uint64_t>(id) << 16) | 0;
            auto it = m_outputs.find(key);
            if (it != m_outputs.end()) return &it->second;
        }
    }
    return nullptr;
}

size_t ProceduralMaterialGraph::NodeCount() const {
    return m_nodes.size();
}

bool ProceduralMaterialGraph::IsCompiled() const {
    return m_compiled;
}

}
