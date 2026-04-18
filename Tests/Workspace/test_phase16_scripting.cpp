// Tests/Workspace/test_phase16_scripting.cpp
// Phase 16 — Workspace Scripting and Automation
//
// Tests for:
//   1. ScriptParamType — enum name helpers
//   2. ScriptParam — typed parameter descriptor
//   3. ScriptBinding — function binding with parameters and handler
//   4. ScriptContext — variable scope, output, error state
//   5. ScriptExecStatus / ScriptExecResult — execution result types
//   6. ScriptEngine — binding registration and execution
//   7. AutomationStepStatus / AutomationStep — step types
//   8. AutomationTaskState / AutomationTask — task execution sequence
//   9. Integration — engine + context + task working together

#include <catch2/catch_test_macros.hpp>
#include "NF/Workspace/WorkspaceScripting.h"
#include <string>
#include <vector>

using namespace NF;

// ═════════════════════════════════════════════════════════════════
// SECTION 1 — Enums
// ═════════════════════════════════════════════════════════════════

TEST_CASE("scriptParamTypeName returns correct strings", "[Phase16][ParamType]") {
    CHECK(std::string(scriptParamTypeName(ScriptParamType::Void))   == "Void");
    CHECK(std::string(scriptParamTypeName(ScriptParamType::Bool))   == "Bool");
    CHECK(std::string(scriptParamTypeName(ScriptParamType::Int))    == "Int");
    CHECK(std::string(scriptParamTypeName(ScriptParamType::Float))  == "Float");
    CHECK(std::string(scriptParamTypeName(ScriptParamType::String)) == "String");
    CHECK(std::string(scriptParamTypeName(ScriptParamType::Path))   == "Path");
    CHECK(std::string(scriptParamTypeName(ScriptParamType::Id))     == "Id");
    CHECK(std::string(scriptParamTypeName(ScriptParamType::Custom)) == "Custom");
}

TEST_CASE("scriptExecStatusName returns correct strings", "[Phase16][ExecStatus]") {
    CHECK(std::string(scriptExecStatusName(ScriptExecStatus::Success))        == "Success");
    CHECK(std::string(scriptExecStatusName(ScriptExecStatus::NotFound))       == "NotFound");
    CHECK(std::string(scriptExecStatusName(ScriptExecStatus::InvalidArgs))    == "InvalidArgs");
    CHECK(std::string(scriptExecStatusName(ScriptExecStatus::HandlerFailed))  == "HandlerFailed");
    CHECK(std::string(scriptExecStatusName(ScriptExecStatus::BindingInvalid)) == "BindingInvalid");
}

TEST_CASE("automationStepStatusName returns correct strings", "[Phase16][StepStatus]") {
    CHECK(std::string(automationStepStatusName(AutomationStepStatus::Pending))   == "Pending");
    CHECK(std::string(automationStepStatusName(AutomationStepStatus::Running))   == "Running");
    CHECK(std::string(automationStepStatusName(AutomationStepStatus::Succeeded)) == "Succeeded");
    CHECK(std::string(automationStepStatusName(AutomationStepStatus::Failed))    == "Failed");
    CHECK(std::string(automationStepStatusName(AutomationStepStatus::Skipped))   == "Skipped");
}

TEST_CASE("automationTaskStateName returns correct strings", "[Phase16][TaskState]") {
    CHECK(std::string(automationTaskStateName(AutomationTaskState::Idle))      == "Idle");
    CHECK(std::string(automationTaskStateName(AutomationTaskState::Running))   == "Running");
    CHECK(std::string(automationTaskStateName(AutomationTaskState::Completed)) == "Completed");
    CHECK(std::string(automationTaskStateName(AutomationTaskState::Failed))    == "Failed");
    CHECK(std::string(automationTaskStateName(AutomationTaskState::Aborted))   == "Aborted");
}

// ═════════════════════════════════════════════════════════════════
// SECTION 2 — ScriptParam
// ═════════════════════════════════════════════════════════════════

