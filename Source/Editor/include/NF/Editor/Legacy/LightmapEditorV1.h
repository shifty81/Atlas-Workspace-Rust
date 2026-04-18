#pragma once
// NF::Editor — Lightmap editor v1: bake job authoring and UV-chart management
#include "NF/Core/Core.h"
#include <algorithm>
#include <cstdint>
#include <functional>
#include <string>
#include <vector>

namespace NF {

enum class Lmv1BakeQuality  : uint8_t { Draft, Low, Medium, High, Production };
enum class Lmv1BakeMode     : uint8_t { Baked, Mixed, Realtime };
enum class Lmv1BakeStatus   : uint8_t { Idle, Pending, Baking, Denoising, Done, Failed, Cancelled };

inline const char* lmv1BakeQualityName(Lmv1BakeQuality q) {
    switch (q) {
        case Lmv1BakeQuality::Draft:      return "Draft";
        case Lmv1BakeQuality::Low:        return "Low";
        case Lmv1BakeQuality::Medium:     return "Medium";
        case Lmv1BakeQuality::High:       return "High";
        case Lmv1BakeQuality::Production: return "Production";
    }
    return "Unknown";
}

inline const char* lmv1BakeModeName(Lmv1BakeMode m) {
    switch (m) {
        case Lmv1BakeMode::Baked:    return "Baked";
        case Lmv1BakeMode::Mixed:    return "Mixed";
        case Lmv1BakeMode::Realtime: return "Realtime";
    }
    return "Unknown";
}

inline const char* lmv1BakeStatusName(Lmv1BakeStatus s) {
    switch (s) {
        case Lmv1BakeStatus::Idle:       return "Idle";
        case Lmv1BakeStatus::Pending:    return "Pending";
        case Lmv1BakeStatus::Baking:     return "Baking";
        case Lmv1BakeStatus::Denoising:  return "Denoising";
        case Lmv1BakeStatus::Done:       return "Done";
        case Lmv1BakeStatus::Failed:     return "Failed";
        case Lmv1BakeStatus::Cancelled:  return "Cancelled";
    }
    return "Unknown";
}

struct Lmv1BakeJob {
    uint64_t       id           = 0;
    std::string    meshName;
    Lmv1BakeQuality quality     = Lmv1BakeQuality::Medium;
    Lmv1BakeMode   mode         = Lmv1BakeMode::Baked;
    Lmv1BakeStatus status       = Lmv1BakeStatus::Idle;
    uint16_t       textureSizePx = 512;
    float          texelDensity  = 1.0f;
    float          progress      = 0.0f;

    [[nodiscard]] bool isValid()    const { return id != 0 && !meshName.empty() && textureSizePx > 0 && texelDensity > 0.f; }
    [[nodiscard]] bool isDone()     const { return status == Lmv1BakeStatus::Done; }
    [[nodiscard]] bool isActive()   const { return status == Lmv1BakeStatus::Baking || status == Lmv1BakeStatus::Denoising; }
    [[nodiscard]] bool isCancelled()const { return status == Lmv1BakeStatus::Cancelled; }
};

using Lmv1JobCallback = std::function<void(uint64_t jobId)>;

class LightmapEditorV1 {
public:
    static constexpr size_t MAX_JOBS = 256;

    bool addJob(const Lmv1BakeJob& job) {
        if (!job.isValid()) return false;
        for (const auto& j : m_jobs) if (j.id == job.id) return false;
        if (m_jobs.size() >= MAX_JOBS) return false;
        m_jobs.push_back(job);
        return true;
    }

    bool removeJob(uint64_t id) {
        for (auto it = m_jobs.begin(); it != m_jobs.end(); ++it) {
            if (it->id == id) { m_jobs.erase(it); return true; }
        }
        return false;
    }

    [[nodiscard]] Lmv1BakeJob* findJob(uint64_t id) {
        for (auto& j : m_jobs) if (j.id == id) return &j;
        return nullptr;
    }

    bool updateStatus(uint64_t id, Lmv1BakeStatus status, float progress = 0.f) {
        auto* j = findJob(id);
        if (!j) return false;
        j->status   = status;
        j->progress = progress;
        if (m_onChange) m_onChange(id);
        return true;
    }

    bool cancelJob(uint64_t id) {
        return updateStatus(id, Lmv1BakeStatus::Cancelled, 0.f);
    }

    [[nodiscard]] size_t jobCount()      const { return m_jobs.size(); }
    [[nodiscard]] size_t doneCount()     const {
        size_t c = 0; for (const auto& j : m_jobs) if (j.isDone())   ++c; return c;
    }
    [[nodiscard]] size_t activeCount()   const {
        size_t c = 0; for (const auto& j : m_jobs) if (j.isActive()) ++c; return c;
    }
    [[nodiscard]] size_t countByQuality(Lmv1BakeQuality q) const {
        size_t c = 0; for (const auto& j : m_jobs) if (j.quality == q) ++c; return c;
    }
    [[nodiscard]] size_t countByMode(Lmv1BakeMode m) const {
        size_t c = 0; for (const auto& j : m_jobs) if (j.mode == m) ++c; return c;
    }

    void setOnChange(Lmv1JobCallback cb) { m_onChange = std::move(cb); }

private:
    std::vector<Lmv1BakeJob> m_jobs;
    Lmv1JobCallback          m_onChange;
};

} // namespace NF
