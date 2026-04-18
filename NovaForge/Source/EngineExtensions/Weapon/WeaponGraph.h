#pragma once
#include <cstdint>
#include <vector>
#include <string>
#include <memory>
#include <unordered_map>

namespace atlas::weapon {

enum class WeaponPinType : uint8_t {
    Float,      // Scalar (damage, range, fire rate)
    Stats,      // Computed weapon statistics
    Seed,       // RNG seed
    Component,  // Weapon component (barrel, receiver, etc.)
    Profile     // Weapon behavior profile (recoil, spread)
};

struct WeaponValue {
    WeaponPinType type;
    std::vector<float> data;
};

struct WeaponPort {
    std::string name;
    WeaponPinType type;
};

using WeaponNodeID = uint32_t;
using WeaponPortID = uint16_t;

struct WeaponEdge {
    WeaponNodeID fromNode;
    WeaponPortID fromPort;
    WeaponNodeID toNode;
    WeaponPortID toPort;
};

struct WeaponContext {
    uint64_t seed;
    float wearLevel;       // 0.0 = pristine, 1.0 = worn out
    uint32_t factionID;
};

class WeaponNode {
public:
    virtual ~WeaponNode() = default;
    virtual const char* GetName() const = 0;
    virtual const char* GetCategory() const = 0;
    virtual std::vector<WeaponPort> Inputs() const = 0;
    virtual std::vector<WeaponPort> Outputs() const = 0;
    virtual void Evaluate(const WeaponContext& ctx,
                          const std::vector<WeaponValue>& inputs,
                          std::vector<WeaponValue>& outputs) const = 0;
};

class WeaponGraph {
public:
    WeaponNodeID AddNode(std::unique_ptr<WeaponNode> node);
    void RemoveNode(WeaponNodeID id);
    void AddEdge(const WeaponEdge& edge);
    void RemoveEdge(const WeaponEdge& edge);

    bool Compile();
    bool Execute(const WeaponContext& ctx);

    const WeaponValue* GetOutput(WeaponNodeID node, WeaponPortID port) const;
    size_t NodeCount() const;
    bool IsCompiled() const;

private:
    WeaponNodeID m_nextID = 1;
    std::unordered_map<WeaponNodeID, std::unique_ptr<WeaponNode>> m_nodes;
    std::vector<WeaponEdge> m_edges;
    std::vector<WeaponNodeID> m_executionOrder;
    bool m_compiled = false;

    std::unordered_map<uint64_t, WeaponValue> m_outputs;

    bool HasCycle() const;
    bool ValidateEdgeTypes() const;
};

}
