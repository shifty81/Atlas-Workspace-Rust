#include <catch2/catch_test_macros.hpp>
#include "NF/Editor/Editor.h"

using namespace NF;

// ── S76: IDEIntegration ──────────────────────────────────────────

TEST_CASE("SourceFileType Header classified from .h extension (via indexFile)", "[Editor][S76]") {
    ProjectIndexer indexer;
    indexer.indexFile("Core.h", SourceFileType::Header, "Core");
    auto headers = indexer.findFilesByType(SourceFileType::Header);
    REQUIRE(headers.size() == 1);
    REQUIRE(headers[0]->fileType == SourceFileType::Header);
}

TEST_CASE("ProjectIndexer starts empty", "[Editor][S76]") {
    ProjectIndexer indexer;
    REQUIRE(indexer.fileCount() == 0);
}

TEST_CASE("ProjectIndexer indexFile adds entry", "[Editor][S76]") {
    ProjectIndexer indexer;
    indexer.indexFile("main.cpp", SourceFileType::Source, "Core");
    REQUIRE(indexer.fileCount() == 1);
}

TEST_CASE("ProjectIndexer indexFile adds multiple entries", "[Editor][S76]") {
    ProjectIndexer indexer;
    indexer.indexFile("a.h",    SourceFileType::Header, "ModA");
    indexer.indexFile("b.cpp",  SourceFileType::Source, "ModA");
    indexer.indexFile("fx.glsl",SourceFileType::Shader, "Render");
    REQUIRE(indexer.fileCount() == 3);
}

TEST_CASE("ProjectIndexer findFilesByType filters correctly", "[Editor][S76]") {
    ProjectIndexer indexer;
    indexer.indexFile("a.h",   SourceFileType::Header, "M");
    indexer.indexFile("b.cpp", SourceFileType::Source, "M");
    indexer.indexFile("c.h",   SourceFileType::Header, "M");
    auto headers = indexer.findFilesByType(SourceFileType::Header);
    REQUIRE(headers.size() == 2);
    auto sources = indexer.findFilesByType(SourceFileType::Source);
    REQUIRE(sources.size() == 1);
}

TEST_CASE("ProjectIndexer findFilesByModule filters correctly", "[Editor][S76]") {
    ProjectIndexer indexer;
    indexer.indexFile("a.h",  SourceFileType::Header, "Core");
    indexer.indexFile("b.cpp",SourceFileType::Source, "Render");
    indexer.indexFile("c.h",  SourceFileType::Header, "Core");
    auto core = indexer.findFilesByModule("Core");
    REQUIRE(core.size() == 2);
    auto render = indexer.findFilesByModule("Render");
    REQUIRE(render.size() == 1);
}

TEST_CASE("ProjectIndexer findFilesByName matches substring", "[Editor][S76]") {
    ProjectIndexer indexer;
    indexer.indexFile("Source/Engine/Engine.h",   SourceFileType::Header, "Engine");
    indexer.indexFile("Source/Renderer/Render.h", SourceFileType::Header, "Renderer");
    auto matches = indexer.findFilesByName("Engine");
    REQUIRE(matches.size() == 1);
}

TEST_CASE("ProjectIndexer clear empties all files", "[Editor][S76]") {
    ProjectIndexer indexer;
    indexer.indexFile("a.h", SourceFileType::Header, "M");
    indexer.indexFile("b.h", SourceFileType::Header, "M");
    indexer.clear();
    REQUIRE(indexer.fileCount() == 0);
}

TEST_CASE("ProjectIndexer addSymbol and findSymbol work", "[Editor][S76]") {
    ProjectIndexer indexer;
    indexer.indexFile("engine.cpp", SourceFileType::Source, "Engine");
    indexer.addSymbol("engine.cpp", "Engine::init");
    indexer.addSymbol("engine.cpp", "Engine::shutdown");
    auto results = indexer.findSymbol("Engine::init");
    REQUIRE(results.size() == 1);
    auto notFound = indexer.findSymbol("ghost");
    REQUIRE(notFound.empty());
}

TEST_CASE("ProjectIndexer findSymbol returns all files containing symbol", "[Editor][S76]") {
    ProjectIndexer indexer;
    indexer.indexFile("a.cpp", SourceFileType::Source, "M");
    indexer.indexFile("b.cpp", SourceFileType::Source, "M");
    indexer.addSymbol("a.cpp", "foo");
    indexer.addSymbol("b.cpp", "foo");
    auto results = indexer.findSymbol("foo");
    REQUIRE(results.size() == 2);
}

