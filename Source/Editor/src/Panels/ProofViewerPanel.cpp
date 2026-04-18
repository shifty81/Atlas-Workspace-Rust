#include "ProofViewerPanel.h"
#include <fstream>
#include <sstream>
#include <regex>
#include <algorithm>
#include <cstdio>

#if defined(_WIN32)
#include <windows.h>
#ifdef DrawText
#undef DrawText
#endif
#else
#include <dirent.h>
#endif

namespace atlas::editor {

// ---------------------------------------------------------------------------
// Construction
// ---------------------------------------------------------------------------

ProofViewerPanel::ProofViewerPanel() = default;

void ProofViewerPanel::Draw() {
    m_drawList.Clear();

    // Background
    m_drawList.DrawRect({0, 0, 700, 500}, {30, 30, 30, 255});

    // Title
    m_drawList.DrawRect({0, 0, 700, 24}, {50, 50, 50, 255});
    m_drawList.DrawText({4, 4, 200, 20}, "Proof Viewer", {220, 220, 220, 255});

    // Spec list (sidebar)
    int32_t y = 28;
    for (const auto& [name, spec] : m_specs) {
        bool selected = (name == m_selectedSpec);
        atlas::ui::UIColor bg = selected
            ? atlas::ui::UIColor{60, 80, 120, 255}
            : atlas::ui::UIColor{40, 40, 40, 255};
        m_drawList.DrawRect({0, y, 180, 18}, bg);

        auto vit = m_verification.find(name);
        atlas::ui::UIColor textColor = {200, 200, 200, 255};
        if (vit != m_verification.end()) {
            if (vit->second.status == VerificationStatus::Passed)
                textColor = {100, 255, 100, 255};
            else if (vit->second.status == VerificationStatus::Failed)
                textColor = {255, 80, 80, 255};
            else if (vit->second.status == VerificationStatus::Running)
                textColor = {255, 200, 100, 255};
        }
        m_drawList.DrawText({4, y + 1, 172, 16}, name, textColor);
        y += 20;
    }

    // Selected spec content (syntax highlighted tokens)
    if (!m_selectedSpec.empty()) {
        const auto& tokens = SelectedTokens();
        int32_t tx = 190;
        int32_t ty = 28;
        for (const auto& tok : tokens) {
            atlas::ui::UIColor tokColor = {200, 200, 200, 255};
            switch (tok.type) {
                case TokenType::Keyword:    tokColor = {180, 120, 255, 255}; break;
                case TokenType::Comment:    tokColor = {100, 160, 100, 255}; break;
                case TokenType::String:     tokColor = {200, 160, 100, 255}; break;
                case TokenType::Number:     tokColor = {100, 200, 200, 255}; break;
                case TokenType::Operator:   tokColor = {220, 220, 180, 255}; break;
                default: break;
            }
            m_drawList.DrawText({tx, ty, 500, 14}, tok.text, tokColor);
            ty += 16;
        }
    }
}

// ---------------------------------------------------------------------------
// Directory scanning
// ---------------------------------------------------------------------------

int ProofViewerPanel::LoadSpecsFromDirectory(const std::string& directory) {
    int count = 0;

#if defined(_WIN32)
    WIN32_FIND_DATAA fd;
    std::string pattern = directory + "/*.tla";
    HANDLE hFind = FindFirstFileA(pattern.c_str(), &fd);
    if (hFind == INVALID_HANDLE_VALUE) return 0;
    do {
        std::string fname = fd.cFileName;
#else
    DIR* dir = opendir(directory.c_str());
    if (!dir) return 0;
    struct dirent* entry;
    while ((entry = readdir(dir)) != nullptr) {
        std::string fname = entry->d_name;
#endif
        if (fname.size() < 5 || fname.substr(fname.size() - 4) != ".tla")
            continue;

        std::string fullPath = directory + "/" + fname;
        std::ifstream file(fullPath);
        if (!file.is_open()) continue;

        std::ostringstream buf;
        buf << file.rdbuf();
        std::string content = buf.str();
        if (content.empty()) continue;

        std::string specName = fname.substr(0, fname.size() - 4);

        ProofSpec spec;
        spec.name = specName;
        spec.path = fullPath;
        spec.content = content;
        spec.tokens = Tokenize(content);
        spec.invariants = ExtractInvariants(content);
        spec.loaded = true;

        m_specs[specName] = std::move(spec);
        ++count;

#if defined(_WIN32)
    } while (FindNextFileA(hFind, &fd));
    FindClose(hFind);
#else
    }
    closedir(dir);
#endif

    return count;
}

// ---------------------------------------------------------------------------
// Queries
// ---------------------------------------------------------------------------

size_t ProofViewerPanel::SpecCount() const {
    return m_specs.size();
}

std::vector<std::string> ProofViewerPanel::SpecNames() const {
    std::vector<std::string> names;
    names.reserve(m_specs.size());
    for (const auto& kv : m_specs)
        names.push_back(kv.first);
    return names;
}

const ProofSpec* ProofViewerPanel::GetSpec(const std::string& name) const {
    auto it = m_specs.find(name);
    if (it == m_specs.end()) return nullptr;
    return &it->second;
}

void ProofViewerPanel::SelectSpec(const std::string& name) {
    if (m_specs.count(name))
        m_selectedSpec = name;
}

const std::string& ProofViewerPanel::SelectedSpec() const {
    return m_selectedSpec;
}

const std::vector<SyntaxToken>& ProofViewerPanel::SelectedTokens() const {
    auto it = m_specs.find(m_selectedSpec);
    if (it == m_specs.end()) return m_emptyTokens;
    return it->second.tokens;
}

std::vector<std::string> ProofViewerPanel::SelectedInvariants() const {
    auto it = m_specs.find(m_selectedSpec);
    if (it == m_specs.end()) return {};
    return it->second.invariants;
}

// ---------------------------------------------------------------------------
// Verification status
// ---------------------------------------------------------------------------

void ProofViewerPanel::SetVerificationStatus(const std::string& specName,
                                             VerificationStatus status,
                                             int states, double elapsed,
                                             const std::string& detail) {
    SpecVerification v;
    v.status = status;
    v.statesExplored = states;
    v.elapsed = elapsed;
    v.detail = detail;
    m_verification[specName] = v;
}

SpecVerification ProofViewerPanel::GetVerificationStatus(
        const std::string& specName) const {
    auto it = m_verification.find(specName);
    if (it == m_verification.end()) return {};
    return it->second;
}

// ---------------------------------------------------------------------------
// Line count
// ---------------------------------------------------------------------------

int ProofViewerPanel::LineCount(const std::string& specName) const {
    auto it = m_specs.find(specName);
    if (it == m_specs.end()) return 0;
    const auto& content = it->second.content;
    if (content.empty()) return 0;
    return static_cast<int>(std::count(content.begin(), content.end(), '\n') + 1);
}

// ---------------------------------------------------------------------------
// Invariant extraction (mirrors TLCModelChecker::ExtractInvariants)
// ---------------------------------------------------------------------------

std::vector<std::string> ProofViewerPanel::ExtractInvariants(
        const std::string& content) {
    std::vector<std::string> invariants;
    std::regex defPattern(R"(^[ \t]*([A-Z][A-Za-z0-9_]*)\s*==\s*)");
    std::istringstream stream(content);
    std::string line;

    static const std::vector<std::string> skipNames = {
        "Init", "Next", "Spec", "TypeOK", "Invariant",
        "NullNode", "HashCombine"
    };

    static const std::vector<std::string> actionPrefixes = {
        "Start", "Record", "Stop", "Run", "End", "Begin",
        "Mark", "Add", "Remove", "Register", "Destroy",
        "Create", "Split", "Merge", "Resize", "Playback"
    };

    while (std::getline(stream, line)) {
        std::smatch match;
        if (!std::regex_search(line, match, defPattern)) continue;
        std::string name = match[1].str();

        bool skip = false;
        for (const auto& s : skipNames) {
            if (name == s) { skip = true; break; }
        }
        for (const auto& prefix : actionPrefixes) {
            if (name.size() >= prefix.size() &&
                name.substr(0, prefix.size()) == prefix) {
                skip = true;
                break;
            }
        }
        if (name == "LeftSize" || name == "RightSize" ||
            name == "AllNodeIDs" || name == "IsLeaf" ||
            name == "IsSplit") {
            skip = true;
        }

        if (!skip)
            invariants.push_back(name);
    }
    return invariants;
}

// ---------------------------------------------------------------------------
// TLA+ tokenizer
// ---------------------------------------------------------------------------

static bool isIdentStart(char c) {
    return (c >= 'A' && c <= 'Z') || (c >= 'a' && c <= 'z') || c == '_';
}

static bool isIdentChar(char c) {
    return isIdentStart(c) || (c >= '0' && c <= '9');
}

static bool isDigit(char c) {
    return c >= '0' && c <= '9';
}

static const std::vector<std::string>& TLAKeywords() {
    static const std::vector<std::string> kw = {
        "MODULE", "EXTENDS", "VARIABLE", "VARIABLES",
        "CONSTANT", "CONSTANTS", "ASSUME", "THEOREM",
        "LEMMA", "INSTANCE", "LOCAL", "LET", "IN",
        "IF", "THEN", "ELSE", "CASE", "OTHER",
        "CHOOSE", "WITH", "EXCEPT", "UNCHANGED",
        "DOMAIN", "SUBSET", "UNION", "ENABLED"
    };
    return kw;
}

std::vector<SyntaxToken> ProofViewerPanel::Tokenize(const std::string& content) {
    std::vector<SyntaxToken> tokens;
    const size_t len = content.size();
    size_t pos = 0;
    int line = 1;
    int col = 1;

    auto push = [&](TokenType type, const std::string& text, int l, int c) {
        SyntaxToken tok;
        tok.type = type;
        tok.text = text;
        tok.line = l;
        tok.column = c;
        tokens.push_back(std::move(tok));
    };

    while (pos < len) {
        char ch = content[pos];

        // Block comment: (* ... *)
        if (ch == '(' && pos + 1 < len && content[pos + 1] == '*') {
            int startLine = line, startCol = col;
            std::string text;
            text += content[pos++]; col++;
            text += content[pos++]; col++;
            while (pos < len) {
                if (content[pos] == '*' && pos + 1 < len && content[pos + 1] == ')') {
                    text += content[pos++]; col++;
                    text += content[pos++]; col++;
                    break;
                }
                if (content[pos] == '\n') { line++; col = 1; }
                else { col++; }
                text += content[pos++];
            }
            push(TokenType::Comment, text, startLine, startCol);
            continue;
        }

        // Line comment: \* ...
        if (ch == '\\' && pos + 1 < len && content[pos + 1] == '*') {
            int startLine = line, startCol = col;
            std::string text;
            while (pos < len && content[pos] != '\n') {
                text += content[pos++];
                col++;
            }
            push(TokenType::Comment, text, startLine, startCol);
            continue;
        }

        // Backslash operators: \\in, \\notin, etc.
        if (ch == '\\' && pos + 1 < len && content[pos + 1] == '\\') {
            int startLine = line, startCol = col;
            std::string text;
            text += content[pos++]; col++;
            text += content[pos++]; col++;
            while (pos < len && isIdentChar(content[pos])) {
                text += content[pos++]; col++;
            }
            push(TokenType::Operator, text, startLine, startCol);
            continue;
        }

        // String literal
        if (ch == '"') {
            int startLine = line, startCol = col;
            std::string text;
            text += content[pos++]; col++;
            while (pos < len && content[pos] != '"') {
                if (content[pos] == '\\' && pos + 1 < len) {
                    text += content[pos++]; col++;
                }
                text += content[pos++]; col++;
            }
            if (pos < len) { text += content[pos++]; col++; }
            push(TokenType::String, text, startLine, startCol);
            continue;
        }

        // Number
        if (isDigit(ch)) {
            int startLine = line, startCol = col;
            std::string text;
            while (pos < len && isDigit(content[pos])) {
                text += content[pos++]; col++;
            }
            push(TokenType::Number, text, startLine, startCol);
            continue;
        }

        // Identifier or keyword
        if (isIdentStart(ch)) {
            int startLine = line, startCol = col;
            std::string text;
            while (pos < len && isIdentChar(content[pos])) {
                text += content[pos++]; col++;
            }
            const auto& kws = TLAKeywords();
            bool isKw = std::find(kws.begin(), kws.end(), text) != kws.end();
            push(isKw ? TokenType::Keyword : TokenType::Identifier,
                 text, startLine, startCol);
            continue;
        }

        // Whitespace
        if (ch == ' ' || ch == '\t' || ch == '\n' || ch == '\r') {
            int startLine = line, startCol = col;
            std::string text;
            while (pos < len && (content[pos] == ' ' || content[pos] == '\t' ||
                                 content[pos] == '\n' || content[pos] == '\r')) {
                if (content[pos] == '\n') { line++; col = 1; text += content[pos++]; }
                else { text += content[pos++]; col++; }
            }
            push(TokenType::Whitespace, text, startLine, startCol);
            continue;
        }

        // Punctuation
        if (ch == '(' || ch == ')' || ch == '{' || ch == '}' ||
            ch == '[' || ch == ']' || ch == ',') {
            push(TokenType::Punctuation, std::string(1, ch), line, col);
            pos++; col++;
            continue;
        }

        // Multi-char operators
        if (pos + 1 < len) {
            std::string two(1, ch);
            two += content[pos + 1];
            if (two == "==" || two == "/\\" || two == "\\/" ||
                two == "=>" || two == "<>" || two == ">=" ||
                two == "<=" || two == ":=" || two == "..") {
                push(TokenType::Operator, two, line, col);
                pos += 2; col += 2;
                continue;
            }
        }

        // Single-char operators
        if (ch == '~' || ch == '#' || ch == '\'' || ch == '-' ||
            ch == '+' || ch == '*' || ch == '/' || ch == '=' ||
            ch == '<' || ch == '>') {
            push(TokenType::Operator, std::string(1, ch), line, col);
            pos++; col++;
            continue;
        }

        // Dashes (---- MODULE ----) and other plain characters
        push(TokenType::Plain, std::string(1, ch), line, col);
        pos++; col++;
    }

    return tokens;
}

}  // namespace atlas::editor
