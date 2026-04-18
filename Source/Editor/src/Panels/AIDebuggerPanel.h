#pragma once
// Atlas AI Debugger Panel — live AI-driven issue detection and fix suggestion

#include "../ui/EditorPanel.h"
#include "../../engine/ui/UIDrawList.h"
#include "../../engine/ai/LLMBackend.h"
#include <string>
#include <vector>
#include <functional>
#include <cstdint>

namespace atlas::editor {

enum class DiagnosticSeverity : uint8_t {
    Info,
    Warning,
    Error,
    Critical
};

struct AIDiagnosticEntry {
    std::string systemName;      // e.g. "NPC AI", "Physics", "WorldGen"
    std::string issue;           // Description of the issue
    std::string suggestedFix;    // AI-suggested fix
    DiagnosticSeverity severity = DiagnosticSeverity::Info;
    bool fixApplied = false;     // Whether the fix has been applied
    uint64_t tick = 0;           // Tick when detected
};

using DiagnosticCallback = std::function<void(const AIDiagnosticEntry&)>;

class AIDebuggerPanel : public EditorPanel {
public:
    const char* Name() const override;
    void Draw() override;
    const atlas::ui::UIDrawList& GetDrawList() const;

    // Bounds
    void SetBounds(int32_t x, int32_t y, int32_t w, int32_t h);

    // Diagnostics management
    void AddDiagnostic(const AIDiagnosticEntry& entry);
    void ClearDiagnostics();
    const std::vector<AIDiagnosticEntry>& GetDiagnostics() const;
    size_t DiagnosticCount() const;

    // Selection
    void SelectEntry(size_t index);
    size_t SelectedIndex() const;
    bool HasSelection() const;

    // Filtering
    void SetSeverityFilter(DiagnosticSeverity minSeverity);
    DiagnosticSeverity GetSeverityFilter() const;
    std::vector<AIDiagnosticEntry> GetFilteredDiagnostics() const;

    // Fix application
    bool ApplyFix(size_t index);
    uint32_t AppliedFixCount() const;

    // LLM integration
    void SetLLMBackend(atlas::ai::ILLMBackend* backend);
    atlas::ai::ILLMBackend* GetLLMBackend() const;
    bool IsLLMConnected() const;

    // Auto-diagnose
    void SetOnFixApplied(DiagnosticCallback callback);
    void RunAutoDiagnose(const std::string& systemName);

private:
    atlas::ui::UIDrawList m_drawList;
    std::vector<AIDiagnosticEntry> m_diagnostics;
    size_t m_selectedIndex = SIZE_MAX;
    DiagnosticSeverity m_severityFilter = DiagnosticSeverity::Info;
    atlas::ai::ILLMBackend* m_llmBackend = nullptr;
    uint32_t m_appliedFixCount = 0;
    DiagnosticCallback m_onFixApplied;
    int32_t m_x = 0, m_y = 0, m_width = 400, m_height = 300;
};

}  // namespace atlas::editor
