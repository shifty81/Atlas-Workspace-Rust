#include <catch2/catch_test_macros.hpp>
#include "NF/Editor/Editor.h"

using namespace NF;

TEST_CASE("AssetDepType names", "[Editor][S17]") {
    REQUIRE(std::string(assetDepTypeName(AssetDepType::Texture))   == "Texture");
    REQUIRE(std::string(assetDepTypeName(AssetDepType::Mesh))      == "Mesh");
    REQUIRE(std::string(assetDepTypeName(AssetDepType::Shader))    == "Shader");
    REQUIRE(std::string(assetDepTypeName(AssetDepType::Script))    == "Script");
    REQUIRE(std::string(assetDepTypeName(AssetDepType::Audio))     == "Audio");
    REQUIRE(std::string(assetDepTypeName(AssetDepType::Material))  == "Material");
    REQUIRE(std::string(assetDepTypeName(AssetDepType::Animation)) == "Animation");
    REQUIRE(std::string(assetDepTypeName(AssetDepType::Level))     == "Level");
}

TEST_CASE("AssetDepNode predicates", "[Editor][S17]") {
    AssetDepNode n;
    n.assetId = "tex01";
    n.status  = AssetDepStatus::Unknown;
    REQUIRE_FALSE(n.isResolved());
    REQUIRE_FALSE(n.isMissing());
    REQUIRE_FALSE(n.isCircular());

    n.status = AssetDepStatus::Resolved;
    REQUIRE(n.isResolved());

    n.status = AssetDepStatus::Missing;
    REQUIRE(n.isMissing());

    n.status = AssetDepStatus::Circular;
    REQUIRE(n.isCircular());
}

TEST_CASE("AssetDepNode addDependency + hasDependency", "[Editor][S17]") {
    AssetDepNode n;
    n.assetId = "mat01";

    REQUIRE(n.addDependency("tex01"));
    REQUIRE(n.addDependency("tex02"));
    REQUIRE_FALSE(n.addDependency("tex01")); // duplicate
    REQUIRE_FALSE(n.addDependency("mat01")); // self-dep
    REQUIRE(n.dependencyCount() == 2);
    REQUIRE(n.hasDependency("tex01"));
    REQUIRE_FALSE(n.hasDependency("tex99"));
}

TEST_CASE("AssetDepGraph addNode + duplicate rejection", "[Editor][S17]") {
    AssetDepGraph g;
    AssetDepNode n1; n1.assetId = "a1"; n1.type = AssetDepType::Texture;
    AssetDepNode n2; n2.assetId = "a1"; // duplicate

    REQUIRE(g.addNode(n1));
    REQUIRE_FALSE(g.addNode(n2));
    REQUIRE(g.nodeCount() == 1);
}

TEST_CASE("AssetDepGraph removeNode + cascade", "[Editor][S17]") {
    AssetDepGraph g;
    AssetDepNode a; a.assetId = "matA";
    AssetDepNode b; b.assetId = "texB";
    g.addNode(a);
    g.addNode(b);
    g.addEdge("matA", "texB");
    REQUIRE(g.hasEdge("matA", "texB"));

    REQUIRE(g.removeNode("texB"));
    REQUIRE(g.nodeCount() == 1);
    REQUIRE_FALSE(g.hasEdge("matA", "texB")); // edge cascaded away
}

TEST_CASE("AssetDepGraph addEdge + hasEdge", "[Editor][S17]") {
    AssetDepGraph g;
    AssetDepNode a; a.assetId = "shader01";
    AssetDepNode b; b.assetId = "tex01";
    g.addNode(a);
    g.addNode(b);

    REQUIRE(g.addEdge("shader01", "tex01"));
    REQUIRE(g.hasEdge("shader01", "tex01"));
    REQUIRE_FALSE(g.hasEdge("tex01", "shader01")); // directed

    REQUIRE_FALSE(g.addEdge("shader01", "missing")); // dep not registered
}

