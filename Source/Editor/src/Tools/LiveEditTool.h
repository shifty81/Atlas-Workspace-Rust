#pragma once
#include "ITool.h"
#include <cstdint>
#include <string>
#include <vector>
#include <utility>

namespace atlas::tools {

/// Record of a live edit operation.
struct LiveEdit {
    uint32_t entityId = 0;
    std::string component;
    std::string field;
    std::string oldValue;
    std::string newValue;
};

/// In-client overlay tool for editing entities while simulation runs.
///
/// Unlike the full editor's pause-edit-resume workflow, LiveEditTool
/// applies property changes to entities in real-time, recording each
/// edit for later undo or commit.
class LiveEditTool : public ITool {
public:
    std::string Name() const override { return "Live Edit"; }
    void Activate() override;
    void Deactivate() override;
    void Update(float deltaTime) override;
    bool IsActive() const override { return m_active; }

    /// Apply an edit to an entity property.
    void ApplyEdit(uint32_t entityId, const std::string& component,
                   const std::string& field, const std::string& oldVal,
                   const std::string& newVal);

    /// Get edit history.
    const std::vector<LiveEdit>& EditHistory() const { return m_edits; }
    size_t EditCount() const { return m_edits.size(); }

    /// Undo last edit (returns false if nothing to undo).
    bool UndoLastEdit();

    /// Clear all edit history.
    void ClearHistory();

    float ElapsedTime() const { return m_elapsed; }

private:
    bool m_active = false;
    std::vector<LiveEdit> m_edits;
    float m_elapsed = 0.0f;
};

} // namespace atlas::tools
