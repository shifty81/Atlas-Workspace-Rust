#pragma once
#include "ProceduralMeshGraph.h"  // reuse MeshData
#include <cstdint>
#include <vector>
#include <string>
#include <unordered_map>

namespace atlas::procedural {

struct LODLevel {
    uint32_t level = 0;        // 0 = highest detail
    float reductionFactor = 1.0f; // fraction of original triangles to keep
    MeshData mesh;
};

struct LODChain {
    std::vector<LODLevel> levels;
    bool IsValid() const { return !levels.empty() && levels[0].mesh.IsValid(); }
    size_t LevelCount() const { return levels.size(); }
};

enum class LODNodeType : uint8_t {
    MeshInput,     // takes a mesh as input
    Decimate,      // reduce triangle count (simple vertex removal)
    MergeVertices, // merge nearby vertices (welding)
    BakeNormals,   // recompute normals
    Output         // final LOD chain output
};

struct LODNode {
    uint32_t id = 0;
    LODNodeType type;
    std::vector<std::pair<std::string, std::string>> properties;

    std::string GetProperty(const std::string& key, const std::string& def = "") const {
        for (auto& p : properties) {
            if (p.first == key) return p.second;
        }
        return def;
    }
};

struct LODEdge {
    uint32_t fromNode;
    uint16_t fromPort;
    uint32_t toNode;
    uint16_t toPort;
};

class LODBakingGraph {
public:
    uint32_t AddNode(LODNodeType type);
    void SetNodeProperty(uint32_t id, const std::string& key, const std::string& value);
    void AddEdge(const LODEdge& edge);
    void RemoveNode(uint32_t id);

    bool Compile();
    bool Execute();

    const LODChain* GetOutput() const;
    size_t NodeCount() const;
    bool IsCompiled() const;

private:
    uint32_t m_nextID = 1;
    std::unordered_map<uint32_t, LODNode> m_nodes;
    std::vector<LODEdge> m_edges;
    std::vector<uint32_t> m_executionOrder;
    bool m_compiled = false;

    // Per-node intermediate mesh results keyed by (nodeID << 16 | port)
    std::unordered_map<uint64_t, MeshData> m_meshOutputs;
    LODChain m_lodOutput;

    bool HasCycle() const;
    void ExecuteNode(const LODNode& node);
};

}