TEST_CASE("AssetDepGraph resolveAll", "[Editor][S17]") {
    AssetDepGraph g;
    AssetDepNode a; a.assetId = "n1"; a.status = AssetDepStatus::Unknown;
    AssetDepNode b; b.assetId = "n2"; b.status = AssetDepStatus::Unknown;
    g.addNode(a);
    g.addNode(b);

    g.resolveAll();
    REQUIRE(g.unresolvedCount() == 0);
    REQUIRE(g.findNode("n1")->isResolved());
    REQUIRE(g.findNode("n2")->isResolved());
}

TEST_CASE("AssetDepGraph detectCircular", "[Editor][S17]") {
    AssetDepGraph g;
    AssetDepNode a; a.assetId = "A";
    AssetDepNode b; b.assetId = "B";
    AssetDepNode c; c.assetId = "C";
    g.addNode(a);
    g.addNode(b);
    g.addNode(c);
    g.addEdge("A", "B");
    g.addEdge("B", "C");
    g.addEdge("C", "A"); // cycle A→B→C→A

    g.detectCircular();
    REQUIRE(g.findNode("A")->isCircular());
    REQUIRE(g.findNode("B")->isCircular());
    REQUIRE(g.findNode("C")->isCircular());
}

TEST_CASE("AssetDepGraph totalEdgeCount", "[Editor][S17]") {
    AssetDepGraph g;
    AssetDepNode a; a.assetId = "X";
    AssetDepNode b; b.assetId = "Y";
    AssetDepNode c; c.assetId = "Z";
    g.addNode(a); g.addNode(b); g.addNode(c);
    g.addEdge("X", "Y");
    g.addEdge("X", "Z");

    REQUIRE(g.totalEdgeCount() == 2);
}

TEST_CASE("AssetDependencyTracker registerAsset + duplicate", "[Editor][S17]") {
    AssetDependencyTracker t;
    REQUIRE(t.registerAsset("mat01", "assets/mat.json", AssetDepType::Material));
    REQUIRE_FALSE(t.registerAsset("mat01", "other/path.json", AssetDepType::Material));
    REQUIRE(t.assetCount() == 1);
}

TEST_CASE("AssetDependencyTracker unregisterAsset", "[Editor][S17]") {
    AssetDependencyTracker t;
    t.registerAsset("tex01", "textures/rock.png", AssetDepType::Texture);
    REQUIRE(t.assetCount() == 1);
    REQUIRE(t.unregisterAsset("tex01"));
    REQUIRE(t.assetCount() == 0);
    REQUIRE_FALSE(t.unregisterAsset("tex01")); // already gone
}

TEST_CASE("AssetDependencyTracker addDependency + hasDependency", "[Editor][S17]") {
    AssetDependencyTracker t;
    t.registerAsset("mat01", "mat.json",  AssetDepType::Material);
    t.registerAsset("tex01", "tex.png",   AssetDepType::Texture);
    t.registerAsset("sdr01", "lit.glsl",  AssetDepType::Shader);

    REQUIRE(t.addDependency("mat01", "tex01"));
    REQUIRE(t.addDependency("mat01", "sdr01"));
    REQUIRE(t.hasDependency("mat01", "tex01"));
    REQUIRE(t.hasDependency("mat01", "sdr01"));
    REQUIRE_FALSE(t.hasDependency("tex01", "mat01"));

    REQUIRE(t.totalDependencies() == 2);
}

TEST_CASE("AssetDependencyTracker resolveAll + unresolvedCount", "[Editor][S17]") {
    AssetDependencyTracker t;
    t.registerAsset("a", "a.json", AssetDepType::Level);
    t.registerAsset("b", "b.json", AssetDepType::Script);
    REQUIRE(t.unresolvedCount() == 2);

    t.resolveAll();
    REQUIRE(t.unresolvedCount() == 0);
    REQUIRE(t.findAsset("a")->isResolved());
}

TEST_CASE("AssetDependencyTracker detectCircular marks nodes", "[Editor][S17]") {
    AssetDependencyTracker t;
    t.registerAsset("p", "p.glsl", AssetDepType::Shader);
    t.registerAsset("q", "q.glsl", AssetDepType::Shader);
    t.addDependency("p", "q");
    t.addDependency("q", "p"); // cycle

    t.detectCircular();
    REQUIRE(t.findAsset("p")->isCircular());
    REQUIRE(t.findAsset("q")->isCircular());
}