TEST_CASE("ScriptParam default is invalid", "[Phase16][Param]") {
    ScriptParam p;
    CHECK_FALSE(p.isValid());
}

TEST_CASE("ScriptParam valid construction", "[Phase16][Param]") {
    ScriptParam p;
    p.name = "count";
    p.type = ScriptParamType::Int;
    p.defaultValue = "10";
    p.required = false;

    CHECK(p.isValid());
    CHECK(p.name == "count");
    CHECK(p.type == ScriptParamType::Int);
    CHECK(p.defaultValue == "10");
    CHECK_FALSE(p.required);
}

TEST_CASE("ScriptParam void type is invalid", "[Phase16][Param]") {
    ScriptParam p;
    p.name = "x";
    p.type = ScriptParamType::Void;
    CHECK_FALSE(p.isValid());
}

TEST_CASE("ScriptParam equality", "[Phase16][Param]") {
    ScriptParam a;
    a.name = "x"; a.type = ScriptParamType::Int;
    ScriptParam b = a;
    CHECK(a == b);

    b.name = "y";
    CHECK(a != b);
}

// ═════════════════════════════════════════════════════════════════
// SECTION 3 — ScriptBinding
// ═════════════════════════════════════════════════════════════════

TEST_CASE("ScriptBinding default is invalid", "[Phase16][Binding]") {
    ScriptBinding sb;
    CHECK_FALSE(sb.isValid());
    CHECK(sb.paramCount() == 0);
    CHECK(sb.returnType() == ScriptParamType::Void);
}

TEST_CASE("ScriptBinding valid with name and handler", "[Phase16][Binding]") {
    ScriptBinding sb("greet", "Say hello");
    sb.setHandler([](const std::vector<std::string>&) { return true; });

    CHECK(sb.isValid());
    CHECK(sb.name() == "greet");
    CHECK(sb.description() == "Say hello");
}

TEST_CASE("ScriptBinding addParam and findParam", "[Phase16][Binding]") {
    ScriptBinding sb("cmd", "Test");
    sb.setHandler([](const std::vector<std::string>&) { return true; });

    ScriptParam p1; p1.name = "path"; p1.type = ScriptParamType::Path;
    ScriptParam p2; p2.name = "verbose"; p2.type = ScriptParamType::Bool; p2.required = false;

    CHECK(sb.addParam(p1));
    CHECK(sb.addParam(p2));
    CHECK(sb.paramCount() == 2);
    CHECK(sb.requiredParamCount() == 1);

    auto* found = sb.findParam("path");
    REQUIRE(found);
    CHECK(found->type == ScriptParamType::Path);

    CHECK(sb.findParam("missing") == nullptr);
}

TEST_CASE("ScriptBinding rejects duplicate param", "[Phase16][Binding]") {
    ScriptBinding sb("cmd", "Test");
    ScriptParam p; p.name = "x"; p.type = ScriptParamType::Int;
    CHECK(sb.addParam(p));
    CHECK_FALSE(sb.addParam(p)); // duplicate
    CHECK(sb.paramCount() == 1);
}

TEST_CASE("ScriptBinding rejects invalid param", "[Phase16][Binding]") {
    ScriptBinding sb("cmd", "Test");
    ScriptParam bad; // invalid
    CHECK_FALSE(sb.addParam(bad));
}

TEST_CASE("ScriptBinding invoke calls handler", "[Phase16][Binding]") {
    ScriptBinding sb("cmd", "Test");
    std::string captured;
    sb.setHandler([&](const std::vector<std::string>& args) {
        if (!args.empty()) captured = args[0];
        return true;
    });

    CHECK(sb.invoke({"hello"}));
    CHECK(captured == "hello");
}

TEST_CASE("ScriptBinding invoke returns false without handler", "[Phase16][Binding]") {
    ScriptBinding sb("cmd", "Test");
    CHECK_FALSE(sb.invoke({}));
}

