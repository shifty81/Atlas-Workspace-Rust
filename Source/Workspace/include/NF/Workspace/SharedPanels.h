#pragma once
// NF::Workspace — SharedPanels: ISharedPanel implementations for workspace-owned panels.
//
// These panels are registered as factories in WorkspaceShell::registerDefaultPanels()
// and created on demand via PanelRegistry::getOrCreatePanel().
//
// Each panel wraps the relevant existing editor panel implementation, binding it
// to the ISharedPanel lifecycle. The workspace shell owns all instances.

#include "NF/Workspace/ISharedPanel.h"
#include "NF/Workspace/NotificationCenterEditor.h"
#include <string>

namespace NF {

// ── ContentBrowserSharedPanel ─────────────────────────────────────
// Wraps content browser as a workspace-owned shared panel.

class ContentBrowserSharedPanel : public ISharedPanel {
public:
    [[nodiscard]] const std::string& panelId()      const override { return s_panelId; }
    [[nodiscard]] const std::string& displayName()  const override { return s_displayName; }

    bool initialize() override { m_initialized = true; return true; }
    void shutdown()   override { m_initialized = false; }
    void update(float dt) override { (void)dt; }

    void onToolActivated(const std::string& toolId) override {
        m_activeToolId = toolId;
    }

    // ── Content browser state ─────────────────────────────────────

    [[nodiscard]] const std::string& currentPath()  const { return m_currentPath; }
    void setCurrentPath(const std::string& path) { m_currentPath = path; }
    void navigateUp() {
        auto pos = m_currentPath.find_last_of('/');
        if (pos != std::string::npos && pos > 0)
            m_currentPath = m_currentPath.substr(0, pos);
    }

    [[nodiscard]] bool isInitialized() const { return m_initialized; }

    static constexpr const char* kPanelId = "content_browser";

private:
    inline static const std::string s_panelId{"content_browser"};
    inline static const std::string s_displayName{"Content Browser"};
    std::string m_currentPath = "/";
    std::string m_activeToolId;
    bool m_initialized = false;
};

// ── ComponentInspectorSharedPanel ─────────────────────────────────
// Wraps component inspector as a workspace-owned shared panel.

class ComponentInspectorSharedPanel : public ISharedPanel {
public:
    [[nodiscard]] const std::string& panelId()      const override { return s_panelId; }
    [[nodiscard]] const std::string& displayName()  const override { return s_displayName; }

    bool initialize() override { m_initialized = true; return true; }
    void shutdown()   override { m_initialized = false; m_selectedEntityId = 0; }
    void update(float dt) override { (void)dt; }

    void onSelectionChanged() override { m_dirtyFlag = true; }

    // ── Inspector state ───────────────────────────────────────────

    void selectEntity(uint32_t entityId) { m_selectedEntityId = entityId; m_dirtyFlag = true; }
    [[nodiscard]] uint32_t selectedEntityId() const { return m_selectedEntityId; }
    [[nodiscard]] bool isDirty() const { return m_dirtyFlag; }
    void clearDirty() { m_dirtyFlag = false; }
    [[nodiscard]] bool isInitialized() const { return m_initialized; }

    static constexpr const char* kPanelId = "component_inspector";

private:
    inline static const std::string s_panelId{"component_inspector"};
    inline static const std::string s_displayName{"Component Inspector"};
    uint32_t m_selectedEntityId = 0;
    bool m_dirtyFlag = false;
    bool m_initialized = false;
};

// ── DiagnosticsSharedPanel ────────────────────────────────────────
// Wraps diagnostics panel as a workspace-owned shared panel.

class DiagnosticsSharedPanel : public ISharedPanel {
public:
    [[nodiscard]] const std::string& panelId()      const override { return s_panelId; }
    [[nodiscard]] const std::string& displayName()  const override { return s_displayName; }

    bool initialize() override { m_initialized = true; return true; }
    void shutdown()   override { m_initialized = false; m_entries.clear(); }
    void update(float dt) override { (void)dt; }

    // ── Diagnostics state ─────────────────────────────────────────

    enum class DiagLevel : uint8_t { Info, Warning, Error };

    struct DiagEntry {
        std::string message;
        DiagLevel   level = DiagLevel::Info;
        std::string source;
    };

    void addEntry(DiagEntry entry) { m_entries.push_back(std::move(entry)); }
    void clearEntries() { m_entries.clear(); }
    [[nodiscard]] size_t entryCount() const { return m_entries.size(); }
    [[nodiscard]] const std::vector<DiagEntry>& entries() const { return m_entries; }

    [[nodiscard]] size_t countByLevel(DiagLevel level) const {
        size_t n = 0;
        for (const auto& e : m_entries) if (e.level == level) ++n;
        return n;
    }

    [[nodiscard]] bool isInitialized() const { return m_initialized; }

