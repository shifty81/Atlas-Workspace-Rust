#pragma once
// NF::Editor — Virtual production editor v1: take and stage management for VP workflows
#include "NF/Core/Core.h"
#include <algorithm>
#include <cstdint>
#include <functional>
#include <string>
#include <vector>

namespace NF {

enum class Vpv1StageType    : uint8_t { SoundStage, VirtualStage, LocationShoot, Composite, Greenscreen };
enum class Vpv1TakeStatus   : uint8_t { Pending, Active, Complete, Void, Hold };
enum class Vpv1OutputFormat : uint8_t { Film4K, UHD, HD, ProRes, RAW };

inline const char* vpv1StageTypeName(Vpv1StageType t) {
    switch (t) {
        case Vpv1StageType::SoundStage:     return "SoundStage";
        case Vpv1StageType::VirtualStage:   return "VirtualStage";
        case Vpv1StageType::LocationShoot:  return "LocationShoot";
        case Vpv1StageType::Composite:      return "Composite";
        case Vpv1StageType::Greenscreen:    return "Greenscreen";
    }
    return "Unknown";
}

inline const char* vpv1TakeStatusName(Vpv1TakeStatus s) {
    switch (s) {
        case Vpv1TakeStatus::Pending:  return "Pending";
        case Vpv1TakeStatus::Active:   return "Active";
        case Vpv1TakeStatus::Complete: return "Complete";
        case Vpv1TakeStatus::Void:     return "Void";
        case Vpv1TakeStatus::Hold:     return "Hold";
    }
    return "Unknown";
}

inline const char* vpv1OutputFormatName(Vpv1OutputFormat f) {
    switch (f) {
        case Vpv1OutputFormat::Film4K:  return "Film4K";
        case Vpv1OutputFormat::UHD:     return "UHD";
        case Vpv1OutputFormat::HD:      return "HD";
        case Vpv1OutputFormat::ProRes:  return "ProRes";
        case Vpv1OutputFormat::RAW:     return "RAW";
    }
    return "Unknown";
}

struct Vpv1Take {
    uint64_t          id           = 0;
    std::string       name;
    std::string       scene;
    Vpv1StageType     stageType    = Vpv1StageType::VirtualStage;
    Vpv1TakeStatus    status       = Vpv1TakeStatus::Pending;
    Vpv1OutputFormat  outputFormat = Vpv1OutputFormat::UHD;
    float             duration     = 0.f;
    uint32_t          takeNumber   = 1;
    bool              isFeatured   = false;

    [[nodiscard]] bool isValid()    const { return id != 0 && !name.empty(); }
    [[nodiscard]] bool isActive()   const { return status == Vpv1TakeStatus::Active; }
    [[nodiscard]] bool isComplete() const { return status == Vpv1TakeStatus::Complete; }
};

using Vpv1ChangeCallback = std::function<void(uint64_t)>;

class VirtualProductionEditorV1 {
public:
    static constexpr size_t MAX_TAKES = 512;

    bool addTake(const Vpv1Take& take) {
        if (!take.isValid()) return false;
        for (const auto& t : m_takes) if (t.id == take.id) return false;
        if (m_takes.size() >= MAX_TAKES) return false;
        m_takes.push_back(take);
        return true;
    }

    bool removeTake(uint64_t id) {
        for (auto it = m_takes.begin(); it != m_takes.end(); ++it) {
            if (it->id == id) {
                if (m_activeId == id) m_activeId = 0;
                m_takes.erase(it);
                return true;
            }
        }
        return false;
    }

    [[nodiscard]] Vpv1Take* findTake(uint64_t id) {
        for (auto& t : m_takes) if (t.id == id) return &t;
        return nullptr;
    }

    bool setActive(uint64_t id) {
        for (const auto& t : m_takes) if (t.id == id) { m_activeId = id; return true; }
        return false;
    }

    bool setStatus(uint64_t id, Vpv1TakeStatus status) {
        auto* t = findTake(id);
        if (!t) return false;
        t->status = status;
        if (m_onChange) m_onChange(id);
        return true;
    }

    bool setOutputFormat(uint64_t id, Vpv1OutputFormat fmt) {
        auto* t = findTake(id);
        if (!t) return false;
        t->outputFormat = fmt;
        if (m_onChange) m_onChange(id);
        return true;
    }

    bool setFeatured(uint64_t id, bool featured) {
        auto* t = findTake(id);
        if (!t) return false;
        t->isFeatured = featured;
        if (m_onChange) m_onChange(id);
        return true;
    }

    [[nodiscard]] uint64_t activeId()       const { return m_activeId; }
    [[nodiscard]] size_t   takeCount()      const { return m_takes.size(); }
    [[nodiscard]] size_t   completeCount()  const {
        size_t c = 0; for (const auto& t : m_takes) if (t.isComplete()) ++c; return c;
    }
    [[nodiscard]] size_t   featuredCount()  const {
        size_t c = 0; for (const auto& t : m_takes) if (t.isFeatured)   ++c; return c;
    }
    [[nodiscard]] size_t   countByStage(Vpv1StageType st) const {
        size_t c = 0; for (const auto& t : m_takes) if (t.stageType == st) ++c; return c;
    }
    [[nodiscard]] size_t   countByStatus(Vpv1TakeStatus s) const {
        size_t c = 0; for (const auto& t : m_takes) if (t.status == s) ++c; return c;
    }

    void setOnChange(Vpv1ChangeCallback cb) { m_onChange = std::move(cb); }

private:
    std::vector<Vpv1Take> m_takes;
    uint64_t              m_activeId = 0;
    Vpv1ChangeCallback    m_onChange;
};

} // namespace NF
