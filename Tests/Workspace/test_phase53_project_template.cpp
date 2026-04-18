// Tests/Workspace/test_phase53_project_template.cpp
// Phase 53 — WorkspaceProjectTemplate: TemplateCategory, TemplateFileStub,
//             TemplateVariable, TemplateDefinition, TemplateInstance, TemplateRegistry
#include <catch2/catch_test_macros.hpp>
#include "NF/Workspace/WorkspaceProjectTemplate.h"

// ═════════════════════════════════════════════════════════════════
// TemplateCategory
// ═════════════════════════════════════════════════════════════════

TEST_CASE("TemplateCategory: valid with id and label", "[template][category]") {
    NF::TemplateCategory cat;
    cat.id    = "game";
    cat.label = "Game";
    REQUIRE(cat.isValid());
}

TEST_CASE("TemplateCategory: invalid without id or label", "[template][category]") {
    NF::TemplateCategory cat;
    REQUIRE_FALSE(cat.isValid());
    cat.id = "x";
    REQUIRE_FALSE(cat.isValid()); // no label
    cat.label = "X";
    REQUIRE(cat.isValid());
}

// ═════════════════════════════════════════════════════════════════
// TemplateFileStub
// ═════════════════════════════════════════════════════════════════

TEST_CASE("TemplateFileStub: valid with relative path", "[template][stub]") {
    NF::TemplateFileStub stub;
    stub.relativePath    = "src/main.cpp";
    stub.contentTemplate = "int main() { return 0; }";
    REQUIRE(stub.isValid());
}

TEST_CASE("TemplateFileStub: invalid without path", "[template][stub]") {
    NF::TemplateFileStub stub;
    REQUIRE_FALSE(stub.isValid());
}

// ═════════════════════════════════════════════════════════════════
// TemplateVariable
// ═════════════════════════════════════════════════════════════════

TEST_CASE("TemplateVariable: valid with key", "[template][variable]") {
    NF::TemplateVariable v;
    v.key          = "PROJECT_NAME";
    v.defaultValue = "MyProject";
    REQUIRE(v.isValid());
}

TEST_CASE("TemplateVariable: invalid without key", "[template][variable]") {
    NF::TemplateVariable v;
    REQUIRE_FALSE(v.isValid());
}

// ═════════════════════════════════════════════════════════════════
// TemplateDefinition
// ═════════════════════════════════════════════════════════════════

static NF::TemplateDefinition makeTemplate(const std::string& id,
                                           const std::string& name,
                                           const std::string& catId = "") {
    NF::TemplateDefinition tmpl;
    tmpl.id         = id;
    tmpl.name       = name;
    tmpl.categoryId = catId;
    tmpl.version    = "1.0";
    return tmpl;
}

TEST_CASE("TemplateDefinition: valid with id and name", "[template][definition]") {
    auto tmpl = makeTemplate("cpp.hello", "Hello World C++", "cpp");
    REQUIRE(tmpl.isValid());
}

TEST_CASE("TemplateDefinition: invalid without id or name", "[template][definition]") {
    NF::TemplateDefinition tmpl;
    REQUIRE_FALSE(tmpl.isValid());
}

TEST_CASE("TemplateDefinition: addStub valid", "[template][definition]") {
    auto tmpl = makeTemplate("t", "T");
    NF::TemplateFileStub stub;
    stub.relativePath = "main.cpp";
    REQUIRE(tmpl.addStub(stub));
    REQUIRE(tmpl.stubCount() == 1);
}

TEST_CASE("TemplateDefinition: addStub invalid rejected", "[template][definition]") {
    auto tmpl = makeTemplate("t", "T");
    REQUIRE_FALSE(tmpl.addStub(NF::TemplateFileStub{}));
}

TEST_CASE("TemplateDefinition: addStub duplicate path rejected", "[template][definition]") {
    auto tmpl = makeTemplate("t", "T");
    NF::TemplateFileStub stub;
    stub.relativePath = "main.cpp";
    tmpl.addStub(stub);
    REQUIRE_FALSE(tmpl.addStub(stub));
    REQUIRE(tmpl.stubCount() == 1);
}

TEST_CASE("TemplateDefinition: removeStub", "[template][definition]") {
    auto tmpl = makeTemplate("t", "T");
    NF::TemplateFileStub stub;
    stub.relativePath = "main.cpp";
    tmpl.addStub(stub);
    REQUIRE(tmpl.removeStub("main.cpp"));
    REQUIRE(tmpl.stubCount() == 0);
}

TEST_CASE("TemplateDefinition: removeStub unknown returns false", "[template][definition]") {
    auto tmpl = makeTemplate("t", "T");
    REQUIRE_FALSE(tmpl.removeStub("no.such"));
}

