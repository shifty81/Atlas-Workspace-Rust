#include <catch2/catch_test_macros.hpp>
#include "NF/Editor/Editor.h"

using namespace NF;

// ── S15 Tests: Scripting Console ─────────────────────────────────────────────

TEST_CASE("ScriptLanguage names", "[Editor][S15]") {
    REQUIRE(std::string(scriptLanguageName(ScriptLanguage::Lua))        == "Lua");
    REQUIRE(std::string(scriptLanguageName(ScriptLanguage::Python))     == "Python");
    REQUIRE(std::string(scriptLanguageName(ScriptLanguage::JavaScript)) == "JavaScript");
    REQUIRE(std::string(scriptLanguageName(ScriptLanguage::TypeScript)) == "TypeScript");
    REQUIRE(std::string(scriptLanguageName(ScriptLanguage::Bash))       == "Bash");
    REQUIRE(std::string(scriptLanguageName(ScriptLanguage::Ruby))       == "Ruby");
    REQUIRE(std::string(scriptLanguageName(ScriptLanguage::CSharp))     == "CSharp");
    REQUIRE(std::string(scriptLanguageName(ScriptLanguage::DSL))        == "DSL");
}

TEST_CASE("ScriptVariable isValid + isReadOnly + set", "[Editor][S15]") {
    ScriptVariable v;
    REQUIRE_FALSE(v.isValid()); // no name

    v.name     = "speed";
    v.value    = "10";
    v.typeName = "float";
    REQUIRE(v.isValid());
    REQUIRE_FALSE(v.isReadOnly());

    REQUIRE(v.set("20"));
    REQUIRE(v.value == "20");

    v.readOnly = true;
    REQUIRE(v.isReadOnly());
    REQUIRE_FALSE(v.set("30")); // readonly blocks set
    REQUIRE(v.value == "20");
}

TEST_CASE("ScriptResult isSuccess + hasOutput + hasError", "[Editor][S15]") {
    ScriptResult r;
    REQUIRE(r.isSuccess());  // default exitCode=0, no error
    REQUIRE_FALSE(r.hasOutput());
    REQUIRE_FALSE(r.hasError());

    r.output = "hello";
    REQUIRE(r.hasOutput());

    r.errorMessage = "syntax error";
    r.exitCode     = 1;
    REQUIRE_FALSE(r.isSuccess());
    REQUIRE(r.hasError());
}

TEST_CASE("ScriptContext setVariable + getVariable + hasVariable", "[Editor][S15]") {
    ScriptContext ctx("main", ScriptLanguage::Python);
    REQUIRE(ctx.name()     == "main");
    REQUIRE(ctx.language() == ScriptLanguage::Python);

    ScriptVariable v; v.name = "x"; v.value = "5"; v.typeName = "int";
    REQUIRE(ctx.setVariable(v));
    REQUIRE(ctx.hasVariable("x"));
    REQUIRE(ctx.variableCount() == 1);

    auto* found = ctx.getVariable("x");
    REQUIRE(found != nullptr);
    REQUIRE(found->value == "5");

    REQUIRE(ctx.getVariable("y") == nullptr);
}

TEST_CASE("ScriptContext update existing variable", "[Editor][S15]") {
    ScriptContext ctx("ctx1");
    ScriptVariable v; v.name = "count"; v.value = "1";
    ctx.setVariable(v);

    ScriptVariable updated; updated.name = "count"; updated.value = "99";
    REQUIRE(ctx.setVariable(updated));
    REQUIRE(ctx.variableCount() == 1); // no duplicate
    REQUIRE(ctx.getVariable("count")->value == "99");
}

TEST_CASE("ScriptContext readonly variable blocks update", "[Editor][S15]") {
    ScriptContext ctx("ctx2");
    ScriptVariable v; v.name = "PI"; v.value = "3.14"; v.readOnly = true;
    ctx.setVariable(v);

    ScriptVariable updated; updated.name = "PI"; updated.value = "4.0"; updated.readOnly = false;
    REQUIRE_FALSE(ctx.setVariable(updated)); // blocked by readOnly
    REQUIRE(ctx.getVariable("PI")->value == "3.14");
}

