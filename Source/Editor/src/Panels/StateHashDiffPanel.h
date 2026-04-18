#pragma once
#include "../ui/EditorPanel.h"
#include "../../engine/sim/StateHasher.h"
#include "../../engine/ui/UIDrawList.h"
#include <string>
#include <vector>

namespace atlas::editor {

struct HashDiffEntry {
    uint64_t tick = 0;
    uint64_t localHash = 0;
    uint64_t remoteHash = 0;
    bool matches = true;
};

struct ComponentHashEntry {
    std::string name;
    uint64_t localHash = 0;
    uint64_t remoteHash = 0;
    bool matches = true;
};

struct ComponentHashBreakdown {
    uint64_t tick = 0;
    std::vector<ComponentHashEntry> components;
};

/// Result of comparing two complete hash ladders side-by-side.
struct HashLadderComparison {
    std::vector<HashDiffEntry> entries;
    int64_t firstDivergenceTick = -1;
    size_t matchCount = 0;
    size_t totalCount = 0;
    double matchPercentage = 100.0;
};

/// Detailed information about the first divergence point.
struct DivergenceDetail {
    int64_t tick = -1;
    uint64_t localHash = 0;
    uint64_t remoteHash = 0;
    std::vector<ComponentHashEntry> divergentComponents;
    std::string summary;
};

struct SystemHashEntry {
    std::string systemName;
    uint64_t hash = 0;
};

struct PerSystemHashBreakdown {
    uint64_t tick = 0;
    std::vector<SystemHashEntry> localSystems;
    std::vector<SystemHashEntry> remoteSystems;
    std::vector<std::string> divergentSystems;
};

class StateHashDiffPanel : public EditorPanel {
public:
    const char* Name() const override { return "State Hash Diff"; }
    void Draw() override;

    void SetLocalHasher(const sim::StateHasher* local);
    void SetRemoteHasher(const sim::StateHasher* remote);
    void Refresh();

    const std::vector<HashDiffEntry>& Entries() const;
    int64_t FirstDivergenceTick() const;
    bool HasDivergence() const;
    std::string Summary() const;

    // Per-component hash breakdown at a specific tick
    void SetComponentBreakdown(const ComponentHashBreakdown& breakdown);
    const ComponentHashBreakdown& GetComponentBreakdown() const;
    bool HasComponentBreakdown() const;
    std::vector<std::string> DivergentComponents() const;

    /// Compare two hash ladders side-by-side and return full results.
    HashLadderComparison CompareHashLadders(const sim::StateHasher& a,
                                            const sim::StateHasher& b) const;

    /// Get detailed information about the first divergence, including which components diverge.
    DivergenceDetail GetDivergenceDetail() const;

    // Per-system hash breakdown
    void SetPerSystemBreakdown(const PerSystemHashBreakdown& breakdown);
    const PerSystemHashBreakdown& GetPerSystemBreakdown() const;
    bool HasPerSystemBreakdown() const;
    std::vector<std::string> DivergentSystems() const;

    // Frame-by-frame hash ladder visualization data
    struct HashLadderFrame {
        uint64_t tick = 0;
        uint64_t hash = 0;
        bool divergent = false;
    };
    std::vector<HashLadderFrame> BuildHashLadder(bool local = true) const;

    const atlas::ui::UIDrawList& GetDrawList() const { return m_drawList; }

private:
    const sim::StateHasher* m_local = nullptr;
    const sim::StateHasher* m_remote = nullptr;
    std::vector<HashDiffEntry> m_entries;
    int64_t m_firstDivergence = -1;
    ComponentHashBreakdown m_componentBreakdown;
    bool m_hasComponentBreakdown = false;
    PerSystemHashBreakdown m_perSystemBreakdown;
    bool m_hasPerSystemBreakdown = false;
    atlas::ui::UIDrawList m_drawList;
};

}
