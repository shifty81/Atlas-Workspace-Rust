#pragma once
// NF::Editor — Drop target handler: Win32-style file drag-and-drop target surface
#include "NF/Workspace/FileIntakePipeline.h"
#include <cstdint>
#include <string>
#include <vector>

namespace NF {

// ── Drop State ────────────────────────────────────────────────────

enum class DropState : uint8_t {
    Idle,       // no drag in progress
    DragOver,   // drag is hovering over target
    DragLeave,  // drag left target
    Dropped,    // drop completed
    Rejected,   // drop was refused
};

inline const char* dropStateName(DropState s) {
    switch (s) {
        case DropState::Idle:      return "Idle";
        case DropState::DragOver:  return "DragOver";
        case DropState::DragLeave: return "DragLeave";
        case DropState::Dropped:   return "Dropped";
        case DropState::Rejected:  return "Rejected";
    }
    return "Unknown";
}

// ── Drop Effect ───────────────────────────────────────────────────

enum class DropEffect : uint8_t { None, Copy, Move, Link };

inline const char* dropEffectName(DropEffect e) {
    switch (e) {
        case DropEffect::None: return "None";
        case DropEffect::Copy: return "Copy";
        case DropEffect::Move: return "Move";
        case DropEffect::Link: return "Link";
    }
    return "Unknown";
}

// ── Drop Target Handler ───────────────────────────────────────────

class DropTargetHandler {
public:
    explicit DropTargetHandler(FileIntakePipeline* pipeline = nullptr)
        : m_pipeline(pipeline) {}

    void bindPipeline(FileIntakePipeline* pipeline) { m_pipeline = pipeline; }
    [[nodiscard]] FileIntakePipeline* pipeline() const { return m_pipeline; }

    // Called when drag enters the target surface
    DropEffect onDragEnter(const std::vector<std::string>& paths) {
        m_state = DropState::DragOver;
        m_hoveredPaths = paths;
        ++m_enterCount;

        // Evaluate if we can accept these file types
        for (const auto& p : paths) {
            auto ext = std::filesystem::path(p).extension().string();
            auto type = detectIntakeFileType(ext);
            if (type == IntakeFileType::Unknown && !m_acceptUnknown) {
                m_state = DropState::Rejected;
                return DropEffect::None;
            }
        }
        return m_defaultEffect;
    }

    // Called when drag hovers (mouse position update)
    DropEffect onDragOver(float /*x*/, float /*y*/) {
        if (m_state == DropState::Rejected) return DropEffect::None;
        m_state = DropState::DragOver;
        return m_defaultEffect;
    }

    // Called when drag leaves
    void onDragLeave() {
        m_state = DropState::DragLeave;
        m_hoveredPaths.clear();
        ++m_leaveCount;
    }

    // Called when files are actually dropped
    size_t onDrop(const std::vector<std::string>& paths) {
        m_state = DropState::Dropped;
        m_lastDroppedPaths = paths;
        ++m_dropCount;

        if (!m_pipeline) return 0;
        size_t accepted = m_pipeline->ingestBatch(paths, IntakeSource::FileDrop);
        m_totalDroppedFiles += accepted;
        return accepted;
    }

    void reset() {
        m_state = DropState::Idle;
        m_hoveredPaths.clear();
        m_lastDroppedPaths.clear();
    }

    // Configuration
    void setDefaultEffect(DropEffect e) { m_defaultEffect = e; }
    void setAcceptUnknown(bool accept)  { m_acceptUnknown = accept; }

    [[nodiscard]] DropState                    state()             const { return m_state;             }
    [[nodiscard]] DropEffect                   defaultEffect()     const { return m_defaultEffect;     }
    [[nodiscard]] bool                         acceptUnknown()     const { return m_acceptUnknown;     }
    [[nodiscard]] const std::vector<std::string>& hoveredPaths()   const { return m_hoveredPaths;      }
    [[nodiscard]] const std::vector<std::string>& lastDroppedPaths() const { return m_lastDroppedPaths; }
    [[nodiscard]] size_t enterCount()     const { return m_enterCount;      }
    [[nodiscard]] size_t leaveCount()     const { return m_leaveCount;      }
    [[nodiscard]] size_t dropCount()      const { return m_dropCount;       }
    [[nodiscard]] size_t totalDropped()   const { return m_totalDroppedFiles; }
    [[nodiscard]] bool   isDragActive()   const { return m_state == DropState::DragOver; }

private:
    FileIntakePipeline*  m_pipeline      = nullptr;
    DropState            m_state         = DropState::Idle;
    DropEffect           m_defaultEffect = DropEffect::Copy;
    bool                 m_acceptUnknown = false;
    std::vector<std::string> m_hoveredPaths;
    std::vector<std::string> m_lastDroppedPaths;
    size_t m_enterCount        = 0;
    size_t m_leaveCount        = 0;
    size_t m_dropCount         = 0;
    size_t m_totalDroppedFiles = 0;
};

} // namespace NF
