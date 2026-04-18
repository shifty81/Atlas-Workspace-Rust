// Tests/Workspace/test_phase57_snippet_library.cpp
// Phase 57 — WorkspaceSnippetLibrary: SnippetLanguage, SnippetEntry,
//             SnippetFolder, SnippetLibrary
#include <catch2/catch_test_macros.hpp>
#include "NF/Workspace/WorkspaceSnippetLibrary.h"

// ═════════════════════════════════════════════════════════════════
// SnippetLanguage
// ═════════════════════════════════════════════════════════════════

TEST_CASE("SnippetLanguage: name helpers", "[snippet][language]") {
    REQUIRE(std::string(NF::snippetLanguageName(NF::SnippetLanguage::None))   == "None");
    REQUIRE(std::string(NF::snippetLanguageName(NF::SnippetLanguage::Cpp))    == "C++");
    REQUIRE(std::string(NF::snippetLanguageName(NF::SnippetLanguage::HLSL))   == "HLSL");
    REQUIRE(std::string(NF::snippetLanguageName(NF::SnippetLanguage::GLSL))   == "GLSL");
    REQUIRE(std::string(NF::snippetLanguageName(NF::SnippetLanguage::Python)) == "Python");
    REQUIRE(std::string(NF::snippetLanguageName(NF::SnippetLanguage::Lua))    == "Lua");
    REQUIRE(std::string(NF::snippetLanguageName(NF::SnippetLanguage::JSON))   == "JSON");
    REQUIRE(std::string(NF::snippetLanguageName(NF::SnippetLanguage::XML))    == "XML");
    REQUIRE(std::string(NF::snippetLanguageName(NF::SnippetLanguage::Custom)) == "Custom");
}

// ═════════════════════════════════════════════════════════════════
// SnippetEntry
// ═════════════════════════════════════════════════════════════════

static NF::SnippetEntry makeSnippet(const std::string& id, const std::string& title,
                                    const std::string& body = "code",
                                    NF::SnippetLanguage lang = NF::SnippetLanguage::Cpp) {
    NF::SnippetEntry s;
    s.id         = id;
    s.title      = title;
    s.body       = body;
    s.language   = lang;
    s.createdMs  = 100;
    s.modifiedMs = 200;
    return s;
}

TEST_CASE("SnippetEntry: default invalid", "[snippet][entry]") {
    NF::SnippetEntry s;
    REQUIRE_FALSE(s.isValid());
}

TEST_CASE("SnippetEntry: valid with id and title", "[snippet][entry]") {
    auto s = makeSnippet("s1", "Hello World");
    REQUIRE(s.isValid());
}

TEST_CASE("SnippetEntry: invalid without title", "[snippet][entry]") {
    NF::SnippetEntry s;
    s.id = "x";
    REQUIRE_FALSE(s.isValid());
}

TEST_CASE("SnippetEntry: equality by id", "[snippet][entry]") {
    auto a = makeSnippet("a", "A");
    auto b = makeSnippet("a", "B");
    auto c = makeSnippet("c", "C");
    REQUIRE(a == b);
    REQUIRE(a != c);
}

TEST_CASE("SnippetEntry: addTag", "[snippet][entry]") {
    auto s = makeSnippet("s1", "Test");
    REQUIRE(s.addTag("math"));
    REQUIRE(s.hasTag("math"));
    REQUIRE_FALSE(s.addTag("math")); // duplicate
    REQUIRE_FALSE(s.addTag(""));     // empty
}

TEST_CASE("SnippetEntry: removeTag", "[snippet][entry]") {
    auto s = makeSnippet("s1", "Test");
    s.addTag("math");
    REQUIRE(s.removeTag("math"));
    REQUIRE_FALSE(s.hasTag("math"));
    REQUIRE_FALSE(s.removeTag("nope"));
}

// ═════════════════════════════════════════════════════════════════
// SnippetFolder
// ═════════════════════════════════════════════════════════════════

