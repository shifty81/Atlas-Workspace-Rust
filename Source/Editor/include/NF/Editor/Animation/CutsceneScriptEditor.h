#pragma once
// NF::Editor — Cutscene script (subtitle/voice-over) editor
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

enum class CutsceneScriptLineType : uint8_t {
    Dialogue, Subtitle, Caption, Narration, Instruction, Comment
};

inline const char* cutsceneScriptLineTypeName(CutsceneScriptLineType t) {
    switch (t) {
        case CutsceneScriptLineType::Dialogue:    return "Dialogue";
        case CutsceneScriptLineType::Subtitle:    return "Subtitle";
        case CutsceneScriptLineType::Caption:     return "Caption";
        case CutsceneScriptLineType::Narration:   return "Narration";
        case CutsceneScriptLineType::Instruction: return "Instruction";
        case CutsceneScriptLineType::Comment:     return "Comment";
    }
    return "Unknown";
}

enum class CutsceneScriptState : uint8_t {
    Draft, Review, Approved, Locked, Archived
};

inline const char* cutsceneScriptStateName(CutsceneScriptState s) {
    switch (s) {
        case CutsceneScriptState::Draft:    return "Draft";
        case CutsceneScriptState::Review:   return "Review";
        case CutsceneScriptState::Approved: return "Approved";
        case CutsceneScriptState::Locked:   return "Locked";
        case CutsceneScriptState::Archived: return "Archived";
    }
    return "Unknown";
}

enum class CutsceneScriptLocaleStatus : uint8_t {
    Missing, InProgress, Done, Verified
};

inline const char* cutsceneScriptLocaleStatusName(CutsceneScriptLocaleStatus s) {
    switch (s) {
        case CutsceneScriptLocaleStatus::Missing:    return "Missing";
        case CutsceneScriptLocaleStatus::InProgress: return "InProgress";
        case CutsceneScriptLocaleStatus::Done:       return "Done";
        case CutsceneScriptLocaleStatus::Verified:   return "Verified";
    }
    return "Unknown";
}

class CutsceneScriptLine {
public:
    explicit CutsceneScriptLine(uint32_t id, CutsceneScriptLineType type)
        : m_id(id), m_type(type) {}

    void setSpeaker(const std::string& s)  { m_speaker   = s; }
    void setText(const std::string& t)     { m_text      = t; }
    void setStartTime(float t)             { m_startTime = t; }
    void setEndTime(float t)               { m_endTime   = t; }
    void setEnabled(bool v)                { m_enabled   = v; }

    [[nodiscard]] uint32_t               id()        const { return m_id;        }
    [[nodiscard]] CutsceneScriptLineType type()      const { return m_type;      }
    [[nodiscard]] const std::string&     speaker()   const { return m_speaker;   }
    [[nodiscard]] const std::string&     text()      const { return m_text;      }
    [[nodiscard]] float                  startTime() const { return m_startTime; }
    [[nodiscard]] float                  endTime()   const { return m_endTime;   }
    [[nodiscard]] bool                   isEnabled() const { return m_enabled;   }

private:
    uint32_t              m_id;
    CutsceneScriptLineType m_type;
    std::string           m_speaker;
    std::string           m_text;
    float                 m_startTime = 0.0f;
    float                 m_endTime   = 0.0f;
    bool                  m_enabled   = true;
};

class CutsceneScriptEditor {
public:
    static constexpr size_t MAX_LINES = 2048;

    [[nodiscard]] bool addLine(const CutsceneScriptLine& line) {
        for (auto& l : m_lines) if (l.id() == line.id()) return false;
        if (m_lines.size() >= MAX_LINES) return false;
        m_lines.push_back(line);
        return true;
    }

    [[nodiscard]] bool removeLine(uint32_t id) {
        for (auto it = m_lines.begin(); it != m_lines.end(); ++it) {
            if (it->id() == id) { m_lines.erase(it); return true; }
        }
        return false;
    }

    [[nodiscard]] CutsceneScriptLine* findLine(uint32_t id) {
        for (auto& l : m_lines) if (l.id() == id) return &l;
        return nullptr;
    }

    void setState(CutsceneScriptState s)              { m_state  = s; }
    void setLocaleStatus(CutsceneScriptLocaleStatus s){ m_locale = s; }

    [[nodiscard]] CutsceneScriptState       state()        const { return m_state;  }
    [[nodiscard]] CutsceneScriptLocaleStatus localeStatus()const { return m_locale; }
    [[nodiscard]] size_t                    lineCount()    const { return m_lines.size(); }
    [[nodiscard]] size_t                    enabledCount() const {
        size_t c = 0; for (auto& l : m_lines) if (l.isEnabled()) ++c; return c;
    }
    [[nodiscard]] size_t countByLineType(CutsceneScriptLineType t) const {
        size_t c = 0; for (auto& l : m_lines) if (l.type() == t) ++c; return c;
    }
    [[nodiscard]] bool isApproved() const {
        return m_state == CutsceneScriptState::Approved || m_state == CutsceneScriptState::Locked;
    }

private:
    std::vector<CutsceneScriptLine> m_lines;
    CutsceneScriptState             m_state  = CutsceneScriptState::Draft;
    CutsceneScriptLocaleStatus      m_locale = CutsceneScriptLocaleStatus::Missing;
};

} // namespace NF
