#include "NPCSpawnerTool.h"

namespace atlas::tools {

void NPCSpawnerTool::Activate() {
    m_active = true;
}

void NPCSpawnerTool::Deactivate() {
    m_active = false;
}

void NPCSpawnerTool::Update(float /*deltaTime*/) {
    // NPC spawner is event-driven (Spawn calls), not per-frame.
}

void NPCSpawnerTool::SetTemplate(const std::string& templateName) {
    m_template = templateName;
}

uint32_t NPCSpawnerTool::Spawn(float x, float y, float z) {
    uint32_t eid = m_nextEntityId++;
    SpawnedNPC npc;
    npc.entityId = eid;
    npc.templateName = m_template;
    npc.posX = x; npc.posY = y; npc.posZ = z;
    m_history.push_back(npc);
    return eid;
}

bool NPCSpawnerTool::UndoLastSpawn() {
    if (m_history.empty()) return false;
    m_history.pop_back();
    return true;
}

} // namespace atlas::tools
