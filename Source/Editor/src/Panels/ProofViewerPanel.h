#pragma once
#include "../ui/EditorPanel.h"
#include "../../engine/ui/UIDrawList.h"
#include <string>
#include <vector>
#include <map>

namespace atlas::editor {

enum class TokenType {
    Keyword,
    Operator,
    Comment,
    String,
    Number,
    Identifier,
    Punctuation,
    Whitespace,
    Plain
};

struct SyntaxToken {
    TokenType type = TokenType::Plain;
    std::string text;
    int line = 0;
    int column = 0;
};

struct ProofSpec {
    std::string name;
    std::string path;
    std::string content;
    std::vector<SyntaxToken> tokens;
    std::vector<std::string> invariants;
    bool loaded = false;
};

enum class VerificationStatus {
    Unknown,
    Passed,
    Failed,
    Running
};

struct SpecVerification {
    VerificationStatus status = VerificationStatus::Unknown;
    int statesExplored = 0;
    double elapsed = 0.0;
    std::string detail;
};

class ProofViewerPanel : public EditorPanel {
public:
    ProofViewerPanel();

    const char* Name() const override { return "ProofViewer"; }
    void Draw() override;

    int LoadSpecsFromDirectory(const std::string& directory);

    size_t SpecCount() const;
    std::vector<std::string> SpecNames() const;
    const ProofSpec* GetSpec(const std::string& name) const;

    void SelectSpec(const std::string& name);
    const std::string& SelectedSpec() const;

    const std::vector<SyntaxToken>& SelectedTokens() const;
    std::vector<std::string> SelectedInvariants() const;

    void SetVerificationStatus(const std::string& specName, VerificationStatus status,
                               int states = 0, double elapsed = 0.0,
                               const std::string& detail = "");
    SpecVerification GetVerificationStatus(const std::string& specName) const;

    static std::vector<SyntaxToken> Tokenize(const std::string& content);

    int LineCount(const std::string& specName) const;

    const atlas::ui::UIDrawList& GetDrawList() const { return m_drawList; }

private:
    static std::vector<std::string> ExtractInvariants(const std::string& content);

    std::map<std::string, ProofSpec> m_specs;
    std::map<std::string, SpecVerification> m_verification;
    std::string m_selectedSpec;
    std::vector<SyntaxToken> m_emptyTokens;
    atlas::ui::UIDrawList m_drawList;
};

}  // namespace atlas::editor
