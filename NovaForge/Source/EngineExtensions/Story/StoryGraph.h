#pragma once
#include <cstdint>
#include <string>
#include <unordered_map>
#include <vector>

namespace atlas::story {

enum class StoryNodeType : uint8_t {
    State,
    Character,
    Relationship,
    Goal,
    Conflict,
    Event,
    Choice,
    Outcome,
    LoreFact
};

struct StoryNode {
    uint32_t id = 0;
    StoryNodeType type = StoryNodeType::State;
    std::string name;
    std::string description;
    std::vector<std::string> tags;
    std::vector<uint32_t> connections;
};

class StoryGraph {
public:
    void Init();
    uint32_t AddNode(StoryNodeType type, const std::string& name, const std::string& description);
    void RemoveNode(uint32_t id);
    const StoryNode* GetNode(uint32_t id) const;
    size_t NodeCount() const;

    void Connect(uint32_t fromId, uint32_t toId);
    std::vector<uint32_t> GetConnections(uint32_t fromId) const;

    void AddTag(uint32_t id, const std::string& tag);
    std::vector<const StoryNode*> GetNodesByTag(const std::string& tag) const;
    std::vector<const StoryNode*> GetNodesByType(StoryNodeType type) const;

private:
    std::unordered_map<uint32_t, StoryNode> m_nodes;
    uint32_t m_nextId = 1;
};

} // namespace atlas::story