TEST_CASE("ScriptBinding setReturnType", "[Phase16][Binding]") {
    ScriptBinding sb("cmd", "Test");
    sb.setReturnType(ScriptParamType::String);
    CHECK(sb.returnType() == ScriptParamType::String);
}

TEST_CASE("ScriptBinding params() returns all", "[Phase16][Binding]") {
    ScriptBinding sb("cmd", "Test");
    ScriptParam p1; p1.name = "a"; p1.type = ScriptParamType::Int;
    ScriptParam p2; p2.name = "b"; p2.type = ScriptParamType::Float;
    sb.addParam(p1);
    sb.addParam(p2);

    auto& ps = sb.params();
    REQUIRE(ps.size() == 2);
    CHECK(ps[0].name == "a");
    CHECK(ps[1].name == "b");
}

// ═════════════════════════════════════════════════════════════════
// SECTION 4 — ScriptContext
// ═════════════════════════════════════════════════════════════════

TEST_CASE("ScriptContext starts empty", "[Phase16][Context]") {
    ScriptContext ctx;
    CHECK(ctx.variableCount() == 0);
    CHECK(ctx.output().empty());
    CHECK_FALSE(ctx.hasError());
}

TEST_CASE("ScriptContext set/get/has variable", "[Phase16][Context]") {
    ScriptContext ctx;
    CHECK(ctx.setVariable("name", "atlas"));
    CHECK(ctx.hasVariable("name"));
    CHECK(ctx.getVariable("name") == "atlas");
    CHECK(ctx.variableCount() == 1);
}

TEST_CASE("ScriptContext getVariable returns empty for missing", "[Phase16][Context]") {
    ScriptContext ctx;
    CHECK(ctx.getVariable("missing").empty());
}

TEST_CASE("ScriptContext getVariableOr returns fallback", "[Phase16][Context]") {
    ScriptContext ctx;
    CHECK(ctx.getVariableOr("missing", "default") == "default");

    ctx.setVariable("key", "value");
    CHECK(ctx.getVariableOr("key", "default") == "value");
}

TEST_CASE("ScriptContext overwrite variable", "[Phase16][Context]") {
    ScriptContext ctx;
    ctx.setVariable("x", "1");
    ctx.setVariable("x", "2");
    CHECK(ctx.getVariable("x") == "2");
    CHECK(ctx.variableCount() == 1);
}

TEST_CASE("ScriptContext rejects empty key", "[Phase16][Context]") {
    ScriptContext ctx;
    CHECK_FALSE(ctx.setVariable("", "val"));
    CHECK(ctx.variableCount() == 0);
}

TEST_CASE("ScriptContext removeVariable", "[Phase16][Context]") {
    ScriptContext ctx;
    ctx.setVariable("x", "1");
    CHECK(ctx.removeVariable("x"));
    CHECK_FALSE(ctx.hasVariable("x"));
    CHECK(ctx.variableCount() == 0);
    CHECK_FALSE(ctx.removeVariable("x")); // already gone
}

TEST_CASE("ScriptContext clearVariables", "[Phase16][Context]") {
    ScriptContext ctx;
    ctx.setVariable("a", "1");
    ctx.setVariable("b", "2");
    ctx.clearVariables();
    CHECK(ctx.variableCount() == 0);
}

TEST_CASE("ScriptContext output capture", "[Phase16][Context]") {
    ScriptContext ctx;
    ctx.appendOutput("line1\n");
    ctx.appendOutput("line2\n");
    CHECK(ctx.output() == "line1\nline2\n");

    ctx.clearOutput();
    CHECK(ctx.output().empty());
}

TEST_CASE("ScriptContext error state", "[Phase16][Context]") {
    ScriptContext ctx;
    CHECK_FALSE(ctx.hasError());

    ctx.setError("something went wrong");
    CHECK(ctx.hasError());
    CHECK(ctx.error() == "something went wrong");

    ctx.clearError();
    CHECK_FALSE(ctx.hasError());
    CHECK(ctx.error().empty());
}

