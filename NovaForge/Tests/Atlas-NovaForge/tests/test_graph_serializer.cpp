#include <cassert>
#include <iostream>
#include <string>
#include <cmath>
#include "graphvm/GraphSerializer.h"

using namespace atlas::graphvm;

void test_json_builder_object() {
    JsonBuilder b;
    b.BeginObject();
    b.AddString("name", "testNode");
    b.AddInt("id", 42);
    b.AddFloat("weight", 3.14f);
    b.AddBool("active", true);
    b.EndObject();

    std::string json = b.Build();
    assert(json.find("\"name\":\"testNode\"") != std::string::npos);
    assert(json.find("\"id\":42") != std::string::npos);
    assert(json.find("\"active\":true") != std::string::npos);
    assert(json[0] == '{');
    assert(json[json.size() - 1] == '}');
    std::cout << "[PASS] test_json_builder_object" << std::endl;
}

void test_json_builder_array() {
    JsonBuilder b;
    b.BeginObject();
    b.AddString("graph", "myGraph");
    b.BeginArray("nodes");
    b.BeginArrayObject();
    b.AddString("type", "add");
    b.AddInt("id", 1);
    b.EndObject();
    b.BeginArrayObject();
    b.AddString("type", "mul");
    b.AddInt("id", 2);
    b.EndObject();
    b.EndArray();
    b.EndObject();

    std::string json = b.Build();
    assert(json.find("\"nodes\":[") != std::string::npos);
    assert(json.find("\"type\":\"add\"") != std::string::npos);
    assert(json.find("\"type\":\"mul\"") != std::string::npos);
    assert(json.find("\"graph\":\"myGraph\"") != std::string::npos);
    std::cout << "[PASS] test_json_builder_array" << std::endl;
}

void test_json_reader_parse() {
    std::string json = R"({"name":"hello","count":7,"rate":1.5,"enabled":true})";
    JsonReader r;
    assert(r.Parse(json));
    assert(r.GetString("name") == "hello");
    assert(r.GetInt("count") == 7);
    assert(std::fabs(r.GetFloat("rate") - 1.5f) < 0.001f);
    assert(r.GetBool("enabled") == true);
    assert(r.HasKey("name"));
    assert(!r.HasKey("missing"));
    std::cout << "[PASS] test_json_reader_parse" << std::endl;
}

void test_json_reader_array() {
    std::string json = R"({"items":[{"id":1,"label":"a"},{"id":2,"label":"b"}]})";
    JsonReader r;
    assert(r.Parse(json));
    assert(r.HasKey("items"));
    auto items = r.GetArray("items");
    assert(items.size() == 2);
    assert(items[0].values.at("id") == "1");
    assert(items[0].values.at("label") == "a");
    assert(items[1].values.at("id") == "2");
    assert(items[1].values.at("label") == "b");
    std::cout << "[PASS] test_json_reader_array" << std::endl;
}

void test_json_roundtrip() {
    JsonBuilder b;
    b.BeginObject();
    b.AddString("graph", "roundtrip");
    b.AddInt("version", 3);
    b.AddBool("dirty", false);
    b.BeginArray("edges");
    b.BeginArrayObject();
    b.AddInt("from", 0);
    b.AddInt("to", 1);
    b.EndObject();
    b.BeginArrayObject();
    b.AddInt("from", 1);
    b.AddInt("to", 2);
    b.EndObject();
    b.EndArray();
    b.EndObject();

    std::string json = b.Build();

    JsonReader r;
    assert(r.Parse(json));
    assert(r.GetString("graph") == "roundtrip");
    assert(r.GetInt("version") == 3);
    assert(r.GetBool("dirty") == false);

    auto edges = r.GetArray("edges");
    assert(edges.size() == 2);
    assert(edges[0].values.at("from") == "0");
    assert(edges[0].values.at("to") == "1");
    assert(edges[1].values.at("from") == "1");
    assert(edges[1].values.at("to") == "2");

    std::cout << "[PASS] test_json_roundtrip" << std::endl;
}
