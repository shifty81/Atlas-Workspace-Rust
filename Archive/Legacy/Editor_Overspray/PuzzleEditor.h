#pragma once
// NF::Editor — puzzle editor
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

enum class PuzzleType : uint8_t {
    Slider, Jigsaw, Logic, Sequence, Pattern, Password, Physics, Combination
};

inline const char* puzzleTypeName(PuzzleType t) {
    switch (t) {
        case PuzzleType::Slider:      return "Slider";
        case PuzzleType::Jigsaw:      return "Jigsaw";
        case PuzzleType::Logic:       return "Logic";
        case PuzzleType::Sequence:    return "Sequence";
        case PuzzleType::Pattern:     return "Pattern";
        case PuzzleType::Password:    return "Password";
        case PuzzleType::Physics:     return "Physics";
        case PuzzleType::Combination: return "Combination";
    }
    return "Unknown";
}

enum class PuzzleSolutionType : uint8_t {
    Single, Multiple, Partial, Progressive, Timed
};

inline const char* puzzleSolutionTypeName(PuzzleSolutionType s) {
    switch (s) {
        case PuzzleSolutionType::Single:      return "Single";
        case PuzzleSolutionType::Multiple:    return "Multiple";
        case PuzzleSolutionType::Partial:     return "Partial";
        case PuzzleSolutionType::Progressive: return "Progressive";
        case PuzzleSolutionType::Timed:       return "Timed";
    }
    return "Unknown";
}

enum class PuzzleHintMode : uint8_t {
    None, OnRequest, Automatic, Penalty
};

inline const char* puzzleHintModeName(PuzzleHintMode h) {
    switch (h) {
        case PuzzleHintMode::None:      return "None";
        case PuzzleHintMode::OnRequest: return "OnRequest";
        case PuzzleHintMode::Automatic: return "Automatic";
        case PuzzleHintMode::Penalty:   return "Penalty";
    }
    return "Unknown";
}

class PuzzleDefinition {
public:
    explicit PuzzleDefinition(uint32_t id, const std::string& name, PuzzleType type)
        : m_id(id), m_name(name), m_type(type) {}

    void setSolutionType(PuzzleSolutionType v)  { m_solutionType = v; }
    void setHintMode(PuzzleHintMode v)          { m_hintMode     = v; }
    void setMaxAttempts(uint32_t v)             { m_maxAttempts  = v; }
    void setSkippable(bool v)                   { m_isSkippable  = v; }
    void setPointValue(uint32_t v)              { m_pointValue   = v; }

    [[nodiscard]] uint32_t            id()            const { return m_id;           }
    [[nodiscard]] const std::string&  name()          const { return m_name;         }
    [[nodiscard]] PuzzleType          type()          const { return m_type;         }
    [[nodiscard]] PuzzleSolutionType  solutionType()  const { return m_solutionType; }
    [[nodiscard]] PuzzleHintMode      hintMode()      const { return m_hintMode;     }
    [[nodiscard]] uint32_t            maxAttempts()   const { return m_maxAttempts;  }
    [[nodiscard]] bool                isSkippable()   const { return m_isSkippable;  }
    [[nodiscard]] uint32_t            pointValue()    const { return m_pointValue;   }

private:
    uint32_t          m_id;
    std::string       m_name;
    PuzzleType        m_type;
    PuzzleSolutionType m_solutionType = PuzzleSolutionType::Single;
    PuzzleHintMode    m_hintMode      = PuzzleHintMode::OnRequest;
    uint32_t          m_maxAttempts   = 3u;
    bool              m_isSkippable   = false;
    uint32_t          m_pointValue    = 100u;
};

class PuzzleEditor {
public:
    void setShowSolution(bool v) { m_showSolution = v; }
    void setShowHints(bool v)    { m_showHints    = v; }
    void setGridSnap(float v)    { m_gridSnap     = v; }

    bool addPuzzle(const PuzzleDefinition& p) {
        for (auto& e : m_puzzles) if (e.id() == p.id()) return false;
        m_puzzles.push_back(p); return true;
    }
    bool removePuzzle(uint32_t id) {
        auto it = std::find_if(m_puzzles.begin(), m_puzzles.end(),
            [&](const PuzzleDefinition& e){ return e.id() == id; });
        if (it == m_puzzles.end()) return false;
        m_puzzles.erase(it); return true;
    }
    [[nodiscard]] PuzzleDefinition* findPuzzle(uint32_t id) {
        for (auto& e : m_puzzles) if (e.id() == id) return &e;
        return nullptr;
    }

    [[nodiscard]] bool   isShowSolution() const { return m_showSolution;    }
    [[nodiscard]] bool   isShowHints()    const { return m_showHints;       }
    [[nodiscard]] float  gridSnap()       const { return m_gridSnap;        }
    [[nodiscard]] size_t puzzleCount()    const { return m_puzzles.size();  }

    [[nodiscard]] size_t countByType(PuzzleType t) const {
        size_t c = 0; for (auto& e : m_puzzles) if (e.type() == t) ++c; return c;
    }
    [[nodiscard]] size_t countByHintMode(PuzzleHintMode h) const {
        size_t c = 0; for (auto& e : m_puzzles) if (e.hintMode() == h) ++c; return c;
    }
    [[nodiscard]] size_t countSkippable() const {
        size_t c = 0; for (auto& e : m_puzzles) if (e.isSkippable()) ++c; return c;
    }

private:
    std::vector<PuzzleDefinition> m_puzzles;
    bool  m_showSolution = false;
    bool  m_showHints    = true;
    float m_gridSnap     = 1.0f;
};

} // namespace NF