static NF::SnippetFolder makeSnipFolder(const std::string& id, const std::string& name) {
    NF::SnippetFolder f;
    f.id   = id;
    f.name = name;
    return f;
}

TEST_CASE("SnippetFolder: default empty", "[snippet][folder]") {
    NF::SnippetFolder f;
    REQUIRE(f.count() == 0);
    REQUIRE(f.empty());
}

TEST_CASE("SnippetFolder: valid", "[snippet][folder]") {
    auto f = makeSnipFolder("gen", "General");
    REQUIRE(f.isValid());
}

TEST_CASE("SnippetFolder: invalid without id", "[snippet][folder]") {
    NF::SnippetFolder f;
    f.name = "Test";
    REQUIRE_FALSE(f.isValid());
}

TEST_CASE("SnippetFolder: addSnippet", "[snippet][folder]") {
    auto f = makeSnipFolder("gen", "General");
    REQUIRE(f.addSnippet(makeSnippet("s1", "Hello")));
    REQUIRE(f.count() == 1);
    REQUIRE(f.containsSnippet("s1"));
}

TEST_CASE("SnippetFolder: addSnippet rejects invalid", "[snippet][folder]") {
    auto f = makeSnipFolder("gen", "General");
    NF::SnippetEntry bad;
    REQUIRE_FALSE(f.addSnippet(bad));
}

TEST_CASE("SnippetFolder: addSnippet rejects duplicate", "[snippet][folder]") {
    auto f = makeSnipFolder("gen", "General");
    f.addSnippet(makeSnippet("s1", "Hello"));
    REQUIRE_FALSE(f.addSnippet(makeSnippet("s1", "World")));
}

TEST_CASE("SnippetFolder: removeSnippet", "[snippet][folder]") {
    auto f = makeSnipFolder("gen", "General");
    f.addSnippet(makeSnippet("s1", "Hello"));
    REQUIRE(f.removeSnippet("s1"));
    REQUIRE(f.empty());
}

TEST_CASE("SnippetFolder: removeSnippet unknown", "[snippet][folder]") {
    auto f = makeSnipFolder("gen", "General");
    REQUIRE_FALSE(f.removeSnippet("nope"));
}

TEST_CASE("SnippetFolder: findSnippet", "[snippet][folder]") {
    auto f = makeSnipFolder("gen", "General");
    f.addSnippet(makeSnippet("s1", "Hello"));
    REQUIRE(f.findSnippet("s1") != nullptr);
    REQUIRE(f.findSnippet("s1")->title == "Hello");
    REQUIRE(f.findSnippet("nope") == nullptr);
}

TEST_CASE("SnippetFolder: findSnippetMut", "[snippet][folder]") {
    auto f = makeSnipFolder("gen", "General");
    f.addSnippet(makeSnippet("s1", "Hello"));
    auto* s = f.findSnippetMut("s1");
    REQUIRE(s != nullptr);
    s->title = "Modified";
    REQUIRE(f.findSnippet("s1")->title == "Modified");
}

TEST_CASE("SnippetFolder: clear", "[snippet][folder]") {
    auto f = makeSnipFolder("gen", "General");
    f.addSnippet(makeSnippet("s1", "Hello"));
    f.clear();
    REQUIRE(f.empty());
}

// ═════════════════════════════════════════════════════════════════
// SnippetLibrary
// ═════════════════════════════════════════════════════════════════

TEST_CASE("SnippetLibrary: default empty", "[snippet][library]") {
    NF::SnippetLibrary lib;
    REQUIRE(lib.folderCount() == 0);
    REQUIRE(lib.totalSnippets() == 0);
}

TEST_CASE("SnippetLibrary: addFolder", "[snippet][library]") {
    NF::SnippetLibrary lib;
    REQUIRE(lib.addFolder(makeSnipFolder("gen", "General")));
    REQUIRE(lib.folderCount() == 1);
    REQUIRE(lib.hasFolder("gen"));
}

