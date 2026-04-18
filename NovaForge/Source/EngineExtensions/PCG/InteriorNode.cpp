#include "InteriorNode.h"
#include <algorithm>

namespace atlas::procedural {

size_t InteriorNode::AddSlot(const ModuleSlot& slot) {
    m_slots.push_back(slot);
    m_modules.emplace_back();  // keep parallel vector in sync
    return m_slots.size() - 1;
}

bool InteriorNode::PlaceModule(size_t slotIndex, const InteriorModule& mod) {
    if (slotIndex >= m_slots.size()) return false;

    ModuleSlot& slot = m_slots[slotIndex];
    if (slot.occupied) return false;
    if (mod.size != slot.size) return false;

    // Validate that the module type is in the slot's allowed list.
    bool typeAllowed = slot.allowedTypes.empty();  // empty = any type
    for (auto allowed : slot.allowedTypes) {
        if (allowed == mod.type) { typeAllowed = true; break; }
    }
    if (!typeAllowed) return false;

    slot.occupied = true;
    slot.occupantId = mod.id;
    m_modules[slotIndex] = mod;
    return true;
}

void InteriorNode::RemoveModule(size_t slotIndex) {
    if (slotIndex >= m_slots.size()) return;
    m_slots[slotIndex].occupied = false;
    m_slots[slotIndex].occupantId = 0;
    m_modules[slotIndex] = InteriorModule{};
}

const InteriorModule* InteriorNode::GetModule(size_t slotIndex) const {
    if (slotIndex >= m_slots.size()) return nullptr;
    if (!m_slots[slotIndex].occupied) return nullptr;
    return &m_modules[slotIndex];
}

const ModuleSlot* InteriorNode::GetSlot(size_t index) const {
    if (index >= m_slots.size()) return nullptr;
    return &m_slots[index];
}

size_t InteriorNode::OccupiedSlotCount() const {
    size_t count = 0;
    for (auto& s : m_slots) {
        if (s.occupied) ++count;
    }
    return count;
}

std::vector<size_t> InteriorNode::FindSlotsByType(ModuleType type) const {
    std::vector<size_t> result;
    for (size_t i = 0; i < m_slots.size(); ++i) {
        auto& allowed = m_slots[i].allowedTypes;
        if (allowed.empty()) {
            result.push_back(i);
            continue;
        }
        for (auto a : allowed) {
            if (a == type) { result.push_back(i); break; }
        }
    }
    return result;
}

std::vector<size_t> InteriorNode::FindSlotsBySize(ModuleSize size) const {
    std::vector<size_t> result;
    for (size_t i = 0; i < m_slots.size(); ++i) {
        if (m_slots[i].size == size) result.push_back(i);
    }
    return result;
}

size_t InteriorNode::TotalModuleCount() const {
    return OccupiedSlotCount();
}

std::vector<InteriorModule> InteriorNode::GetModulesByType(ModuleType type) const {
    std::vector<InteriorModule> result;
    for (size_t i = 0; i < m_slots.size(); ++i) {
        if (m_slots[i].occupied && m_modules[i].type == type) {
            result.push_back(m_modules[i]);
        }
    }
    return result;
}

void InteriorNode::Clear() {
    for (size_t i = 0; i < m_slots.size(); ++i) {
        m_slots[i].occupied = false;
        m_slots[i].occupantId = 0;
        m_modules[i] = InteriorModule{};
    }
}

bool InteriorNode::HasPower() const {
    for (size_t i = 0; i < m_slots.size(); ++i) {
        if (m_slots[i].occupied &&
            m_modules[i].type == ModuleType::PowerPlant &&
            m_modules[i].IsOperational()) {
            return true;
        }
    }
    return false;
}

} // namespace atlas::procedural
