#include "ProceduralMeshGraph.h"
#include "ProceduralMeshNodes.h"
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

uint32_t ProceduralMeshGraph::AddNode(ProceduralNodeType type) {
    uint32_t id = m_nextID++;
    ProceduralNode node;
    node.id = id;
    node.type = type;
    m_nodes[id] = std::move(node);
    m_compiled = false;
    return id;
}

void ProceduralMeshGraph::SetNodeProperty(uint32_t id, const std::string& key, const std::string& value) {
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

void ProceduralMeshGraph::AddEdge(const ProceduralEdge& edge) {
    m_edges.push_back(edge);
    m_compiled = false;
}

void ProceduralMeshGraph::RemoveNode(uint32_t id) {
    m_nodes.erase(id);
    m_edges.erase(
        std::remove_if(m_edges.begin(), m_edges.end(),
            [id](const ProceduralEdge& e) {
                return e.fromNode == id || e.toNode == id;
            }),
        m_edges.end()
    );
    m_compiled = false;
}

bool ProceduralMeshGraph::HasCycle() const {
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

bool ProceduralMeshGraph::Compile() {
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

void ProceduralMeshGraph::ExecuteNode(const ProceduralNode& node) {
    // Collect input meshes from upstream edges (port 0 = primary, port 1 = secondary)
    MeshData input0, input1;
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

    MeshData result;

    switch (node.type) {
        case ProceduralNodeType::Primitive: {
            std::string shape = node.GetProperty("shape", "cube");
            float size = SafeStof(node.GetProperty("size"), 1.0f);
            int segments = SafeStoi(node.GetProperty("segments"), 8);

            if (shape == "cube") result = GenerateCube(size);
            else if (shape == "plane") result = GeneratePlane(size);
            else if (shape == "sphere") result = GenerateSphere(size, segments);
            else if (shape == "cylinder") result = GenerateCylinder(size, segments);
            else result = GenerateCube(size);
            break;
        }
        case ProceduralNodeType::Transform: {
            result = input0;
            float tx = SafeStof(node.GetProperty("translateX"), 0.0f);
            float ty = SafeStof(node.GetProperty("translateY"), 0.0f);
            float tz = SafeStof(node.GetProperty("translateZ"), 0.0f);
            float sx = SafeStof(node.GetProperty("scaleX"), 1.0f);
            float sy = SafeStof(node.GetProperty("scaleY"), 1.0f);
            float sz = SafeStof(node.GetProperty("scaleZ"), 1.0f);

            for (size_t i = 0; i + 2 < result.vertices.size(); i += 3) {
                result.vertices[i]     = result.vertices[i]     * sx + tx;
                result.vertices[i + 1] = result.vertices[i + 1] * sy + ty;
                result.vertices[i + 2] = result.vertices[i + 2] * sz + tz;
            }
            break;
        }
        case ProceduralNodeType::Merge: {
            result = input0;
            uint32_t offset = static_cast<uint32_t>(result.VertexCount());
            result.vertices.insert(result.vertices.end(),
                                   input1.vertices.begin(), input1.vertices.end());
            result.normals.insert(result.normals.end(),
                                  input1.normals.begin(), input1.normals.end());
            for (uint32_t idx : input1.indices) {
                result.indices.push_back(idx + offset);
            }
            break;
        }
        case ProceduralNodeType::Subdivide: {
            result = SubdivideMesh(input0);
            break;
        }
        case ProceduralNodeType::Noise: {
            float amplitude = SafeStof(node.GetProperty("amplitude"), 0.1f);
            uint64_t seed = SafeStoull(node.GetProperty("seed"), 0);
            result = ApplyNoiseDisplacement(input0, amplitude, seed);
            break;
        }
        case ProceduralNodeType::Output: {
            result = input0;
            break;
        }
    }

    uint64_t key = (static_cast<uint64_t>(node.id) << 16) | 0;
    m_outputs[key] = std::move(result);
}

bool ProceduralMeshGraph::Execute() {
    if (!m_compiled) return false;

    m_outputs.clear();

    for (uint32_t id : m_executionOrder) {
        auto it = m_nodes.find(id);
        if (it == m_nodes.end()) return false;
        ExecuteNode(it->second);
    }

    return true;
}

const MeshData* ProceduralMeshGraph::GetOutput() const {
    // Find the Output node and return its result
    for (auto& [id, node] : m_nodes) {
        if (node.type == ProceduralNodeType::Output) {
            uint64_t key = (static_cast<uint64_t>(id) << 16) | 0;
            auto it = m_outputs.find(key);
            if (it != m_outputs.end()) return &it->second;
        }
    }
    return nullptr;
}

size_t ProceduralMeshGraph::NodeCount() const {
    return m_nodes.size();
}

bool ProceduralMeshGraph::IsCompiled() const {
    return m_compiled;
}

}
