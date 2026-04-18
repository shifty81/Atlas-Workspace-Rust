#pragma once
// NF::Editor — Tool window manager
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

enum class ToolLaunchMode : uint8_t { Embedded, External, Docked };

struct ToolDescriptor {
    std::string name;
    std::string executable;
    std::string icon;
    ToolLaunchMode mode = ToolLaunchMode::Docked;
    bool isRunning = false;
};

class ToolWindowManager {
public:
    void registerTool(ToolDescriptor desc) {
        m_tools.push_back(std::move(desc));
        NF_LOG_INFO("Editor", "ToolWindowManager: registered tool '" + m_tools.back().name + "'");
    }

    bool launchTool(const std::string& name) {
        for (auto& t : m_tools) {
            if (t.name == name) {
                t.isRunning = true;
                NF_LOG_INFO("Editor", "ToolWindowManager: launched '" + name + "'");
                return true;
            }
        }
        return false;
    }

    void stopTool(const std::string& name) {
        for (auto& t : m_tools) {
            if (t.name == name) { t.isRunning = false; return; }
        }
    }

    [[nodiscard]] const ToolDescriptor* findTool(const std::string& name) const {
        for (auto& t : m_tools) if (t.name == name) return &t;
        return nullptr;
    }

    [[nodiscard]] const std::vector<ToolDescriptor>& tools() const { return m_tools; }
    [[nodiscard]] size_t toolCount() const { return m_tools.size(); }

    [[nodiscard]] size_t runningCount() const {
        size_t c = 0;
        for (auto& t : m_tools) if (t.isRunning) ++c;
        return c;
    }

private:
    std::vector<ToolDescriptor> m_tools;
};

// ── Pipeline Monitor Panel ───────────────────────────────────────

struct PipelineEventEntry {
    std::string type;
    std::string source;
    std::string details;
    float timestamp = 0.f;
};

// DEPRECATED: Use NF::UI::AtlasUI::PipelineMonitorPanel instead.

} // namespace NF
