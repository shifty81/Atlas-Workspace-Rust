#include "VisualDiff.h"
#include <unordered_map>
#include <sstream>

namespace atlas::editor {

DiffResult VisualDiff::Compare(
    const std::vector<std::pair<std::string, std::string>>& before,
    const std::vector<std::pair<std::string, std::string>>& after)
{
    DiffResult result;

    std::unordered_map<std::string, std::string> beforeMap;
    for (const auto& [key, value] : before) {
        beforeMap[key] = value;
    }

    std::unordered_map<std::string, std::string> afterMap;
    for (const auto& [key, value] : after) {
        afterMap[key] = value;
    }

    // Check for modified and removed
    for (const auto& [key, oldVal] : beforeMap) {
        auto it = afterMap.find(key);
        if (it == afterMap.end()) {
            DiffEntry entry;
            entry.type = DiffEntryType::Removed;
            entry.key = key;
            entry.oldValue = oldVal;
            result.entries.push_back(std::move(entry));
            result.removedCount++;
        } else if (it->second != oldVal) {
            DiffEntry entry;
            entry.type = DiffEntryType::Modified;
            entry.key = key;
            entry.oldValue = oldVal;
            entry.newValue = it->second;
            result.entries.push_back(std::move(entry));
            result.modifiedCount++;
        } else {
            DiffEntry entry;
            entry.type = DiffEntryType::Unchanged;
            entry.key = key;
            entry.oldValue = oldVal;
            entry.newValue = oldVal;
            result.entries.push_back(std::move(entry));
            result.unchangedCount++;
        }
    }

    // Check for added
    for (const auto& [key, newVal] : afterMap) {
        if (beforeMap.find(key) == beforeMap.end()) {
            DiffEntry entry;
            entry.type = DiffEntryType::Added;
            entry.key = key;
            entry.newValue = newVal;
            result.entries.push_back(std::move(entry));
            result.addedCount++;
        }
    }

    return result;
}

DiffResult VisualDiff::CompareGraphs(const GraphSnapshot& before, const GraphSnapshot& after) {
    return Compare(before.nodeProperties, after.nodeProperties);
}

std::string VisualDiff::Summarize(const DiffResult& result) {
    std::ostringstream ss;
    ss << result.addedCount << " added, "
       << result.removedCount << " removed, "
       << result.modifiedCount << " modified, "
       << result.unchangedCount << " unchanged";
    return ss.str();
}

bool VisualDiff::HasChanges(const DiffResult& result) {
    return result.addedCount > 0 || result.removedCount > 0 || result.modifiedCount > 0;
}

}
