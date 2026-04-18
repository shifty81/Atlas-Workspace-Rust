#include <catch2/catch_test_macros.hpp>
#include "NF/Editor/Editor.h"
#include "NF/Editor/LogicGraph.h"

using namespace NF;

// 1. LogicPinType all 8 names
TEST_CASE("LogicPinType names cover all 8 types", "[Editor][S7]") {
    CHECK(std::string(logicPinTypeName(LogicPinType::Flow))   == "Flow");
    CHECK(std::string(logicPinTypeName(LogicPinType::Bool))   == "Bool");
    CHECK(std::string(logicPinTypeName(LogicPinType::Int))    == "Int");
    CHECK(std::string(logicPinTypeName(LogicPinType::Float))  == "Float");
    CHECK(std::string(logicPinTypeName(LogicPinType::String)) == "String");
    CHECK(std::string(logicPinTypeName(LogicPinType::Vector)) == "Vector");
    CHECK(std::string(logicPinTypeName(LogicPinType::Event))  == "Event");
    CHECK(std::string(logicPinTypeName(LogicPinType::Object)) == "Object");
}

// 2. LogicPin defaults
TEST_CASE("LogicPin has correct defaults", "[Editor][S7]") {
    LogicPin pin;
    CHECK(pin.id.empty());
    CHECK(pin.name.empty());
    CHECK(pin.type == LogicPinType::Flow);
    CHECK(pin.isOutput == false);
    CHECK(pin.connected == false);
    CHECK(pin.value == 0.f);
}

// 3. LogicNodeType all 8 names
TEST_CASE("LogicNodeType names cover all 8 types", "[Editor][S7]") {
    CHECK(std::string(logicNodeTypeName(LogicNodeType::AndGate)) == "AND Gate");
    CHECK(std::string(logicNodeTypeName(LogicNodeType::OrGate))  == "OR Gate");
    CHECK(std::string(logicNodeTypeName(LogicNodeType::NotGate)) == "NOT Gate");
    CHECK(std::string(logicNodeTypeName(LogicNodeType::Latch))   == "Latch");
    CHECK(std::string(logicNodeTypeName(LogicNodeType::Delay))   == "Delay");
    CHECK(std::string(logicNodeTypeName(LogicNodeType::Switch))  == "Switch");
    CHECK(std::string(logicNodeTypeName(LogicNodeType::Compare)) == "Compare");
    CHECK(std::string(logicNodeTypeName(LogicNodeType::MathOp))  == "Math Op");
}

// 4. LogicNodeDef defaults
TEST_CASE("LogicNodeDef has correct defaults", "[Editor][S7]") {
    LogicNodeDef def;
    CHECK(def.name.empty());
    CHECK(def.nodeType == LogicNodeType::AndGate);
    CHECK(def.inputs.empty());
    CHECK(def.outputs.empty());
    CHECK(def.description.empty());
}

// 5. LogicWireNode add inputs/outputs
TEST_CASE("LogicWireNode add inputs and outputs", "[Editor][S7]") {
    LogicWireNode node;
    LogicPin inPin;
    inPin.id = "in1";
    LogicPin outPin;
    outPin.id = "out1";
    outPin.isOutput = true;

    CHECK(node.addInput(inPin));
    CHECK(node.addOutput(outPin));
    CHECK(node.inputCount() == 1);
    CHECK(node.outputCount() == 1);
}

// 6. LogicWireNode max pins
TEST_CASE("LogicWireNode enforces max pins", "[Editor][S7]") {
    LogicWireNode node;
    for (size_t i = 0; i < LogicWireNode::kMaxPins; ++i) {
        LogicPin pin;
        pin.id = "p" + std::to_string(i);
        CHECK(node.addInput(pin));
    }
    LogicPin extra;
    extra.id = "extra";
    CHECK_FALSE(node.addInput(extra));
    CHECK(node.inputCount() == LogicWireNode::kMaxPins);
}

