#include "../engine/ui/GUIDSLParser.h"
#include <iostream>
#include <cassert>

using namespace atlas::ui;

void test_dsl_parse_simple_layout() {
    GUIDSLParser parser;
    auto result = parser.Parse(R"(
layout "TestLayout" {
    panel "MainPanel"
}
)");

    assert(result.success);
    assert(result.root != nullptr);
    assert(result.root->type == DSLNodeType::Layout);
    assert(result.root->name == "TestLayout");
    assert(result.root->children.size() == 1);
    assert(result.root->children[0]->type == DSLNodeType::Panel);
    assert(result.root->children[0]->name == "MainPanel");

    std::cout << "[PASS] test_dsl_parse_simple_layout" << std::endl;
}

void test_dsl_parse_split() {
    GUIDSLParser parser;
    auto result = parser.Parse(R"(
layout "SplitLayout" {
    split horizontal 0.25 {
        panel "Left"
        panel "Right"
    }
}
)");

    assert(result.success);
    assert(result.root->children.size() == 1);

    auto& split = result.root->children[0];
    assert(split->type == DSLNodeType::Split);
    assert(split->splitDir == DSLSplitDir::Horizontal);
    assert(split->ratio > 0.24f && split->ratio < 0.26f);
    assert(split->children.size() == 2);
    assert(split->children[0]->name == "Left");
    assert(split->children[1]->name == "Right");

    std::cout << "[PASS] test_dsl_parse_split" << std::endl;
}

void test_dsl_parse_nested_splits() {
    GUIDSLParser parser;
    auto result = parser.Parse(R"(
layout "Nested" {
    split horizontal 0.25 {
        panel "ToolShelf"
        split vertical 0.70 {
            panel "SceneView"
            panel "Properties"
        }
    }
}
)");

    assert(result.success);
    auto& outer = result.root->children[0];
    assert(outer->type == DSLNodeType::Split);
    assert(outer->children.size() == 2);
    assert(outer->children[0]->name == "ToolShelf");

    auto& inner = outer->children[1];
    assert(inner->type == DSLNodeType::Split);
    assert(inner->splitDir == DSLSplitDir::Vertical);
    assert(inner->children.size() == 2);
    assert(inner->children[0]->name == "SceneView");
    assert(inner->children[1]->name == "Properties");

    std::cout << "[PASS] test_dsl_parse_nested_splits" << std::endl;
}

void test_dsl_parse_dock() {
    GUIDSLParser parser;
    auto result = parser.Parse(R"(
layout "DockLayout" {
    dock bottom 0.30 {
        panel "Console"
    }
}
)");

    assert(result.success);
    auto& dock = result.root->children[0];
    assert(dock->type == DSLNodeType::Dock);
    assert(dock->dockTarget == DSLDockTarget::Bottom);
    assert(dock->ratio > 0.29f && dock->ratio < 0.31f);
    assert(dock->children.size() == 1);
    assert(dock->children[0]->name == "Console");

    std::cout << "[PASS] test_dsl_parse_dock" << std::endl;
}

void test_dsl_parse_tabs() {
    GUIDSLParser parser;
    auto result = parser.Parse(R"(
layout "TabLayout" {
    dock bottom 0.30 {
        tabs {
            panel "Console"
            panel "Timeline"
            panel "NodeGraph"
        }
    }
}
)");

    assert(result.success);
    auto& dock = result.root->children[0];
    assert(dock->type == DSLNodeType::Dock);

    auto& tabs = dock->children[0];
    assert(tabs->type == DSLNodeType::Tabs);
    assert(tabs->children.size() == 3);
    assert(tabs->children[0]->name == "Console");
    assert(tabs->children[1]->name == "Timeline");
    assert(tabs->children[2]->name == "NodeGraph");

    std::cout << "[PASS] test_dsl_parse_tabs" << std::endl;
}

void test_dsl_parse_full_example() {
    GUIDSLParser parser;
    auto result = parser.Parse(R"(
layout "DefaultEditor" {
    split horizontal 0.25 {
        panel "ToolShelf"
        split vertical 0.70 {
            panel "SceneView"
            panel "Properties"
        }
    }
    dock bottom 0.30 {
        tabs {
            panel "Console"
            panel "Timeline"
            panel "NodeGraph"
        }
    }
}
)");

    assert(result.success);
    assert(result.root->name == "DefaultEditor");
    assert(result.root->children.size() == 2);

    std::cout << "[PASS] test_dsl_parse_full_example" << std::endl;
}

void test_dsl_parse_comments() {
    GUIDSLParser parser;
    auto result = parser.Parse(R"(
# This is a comment
layout "Commented" {
    # Another comment
    panel "Main"
}
)");

    assert(result.success);
    assert(result.root->children.size() == 1);
    assert(result.root->children[0]->name == "Main");

    std::cout << "[PASS] test_dsl_parse_comments" << std::endl;
}

void test_dsl_parse_error_missing_layout() {
    GUIDSLParser parser;
    auto result = parser.Parse(R"(
panel "Orphan"
)");

    assert(!result.success);
    assert(!result.errorMessage.empty());

    std::cout << "[PASS] test_dsl_parse_error_missing_layout" << std::endl;
}

void test_dsl_parse_error_unterminated_string() {
    GUIDSLParser parser;
    auto result = parser.Parse(R"(
layout "Missing
)");

    assert(!result.success);

    std::cout << "[PASS] test_dsl_parse_error_unterminated_string" << std::endl;
}

void test_dsl_parse_error_unknown_statement() {
    GUIDSLParser parser;
    auto result = parser.Parse(R"(
layout "Bad" {
    widget "Unknown"
}
)");

    assert(!result.success);
    assert(result.errorMessage.find("unknown") != std::string::npos ||
           result.errorMessage.find("Unknown") != std::string::npos ||
           result.errorMessage.find("widget") != std::string::npos);

    std::cout << "[PASS] test_dsl_parse_error_unknown_statement" << std::endl;
}

void test_dsl_version() {
    assert(GUIDSLParser::Version() == 1);
    std::cout << "[PASS] test_dsl_version" << std::endl;
}

void test_dsl_parse_dock_targets() {
    const char* targets[] = {"top", "bottom", "left", "right", "center", "float"};
    DSLDockTarget expected[] = {
        DSLDockTarget::Top, DSLDockTarget::Bottom, DSLDockTarget::Left,
        DSLDockTarget::Right, DSLDockTarget::Center, DSLDockTarget::Float
    };

    for (int i = 0; i < 6; ++i) {
        GUIDSLParser parser;
        std::string src = "layout \"Test\" { dock " + std::string(targets[i]) + " 0.5 { panel \"P\" } }";
        auto result = parser.Parse(src);
        assert(result.success);
        assert(result.root->children[0]->dockTarget == expected[i]);
    }

    std::cout << "[PASS] test_dsl_parse_dock_targets" << std::endl;
}
