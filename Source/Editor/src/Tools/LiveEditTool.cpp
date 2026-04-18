#include "LiveEditTool.h"

namespace atlas::tools {

void LiveEditTool::Activate() {
    m_active = true;
    m_elapsed = 0.0f;
}

void LiveEditTool::Deactivate() {
    m_active = false;
}

void LiveEditTool::Update(float deltaTime) {
    if (!m_active) return;
    m_elapsed += deltaTime;
}

void LiveEditTool::ApplyEdit(uint32_t entityId, const std::string& component,
                              const std::string& field, const std::string& oldVal,
                              const std::string& newVal) {
    LiveEdit edit;
    edit.entityId = entityId;
    edit.component = component;
    edit.field = field;
    edit.oldValue = oldVal;
    edit.newValue = newVal;
    m_edits.push_back(std::move(edit));
}

bool LiveEditTool::UndoLastEdit() {
    if (m_edits.empty()) return false;
    m_edits.pop_back();
    return true;
}

void LiveEditTool::ClearHistory() {
    m_edits.clear();
}

} // namespace atlas::tools