TEST_CASE("TemplateDefinition: findStub", "[template][definition]") {
    auto tmpl = makeTemplate("t", "T");
    NF::TemplateFileStub stub;
    stub.relativePath = "src/main.cpp";
    tmpl.addStub(stub);
    REQUIRE(tmpl.findStub("src/main.cpp") != nullptr);
    REQUIRE(tmpl.findStub("missing") == nullptr);
}

TEST_CASE("TemplateDefinition: addVariable valid", "[template][definition]") {
    auto tmpl = makeTemplate("t", "T");
    NF::TemplateVariable v;
    v.key = "PROJECT_NAME";
    REQUIRE(tmpl.addVariable(v));
    REQUIRE(tmpl.variableCount() == 1);
}

TEST_CASE("TemplateDefinition: addVariable invalid rejected", "[template][definition]") {
    auto tmpl = makeTemplate("t", "T");
    REQUIRE_FALSE(tmpl.addVariable(NF::TemplateVariable{}));
}

TEST_CASE("TemplateDefinition: addVariable duplicate key rejected", "[template][definition]") {
    auto tmpl = makeTemplate("t", "T");
    NF::TemplateVariable v;
    v.key = "NAME";
    tmpl.addVariable(v);
    REQUIRE_FALSE(tmpl.addVariable(v));
    REQUIRE(tmpl.variableCount() == 1);
}

TEST_CASE("TemplateDefinition: removeVariable", "[template][definition]") {
    auto tmpl = makeTemplate("t", "T");
    NF::TemplateVariable v; v.key = "NAME";
    tmpl.addVariable(v);
    REQUIRE(tmpl.removeVariable("NAME"));
    REQUIRE(tmpl.variableCount() == 0);
}

TEST_CASE("TemplateDefinition: substitute replaces placeholders", "[template][definition]") {
    auto tmpl = makeTemplate("t", "T");
    NF::TemplateVariable v; v.key = "NAME"; v.defaultValue = "World";
    tmpl.addVariable(v);

    std::map<std::string, std::string> vars = { {"NAME", "Atlas"} };
    std::string result = tmpl.substitute("Hello, {{NAME}}!", vars);
    REQUIRE(result == "Hello, Atlas!");
}

TEST_CASE("TemplateDefinition: substitute uses default when key absent", "[template][definition]") {
    auto tmpl = makeTemplate("t", "T");
    NF::TemplateVariable v; v.key = "NAME"; v.defaultValue = "Default";
    tmpl.addVariable(v);

    std::map<std::string, std::string> vars; // no NAME supplied
    std::string result = tmpl.substitute("Hi {{NAME}}", vars);
    REQUIRE(result == "Hi Default");
}

TEST_CASE("TemplateDefinition: substitute multiple occurrences", "[template][definition]") {
    auto tmpl = makeTemplate("t", "T");
    NF::TemplateVariable v; v.key = "N"; v.defaultValue = "x";
    tmpl.addVariable(v);

    std::map<std::string, std::string> vars = { {"N", "Foo"} };
    std::string result = tmpl.substitute("{{N}} and {{N}}", vars);
    REQUIRE(result == "Foo and Foo");
}

// ═════════════════════════════════════════════════════════════════
// TemplateRegistry
// ═════════════════════════════════════════════════════════════════

TEST_CASE("TemplateRegistry: default empty", "[template][registry]") {
    NF::TemplateRegistry reg;
    REQUIRE(reg.categoryCount() == 0);
    REQUIRE(reg.templateCount() == 0);
}

TEST_CASE("TemplateRegistry: addCategory", "[template][registry]") {
    NF::TemplateRegistry reg;
    NF::TemplateCategory cat;
    cat.id = "game"; cat.label = "Game";
    REQUIRE(reg.addCategory(cat));
    REQUIRE(reg.categoryCount() == 1);
    REQUIRE(reg.hasCategory("game"));
}

TEST_CASE("TemplateRegistry: addCategory invalid rejected", "[template][registry]") {
    NF::TemplateRegistry reg;
    REQUIRE_FALSE(reg.addCategory(NF::TemplateCategory{}));
}

TEST_CASE("TemplateRegistry: addCategory duplicate rejected", "[template][registry]") {
    NF::TemplateRegistry reg;
    NF::TemplateCategory cat; cat.id = "x"; cat.label = "X";
    reg.addCategory(cat);
    REQUIRE_FALSE(reg.addCategory(cat));
    REQUIRE(reg.categoryCount() == 1);
}

TEST_CASE("TemplateRegistry: removeCategory", "[template][registry]") {
    NF::TemplateRegistry reg;
    NF::TemplateCategory cat; cat.id = "x"; cat.label = "X";
    reg.addCategory(cat);
    REQUIRE(reg.removeCategory("x"));
    REQUIRE(reg.categoryCount() == 0);
}

