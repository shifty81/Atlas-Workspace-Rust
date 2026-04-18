#pragma once
// NF::Editor — Mesh optimizer tool
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

enum class MeshOptimizeTarget : uint8_t {
    Vertices, Triangles, DrawCalls, Memory, All
};

inline const char* meshOptimizeTargetName(MeshOptimizeTarget t) {
    switch (t) {
        case MeshOptimizeTarget::Vertices:  return "Vertices";
        case MeshOptimizeTarget::Triangles: return "Triangles";
        case MeshOptimizeTarget::DrawCalls: return "DrawCalls";
        case MeshOptimizeTarget::Memory:    return "Memory";
        case MeshOptimizeTarget::All:       return "All";
    }
    return "Unknown";
}

enum class MeshWeldMode : uint8_t {
    None, ByPosition, ByNormal, ByUV, All
};

inline const char* meshWeldModeName(MeshWeldMode m) {
    switch (m) {
        case MeshWeldMode::None:       return "None";
        case MeshWeldMode::ByPosition: return "ByPosition";
        case MeshWeldMode::ByNormal:   return "ByNormal";
        case MeshWeldMode::ByUV:       return "ByUV";
        case MeshWeldMode::All:        return "All";
    }
    return "Unknown";
}

enum class MeshOptimizeStatus : uint8_t {
    Idle, Running, Done, Failed, Cancelled
};

inline const char* meshOptimizeStatusName(MeshOptimizeStatus s) {
    switch (s) {
        case MeshOptimizeStatus::Idle:      return "Idle";
        case MeshOptimizeStatus::Running:   return "Running";
        case MeshOptimizeStatus::Done:      return "Done";
        case MeshOptimizeStatus::Failed:    return "Failed";
        case MeshOptimizeStatus::Cancelled: return "Cancelled";
    }
    return "Unknown";
}

class MeshOptimizeJob {
public:
    explicit MeshOptimizeJob(const std::string& meshName,
                              MeshOptimizeTarget target)
        : m_meshName(meshName), m_target(target) {}

    void setWeldMode(MeshWeldMode m)   { m_weldMode    = m;    }
    void setStatus(MeshOptimizeStatus s) { m_status    = s;    }
    void setReductionPct(float pct)    { m_reductionPct = pct; }
    void setOriginalTriCount(uint32_t n) { m_origTris  = n;    }
    void setResultTriCount(uint32_t n)   { m_resultTris = n;   }

    [[nodiscard]] const std::string&  meshName()       const { return m_meshName;      }
    [[nodiscard]] MeshOptimizeTarget  target()         const { return m_target;        }
    [[nodiscard]] MeshWeldMode        weldMode()       const { return m_weldMode;      }
    [[nodiscard]] MeshOptimizeStatus  status()         const { return m_status;        }
    [[nodiscard]] float               reductionPct()   const { return m_reductionPct;  }
    [[nodiscard]] uint32_t            originalTris()   const { return m_origTris;      }
    [[nodiscard]] uint32_t            resultTris()     const { return m_resultTris;    }

    [[nodiscard]] bool isComplete() const { return m_status == MeshOptimizeStatus::Done; }
    [[nodiscard]] bool isRunning()  const { return m_status == MeshOptimizeStatus::Running; }

private:
    std::string         m_meshName;
    MeshOptimizeTarget  m_target;
    MeshWeldMode        m_weldMode      = MeshWeldMode::ByPosition;
    MeshOptimizeStatus  m_status        = MeshOptimizeStatus::Idle;
    float               m_reductionPct  = 0.0f;
    uint32_t            m_origTris      = 0;
    uint32_t            m_resultTris    = 0;
};

class MeshOptimizerPanel {
public:
    static constexpr size_t MAX_JOBS = 128;

    [[nodiscard]] bool addJob(const MeshOptimizeJob& job) {
        for (auto& j : m_jobs) if (j.meshName() == job.meshName()) return false;
        if (m_jobs.size() >= MAX_JOBS) return false;
        m_jobs.push_back(job);
        return true;
    }

    [[nodiscard]] bool removeJob(const std::string& meshName) {
        for (auto it = m_jobs.begin(); it != m_jobs.end(); ++it) {
            if (it->meshName() == meshName) { m_jobs.erase(it); return true; }
        }
        return false;
    }

    [[nodiscard]] MeshOptimizeJob* findJob(const std::string& meshName) {
        for (auto& j : m_jobs) if (j.meshName() == meshName) return &j;
        return nullptr;
    }

    [[nodiscard]] size_t jobCount()     const { return m_jobs.size(); }
    [[nodiscard]] size_t completedCount() const {
        size_t c = 0; for (auto& j : m_jobs) if (j.isComplete()) ++c; return c;
    }
    [[nodiscard]] size_t runningCount() const {
        size_t c = 0; for (auto& j : m_jobs) if (j.isRunning())  ++c; return c;
    }
    [[nodiscard]] size_t countByTarget(MeshOptimizeTarget t) const {
        size_t c = 0; for (auto& j : m_jobs) if (j.target() == t) ++c; return c;
    }
    [[nodiscard]] size_t countByStatus(MeshOptimizeStatus s) const {
        size_t c = 0; for (auto& j : m_jobs) if (j.status() == s) ++c; return c;
    }

private:
    std::vector<MeshOptimizeJob> m_jobs;
};

} // namespace NF
