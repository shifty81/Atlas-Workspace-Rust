#pragma once
// NF::Editor — scripting console
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

enum class SConLanguage : uint8_t {
    Lua, Python, JavaScript, AngelScript, Squirrel, CSharp
};

inline const char* sConLanguageName(SConLanguage l) {
    switch (l) {
        case SConLanguage::Lua:         return "Lua";
        case SConLanguage::Python:      return "Python";
        case SConLanguage::JavaScript:  return "JavaScript";
        case SConLanguage::AngelScript: return "AngelScript";
        case SConLanguage::Squirrel:    return "Squirrel";
        case SConLanguage::CSharp:      return "CSharp";
    }
    return "Unknown";
}

enum class ScriptExecutionMode : uint8_t {
    Immediate, Deferred, Scheduled, OnEvent
};

inline const char* scriptExecutionModeName(ScriptExecutionMode m) {
    switch (m) {
        case ScriptExecutionMode::Immediate: return "Immediate";
        case ScriptExecutionMode::Deferred:  return "Deferred";
        case ScriptExecutionMode::Scheduled: return "Scheduled";
        case ScriptExecutionMode::OnEvent:   return "OnEvent";
    }
    return "Unknown";
}

enum class ScriptSandboxLevel : uint8_t {
    None, Low, Medium, High, Strict
};

inline const char* scriptSandboxLevelName(ScriptSandboxLevel s) {
    switch (s) {
        case ScriptSandboxLevel::None:   return "None";
        case ScriptSandboxLevel::Low:    return "Low";
        case ScriptSandboxLevel::Medium: return "Medium";
        case ScriptSandboxLevel::High:   return "High";
        case ScriptSandboxLevel::Strict: return "Strict";
    }
    return "Unknown";
}

class ScriptEntry {
public:
    explicit ScriptEntry(uint32_t id, const std::string& name, SConLanguage lang)
        : m_id(id), m_name(name), m_language(lang) {}

    void setExecutionMode(ScriptExecutionMode v) { m_executionMode = v; }
    void setSandboxLevel(ScriptSandboxLevel v)   { m_sandboxLevel  = v; }
    void setIsEnabled(bool v)                    { m_isEnabled     = v; }
    void setSource(const std::string& v)         { m_source        = v; }

    [[nodiscard]] uint32_t             id()            const { return m_id;            }
    [[nodiscard]] const std::string&   name()          const { return m_name;          }
    [[nodiscard]] SConLanguage       language()      const { return m_language;      }
    [[nodiscard]] ScriptExecutionMode  executionMode() const { return m_executionMode; }
    [[nodiscard]] ScriptSandboxLevel   sandboxLevel()  const { return m_sandboxLevel;  }
    [[nodiscard]] bool                 isEnabled()     const { return m_isEnabled;     }
    [[nodiscard]] const std::string&   source()        const { return m_source;        }

private:
    uint32_t            m_id;
    std::string         m_name;
    SConLanguage      m_language;
    ScriptExecutionMode m_executionMode = ScriptExecutionMode::Immediate;
    ScriptSandboxLevel  m_sandboxLevel  = ScriptSandboxLevel::Medium;
    bool                m_isEnabled     = true;
    std::string         m_source;
};

class ScriptingConsole {
public:
    void setActiveLanguage(SConLanguage v)  { m_activeLanguage  = v; }
    void setShowLineNumbers(bool v)           { m_showLineNumbers = v; }
    void setShowAutoComplete(bool v)          { m_showAutoComplete = v; }
    void setSandboxLevel(ScriptSandboxLevel v){ m_sandboxLevel    = v; }

    bool addScript(const ScriptEntry& s) {
        for (auto& e : m_scripts) if (e.id() == s.id()) return false;
        m_scripts.push_back(s); return true;
    }
    bool removeScript(uint32_t id) {
        auto it = std::find_if(m_scripts.begin(), m_scripts.end(),
            [&](const ScriptEntry& e){ return e.id() == id; });
        if (it == m_scripts.end()) return false;
        m_scripts.erase(it); return true;
    }
    [[nodiscard]] ScriptEntry* findScript(uint32_t id) {
        for (auto& e : m_scripts) if (e.id() == id) return &e;
        return nullptr;
    }

    [[nodiscard]] SConLanguage    activeLanguage()   const { return m_activeLanguage;   }
    [[nodiscard]] bool              isShowLineNumbers()const { return m_showLineNumbers;  }
    [[nodiscard]] bool              isShowAutoComplete()const{ return m_showAutoComplete; }
    [[nodiscard]] ScriptSandboxLevel sandboxLevel()    const { return m_sandboxLevel;     }
    [[nodiscard]] size_t            scriptCount()      const { return m_scripts.size();   }

    [[nodiscard]] size_t countByLanguage(SConLanguage l) const {
        size_t n = 0; for (auto& e : m_scripts) if (e.language() == l) ++n; return n;
    }
    [[nodiscard]] size_t countByExecutionMode(ScriptExecutionMode m) const {
        size_t n = 0; for (auto& e : m_scripts) if (e.executionMode() == m) ++n; return n;
    }
    [[nodiscard]] size_t countEnabled() const {
        size_t n = 0; for (auto& e : m_scripts) if (e.isEnabled()) ++n; return n;
    }

private:
    std::vector<ScriptEntry> m_scripts;
    SConLanguage     m_activeLanguage  = SConLanguage::Lua;
    bool               m_showLineNumbers = true;
    bool               m_showAutoComplete = true;
    ScriptSandboxLevel m_sandboxLevel    = ScriptSandboxLevel::Medium;
};

} // namespace NF
