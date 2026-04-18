// NF::Editor — BuildTool implementation.
//
// Seventh real NF::IHostedTool from Phase 3 consolidation.
// Manages build, packaging, and deploy workflows within AtlasWorkspace.

#include "NF/Editor/BuildTool.h"
#include "NF/Workspace/ToolViewRenderContext.h"
#include "NF/Workspace/WorkspacePanelHost.h"
#include "NF/Workspace/WorkspaceShell.h"
#include <chrono>
#include <cstdio>
#include <cstring>
#include <algorithm>

#if defined(_WIN32)
#  ifndef WIN32_LEAN_AND_MEAN
#    define WIN32_LEAN_AND_MEAN
#  endif
#  include <windows.h>
#else
#  include <cstdlib>
#endif

namespace NF {

BuildTool::BuildTool() {
    buildDescriptor();
}

void BuildTool::buildDescriptor() {
    m_descriptor.toolId      = kToolId;
    m_descriptor.displayName = "Build Tool";
    m_descriptor.category    = HostedToolCategory::BuildPackaging;
    m_descriptor.isPrimary   = true;
    m_descriptor.acceptsProjectExtensions = true;

    m_descriptor.supportedPanels = {
        "panel.build_log",
        "panel.inspector",
        "panel.console",
    };

    m_descriptor.commands = {
        "build.run",
        "build.clean",
        "build.cancel",
        "build.open_output",
        "build.set_target",
        "build.run_tests",
    };
}

bool BuildTool::initialize() {
    if (m_state != HostedToolState::Unloaded) return false;
    m_buildMode    = BuildMode::Incremental;
    m_stats        = {};
    m_activeTarget = {};
    clearLog();
    appendLogLine("[INFO]  Build system ready");
    m_state        = HostedToolState::Ready;
    return true;
}

void BuildTool::shutdown() {
    m_state           = HostedToolState::Unloaded;
    m_buildMode       = BuildMode::Incremental;
    m_stats           = {};
    m_activeTarget    = {};
    m_activeProjectId = {};
}

void BuildTool::activate() {
    if (m_state == HostedToolState::Ready || m_state == HostedToolState::Suspended) {
        m_state = HostedToolState::Active;
    }
}

void BuildTool::suspend() {
    if (m_state == HostedToolState::Active) {
        m_state = HostedToolState::Suspended;
    }
}

void BuildTool::update(float /*dt*/) {
    // Poll the active build process pipe for new output lines.
    if (m_buildProcess) {
        char buf[512];
        if (std::fgets(buf, sizeof(buf), m_buildProcess)) {
            // Strip trailing newline
            std::string line(buf);
            if (!line.empty() && (line.back() == '\n' || line.back() == '\r'))
                line.pop_back();
            if (!line.empty() && (line.back() == '\n' || line.back() == '\r'))
                line.pop_back();
            // Count errors and warnings
            if (line.find("error:") != std::string::npos ||
                line.find("Error:") != std::string::npos ||
                line.find("[ERROR]") != std::string::npos)
                ++m_stats.errorCount;
            else if (line.find("warning:") != std::string::npos ||
                     line.find("Warning:") != std::string::npos ||
                     line.find("[WARN]") != std::string::npos)
                ++m_stats.warningCount;
            appendLogLine(line);
        } else {
            // Process finished (EOF or error)
#if defined(_WIN32)
            int exitCode = _pclose(m_buildProcess);
#else
            int exitCode = pclose(m_buildProcess);
#endif
            m_buildProcess = nullptr;
            setBuilding(false);
            m_viewBuildRequested = false;

            auto now = std::chrono::steady_clock::now();
            float elapsedMs = std::chrono::duration<float, std::milli>(
                now - m_buildStartTime).count();
            setLastBuildMs(elapsedMs);

            if (exitCode == 0 && m_stats.errorCount == 0) {
                appendLogLine("[OK]  Build succeeded in " +
                    std::to_string(static_cast<int>(elapsedMs)) + " ms");
            } else {
                appendLogLine("[ERROR]  Build failed (exit " +
                    std::to_string(exitCode) + ")");
            }
        }
        return;
    }

    // Kick off a new build if requested (via "Build Now" button or build.start command).
    if (m_viewBuildRequested && !m_stats.isBuilding) {
        clearStats();
        setBuilding(true);
        clearLog();

        // Build the cmake invocation.
        // Prefer the project root if available; fall back to the current directory.
        std::string buildDir = m_buildRootPath.empty() ? "." : m_buildRootPath;
        // Remove trailing separator
        while (!buildDir.empty() && (buildDir.back() == '/' || buildDir.back() == '\\'))
            buildDir.pop_back();

        std::string cmd = "cmake --build \"" + buildDir + "\" 2>&1";

        appendLogLine("[INFO]  Starting build: " + cmd);
        m_buildStartTime = std::chrono::steady_clock::now();

#if defined(_WIN32)
        m_buildProcess = _popen(cmd.c_str(), "r");
#else
        m_buildProcess = popen(cmd.c_str(), "r");
#endif

        if (!m_buildProcess) {
            appendLogLine("[ERROR]  Failed to launch build process");
            setBuilding(false);
            m_viewBuildRequested = false;
        }
    }
}

void BuildTool::onProjectLoaded(const std::string& projectId) {
    m_activeProjectId = projectId;
    m_stats           = {};
    m_activeTarget    = {};
    // The projectId is used as a hint for the build directory;
    // the caller (WorkspaceShell) may also set m_buildRootPath directly.
    // Use the projectId as the build root fallback when it looks like a path.
    if (!projectId.empty() && (projectId.find('/') != std::string::npos ||
                                projectId.find('\\') != std::string::npos)) {
        m_buildRootPath = projectId + "/build";
    }
    clearLog();
    appendLogLine("[INFO]  Project loaded: " + projectId);
    appendLogLine("[INFO]  Mode: " + std::string(buildModeName(m_buildMode)));
}

void BuildTool::onProjectUnloaded() {
    m_activeProjectId = {};
    m_stats           = {};
    m_activeTarget    = {};
}

void BuildTool::setBuildMode(BuildMode mode) {
    m_buildMode = mode;
}

void BuildTool::setBuilding(bool building) {
    m_stats.isBuilding = building;
}

void BuildTool::setLastBuildMs(float ms) {
    m_stats.lastBuildMs = ms;
}

void BuildTool::setWarningCount(uint32_t count) {
    m_stats.warningCount = count;
}

void BuildTool::setErrorCount(uint32_t count) {
    m_stats.errorCount = count;
}

void BuildTool::clearStats() {
    m_stats = {};
}

void BuildTool::appendLogLine(const std::string& line) {
    m_logLines.push_back(line);
    // Keep the log buffer bounded
    static constexpr size_t kMaxLogLines = 500;
    if (m_logLines.size() > kMaxLogLines)
        m_logLines.erase(m_logLines.begin());
}

void BuildTool::clearLog() {
    m_logLines.clear();
}

void BuildTool::setActiveTarget(const std::string& target) {
    m_activeTarget = target;
}

void BuildTool::renderToolView(const ToolViewRenderContext& ctx) const {
    // Three-column layout: Build Config (25%) | Build Log (55%) | Metrics (20%)
    const float configW  = ctx.w * 0.25f;
    const float logW     = ctx.w * 0.55f;
    const float metricsW = ctx.w - configW - logW;

    // Build mode options
    static constexpr struct { const char* label; BuildMode mode; } kModes[] = {
        {"Incremental", BuildMode::Incremental},
        {"Full",        BuildMode::Full},
        {"ShaderOnly",  BuildMode::ShaderOnly},
    };

    // Platform targets (DeploymentTargetEditorV1)
    static constexpr struct { const char* name; uint32_t color; } kPlatforms[] = {
        {"Win64",   0x4488CCFF},
        {"Mac",     0x88AABBFF},
        {"Linux",   0xCCAA44FF},
        {"iOS",     0x44AACCFF},
        {"Android", 0x4EC94EFF},
        {"Console", 0xAA6633FF},
    };
    static constexpr int kPlatCount = 6;

    // Pipeline stages (BuildPipelineEditorV1: Compile/Link/Package/Cook/Deploy)
    static constexpr struct {
        const char* name;
        bool        enabled;
        uint32_t    color;
    } kStages[] = {
        {"Compile",  true,  0x4488CCFF},
        {"Link",     true,  0x44CC88FF},
        {"Cook",     true,  0xCCAA44FF},
        {"Package",  true,  0x884488FF},
        {"Sign",     false, 0x886644FF},
        {"Deploy",   false, 0xCC4466FF},
    };
    static constexpr int kStageCount = 6;

    // ── Build Config panel ────────────────────────────────────────
    ctx.drawPanel(ctx.x, ctx.y, configW, ctx.h, "Build Config");
    {
        ctx.drawStatRow(ctx.x + 8.f, ctx.y + 28.f, "Mode:", buildModeName(m_buildMode));
        if (!m_activeTarget.empty()) {
            std::string tgt = m_activeTarget;
            if (tgt.size() > 20) tgt = tgt.substr(0, 17) + "...";
            ctx.drawStatRow(ctx.x + 8.f, ctx.y + 46.f, "Target:", tgt.c_str());
        }

        // Build mode selector buttons
        float mbY = ctx.y + 68.f;
        ctx.ui.drawText(ctx.x + 8.f, mbY - 10.f, "Mode:", ctx.kTextMuted);
        for (const auto& m : kModes) {
            if (mbY + 18.f > ctx.y + ctx.h - 90.f) break;
            bool active = (m_buildMode == m.mode);
            uint32_t bg = active ? ctx.kAccentBlue : ctx.kButtonBg;
            if (ctx.drawButton(ctx.x + 8.f, mbY, configW - 16.f, 16.f, m.label, bg))
                m_buildMode = m.mode;
            mbY += 20.f;
        }

        // Platform selector (DeploymentTargetEditorV1)
        mbY += 6.f;
        ctx.ui.drawText(ctx.x + 8.f, mbY, "Platform:", ctx.kTextMuted);
        mbY += 14.f;
        float pbx = ctx.x + 8.f;
        for (int i = 0; i < kPlatCount; ++i) {
            if (pbx + 40.f > ctx.x + configW - 4.f) { pbx = ctx.x + 8.f; mbY += 20.f; }
            if (mbY + 18.f > ctx.y + ctx.h - 36.f) break;
            ctx.ui.drawRect({pbx, mbY, 36.f, 16.f}, kPlatforms[i].color);
            ctx.ui.drawRectOutline({pbx, mbY, 36.f, 16.f}, ctx.kBorder, 1.f);
            ctx.ui.drawText(pbx + 4.f, mbY + 2.f, kPlatforms[i].name, ctx.kTextPrimary);
            pbx += 40.f;
        }
    }

    // Build button at the bottom of the config panel
    bool isBuilding = m_stats.isBuilding || m_viewBuildRequested;
    if (!isBuilding) {
        if (ctx.drawButton(ctx.x + 8.f, ctx.y + ctx.h - 30.f,
                           configW - 16.f, 22.f, "Build Now",
                           0x005A9EFF, ctx.kAccentBlue)) {
            m_viewBuildRequested = true;
            if (ctx.shell)
                (void)ctx.shell->commandBus().execute("build.start");
        }
    } else {
        ctx.drawStatusPill(ctx.x + 8.f, ctx.y + ctx.h - 28.f, "Building...", ctx.kAccentBlue);
    }

    // ── Build Log panel ───────────────────────────────────────────
    ctx.drawPanel(ctx.x + configW, ctx.y, logW, ctx.h, "Build Log");
    {
        float ly = ctx.y + 10.f;

        // Pipeline stage list (BuildPipelineEditorV1)
        ctx.ui.drawText(ctx.x + configW + 8.f, ly, "Pipeline stages:", ctx.kTextMuted);
        ly += 14.f;
        float stX = ctx.x + configW + 8.f;
        for (int i = 0; i < kStageCount; ++i) {
            float stW = static_cast<float>(std::strlen(kStages[i].name)) * 6.f + 14.f;
            if (stX + stW > ctx.x + configW + logW - 4.f) { stX = ctx.x + configW + 8.f; ly += 18.f; }
            uint32_t stageBg = kStages[i].enabled ? kStages[i].color : 0x333333FF;
            ctx.ui.drawRect({stX, ly, stW, 14.f}, stageBg);
            ctx.ui.drawRectOutline({stX, ly, stW, 14.f}, ctx.kBorder, 1.f);
            ctx.ui.drawText(stX + 4.f, ly + 1.f, kStages[i].name,
                            kStages[i].enabled ? ctx.kTextPrimary : ctx.kTextMuted);
            // Arrow between stages
            if (i < kStageCount - 1 && kStages[i].enabled && kStages[i + 1].enabled)
                ctx.ui.drawText(stX + stW + 2.f, ly + 1.f, ">", ctx.kTextMuted);
            stX += stW + 18.f;
        }
        ly += 22.f;
        ctx.ui.drawRect({ctx.x + configW + 4.f, ly, logW - 8.f, 1.f}, ctx.kBorder);
        ly += 8.f;

        // Build status & log lines
        if (m_stats.errorCount > 0) {
            char errBuf[32];
            std::snprintf(errBuf, sizeof(errBuf), "%u errors", m_stats.errorCount);
            ctx.drawStatusPill(ctx.x + configW + 8.f, ly, errBuf, ctx.kRed);
            ly += 24.f;
        }
        if (m_stats.warningCount > 0) {
            char warnBuf[32];
            std::snprintf(warnBuf, sizeof(warnBuf), "%u warnings", m_stats.warningCount);
            ctx.drawStatusPill(ctx.x + configW + 8.f, ly, warnBuf, 0xE09020FFu);
            ly += 24.f;
        }
        if (m_stats.errorCount == 0 && m_stats.warningCount == 0 && !isBuilding) {
            ctx.ui.drawText(ctx.x + configW + 8.f, ly, "Build clean", ctx.kGreen);
            ly += 18.f;
        }

        // Live build log — streamed by runBuildProcess() / build.start handler
        if (m_logLines.empty()) {
            ctx.ui.drawText(ctx.x + configW + 8.f, ly, "[INFO]  Awaiting start...", ctx.kTextSecond);
        } else {
            // Render the most recent lines that fit in the panel
            size_t firstVisible = 0;
            {
                float avail = ctx.y + ctx.h - 4.f - ly;
                size_t maxLines = static_cast<size_t>(avail / 16.f);
                if (m_logLines.size() > maxLines)
                    firstVisible = m_logLines.size() - maxLines;
            }
            for (size_t i = firstVisible; i < m_logLines.size(); ++i) {
                if (ly + 16.f > ctx.y + ctx.h - 4.f) break;
                const auto& line = m_logLines[i];
                // Colour-code the line by prefix
                uint32_t col = ctx.kTextSecond;
                if (line.find("[ERROR]") != std::string::npos ||
                    line.find("error:") != std::string::npos)
                    col = ctx.kRed;
                else if (line.find("[WARN]") != std::string::npos ||
                         line.find("warning:") != std::string::npos)
                    col = 0xE09020FFu;
                else if (line.find("[OK]") != std::string::npos ||
                         line.find("Succeeded") != std::string::npos)
                    col = ctx.kGreen;
                ctx.ui.drawText(ctx.x + configW + 8.f, ly, line.c_str(), col);
                ly += 16.f;
            }
        }
    }

    // ── Metrics panel ─────────────────────────────────────────────
    ctx.drawPanel(ctx.x + configW + logW, ctx.y, metricsW, ctx.h, "Metrics");
    const float mx = ctx.x + configW + logW;
    if (m_stats.lastBuildMs > 0.f) {
        char timeBuf[32];
        std::snprintf(timeBuf, sizeof(timeBuf), "%.0f ms", m_stats.lastBuildMs);
        ctx.drawStatRow(mx + 8.f, ctx.y + 30.f, "Last build:", timeBuf);
    } else {
        ctx.ui.drawText(mx + 8.f, ctx.y + 30.f, "No build yet", ctx.kTextMuted);
    }
    ctx.drawStatRow(mx + 8.f, ctx.y + 50.f, "Mode:", buildModeName(m_buildMode));
    char errStr[16]; std::snprintf(errStr, sizeof(errStr), "%u", m_stats.errorCount);
    ctx.drawStatRow(mx + 8.f, ctx.y + 68.f, "Errors:", errStr);
    char warnStr[16]; std::snprintf(warnStr, sizeof(warnStr), "%u", m_stats.warningCount);
    ctx.drawStatRow(mx + 8.f, ctx.y + 86.f, "Warnings:", warnStr);
}


void BuildTool::syncPanels(WorkspacePanelHost& host) const {
    (void)host;
}

} // namespace NF
