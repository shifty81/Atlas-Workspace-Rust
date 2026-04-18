#include <catch2/catch_test_macros.hpp>
#include "NF/GraphVM/GraphVM.h"

// ── Original tests ───────────────────────────────────────────────

TEST_CASE("GraphVM initializes", "[GraphVM]") {
    NF::GraphVM vm;
    vm.init();
    REQUIRE_FALSE(vm.halted());
}

TEST_CASE("GraphVM loads and runs empty program", "[GraphVM]") {
    NF::GraphVM vm;
    vm.init();

    std::vector<NF::GraphInstruction> program;
    vm.loadProgram(program);

    vm.step();
    REQUIRE(vm.halted()); // empty program halts immediately
}

TEST_CASE("GraphVM steps through program", "[GraphVM]") {
    NF::GraphVM vm;
    vm.init();

    std::vector<NF::GraphInstruction> program = {
        {NF::GraphOpCode::Nop, 0, 0, 0},
        {NF::GraphOpCode::Nop, 0, 0, 0},
        {NF::GraphOpCode::Nop, 0, 0, 0}
    };
    vm.loadProgram(program);

    vm.step(); // Nop #1
    REQUIRE_FALSE(vm.halted());
    vm.step(); // Nop #2
    REQUIRE_FALSE(vm.halted());
    vm.step(); // Nop #3 — now at end
    REQUIRE_FALSE(vm.halted());
    vm.step(); // past end — halts
    REQUIRE(vm.halted());
}

TEST_CASE("GraphType enum covers 14 types", "[GraphVM]") {
    // Verify all 14 graph types are defined
    REQUIRE(static_cast<uint8_t>(NF::GraphType::World) == 0);
    REQUIRE(static_cast<uint8_t>(NF::GraphType::Asset) == 13);
}

// ── Arithmetic tests ─────────────────────────────────────────────

TEST_CASE("GraphVM Add integers", "[GraphVM]") {
    NF::GraphVM vm;
    vm.init();

    NF::GraphProgram prog;
    prog.constants = {int32_t(10), int32_t(20)};
    prog.instructions = {
        {NF::GraphOpCode::LoadConst, 0, 0, 0}, // r0 = 10
        {NF::GraphOpCode::LoadConst, 1, 1, 0}, // r1 = 20
        {NF::GraphOpCode::Add,       2, 0, 1}, // r2 = r0 + r1
        {NF::GraphOpCode::Halt,      0, 0, 0}
    };
    vm.loadProgram(prog);
    vm.run();

    REQUIRE(vm.halted());
    REQUIRE(std::get<int32_t>(vm.getRegister(2)) == 30);
}

TEST_CASE("GraphVM Add floats", "[GraphVM]") {
    NF::GraphVM vm;
    vm.init();

    NF::GraphProgram prog;
    prog.constants = {1.5f, 2.5f};
    prog.instructions = {
        {NF::GraphOpCode::LoadConst, 0, 0, 0},
        {NF::GraphOpCode::LoadConst, 1, 1, 0},
        {NF::GraphOpCode::Add,       2, 0, 1},
        {NF::GraphOpCode::Halt,      0, 0, 0}
    };
    vm.loadProgram(prog);
    vm.run();

    REQUIRE(std::get<float>(vm.getRegister(2)) == 4.0f);
}

TEST_CASE("GraphVM Subtract", "[GraphVM]") {
    NF::GraphVM vm;
    vm.init();

    NF::GraphProgram prog;
    prog.constants = {int32_t(50), int32_t(18)};
    prog.instructions = {
        {NF::GraphOpCode::LoadConst, 0, 0, 0},
        {NF::GraphOpCode::LoadConst, 1, 1, 0},
        {NF::GraphOpCode::Subtract,  2, 0, 1},
        {NF::GraphOpCode::Halt,      0, 0, 0}
    };
    vm.loadProgram(prog);
    vm.run();

    REQUIRE(std::get<int32_t>(vm.getRegister(2)) == 32);
}

TEST_CASE("GraphVM Multiply", "[GraphVM]") {
    NF::GraphVM vm;
    vm.init();

    NF::GraphProgram prog;
    prog.constants = {int32_t(7), int32_t(6)};
    prog.instructions = {
        {NF::GraphOpCode::LoadConst, 0, 0, 0},
        {NF::GraphOpCode::LoadConst, 1, 1, 0},
        {NF::GraphOpCode::Multiply,  2, 0, 1},
        {NF::GraphOpCode::Halt,      0, 0, 0}
    };
    vm.loadProgram(prog);
    vm.run();

    REQUIRE(std::get<int32_t>(vm.getRegister(2)) == 42);
}