TEST_CASE("ScriptContext reset clears everything", "[Phase16][Context]") {
    ScriptContext ctx;
    ctx.setVariable("x", "1");
    ctx.appendOutput("text");
    ctx.setError("err");

    ctx.reset();
    CHECK(ctx.variableCount() == 0);
    CHECK(ctx.output().empty());
    CHECK_FALSE(ctx.hasError());
}

// ═════════════════════════════════════════════════════════════════
// SECTION 5 — ScriptExecResult
// ═════════════════════════════════════════════════════════════════

TEST_CASE("ScriptExecResult ok factory", "[Phase16][ExecResult]") {
    auto r = ScriptExecResult::ok("cmd");
    CHECK(r.succeeded());
    CHECK_FALSE(r.failed());
    CHECK(r.bindingId == "cmd");
}

TEST_CASE("ScriptExecResult fail factory", "[Phase16][ExecResult]") {
    auto r = ScriptExecResult::fail(ScriptExecStatus::NotFound, "cmd", "not found");
    CHECK(r.failed());
    CHECK_FALSE(r.succeeded());
    CHECK(r.bindingId == "cmd");
    CHECK(r.errorMessage == "not found");
}

// ═════════════════════════════════════════════════════════════════
// SECTION 6 — ScriptEngine
// ═════════════════════════════════════════════════════════════════

TEST_CASE("ScriptEngine starts empty", "[Phase16][Engine]") {
    ScriptEngine eng;
    CHECK(eng.bindingCount() == 0);
    CHECK(eng.totalExecutions() == 0);
    CHECK(eng.successfulExecutions() == 0);
}

TEST_CASE("ScriptEngine register and find binding", "[Phase16][Engine]") {
    ScriptEngine eng;
    ScriptBinding sb("greet", "Say hello");
    sb.setHandler([](const std::vector<std::string>&) { return true; });

    CHECK(eng.registerBinding(sb));
    CHECK(eng.isRegistered("greet"));
    CHECK(eng.bindingCount() == 1);

    auto* found = eng.findBinding("greet");
    REQUIRE(found);
    CHECK(found->description() == "Say hello");
}

TEST_CASE("ScriptEngine rejects duplicate binding", "[Phase16][Engine]") {
    ScriptEngine eng;
    ScriptBinding sb("cmd", "Test");
    sb.setHandler([](const std::vector<std::string>&) { return true; });

    CHECK(eng.registerBinding(sb));
    CHECK_FALSE(eng.registerBinding(sb));
    CHECK(eng.bindingCount() == 1);
}

TEST_CASE("ScriptEngine rejects invalid binding", "[Phase16][Engine]") {
    ScriptEngine eng;
    ScriptBinding sb; // no name, no handler
    CHECK_FALSE(eng.registerBinding(sb));
}

TEST_CASE("ScriptEngine unregister binding", "[Phase16][Engine]") {
    ScriptEngine eng;
    ScriptBinding sb("cmd", "Test");
    sb.setHandler([](const std::vector<std::string>&) { return true; });
    eng.registerBinding(sb);

    CHECK(eng.unregisterBinding("cmd"));
    CHECK_FALSE(eng.isRegistered("cmd"));
    CHECK(eng.bindingCount() == 0);
    CHECK_FALSE(eng.unregisterBinding("cmd")); // already gone
}

TEST_CASE("ScriptEngine execute success", "[Phase16][Engine]") {
    ScriptEngine eng;
    ScriptContext ctx;
    std::string captured;

    ScriptBinding sb("echo", "Echo text");
    sb.setHandler([&](const std::vector<std::string>& args) {
        if (!args.empty()) { captured = args[0]; ctx.appendOutput(args[0]); }
        return true;
    });
    eng.registerBinding(sb);

    auto r = eng.execute("echo", {"hello"}, ctx);
    CHECK(r.succeeded());
    CHECK(captured == "hello");
    CHECK(ctx.output() == "hello");
    CHECK(eng.totalExecutions() == 1);
    CHECK(eng.successfulExecutions() == 1);
}

