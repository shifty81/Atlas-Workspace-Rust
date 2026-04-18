#pragma once
#include "ITool.h"
#include <cstdint>
#include <string>
#include <vector>

namespace atlas::tools {

/// Record of a spawned NPC.
struct SpawnedNPC {
    uint32_t entityId = 0;
    std::string templateName;
    float posX = 0, posY = 0, posZ = 0;
};

/// In-client overlay tool for spawning NPCs during play.
///
/// Place NPC entities into the running scene from a catalogue of
/// templates (faction, type, difficulty level).
class NPCSpawnerTool : public ITool {
public:
    std::string Name() const override { return "NPC Spawner"; }
    void Activate() override;
    void Deactivate() override;
    void Update(float deltaTime) override;
    bool IsActive() const override { return m_active; }

    // Template selection
    void SetTemplate(const std::string& templateName);
    std::string CurrentTemplate() const { return m_template; }

    // Spawn at position (returns assigned entity id)
    uint32_t Spawn(float x, float y, float z);

    // History
    const std::vector<SpawnedNPC>& SpawnHistory() const { return m_history; }
    size_t SpawnCount() const { return m_history.size(); }

    // Remove last spawn
    bool UndoLastSpawn();

private:
    bool m_active = false;
    std::string m_template = "default_npc";
    std::vector<SpawnedNPC> m_history;
    uint32_t m_nextEntityId = 1000;
};

} // namespace atlas::tools
