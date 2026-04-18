#include "../engine/ui/WidgetDSL.h"
#include <iostream>
#include <cassert>

void test_widget_dsl_tokenize() {
    atlas::ui::WidgetDSLParser parser;
    auto tokens = parser.Tokenize("panel \"test\" { }");
    assert(!tokens.empty());
    assert(tokens[0].type == atlas::ui::WidgetDSLToken::Keyword);
    assert(tokens[0].value == "panel");
    assert(tokens[1].type == atlas::ui::WidgetDSLToken::String);
    assert(tokens[1].value == "test");
    std::cout << "[PASS] test_widget_dsl_tokenize" << std::endl;
}

void test_widget_dsl_parse_simple() {
    atlas::ui::WidgetDSLParser parser;
    auto asset = parser.Parse("panel \"inventory\" { }");
    assert(asset.WidgetCount() == 1);
    auto* root = asset.Root();
    assert(root != nullptr);
    assert(root->name == "inventory");
    assert(root->type == atlas::ui::WidgetType::Panel);
    std::cout << "[PASS] test_widget_dsl_parse_simple" << std::endl;
}

void test_widget_dsl_parse_nested() {
    atlas::ui::WidgetDSLParser parser;
    std::string src = R"(
        panel "main" {
            label "title" { text: "Hello"; }
            button "ok" { text: "OK"; }
        }
    )";
    auto asset = parser.Parse(src);
    assert(asset.WidgetCount() == 3);
    std::cout << "[PASS] test_widget_dsl_parse_nested" << std::endl;
}

void test_widget_dsl_parse_properties() {
    atlas::ui::WidgetDSLParser parser;
    std::string src = R"(
        panel "inv" {
            grid "items" { columns: 4; rows: 3; }
        }
    )";
    auto asset = parser.Parse(src);
    assert(asset.WidgetCount() >= 2);
    std::cout << "[PASS] test_widget_dsl_parse_properties" << std::endl;
}

void test_widget_dsl_validate_valid() {
    atlas::ui::WidgetDSLParser parser;
    std::vector<std::string> errors;
    bool valid = parser.Validate("panel \"test\" { }", errors);
    assert(valid);
    assert(errors.empty());
    std::cout << "[PASS] test_widget_dsl_validate_valid" << std::endl;
}

void test_widget_dsl_validate_invalid() {
    atlas::ui::WidgetDSLParser parser;
    std::vector<std::string> errors;
    bool valid = parser.Validate("panel \"test\" { ", errors);
    assert(!valid);
    assert(!errors.empty());
    std::cout << "[PASS] test_widget_dsl_validate_invalid" << std::endl;
}

void test_widget_dsl_empty() {
    atlas::ui::WidgetDSLParser parser;
    auto asset = parser.Parse("");
    assert(asset.WidgetCount() == 0);
    std::cout << "[PASS] test_widget_dsl_empty" << std::endl;
}

void test_widget_dsl_multiple_roots() {
    atlas::ui::WidgetDSLParser parser;
    std::string src = R"(
        panel "hud" { }
        panel "menu" { }
    )";
    auto asset = parser.Parse(src);
    assert(asset.WidgetCount() == 2);
    std::cout << "[PASS] test_widget_dsl_multiple_roots" << std::endl;
}
