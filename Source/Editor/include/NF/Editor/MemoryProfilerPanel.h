#pragma once
// NF::Editor — Memory profiler panel
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

enum class MemoryCategory : uint8_t {
    Textures, Meshes, Audio, Animations, Scripts, Physics, AI, Misc
};

inline const char* memoryCategoryName(MemoryCategory c) {
    switch (c) {
        case MemoryCategory::Textures:   return "Textures";
        case MemoryCategory::Meshes:     return "Meshes";
        case MemoryCategory::Audio:      return "Audio";
        case MemoryCategory::Animations: return "Animations";
        case MemoryCategory::Scripts:    return "Scripts";
        case MemoryCategory::Physics:    return "Physics";
        case MemoryCategory::AI:         return "AI";
        case MemoryCategory::Misc:       return "Misc";
    }
    return "Unknown";
}

enum class MemorySortOrder : uint8_t {
    BySize, ByCategory, ByName, ByAge, ByRefCount
};

inline const char* memorySortOrderName(MemorySortOrder o) {
    switch (o) {
        case MemorySortOrder::BySize:     return "BySize";
        case MemorySortOrder::ByCategory: return "ByCategory";
        case MemorySortOrder::ByName:     return "ByName";
        case MemorySortOrder::ByAge:      return "ByAge";
        case MemorySortOrder::ByRefCount: return "ByRefCount";
    }
    return "Unknown";
}

enum class MemorySnapshotState : uint8_t {
    Empty, Capturing, Ready, Comparing
};

inline const char* memorySnapshotStateName(MemorySnapshotState s) {
    switch (s) {
        case MemorySnapshotState::Empty:     return "Empty";
        case MemorySnapshotState::Capturing: return "Capturing";
        case MemorySnapshotState::Ready:     return "Ready";
        case MemorySnapshotState::Comparing: return "Comparing";
    }
    return "Unknown";
}

class MemoryRecord {
public:
    explicit MemoryRecord(const std::string& name, MemoryCategory cat, size_t bytes)
        : m_name(name), m_category(cat), m_bytes(bytes) {}

    void setRefCount(uint32_t r)   { m_refCount = r; }
    void setResident(bool v)       { m_resident = v; }
    void setFlagged(bool v)        { m_flagged  = v; }

    [[nodiscard]] const std::string& name()      const { return m_name;     }
    [[nodiscard]] MemoryCategory     category()  const { return m_category; }
    [[nodiscard]] size_t             bytes()     const { return m_bytes;    }
    [[nodiscard]] uint32_t           refCount()  const { return m_refCount; }
    [[nodiscard]] bool               isResident()const { return m_resident; }
    [[nodiscard]] bool               isFlagged() const { return m_flagged;  }

private:
    std::string    m_name;
    MemoryCategory m_category;
    size_t         m_bytes;
    uint32_t       m_refCount = 1;
    bool           m_resident = true;
    bool           m_flagged  = false;
};

class MemoryProfilerPanel {
public:
    void setSortOrder(MemorySortOrder o)      { m_sortOrder     = o; }
    void setSnapshotState(MemorySnapshotState s){ m_snapshotState = s; }
    void setFilterCategory(MemoryCategory c)  { m_filterCat = c; m_filtering = true; }
    void clearFilter()                         { m_filtering = false; }

    void addRecord(const MemoryRecord& rec) { m_records.push_back(rec); }

    void clearRecords()                     { m_records.clear(); }

    [[nodiscard]] MemorySortOrder    sortOrder()     const { return m_sortOrder;     }
    [[nodiscard]] MemorySnapshotState snapshotState()const { return m_snapshotState; }
    [[nodiscard]] bool               isFiltering()  const { return m_filtering;     }
    [[nodiscard]] size_t             recordCount()  const { return m_records.size(); }

    [[nodiscard]] size_t totalBytes() const {
        size_t t = 0; for (auto& r : m_records) t += r.bytes(); return t;
    }
    [[nodiscard]] size_t flaggedCount() const {
        size_t c = 0; for (auto& r : m_records) if (r.isFlagged()) ++c; return c;
    }
    [[nodiscard]] size_t countByCategory(MemoryCategory cat) const {
        size_t c = 0; for (auto& r : m_records) if (r.category() == cat) ++c; return c;
    }
    [[nodiscard]] size_t bytesForCategory(MemoryCategory cat) const {
        size_t t = 0; for (auto& r : m_records) if (r.category() == cat) t += r.bytes(); return t;
    }

private:
    std::vector<MemoryRecord>  m_records;
    MemorySortOrder            m_sortOrder     = MemorySortOrder::BySize;
    MemorySnapshotState        m_snapshotState = MemorySnapshotState::Empty;
    MemoryCategory             m_filterCat     = MemoryCategory::Misc;
    bool                       m_filtering     = false;
};

} // namespace NF