TEST_CASE("ScriptEngine execute NotFound", "[Phase16][Engine]") {
    ScriptEngine eng;
    ScriptContext ctx;

    auto r = eng.execute("missing", ctx);
    CHECK(r.failed());
    CHECK(r.status == ScriptExecStatus::NotFound);
}

TEST_CASE("ScriptEngine execute HandlerFailed", "[Phase16][Engine]") {
    ScriptEngine eng;
    ScriptContext ctx;

    ScriptBinding sb("fail_cmd", "Always fails");
    sb.setHandler([](const std::vector<std::string>&) { return false; });
    eng.registerBinding(sb);

    auto r = eng.execute("fail_cmd", ctx);
    CHECK(r.failed());
    CHECK(r.status == ScriptExecStatus::HandlerFailed);
    CHECK(ctx.hasError());
    CHECK(eng.totalExecutions() == 1);
    CHECK(eng.successfulExecutions() == 0);
}

TEST_CASE("ScriptEngine execute InvalidArgs too few", "[Phase16][Engine]") {
    ScriptEngine eng;
    ScriptContext ctx;

    ScriptBinding sb("cmd", "Needs args");
    sb.setHandler([](const std::vector<std::string>&) { return true; });
    ScriptParam p; p.name = "path"; p.type = ScriptParamType::Path; p.required = true;
    sb.addParam(p);
    eng.registerBinding(sb);

    auto r = eng.execute("cmd", {}, ctx);  // 0 args, need 1
    CHECK(r.failed());
    CHECK(r.status == ScriptExecStatus::InvalidArgs);
}

TEST_CASE("ScriptEngine execute with sufficient args", "[Phase16][Engine]") {
    ScriptEngine eng;
    ScriptContext ctx;

    ScriptBinding sb("cmd", "Needs args");
    sb.setHandler([](const std::vector<std::string>&) { return true; });
    ScriptParam p; p.name = "path"; p.type = ScriptParamType::Path; p.required = true;
    sb.addParam(p);
    eng.registerBinding(sb);

    auto r = eng.execute("cmd", {"/tmp/test"}, ctx);
    CHECK(r.succeeded());
}

TEST_CASE("ScriptEngine allBindings returns all", "[Phase16][Engine]") {
    ScriptEngine eng;
    ScriptBinding sb1("a", "A"); sb1.setHandler([](const std::vector<std::string>&) { return true; });
    ScriptBinding sb2("b", "B"); sb2.setHandler([](const std::vector<std::string>&) { return true; });
    eng.registerBinding(sb1);
    eng.registerBinding(sb2);

    auto& all = eng.allBindings();
    REQUIRE(all.size() == 2);
    CHECK(all[0].name() == "a");
    CHECK(all[1].name() == "b");
}

TEST_CASE("ScriptEngine clear resets everything", "[Phase16][Engine]") {
    ScriptEngine eng;
    ScriptContext ctx;

    ScriptBinding sb("cmd", "T");
    sb.setHandler([](const std::vector<std::string>&) { return true; });
    eng.registerBinding(sb);
    eng.execute("cmd", ctx);

    eng.clear();
    CHECK(eng.bindingCount() == 0);
    CHECK(eng.totalExecutions() == 0);
    CHECK(eng.successfulExecutions() == 0);
}

// ═════════════════════════════════════════════════════════════════
// SECTION 7 — AutomationStep
// ═════════════════════════════════════════════════════════════════

TEST_CASE("AutomationStep default is invalid", "[Phase16][Step]") {
    AutomationStep s;
    CHECK_FALSE(s.isValid());
    CHECK(s.status == AutomationStepStatus::Pending);
}

TEST_CASE("AutomationStep valid construction", "[Phase16][Step]") {
    AutomationStep s;
    s.name = "build";
    s.description = "Build project";
    s.handler = [](ScriptContext&) { return true; };

    CHECK(s.isValid());
    CHECK(s.name == "build");
}