TEST_CASE("GraphVM Divide", "[GraphVM]") {
    NF::GraphVM vm;
    vm.init();

    NF::GraphProgram prog;
    prog.constants = {int32_t(100), int32_t(4)};
    prog.instructions = {
        {NF::GraphOpCode::LoadConst, 0, 0, 0},
        {NF::GraphOpCode::LoadConst, 1, 1, 0},
        {NF::GraphOpCode::Divide,    2, 0, 1},
        {NF::GraphOpCode::Halt,      0, 0, 0}
    };
    vm.loadProgram(prog);
    vm.run();

    REQUIRE(std::get<int32_t>(vm.getRegister(2)) == 25);
}

TEST_CASE("GraphVM Divide by zero returns zero", "[GraphVM]") {
    NF::GraphVM vm;
    vm.init();

    NF::GraphProgram prog;
    prog.constants = {int32_t(42), int32_t(0)};
    prog.instructions = {
        {NF::GraphOpCode::LoadConst, 0, 0, 0},
        {NF::GraphOpCode::LoadConst, 1, 1, 0},
        {NF::GraphOpCode::Divide,    2, 0, 1},
        {NF::GraphOpCode::Halt,      0, 0, 0}
    };
    vm.loadProgram(prog);
    vm.run();

    REQUIRE(std::get<int32_t>(vm.getRegister(2)) == 0);
}

// ── Compare and Branch ───────────────────────────────────────────

TEST_CASE("GraphVM Compare and Branch", "[GraphVM]") {
    NF::GraphVM vm;
    vm.init();

    // Compare 3 < 5 → true → branch to instruction 5 (load 100)
    NF::GraphProgram prog;
    prog.constants = {int32_t(3), int32_t(5), int32_t(100), int32_t(999)};
    prog.instructions = {
        {NF::GraphOpCode::LoadConst, 0, 0, 0}, // 0: r0=3
        {NF::GraphOpCode::LoadConst, 1, 1, 0}, // 1: r1=5
        {NF::GraphOpCode::Compare,   2, 0, 1}, // 2: r2 = (r0 < r1) = true
        {NF::GraphOpCode::Branch,    2, 5, 7}, // 3: if r2 goto 5 else 7
        {NF::GraphOpCode::Nop,       0, 0, 0}, // 4: (unreachable)
        {NF::GraphOpCode::LoadConst, 3, 2, 0}, // 5: r3=100
        {NF::GraphOpCode::Halt,      0, 0, 0}, // 6: halt
        {NF::GraphOpCode::LoadConst, 3, 3, 0}, // 7: r3=999
        {NF::GraphOpCode::Halt,      0, 0, 0}  // 8: halt
    };
    vm.loadProgram(prog);
    vm.run();

    REQUIRE(std::get<bool>(vm.getRegister(2)) == true);
    REQUIRE(std::get<int32_t>(vm.getRegister(3)) == 100);
}

TEST_CASE("GraphVM Compare false branch", "[GraphVM]") {
    NF::GraphVM vm;
    vm.init();

    // Compare 10 < 2 → false → branch to instruction 7 (load 999)
    NF::GraphProgram prog;
    prog.constants = {int32_t(10), int32_t(2), int32_t(100), int32_t(999)};
    prog.instructions = {
        {NF::GraphOpCode::LoadConst, 0, 0, 0}, // 0: r0=10
        {NF::GraphOpCode::LoadConst, 1, 1, 0}, // 1: r1=2
        {NF::GraphOpCode::Compare,   2, 0, 1}, // 2: r2 = (10 < 2) = false
        {NF::GraphOpCode::Branch,    2, 5, 7}, // 3: if r2 goto 5 else 7
        {NF::GraphOpCode::Nop,       0, 0, 0}, // 4: (unreachable)
        {NF::GraphOpCode::LoadConst, 3, 2, 0}, // 5: r3=100
        {NF::GraphOpCode::Halt,      0, 0, 0}, // 6: halt
        {NF::GraphOpCode::LoadConst, 3, 3, 0}, // 7: r3=999
        {NF::GraphOpCode::Halt,      0, 0, 0}  // 8: halt
    };
    vm.loadProgram(prog);
    vm.run();

    REQUIRE(std::get<bool>(vm.getRegister(2)) == false);
    REQUIRE(std::get<int32_t>(vm.getRegister(3)) == 999);
}

// ── Store / Load memory ──────────────────────────────────────────

