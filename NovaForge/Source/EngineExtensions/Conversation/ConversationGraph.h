#pragma once
#include <cstdint>
#include <vector>
#include <string>
#include <memory>
#include <unordered_map>

namespace atlas::conversation {

enum class ConversationPinType : uint8_t {
    Dialogue,
    Choice,
    Memory,
    Relationship,
    Signal
};

struct ConversationValue {
    ConversationPinType type;
    std::string text;
    float numericValue = 0.0f;
    std::unordered_map<std::string, std::string> metadata;
};

struct ConversationPort {
    std::string name;
    ConversationPinType type;
};

using ConversationNodeID = uint32_t;
using ConversationPortID = uint16_t;

struct ConversationEdge {
    ConversationNodeID fromNode;
    ConversationPortID fromPort;
    ConversationNodeID toNode;
    ConversationPortID toPort;
};

struct ConversationContext {
    uint64_t speakerID = 0;
    uint64_t listenerID = 0;
    uint64_t tick = 0;
};

class ConversationNode {
public:
    virtual ~ConversationNode() = default;

    virtual const char* GetName() const = 0;
    virtual const char* GetCategory() const = 0;
    virtual std::vector<ConversationPort> Inputs() const = 0;
    virtual std::vector<ConversationPort> Outputs() const = 0;

    virtual void Evaluate(
        const ConversationContext& ctx,
        const std::vector<ConversationValue>& inputs,
        std::vector<ConversationValue>& outputs
    ) const = 0;
};

class ConversationGraph {
public:
    ConversationNodeID AddNode(std::unique_ptr<ConversationNode> node);
    void RemoveNode(ConversationNodeID id);
    void AddEdge(const ConversationEdge& edge);
    void RemoveEdge(const ConversationEdge& edge);

    bool Compile();
    bool Execute(const ConversationContext& ctx);

    const ConversationValue* GetOutput(ConversationNodeID node, ConversationPortID port) const;
    size_t NodeCount() const;
    bool IsCompiled() const;

private:
    ConversationNodeID m_nextID = 1;
    std::unordered_map<ConversationNodeID, std::unique_ptr<ConversationNode>> m_nodes;
    std::vector<ConversationEdge> m_edges;
    std::vector<ConversationNodeID> m_executionOrder;
    bool m_compiled = false;

    std::unordered_map<uint64_t, ConversationValue> m_outputs;

    bool HasCycle() const;
    bool ValidateEdgeTypes() const;
};

}