TEST_CASE("AutomationStep reset", "[Phase16][Step]") {
    AutomationStep s;
    s.name = "build";
    s.handler = [](ScriptContext&) { return true; };
    s.status = AutomationStepStatus::Failed;
    s.errorMessage = "oops";

    s.reset();
    CHECK(s.status == AutomationStepStatus::Pending);
    CHECK(s.errorMessage.empty());
}

// ═════════════════════════════════════════════════════════════════
// SECTION 8 — AutomationTask
// ═════════════════════════════════════════════════════════════════

TEST_CASE("AutomationTask default state", "[Phase16][Task]") {
    AutomationTask task("build_all", "Build everything");
    CHECK(task.name() == "build_all");
    CHECK(task.state() == AutomationTaskState::Idle);
    CHECK(task.stepCount() == 0);
    CHECK_FALSE(task.isValid()); // no steps
}

TEST_CASE("AutomationTask addStep and findStep", "[Phase16][Task]") {
    AutomationTask task("t");
    AutomationStep s;
    s.name = "step1"; s.handler = [](ScriptContext&) { return true; };

    CHECK(task.addStep(s));
    CHECK(task.stepCount() == 1);
    CHECK(task.isValid());

    auto* found = task.findStep("step1");
    REQUIRE(found);
    CHECK(found->name == "step1");
    CHECK(task.findStep("nope") == nullptr);
}

TEST_CASE("AutomationTask rejects duplicate step", "[Phase16][Task]") {
    AutomationTask task("t");
    AutomationStep s;
    s.name = "step1"; s.handler = [](ScriptContext&) { return true; };

    CHECK(task.addStep(s));
    CHECK_FALSE(task.addStep(s));
    CHECK(task.stepCount() == 1);
}

TEST_CASE("AutomationTask rejects invalid step", "[Phase16][Task]") {
    AutomationTask task("t");
    AutomationStep s; // invalid: no name or handler
    CHECK_FALSE(task.addStep(s));
}

TEST_CASE("AutomationTask removeStep", "[Phase16][Task]") {
    AutomationTask task("t");
    AutomationStep s;
    s.name = "step1"; s.handler = [](ScriptContext&) { return true; };
    task.addStep(s);

    CHECK(task.removeStep("step1"));
    CHECK(task.stepCount() == 0);
    CHECK_FALSE(task.removeStep("step1")); // already gone
}

TEST_CASE("AutomationTask enableStep", "[Phase16][Task]") {
    AutomationTask task("t");
    AutomationStep s;
    s.name = "step1"; s.handler = [](ScriptContext&) { return true; };
    task.addStep(s);

    CHECK(task.enableStep("step1", false));
    auto* found = task.findStep("step1");
    REQUIRE(found);
    CHECK_FALSE(found->enabled);

    CHECK_FALSE(task.enableStep("nonexistent", true));
}

TEST_CASE("AutomationTask run all succeed", "[Phase16][Task]") {
    AutomationTask task("t");
    ScriptContext ctx;

    AutomationStep s1;
    s1.name = "s1"; s1.handler = [](ScriptContext& c) { c.appendOutput("1"); return true; };
    AutomationStep s2;
    s2.name = "s2"; s2.handler = [](ScriptContext& c) { c.appendOutput("2"); return true; };
    task.addStep(s1);
    task.addStep(s2);

    auto state = task.run(ctx);
    CHECK(state == AutomationTaskState::Completed);
    CHECK(task.stepsRun() == 2);
    CHECK(task.stepsSucceeded() == 2);
    CHECK(task.stepsFailed() == 0);
    CHECK(ctx.output() == "12");
}

TEST_CASE("AutomationTask run with failure aborts", "[Phase16][Task]") {
    AutomationTask task("t");
    task.setAbortOnFailure(true);
    ScriptContext ctx;

    AutomationStep s1;
    s1.name = "s1"; s1.handler = [](ScriptContext&) { return true; };
    AutomationStep s2;
    s2.name = "s2"; s2.handler = [](ScriptContext&) { return false; };
    AutomationStep s3;
    s3.name = "s3"; s3.handler = [](ScriptContext&) { return true; };

    task.addStep(s1);
    task.addStep(s2);
    task.addStep(s3);

    auto state = task.run(ctx);
    CHECK(state == AutomationTaskState::Failed);
    CHECK(task.stepsRun() == 2); // s1 + s2
    CHECK(task.stepsSucceeded() == 1);
    CHECK(task.stepsFailed() == 1);
    CHECK(task.stepsSkipped() == 1); // s3 skipped
}

