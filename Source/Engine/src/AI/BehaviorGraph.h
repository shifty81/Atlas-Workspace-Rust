#pragma once
// ============================================================
// Atlas AI — Behavior Graph
// ============================================================
//
// DETERMINISM CONTRACT:
//   All AI systems MUST produce bit-identical decisions given
//   identical inputs. This is enforced by the following rules:
//
//   1. Randomness: Use ONLY atlas::DeterministicRNG (from
//      core/contract/DeterministicRNG.h). Standard library
//      random facilities and OS entropy are FORBIDDEN.
//
//   2. Decision reproducibility: The same AIMemory + inputs
//      must produce the same BehaviorGraph output on every
//      platform and across save/load boundaries.
//
//   3. Planner state: All BehaviorGraph execution state
//      (active node, blackboard, running status) must be
//      serializable for save/load.
//
//   4. No wall-clock time: AI planning horizons use
//      SimulationTime ticks, never real-time.
//
//   5. No floating-point ambiguity: AI scoring uses
//      deterministic comparison (no platform-dependent FP
//      rounding in decision thresholds).
//
// See: docs/ATLAS_CORE_CONTRACT.md §5 (AI Determinism)
//      docs/ATLAS_DETERMINISM_ENFORCEMENT.md
// ============================================================
#include <cstdint>
#include <vector>
#include <string>
#include <memory>
#include <unordered_map>

namespace atlas::ai {

enum class BehaviorPinType : uint8_t {
    Float,
    Bool,
    Action,
    Perception,
    EmotionState
};

struct BehaviorValue {
    BehaviorPinType type;
    std::vector<float> data;
};

struct BehaviorPort {
    std::string name;
    BehaviorPinType type;
};

using BehaviorNodeID = uint32_t;
using BehaviorPortID = uint16_t;

struct BehaviorEdge {
    BehaviorNodeID fromNode;
    BehaviorPortID fromPort;
    BehaviorNodeID toNode;
    BehaviorPortID toPort;
};

struct AIContext {
    float threatLevel;
    float healthPercent;
    float ammoPercent;
    float morale;
    uint32_t tick;
};

class BehaviorNode {
public:
    virtual ~BehaviorNode() = default;
    virtual const char* GetName() const = 0;
    virtual const char* GetCategory() const = 0;
    virtual std::vector<BehaviorPort> Inputs() const = 0;
    virtual std::vector<BehaviorPort> Outputs() const = 0;
    virtual void Evaluate(const AIContext& ctx,
                          const std::vector<BehaviorValue>& inputs,
                          std::vector<BehaviorValue>& outputs) const = 0;
};

class BehaviorGraph {
public:
    BehaviorNodeID AddNode(std::unique_ptr<BehaviorNode> node);
    void RemoveNode(BehaviorNodeID id);
    void AddEdge(const BehaviorEdge& edge);
    void RemoveEdge(const BehaviorEdge& edge);

    bool Compile();
    bool Execute(const AIContext& ctx);

    const BehaviorValue* GetOutput(BehaviorNodeID node, BehaviorPortID port) const;
    size_t NodeCount() const;
    bool IsCompiled() const;

    /// Serialize execution state (outputs + execution order) for save/load.
    std::vector<uint8_t> SerializeState() const;

    /// Deserialize previously saved execution state.
    bool DeserializeState(const std::vector<uint8_t>& data);

private:
    BehaviorNodeID m_nextID = 1;
    std::unordered_map<BehaviorNodeID, std::unique_ptr<BehaviorNode>> m_nodes;
    std::vector<BehaviorEdge> m_edges;
    std::vector<BehaviorNodeID> m_executionOrder;
    bool m_compiled = false;

    std::unordered_map<uint64_t, BehaviorValue> m_outputs;

    bool HasCycle() const;
    bool ValidateEdgeTypes() const;
};

}