TEST_CASE("SnippetLibrary: addFolder rejects invalid", "[snippet][library]") {
    NF::SnippetLibrary lib;
    NF::SnippetFolder bad;
    REQUIRE_FALSE(lib.addFolder(bad));
}

TEST_CASE("SnippetLibrary: addFolder rejects duplicate", "[snippet][library]") {
    NF::SnippetLibrary lib;
    lib.addFolder(makeSnipFolder("gen", "General"));
    REQUIRE_FALSE(lib.addFolder(makeSnipFolder("gen", "General2")));
}

TEST_CASE("SnippetLibrary: removeFolder", "[snippet][library]") {
    NF::SnippetLibrary lib;
    lib.addFolder(makeSnipFolder("gen", "General"));
    REQUIRE(lib.removeFolder("gen"));
    REQUIRE(lib.folderCount() == 0);
}

TEST_CASE("SnippetLibrary: removeFolder unknown", "[snippet][library]") {
    NF::SnippetLibrary lib;
    REQUIRE_FALSE(lib.removeFolder("nope"));
}

TEST_CASE("SnippetLibrary: addSnippet to folder", "[snippet][library]") {
    NF::SnippetLibrary lib;
    lib.addFolder(makeSnipFolder("gen", "General"));
    REQUIRE(lib.addSnippet("gen", makeSnippet("s1", "Hello")));
    REQUIRE(lib.totalSnippets() == 1);
}

TEST_CASE("SnippetLibrary: addSnippet unknown folder", "[snippet][library]") {
    NF::SnippetLibrary lib;
    REQUIRE_FALSE(lib.addSnippet("nope", makeSnippet("s1", "Hello")));
}

TEST_CASE("SnippetLibrary: removeSnippet", "[snippet][library]") {
    NF::SnippetLibrary lib;
    lib.addFolder(makeSnipFolder("gen", "General"));
    lib.addSnippet("gen", makeSnippet("s1", "Hello"));
    REQUIRE(lib.removeSnippet("gen", "s1"));
    REQUIRE(lib.totalSnippets() == 0);
}

TEST_CASE("SnippetLibrary: removeSnippet unknown", "[snippet][library]") {
    NF::SnippetLibrary lib;
    lib.addFolder(makeSnipFolder("gen", "General"));
    REQUIRE_FALSE(lib.removeSnippet("gen", "nope"));
}

TEST_CASE("SnippetLibrary: searchByTag", "[snippet][library]") {
    NF::SnippetLibrary lib;
    lib.addFolder(makeSnipFolder("gen", "General"));
    auto s1 = makeSnippet("s1", "Math util");
    s1.addTag("math");
    lib.addSnippet("gen", s1);
    lib.addSnippet("gen", makeSnippet("s2", "No tags"));

    auto results = lib.searchByTag("math");
    REQUIRE(results.size() == 1);
    REQUIRE(results[0]->id == "s1");
}

TEST_CASE("SnippetLibrary: searchByLanguage", "[snippet][library]") {
    NF::SnippetLibrary lib;
    lib.addFolder(makeSnipFolder("gen", "General"));
    lib.addSnippet("gen", makeSnippet("s1", "C++ code", "int x;", NF::SnippetLanguage::Cpp));
    lib.addSnippet("gen", makeSnippet("s2", "Python code", "x = 1", NF::SnippetLanguage::Python));

    auto results = lib.searchByLanguage(NF::SnippetLanguage::Cpp);
    REQUIRE(results.size() == 1);
    REQUIRE(results[0]->id == "s1");
}

TEST_CASE("SnippetLibrary: searchByText", "[snippet][library]") {
    NF::SnippetLibrary lib;
    lib.addFolder(makeSnipFolder("gen", "General"));
    lib.addSnippet("gen", makeSnippet("s1", "Matrix multiply", "mat4 result"));
    lib.addSnippet("gen", makeSnippet("s2", "Vector add", "vec3 sum"));

    auto results = lib.searchByText("matrix");
    REQUIRE(results.size() == 1);
    REQUIRE(results[0]->id == "s1");
}