// 7. LogicWireNode findInput/findOutput
TEST_CASE("LogicWireNode findInput and findOutput", "[Editor][S7]") {
    LogicWireNode node;
    LogicPin in1;
    in1.id = "in_a";
    in1.value = 3.f;
    LogicPin out1;
    out1.id = "out_a";
    out1.value = 7.f;
    node.addInput(in1);
    node.addOutput(out1);

    CHECK(node.findInput("in_a") != nullptr);
    CHECK(node.findInput("in_a")->value == 3.f);
    CHECK(node.findOutput("out_a") != nullptr);
    CHECK(node.findOutput("out_a")->value == 7.f);
    CHECK(node.findInput("nope") == nullptr);
    CHECK(node.findOutput("nope") == nullptr);

    // const overloads
    const auto& cnode = node;
    CHECK(cnode.findInput("in_a") != nullptr);
    CHECK(cnode.findOutput("out_a") != nullptr);
}

// 8. LogicWireNode evaluate AND gate
TEST_CASE("LogicWireNode evaluate AND gate", "[Editor][S7]") {
    LogicWireNode node;
    node.setNodeType(LogicNodeType::AndGate);
    LogicPin in1; in1.id = "a"; in1.value = 1.f;
    LogicPin in2; in2.id = "b"; in2.value = 1.f;
    LogicPin out; out.id = "o"; out.isOutput = true;
    node.addInput(in1);
    node.addInput(in2);
    node.addOutput(out);

    node.evaluate();
    CHECK(node.outputs()[0].value == 1.f);

    // false case: set one input low
    node.findInput("b")->value = 0.f;
    node.evaluate();
    CHECK(node.outputs()[0].value == 0.f);
}

// 9. LogicWireNode evaluate OR gate
TEST_CASE("LogicWireNode evaluate OR gate", "[Editor][S7]") {
    LogicWireNode node;
    node.setNodeType(LogicNodeType::OrGate);
    LogicPin in1; in1.id = "a"; in1.value = 0.f;
    LogicPin in2; in2.id = "b"; in2.value = 1.f;
    LogicPin out; out.id = "o"; out.isOutput = true;
    node.addInput(in1);
    node.addInput(in2);
    node.addOutput(out);

    node.evaluate();
    CHECK(node.outputs()[0].value == 1.f);

    node.findInput("b")->value = 0.f;
    node.evaluate();
    CHECK(node.outputs()[0].value == 0.f);
}

// 10. LogicWireNode evaluate NOT gate
TEST_CASE("LogicWireNode evaluate NOT gate", "[Editor][S7]") {
    LogicWireNode node;
    node.setNodeType(LogicNodeType::NotGate);
    LogicPin in1; in1.id = "a"; in1.value = 0.f;
    LogicPin out; out.id = "o"; out.isOutput = true;
    node.addInput(in1);
    node.addOutput(out);

    node.evaluate();
    CHECK(node.outputs()[0].value == 1.f);

    node.findInput("a")->value = 1.f;
    node.evaluate();
    CHECK(node.outputs()[0].value == 0.f);
}

// 11. LogicWireNode evaluate MathOp
TEST_CASE("LogicWireNode evaluate MathOp sums inputs", "[Editor][S7]") {
    LogicWireNode node;
    node.setNodeType(LogicNodeType::MathOp);
    LogicPin a; a.id = "a"; a.value = 2.5f;
    LogicPin b; b.id = "b"; b.value = 3.5f;
    LogicPin out; out.id = "o"; out.isOutput = true;
    node.addInput(a);
    node.addInput(b);
    node.addOutput(out);

    node.evaluate();
    CHECK(node.outputs()[0].value == 6.f);
}

// 12. LogicWire defaults
TEST_CASE("LogicWire has correct defaults", "[Editor][S7]") {
    LogicWire wire;
    CHECK(wire.sourceNodeId == -1);
    CHECK(wire.sourcePin.empty());
    CHECK(wire.targetNodeId == -1);
    CHECK(wire.targetPin.empty());
}

// 13. LogicWireGraph add/remove nodes
TEST_CASE("LogicWireGraph add and remove nodes", "[Editor][S7]") {
    LogicWireGraph graph;
    LogicWireNode n;
    n.setName("node1");
    int id = graph.addNode(n);
    CHECK(id >= 0);
    CHECK(graph.nodeCount() == 1);
    CHECK(graph.findNode(id) != nullptr);
    CHECK(graph.findNode(id)->name() == "node1");

    CHECK(graph.removeNode(id));
    CHECK(graph.nodeCount() == 0);
    CHECK(graph.findNode(id) == nullptr);
}

