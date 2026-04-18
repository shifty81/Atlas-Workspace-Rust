#include <catch2/catch_test_macros.hpp>
#include <catch2/catch_approx.hpp>
#include "NF/Editor/Editor.h"

using namespace NF;
using Catch::Approx;

// ── S72: Scripting ───────────────────────────────────────────────

TEST_CASE("ScriptLanguage names are correct", "[Editor][S72]") {
    REQUIRE(std::string(scriptLanguageName(ScriptLanguage::Lua))        == "Lua");
    REQUIRE(std::string(scriptLanguageName(ScriptLanguage::Python))     == "Python");
    REQUIRE(std::string(scriptLanguageName(ScriptLanguage::JavaScript)) == "JavaScript");
    REQUIRE(std::string(scriptLanguageName(ScriptLanguage::TypeScript)) == "TypeScript");
    REQUIRE(std::string(scriptLanguageName(ScriptLanguage::Bash))       == "Bash");
    REQUIRE(std::string(scriptLanguageName(ScriptLanguage::Ruby))       == "Ruby");
    REQUIRE(std::string(scriptLanguageName(ScriptLanguage::CSharp))     == "CSharp");
    REQUIRE(std::string(scriptLanguageName(ScriptLanguage::DSL))        == "DSL");
}

TEST_CASE("ScriptVariable defaults are invalid", "[Editor][S72]") {
    ScriptVariable v;
    REQUIRE_FALSE(v.isValid());
}

TEST_CASE("ScriptVariable valid when name is set", "[Editor][S72]") {
    ScriptVariable v;
    v.name = "score";
    REQUIRE(v.isValid());
}

TEST_CASE("ScriptVariable set updates value", "[Editor][S72]") {
    ScriptVariable v;
    v.name = "score";
    v.value = "0";
    REQUIRE(v.set("42"));
    REQUIRE(v.value == "42");
}

TEST_CASE("ScriptVariable set fails when readOnly", "[Editor][S72]") {
    ScriptVariable v;
    v.name = "score";
    v.value = "0";
    v.readOnly = true;
    REQUIRE_FALSE(v.set("99"));
    REQUIRE(v.value == "0");
}

TEST_CASE("ScriptVariable isReadOnly reflects flag", "[Editor][S72]") {
    ScriptVariable v;
    v.name = "x";
    REQUIRE_FALSE(v.isReadOnly());
    v.readOnly = true;
    REQUIRE(v.isReadOnly());
}

TEST_CASE("ScriptResult defaults are success", "[Editor][S72]") {
    ScriptResult r;
    REQUIRE(r.exitCode == 0);
    REQUIRE(r.isSuccess());
    REQUIRE_FALSE(r.hasOutput());
    REQUIRE_FALSE(r.hasError());
}

TEST_CASE("ScriptResult isSuccess false when errorMessage set", "[Editor][S72]") {
    ScriptResult r;
    r.errorMessage = "syntax error";
    REQUIRE_FALSE(r.isSuccess());
    REQUIRE(r.hasError());
}

TEST_CASE("ScriptResult isSuccess false when exitCode non-zero", "[Editor][S72]") {
    ScriptResult r;
    r.exitCode = 1;
    REQUIRE_FALSE(r.isSuccess());
}

TEST_CASE("ScriptResult hasOutput true when output set", "[Editor][S72]") {
    ScriptResult r;
    r.output = "hello";
    REQUIRE(r.hasOutput());
}

TEST_CASE("ScriptContext starts with correct name and language", "[Editor][S72]") {
    ScriptContext ctx("main", ScriptLanguage::Python);
    REQUIRE(ctx.name() == "main");
    REQUIRE(ctx.language() == ScriptLanguage::Python);
}

TEST_CASE("ScriptContext variableCount starts at zero", "[Editor][S72]") {
    ScriptContext ctx("ctx");
    REQUIRE(ctx.variableCount() == 0);
}

TEST_CASE("ScriptContext setVariable adds new variable", "[Editor][S72]") {
    ScriptContext ctx("ctx");
    ScriptVariable v;
    v.name = "x";
    v.value = "10";
    REQUIRE(ctx.setVariable(v));
    REQUIRE(ctx.variableCount() == 1);
}

TEST_CASE("ScriptContext setVariable updates existing variable", "[Editor][S72]") {
    ScriptContext ctx("ctx");
    ScriptVariable v;
    v.name = "x";
    v.value = "10";
    ctx.setVariable(v);
    v.value = "20";
    REQUIRE(ctx.setVariable(v));
    REQUIRE(ctx.variableCount() == 1);
    REQUIRE(ctx.getVariable("x")->value == "20");
}

TEST_CASE("ScriptContext setVariable fails when updating readOnly variable", "[Editor][S72]") {
    ScriptContext ctx("ctx");
    ScriptVariable v;
    v.name = "pi";
    v.value = "3.14";
    v.readOnly = true;
    ctx.setVariable(v);
    ScriptVariable v2;
    v2.name = "pi";
    v2.value = "4.0";
    REQUIRE_FALSE(ctx.setVariable(v2));
}

