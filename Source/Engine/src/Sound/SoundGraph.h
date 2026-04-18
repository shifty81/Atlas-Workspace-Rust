#pragma once
#include <cstdint>
#include <vector>
#include <string>
#include <memory>
#include <unordered_map>

namespace atlas::sound {

enum class SoundPinType : uint8_t {
    AudioBuffer,   // PCM sample buffer
    Float,         // Scalar parameter (frequency, gain, etc.)
    Seed,          // RNG seed for variation
    Trigger,       // Event trigger signal
    Envelope       // ADSR envelope data
};

struct SoundValue {
    SoundPinType type;
    std::vector<float> data;
};

struct SoundPort {
    std::string name;
    SoundPinType type;
};

using SoundNodeID = uint32_t;
using SoundPortID = uint16_t;

struct SoundEdge {
    SoundNodeID fromNode;
    SoundPortID fromPort;
    SoundNodeID toNode;
    SoundPortID toPort;
};

struct SoundContext {
    uint32_t sampleRate;
    uint32_t bufferSize;
    uint64_t seed;
};

class SoundNode {
public:
    virtual ~SoundNode() = default;
    virtual const char* GetName() const = 0;
    virtual const char* GetCategory() const = 0;
    virtual std::vector<SoundPort> Inputs() const = 0;
    virtual std::vector<SoundPort> Outputs() const = 0;
    virtual void Evaluate(const SoundContext& ctx, const std::vector<SoundValue>& inputs, std::vector<SoundValue>& outputs) const = 0;
};

class SoundGraph {
public:
    SoundNodeID AddNode(std::unique_ptr<SoundNode> node);
    void RemoveNode(SoundNodeID id);
    void AddEdge(const SoundEdge& edge);
    void RemoveEdge(const SoundEdge& edge);
    bool Compile();
    bool Execute(const SoundContext& ctx);
    const SoundValue* GetOutput(SoundNodeID node, SoundPortID port) const;
    size_t NodeCount() const;
    bool IsCompiled() const;
private:
    SoundNodeID m_nextID = 1;
    std::unordered_map<SoundNodeID, std::unique_ptr<SoundNode>> m_nodes;
    std::vector<SoundEdge> m_edges;
    std::vector<SoundNodeID> m_executionOrder;
    bool m_compiled = false;
    std::unordered_map<uint64_t, SoundValue> m_outputs;
    bool HasCycle() const;
    bool ValidateEdgeTypes() const;
};

}