TEST_CASE("GraphVM Store and Load memory", "[GraphVM]") {
    NF::GraphVM vm;
    vm.init();

    NF::GraphProgram prog;
    prog.constants = {int32_t(42)};
    prog.instructions = {
        {NF::GraphOpCode::LoadConst, 0, 0, 0}, // r0 = 42
        {NF::GraphOpCode::Store,     0, 5, 0}, // mem[5] = r0
        {NF::GraphOpCode::Load,      1, 5, 0}, // r1 = mem[5]
        {NF::GraphOpCode::Halt,      0, 0, 0}
    };
    vm.loadProgram(prog);
    vm.run();

    REQUIRE(std::get<int32_t>(vm.getRegister(1)) == 42);
    REQUIRE(std::get<int32_t>(vm.getMemory(5)) == 42);
}

// ── Call / Return ────────────────────────────────────────────────

TEST_CASE("GraphVM Call and Return", "[GraphVM]") {
    NF::GraphVM vm;
    vm.init();

    NF::GraphProgram prog;
    prog.constants = {int32_t(7), int32_t(3)};
    prog.instructions = {
        {NF::GraphOpCode::LoadConst, 0, 0, 0}, // 0: r0=7
        {NF::GraphOpCode::Call,      4, 0, 0}, // 1: call addr 4, push return=2
        {NF::GraphOpCode::Halt,      0, 0, 0}, // 2: halt (return here)
        {NF::GraphOpCode::Nop,       0, 0, 0}, // 3: (unreachable)
        {NF::GraphOpCode::LoadConst, 1, 1, 0}, // 4: r1=3 (subroutine)
        {NF::GraphOpCode::Add,       2, 0, 1}, // 5: r2 = r0 + r1 = 10
        {NF::GraphOpCode::Return,    0, 0, 0}  // 6: return
    };
    vm.loadProgram(prog);
    vm.run();

    REQUIRE(std::get<int32_t>(vm.getRegister(2)) == 10);
}

// ── Constant pool loading ────────────────────────────────────────

TEST_CASE("GraphVM constant pool loading", "[GraphVM]") {
    NF::GraphVM vm;
    vm.init();

    NF::GraphProgram prog;
    prog.constants = {int32_t(11), 3.14f, true, std::string("hello")};
    prog.instructions = {
        {NF::GraphOpCode::LoadConst, 0, 0, 0},
        {NF::GraphOpCode::LoadConst, 1, 1, 0},
        {NF::GraphOpCode::LoadConst, 2, 2, 0},
        {NF::GraphOpCode::LoadConst, 3, 3, 0},
        {NF::GraphOpCode::Halt,      0, 0, 0}
    };
    vm.loadProgram(prog);
    vm.run();

    REQUIRE(std::get<int32_t>(vm.getRegister(0)) == 11);
    REQUIRE(std::get<float>(vm.getRegister(1)) == 3.14f);
    REQUIRE(std::get<bool>(vm.getRegister(2)) == true);
    REQUIRE(std::get<std::string>(vm.getRegister(3)) == "hello");
}

// ── Graph model tests ────────────────────────────────────────────

TEST_CASE("GraphNode / GraphPort construction", "[GraphVM]") {
    NF::GraphPort inPort{"input", NF::GraphDataType::Float, 0};
    NF::GraphPort outPort{"output", NF::GraphDataType::Int, 0};

    NF::GraphNode node;
    node.id   = 1;
    node.name = "TestNode";
    node.type = NF::GraphType::World;
    node.position = {10.f, 20.f};
    node.inputs.push_back(inPort);
    node.outputs.push_back(outPort);

    REQUIRE(node.id == 1);
    REQUIRE(node.name == "TestNode");
    REQUIRE(node.position.x == 10.f);
    REQUIRE(node.inputs.size() == 1);
    REQUIRE(node.inputs[0].name == "input");
    REQUIRE(node.outputs[0].dataType == NF::GraphDataType::Int);
}

TEST_CASE("Graph add/remove nodes and links", "[GraphVM]") {
    NF::Graph graph;

    NF::GraphNode n1; n1.id = 1; n1.name = "A";
    NF::GraphNode n2; n2.id = 2; n2.name = "B";
    NF::GraphNode n3; n3.id = 3; n3.name = "C";
    graph.addNode(n1);
    graph.addNode(n2);
    graph.addNode(n3);

    REQUIRE(graph.nodes().size() == 3);
    REQUIRE(graph.findNode(2)->name == "B");

    NF::GraphLink link{1, 0, 2, 0};
    graph.addLink(link);
    REQUIRE(graph.links().size() == 1);

    graph.removeNode(2);
    REQUIRE(graph.nodes().size() == 2);
    REQUIRE(graph.links().empty()); // link referencing node 2 removed
    REQUIRE(graph.findNode(2) == nullptr);
}

// ── GraphCompiler test ───────────────────────────────────────────

