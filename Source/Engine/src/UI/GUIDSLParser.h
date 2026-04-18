#pragma once
#include <string>
#include <vector>
#include <memory>
#include <functional>
#include <cstdint>

namespace atlas::ui {

enum class DSLNodeType : uint8_t {
    Layout,
    Split,
    Panel,
    Dock,
    Tabs
};

enum class DSLSplitDir : uint8_t {
    Horizontal,
    Vertical
};

enum class DSLDockTarget : uint8_t {
    Top,
    Bottom,
    Left,
    Right,
    Center,
    Float
};

struct DSLNode {
    DSLNodeType type = DSLNodeType::Panel;
    std::string name;
    DSLSplitDir splitDir = DSLSplitDir::Horizontal;
    DSLDockTarget dockTarget = DSLDockTarget::Bottom;
    float ratio = 0.5f;
    std::vector<std::unique_ptr<DSLNode>> children;
};

struct DSLParseResult {
    bool success = false;
    std::string errorMessage;
    int errorLine = -1;
    std::unique_ptr<DSLNode> root;
};

class GUIDSLParser {
public:
    DSLParseResult Parse(const std::string& source);
    static uint32_t Version() { return 1; }

private:
    struct Token {
        enum Type { Identifier, String, Number, LBrace, RBrace, Eof, Error };
        Type type = Eof;
        std::string value;
        int line = 1;
    };

    std::string m_source;
    size_t m_pos = 0;
    int m_line = 1;

    Token NextToken();
    Token PeekToken();
    void SkipWhitespace();

    std::unique_ptr<DSLNode> ParseLayout();
    std::unique_ptr<DSLNode> ParseStatement();
    std::unique_ptr<DSLNode> ParseSplit();
    std::unique_ptr<DSLNode> ParsePanel();
    std::unique_ptr<DSLNode> ParseDock();
    std::unique_ptr<DSLNode> ParseTabs();
    std::vector<std::unique_ptr<DSLNode>> ParseBlock();
};

} // namespace atlas::ui