    static constexpr const char* kPanelId = "diagnostics";

private:
    inline static const std::string s_panelId{"diagnostics"};
    inline static const std::string s_displayName{"Diagnostics"};
    std::vector<DiagEntry> m_entries;
    bool m_initialized = false;
};

// ── MemoryProfilerSharedPanel ─────────────────────────────────────
// Wraps memory profiler as a workspace-owned shared panel.

class MemoryProfilerSharedPanel : public ISharedPanel {
public:
    [[nodiscard]] const std::string& panelId()      const override { return s_panelId; }
    [[nodiscard]] const std::string& displayName()  const override { return s_displayName; }

    bool initialize() override { m_initialized = true; return true; }
    void shutdown()   override { m_initialized = false; }
    void update(float dt) override { (void)dt; m_frameCount++; }

    // ── Profiler state ────────────────────────────────────────────

    void recordAllocation(size_t bytes) { m_totalAllocated += bytes; m_allocationCount++; }
    void recordDeallocation(size_t bytes) {
        m_totalAllocated = (bytes <= m_totalAllocated) ? m_totalAllocated - bytes : 0;
        m_deallocationCount++;
    }

    [[nodiscard]] size_t totalAllocated()    const { return m_totalAllocated; }
    [[nodiscard]] size_t allocationCount()   const { return m_allocationCount; }
    [[nodiscard]] size_t deallocationCount() const { return m_deallocationCount; }
    [[nodiscard]] size_t frameCount()        const { return m_frameCount; }
    [[nodiscard]] bool   isInitialized()     const { return m_initialized; }

    static constexpr const char* kPanelId = "memory_profiler";

private:
    inline static const std::string s_panelId{"memory_profiler"};
    inline static const std::string s_displayName{"Memory Profiler"};
    size_t m_totalAllocated    = 0;
    size_t m_allocationCount   = 0;
    size_t m_deallocationCount = 0;
    size_t m_frameCount        = 0;
    bool   m_initialized       = false;
};

// ── PipelineMonitorSharedPanel ────────────────────────────────────
// Wraps pipeline monitor as a workspace-owned shared panel.

class PipelineMonitorSharedPanel : public ISharedPanel {
public:
    [[nodiscard]] const std::string& panelId()      const override { return s_panelId; }
    [[nodiscard]] const std::string& displayName()  const override { return s_displayName; }

    bool initialize() override { m_initialized = true; return true; }
    void shutdown()   override { m_initialized = false; m_stages.clear(); }
    void update(float dt) override { (void)dt; }

    // ── Pipeline state ────────────────────────────────────────────

    enum class StageStatus : uint8_t { Idle, Running, Completed, Failed };

    struct PipelineStage {
        std::string name;
        StageStatus status = StageStatus::Idle;
        float       progress = 0.f;  // 0..1
    };

    void addStage(PipelineStage stage) { m_stages.push_back(std::move(stage)); }
    void clearStages() { m_stages.clear(); }
    [[nodiscard]] size_t stageCount() const { return m_stages.size(); }
    [[nodiscard]] const std::vector<PipelineStage>& stages() const { return m_stages; }

    bool setStageStatus(const std::string& name, StageStatus status) {
        for (auto& s : m_stages) {
            if (s.name == name) { s.status = status; return true; }
        }
        return false;
    }

    bool setStageProgress(const std::string& name, float progress) {
        for (auto& s : m_stages) {
            if (s.name == name) { s.progress = progress; return true; }
        }
        return false;
    }

    [[nodiscard]] bool isInitialized() const { return m_initialized; }

    static constexpr const char* kPanelId = "pipeline_monitor";

private:
    inline static const std::string s_panelId{"pipeline_monitor"};
    inline static const std::string s_displayName{"Pipeline Monitor"};
    std::vector<PipelineStage> m_stages;
    bool m_initialized = false;
};

// ── NotificationCenterSharedPanel ─────────────────────────────────
// Wraps notification center as a workspace-owned shared panel.

class NotificationCenterSharedPanel : public ISharedPanel {
public:
    [[nodiscard]] const std::string& panelId()      const override { return s_panelId; }
    [[nodiscard]] const std::string& displayName()  const override { return s_displayName; }

    bool initialize() override { m_initialized = true; return true; }
    void shutdown()   override { m_initialized = false; }
    void update(float dt) override { (void)dt; }

    // ── Notification state (delegates to NotificationCenterEditor) ──

    NotificationCenterEditor&       editor()       { return m_editor; }
    const NotificationCenterEditor& editor() const { return m_editor; }

    [[nodiscard]] bool isInitialized() const { return m_initialized; }

    static constexpr const char* kPanelId = "notification_center";

private:
    inline static const std::string s_panelId{"notification_center"};
    inline static const std::string s_displayName{"Notification Center"};
    NotificationCenterEditor m_editor;
    bool m_initialized = false;
};

} // namespace NF