TEST_CASE("GraphCompiler compiles simple add graph", "[GraphVM]") {
    // Build graph: Constant(3) + Constant(7) = 10
    NF::Graph graph;

    NF::GraphNode c1;
    c1.id = 1; c1.name = "Constant";
    c1.position = {3.f, 0.f}; // position.x = constant value
    c1.outputs.push_back({"out", NF::GraphDataType::Int, 0});
    graph.addNode(c1);

    NF::GraphNode c2;
    c2.id = 2; c2.name = "Constant";
    c2.position = {7.f, 0.f};
    c2.outputs.push_back({"out", NF::GraphDataType::Int, 0});
    graph.addNode(c2);

    NF::GraphNode add;
    add.id = 3; add.name = "Add";
    add.inputs.push_back({"a", NF::GraphDataType::Int, 0});
    add.inputs.push_back({"b", NF::GraphDataType::Int, 1});
    add.outputs.push_back({"result", NF::GraphDataType::Int, 0});
    graph.addNode(add);

    graph.addLink({1, 0, 3, 0}); // c1 → add input 0
    graph.addLink({2, 0, 3, 1}); // c2 → add input 1

    auto prog = NF::GraphCompiler::compile(graph);
    REQUIRE(!prog.instructions.empty());
    REQUIRE(prog.constants.size() == 2);

    NF::GraphVM vm;
    vm.init();
    vm.loadProgram(prog);
    vm.run();

    // The add node's register should contain 10
    // Constants go into r0=3, r1=7 (topological order), add into r2=10
    REQUIRE(std::get<int32_t>(vm.getRegister(2)) == 10);
}

// ── GraphSerializer tests ────────────────────────────────────────

TEST_CASE("GraphSerializer program round-trip", "[GraphVM]") {
    NF::GraphProgram original;
    original.constants = {int32_t(42), 3.14f, true, std::string("test")};
    original.instructions = {
        {NF::GraphOpCode::LoadConst, 0, 0, 0},
        {NF::GraphOpCode::LoadConst, 1, 1, 0},
        {NF::GraphOpCode::Add,       2, 0, 1},
        {NF::GraphOpCode::Halt,      0, 0, 0}
    };

    auto json = NF::GraphSerializer::programToJson(original);
    std::string jsonStr = json.toJson();
    auto parsed = NF::JsonParser::parse(jsonStr);
    auto restored = NF::GraphSerializer::programFromJson(parsed);

    REQUIRE(restored.instructions.size() == original.instructions.size());
    REQUIRE(restored.constants.size() == original.constants.size());

    for (size_t i = 0; i < original.instructions.size(); ++i) {
        REQUIRE(restored.instructions[i].opcode  == original.instructions[i].opcode);
        REQUIRE(restored.instructions[i].operandA == original.instructions[i].operandA);
        REQUIRE(restored.instructions[i].operandB == original.instructions[i].operandB);
        REQUIRE(restored.instructions[i].operandC == original.instructions[i].operandC);
    }

    REQUIRE(std::get<int32_t>(restored.constants[0]) == 42);
    REQUIRE(std::get<float>(restored.constants[1]) == 3.14f);
    REQUIRE(std::get<bool>(restored.constants[2]) == true);
    REQUIRE(std::get<std::string>(restored.constants[3]) == "test");
}

TEST_CASE("GraphSerializer graph round-trip", "[GraphVM]") {
    NF::Graph original;

    NF::GraphNode n1;
    n1.id = 1; n1.name = "Input"; n1.type = NF::GraphType::World;
    n1.position = {10.f, 20.f};
    n1.outputs.push_back({"out", NF::GraphDataType::Float, 0});
    original.addNode(n1);

    NF::GraphNode n2;
    n2.id = 2; n2.name = "Output"; n2.type = NF::GraphType::Strategy;
    n2.position = {100.f, 200.f};
    n2.inputs.push_back({"in", NF::GraphDataType::Float, 0});
    original.addNode(n2);

    original.addLink({1, 0, 2, 0});

    auto json = NF::GraphSerializer::graphToJson(original);
    std::string jsonStr = json.toJson();
    auto parsed = NF::JsonParser::parse(jsonStr);
    auto restored = NF::GraphSerializer::graphFromJson(parsed);

    REQUIRE(restored.nodes().size() == 2);
    REQUIRE(restored.links().size() == 1);

    auto* rn1 = restored.findNode(1);
    REQUIRE(rn1 != nullptr);
    REQUIRE(rn1->name == "Input");
    REQUIRE(rn1->position.x == 10.f);
    REQUIRE(rn1->outputs.size() == 1);

    auto* rn2 = restored.findNode(2);
    REQUIRE(rn2 != nullptr);
    REQUIRE(rn2->name == "Output");

    REQUIRE(restored.links()[0].sourceNode == 1);
    REQUIRE(restored.links()[0].targetNode == 2);
}
