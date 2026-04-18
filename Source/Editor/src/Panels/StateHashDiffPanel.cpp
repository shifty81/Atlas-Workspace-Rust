#include "StateHashDiffPanel.h"
#include <sstream>
#include <cstdio>

namespace atlas::editor {

void StateHashDiffPanel::Draw() {
    Refresh();

    m_drawList.Clear();

    // Background
    m_drawList.DrawRect({0, 0, 600, 400}, {30, 30, 30, 255});

    // Title bar
    m_drawList.DrawRect({0, 0, 600, 24}, {50, 50, 50, 255});
    m_drawList.DrawText({4, 4, 300, 20}, "State Hash Diff", {220, 220, 220, 255});

    int32_t y = 28;

    if (!m_local || !m_remote) {
        m_drawList.DrawText({4, y, 590, 16}, "No hashers attached", {160, 160, 160, 255});
        return;
    }

    if (m_entries.empty()) {
        m_drawList.DrawText({4, y, 590, 16}, "No hash entries", {160, 160, 160, 255});
        return;
    }

    // Status line
    std::string status = HasDivergence()
        ? Summary()
        : "No divergence";
    atlas::ui::UIColor statusColor = HasDivergence()
        ? atlas::ui::UIColor{255, 100, 100, 255}
        : atlas::ui::UIColor{100, 200, 100, 255};
    m_drawList.DrawText({4, y, 590, 16}, status, statusColor);
    y += 20;

    // Column header
    m_drawList.DrawRect({0, y, 600, 20}, {45, 45, 45, 255});
    m_drawList.DrawText({4, y + 2, 590, 16}, "Tick       Local Hash       Remote Hash      Status",
                        {180, 200, 220, 255});
    y += 22;

    // Hash entries
    char buf[192];
    for (const auto& e : m_entries) {
        atlas::ui::UIColor rowColor = e.matches
            ? atlas::ui::UIColor{200, 200, 200, 255}
            : atlas::ui::UIColor{255, 100, 100, 255};
        std::snprintf(buf, sizeof(buf), "%-10llu 0x%012llX   0x%012llX   %s",
                      (unsigned long long)e.tick,
                      (unsigned long long)e.localHash,
                      (unsigned long long)e.remoteHash,
                      e.matches ? "Match" : "DIVERGENT");
        m_drawList.DrawText({4, y, 590, 16}, buf, rowColor);
        y += 20;
    }

    // Per-system breakdown if available
    if (m_hasPerSystemBreakdown) {
        y += 4;
        m_drawList.DrawRect({0, y, 600, 20}, {45, 45, 45, 255});
        m_drawList.DrawText({4, y + 2, 590, 16}, "Per-System Breakdown", {180, 200, 220, 255});
        y += 22;

        auto divSystems = DivergentSystems();
        if (divSystems.empty()) {
            m_drawList.DrawText({4, y, 590, 16}, "All systems match", {100, 200, 100, 255});
        } else {
            for (const auto& sys : divSystems) {
                m_drawList.DrawText({8, y, 580, 16}, "Divergent: " + sys, {255, 100, 100, 255});
                y += 20;
            }
        }
    }
}

void StateHashDiffPanel::SetLocalHasher(const sim::StateHasher* local) {
    m_local = local;
}

void StateHashDiffPanel::SetRemoteHasher(const sim::StateHasher* remote) {
    m_remote = remote;
}

void StateHashDiffPanel::Refresh() {
    m_entries.clear();
    m_firstDivergence = -1;

    if (!m_local || !m_remote) return;

    const auto& localHist = m_local->History();
    const auto& remoteHist = m_remote->History();

    size_t count = localHist.size();
    if (remoteHist.size() < count) count = remoteHist.size();

    for (size_t i = 0; i < count; ++i) {
        HashDiffEntry entry;
        entry.tick = localHist[i].tick;
        entry.localHash = localHist[i].hash;
        entry.remoteHash = remoteHist[i].hash;
        entry.matches = (entry.localHash == entry.remoteHash);
        m_entries.push_back(entry);

        if (!entry.matches && m_firstDivergence < 0) {
            m_firstDivergence = static_cast<int64_t>(entry.tick);
        }
    }
}

const std::vector<HashDiffEntry>& StateHashDiffPanel::Entries() const {
    return m_entries;
}

int64_t StateHashDiffPanel::FirstDivergenceTick() const {
    return m_firstDivergence;
}

bool StateHashDiffPanel::HasDivergence() const {
    return m_firstDivergence >= 0;
}

std::string StateHashDiffPanel::Summary() const {
    if (!HasDivergence()) {
        return "No divergence";
    }

    // Find the divergent entry
    for (const auto& e : m_entries) {
        if (!e.matches) {
            char buf[128];
            std::snprintf(buf, sizeof(buf),
                "Divergence at tick %llu: local=0x%llX remote=0x%llX",
                (unsigned long long)e.tick,
                (unsigned long long)e.localHash,
                (unsigned long long)e.remoteHash);
            return std::string(buf);
        }
    }

    return "No divergence";
}

void StateHashDiffPanel::SetComponentBreakdown(const ComponentHashBreakdown& breakdown) {
    m_componentBreakdown = breakdown;
    m_hasComponentBreakdown = true;
}

const ComponentHashBreakdown& StateHashDiffPanel::GetComponentBreakdown() const {
    return m_componentBreakdown;
}

bool StateHashDiffPanel::HasComponentBreakdown() const {
    return m_hasComponentBreakdown;
}

std::vector<std::string> StateHashDiffPanel::DivergentComponents() const {
    std::vector<std::string> result;
    if (!m_hasComponentBreakdown) return result;
    for (const auto& c : m_componentBreakdown.components) {
        if (!c.matches) {
            result.push_back(c.name);
        }
    }
    return result;
}

HashLadderComparison StateHashDiffPanel::CompareHashLadders(
        const sim::StateHasher& a,
        const sim::StateHasher& b) const {
    HashLadderComparison result;

    const auto& ah = a.History();
    const auto& bh = b.History();
    size_t count = std::min(ah.size(), bh.size());
    result.totalCount = count;
    result.matchCount = 0;

    for (size_t i = 0; i < count; ++i) {
        HashDiffEntry entry;
        entry.tick = ah[i].tick;
        entry.localHash = ah[i].hash;
        entry.remoteHash = bh[i].hash;
        entry.matches = (entry.localHash == entry.remoteHash);
        result.entries.push_back(entry);

        if (entry.matches) {
            ++result.matchCount;
        } else if (result.firstDivergenceTick < 0) {
            result.firstDivergenceTick = static_cast<int64_t>(entry.tick);
        }
    }

    result.matchPercentage = (count > 0)
        ? (static_cast<double>(result.matchCount) / static_cast<double>(count)) * 100.0
        : 100.0;

    return result;
}

DivergenceDetail StateHashDiffPanel::GetDivergenceDetail() const {
    DivergenceDetail detail;

    if (!HasDivergence()) {
        detail.summary = "No divergence";
        return detail;
    }

    // Find first non-matching entry
    for (const auto& e : m_entries) {
        if (!e.matches) {
            detail.tick = static_cast<int64_t>(e.tick);
            detail.localHash = e.localHash;
            detail.remoteHash = e.remoteHash;
            break;
        }
    }

    // Include component breakdown if available at that tick
    if (m_hasComponentBreakdown &&
        static_cast<int64_t>(m_componentBreakdown.tick) == detail.tick) {
        for (const auto& c : m_componentBreakdown.components) {
            if (!c.matches) {
                detail.divergentComponents.push_back(c);
            }
        }
    }

    char buf[256];
    std::snprintf(buf, sizeof(buf),
        "Divergence at tick %lld: local=0x%llX remote=0x%llX, %zu divergent component(s)",
        (long long)detail.tick,
        (unsigned long long)detail.localHash,
        (unsigned long long)detail.remoteHash,
        detail.divergentComponents.size());
    detail.summary = buf;

    return detail;
}

void StateHashDiffPanel::SetPerSystemBreakdown(const PerSystemHashBreakdown& breakdown) {
    m_perSystemBreakdown = breakdown;
    m_hasPerSystemBreakdown = true;
}

const PerSystemHashBreakdown& StateHashDiffPanel::GetPerSystemBreakdown() const {
    return m_perSystemBreakdown;
}

bool StateHashDiffPanel::HasPerSystemBreakdown() const {
    return m_hasPerSystemBreakdown;
}

std::vector<std::string> StateHashDiffPanel::DivergentSystems() const {
    if (!m_hasPerSystemBreakdown) return {};
    return m_perSystemBreakdown.divergentSystems;
}

std::vector<StateHashDiffPanel::HashLadderFrame> StateHashDiffPanel::BuildHashLadder(bool local) const {
    std::vector<HashLadderFrame> frames;
    for (const auto& e : m_entries) {
        HashLadderFrame f;
        f.tick = e.tick;
        f.hash = local ? e.localHash : e.remoteHash;
        f.divergent = !e.matches;
        frames.push_back(f);
    }
    return frames;
}

}
