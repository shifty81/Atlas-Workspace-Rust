#include "WidgetDSL.h"
#include <cctype>
#include <algorithm>

namespace atlas::ui {

GameGUIAsset WidgetDSLParser::Parse(const std::string& source) {
    GameGUIAsset asset;
    m_source = source;
    m_pos = 0;
    m_line = 1;
    m_errors.clear();

    while (m_pos < m_source.size()) {
        SkipWhitespace();
        if (m_pos >= m_source.size()) break;
        WidgetDSLToken tok = NextToken();
        if (tok.type == WidgetDSLToken::Eof) break;
        if (tok.type == WidgetDSLToken::Keyword || tok.type == WidgetDSLToken::Identifier) {
            if (IsWidgetKeyword(tok.value)) {
                // Read widget name
                WidgetDSLToken nameTok = NextToken();
                std::string widgetName;
                if (nameTok.type == WidgetDSLToken::String || nameTok.type == WidgetDSLToken::Identifier) {
                    widgetName = nameTok.value;
                }
                uint64_t id = asset.AddWidget(TypeFromKeyword(tok.value), widgetName);
                // Check for brace block
                SkipWhitespace();
                if (m_pos < m_source.size() && m_source[m_pos] == '{') {
                    m_pos++; // consume '{'
                    // Parse properties and children
                    while (m_pos < m_source.size()) {
                        SkipWhitespace();
                        if (m_pos >= m_source.size()) break;
                        if (m_source[m_pos] == '}') { m_pos++; break; }

                        WidgetDSLToken inner = NextToken();
                        if (inner.type == WidgetDSLToken::Eof) break;
                        if (IsWidgetKeyword(inner.value)) {
                            // Nested widget
                            WidgetDSLToken childName = NextToken();
                            std::string cName;
                            if (childName.type == WidgetDSLToken::String || childName.type == WidgetDSLToken::Identifier) {
                                cName = childName.value;
                            }
                            uint64_t childID = asset.AddChild(id, TypeFromKeyword(inner.value), cName);
                            SkipWhitespace();
                            if (m_pos < m_source.size() && m_source[m_pos] == '{') {
                                m_pos++;
                                // Parse child properties
                                while (m_pos < m_source.size()) {
                                    SkipWhitespace();
                                    if (m_pos >= m_source.size()) break;
                                    if (m_source[m_pos] == '}') { m_pos++; break; }

                                    WidgetDSLToken propKey = NextToken();
                                    if (propKey.type == WidgetDSLToken::Eof) break;
                                    SkipWhitespace();
                                    if (m_pos < m_source.size() && m_source[m_pos] == ':') {
                                        m_pos++; // consume ':'
                                        SkipWhitespace();
                                        WidgetDSLToken propVal = NextToken();
                                        asset.SetProperty(childID, propKey.value, propVal.value);
                                        SkipWhitespace();
                                        if (m_pos < m_source.size() && m_source[m_pos] == ';') m_pos++;
                                    }
                                }
                            }
                        } else {
                            // Property: key : value ;
                            SkipWhitespace();
                            if (m_pos < m_source.size() && m_source[m_pos] == ':') {
                                m_pos++;
                                SkipWhitespace();
                                WidgetDSLToken propVal = NextToken();
                                asset.SetProperty(id, inner.value, propVal.value);
                                SkipWhitespace();
                                if (m_pos < m_source.size() && m_source[m_pos] == ';') m_pos++;
                            }
                        }
                    }
                }
            }
        }
    }
    return asset;
}

std::vector<WidgetDSLToken> WidgetDSLParser::Tokenize(const std::string& source) {
    std::vector<WidgetDSLToken> tokens;
    m_source = source;
    m_pos = 0;
    m_line = 1;

    while (m_pos < m_source.size()) {
        WidgetDSLToken tok = NextToken();
        if (tok.type == WidgetDSLToken::Eof) break;
        tokens.push_back(tok);
    }
    return tokens;
}

bool WidgetDSLParser::Validate(const std::string& source, std::vector<std::string>& errors) {
    errors.clear();
    if (source.empty()) {
        return true; // empty is valid
    }
    m_source = source;
    m_pos = 0;
    m_line = 1;
    m_errors.clear();

    int braceDepth = 0;
    bool hasWidget = false;

    while (m_pos < m_source.size()) {
        WidgetDSLToken tok = NextToken();
        if (tok.type == WidgetDSLToken::Eof) break;
        if (tok.type == WidgetDSLToken::Error) {
            errors.push_back("Error at line " + std::to_string(tok.line) + ": " + tok.value);
            return false;
        }
        if (tok.type == WidgetDSLToken::OpenBrace) braceDepth++;
        if (tok.type == WidgetDSLToken::CloseBrace) braceDepth--;
        if (braceDepth < 0) {
            errors.push_back("Unexpected '}' at line " + std::to_string(tok.line));
            return false;
        }
        if (IsWidgetKeyword(tok.value)) hasWidget = true;
    }
    if (braceDepth != 0) {
        errors.push_back("Unmatched braces");
        return false;
    }
    if (!hasWidget && !source.empty()) {
        // Check if there's any real content
        bool hasContent = false;
        for (char c : source) {
            if (!std::isspace(static_cast<unsigned char>(c))) { hasContent = true; break; }
        }
        if (hasContent) {
            errors.push_back("No widget definitions found");
            return false;
        }
    }
    return true;
}

WidgetDSLToken WidgetDSLParser::NextToken() {
    SkipWhitespace();
    if (m_pos >= m_source.size()) return {WidgetDSLToken::Eof, "", m_line};

    char c = m_source[m_pos];
    if (c == '{') { m_pos++; return {WidgetDSLToken::OpenBrace, "{", m_line}; }
    if (c == '}') { m_pos++; return {WidgetDSLToken::CloseBrace, "}", m_line}; }
    if (c == ';') { m_pos++; return {WidgetDSLToken::Semicolon, ";", m_line}; }
    if (c == ':') { m_pos++; return {WidgetDSLToken::Colon, ":", m_line}; }
    if (c == '=') { m_pos++; return {WidgetDSLToken::Equals, "=", m_line}; }

    if (c == '"') {
        m_pos++;
        std::string val;
        while (m_pos < m_source.size() && m_source[m_pos] != '"') {
            val += m_source[m_pos++];
        }
        if (m_pos < m_source.size()) m_pos++; // closing quote
        return {WidgetDSLToken::String, val, m_line};
    }

    if (std::isdigit(static_cast<unsigned char>(c)) || c == '-') {
        std::string val;
        val += c; m_pos++;
        while (m_pos < m_source.size() && (std::isdigit(static_cast<unsigned char>(m_source[m_pos])) || m_source[m_pos] == '.')) {
            val += m_source[m_pos++];
        }
        return {WidgetDSLToken::Number, val, m_line};
    }

    if (std::isalpha(static_cast<unsigned char>(c)) || c == '_') {
        std::string val;
        while (m_pos < m_source.size() && (std::isalnum(static_cast<unsigned char>(m_source[m_pos])) || m_source[m_pos] == '_')) {
            val += m_source[m_pos++];
        }
        if (IsWidgetKeyword(val)) return {WidgetDSLToken::Keyword, val, m_line};
        return {WidgetDSLToken::Identifier, val, m_line};
    }

    m_pos++;
    return {WidgetDSLToken::Error, std::string(1, c), m_line};
}

void WidgetDSLParser::SkipWhitespace() {
    while (m_pos < m_source.size()) {
        char c = m_source[m_pos];
        if (c == '\n') { m_line++; m_pos++; }
        else if (std::isspace(static_cast<unsigned char>(c))) { m_pos++; }
        else break;
    }
}

WidgetType WidgetDSLParser::TypeFromKeyword(const std::string& kw) const {
    if (kw == "label") return WidgetType::Label;
    if (kw == "button") return WidgetType::Button;
    if (kw == "panel") return WidgetType::Panel;
    if (kw == "image") return WidgetType::Image;
    if (kw == "progressbar") return WidgetType::ProgressBar;
    if (kw == "list") return WidgetType::List;
    if (kw == "grid") return WidgetType::Grid;
    if (kw == "slider") return WidgetType::Slider;
    if (kw == "toggle") return WidgetType::Toggle;
    if (kw == "textinput") return WidgetType::TextInput;
    return WidgetType::Panel;
}

bool WidgetDSLParser::IsWidgetKeyword(const std::string& kw) const {
    static const std::vector<std::string> keywords = {
        "label", "button", "panel", "image", "progressbar",
        "list", "grid", "slider", "toggle", "textinput"
    };
    return std::find(keywords.begin(), keywords.end(), kw) != keywords.end();
}

} // namespace atlas::ui
