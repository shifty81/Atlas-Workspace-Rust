#pragma once
// NF::Editor — Build report viewer and analyzer
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

enum class BuildReportSeverity : uint8_t {
    Info, Warning, Error, Fatal
};

inline const char* buildReportSeverityName(BuildReportSeverity s) {
    switch (s) {
        case BuildReportSeverity::Info:    return "Info";
        case BuildReportSeverity::Warning: return "Warning";
        case BuildReportSeverity::Error:   return "Error";
        case BuildReportSeverity::Fatal:   return "Fatal";
    }
    return "Unknown";
}

enum class BuildReportCategory : uint8_t {
    Shader, Asset, Code, Config, Dependency, Size, Performance
};

inline const char* buildReportCategoryName(BuildReportCategory c) {
    switch (c) {
        case BuildReportCategory::Shader:      return "Shader";
        case BuildReportCategory::Asset:       return "Asset";
        case BuildReportCategory::Code:        return "Code";
        case BuildReportCategory::Config:      return "Config";
        case BuildReportCategory::Dependency:  return "Dependency";
        case BuildReportCategory::Size:        return "Size";
        case BuildReportCategory::Performance: return "Performance";
    }
    return "Unknown";
}

enum class BuildReportStatus : uint8_t {
    Empty, Building, Success, SuccessWithWarnings, Failed
};

inline const char* buildReportStatusName(BuildReportStatus s) {
    switch (s) {
        case BuildReportStatus::Empty:               return "Empty";
        case BuildReportStatus::Building:            return "Building";
        case BuildReportStatus::Success:             return "Success";
        case BuildReportStatus::SuccessWithWarnings: return "SuccessWithWarnings";
        case BuildReportStatus::Failed:              return "Failed";
    }
    return "Unknown";
}

class BuildReportEntry {
public:
    explicit BuildReportEntry(const std::string& message,
                               BuildReportSeverity severity,
                               BuildReportCategory category)
        : m_message(message), m_severity(severity), m_category(category) {}

    void setSource(const std::string& s)  { m_source = s; }
    void setLine(uint32_t l)              { m_line   = l; }
    void setResolved(bool v)              { m_resolved = v; }

    [[nodiscard]] const std::string&  message()    const { return m_message;  }
    [[nodiscard]] BuildReportSeverity severity()   const { return m_severity; }
    [[nodiscard]] BuildReportCategory category()   const { return m_category; }
    [[nodiscard]] const std::string&  source()     const { return m_source;   }
    [[nodiscard]] uint32_t            line()       const { return m_line;     }
    [[nodiscard]] bool                isResolved() const { return m_resolved; }

    [[nodiscard]] bool isError()   const { return m_severity == BuildReportSeverity::Error ||
                                                  m_severity == BuildReportSeverity::Fatal;  }
    [[nodiscard]] bool isWarning() const { return m_severity == BuildReportSeverity::Warning; }

private:
    std::string         m_message;
    BuildReportSeverity m_severity;
    BuildReportCategory m_category;
    std::string         m_source;
    uint32_t            m_line     = 0;
    bool                m_resolved = false;
};

class BuildReport {
public:
    static constexpr size_t MAX_ENTRIES = 4096;

    void setStatus(BuildReportStatus s) { m_status   = s; }
    void setBuildTime(float seconds)    { m_buildTime = seconds; }
    void setOutputSizeBytes(uint64_t n) { m_outputSize = n; }

    [[nodiscard]] bool addEntry(const BuildReportEntry& entry) {
        if (m_entries.size() >= MAX_ENTRIES) return false;
        m_entries.push_back(entry);
        return true;
    }

    void clearEntries() { m_entries.clear(); }

    [[nodiscard]] BuildReportStatus status()         const { return m_status;     }
    [[nodiscard]] float             buildTime()      const { return m_buildTime;  }
    [[nodiscard]] uint64_t          outputSizeBytes()const { return m_outputSize; }
    [[nodiscard]] size_t            entryCount()     const { return m_entries.size(); }

    [[nodiscard]] bool   isSuccess() const {
        return m_status == BuildReportStatus::Success ||
               m_status == BuildReportStatus::SuccessWithWarnings;
    }

    [[nodiscard]] size_t errorCount() const {
        size_t c = 0; for (auto& e : m_entries) if (e.isError())   ++c; return c;
    }
    [[nodiscard]] size_t warningCount() const {
        size_t c = 0; for (auto& e : m_entries) if (e.isWarning()) ++c; return c;
    }
    [[nodiscard]] size_t countBySeverity(BuildReportSeverity s) const {
        size_t c = 0; for (auto& e : m_entries) if (e.severity() == s) ++c; return c;
    }
    [[nodiscard]] size_t countByCategory(BuildReportCategory cat) const {
        size_t c = 0; for (auto& e : m_entries) if (e.category() == cat) ++c; return c;
    }
    [[nodiscard]] size_t resolvedCount() const {
        size_t c = 0; for (auto& e : m_entries) if (e.isResolved()) ++c; return c;
    }

private:
    std::vector<BuildReportEntry> m_entries;
    BuildReportStatus             m_status     = BuildReportStatus::Empty;
    float                         m_buildTime  = 0.0f;
    uint64_t                      m_outputSize = 0;
};

} // namespace NF
