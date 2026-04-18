#include <catch2/catch_test_macros.hpp>
#include <catch2/catch_approx.hpp>
#include "NF/Editor/Editor.h"
#include "NF/Editor/ShaderGraphEditor.h"

using namespace NF;

TEST_CASE("ShaderNodeType names cover all 8 values", "[Editor][S33]") {
    REQUIRE(std::string(shaderNodeTypeName(ShaderNodeType::Input))   == "Input");
    REQUIRE(std::string(shaderNodeTypeName(ShaderNodeType::Output))  == "Output");
    REQUIRE(std::string(shaderNodeTypeName(ShaderNodeType::Math))    == "Math");
    REQUIRE(std::string(shaderNodeTypeName(ShaderNodeType::Texture)) == "Texture");
    REQUIRE(std::string(shaderNodeTypeName(ShaderNodeType::Color))   == "Color");
    REQUIRE(std::string(shaderNodeTypeName(ShaderNodeType::Vector))  == "Vector");
    REQUIRE(std::string(shaderNodeTypeName(ShaderNodeType::Blend))   == "Blend");
    REQUIRE(std::string(shaderNodeTypeName(ShaderNodeType::Custom))  == "Custom");
}

TEST_CASE("ShaderPortKind names cover all 4 values", "[Editor][S33]") {
    REQUIRE(std::string(shaderPortKindName(ShaderPortKind::Float))   == "Float");
    REQUIRE(std::string(shaderPortKindName(ShaderPortKind::Vector2)) == "Vector2");
    REQUIRE(std::string(shaderPortKindName(ShaderPortKind::Vector3)) == "Vector3");
    REQUIRE(std::string(shaderPortKindName(ShaderPortKind::Vector4)) == "Vector4");
}

TEST_CASE("ShaderNode select and deselect toggle selected flag", "[Editor][S33]") {
    ShaderNode n; n.id = "n1";
    REQUIRE_FALSE(n.selected);
    n.select();
    REQUIRE(n.selected);
    n.deselect();
    REQUIRE_FALSE(n.selected);
}

TEST_CASE("ShaderNode setPosition updates posX and posY", "[Editor][S33]") {
    ShaderNode n; n.id = "n2";
    n.setPosition(100.0f, 200.0f);
    REQUIRE(n.posX == Catch::Approx(100.0f));
    REQUIRE(n.posY == Catch::Approx(200.0f));
}

TEST_CASE("ShaderGraphEditor addNode and duplicate rejection", "[Editor][S33]") {
    ShaderGraphEditor editor;
    ShaderNode a; a.id = "a";
    ShaderNode b; b.id = "b";
    ShaderNode dup; dup.id = "a";

    REQUIRE(editor.addNode(a));
    REQUIRE(editor.addNode(b));
    REQUIRE_FALSE(editor.addNode(dup));
    REQUIRE(editor.nodeCount() == 2);
}

TEST_CASE("ShaderGraphEditor removeNode reduces nodeCount", "[Editor][S33]") {
    ShaderGraphEditor editor;
    ShaderNode n; n.id = "n";
    editor.addNode(n);

    REQUIRE(editor.removeNode("n"));
    REQUIRE_FALSE(editor.removeNode("n"));
    REQUIRE(editor.nodeCount() == 0);
}

TEST_CASE("ShaderGraphEditor findNode returns pointer or nullptr", "[Editor][S33]") {
    ShaderGraphEditor editor;
    ShaderNode n; n.id = "x"; n.type = ShaderNodeType::Texture;
    editor.addNode(n);

    REQUIRE(editor.findNode("x") != nullptr);
    REQUIRE(editor.findNode("x")->type == ShaderNodeType::Texture);
    REQUIRE(editor.findNode("missing") == nullptr);
}

TEST_CASE("ShaderGraphEditor addEdge requires valid endpoints", "[Editor][S33]") {
    ShaderGraphEditor editor;
    ShaderNode a; a.id = "a";
    ShaderNode b; b.id = "b";
    editor.addNode(a);
    editor.addNode(b);

    ShaderGraphEdge e; e.id = "e1"; e.fromNode = "a"; e.toNode = "b";
    REQUIRE(editor.addEdge(e));
    REQUIRE(editor.edgeCount() == 1);

    ShaderGraphEdge bad; bad.id = "bad"; bad.fromNode = "a"; bad.toNode = "missing";
    REQUIRE_FALSE(editor.addEdge(bad));
    REQUIRE(editor.edgeCount() == 1);
}