TEST_CASE("TemplateRegistry: removeCategory unknown returns false", "[template][registry]") {
    NF::TemplateRegistry reg;
    REQUIRE_FALSE(reg.removeCategory("no.such"));
}

TEST_CASE("TemplateRegistry: addTemplate valid", "[template][registry]") {
    NF::TemplateRegistry reg;
    REQUIRE(reg.addTemplate(makeTemplate("cpp.hello", "Hello World")));
    REQUIRE(reg.templateCount() == 1);
    REQUIRE(reg.hasTemplate("cpp.hello"));
}

TEST_CASE("TemplateRegistry: addTemplate invalid rejected", "[template][registry]") {
    NF::TemplateRegistry reg;
    REQUIRE_FALSE(reg.addTemplate(NF::TemplateDefinition{}));
}

TEST_CASE("TemplateRegistry: addTemplate duplicate rejected", "[template][registry]") {
    NF::TemplateRegistry reg;
    reg.addTemplate(makeTemplate("t", "T"));
    REQUIRE_FALSE(reg.addTemplate(makeTemplate("t", "T2")));
    REQUIRE(reg.templateCount() == 1);
}

TEST_CASE("TemplateRegistry: removeTemplate", "[template][registry]") {
    NF::TemplateRegistry reg;
    reg.addTemplate(makeTemplate("t", "T"));
    REQUIRE(reg.removeTemplate("t"));
    REQUIRE(reg.templateCount() == 0);
}

TEST_CASE("TemplateRegistry: removeTemplate unknown returns false", "[template][registry]") {
    NF::TemplateRegistry reg;
    REQUIRE_FALSE(reg.removeTemplate("no.such"));
}

TEST_CASE("TemplateRegistry: findByCategory", "[template][registry]") {
    NF::TemplateRegistry reg;
    reg.addTemplate(makeTemplate("cpp.a", "A", "cpp"));
    reg.addTemplate(makeTemplate("cpp.b", "B", "cpp"));
    reg.addTemplate(makeTemplate("py.c",  "C", "python"));
    auto cppTmpls = reg.findByCategory("cpp");
    REQUIRE(cppTmpls.size() == 2);
    auto pyTmpls  = reg.findByCategory("python");
    REQUIRE(pyTmpls.size() == 1);
    REQUIRE(reg.findByCategory("missing").empty());
}

TEST_CASE("TemplateRegistry: instantiate produces resolved files", "[template][registry]") {
    NF::TemplateDefinition tmpl = makeTemplate("hello", "Hello World", "cpp");
    NF::TemplateVariable v; v.key = "PROJECT_NAME"; v.defaultValue = "MyApp";
    tmpl.addVariable(v);
    NF::TemplateFileStub stub;
    stub.relativePath    = "README.md";
    stub.contentTemplate = "# {{PROJECT_NAME}}\nA project.";
    tmpl.addStub(stub);

    NF::TemplateRegistry reg;
    reg.addTemplate(tmpl);

    auto inst = reg.instantiate("hello", { {"PROJECT_NAME", "Atlas"} });
    REQUIRE(inst.isComplete());
    REQUIRE(inst.resolvedFiles.count("README.md") == 1);
    REQUIRE(inst.resolvedFiles.at("README.md") == "# Atlas\nA project.");
    REQUIRE(inst.variables.at("PROJECT_NAME") == "Atlas");
}

TEST_CASE("TemplateRegistry: instantiate uses default when var absent", "[template][registry]") {
    NF::TemplateDefinition tmpl = makeTemplate("t", "T");
    NF::TemplateVariable v; v.key = "NAME"; v.defaultValue = "Default";
    tmpl.addVariable(v);
    NF::TemplateFileStub stub;
    stub.relativePath = "f.txt"; stub.contentTemplate = "{{NAME}}";
    tmpl.addStub(stub);

    NF::TemplateRegistry reg;
    reg.addTemplate(tmpl);

    auto inst = reg.instantiate("t"); // no vars supplied
    REQUIRE(inst.isComplete());
    REQUIRE(inst.resolvedFiles.at("f.txt") == "Default");
}

TEST_CASE("TemplateRegistry: instantiate reports missing required vars", "[template][registry]") {
    NF::TemplateDefinition tmpl = makeTemplate("t", "T");
    NF::TemplateVariable v; v.key = "REQUIRED"; v.required = true;
    tmpl.addVariable(v);
    NF::TemplateFileStub stub;
    stub.relativePath = "f.txt"; stub.contentTemplate = "{{REQUIRED}}";
    tmpl.addStub(stub);

    NF::TemplateRegistry reg;
    reg.addTemplate(tmpl);

    auto inst = reg.instantiate("t"); // required var missing
    REQUIRE_FALSE(inst.isComplete());
    REQUIRE(inst.missingRequired().size() == 1);
    REQUIRE(inst.missingRequired()[0] == "REQUIRED");
}

