#pragma once
// NF::Editor — Compiler settings v1: flags, optimization levels, command-line generation
#include "NF/Core/Core.h"
#include <cstdint>
#include <string>
#include <vector>

namespace NF {

enum class CsOptLevel : uint8_t { Debug, O1, O2, O3, Size };

inline const char* csOptLevelName(CsOptLevel l) {
    switch(l){
        case CsOptLevel::Debug: return "Debug";
        case CsOptLevel::O1:    return "O1";
        case CsOptLevel::O2:    return "O2";
        case CsOptLevel::O3:    return "O3";
        case CsOptLevel::Size:  return "Size";
    }
    return "Unknown";
}

struct CsFlag {
    uint32_t    id      = 0;
    std::string name;
    std::string value;
    bool        enabled = true;
    [[nodiscard]] bool isValid() const { return id != 0 && !name.empty(); }
};

class CompilerSettingsV1 {
public:
    static constexpr size_t MAX_FLAGS = 128;

    bool addFlag(const CsFlag& flag) {
        if (!flag.isValid()) return false;
        if (m_flags.size() >= MAX_FLAGS) return false;
        for (const auto& f : m_flags) if (f.id == flag.id) return false;
        m_flags.push_back(flag);
        return true;
    }

    bool removeFlag(uint32_t id) {
        for (auto it = m_flags.begin(); it != m_flags.end(); ++it) {
            if (it->id == id) { m_flags.erase(it); return true; }
        }
        return false;
    }

    void setOptLevel(CsOptLevel level) { m_optLevel = level; }
    [[nodiscard]] CsOptLevel getOptLevel() const { return m_optLevel; }

    const CsFlag* findFlag(const std::string& name) const {
        for (const auto& f : m_flags) if (f.name == name) return &f;
        return nullptr;
    }

    [[nodiscard]] size_t flagCount() const { return m_flags.size(); }

    [[nodiscard]] std::string generateCommandLine() const {
        std::string cmd;
        switch(m_optLevel){
            case CsOptLevel::Debug: cmd += "-O0 -g"; break;
            case CsOptLevel::O1:    cmd += "-O1";    break;
            case CsOptLevel::O2:    cmd += "-O2";    break;
            case CsOptLevel::O3:    cmd += "-O3";    break;
            case CsOptLevel::Size:  cmd += "-Os";    break;
        }
        for (const auto& f : m_flags) {
            if (!f.enabled) continue;
            cmd += " " + f.name;
            if (!f.value.empty()) cmd += "=" + f.value;
        }
        return cmd;
    }

private:
    std::vector<CsFlag> m_flags;
    CsOptLevel          m_optLevel = CsOptLevel::Debug;
};

} // namespace NF
