#include <iostream>
#include <cassert>
#include "schema/SchemaValidator.h"

void test_schema_valid() {
    atlas::schema::SchemaDefinition schema;
    schema.id = "test_schema";
    schema.version = 1;
    atlas::schema::SchemaNodeDef node;
    node.id = "node1";
    node.inputs.push_back({"in_val", atlas::schema::SchemaValueType::Float, true});
    node.outputs.push_back({"out_val", atlas::schema::SchemaValueType::Float, false});
    schema.nodes.push_back(node);

    atlas::schema::SchemaValidator validator;
    assert(validator.Validate(schema));
    assert(validator.GetErrors().empty());
    std::cout << "  PASS: test_schema_valid" << std::endl;
}

void test_schema_empty_id() {
    atlas::schema::SchemaDefinition schema;
    schema.id = "";
    schema.version = 1;

    atlas::schema::SchemaValidator validator;
    assert(!validator.Validate(schema));
    assert(!validator.GetErrors().empty());
    std::cout << "  PASS: test_schema_empty_id" << std::endl;
}

void test_schema_zero_version() {
    atlas::schema::SchemaDefinition schema;
    schema.id = "test";
    schema.version = 0;

    atlas::schema::SchemaValidator validator;
    assert(!validator.Validate(schema));
    assert(!validator.GetErrors().empty());
    std::cout << "  PASS: test_schema_zero_version" << std::endl;
}

void test_schema_duplicate_node_id() {
    atlas::schema::SchemaDefinition schema;
    schema.id = "test";
    schema.version = 1;
    schema.nodes.push_back({"dup", {}, {}});
    schema.nodes.push_back({"dup", {}, {}});

    atlas::schema::SchemaValidator validator;
    assert(!validator.Validate(schema));
    assert(!validator.GetErrors().empty());
    std::cout << "  PASS: test_schema_duplicate_node_id" << std::endl;
}

void test_schema_empty_node_id() {
    atlas::schema::SchemaDefinition schema;
    schema.id = "test";
    schema.version = 1;
    schema.nodes.push_back({"", {}, {}});

    atlas::schema::SchemaValidator validator;
    assert(!validator.Validate(schema));
    assert(!validator.GetErrors().empty());
    std::cout << "  PASS: test_schema_empty_node_id" << std::endl;
}

void test_schema_no_nodes() {
    atlas::schema::SchemaDefinition schema;
    schema.id = "empty_schema";
    schema.version = 2;

    atlas::schema::SchemaValidator validator;
    assert(validator.Validate(schema));
    assert(validator.GetErrors().empty());
    std::cout << "  PASS: test_schema_no_nodes" << std::endl;
}
