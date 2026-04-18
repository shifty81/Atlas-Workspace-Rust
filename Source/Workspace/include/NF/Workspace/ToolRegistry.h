#pragma once
// NF::Editor — ToolRegistry: central registry for workspace-hosted tools.
//
// The ToolRegistry owns all registered IHostedTool instances.
// WorkspaceShell delegates tool lifecycle (init, activate, suspend, shutdown)
// through this registry. Tools register at boot and are enumerable by
// category, tool-id, and primary/non-primary status.
//
// See Docs/Canon/05_EDITOR_STRATEGY.md for the locked ~10-tool roster.
// See ProjectSystemsTool.h for the HostToolId namespace.

#include "NF/Workspace/IHostedTool.h"
#include <algorithm>
#include <memory>
#include <string>
#include <vector>

namespace NF {

class ToolRegistry {
public:
    static constexpr size_t MAX_TOOLS = 32;

    // ── Registration ──────────────────────────────────────────────

    bool registerTool(std::unique_ptr<IHostedTool> tool) {
        if (!tool) return false;
        if (!tool->descriptor().isValid()) return false;
        if (m_tools.size() >= MAX_TOOLS) return false;
        // Reject duplicate tool-id
        for (const auto& t : m_tools)
            if (t->toolId() == tool->toolId()) return false;
        m_tools.push_back(std::move(tool));
        return true;
    }

    bool unregisterTool(const std::string& toolId) {
        auto it = std::find_if(m_tools.begin(), m_tools.end(),
            [&](const auto& t) { return t->toolId() == toolId; });
        if (it == m_tools.end()) return false;
        m_tools.erase(it);
        if (m_activeToolId == toolId) m_activeToolId.clear();
        return true;
    }

    // ── Lookup ────────────────────────────────────────────────────

    [[nodiscard]] IHostedTool* find(const std::string& toolId) const {
        for (const auto& t : m_tools)
            if (t->toolId() == toolId) return t.get();
        return nullptr;
    }

    [[nodiscard]] bool isRegistered(const std::string& toolId) const {
        return find(toolId) != nullptr;
    }

    [[nodiscard]] size_t count() const { return m_tools.size(); }
    [[nodiscard]] bool   empty() const { return m_tools.empty(); }

    // ── Filtered queries ──────────────────────────────────────────

    [[nodiscard]] std::vector<IHostedTool*> byCategory(HostedToolCategory cat) const {
        std::vector<IHostedTool*> out;
        for (const auto& t : m_tools)
            if (t->descriptor().category == cat) out.push_back(t.get());
        return out;
    }

    [[nodiscard]] std::vector<IHostedTool*> primaryTools() const {
        std::vector<IHostedTool*> out;
        for (const auto& t : m_tools)
            if (t->descriptor().isPrimary) out.push_back(t.get());
        return out;
    }

    [[nodiscard]] std::vector<const HostedToolDescriptor*> allDescriptors() const {
        std::vector<const HostedToolDescriptor*> out;
        for (const auto& t : m_tools) out.push_back(&t->descriptor());
        return out;
    }

    // ── Active tool ───────────────────────────────────────────────

    bool activateTool(const std::string& toolId) {
        auto* tool = find(toolId);
        if (!tool) return false;
        if (tool->state() == HostedToolState::Unloaded) return false;

        // Suspend the currently active tool (if different)
        if (!m_activeToolId.empty() && m_activeToolId != toolId) {
            if (auto* prev = find(m_activeToolId))
                if (prev->state() == HostedToolState::Active) prev->suspend();
        }
        tool->activate();
        m_activeToolId = toolId;
        return true;
    }

    [[nodiscard]] const std::string& activeToolId() const { return m_activeToolId; }

    /// Suspend and deselect the currently active tool.
    /// After this call, activeTool() returns nullptr and activeToolId() is empty.
    /// Returns false if no tool was active.
    bool deactivateTool() {
        if (m_activeToolId.empty()) return false;
        if (auto* t = find(m_activeToolId)) {
            if (t->state() == HostedToolState::Active) t->suspend();
        }
        m_activeToolId.clear();
        return true;
    }

    [[nodiscard]] IHostedTool* activeTool() const {
        if (m_activeToolId.empty()) return nullptr;
        return find(m_activeToolId);
    }

    // ── Lifecycle helpers ─────────────────────────────────────────

    bool initializeAll() {
        bool ok = true;
        for (auto& t : m_tools) {
            if (t->state() == HostedToolState::Unloaded) {
                if (!t->initialize()) ok = false;
            }
        }
        return ok;
    }

    void shutdownAll() {
        m_activeToolId.clear();
        for (auto& t : m_tools) t->shutdown();
    }

    void updateActive(float dt) {
        if (auto* t = activeTool()) {
            if (t->state() == HostedToolState::Active) t->update(dt);
        }
    }

    // ── Project events ────────────────────────────────────────────

    void notifyProjectLoaded(const std::string& projectId) {
        for (auto& t : m_tools) t->onProjectLoaded(projectId);
    }

    void notifyProjectUnloaded() {
        for (auto& t : m_tools) t->onProjectUnloaded();
    }

private:
    std::vector<std::unique_ptr<IHostedTool>> m_tools;
    std::string m_activeToolId;
};

} // namespace NF
