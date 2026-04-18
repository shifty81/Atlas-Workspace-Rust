#pragma once
#include <cstdint>
#include <vector>
#include <string>
#include <memory>
#include <unordered_map>

namespace atlas::ui {

enum class UIPinType : uint8_t {
    Float,
    Bool,
    String,
    Layout,
    Action
};

struct UIValue {
    UIPinType type;
    std::vector<float> data;
    std::string text;
};

struct UIPort {
    std::string name;
    UIPinType type;
};

using UINodeID = uint32_t;
using UIPortID = uint16_t;

struct UIEdge {
    UINodeID fromNode;
    UIPortID fromPort;
    UINodeID toNode;
    UIPortID toPort;
};

struct UIContext {
    float screenWidth;
    float screenHeight;
    float deltaTime;
    uint32_t tick;
};

class UINode {
public:
    virtual ~UINode() = default;
    virtual const char* GetName() const = 0;
    virtual const char* GetCategory() const = 0;
    virtual std::vector<UIPort> Inputs() const = 0;
    virtual std::vector<UIPort> Outputs() const = 0;
    virtual void Evaluate(const UIContext& ctx,
                          const std::vector<UIValue>& inputs,
                          std::vector<UIValue>& outputs) const = 0;
};

class UIGraph {
public:
    UINodeID AddNode(std::unique_ptr<UINode> node);
    void RemoveNode(UINodeID id);
    void AddEdge(const UIEdge& edge);
    void RemoveEdge(const UIEdge& edge);

    bool Compile();
    bool Execute(const UIContext& ctx);

    const UIValue* GetOutput(UINodeID node, UIPortID port) const;
    size_t NodeCount() const;
    bool IsCompiled() const;

private:
    UINodeID m_nextID = 1;
    std::unordered_map<UINodeID, std::unique_ptr<UINode>> m_nodes;
    std::vector<UIEdge> m_edges;
    std::vector<UINodeID> m_executionOrder;
    bool m_compiled = false;

    std::unordered_map<uint64_t, UIValue> m_outputs;

    bool HasCycle() const;
    bool ValidateEdgeTypes() const;
};

}