TEST_CASE("ScriptContext removeVariable + clear", "[Editor][S15]") {
    ScriptContext ctx("ctx3");
    ScriptVariable v1; v1.name = "a"; v1.value = "1";
    ScriptVariable v2; v2.name = "b"; v2.value = "2";
    ctx.setVariable(v1);
    ctx.setVariable(v2);
    REQUIRE(ctx.variableCount() == 2);

    REQUIRE(ctx.removeVariable("a"));
    REQUIRE(ctx.variableCount() == 1);
    REQUIRE_FALSE(ctx.removeVariable("nonexistent"));

    ctx.clear();
    REQUIRE(ctx.variableCount() == 0);
}

TEST_CASE("ScriptConsole init/shutdown", "[Editor][S15]") {
    ScriptConsole console;
    REQUIRE_FALSE(console.isInitialized());
    console.init();
    REQUIRE(console.isInitialized());
    console.shutdown();
    REQUIRE_FALSE(console.isInitialized());
}

TEST_CASE("ScriptConsole createContext + contextByName", "[Editor][S15]") {
    ScriptConsole console;
    console.init();

    auto* ctx = console.createContext("main", ScriptLanguage::Lua);
    REQUIRE(ctx != nullptr);
    REQUIRE(ctx->name()     == "main");
    REQUIRE(ctx->language() == ScriptLanguage::Lua);
    REQUIRE(console.totalContexts() == 1);

    REQUIRE(console.createContext("main") == nullptr); // duplicate name
    REQUIRE(console.totalContexts() == 1);

    REQUIRE(console.contextByName("main") != nullptr);
    REQUIRE(console.contextByName("other") == nullptr);
}

TEST_CASE("ScriptConsole createContext before init returns nullptr", "[Editor][S15]") {
    ScriptConsole console;
    REQUIRE(console.createContext("ctx") == nullptr);
}

TEST_CASE("ScriptConsole execute success", "[Editor][S15]") {
    ScriptConsole console;
    console.init();

    ScriptResult r = console.execute("print('hello')");
    REQUIRE(r.isSuccess());
    REQUIRE(r.hasOutput());
    REQUIRE(console.executionCount() == 1);
    REQUIRE(console.errorCount() == 0);
}

TEST_CASE("ScriptConsole execute empty script returns error", "[Editor][S15]") {
    ScriptConsole console;
    console.init();

    ScriptResult r = console.execute("");
    REQUIRE_FALSE(r.isSuccess());
    REQUIRE(r.hasError());
    REQUIRE(console.errorCount() == 1);
    REQUIRE(console.executionCount() == 0);
}

TEST_CASE("ScriptConsole execute without init returns error", "[Editor][S15]") {
    ScriptConsole console;
    ScriptResult r = console.execute("some script");
    REQUIRE_FALSE(r.isSuccess());
    REQUIRE(console.errorCount() == 1);
}

TEST_CASE("ScriptConsole execute with context", "[Editor][S15]") {
    ScriptConsole console;
    console.init();

    auto* ctx = console.createContext("runtime", ScriptLanguage::JavaScript);
    ScriptVariable v; v.name = "x"; v.value = "42";
    ctx->setVariable(v);

    ScriptResult r = console.execute("return x + 1", ctx);
    REQUIRE(r.isSuccess());
    REQUIRE(console.executionCount() == 1);
}

TEST_CASE("ScriptConsole tick + counters", "[Editor][S15]") {
    ScriptConsole console;
    console.init();
    console.tick(0.016f);
    console.tick(0.016f);
    console.tick(0.016f);
    REQUIRE(console.tickCount() == 3);

    console.execute("x = 1");
    console.execute("y = 2");
    REQUIRE(console.executionCount() == 2);
}