TEST_CASE("AutomationTask run without abort continues on failure", "[Phase16][Task]") {
    AutomationTask task("t");
    task.setAbortOnFailure(false);
    ScriptContext ctx;

    AutomationStep s1;
    s1.name = "s1"; s1.handler = [](ScriptContext&) { return false; };
    AutomationStep s2;
    s2.name = "s2"; s2.handler = [](ScriptContext&) { return true; };

    task.addStep(s1);
    task.addStep(s2);

    auto state = task.run(ctx);
    CHECK(state == AutomationTaskState::Failed); // has failures
    CHECK(task.stepsRun() == 2);
    CHECK(task.stepsSucceeded() == 1);
    CHECK(task.stepsFailed() == 1);
}

TEST_CASE("AutomationTask run skips disabled steps", "[Phase16][Task]") {
    AutomationTask task("t");
    ScriptContext ctx;

    AutomationStep s1;
    s1.name = "s1"; s1.handler = [](ScriptContext&) { return true; };
    AutomationStep s2;
    s2.name = "s2"; s2.handler = [](ScriptContext&) { return true; }; s2.enabled = false;
    AutomationStep s3;
    s3.name = "s3"; s3.handler = [](ScriptContext&) { return true; };

    task.addStep(s1);
    task.addStep(s2);
    task.addStep(s3);

    auto state = task.run(ctx);
    CHECK(state == AutomationTaskState::Completed);
    CHECK(task.stepsRun() == 2);
    CHECK(task.stepsSkipped() == 1);
}

TEST_CASE("AutomationTask reset clears state", "[Phase16][Task]") {
    AutomationTask task("t");
    ScriptContext ctx;

    AutomationStep s;
    s.name = "s1"; s.handler = [](ScriptContext&) { return true; };
    task.addStep(s);
    task.run(ctx);

    CHECK(task.state() == AutomationTaskState::Completed);
    task.reset();
    CHECK(task.state() == AutomationTaskState::Idle);
    CHECK(task.stepsRun() == 0);
    CHECK(task.stepsSucceeded() == 0);
}

TEST_CASE("AutomationTask steps() returns all", "[Phase16][Task]") {
    AutomationTask task("t");
    AutomationStep s1; s1.name = "a"; s1.handler = [](ScriptContext&) { return true; };
    AutomationStep s2; s2.name = "b"; s2.handler = [](ScriptContext&) { return true; };
    task.addStep(s1);
    task.addStep(s2);

    auto& steps = task.steps();
    REQUIRE(steps.size() == 2);
    CHECK(steps[0].name == "a");
    CHECK(steps[1].name == "b");
}

TEST_CASE("AutomationTask run empty returns Idle", "[Phase16][Task]") {
    AutomationTask task("t");
    ScriptContext ctx;
    auto state = task.run(ctx);
    CHECK(state == AutomationTaskState::Idle);
}

// ═════════════════════════════════════════════════════════════════
// SECTION 9 — Integration
// ═════════════════════════════════════════════════════════════════

TEST_CASE("Integration: engine executes binding and context captures output", "[Phase16][Integration]") {
    ScriptEngine eng;
    ScriptContext ctx;

    ScriptBinding sb("list_files", "List files in directory");
    ScriptParam p; p.name = "dir"; p.type = ScriptParamType::Path; p.required = true;
    sb.addParam(p);
    sb.setHandler([&](const std::vector<std::string>& args) {
        ctx.appendOutput("Files in " + args[0] + ": a.txt, b.txt\n");
        ctx.setVariable("last_dir", args[0]);
        return true;
    });
    eng.registerBinding(sb);

    auto r = eng.execute("list_files", {"/workspace/assets"}, ctx);
    CHECK(r.succeeded());
    CHECK(ctx.output() == "Files in /workspace/assets: a.txt, b.txt\n");
    CHECK(ctx.getVariable("last_dir") == "/workspace/assets");
}