TEST_CASE("SnippetLibrary: searchByText case insensitive", "[snippet][library]") {
    NF::SnippetLibrary lib;
    lib.addFolder(makeSnipFolder("gen", "General"));
    lib.addSnippet("gen", makeSnippet("s1", "HELLO World", "code"));

    auto results = lib.searchByText("hello");
    REQUIRE(results.size() == 1);
}

TEST_CASE("SnippetLibrary: searchByText empty query", "[snippet][library]") {
    NF::SnippetLibrary lib;
    auto results = lib.searchByText("");
    REQUIRE(results.empty());
}

TEST_CASE("SnippetLibrary: observer on add", "[snippet][library]") {
    NF::SnippetLibrary lib;
    lib.addFolder(makeSnipFolder("gen", "General"));
    int addCount = 0;
    lib.addObserver([&](const NF::SnippetEntry&, bool added) { if (added) ++addCount; });
    lib.addSnippet("gen", makeSnippet("s1", "Hello"));
    REQUIRE(addCount == 1);
}

TEST_CASE("SnippetLibrary: observer on remove", "[snippet][library]") {
    NF::SnippetLibrary lib;
    lib.addFolder(makeSnipFolder("gen", "General"));
    lib.addSnippet("gen", makeSnippet("s1", "Hello"));
    int removeCount = 0;
    lib.addObserver([&](const NF::SnippetEntry&, bool added) { if (!added) ++removeCount; });
    lib.removeSnippet("gen", "s1");
    REQUIRE(removeCount == 1);
}

TEST_CASE("SnippetLibrary: clearObservers", "[snippet][library]") {
    NF::SnippetLibrary lib;
    lib.addFolder(makeSnipFolder("gen", "General"));
    int count = 0;
    lib.addObserver([&](const NF::SnippetEntry&, bool) { ++count; });
    lib.clearObservers();
    lib.addSnippet("gen", makeSnippet("s1", "Hello"));
    REQUIRE(count == 0);
}

TEST_CASE("SnippetLibrary: serialize empty", "[snippet][serialization]") {
    NF::SnippetLibrary lib;
    REQUIRE(lib.serialize().empty());
}

TEST_CASE("SnippetLibrary: serialize round-trip", "[snippet][serialization]") {
    NF::SnippetLibrary lib;
    lib.addFolder(makeSnipFolder("gen", "General"));
    auto s1 = makeSnippet("s1", "Hello", "int x = 42;", NF::SnippetLanguage::Cpp);
    s1.description = "A test snippet";
    s1.addTag("test");
    s1.addTag("math");
    lib.addSnippet("gen", s1);

    std::string text = lib.serialize();
    NF::SnippetLibrary lib2;
    REQUIRE(lib2.deserialize(text));
    REQUIRE(lib2.folderCount() == 1);
    REQUIRE(lib2.totalSnippets() == 1);
    auto* restored = lib2.findFolder("gen")->findSnippet("s1");
    REQUIRE(restored != nullptr);
    REQUIRE(restored->title == "Hello");
    REQUIRE(restored->body == "int x = 42;");
    REQUIRE(restored->language == NF::SnippetLanguage::Cpp);
    REQUIRE(restored->description == "A test snippet");
    REQUIRE(restored->hasTag("test"));
    REQUIRE(restored->hasTag("math"));
    REQUIRE(restored->createdMs == 100);
    REQUIRE(restored->modifiedMs == 200);
}

TEST_CASE("SnippetLibrary: serialize multiline body", "[snippet][serialization]") {
    NF::SnippetLibrary lib;
    lib.addFolder(makeSnipFolder("gen", "General"));
    auto s = makeSnippet("s1", "Multi", "line1\nline2\nline3");
    lib.addSnippet("gen", s);

    std::string text = lib.serialize();
    NF::SnippetLibrary lib2;
    lib2.deserialize(text);
    auto* restored = lib2.findFolder("gen")->findSnippet("s1");
    REQUIRE(restored->body == "line1\nline2\nline3");
}

