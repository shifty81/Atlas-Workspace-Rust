#include "AIDebuggerPanel.h"
#include <sstream>

namespace atlas::editor {

const char* AIDebuggerPanel::Name() const { return "AI Debugger"; }

const atlas::ui::UIDrawList& AIDebuggerPanel::GetDrawList() const {
    return m_drawList;
}

void AIDebuggerPanel::SetBounds(int32_t x, int32_t y, int32_t w, int32_t h) {
    m_x = x;
    m_y = y;
    m_width = w;
    m_height = h;
}

// --- Diagnostics management ---

void AIDebuggerPanel::AddDiagnostic(const AIDiagnosticEntry& entry) {
    m_diagnostics.push_back(entry);
}

void AIDebuggerPanel::ClearDiagnostics() {
    m_diagnostics.clear();
    m_selectedIndex = SIZE_MAX;
}

const std::vector<AIDiagnosticEntry>& AIDebuggerPanel::GetDiagnostics() const {
    return m_diagnostics;
}

size_t AIDebuggerPanel::DiagnosticCount() const {
    return m_diagnostics.size();
}

// --- Selection ---

void AIDebuggerPanel::SelectEntry(size_t index) {
    if (index < m_diagnostics.size()) {
        m_selectedIndex = index;
    }
}

size_t AIDebuggerPanel::SelectedIndex() const {
    return m_selectedIndex;
}

bool AIDebuggerPanel::HasSelection() const {
    return m_selectedIndex < m_diagnostics.size();
}

// --- Filtering ---

void AIDebuggerPanel::SetSeverityFilter(DiagnosticSeverity minSeverity) {
    m_severityFilter = minSeverity;
}

DiagnosticSeverity AIDebuggerPanel::GetSeverityFilter() const {
    return m_severityFilter;
}

std::vector<AIDiagnosticEntry> AIDebuggerPanel::GetFilteredDiagnostics() const {
    std::vector<AIDiagnosticEntry> result;
    for (const auto& d : m_diagnostics) {
        if (static_cast<uint8_t>(d.severity) >= static_cast<uint8_t>(m_severityFilter)) {
            result.push_back(d);
        }
    }
    return result;
}

// --- Fix application ---

bool AIDebuggerPanel::ApplyFix(size_t index) {
    if (index >= m_diagnostics.size()) return false;
    if (m_diagnostics[index].fixApplied) return false;

    m_diagnostics[index].fixApplied = true;
    ++m_appliedFixCount;

    if (m_onFixApplied) {
        m_onFixApplied(m_diagnostics[index]);
    }
    return true;
}

uint32_t AIDebuggerPanel::AppliedFixCount() const {
    return m_appliedFixCount;
}

// --- LLM integration ---

void AIDebuggerPanel::SetLLMBackend(atlas::ai::ILLMBackend* backend) {
    m_llmBackend = backend;
}

atlas::ai::ILLMBackend* AIDebuggerPanel::GetLLMBackend() const {
    return m_llmBackend;
}

bool AIDebuggerPanel::IsLLMConnected() const {
    return m_llmBackend && m_llmBackend->IsAvailable();
}

// --- Auto-diagnose ---

void AIDebuggerPanel::SetOnFixApplied(DiagnosticCallback callback) {
    m_onFixApplied = std::move(callback);
}

void AIDebuggerPanel::RunAutoDiagnose(const std::string& systemName) {
    AIDiagnosticEntry entry;
    entry.systemName = systemName;

    if (m_llmBackend) {
        atlas::ai::LLMRequest req;
        req.prompt = "Diagnose issues in system: " + systemName;
        atlas::ai::LLMResponse resp = m_llmBackend->Complete(req);
        if (!resp.text.empty()) {
            entry.issue = resp.text;
            entry.suggestedFix = "Review LLM suggestion above";
            entry.severity = DiagnosticSeverity::Warning;
        } else {
            entry.issue = "LLM returned empty response";
            entry.suggestedFix = "Check LLM backend configuration";
            entry.severity = DiagnosticSeverity::Info;
        }
    } else {
        entry.issue = "No LLM backend available for diagnostics";
        entry.suggestedFix = "Connect an LLM backend to enable AI diagnostics";
        entry.severity = DiagnosticSeverity::Info;
    }

    m_diagnostics.push_back(entry);
}

// --- Severity helpers ---

static atlas::ui::UIColor SeverityColor(DiagnosticSeverity sev) {
    switch (sev) {
    case DiagnosticSeverity::Info:     return {160, 160, 160, 255};
    case DiagnosticSeverity::Warning:  return {230, 200, 50, 255};
    case DiagnosticSeverity::Error:    return {220, 60, 60, 255};
    case DiagnosticSeverity::Critical: return {255, 40, 40, 255};
    }
    return {160, 160, 160, 255};
}

static const char* SeverityLabel(DiagnosticSeverity sev) {
    switch (sev) {
    case DiagnosticSeverity::Info:     return "Info";
    case DiagnosticSeverity::Warning:  return "Warning";
    case DiagnosticSeverity::Error:    return "Error";
    case DiagnosticSeverity::Critical: return "Critical";
    }
    return "Info";
}

// --- Draw ---

void AIDebuggerPanel::Draw() {
    m_drawList.Clear();

    // Panel background
    m_drawList.DrawRect({m_x, m_y, m_width, m_height}, {43, 43, 43, 255});

    // Border
    m_drawList.DrawBorder({m_x, m_y, m_width, m_height}, 1, {70, 73, 75, 255});

    // Header bar
    m_drawList.DrawRect({m_x, m_y, m_width, 24}, {60, 63, 65, 255});
    m_drawList.DrawText({m_x + 4, m_y + 4, m_width - 8, 20},
                        "AI Debugger", {220, 220, 220, 255});

    int32_t y = m_y + 28;

    // LLM status line
    if (IsLLMConnected()) {
        m_drawList.DrawText({m_x + 4, y, m_width - 8, 16},
                            "LLM: Connected", {100, 200, 100, 255});
    } else {
        m_drawList.DrawText({m_x + 4, y, m_width - 8, 16},
                            "LLM: Not Connected", {120, 120, 120, 255});
    }
    y += 18;

    // Build filtered list with original indices
    std::vector<size_t> filteredIndices;
    for (size_t i = 0; i < m_diagnostics.size(); ++i) {
        if (static_cast<uint8_t>(m_diagnostics[i].severity) >=
            static_cast<uint8_t>(m_severityFilter)) {
            filteredIndices.push_back(i);
        }
    }

    // Diagnostic count and filter status
    std::string info = "Diagnostics: " + std::to_string(filteredIndices.size()) +
                       "/" + std::to_string(m_diagnostics.size()) +
                       "  Filter: " + SeverityLabel(m_severityFilter) + "+";
    m_drawList.DrawText({m_x + 4, y, m_width - 8, 16}, info, {180, 180, 180, 255});
    y += 20;

    if (filteredIndices.empty()) {
        m_drawList.DrawText({m_x + 4, y, m_width - 8, 16},
                            "No diagnostics", {120, 120, 120, 255});
        return;
    }

    // Diagnostic list
    for (size_t fi = 0; fi < filteredIndices.size(); ++fi) {
        size_t origIdx = filteredIndices[fi];
        const auto& diag = m_diagnostics[origIdx];

        bool selected = (origIdx == m_selectedIndex);

        // Row background
        atlas::ui::UIColor bg = selected
            ? atlas::ui::UIColor{65, 105, 170, 255}
            : atlas::ui::UIColor{43, 43, 43, 255};
        m_drawList.DrawRect({m_x, y, m_width, 20}, bg);

        // Severity indicator
        atlas::ui::UIColor sevColor = SeverityColor(diag.severity);

        // Fix applied marker
        std::string prefix;
        if (diag.fixApplied) {
            prefix = "\xe2\x9c\x93 ";  // UTF-8 ✓
        }

        std::string line = prefix + "[" + SeverityLabel(diag.severity) + "] " +
                           diag.systemName + ": " + diag.issue;
        m_drawList.DrawText({m_x + 4, y + 2, m_width - 8, 16}, line, sevColor);
        y += 20;
    }

    // Detail area for selected entry
    if (HasSelection()) {
        const auto& sel = m_diagnostics[m_selectedIndex];
        y += 4;
        m_drawList.DrawRect({m_x, y, m_width, 2}, {70, 73, 75, 255});
        y += 6;
        m_drawList.DrawText({m_x + 4, y, m_width - 8, 16},
                            "Suggested Fix:", {220, 220, 220, 255});
        y += 18;
        m_drawList.DrawText({m_x + 8, y, m_width - 16, 16},
                            sel.suggestedFix, {180, 200, 220, 255});
    }
}

}  // namespace atlas::editor