TEST_CASE("Integration: automation task uses engine bindings through context", "[Phase16][Integration]") {
    ScriptEngine eng;
    ScriptContext ctx;

    // Register bindings
    ScriptBinding build("build", "Build project");
    build.setHandler([](const std::vector<std::string>&) { return true; });
    eng.registerBinding(build);

    ScriptBinding test("test", "Run tests");
    test.setHandler([](const std::vector<std::string>&) { return true; });
    eng.registerBinding(test);

    // Create automation task that uses engine
    AutomationTask task("ci_pipeline", "Build and test");

    AutomationStep buildStep;
    buildStep.name = "build";
    buildStep.handler = [&](ScriptContext& c) {
        auto r = eng.execute("build", c);
        if (r.succeeded()) c.appendOutput("Build OK\n");
        return r.succeeded();
    };

    AutomationStep testStep;
    testStep.name = "test";
    testStep.handler = [&](ScriptContext& c) {
        auto r = eng.execute("test", c);
        if (r.succeeded()) c.appendOutput("Tests OK\n");
        return r.succeeded();
    };

    task.addStep(buildStep);
    task.addStep(testStep);

    auto state = task.run(ctx);
    CHECK(state == AutomationTaskState::Completed);
    CHECK(ctx.output() == "Build OK\nTests OK\n");
    CHECK(eng.totalExecutions() == 2);
    CHECK(eng.successfulExecutions() == 2);
}

TEST_CASE("Integration: automation task aborts on engine failure", "[Phase16][Integration]") {
    ScriptEngine eng;
    ScriptContext ctx;

    ScriptBinding failing("deploy", "Deploy");
    failing.setHandler([](const std::vector<std::string>&) { return false; });
    eng.registerBinding(failing);

    ScriptBinding notify("notify", "Notify");
    notify.setHandler([](const std::vector<std::string>&) { return true; });
    eng.registerBinding(notify);

    AutomationTask task("deploy_pipeline");
    task.setAbortOnFailure(true);

    AutomationStep deployStep;
    deployStep.name = "deploy";
    deployStep.handler = [&](ScriptContext& c) {
        return eng.execute("deploy", c).succeeded();
    };

    AutomationStep notifyStep;
    notifyStep.name = "notify";
    notifyStep.handler = [&](ScriptContext& c) {
        return eng.execute("notify", c).succeeded();
    };

    task.addStep(deployStep);
    task.addStep(notifyStep);

    auto state = task.run(ctx);
    CHECK(state == AutomationTaskState::Failed);
    CHECK(task.stepsRun() == 1); // only deploy ran
    CHECK(task.stepsSkipped() == 1); // notify skipped
}

TEST_CASE("Integration: context variables persist across task steps", "[Phase16][Integration]") {
    AutomationTask task("pipeline");
    ScriptContext ctx;

    AutomationStep step1;
    step1.name = "setup";
    step1.handler = [](ScriptContext& c) {
        c.setVariable("build_dir", "/tmp/build");
        c.setVariable("config", "Release");
        return true;
    };

    AutomationStep step2;
    step2.name = "build";
    step2.handler = [](ScriptContext& c) {
        auto dir = c.getVariable("build_dir");
        auto cfg = c.getVariable("config");
        c.appendOutput("Building in " + dir + " [" + cfg + "]\n");
        return !dir.empty() && !cfg.empty();
    };

    task.addStep(step1);
    task.addStep(step2);

    auto state = task.run(ctx);
    CHECK(state == AutomationTaskState::Completed);
    CHECK(ctx.getVariable("build_dir") == "/tmp/build");
    CHECK(ctx.output() == "Building in /tmp/build [Release]\n");
}
