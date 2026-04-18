#pragma once
// NF::Editor — Diagnostic panel v1: categorized diagnostic entries with filtering
#include "NF/Core/Core.h"
#include <algorithm>
#include <cstdint>
#include <functional>
#include <string>
#include <vector>

namespace NF {

enum class DpvSeverity : uint8_t { Info, Warning, Error, Fatal };
enum class DpvCategory : uint8_t { Performance, Memory, Rendering, Audio, Network, Scripting };

struct DpvEntry {
    uint32_t     id          = 0;
    DpvSeverity  severity    = DpvSeverity::Info;
    DpvCategory  category    = DpvCategory::Performance;
    std::string  message;
    std::string  source;
    uint64_t     timestampMs = 0;
    bool         dismissed   = false;
    [[nodiscard]] bool isValid() const { return id != 0 && !message.empty(); }
};

using DpvEntryCallback = std::function<void(const DpvEntry&)>;

class DiagnosticPanelV1 {
public:
    bool addEntry(const DpvEntry& entry) {
        if (!entry.isValid()) return false;
        for (const auto& e : m_entries) if (e.id == entry.id) return false;
        m_entries.push_back(entry);
        if (m_onEntry) m_onEntry(m_entries.back());
        return true;
    }

    bool removeEntry(uint32_t id) {
        for (auto it = m_entries.begin(); it != m_entries.end(); ++it) {
            if (it->id == id) { m_entries.erase(it); return true; }
        }
        return false;
    }

    bool dismiss(uint32_t id) {
        for (auto& e : m_entries) {
            if (e.id == id) { e.dismissed = true; return true; }
        }
        return false;
    }

    void dismissAll(DpvCategory category) {
        for (auto& e : m_entries)
            if (e.category == category) e.dismissed = true;
    }

    [[nodiscard]] size_t entryCount() const { return m_entries.size(); }

    [[nodiscard]] size_t countBySeverity(DpvSeverity sev) const {
        size_t n = 0;
        for (const auto& e : m_entries) if (!e.dismissed && e.severity == sev) ++n;
        return n;
    }

    [[nodiscard]] size_t countByCategory(DpvCategory cat) const {
        size_t n = 0;
        for (const auto& e : m_entries) if (!e.dismissed && e.category == cat) ++n;
        return n;
    }

    void clearDismissed() {
        m_entries.erase(
            std::remove_if(m_entries.begin(), m_entries.end(),
                           [](const DpvEntry& e){ return e.dismissed; }),
            m_entries.end());
    }

    [[nodiscard]] std::vector<DpvEntry> filterBySeverity(DpvSeverity minSev) const {
        std::vector<DpvEntry> result;
        for (const auto& e : m_entries)
            if (!e.dismissed && static_cast<uint8_t>(e.severity) >= static_cast<uint8_t>(minSev))
                result.push_back(e);
        return result;
    }

    void setOnEntry(DpvEntryCallback cb) { m_onEntry = std::move(cb); }

private:
    std::vector<DpvEntry> m_entries;
    DpvEntryCallback      m_onEntry;
};

} // namespace NF
