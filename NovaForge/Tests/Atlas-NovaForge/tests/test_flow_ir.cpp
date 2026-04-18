#include "../engine/flow/FlowGraphIR.h"
#include <iostream>
#include <cassert>

void test_flow_ir_default_values() {
    atlas::flow::FlowGraphIR ir;
    assert(ir.version == atlas::flow::FlowGraphIR::CURRENT_VERSION);
    assert(ir.name.empty());
    assert(ir.nodes.empty());
    assert(ir.edges.empty());
    std::cout << "[PASS] test_flow_ir_default_values" << std::endl;
}

void test_flow_ir_add_nodes() {
    atlas::flow::FlowGraphIR ir;
    atlas::flow::FlowNodeIR node;
    node.id = 1;
    node.type = "State";
    node.category = "Flow";
    ir.nodes.push_back(node);
    node.id = 2;
    node.type = "Condition";
    ir.nodes.push_back(node);
    assert(ir.nodes.size() == 2);
    std::cout << "[PASS] test_flow_ir_add_nodes" << std::endl;
}

void test_flow_ir_add_edges() {
    atlas::flow::FlowGraphIR ir;
    atlas::flow::FlowEdge edge{1, 0, 2, 0};
    ir.edges.push_back(edge);
    ir.edges.push_back({2, 0, 3, 0});
    assert(ir.edges.size() == 2);
    std::cout << "[PASS] test_flow_ir_add_edges" << std::endl;
}

void test_flow_ir_to_json() {
    atlas::flow::FlowGraphIR ir;
    ir.name = "TestGraph";
    ir.graphType = "Gameplay";
    atlas::flow::FlowNodeIR node;
    node.id = 1;
    node.type = "State";
    node.category = "Flow";
    ir.nodes.push_back(node);
    std::string json = ir.ToJSON();
    assert(!json.empty());
    assert(json.find("TestGraph") != std::string::npos);
    assert(json.find("Gameplay") != std::string::npos);
    std::cout << "[PASS] test_flow_ir_to_json" << std::endl;
}

void test_flow_ir_roundtrip() {
    atlas::flow::FlowGraphIR ir;
    ir.name = "RoundTrip";
    ir.graphType = "UI";
    atlas::flow::FlowNodeIR node;
    node.id = 1;
    node.type = "State";
    node.category = "Flow";
    node.posX = 10.0f;
    node.posY = 20.0f;
    ir.nodes.push_back(node);
    ir.edges.push_back({1, 0, 1, 0});

    std::string json = ir.ToJSON();
    auto restored = atlas::flow::FlowGraphIR::FromJSON(json);
    assert(restored.version == ir.version);
    assert(restored.name == ir.name);
    assert(restored.graphType == ir.graphType);
    assert(restored.nodes.size() == ir.nodes.size());
    assert(restored.nodes[0].id == 1);
    assert(restored.nodes[0].type == "State");
    assert(restored.edges.size() == ir.edges.size());
    std::cout << "[PASS] test_flow_ir_roundtrip" << std::endl;
}

void test_flow_ir_validate_valid() {
    atlas::flow::FlowGraphIR ir;
    ir.name = "Valid";
    ir.graphType = "Gameplay";
    atlas::flow::FlowNodeIR node;
    node.id = 1;
    node.type = "State";
    node.category = "Flow";
    ir.nodes.push_back(node);
    assert(ir.Validate());
    std::cout << "[PASS] test_flow_ir_validate_valid" << std::endl;
}

void test_flow_ir_validate_invalid() {
    atlas::flow::FlowGraphIR ir;
    ir.name = "Invalid";
    atlas::flow::FlowNodeIR node;
    node.id = 1;
    node.type = "State";
    ir.nodes.push_back(node);
    node.id = 1; // duplicate ID
    node.type = "Condition";
    ir.nodes.push_back(node);
    assert(!ir.Validate());
    std::cout << "[PASS] test_flow_ir_validate_invalid" << std::endl;
}

void test_flow_ir_migrate_v1() {
    atlas::flow::FlowGraphIR ir;
    ir.version = 1;
    ir.name = "MigrateTest";
    ir.graphType = "AI";
    atlas::flow::FlowNodeIR node;
    node.id = 1;
    node.type = "State";
    ir.nodes.push_back(node);

    auto migrated = atlas::flow::FlowGraphIR::Migrate(ir, 1);
    assert(migrated.version == 1);
    assert(migrated.name == ir.name);
    assert(migrated.graphType == ir.graphType);
    assert(migrated.nodes.size() == ir.nodes.size());
    std::cout << "[PASS] test_flow_ir_migrate_v1" << std::endl;
}