TEST_CASE("TemplateRegistry: instantiate unknown template returns empty", "[template][registry]") {
    NF::TemplateRegistry reg;
    auto inst = reg.instantiate("no.such");
    REQUIRE(inst.resolvedFiles.empty());
}

TEST_CASE("TemplateRegistry: observer fires on addTemplate", "[template][registry]") {
    NF::TemplateRegistry reg;
    int count = 0;
    reg.addObserver([&]{ ++count; });
    reg.addTemplate(makeTemplate("t", "T"));
    REQUIRE(count == 1);
}

TEST_CASE("TemplateRegistry: observer fires on removeTemplate", "[template][registry]") {
    NF::TemplateRegistry reg;
    reg.addTemplate(makeTemplate("t", "T"));
    int count = 0;
    reg.addObserver([&]{ ++count; });
    reg.removeTemplate("t");
    REQUIRE(count == 1);
}

TEST_CASE("TemplateRegistry: clearObservers stops notifications", "[template][registry]") {
    NF::TemplateRegistry reg;
    int count = 0;
    reg.addObserver([&]{ ++count; });
    reg.addTemplate(makeTemplate("a", "A"));
    REQUIRE(count == 1);
    reg.clearObservers();
    reg.addTemplate(makeTemplate("b", "B"));
    REQUIRE(count == 1);
}

TEST_CASE("TemplateRegistry: clear removes all", "[template][registry]") {
    NF::TemplateRegistry reg;
    NF::TemplateCategory cat; cat.id = "c"; cat.label = "C";
    reg.addCategory(cat);
    reg.addTemplate(makeTemplate("t", "T"));
    reg.clear();
    REQUIRE(reg.categoryCount() == 0);
    REQUIRE(reg.templateCount() == 0);
}

// ═════════════════════════════════════════════════════════════════
// Integration
// ═════════════════════════════════════════════════════════════════

TEST_CASE("Template integration: multi-file project instantiation", "[template][integration]") {
    NF::TemplateDefinition tmpl = makeTemplate("atlas.game", "Atlas Game Project", "game");
    tmpl.description = "Standard game project template";

    NF::TemplateVariable projName; projName.key = "PROJECT_NAME"; projName.required = true;
    NF::TemplateVariable ns;       ns.key = "NAMESPACE";   ns.defaultValue = "Game";
    tmpl.addVariable(projName);
    tmpl.addVariable(ns);

    NF::TemplateFileStub cmake;
    cmake.relativePath    = "CMakeLists.txt";
    cmake.contentTemplate = "project({{PROJECT_NAME}})\n";
    tmpl.addStub(cmake);

    NF::TemplateFileStub main;
    main.relativePath    = "src/main.cpp";
    main.contentTemplate = "namespace {{NAMESPACE}} { int main() {} }";
    tmpl.addStub(main);

    NF::TemplateRegistry reg;
    NF::TemplateCategory gameCat; gameCat.id = "game"; gameCat.label = "Game";
    reg.addCategory(gameCat);
    reg.addTemplate(tmpl);

    REQUIRE(reg.findByCategory("game").size() == 1);

    auto inst = reg.instantiate("atlas.game",
                                { {"PROJECT_NAME", "SuperGame"}, {"NAMESPACE", "SG"} });
    REQUIRE(inst.isComplete());
    REQUIRE(inst.resolvedFiles.at("CMakeLists.txt") == "project(SuperGame)\n");
    REQUIRE(inst.resolvedFiles.at("src/main.cpp")   == "namespace SG { int main() {} }");
}

TEST_CASE("Template integration: missing required var prevents completion", "[template][integration]") {
    NF::TemplateDefinition tmpl = makeTemplate("strict", "Strict");
    NF::TemplateVariable v1; v1.key = "A"; v1.required = true;
    NF::TemplateVariable v2; v2.key = "B"; v2.required = true;
    NF::TemplateVariable v3; v3.key = "C"; v3.defaultValue = "ok";
    tmpl.addVariable(v1);
    tmpl.addVariable(v2);
    tmpl.addVariable(v3);

    NF::TemplateRegistry reg;
    reg.addTemplate(tmpl);

    // Only supply B — A is missing
    auto inst = reg.instantiate("strict", { {"B", "hello"} });
    REQUIRE_FALSE(inst.isComplete());
    REQUIRE(inst.missingRequired().size() == 1);
    REQUIRE(inst.missingRequired()[0] == "A");
}
