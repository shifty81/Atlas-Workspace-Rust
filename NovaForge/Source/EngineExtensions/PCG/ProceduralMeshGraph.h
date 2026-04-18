#pragma once
#include <cstdint>
#include <vector>
#include <string>
#include <unordered_map>

namespace atlas::procedural {

struct MeshData {
    std::vector<float> vertices;     // x,y,z triples
    std::vector<float> normals;      // x,y,z triples
    std::vector<uint32_t> indices;   // triangle indices

    size_t VertexCount() const { return vertices.size() / 3; }
    size_t TriangleCount() const { return indices.size() / 3; }
    void Clear() { vertices.clear(); normals.clear(); indices.clear(); }
    bool IsValid() const {
        return vertices.size() % 3 == 0 &&
               normals.size() == vertices.size() &&
               indices.size() % 3 == 0;
    }
};

enum class ProceduralNodeType : uint8_t {
    Primitive,
    Transform,
    Merge,
    Subdivide,
    Noise,
    Output
};

struct ProceduralNode {
    uint32_t id = 0;
    ProceduralNodeType type;
    std::vector<std::pair<std::string, std::string>> properties;

    std::string GetProperty(const std::string& key, const std::string& def = "") const {
        for (auto& p : properties) {
            if (p.first == key) return p.second;
        }
        return def;
    }
};

struct ProceduralEdge {
    uint32_t fromNode;
    uint16_t fromPort;
    uint32_t toNode;
    uint16_t toPort;
};

class ProceduralMeshGraph {
public:
    uint32_t AddNode(ProceduralNodeType type);
    void SetNodeProperty(uint32_t id, const std::string& key, const std::string& value);
    void AddEdge(const ProceduralEdge& edge);
    void RemoveNode(uint32_t id);

    bool Compile();
    bool Execute();

    const MeshData* GetOutput() const;
    size_t NodeCount() const;
    bool IsCompiled() const;

private:
    uint32_t m_nextID = 1;
    std::unordered_map<uint32_t, ProceduralNode> m_nodes;
    std::vector<ProceduralEdge> m_edges;
    std::vector<uint32_t> m_executionOrder;
    bool m_compiled = false;

    // Per-node intermediate mesh results keyed by (nodeID << 16 | port)
    std::unordered_map<uint64_t, MeshData> m_outputs;

    bool HasCycle() const;
    void ExecuteNode(const ProceduralNode& node);
};

}
