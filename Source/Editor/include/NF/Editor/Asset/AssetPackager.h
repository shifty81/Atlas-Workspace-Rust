#pragma once
// NF::Editor — Asset packager for distribution
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

enum class PackageTargetPlatform : uint8_t {
    Windows, Linux, macOS, Android, iOS, Console, Web
};

inline const char* packageTargetPlatformName(PackageTargetPlatform p) {
    switch (p) {
        case PackageTargetPlatform::Windows: return "Windows";
        case PackageTargetPlatform::Linux:   return "Linux";
        case PackageTargetPlatform::macOS:   return "macOS";
        case PackageTargetPlatform::Android: return "Android";
        case PackageTargetPlatform::iOS:     return "iOS";
        case PackageTargetPlatform::Console: return "Console";
        case PackageTargetPlatform::Web:     return "Web";
    }
    return "Unknown";
}

enum class PackageCompressionMode : uint8_t {
    None, Fast, Default, Best, StreamingChunks
};

inline const char* packageCompressionModeName(PackageCompressionMode m) {
    switch (m) {
        case PackageCompressionMode::None:            return "None";
        case PackageCompressionMode::Fast:            return "Fast";
        case PackageCompressionMode::Default:         return "Default";
        case PackageCompressionMode::Best:            return "Best";
        case PackageCompressionMode::StreamingChunks: return "StreamingChunks";
    }
    return "Unknown";
}

enum class PackageJobStatus : uint8_t {
    Queued, Cooking, Packaging, Done, Failed, Cancelled
};

inline const char* packageJobStatusName(PackageJobStatus s) {
    switch (s) {
        case PackageJobStatus::Queued:    return "Queued";
        case PackageJobStatus::Cooking:   return "Cooking";
        case PackageJobStatus::Packaging: return "Packaging";
        case PackageJobStatus::Done:      return "Done";
        case PackageJobStatus::Failed:    return "Failed";
        case PackageJobStatus::Cancelled: return "Cancelled";
    }
    return "Unknown";
}

class PackageJob {
public:
    explicit PackageJob(const std::string& name, PackageTargetPlatform platform)
        : m_name(name), m_platform(platform) {}

    void setCompressionMode(PackageCompressionMode c) { m_compression = c; }
    void setStatus(PackageJobStatus s)                { m_status      = s; }
    void setOutputPath(const std::string& p)          { m_outputPath  = p; }
    void setProgress(float pct)                       { m_progress    = pct; }
    void setIncludeDebugInfo(bool v)                  { m_debugInfo   = v; }

    [[nodiscard]] const std::string&     name()         const { return m_name;       }
    [[nodiscard]] PackageTargetPlatform  platform()     const { return m_platform;   }
    [[nodiscard]] PackageCompressionMode compression()  const { return m_compression;}
    [[nodiscard]] PackageJobStatus       status()       const { return m_status;     }
    [[nodiscard]] const std::string&     outputPath()   const { return m_outputPath; }
    [[nodiscard]] float                  progress()     const { return m_progress;   }
    [[nodiscard]] bool                   includeDebugInfo() const { return m_debugInfo; }

    [[nodiscard]] bool isDone()    const { return m_status == PackageJobStatus::Done;   }
    [[nodiscard]] bool isCooking() const { return m_status == PackageJobStatus::Cooking; }

private:
    std::string              m_name;
    PackageTargetPlatform    m_platform;
    PackageCompressionMode   m_compression = PackageCompressionMode::Default;
    PackageJobStatus         m_status      = PackageJobStatus::Queued;
    std::string              m_outputPath;
    float                    m_progress    = 0.0f;
    bool                     m_debugInfo   = false;
};

class AssetPackager {
public:
    static constexpr size_t MAX_JOBS = 32;

    [[nodiscard]] bool addJob(const PackageJob& job) {
        for (auto& j : m_jobs) if (j.name() == job.name()) return false;
        if (m_jobs.size() >= MAX_JOBS) return false;
        m_jobs.push_back(job);
        return true;
    }

    [[nodiscard]] bool removeJob(const std::string& name) {
        for (auto it = m_jobs.begin(); it != m_jobs.end(); ++it) {
            if (it->name() == name) { m_jobs.erase(it); return true; }
        }
        return false;
    }

    [[nodiscard]] PackageJob* findJob(const std::string& name) {
        for (auto& j : m_jobs) if (j.name() == name) return &j;
        return nullptr;
    }

    [[nodiscard]] size_t jobCount()     const { return m_jobs.size(); }
    [[nodiscard]] size_t doneCount()    const {
        size_t c = 0; for (auto& j : m_jobs) if (j.isDone())    ++c; return c;
    }
    [[nodiscard]] size_t cookingCount() const {
        size_t c = 0; for (auto& j : m_jobs) if (j.isCooking()) ++c; return c;
    }
    [[nodiscard]] size_t countByPlatform(PackageTargetPlatform p) const {
        size_t c = 0; for (auto& j : m_jobs) if (j.platform() == p) ++c; return c;
    }
    [[nodiscard]] size_t countByStatus(PackageJobStatus s) const {
        size_t c = 0; for (auto& j : m_jobs) if (j.status() == s) ++c; return c;
    }

private:
    std::vector<PackageJob> m_jobs;
};

} // namespace NF