// 14. LogicWireGraph add wire validation
TEST_CASE("LogicWireGraph rejects wires with bad node refs", "[Editor][S7]") {
    LogicWireGraph graph;
    LogicWire wire;
    wire.sourceNodeId = 999;
    wire.targetNodeId = 888;
    CHECK_FALSE(graph.addWire(wire));

    LogicWireNode n;
    int id = graph.addNode(n);
    wire.sourceNodeId = id;
    wire.targetNodeId = 888;
    CHECK_FALSE(graph.addWire(wire));
}

// 15. LogicWireGraph max nodes
TEST_CASE("LogicWireGraph enforces max nodes", "[Editor][S7]") {
    LogicWireGraph graph;
    for (size_t i = 0; i < LogicWireGraph::kMaxNodes; ++i) {
        LogicWireNode n;
        CHECK(graph.addNode(n) >= 0);
    }
    LogicWireNode extra;
    CHECK(graph.addNode(extra) == -1);
    CHECK(graph.nodeCount() == LogicWireGraph::kMaxNodes);
}

// 16. LogicWireGraph isValid
TEST_CASE("LogicWireGraph isValid", "[Editor][S7]") {
    LogicWireGraph graph;
    CHECK(graph.isValid());  // no wires = valid

    LogicWireNode a, b;
    int idA = graph.addNode(a);
    int idB = graph.addNode(b);
    LogicWire wire;
    wire.sourceNodeId = idA;
    wire.targetNodeId = idB;
    CHECK(graph.addWire(wire));
    CHECK(graph.isValid());
}

// 17. LogicWireGraph evaluate propagation
TEST_CASE("LogicWireGraph evaluate calls evaluate on nodes", "[Editor][S7]") {
    LogicWireGraph graph;
    LogicWireNode n;
    n.setNodeType(LogicNodeType::AndGate);
    LogicPin in1; in1.id = "a"; in1.value = 1.f;
    LogicPin out; out.id = "o"; out.isOutput = true; out.value = 0.f;
    n.addInput(in1);
    n.addOutput(out);
    int id = graph.addNode(n);

    graph.evaluate();
    CHECK(graph.findNode(id)->outputs()[0].value == 1.f);
}

// 18. LogicGraphTemplate defaults
TEST_CASE("LogicGraphTemplate has correct defaults", "[Editor][S7]") {
    LogicGraphTemplate tmpl;
    CHECK(tmpl.name.empty());
    CHECK(tmpl.description.empty());
    CHECK(tmpl.category.empty());
    CHECK(tmpl.nodeDefs.empty());
}

// 19. LogicTemplateLibrary add/remove/find
TEST_CASE("LogicTemplateLibrary add remove find", "[Editor][S7]") {
    LogicTemplateLibrary lib;
    LogicGraphTemplate t;
    t.name = "FlipFlop";
    t.category = "Logic";

    CHECK(lib.addTemplate(t));
    CHECK(lib.templateCount() == 1);
    CHECK(lib.findTemplate("FlipFlop") != nullptr);
    CHECK(lib.findTemplate("FlipFlop")->category == "Logic");

    CHECK(lib.removeTemplate("FlipFlop"));
    CHECK(lib.templateCount() == 0);
    CHECK(lib.findTemplate("FlipFlop") == nullptr);
}

// 20. LogicTemplateLibrary duplicates rejected
TEST_CASE("LogicTemplateLibrary rejects duplicate names", "[Editor][S7]") {
    LogicTemplateLibrary lib;
    LogicGraphTemplate t;
    t.name = "Dup";
    CHECK(lib.addTemplate(t));
    CHECK_FALSE(lib.addTemplate(t));
    CHECK(lib.templateCount() == 1);
}

// 21. LogicTemplateLibrary category filtering
TEST_CASE("LogicTemplateLibrary category filtering", "[Editor][S7]") {
    LogicTemplateLibrary lib;
    LogicGraphTemplate t1; t1.name = "A"; t1.category = "Logic";
    LogicGraphTemplate t2; t2.name = "B"; t2.category = "Math";
    LogicGraphTemplate t3; t3.name = "C"; t3.category = "Logic";
    lib.addTemplate(t1);
    lib.addTemplate(t2);
    lib.addTemplate(t3);

    auto logic = lib.templatesInCategory("Logic");
    CHECK(logic.size() == 2);

    auto math = lib.templatesInCategory("Math");
    CHECK(math.size() == 1);

    CHECK(lib.categoryCount() == 2);
}
