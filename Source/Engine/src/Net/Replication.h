#pragma once
#include <cstdint>
#include <vector>
#include <string>
#include <unordered_map>
#include <unordered_set>
#include <functional>

namespace atlas::ecs { class World; }

namespace atlas::net {

enum class ReplicateFrequency {
    EveryTick,
    OnChange,
    Manual
};

enum class ReplicateDirection {
    ServerToClient,
    ClientToServer,
    Bidirectional
};

struct ReplicationRule {
    uint32_t typeTag = 0;
    std::string componentName;
    ReplicateFrequency frequency = ReplicateFrequency::OnChange;
    ReplicateDirection direction = ReplicateDirection::ServerToClient;
    bool reliable = true;
    uint8_t priority = 128;
};

class ReplicationManager {
public:
    void SetWorld(ecs::World* world);

    void AddRule(const ReplicationRule& rule);
    void RemoveRule(uint32_t typeTag);
    bool HasRule(uint32_t typeTag) const;
    const ReplicationRule* GetRule(uint32_t typeTag) const;

    const std::vector<ReplicationRule>& Rules() const;
    size_t RuleCount() const;

    // Collect dirty components based on rules and produce a delta payload (reliable only)
    std::vector<uint8_t> CollectDelta(uint32_t tick);

    // Collect delta for unreliable rules only
    std::vector<uint8_t> CollectUnreliableDelta(uint32_t tick);

    // Trigger replication for a Manual-frequency component type
    void TriggerManualReplication(uint32_t typeTag);

    // Apply a received delta payload to the local world
    bool ApplyDelta(const std::vector<uint8_t>& data);

    // Mark a component type as dirty (for OnChange mode)
    void MarkDirty(uint32_t typeTag, uint32_t entityID);

    // Query dirty state
    bool IsDirty(uint32_t typeTag, uint32_t entityID) const;

    // Clear all dirty flags (called after CollectDelta)
    void ClearDirty();

    // Set callback for reliable delta payloads
    void SetReliableCallback(std::function<void(const std::vector<uint8_t>&)> cb);

    // Set callback for unreliable delta payloads
    void SetUnreliableCallback(std::function<void(const std::vector<uint8_t>&)> cb);

private:
    // Shared implementation for CollectDelta and CollectUnreliableDelta
    std::vector<uint8_t> CollectDeltaFiltered(uint32_t tick, bool collectReliable);

    ecs::World* m_world = nullptr;
    std::vector<ReplicationRule> m_rules;
    // typeTag -> set of dirty entity IDs
    std::unordered_map<uint32_t, std::vector<uint32_t>> m_dirty;

    // Manual replication triggers
    std::unordered_set<uint32_t> m_manuallyTriggered;

    // Callbacks for reliable/unreliable deltas
    std::function<void(const std::vector<uint8_t>&)> m_reliableCallback;
    std::function<void(const std::vector<uint8_t>&)> m_unreliableCallback;
};

}
