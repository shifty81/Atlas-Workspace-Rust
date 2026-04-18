#pragma once
// NF::Editor — Layout manager v1: named layout slots, save/load, active layout tracking
#include "NF/Workspace/PanelStateSerializer.h"
#include "NF/Workspace/DockTreeSerializer.h"
#include <cstdint>
#include <string>
#include <vector>

namespace NF {

// ── Layout Slot ───────────────────────────────────────────────────

struct LayoutSlot {
    uint32_t    id          = 0;
    std::string name;
    std::string description;
    std::string dockTreeData;          // serialized DockTree
    std::string panelStateData;        // serialized PanelStateEntry collection
    bool        isBuiltIn   = false;   // built-in presets cannot be deleted
    bool        isDirty     = false;

    [[nodiscard]] bool isValid() const { return id != 0 && !name.empty(); }
    [[nodiscard]] bool isEmpty()  const {
        return dockTreeData.empty() && panelStateData.empty();
    }
};

// ── Layout Manager V1 ─────────────────────────────────────────────

class LayoutManagerV1 {
public:
    static constexpr size_t MAX_SLOTS = 32;

    // Register a layout slot
    bool addSlot(const LayoutSlot& slot) {
        if (!slot.isValid()) return false;
        if (m_slots.size() >= MAX_SLOTS) return false;
        for (const auto& s : m_slots) if (s.id == slot.id) return false;
        m_slots.push_back(slot);
        return true;
    }

    bool removeSlot(uint32_t id) {
        auto* s = findSlot(id);
        if (!s || s->isBuiltIn) return false;
        m_slots.erase(std::remove_if(m_slots.begin(), m_slots.end(),
            [id](const LayoutSlot& sl) { return sl.id == id; }), m_slots.end());
        if (m_activeSlotId == id) m_activeSlotId = 0;
        return true;
    }

    [[nodiscard]] LayoutSlot* findSlot(uint32_t id) {
        for (auto& s : m_slots) if (s.id == id) return &s;
        return nullptr;
    }
    [[nodiscard]] const LayoutSlot* findSlot(uint32_t id) const {
        for (const auto& s : m_slots) if (s.id == id) return &s;
        return nullptr;
    }
    [[nodiscard]] const LayoutSlot* findSlotByName(const std::string& name) const {
        for (const auto& s : m_slots) if (s.name == name) return &s;
        return nullptr;
    }

    // Save current dock tree + panel state into a slot
    bool saveToSlot(uint32_t id, const DockTree& tree,
                    const std::vector<PanelStateEntry>& panels) {
        auto* slot = findSlot(id);
        if (!slot) return false;
        slot->dockTreeData  = DockTreeSerializer::serialize(tree);
        slot->panelStateData = PanelStateSerializer::serialize(panels);
        slot->isDirty = false;
        ++m_saveCount;
        return true;
    }

    // Load a slot into tree + panels
    bool loadFromSlot(uint32_t id, DockTree& outTree,
                      std::vector<PanelStateEntry>& outPanels) const {
        const auto* slot = findSlot(id);
        if (!slot || slot->isEmpty()) return false;
        DockTreeSerializer::deserialize(slot->dockTreeData, outTree);
        PanelStateSerializer::deserialize(slot->panelStateData, outPanels);
        return true;
    }

    bool setActiveSlot(uint32_t id) {
        if (!findSlot(id)) return false;
        m_activeSlotId = id;
        return true;
    }

    void markDirty(uint32_t id) {
        auto* s = findSlot(id);
        if (s) s->isDirty = true;
    }

    // Load named built-in default layouts
    void loadDefaults() {
        auto mk = [&](uint32_t id, const char* name, const char* desc) {
            LayoutSlot s; s.id = id; s.name = name; s.description = desc;
            s.isBuiltIn = true;
            addSlot(s);
        };
        mk(1, "Default",     "Standard workspace layout");
        mk(2, "Coding",      "Focus layout for code editing");
        mk(3, "Debugging",   "Layout optimized for debugging sessions");
        mk(4, "AssetBrowser","Layout with asset browser prominent");
    }

    [[nodiscard]] uint32_t activeSlotId()  const { return m_activeSlotId; }
    [[nodiscard]] size_t   slotCount()     const { return m_slots.size();  }
    [[nodiscard]] size_t   saveCount()     const { return m_saveCount;     }
    [[nodiscard]] const std::vector<LayoutSlot>& slots() const { return m_slots; }

    [[nodiscard]] size_t dirtySlotCount() const {
        size_t n = 0;
        for (const auto& s : m_slots) if (s.isDirty) ++n;
        return n;
    }

private:
    std::vector<LayoutSlot> m_slots;
    uint32_t m_activeSlotId = 0;
    size_t   m_saveCount    = 0;
};

} // namespace NF
