#pragma once
// NF::Editor — game save data management editor
#include "NF/Core/Core.h"
#include "NF/Engine/Engine.h"
#include "NF/Renderer/Renderer.h"
#include "NF/UI/UI.h"
#include "NF/GraphVM/GraphVM.h"
#include "NF/Input/Input.h"
#include "NF/UI/UIWidgets.h"
#include <filesystem>
#include <fstream>
#include <set>
#include <map>
#include <deque>
#include <unordered_map>

namespace NF {

enum class SaveDataSlot : uint8_t { Auto, Manual, Checkpoint, Cloud, Local };
inline const char* saveDataSlotName(SaveDataSlot v) {
    switch (v) {
        case SaveDataSlot::Auto:       return "Auto";
        case SaveDataSlot::Manual:     return "Manual";
        case SaveDataSlot::Checkpoint: return "Checkpoint";
        case SaveDataSlot::Cloud:      return "Cloud";
        case SaveDataSlot::Local:      return "Local";
    }
    return "Unknown";
}

enum class SaveDataState : uint8_t { Empty, Valid, Corrupt, Migrating, Locked };
inline const char* saveDataStateName(SaveDataState v) {
    switch (v) {
        case SaveDataState::Empty:     return "Empty";
        case SaveDataState::Valid:     return "Valid";
        case SaveDataState::Corrupt:   return "Corrupt";
        case SaveDataState::Migrating: return "Migrating";
        case SaveDataState::Locked:    return "Locked";
    }
    return "Unknown";
}

class SaveDataEntry {
public:
    explicit SaveDataEntry(uint32_t id, const std::string& name, SaveDataSlot slot)
        : m_id(id), m_name(name), m_slot(slot) {}

    void setState(SaveDataState v)   { m_state        = v; }
    void setSizeKB(uint32_t v)        { m_sizeKB         = v; }
    void setIsCompressed(bool v)     { m_isCompressed  = v; }
    void setIsEnabled(bool v)        { m_isEnabled     = v; }

    [[nodiscard]] uint32_t           id()           const { return m_id;           }
    [[nodiscard]] const std::string& name()         const { return m_name;         }
    [[nodiscard]] SaveDataSlot       slot()         const { return m_slot;         }
    [[nodiscard]] SaveDataState      state()        const { return m_state;        }
    [[nodiscard]] uint32_t           sizeKB()        const { return m_sizeKB;         }
    [[nodiscard]] bool               isCompressed() const { return m_isCompressed; }
    [[nodiscard]] bool               isEnabled()    const { return m_isEnabled;    }

private:
    uint32_t      m_id;
    std::string   m_name;
    SaveDataSlot  m_slot;
    SaveDataState m_state        = SaveDataState::Empty;
    uint32_t      m_sizeKB         = 0u;
    bool          m_isCompressed  = true;
    bool          m_isEnabled    = true;
};

class SaveDataEditor {
public:
    void setIsShowEmpty(bool v)        { m_isShowEmpty      = v; }
    void setIsGroupBySlot(bool v)      { m_isGroupBySlot    = v; }
    void setMaxSlotsPerUser(uint32_t v){ m_maxSlotsPerUser  = v; }

    bool addSaveData(const SaveDataEntry& e) {
        for (auto& x : m_entries) if (x.id() == e.id()) return false;
        m_entries.push_back(e); return true;
    }
    bool removeSaveData(uint32_t id) {
        auto it = std::find_if(m_entries.begin(), m_entries.end(),
            [&](const SaveDataEntry& e){ return e.id() == id; });
        if (it == m_entries.end()) return false;
        m_entries.erase(it); return true;
    }
    [[nodiscard]] SaveDataEntry* findSaveData(uint32_t id) {
        for (auto& e : m_entries) if (e.id() == id) return &e;
        return nullptr;
    }

    [[nodiscard]] bool     isShowEmpty()      const { return m_isShowEmpty;      }
    [[nodiscard]] bool     isGroupBySlot()    const { return m_isGroupBySlot;    }
    [[nodiscard]] uint32_t maxSlotsPerUser()  const { return m_maxSlotsPerUser;  }
    [[nodiscard]] size_t   saveDataCount()    const { return m_entries.size();   }

    [[nodiscard]] size_t countBySlot(SaveDataSlot s) const {
        size_t n = 0; for (auto& e : m_entries) if (e.slot() == s) ++n; return n;
    }
    [[nodiscard]] size_t countByState(SaveDataState s) const {
        size_t n = 0; for (auto& e : m_entries) if (e.state() == s) ++n; return n;
    }
    [[nodiscard]] size_t countEnabled() const {
        size_t n = 0; for (auto& e : m_entries) if (e.isEnabled()) ++n; return n;
    }

private:
    std::vector<SaveDataEntry> m_entries;
    bool     m_isShowEmpty     = true;
    bool     m_isGroupBySlot   = false;
    uint32_t m_maxSlotsPerUser = 10u;
};

} // namespace NF
