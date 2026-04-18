#pragma once
#include <string>
#include <vector>
#include "GameGUIAsset.h"

namespace atlas::ui {

struct WidgetDSLToken {
    enum Type { Keyword, Identifier, Number, String, OpenBrace, CloseBrace, Semicolon, Colon, Equals, Eof, Error };
    Type type = Eof;
    std::string value;
    int line = 1;
};

class WidgetDSLParser {
public:
    GameGUIAsset Parse(const std::string& source);
    std::vector<WidgetDSLToken> Tokenize(const std::string& source);
    bool Validate(const std::string& source, std::vector<std::string>& errors);

private:
    std::string m_source;
    size_t m_pos = 0;
    int m_line = 1;
    std::vector<std::string> m_errors;

    WidgetDSLToken NextToken();
    void SkipWhitespace();

    void ParseWidget(GameGUIAsset& asset, uint64_t parentID);
    WidgetType TypeFromKeyword(const std::string& kw) const;
    bool IsWidgetKeyword(const std::string& kw) const;
};

} // namespace atlas::ui
