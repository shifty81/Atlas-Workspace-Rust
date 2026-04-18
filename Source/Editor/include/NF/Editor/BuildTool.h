#pragma once
// NF::Editor — BuildTool: primary build, packaging, and deploy tool.
//
// Implements NF::IHostedTool for the Build Tool, one of the ~10
// primary tools in the canonical workspace roster.
//
// The Build Tool hosts:
//   Shared panels: build_log, inspector, console
//   Commands:      build.run, build.clean, build.cancel,
//                  build.open_output, build.set_target, build.run_tests
//
// See Docs/Canon/05_EDITOR_STRATEGY.md for the locked tool roster.
// See Docs/Roadmap/04_EDITOR_CONSOLIDATION.md for Phase 3 status.

#include "NF/Workspace/IHostedTool.h"
#include "NF/Workspace/ToolViewRenderContext.h"
#include <chrono>
#include <cstdio>
#include <string>
#include <vector>

namespace NF {

// ── Build mode ────────────────────────────────────────────────────

enum class BuildMode : uint8_t {
    Full,        // full rebuild
    Incremental, // incremental / delta build
    ShaderOnly,  // shader compilation only
};

inline const char* buildModeName(BuildMode m) {
    switch (m) {
        case BuildMode::Full:        return "Full";
        case BuildMode::Incremental: return "Incremental";
        case BuildMode::ShaderOnly:  return "ShaderOnly";
    }
    return "Unknown";
}

// ── Build Tool statistics ─────────────────────────────────────────

struct BuildToolStats {
    uint32_t warningCount = 0;
    uint32_t errorCount   = 0;
    float    lastBuildMs  = 0.0f; // duration of last completed build
    bool     isBuilding   = false;
};

// ── BuildTool ─────────────────────────────────────────────────────

class BuildTool final : public IHostedTool {
public:
    static constexpr const char* kToolId = "workspace.build_tool";

    BuildTool();
    ~BuildTool() override = default;

    // ── IHostedTool identity ──────────────────────────────────────
    [[nodiscard]] const HostedToolDescriptor& descriptor() const override { return m_descriptor; }
    [[nodiscard]] const std::string& toolId()              const override { return m_descriptor.toolId; }

    // ── IHostedTool lifecycle ─────────────────────────────────────
    bool initialize() override;
    void shutdown()   override;
    void activate()   override;
    void suspend()    override;
    void update(float dt) override;

    [[nodiscard]] HostedToolState state() const override { return m_state; }

    // ── Project adapter hooks ─────────────────────────────────────
    void onProjectLoaded(const std::string& projectId) override;
    void onProjectUnloaded() override;

    // ── Build Tool interface ──────────────────────────────────────

    [[nodiscard]] BuildMode            buildMode()   const { return m_buildMode; }
    void                               setBuildMode(BuildMode mode);

    [[nodiscard]] const BuildToolStats& stats()      const { return m_stats; }
    [[nodiscard]] bool                  isBuilding() const { return m_stats.isBuilding; }
    [[nodiscard]] uint32_t              errorCount() const { return m_stats.errorCount; }
    [[nodiscard]] uint32_t              warningCount() const { return m_stats.warningCount; }

    void setBuilding(bool building);
    void setLastBuildMs(float ms);
    void setWarningCount(uint32_t count);
    void setErrorCount(uint32_t count);
    void clearStats();

    [[nodiscard]] const std::string& activeTarget() const { return m_activeTarget; }
    void                             setActiveTarget(const std::string& target);

    // ── Build log access ──────────────────────────────────────────
    [[nodiscard]] const std::vector<std::string>& logLines() const { return m_logLines; }
    void appendLogLine(const std::string& line);
    void clearLog();

    // ── Build trigger ─────────────────────────────────────────────
    /// Request a build to start on the next update() tick.
    /// Can be called from the command bus handler without render context.
    void requestBuild() { m_viewBuildRequested = true; }

    // ── Render contract ───────────────────────────────────────────
    // Renders: Build Config | Build Log | Metrics — three-column layout.
    void renderToolView(const ToolViewRenderContext& ctx) const override;
    void syncPanels(WorkspacePanelHost& host) const override;

private:
    HostedToolDescriptor m_descriptor;
    HostedToolState      m_state      = HostedToolState::Unloaded;
    mutable BuildMode    m_buildMode  = BuildMode::Incremental;
    BuildToolStats       m_stats;
    std::string          m_activeTarget;
    std::string          m_activeProjectId;
    std::string          m_buildRootPath;          ///< cmake build directory (set on project load)
    std::vector<std::string> m_logLines;           ///< live build log (streamed from build process)
    std::FILE*           m_buildProcess = nullptr; ///< active popen pipe
    std::chrono::steady_clock::time_point m_buildStartTime;

    // ── Mutable per-view UI state (safe from const renderToolView) ─
    mutable bool m_viewBuildRequested = false;

    void buildDescriptor();
};

} // namespace NF
