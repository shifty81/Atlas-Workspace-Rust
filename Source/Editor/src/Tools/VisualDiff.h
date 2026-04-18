#pragma once
#include <string>
#include <vector>
#include <cstdint>

namespace atlas::editor {

enum class DiffEntryType {
    Added,
    Removed,
    Modified,
    Unchanged,
};

struct DiffEntry {
    DiffEntryType type = DiffEntryType::Unchanged;
    std::string key;
    std::string oldValue;
    std::string newValue;
};

struct DiffResult {
    std::vector<DiffEntry> entries;
    uint32_t addedCount = 0;
    uint32_t removedCount = 0;
    uint32_t modifiedCount = 0;
    uint32_t unchangedCount = 0;
};

struct GraphSnapshot {
    std::string graphId;
    std::vector<std::pair<std::string, std::string>> nodeProperties;
};

class VisualDiff {
public:
    // Compare two key-value snapshots
    static DiffResult Compare(
        const std::vector<std::pair<std::string, std::string>>& before,
        const std::vector<std::pair<std::string, std::string>>& after);

    // Compare two graph snapshots
    static DiffResult CompareGraphs(const GraphSnapshot& before, const GraphSnapshot& after);

    // Summary of diff as string
    static std::string Summarize(const DiffResult& result);

    // Check if there are any changes
    static bool HasChanges(const DiffResult& result);
};

}
