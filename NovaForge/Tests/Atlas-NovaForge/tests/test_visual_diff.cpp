#include "../editor/tools/VisualDiff.h"
#include <iostream>
#include <cassert>

using namespace atlas::editor;

void test_diff_identical() {
    std::vector<std::pair<std::string, std::string>> before = {
        {"nodeA", "value1"},
        {"nodeB", "value2"},
    };

    DiffResult result = VisualDiff::Compare(before, before);
    assert(!VisualDiff::HasChanges(result));
    assert(result.unchangedCount == 2);
    assert(result.addedCount == 0);
    assert(result.removedCount == 0);
    assert(result.modifiedCount == 0);

    std::cout << "[PASS] test_diff_identical" << std::endl;
}

void test_diff_added() {
    std::vector<std::pair<std::string, std::string>> before = {
        {"nodeA", "value1"},
    };
    std::vector<std::pair<std::string, std::string>> after = {
        {"nodeA", "value1"},
        {"nodeB", "value2"},
    };

    DiffResult result = VisualDiff::Compare(before, after);
    assert(VisualDiff::HasChanges(result));
    assert(result.addedCount == 1);
    assert(result.unchangedCount == 1);

    std::cout << "[PASS] test_diff_added" << std::endl;
}

void test_diff_removed() {
    std::vector<std::pair<std::string, std::string>> before = {
        {"nodeA", "value1"},
        {"nodeB", "value2"},
    };
    std::vector<std::pair<std::string, std::string>> after = {
        {"nodeA", "value1"},
    };

    DiffResult result = VisualDiff::Compare(before, after);
    assert(VisualDiff::HasChanges(result));
    assert(result.removedCount == 1);
    assert(result.unchangedCount == 1);

    std::cout << "[PASS] test_diff_removed" << std::endl;
}

void test_diff_modified() {
    std::vector<std::pair<std::string, std::string>> before = {
        {"nodeA", "value1"},
        {"nodeB", "value2"},
    };
    std::vector<std::pair<std::string, std::string>> after = {
        {"nodeA", "value1"},
        {"nodeB", "CHANGED"},
    };

    DiffResult result = VisualDiff::Compare(before, after);
    assert(VisualDiff::HasChanges(result));
    assert(result.modifiedCount == 1);
    assert(result.unchangedCount == 1);

    // Find the modified entry
    bool foundModified = false;
    for (const auto& e : result.entries) {
        if (e.type == DiffEntryType::Modified) {
            assert(e.key == "nodeB");
            assert(e.oldValue == "value2");
            assert(e.newValue == "CHANGED");
            foundModified = true;
        }
    }
    assert(foundModified);

    std::cout << "[PASS] test_diff_modified" << std::endl;
}

void test_diff_empty() {
    std::vector<std::pair<std::string, std::string>> empty;
    DiffResult result = VisualDiff::Compare(empty, empty);
    assert(!VisualDiff::HasChanges(result));
    assert(result.entries.empty());

    std::cout << "[PASS] test_diff_empty" << std::endl;
}

void test_diff_compare_graphs() {
    GraphSnapshot before;
    before.graphId = "worldgen";
    before.nodeProperties = {{"noise", "perlin"}, {"scale", "1.0"}};

    GraphSnapshot after;
    after.graphId = "worldgen";
    after.nodeProperties = {{"noise", "fbm"}, {"scale", "1.0"}, {"octaves", "4"}};

    DiffResult result = VisualDiff::CompareGraphs(before, after);
    assert(VisualDiff::HasChanges(result));
    assert(result.modifiedCount == 1);
    assert(result.addedCount == 1);
    assert(result.unchangedCount == 1);

    std::cout << "[PASS] test_diff_compare_graphs" << std::endl;
}

void test_diff_summarize() {
    std::vector<std::pair<std::string, std::string>> before = {{"a", "1"}};
    std::vector<std::pair<std::string, std::string>> after = {{"a", "2"}, {"b", "3"}};

    DiffResult result = VisualDiff::Compare(before, after);
    std::string summary = VisualDiff::Summarize(result);
    assert(!summary.empty());
    assert(summary.find("1 added") != std::string::npos);
    assert(summary.find("1 modified") != std::string::npos);

    std::cout << "[PASS] test_diff_summarize" << std::endl;
}
