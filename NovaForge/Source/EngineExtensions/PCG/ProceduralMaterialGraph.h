#pragma once
#include <cstdint>
#include <vector>
#include <string>
#include <unordered_map>

namespace atlas::procedural {

struct MaterialData {
    std::vector<float> albedo;     // RGBA per pixel
    std::vector<float> normal;     // XYZ per pixel
    std::vector<float> roughness;  // scalar per pixel
    std::vector<float> metallic;   // scalar per pixel
    uint32_t width = 0;
    uint32_t height = 0;

    size_t PixelCount() const { return static_cast<size_t>(width) * height; }
    void Clear() { albedo.clear(); normal.clear(); roughness.clear(); metallic.clear(); width = height = 0; }
    bool IsValid() const {
        size_t pc = PixelCount();
        return pc > 0 &&
               albedo.size() == pc * 4 &&
               normal.size() == pc * 3 &&
               roughness.size() == pc &&
               metallic.size() == pc;
    }
};

enum class MaterialNodeType : uint8_t {
    SolidColor,      // uniform color output
    Noise,           // procedural noise pattern
    Checker,         // checkerboard pattern
    Blend,           // blend two materials
    NormalMap,       // generate normal map from heightmap
    Output           // final output
};

struct MaterialNode {
    uint32_t id = 0;
    MaterialNodeType type;
    std::vector<std::pair<std::string, std::string>> properties;

    std::string GetProperty(const std::string& key, const std::string& def = "") const {
        for (auto& p : properties) {
            if (p.first == key) return p.second;
        }
        return def;
    }
};

struct MaterialEdge {
    uint32_t fromNode;
    uint16_t fromPort;
    uint32_t toNode;
    uint16_t toPort;
};

class ProceduralMaterialGraph {
public:
    uint32_t AddNode(MaterialNodeType type);
    void SetNodeProperty(uint32_t id, const std::string& key, const std::string& value);
    void AddEdge(const MaterialEdge& edge);
    void RemoveNode(uint32_t id);

    bool Compile();
    bool Execute();

    const MaterialData* GetOutput() const;
    size_t NodeCount() const;
    bool IsCompiled() const;

private:
    uint32_t m_nextID = 1;
    std::unordered_map<uint32_t, MaterialNode> m_nodes;
    std::vector<MaterialEdge> m_edges;
    std::vector<uint32_t> m_executionOrder;
    bool m_compiled = false;

    // Per-node intermediate material results keyed by (nodeID << 16 | port)
    std::unordered_map<uint64_t, MaterialData> m_outputs;

    bool HasCycle() const;
    void ExecuteNode(const MaterialNode& node);
};

}