TEST_CASE("SnippetLibrary: serialize pipe in title", "[snippet][serialization]") {
    NF::SnippetLibrary lib;
    lib.addFolder(makeSnipFolder("gen", "General"));
    auto s = makeSnippet("s1", "Foo|Bar", "code");
    lib.addSnippet("gen", s);

    std::string text = lib.serialize();
    NF::SnippetLibrary lib2;
    lib2.deserialize(text);
    auto* restored = lib2.findFolder("gen")->findSnippet("s1");
    REQUIRE(restored->title == "Foo|Bar");
}

TEST_CASE("SnippetLibrary: deserialize empty", "[snippet][serialization]") {
    NF::SnippetLibrary lib;
    REQUIRE(lib.deserialize(""));
    REQUIRE(lib.folderCount() == 0);
}

TEST_CASE("SnippetLibrary: clear", "[snippet][library]") {
    NF::SnippetLibrary lib;
    lib.addFolder(makeSnipFolder("gen", "General"));
    lib.addSnippet("gen", makeSnippet("s1", "Hello"));
    lib.clear();
    REQUIRE(lib.folderCount() == 0);
}

// ═════════════════════════════════════════════════════════════════
// Integration
// ═════════════════════════════════════════════════════════════════

TEST_CASE("Integration: multi-folder snippet library with search", "[snippet][integration]") {
    NF::SnippetLibrary lib;
    lib.addFolder(makeSnipFolder("cpp", "C++ Snippets"));
    lib.addFolder(makeSnipFolder("shader", "Shader Snippets"));

    auto s1 = makeSnippet("cpp.matrix", "Matrix Multiply", "mat4 mul(mat4 a, mat4 b) { ... }", NF::SnippetLanguage::Cpp);
    s1.addTag("math");
    s1.addTag("matrix");
    lib.addSnippet("cpp", s1);

    auto s2 = makeSnippet("shader.frag", "Fragment Shader", "void main() { gl_FragColor = vec4(1); }", NF::SnippetLanguage::GLSL);
    s2.addTag("shader");
    lib.addSnippet("shader", s2);

    REQUIRE(lib.totalSnippets() == 2);
    REQUIRE(lib.searchByTag("math").size() == 1);
    REQUIRE(lib.searchByLanguage(NF::SnippetLanguage::GLSL).size() == 1);
    REQUIRE(lib.searchByText("FragColor").size() == 1);
}

TEST_CASE("Integration: serialize/deserialize multi-folder with tags", "[snippet][integration]") {
    NF::SnippetLibrary lib;
    lib.addFolder(makeSnipFolder("f1", "Folder 1"));
    lib.addFolder(makeSnipFolder("f2", "Folder 2"));

    auto s1 = makeSnippet("a", "A", "body_a", NF::SnippetLanguage::Python);
    s1.addTag("tag1");
    lib.addSnippet("f1", s1);

    auto s2 = makeSnippet("b", "B", "line1\nline2", NF::SnippetLanguage::Lua);
    s2.addTag("tag2");
    s2.addTag("tag3");
    lib.addSnippet("f2", s2);

    std::string text = lib.serialize();
    NF::SnippetLibrary lib2;
    lib2.deserialize(text);

    REQUIRE(lib2.folderCount() == 2);
    REQUIRE(lib2.totalSnippets() == 2);

    auto* ra = lib2.findFolder("f1")->findSnippet("a");
    REQUIRE(ra->language == NF::SnippetLanguage::Python);
    REQUIRE(ra->hasTag("tag1"));

    auto* rb = lib2.findFolder("f2")->findSnippet("b");
    REQUIRE(rb->body == "line1\nline2");
    REQUIRE(rb->hasTag("tag2"));
    REQUIRE(rb->hasTag("tag3"));
}