TEST_CASE("SymbolKind available as enum", "[Editor][S76]") {
    SymbolKind k = SymbolKind::Function;
    REQUIRE(k == SymbolKind::Function);
    k = SymbolKind::Class;
    REQUIRE(k == SymbolKind::Class);
}

TEST_CASE("CodeNavigator starts empty", "[Editor][S76]") {
    CodeNavigator nav;
    REQUIRE(nav.entryCount() == 0);
}

TEST_CASE("CodeNavigator addEntry increases entryCount", "[Editor][S76]") {
    CodeNavigator nav;
    NavigationEntry e;
    e.symbol = "MyClass";
    e.kind = SymbolKind::Class;
    e.filePath = "MyClass.h";
    e.line = 10;
    nav.addEntry(e);
    REQUIRE(nav.entryCount() == 1);
}

TEST_CASE("CodeNavigator goToDefinition returns target for known symbol", "[Editor][S76]") {
    CodeNavigator nav;
    NavigationEntry e;
    e.symbol = "Foo";
    e.kind = SymbolKind::Function;
    e.filePath = "foo.cpp";
    e.line = 42;
    nav.addEntry(e);
    auto target = nav.goToDefinition("Foo");
    REQUIRE(target.has_value());
    REQUIRE(target->symbolName == "Foo");
    REQUIRE(target->filePath == "foo.cpp");
    REQUIRE(target->line == 42);
}

TEST_CASE("CodeNavigator goToDefinition returns nullopt for unknown symbol", "[Editor][S76]") {
    CodeNavigator nav;
    REQUIRE_FALSE(nav.goToDefinition("ghost").has_value());
}

TEST_CASE("CodeNavigator findReferences returns all matches", "[Editor][S76]") {
    CodeNavigator nav;
    NavigationEntry e1; e1.symbol = "Bar"; e1.kind = SymbolKind::Function; e1.filePath = "a.cpp"; e1.line = 1;
    NavigationEntry e2; e2.symbol = "Bar"; e2.kind = SymbolKind::Function; e2.filePath = "b.cpp"; e2.line = 2;
    nav.addEntry(e1);
    nav.addEntry(e2);
    auto refs = nav.findReferences("Bar");
    REQUIRE(refs.size() == 2);
}

TEST_CASE("CodeNavigator findSymbolsByKind filters correctly", "[Editor][S76]") {
    CodeNavigator nav;
    NavigationEntry e1; e1.symbol = "MyClass"; e1.kind = SymbolKind::Class; e1.filePath = "a.h"; e1.line = 1;
    NavigationEntry e2; e2.symbol = "myFunc"; e2.kind = SymbolKind::Function; e2.filePath = "b.cpp"; e2.line = 2;
    NavigationEntry e3; e3.symbol = "OtherClass"; e3.kind = SymbolKind::Class; e3.filePath = "c.h"; e3.line = 3;
    nav.addEntry(e1); nav.addEntry(e2); nav.addEntry(e3);
    auto classes = nav.findSymbolsByKind(SymbolKind::Class);
    REQUIRE(classes.size() == 2);
    auto funcs = nav.findSymbolsByKind(SymbolKind::Function);
    REQUIRE(funcs.size() == 1);
}

TEST_CASE("CodeNavigator searchSymbols finds by substring", "[Editor][S76]") {
    CodeNavigator nav;
    NavigationEntry e1; e1.symbol = "Engine::init"; e1.kind = SymbolKind::Function; e1.filePath = "e.cpp"; e1.line = 1;
    NavigationEntry e2; e2.symbol = "Engine::shutdown"; e2.kind = SymbolKind::Function; e2.filePath = "e.cpp"; e2.line = 2;
    NavigationEntry e3; e3.symbol = "Renderer::init"; e3.kind = SymbolKind::Function; e3.filePath = "r.cpp"; e3.line = 1;
    nav.addEntry(e1); nav.addEntry(e2); nav.addEntry(e3);
    auto engineSymbols = nav.searchSymbols("Engine");
    REQUIRE(engineSymbols.size() == 2);
    auto initSymbols = nav.searchSymbols("init");
    REQUIRE(initSymbols.size() == 2);
}

TEST_CASE("CodeNavigator clear removes all entries", "[Editor][S76]") {
    CodeNavigator nav;
    NavigationEntry e; e.symbol = "X"; e.kind = SymbolKind::Variable; e.filePath = "x.cpp"; e.line = 1;
    nav.addEntry(e);
    nav.clear();
    REQUIRE(nav.entryCount() == 0);
}

