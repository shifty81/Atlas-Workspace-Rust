#include <iostream>
#include <cassert>
#include <string>
#include <vector>
#include <algorithm>

#include "../editor/panels/ProofViewerPanel.h"

static std::string specsDir() {
#ifdef CMAKE_SOURCE_DIR
    return std::string(CMAKE_SOURCE_DIR) + "/specs";
#else
    return "specs";
#endif
}

void test_proof_viewer_init() {
    atlas::editor::ProofViewerPanel panel;
    assert(std::string(panel.Name()) == "ProofViewer");
    assert(panel.SpecCount() == 0);
    std::cout << "[PASS] test_proof_viewer_init" << std::endl;
}

void test_proof_viewer_load_specs() {
    atlas::editor::ProofViewerPanel panel;
    int loaded = panel.LoadSpecsFromDirectory(specsDir());
    assert(loaded >= 3);
    assert(panel.SpecCount() >= 3);
    std::cout << "[PASS] test_proof_viewer_load_specs" << std::endl;
}

void test_proof_viewer_select_spec() {
    atlas::editor::ProofViewerPanel panel;
    panel.LoadSpecsFromDirectory(specsDir());
    panel.SelectSpec("ecs");
    assert(panel.SelectedSpec() == "ecs");
    std::cout << "[PASS] test_proof_viewer_select_spec" << std::endl;
}

void test_proof_viewer_tokenize() {
    std::string snippet = "VARIABLE x\nInit == x = 0\n";
    auto tokens = atlas::editor::ProofViewerPanel::Tokenize(snippet);
    assert(!tokens.empty());

    bool hasKeyword = false;
    bool hasIdentifier = false;
    bool hasOperator = false;
    bool hasNumber = false;
    for (const auto& t : tokens) {
        if (t.type == atlas::editor::TokenType::Keyword) hasKeyword = true;
        if (t.type == atlas::editor::TokenType::Identifier) hasIdentifier = true;
        if (t.type == atlas::editor::TokenType::Operator) hasOperator = true;
        if (t.type == atlas::editor::TokenType::Number) hasNumber = true;
    }
    assert(hasKeyword);
    assert(hasIdentifier);
    assert(hasOperator);
    assert(hasNumber);
    std::cout << "[PASS] test_proof_viewer_tokenize" << std::endl;
}

void test_proof_viewer_invariants() {
    atlas::editor::ProofViewerPanel panel;
    panel.LoadSpecsFromDirectory(specsDir());
    panel.SelectSpec("ecs");
    auto inv = panel.SelectedInvariants();
    assert(!inv.empty());
    assert(std::find(inv.begin(), inv.end(), "UniqueIDs") != inv.end());
    std::cout << "[PASS] test_proof_viewer_invariants" << std::endl;
}

void test_proof_viewer_verification_status() {
    atlas::editor::ProofViewerPanel panel;
    panel.SetVerificationStatus("ecs", atlas::editor::VerificationStatus::Passed, 42, 1.5, "ok");
    auto v = panel.GetVerificationStatus("ecs");
    assert(v.status == atlas::editor::VerificationStatus::Passed);
    assert(v.statesExplored == 42);
    assert(v.elapsed == 1.5);
    assert(v.detail == "ok");

    auto unknown = panel.GetVerificationStatus("missing");
    assert(unknown.status == atlas::editor::VerificationStatus::Unknown);
    std::cout << "[PASS] test_proof_viewer_verification_status" << std::endl;
}

void test_proof_viewer_line_count() {
    atlas::editor::ProofViewerPanel panel;
    panel.LoadSpecsFromDirectory(specsDir());
    int lines = panel.LineCount("ecs");
    assert(lines > 0);

    assert(panel.LineCount("nonexistent") == 0);
    std::cout << "[PASS] test_proof_viewer_line_count" << std::endl;
}

void test_proof_viewer_empty_select() {
    atlas::editor::ProofViewerPanel panel;
    panel.LoadSpecsFromDirectory(specsDir());
    panel.SelectSpec("nonexistent_spec_xyz");
    const auto& tokens = panel.SelectedTokens();
    assert(tokens.empty());
    std::cout << "[PASS] test_proof_viewer_empty_select" << std::endl;
}
