#pragma once
#include <cstdint>
#include <vector>
#include <string>
#include <memory>
#include <unordered_map>

namespace atlas::strategy {

enum class StrategyPinType : uint8_t {
    ScalarField,
    VectorField,
    Mask,
    Influence,
    Heatmap
};

struct StrategyValue {
    StrategyPinType type;
    std::vector<float> data;
};

struct StrategyPort {
    std::string name;
    StrategyPinType type;
};

using StrategyNodeID = uint32_t;
using StrategyPortID = uint16_t;

struct StrategyEdge {
    StrategyNodeID fromNode;
    StrategyPortID fromPort;
    StrategyNodeID toNode;
    StrategyPortID toPort;
};

struct StrategyContext {
    uint64_t tick = 0;
    uint64_t seed = 0;
};

class StrategyNode {
public:
    virtual ~StrategyNode() = default;

    virtual const char* GetName() const = 0;
    virtual const char* GetCategory() const = 0;
    virtual std::vector<StrategyPort> Inputs() const = 0;
    virtual std::vector<StrategyPort> Outputs() const = 0;

    virtual void Evaluate(
        const StrategyContext& ctx,
        const std::vector<StrategyValue>& inputs,
        std::vector<StrategyValue>& outputs
    ) const = 0;
};

class StrategyGraph {
public:
    StrategyNodeID AddNode(std::unique_ptr<StrategyNode> node);
    void RemoveNode(StrategyNodeID id);
    void AddEdge(const StrategyEdge& edge);
    void RemoveEdge(const StrategyEdge& edge);

    bool Compile();
    bool Execute(const StrategyContext& ctx);

    const StrategyValue* GetOutput(StrategyNodeID node, StrategyPortID port) const;
    size_t NodeCount() const;
    bool IsCompiled() const;

private:
    StrategyNodeID m_nextID = 1;
    std::unordered_map<StrategyNodeID, std::unique_ptr<StrategyNode>> m_nodes;
    std::vector<StrategyEdge> m_edges;
    std::vector<StrategyNodeID> m_executionOrder;
    bool m_compiled = false;

    std::unordered_map<uint64_t, StrategyValue> m_outputs;

    bool HasCycle() const;
    bool ValidateEdgeTypes() const;
};

}