TEST_CASE("ScriptContext getVariable returns nullptr for missing", "[Editor][S72]") {
    ScriptContext ctx("ctx");
    REQUIRE(ctx.getVariable("missing") == nullptr);
}

TEST_CASE("ScriptContext hasVariable returns true after add", "[Editor][S72]") {
    ScriptContext ctx("ctx");
    ScriptVariable v;
    v.name = "flag";
    ctx.setVariable(v);
    REQUIRE(ctx.hasVariable("flag"));
    REQUIRE_FALSE(ctx.hasVariable("other"));
}

TEST_CASE("ScriptContext removeVariable removes entry", "[Editor][S72]") {
    ScriptContext ctx("ctx");
    ScriptVariable v;
    v.name = "temp";
    ctx.setVariable(v);
    REQUIRE(ctx.removeVariable("temp"));
    REQUIRE(ctx.variableCount() == 0);
}

TEST_CASE("ScriptContext removeVariable returns false for missing", "[Editor][S72]") {
    ScriptContext ctx("ctx");
    REQUIRE_FALSE(ctx.removeVariable("nope"));
}

TEST_CASE("ScriptContext clear removes all variables", "[Editor][S72]") {
    ScriptContext ctx("ctx");
    ScriptVariable v1; v1.name = "a";
    ScriptVariable v2; v2.name = "b";
    ctx.setVariable(v1);
    ctx.setVariable(v2);
    ctx.clear();
    REQUIRE(ctx.variableCount() == 0);
}

TEST_CASE("ScriptContext kMaxVariables is 128", "[Editor][S72]") {
    REQUIRE(ScriptContext::kMaxVariables == 128);
}

TEST_CASE("ScriptConsole starts uninitialized", "[Editor][S72]") {
    ScriptConsole console;
    REQUIRE_FALSE(console.isInitialized());
}

TEST_CASE("ScriptConsole init sets initialized", "[Editor][S72]") {
    ScriptConsole console;
    console.init();
    REQUIRE(console.isInitialized());
}

TEST_CASE("ScriptConsole createContext before init returns nullptr", "[Editor][S72]") {
    ScriptConsole console;
    REQUIRE(console.createContext("ctx") == nullptr);
}

TEST_CASE("ScriptConsole createContext creates a context", "[Editor][S72]") {
    ScriptConsole console;
    console.init();
    auto* ctx = console.createContext("main");
    REQUIRE(ctx != nullptr);
    REQUIRE(ctx->name() == "main");
    REQUIRE(console.totalContexts() == 1);
}

TEST_CASE("ScriptConsole createContext rejects duplicate name", "[Editor][S72]") {
    ScriptConsole console;
    console.init();
    console.createContext("main");
    REQUIRE(console.createContext("main") == nullptr);
    REQUIRE(console.totalContexts() == 1);
}

TEST_CASE("ScriptConsole contextByName returns context", "[Editor][S72]") {
    ScriptConsole console;
    console.init();
    console.createContext("editor");
    REQUIRE(console.contextByName("editor") != nullptr);
    REQUIRE(console.contextByName("other") == nullptr);
}

TEST_CASE("ScriptConsole execute when uninitialized returns error", "[Editor][S72]") {
    ScriptConsole console;
    auto result = console.execute("print('hi')");
    REQUIRE_FALSE(result.isSuccess());
    REQUIRE(result.hasError());
    REQUIRE(console.errorCount() == 1);
}

TEST_CASE("ScriptConsole execute with empty code returns error", "[Editor][S72]") {
    ScriptConsole console;
    console.init();
    auto result = console.execute("");
    REQUIRE_FALSE(result.isSuccess());
    REQUIRE(console.errorCount() == 1);
}

TEST_CASE("ScriptConsole execute with valid code succeeds", "[Editor][S72]") {
    ScriptConsole console;
    console.init();
    auto result = console.execute("print('hello')");
    REQUIRE(result.isSuccess());
    REQUIRE(result.hasOutput());
    REQUIRE(console.executionCount() == 1);
}

TEST_CASE("ScriptConsole executionCount increments on success", "[Editor][S72]") {
    ScriptConsole console;
    console.init();
    console.execute("1+1");
    console.execute("2+2");
    REQUIRE(console.executionCount() == 2);
}

TEST_CASE("ScriptConsole tick increments tickCount", "[Editor][S72]") {
    ScriptConsole console;
    console.init();
    REQUIRE(console.tickCount() == 0);
    console.tick(0.016f);
    REQUIRE(console.tickCount() == 1);
}

TEST_CASE("ScriptConsole tick does nothing when uninitialized", "[Editor][S72]") {
    ScriptConsole console;
    console.tick(0.016f);
    REQUIRE(console.tickCount() == 0);
}

TEST_CASE("ScriptConsole shutdown clears state", "[Editor][S72]") {
    ScriptConsole console;
    console.init();
    console.createContext("ctx");
    console.execute("code");
    console.shutdown();
    REQUIRE_FALSE(console.isInitialized());
    REQUIRE(console.totalContexts() == 0);
    REQUIRE(console.executionCount() == 0);
}

TEST_CASE("ScriptConsole kMaxContexts is 16", "[Editor][S72]") {
    REQUIRE(ScriptConsole::kMaxContexts == 16);
}
