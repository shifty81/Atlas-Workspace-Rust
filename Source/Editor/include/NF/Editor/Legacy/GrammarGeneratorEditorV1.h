#pragma once
// NF::Editor — Grammar generator editor v1: procedural grammar rule and symbol management
#include "NF/Core/Core.h"
#include <algorithm>
#include <cstdint>
#include <functional>
#include <string>
#include <vector>

namespace NF {

enum class Ggev1SymbolType      : uint8_t { Terminal, NonTerminal, Start, End, Special };
enum class Ggev1ProductionState : uint8_t { Draft, Active, Disabled, Experimental };

inline const char* ggev1SymbolTypeName(Ggev1SymbolType t) {
    switch (t) {
        case Ggev1SymbolType::Terminal:    return "Terminal";
        case Ggev1SymbolType::NonTerminal: return "NonTerminal";
        case Ggev1SymbolType::Start:       return "Start";
        case Ggev1SymbolType::End:         return "End";
        case Ggev1SymbolType::Special:     return "Special";
    }
    return "Unknown";
}

inline const char* ggev1ProductionStateName(Ggev1ProductionState s) {
    switch (s) {
        case Ggev1ProductionState::Draft:        return "Draft";
        case Ggev1ProductionState::Active:       return "Active";
        case Ggev1ProductionState::Disabled:     return "Disabled";
        case Ggev1ProductionState::Experimental: return "Experimental";
    }
    return "Unknown";
}

struct Ggev1Symbol {
    uint64_t         id   = 0;
    std::string      name;
    Ggev1SymbolType  type = Ggev1SymbolType::NonTerminal;

    [[nodiscard]] bool isValid()    const { return id != 0 && !name.empty(); }
    [[nodiscard]] bool isTerminal() const { return type == Ggev1SymbolType::Terminal; }
};

struct Ggev1Production {
    uint64_t             id       = 0;
    uint64_t             symbolId = 0;
    std::string          rule;
    Ggev1ProductionState state    = Ggev1ProductionState::Draft;

    [[nodiscard]] bool isValid()  const { return id != 0 && symbolId != 0 && !rule.empty(); }
    [[nodiscard]] bool isActive() const { return state == Ggev1ProductionState::Active; }
};

using Ggev1ChangeCallback = std::function<void(uint64_t)>;

class GrammarGeneratorEditorV1 {
public:
    static constexpr size_t MAX_SYMBOLS     = 1024;
    static constexpr size_t MAX_PRODUCTIONS = 4096;

    bool addSymbol(const Ggev1Symbol& symbol) {
        if (!symbol.isValid()) return false;
        for (const auto& s : m_symbols) if (s.id == symbol.id) return false;
        if (m_symbols.size() >= MAX_SYMBOLS) return false;
        m_symbols.push_back(symbol);
        if (m_onChange) m_onChange(symbol.id);
        return true;
    }

    bool removeSymbol(uint64_t id) {
        for (auto it = m_symbols.begin(); it != m_symbols.end(); ++it) {
            if (it->id == id) { m_symbols.erase(it); return true; }
        }
        return false;
    }

    [[nodiscard]] Ggev1Symbol* findSymbol(uint64_t id) {
        for (auto& s : m_symbols) if (s.id == id) return &s;
        return nullptr;
    }

    bool addProduction(const Ggev1Production& production) {
        if (!production.isValid()) return false;
        for (const auto& p : m_productions) if (p.id == production.id) return false;
        if (m_productions.size() >= MAX_PRODUCTIONS) return false;
        m_productions.push_back(production);
        if (m_onChange) m_onChange(production.symbolId);
        return true;
    }

    bool removeProduction(uint64_t id) {
        for (auto it = m_productions.begin(); it != m_productions.end(); ++it) {
            if (it->id == id) { m_productions.erase(it); return true; }
        }
        return false;
    }

    [[nodiscard]] size_t symbolCount()     const { return m_symbols.size(); }
    [[nodiscard]] size_t productionCount() const { return m_productions.size(); }

    [[nodiscard]] size_t activeCount() const {
        size_t c = 0; for (const auto& p : m_productions) if (p.isActive()) ++c; return c;
    }
    [[nodiscard]] size_t terminalCount() const {
        size_t c = 0; for (const auto& s : m_symbols) if (s.isTerminal()) ++c; return c;
    }
    [[nodiscard]] size_t countBySymbolType(Ggev1SymbolType type) const {
        size_t c = 0; for (const auto& s : m_symbols) if (s.type == type) ++c; return c;
    }

    void setOnChange(Ggev1ChangeCallback cb) { m_onChange = std::move(cb); }

private:
    std::vector<Ggev1Symbol>     m_symbols;
    std::vector<Ggev1Production> m_productions;
    Ggev1ChangeCallback          m_onChange;
};

} // namespace NF