TEST_CASE("ShaderGraphEditor addEdge rejects duplicate edge id", "[Editor][S33]") {
    ShaderGraphEditor editor;
    ShaderNode a; a.id = "a";
    ShaderNode b; b.id = "b";
    editor.addNode(a);
    editor.addNode(b);

    ShaderGraphEdge e; e.id = "e1"; e.fromNode = "a"; e.toNode = "b";
    REQUIRE(editor.addEdge(e));
    REQUIRE_FALSE(editor.addEdge(e));
    REQUIRE(editor.edgeCount() == 1);
}

TEST_CASE("ShaderGraphEditor removeEdge works correctly", "[Editor][S33]") {
    ShaderGraphEditor editor;
    ShaderNode a; a.id = "a";
    ShaderNode b; b.id = "b";
    editor.addNode(a);
    editor.addNode(b);

    ShaderGraphEdge e; e.id = "e1"; e.fromNode = "a"; e.toNode = "b";
    editor.addEdge(e);

    REQUIRE(editor.removeEdge("e1"));
    REQUIRE_FALSE(editor.removeEdge("e1"));
    REQUIRE(editor.edgeCount() == 0);
}

TEST_CASE("ShaderGraphEditor removeNode also removes connected edges", "[Editor][S33]") {
    ShaderGraphEditor editor;
    ShaderNode a; a.id = "a";
    ShaderNode b; b.id = "b";
    editor.addNode(a);
    editor.addNode(b);

    ShaderGraphEdge e; e.id = "e1"; e.fromNode = "a"; e.toNode = "b";
    editor.addEdge(e);
    REQUIRE(editor.edgeCount() == 1);

    editor.removeNode("a");
    REQUIRE(editor.edgeCount() == 0);
}

TEST_CASE("ShaderGraphEditor setActiveNode and activeNode", "[Editor][S33]") {
    ShaderGraphEditor editor;
    ShaderNode a; a.id = "a";
    ShaderNode b; b.id = "b";
    editor.addNode(a);
    editor.addNode(b);

    REQUIRE(editor.activeNode().empty());
    REQUIRE(editor.setActiveNode("a"));
    REQUIRE(editor.activeNode() == "a");
    REQUIRE_FALSE(editor.setActiveNode("nonexistent"));
    REQUIRE(editor.activeNode() == "a");
}

TEST_CASE("ShaderGraphEditor removeNode clears activeNode if active", "[Editor][S33]") {
    ShaderGraphEditor editor;
    ShaderNode n; n.id = "active";
    editor.addNode(n);
    editor.setActiveNode("active");
    REQUIRE(editor.activeNode() == "active");

    editor.removeNode("active");
    REQUIRE(editor.activeNode().empty());
}

TEST_CASE("ShaderGraphEditor selectAll and deselectAll", "[Editor][S33]") {
    ShaderGraphEditor editor;
    ShaderNode a; a.id = "a";
    ShaderNode b; b.id = "b";
    editor.addNode(a);
    editor.addNode(b);

    REQUIRE(editor.selectedCount() == 0);
    editor.selectAll();
    REQUIRE(editor.selectedCount() == 2);
    editor.deselectAll();
    REQUIRE(editor.selectedCount() == 0);
}

TEST_CASE("ShaderGraphEditor selectedCount reflects individual node selections", "[Editor][S33]") {
    ShaderGraphEditor editor;
    ShaderNode a; a.id = "a";
    ShaderNode b; b.id = "b";
    ShaderNode c; c.id = "c";
    editor.addNode(a);
    editor.addNode(b);
    editor.addNode(c);

    editor.findNode("a")->select();
    editor.findNode("c")->select();
    REQUIRE(editor.selectedCount() == 2);
}

TEST_CASE("ShaderGraphEditor MAX_NODES limit enforced", "[Editor][S33]") {
    ShaderGraphEditor editor;
    for (size_t i = 0; i < ShaderGraphEditor::MAX_NODES; ++i) {
        ShaderNode n; n.id = "n" + std::to_string(i);
        REQUIRE(editor.addNode(n));
    }
    ShaderNode overflow; overflow.id = "overflow";
    REQUIRE_FALSE(editor.addNode(overflow));
    REQUIRE(editor.nodeCount() == ShaderGraphEditor::MAX_NODES);
}