TEST_CASE("BreadcrumbTrail starts empty", "[Editor][S76]") {
    BreadcrumbTrail trail;
    REQUIRE(trail.depth() == 0);
    REQUIRE(trail.current() == nullptr);
}

TEST_CASE("BreadcrumbTrail push adds item", "[Editor][S76]") {
    BreadcrumbTrail trail;
    BreadcrumbItem item;
    item.label = "Engine::init";
    item.filePath = "engine.cpp";
    item.line = 10;
    trail.push(item);
    REQUIRE(trail.depth() == 1);
    REQUIRE(trail.current() != nullptr);
    REQUIRE(trail.current()->label == "Engine::init");
}

TEST_CASE("BreadcrumbTrail pop returns last item", "[Editor][S76]") {
    BreadcrumbTrail trail;
    BreadcrumbItem item;
    item.label = "MyFunc";
    item.filePath = "my.cpp";
    item.line = 5;
    trail.push(item);
    auto popped = trail.pop();
    REQUIRE(popped.has_value());
    REQUIRE(popped->label == "MyFunc");
    REQUIRE(trail.depth() == 0);
}

TEST_CASE("BreadcrumbTrail pop returns nullopt when empty", "[Editor][S76]") {
    BreadcrumbTrail trail;
    REQUIRE_FALSE(trail.pop().has_value());
}

TEST_CASE("BreadcrumbTrail maintains LIFO order", "[Editor][S76]") {
    BreadcrumbTrail trail;
    BreadcrumbItem a; a.label = "A"; a.filePath = "a.cpp"; a.line = 1;
    BreadcrumbItem b; b.label = "B"; b.filePath = "b.cpp"; b.line = 2;
    trail.push(a);
    trail.push(b);
    REQUIRE(trail.current()->label == "B");
    trail.pop();
    REQUIRE(trail.current()->label == "A");
}

TEST_CASE("BreadcrumbTrail clear removes all items", "[Editor][S76]") {
    BreadcrumbTrail trail;
    BreadcrumbItem a; a.label = "A"; a.filePath = "a.cpp"; a.line = 1;
    trail.push(a);
    trail.clear();
    REQUIRE(trail.depth() == 0);
}

TEST_CASE("IDEService starts uninitialized", "[Editor][S76]") {
    IDEService svc;
    REQUIRE_FALSE(svc.isInitialized());
}

TEST_CASE("IDEService init sets initialized", "[Editor][S76]") {
    IDEService svc;
    svc.init();
    REQUIRE(svc.isInitialized());
}

TEST_CASE("IDEService indexer accessible after init", "[Editor][S76]") {
    IDEService svc;
    svc.init();
    svc.indexer().indexFile("main.cpp", SourceFileType::Source, "App");
    REQUIRE(svc.indexer().fileCount() == 1);
}

TEST_CASE("IDEService navigator accessible after init", "[Editor][S76]") {
    IDEService svc;
    svc.init();
    NavigationEntry e; e.symbol = "Foo"; e.kind = SymbolKind::Function; e.filePath = "foo.cpp"; e.line = 1;
    svc.navigator().addEntry(e);
    REQUIRE(svc.navigator().entryCount() == 1);
}

TEST_CASE("IDEService navigateTo pushes breadcrumb", "[Editor][S76]") {
    IDEService svc;
    svc.init();
    svc.navigateTo("engine.cpp", 42, "Engine::init");
    REQUIRE(svc.breadcrumbs().depth() == 1);
    REQUIRE(svc.breadcrumbs().current()->label == "Engine::init");
}

TEST_CASE("IDEService goBack pops breadcrumb", "[Editor][S76]") {
    IDEService svc;
    svc.init();
    svc.navigateTo("engine.cpp", 1, "Engine::init");
    REQUIRE(svc.goBack());
    REQUIRE(svc.breadcrumbs().depth() == 0);
}

TEST_CASE("IDEService goBack returns false when history empty", "[Editor][S76]") {
    IDEService svc;
    svc.init();
    REQUIRE_FALSE(svc.goBack());
}

TEST_CASE("IDEService shutdown clears state", "[Editor][S76]") {
    IDEService svc;
    svc.init();
    svc.indexer().indexFile("a.cpp", SourceFileType::Source, "M");
    svc.shutdown();
    REQUIRE_FALSE(svc.isInitialized());
    REQUIRE(svc.indexer().fileCount() == 0);
}
