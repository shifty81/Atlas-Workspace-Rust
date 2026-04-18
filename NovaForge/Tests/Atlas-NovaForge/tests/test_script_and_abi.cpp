#include <cassert>
#include <cmath>
#include <iostream>
#include <string>
#include <vector>
#include "../engine/script/ScriptVM.h"
#include "../engine/script/ScriptSystem.h"
#include "../engine/script/ScriptSandbox.h"
#include "../engine/abi/ABICapsule.h"
#include "../engine/abi/ABIRegistry.h"

using namespace atlas::script;
using namespace atlas::abi;

// ============================================================
// Helper: build a CompiledScript from instructions + constants
// ============================================================

static CompiledScript make_script(
    const std::string& name,
    std::vector<Instruction> code,
    std::vector<ScriptValue> constants = {},
    bool deterministic = true)
{
    CompiledScript s;
    s.name = name;
    s.version = 1;
    s.code = std::move(code);
    s.constants = std::move(constants);
    s.sourceHash = 0;
    s.deterministicDeclared = deterministic;
    s.replaySafe = deterministic;
    s.migrationSafe = deterministic;
    return s;
}

// ============================================================
// ScriptVM tests
// ============================================================

void test_script_value_types() {
    ScriptValue none;
    assert(none.type == ScriptValue::Type::None);

    ScriptValue iv(int64_t(42));
    assert(iv.type == ScriptValue::Type::Int);
    assert(iv.intVal == 42);

    ScriptValue fv(3.14);
    assert(fv.type == ScriptValue::Type::Float);
    assert(fv.floatVal == 3.14);

    ScriptValue bv(true);
    assert(bv.type == ScriptValue::Type::Bool);
    assert(bv.intVal == 1);

    ScriptValue sv(std::string("hello"));
    assert(sv.type == ScriptValue::Type::String);
    assert(sv.strVal == "hello");

    std::cout << "[PASS] test_script_value_types" << std::endl;
}

void test_script_value_equality() {
    ScriptValue a(int64_t(10));
    ScriptValue b(int64_t(10));
    ScriptValue c(int64_t(20));
    ScriptValue d(3.14);
    ScriptValue e(std::string("hi"));
    ScriptValue f(std::string("hi"));
    ScriptValue g(std::string("bye"));

    assert(a == b);
    assert(a != c);
    assert(a != d);
    assert(e == f);
    assert(e != g);
    assert(ScriptValue() == ScriptValue());

    std::cout << "[PASS] test_script_value_equality" << std::endl;
}

void test_script_value_truthy() {
    assert(!ScriptValue().IsTruthy());
    assert(!ScriptValue(int64_t(0)).IsTruthy());
    assert(ScriptValue(int64_t(1)).IsTruthy());
    assert(ScriptValue(int64_t(-1)).IsTruthy());
    assert(!ScriptValue(0.0).IsTruthy());
    assert(ScriptValue(1.5).IsTruthy());
    assert(!ScriptValue(false).IsTruthy());
    assert(ScriptValue(true).IsTruthy());
    assert(!ScriptValue(std::string("")).IsTruthy());
    assert(ScriptValue(std::string("x")).IsTruthy());

    std::cout << "[PASS] test_script_value_truthy" << std::endl;
}

void test_vm_push_int() {
    ScriptVM vm;
    // constants[0] = Int(99), code: PUSH_INT 0, HALT
    auto script = make_script("push_int",
        {{Opcode::PUSH_INT, 0}, {Opcode::HALT, 0}},
        {ScriptValue(int64_t(99))});
    ScriptValue result = vm.Execute(script);
    assert(result.type == ScriptValue::Type::Int);
    assert(result.intVal == 99);

    std::cout << "[PASS] test_vm_push_int" << std::endl;
}

void test_vm_arithmetic() {
    ScriptVM vm;
    // ADD: 10 + 20 = 30
    {
        auto s = make_script("add",
            {{Opcode::PUSH_INT, 0}, {Opcode::PUSH_INT, 1}, {Opcode::ADD, 0}, {Opcode::HALT, 0}},
            {ScriptValue(int64_t(10)), ScriptValue(int64_t(20))});
        auto r = vm.Execute(s);
        assert(r.type == ScriptValue::Type::Int && r.intVal == 30);
    }
    // SUB: 20 - 7 = 13
    {
        auto s = make_script("sub",
            {{Opcode::PUSH_INT, 0}, {Opcode::PUSH_INT, 1}, {Opcode::SUB, 0}, {Opcode::HALT, 0}},
            {ScriptValue(int64_t(20)), ScriptValue(int64_t(7))});
        auto r = vm.Execute(s);
        assert(r.type == ScriptValue::Type::Int && r.intVal == 13);
    }
    // MUL: 6 * 7 = 42
    {
        auto s = make_script("mul",
            {{Opcode::PUSH_INT, 0}, {Opcode::PUSH_INT, 1}, {Opcode::MUL, 0}, {Opcode::HALT, 0}},
            {ScriptValue(int64_t(6)), ScriptValue(int64_t(7))});
        auto r = vm.Execute(s);
        assert(r.type == ScriptValue::Type::Int && r.intVal == 42);
    }
    // DIV: 20 / 4 = 5
    {
        auto s = make_script("div",
            {{Opcode::PUSH_INT, 0}, {Opcode::PUSH_INT, 1}, {Opcode::DIV, 0}, {Opcode::HALT, 0}},
            {ScriptValue(int64_t(20)), ScriptValue(int64_t(4))});
        auto r = vm.Execute(s);
        assert(r.type == ScriptValue::Type::Int && r.intVal == 5);
    }
    // MOD: 17 % 5 = 2
    {
        auto s = make_script("mod",
            {{Opcode::PUSH_INT, 0}, {Opcode::PUSH_INT, 1}, {Opcode::MOD, 0}, {Opcode::HALT, 0}},
            {ScriptValue(int64_t(17)), ScriptValue(int64_t(5))});
        auto r = vm.Execute(s);
        assert(r.type == ScriptValue::Type::Int && r.intVal == 2);
    }

    std::cout << "[PASS] test_vm_arithmetic" << std::endl;
}

void test_vm_division_by_zero() {
    ScriptVM vm;
    // DIV by 0 -> Int(0)
    {
        auto s = make_script("div0",
            {{Opcode::PUSH_INT, 0}, {Opcode::PUSH_INT, 1}, {Opcode::DIV, 0}, {Opcode::HALT, 0}},
            {ScriptValue(int64_t(10)), ScriptValue(int64_t(0))});
        auto r = vm.Execute(s);
        assert(r.type == ScriptValue::Type::Int && r.intVal == 0);
    }
    // MOD by 0 -> Int(0)
    {
        auto s = make_script("mod0",
            {{Opcode::PUSH_INT, 0}, {Opcode::PUSH_INT, 1}, {Opcode::MOD, 0}, {Opcode::HALT, 0}},
            {ScriptValue(int64_t(10)), ScriptValue(int64_t(0))});
        auto r = vm.Execute(s);
        assert(r.type == ScriptValue::Type::Int && r.intVal == 0);
    }

    std::cout << "[PASS] test_vm_division_by_zero" << std::endl;
}

void test_vm_float_arithmetic() {
    ScriptVM vm;
    auto run = [&](Opcode op, double a, double b) -> double {
        auto s = make_script("fop",
            {{Opcode::PUSH_FLOAT, 0}, {Opcode::PUSH_FLOAT, 1}, {op, 0}, {Opcode::HALT, 0}},
            {ScriptValue(a), ScriptValue(b)});
        return vm.Execute(s).floatVal;
    };
    assert(std::abs(run(Opcode::ADD, 1.5, 2.5) - 4.0) < 1e-9);
    assert(std::abs(run(Opcode::SUB, 5.0, 1.5) - 3.5) < 1e-9);
    assert(std::abs(run(Opcode::MUL, 3.0, 2.5) - 7.5) < 1e-9);
    assert(std::abs(run(Opcode::DIV, 7.5, 2.5) - 3.0) < 1e-9);

    std::cout << "[PASS] test_vm_float_arithmetic" << std::endl;
}

void test_vm_comparison() {
    ScriptVM vm;
    auto cmp = [&](Opcode op, int64_t a, int64_t b) -> bool {
        auto s = make_script("cmp",
            {{Opcode::PUSH_INT, 0}, {Opcode::PUSH_INT, 1}, {op, 0}, {Opcode::HALT, 0}},
            {ScriptValue(a), ScriptValue(b)});
        return vm.Execute(s).IsTruthy();
    };
    assert(cmp(Opcode::EQ, 5, 5));
    assert(!cmp(Opcode::EQ, 5, 6));
    assert(cmp(Opcode::NEQ, 5, 6));
    assert(!cmp(Opcode::NEQ, 5, 5));
    assert(cmp(Opcode::LT, 3, 5));
    assert(!cmp(Opcode::LT, 5, 3));
    assert(cmp(Opcode::GT, 5, 3));
    assert(!cmp(Opcode::GT, 3, 5));
    assert(cmp(Opcode::LTE, 3, 5));
    assert(cmp(Opcode::LTE, 5, 5));
    assert(!cmp(Opcode::LTE, 6, 5));
    assert(cmp(Opcode::GTE, 5, 3));
    assert(cmp(Opcode::GTE, 5, 5));
    assert(!cmp(Opcode::GTE, 3, 5));

    std::cout << "[PASS] test_vm_comparison" << std::endl;
}

void test_vm_logic() {
    ScriptVM vm;
    // AND: true && false = false
    {
        auto s = make_script("and",
            {{Opcode::PUSH_INT, 0}, {Opcode::PUSH_INT, 1}, {Opcode::AND, 0}, {Opcode::HALT, 0}},
            {ScriptValue(int64_t(1)), ScriptValue(int64_t(0))});
        assert(!vm.Execute(s).IsTruthy());
    }
    // AND: true && true = true
    {
        auto s = make_script("and2",
            {{Opcode::PUSH_INT, 0}, {Opcode::PUSH_INT, 0}, {Opcode::AND, 0}, {Opcode::HALT, 0}},
            {ScriptValue(int64_t(1))});
        assert(vm.Execute(s).IsTruthy());
    }
    // OR: false || true = true
    {
        auto s = make_script("or",
            {{Opcode::PUSH_INT, 0}, {Opcode::PUSH_INT, 1}, {Opcode::OR, 0}, {Opcode::HALT, 0}},
            {ScriptValue(int64_t(0)), ScriptValue(int64_t(1))});
        assert(vm.Execute(s).IsTruthy());
    }
    // OR: false || false = false
    {
        auto s = make_script("or2",
            {{Opcode::PUSH_INT, 0}, {Opcode::PUSH_INT, 0}, {Opcode::OR, 0}, {Opcode::HALT, 0}},
            {ScriptValue(int64_t(0))});
        assert(!vm.Execute(s).IsTruthy());
    }
    // NOT: !true = false
    {
        auto s = make_script("not",
            {{Opcode::PUSH_INT, 0}, {Opcode::NOT, 0}, {Opcode::HALT, 0}},
            {ScriptValue(int64_t(1))});
        assert(!vm.Execute(s).IsTruthy());
    }
    // NOT: !false = true
    {
        auto s = make_script("not2",
            {{Opcode::PUSH_INT, 0}, {Opcode::NOT, 0}, {Opcode::HALT, 0}},
            {ScriptValue(int64_t(0))});
        assert(vm.Execute(s).IsTruthy());
    }

    std::cout << "[PASS] test_vm_logic" << std::endl;
}

void test_vm_variables() {
    ScriptVM vm;
    // PUSH_INT 42, STORE_VAR "x", LOAD_VAR "x", HALT
    auto s = make_script("vars",
        {
            {Opcode::PUSH_INT, 0},
            {Opcode::STORE_VAR, 1},
            {Opcode::LOAD_VAR, 1},
            {Opcode::HALT, 0}
        },
        {ScriptValue(int64_t(42)), ScriptValue(std::string("x"))});
    auto r = vm.Execute(s);
    assert(r.type == ScriptValue::Type::Int && r.intVal == 42);

    std::cout << "[PASS] test_vm_variables" << std::endl;
}

void test_vm_set_get_variable() {
    ScriptVM vm;
    vm.SetVariable("health", ScriptValue(int64_t(100)));
    auto v = vm.GetVariable("health");
    assert(v.type == ScriptValue::Type::Int && v.intVal == 100);

    vm.SetVariable("name", ScriptValue(std::string("atlas")));
    auto v2 = vm.GetVariable("name");
    assert(v2.type == ScriptValue::Type::String && v2.strVal == "atlas");

    // Non-existent variable returns None
    auto v3 = vm.GetVariable("missing");
    assert(v3.type == ScriptValue::Type::None);

    std::cout << "[PASS] test_vm_set_get_variable" << std::endl;
}

void test_vm_jump() {
    ScriptVM vm;
    // JMP over PUSH_INT(99) to PUSH_INT(42), HALT
    // pc0: JMP -> 2
    // pc1: PUSH_INT 0 (99) — skipped
    // pc2: PUSH_INT 1 (42)
    // pc3: HALT
    auto s = make_script("jmp",
        {
            {Opcode::JMP, 2},
            {Opcode::PUSH_INT, 0},
            {Opcode::PUSH_INT, 1},
            {Opcode::HALT, 0}
        },
        {ScriptValue(int64_t(99)), ScriptValue(int64_t(42))});
    auto r = vm.Execute(s);
    assert(r.type == ScriptValue::Type::Int && r.intVal == 42);

    std::cout << "[PASS] test_vm_jump" << std::endl;
}

void test_vm_jump_if() {
    ScriptVM vm;
    // Truthy: push 1, JMP_IF -> 3, push 99, halt, push 42, halt
    {
        auto s = make_script("jmpif_t",
            {
                {Opcode::PUSH_INT, 0},   // pc0: push 1 (truthy)
                {Opcode::JMP_IF, 4},      // pc1: jump to pc4 if truthy
                {Opcode::PUSH_INT, 1},    // pc2: push 99 (not taken)
                {Opcode::HALT, 0},        // pc3
                {Opcode::PUSH_INT, 2},    // pc4: push 42
                {Opcode::HALT, 0}         // pc5
            },
            {ScriptValue(int64_t(1)), ScriptValue(int64_t(99)), ScriptValue(int64_t(42))});
        auto r = vm.Execute(s);
        assert(r.type == ScriptValue::Type::Int && r.intVal == 42);
    }
    // Falsy: push 0, JMP_IF -> 4, push 99, halt, push 42, halt
    {
        auto s = make_script("jmpif_f",
            {
                {Opcode::PUSH_INT, 0},   // pc0: push 0 (falsy)
                {Opcode::JMP_IF, 4},      // pc1: not taken
                {Opcode::PUSH_INT, 1},    // pc2: push 99
                {Opcode::HALT, 0},        // pc3
                {Opcode::PUSH_INT, 2},    // pc4: push 42 (not reached)
                {Opcode::HALT, 0}         // pc5
            },
            {ScriptValue(int64_t(0)), ScriptValue(int64_t(99)), ScriptValue(int64_t(42))});
        auto r = vm.Execute(s);
        assert(r.type == ScriptValue::Type::Int && r.intVal == 99);
    }

    std::cout << "[PASS] test_vm_jump_if" << std::endl;
}

void test_vm_call_native() {
    ScriptVM vm;
    vm.RegisterFunction("double_it", [](const std::vector<ScriptValue>& args) -> ScriptValue {
        if (args.empty()) return ScriptValue(int64_t(0));
        return ScriptValue(args[0].intVal * 2);
    });
    // constants: [0]=Int(21), [1]=String("double_it")
    // CALL operand indexes the function name in constants.
    // The instruction after CALL is a pseudo-instruction whose operand = arg count.
    auto s = make_script("call",
        {
            {Opcode::PUSH_INT, 0},
            {Opcode::CALL, 1},
            {Opcode::HALT, 1},   // pseudo-instruction: arg count = 1 (opcode ignored, operand used)
            {Opcode::HALT, 0}
        },
        {ScriptValue(int64_t(21)), ScriptValue(std::string("double_it"))});
    auto r = vm.Execute(s);
    assert(r.type == ScriptValue::Type::Int && r.intVal == 42);

    std::cout << "[PASS] test_vm_call_native" << std::endl;
}

void test_vm_budget() {
    ScriptVM vm;
    vm.SetMaxSteps(5);
    // Long script: push, push, add, push, push, add, push, push, add, halt
    auto s = make_script("budget",
        {
            {Opcode::PUSH_INT, 0}, {Opcode::PUSH_INT, 0}, {Opcode::ADD, 0},
            {Opcode::PUSH_INT, 0}, {Opcode::PUSH_INT, 0}, {Opcode::ADD, 0},
            {Opcode::PUSH_INT, 0}, {Opcode::PUSH_INT, 0}, {Opcode::ADD, 0},
            {Opcode::HALT, 0}
        },
        {ScriptValue(int64_t(1))});
    vm.Execute(s);
    assert(vm.WasBudgetExceeded());

    // Without budget exceeded: short script
    vm.Reset();
    vm.SetMaxSteps(100);
    auto s2 = make_script("short",
        {{Opcode::PUSH_INT, 0}, {Opcode::HALT, 0}},
        {ScriptValue(int64_t(1))});
    vm.Execute(s2);
    assert(!vm.WasBudgetExceeded());

    std::cout << "[PASS] test_vm_budget" << std::endl;
}

void test_vm_state_hash() {
    // Same execution -> same hash
    ScriptVM vm1;
    ScriptVM vm2;
    auto s = make_script("hash",
        {
            {Opcode::PUSH_INT, 0},
            {Opcode::STORE_VAR, 1},
            {Opcode::HALT, 0}
        },
        {ScriptValue(int64_t(42)), ScriptValue(std::string("x"))});
    vm1.Execute(s);
    vm2.Execute(s);
    assert(vm1.StateHash() == vm2.StateHash());

    // Different state -> different hash
    ScriptVM vm3;
    auto s2 = make_script("hash2",
        {
            {Opcode::PUSH_INT, 0},
            {Opcode::STORE_VAR, 1},
            {Opcode::HALT, 0}
        },
        {ScriptValue(int64_t(99)), ScriptValue(std::string("x"))});
    vm3.Execute(s2);
    assert(vm1.StateHash() != vm3.StateHash());

    std::cout << "[PASS] test_vm_state_hash" << std::endl;
}

void test_vm_reset() {
    ScriptVM vm;
    vm.SetVariable("x", ScriptValue(int64_t(10)));
    auto s = make_script("r",
        {{Opcode::PUSH_INT, 0}, {Opcode::HALT, 0}},
        {ScriptValue(int64_t(1))});
    vm.Execute(s);
    assert(vm.StepCount() > 0);

    vm.Reset();
    assert(vm.GetVariable("x").type == ScriptValue::Type::None);
    assert(vm.StepCount() == 0);

    std::cout << "[PASS] test_vm_reset" << std::endl;
}

void test_vm_neg() {
    ScriptVM vm;
    // NEG int
    {
        auto s = make_script("neg_i",
            {{Opcode::PUSH_INT, 0}, {Opcode::NEG, 0}, {Opcode::HALT, 0}},
            {ScriptValue(int64_t(7))});
        auto r = vm.Execute(s);
        assert(r.type == ScriptValue::Type::Int && r.intVal == -7);
    }
    // NEG float
    {
        auto s = make_script("neg_f",
            {{Opcode::PUSH_FLOAT, 0}, {Opcode::NEG, 0}, {Opcode::HALT, 0}},
            {ScriptValue(2.5)});
        auto r = vm.Execute(s);
        assert(r.type == ScriptValue::Type::Float && std::abs(r.floatVal - (-2.5)) < 1e-9);
    }

    std::cout << "[PASS] test_vm_neg" << std::endl;
}

void test_vm_string_push() {
    ScriptVM vm;
    auto s = make_script("str",
        {{Opcode::PUSH_STRING, 0}, {Opcode::HALT, 0}},
        {ScriptValue(std::string("hello world"))});
    auto r = vm.Execute(s);
    assert(r.type == ScriptValue::Type::String && r.strVal == "hello world");

    std::cout << "[PASS] test_vm_string_push" << std::endl;
}

void test_vm_ret() {
    ScriptVM vm;
    // RET should stop execution and return top of stack
    auto s = make_script("ret",
        {
            {Opcode::PUSH_INT, 0},
            {Opcode::RET, 0},
            {Opcode::PUSH_INT, 1},
            {Opcode::HALT, 0}
        },
        {ScriptValue(int64_t(77)), ScriptValue(int64_t(99))});
    auto r = vm.Execute(s);
    assert(r.type == ScriptValue::Type::Int && r.intVal == 77);

    std::cout << "[PASS] test_vm_ret" << std::endl;
}

// ============================================================
// ScriptSystem tests
// ============================================================

void test_script_system_register() {
    ScriptSystem sys;
    auto s1 = make_script("alpha", {{Opcode::HALT, 0}});
    auto s2 = make_script("beta",  {{Opcode::HALT, 0}});
    sys.RegisterScript(s1);
    sys.RegisterScript(s2);
    assert(sys.ScriptCount() == 2);
    auto names = sys.RegisteredScripts();
    assert(names.size() == 2);
    assert(names[0] == "alpha");
    assert(names[1] == "beta");

    std::cout << "[PASS] test_script_system_register" << std::endl;
}

void test_script_system_execute_tick() {
    ScriptSystem sys;
    // Script that reads atlas_tick and stores it to result
    auto s = make_script("tick_reader",
        {
            {Opcode::LOAD_VAR, 0},
            {Opcode::STORE_VAR, 1},
            {Opcode::HALT, 0}
        },
        {ScriptValue(std::string("atlas_tick")), ScriptValue(std::string("result"))});
    sys.RegisterScript(s);
    sys.ExecuteTick(42, 123);

    auto result = sys.GetVM().GetVariable("result");
    assert(result.type == ScriptValue::Type::Int && result.intVal == 42);

    // Check atlas_seed was also set
    auto seed = sys.GetVM().GetVariable("atlas_seed");
    assert(seed.type == ScriptValue::Type::Int && seed.intVal == 123);

    std::cout << "[PASS] test_script_system_execute_tick" << std::endl;
}

void test_script_system_validate_contracts() {
    ScriptSystem sys;
    // Script with deterministicDeclared = false should fail validation
    auto s = make_script("bad_script", {{Opcode::HALT, 0}}, {}, false);
    sys.RegisterScript(s);
    assert(!sys.ValidateContracts());
    assert(!sys.ValidationErrors().empty());

    // Script with deterministicDeclared = true should pass
    ScriptSystem sys2;
    auto s2 = make_script("good_script", {{Opcode::HALT, 0}}, {}, true);
    sys2.RegisterScript(s2);
    assert(sys2.ValidateContracts());

    std::cout << "[PASS] test_script_system_validate_contracts" << std::endl;
}

void test_script_system_combined_hash() {
    ScriptSystem sys1;
    ScriptSystem sys2;
    auto s = make_script("hash_script",
        {
            {Opcode::PUSH_INT, 0},
            {Opcode::STORE_VAR, 1},
            {Opcode::HALT, 0}
        },
        {ScriptValue(int64_t(42)), ScriptValue(std::string("x"))});
    sys1.RegisterScript(s);
    sys2.RegisterScript(s);
    sys1.ExecuteTick(1, 100);
    sys2.ExecuteTick(1, 100);
    assert(sys1.CombinedHash() == sys2.CombinedHash());

    std::cout << "[PASS] test_script_system_combined_hash" << std::endl;
}

void test_script_system_unregister() {
    ScriptSystem sys;
    sys.RegisterScript(make_script("a", {{Opcode::HALT, 0}}));
    sys.RegisterScript(make_script("b", {{Opcode::HALT, 0}}));
    assert(sys.ScriptCount() == 2);
    sys.UnregisterScript("a");
    assert(sys.ScriptCount() == 1);
    auto names = sys.RegisteredScripts();
    assert(names.size() == 1 && names[0] == "b");

    std::cout << "[PASS] test_script_system_unregister" << std::endl;
}

// ============================================================
// ScriptSandbox tests
// ============================================================

void test_sandbox_register_builtins() {
    ScriptVM vm;
    ScriptSandbox::RegisterBuiltins(vm);
    // After registration, calling atlas_abs from script should work
    // CALL is followed by a pseudo-instruction with the arg count
    auto s = make_script("builtin_test",
        {
            {Opcode::PUSH_INT, 0},
            {Opcode::CALL, 1},
            {Opcode::HALT, 1},   // pseudo-instruction: arg count = 1
            {Opcode::HALT, 0}
        },
        {ScriptValue(int64_t(-5)), ScriptValue(std::string("atlas_abs"))});
    auto r = vm.Execute(s);
    assert(r.type == ScriptValue::Type::Int && r.intVal == 5);

    std::cout << "[PASS] test_sandbox_register_builtins" << std::endl;
}

void test_sandbox_abs() {
    auto r = ScriptSandbox::Fn_atlas_abs({ScriptValue(int64_t(-5))});
    assert(r.type == ScriptValue::Type::Int && r.intVal == 5);

    auto r2 = ScriptSandbox::Fn_atlas_abs({ScriptValue(int64_t(3))});
    assert(r2.type == ScriptValue::Type::Int && r2.intVal == 3);

    auto r3 = ScriptSandbox::Fn_atlas_abs({ScriptValue(-2.5)});
    assert(r3.type == ScriptValue::Type::Float && std::abs(r3.floatVal - 2.5) < 1e-9);

    std::cout << "[PASS] test_sandbox_abs" << std::endl;
}

void test_sandbox_min_max() {
    auto mn = ScriptSandbox::Fn_atlas_min({ScriptValue(int64_t(3)), ScriptValue(int64_t(7))});
    assert(mn.type == ScriptValue::Type::Int && mn.intVal == 3);

    auto mx = ScriptSandbox::Fn_atlas_max({ScriptValue(int64_t(3)), ScriptValue(int64_t(7))});
    assert(mx.type == ScriptValue::Type::Int && mx.intVal == 7);

    std::cout << "[PASS] test_sandbox_min_max" << std::endl;
}

void test_sandbox_clamp() {
    // clamp(10, 0, 5) == 5
    auto r = ScriptSandbox::Fn_atlas_clamp(
        {ScriptValue(int64_t(10)), ScriptValue(int64_t(0)), ScriptValue(int64_t(5))});
    assert(r.type == ScriptValue::Type::Int && r.intVal == 5);

    // clamp(-3, 0, 5) == 0
    auto r2 = ScriptSandbox::Fn_atlas_clamp(
        {ScriptValue(int64_t(-3)), ScriptValue(int64_t(0)), ScriptValue(int64_t(5))});
    assert(r2.type == ScriptValue::Type::Int && r2.intVal == 0);

    // clamp(3, 0, 5) == 3
    auto r3 = ScriptSandbox::Fn_atlas_clamp(
        {ScriptValue(int64_t(3)), ScriptValue(int64_t(0)), ScriptValue(int64_t(5))});
    assert(r3.type == ScriptValue::Type::Int && r3.intVal == 3);

    std::cout << "[PASS] test_sandbox_clamp" << std::endl;
}

void test_sandbox_floor_ceil() {
    auto fl = ScriptSandbox::Fn_atlas_floor({ScriptValue(3.7)});
    assert(fl.type == ScriptValue::Type::Int && fl.intVal == 3);

    auto cl = ScriptSandbox::Fn_atlas_ceil({ScriptValue(3.2)});
    assert(cl.type == ScriptValue::Type::Int && cl.intVal == 4);

    std::cout << "[PASS] test_sandbox_floor_ceil" << std::endl;
}

void test_sandbox_sqrt() {
    auto r = ScriptSandbox::Fn_atlas_sqrt({ScriptValue(9.0)});
    assert(r.type == ScriptValue::Type::Float && std::abs(r.floatVal - 3.0) < 1e-9);

    std::cout << "[PASS] test_sandbox_sqrt" << std::endl;
}

void test_sandbox_strlen() {
    auto r = ScriptSandbox::Fn_atlas_strlen({ScriptValue(std::string("hello"))});
    assert(r.type == ScriptValue::Type::Int && r.intVal == 5);

    auto r2 = ScriptSandbox::Fn_atlas_strlen({ScriptValue(std::string(""))});
    assert(r2.type == ScriptValue::Type::Int && r2.intVal == 0);

    std::cout << "[PASS] test_sandbox_strlen" << std::endl;
}

void test_sandbox_builtin_names() {
    auto names = ScriptSandbox::BuiltinNames();
    assert(!names.empty());
    // Verify expected builtins are present
    auto has = [&](const std::string& n) {
        for (auto& s : names) if (s == n) return true;
        return false;
    };
    assert(has("atlas_abs"));
    assert(has("atlas_min"));
    assert(has("atlas_max"));
    assert(has("atlas_clamp"));
    assert(has("atlas_floor"));
    assert(has("atlas_ceil"));
    assert(has("atlas_sqrt"));
    assert(has("atlas_strlen"));

    std::cout << "[PASS] test_sandbox_builtin_names" << std::endl;
}

// ============================================================
// ABICapsule tests
// ============================================================

void test_abi_version_comparison() {
    ABIVersion v1{1, 0};
    ABIVersion v2{1, 0};
    ABIVersion v3{1, 1};
    ABIVersion v4{2, 0};

    assert(v1 == v2);
    assert(v1 != v3);
    assert(v1 < v3);
    assert(v1 < v4);
    assert(v3 < v4);

    std::cout << "[PASS] test_abi_version_comparison" << std::endl;
}

void test_abi_version_to_string() {
    ABIVersion v{1, 2};
    assert(v.ToString() == "atlas_abi_v1_2");

    ABIVersion v2{0, 0};
    assert(v2.ToString() == "atlas_abi_v0_0");

    std::cout << "[PASS] test_abi_version_to_string" << std::endl;
}

void test_abi_version_from_string() {
    auto v = ABIVersion::FromString("atlas_abi_v1_0");
    assert(v.major == 1 && v.minor == 0);

    auto v2 = ABIVersion::FromString("atlas_abi_v3_7");
    assert(v2.major == 3 && v2.minor == 7);

    std::cout << "[PASS] test_abi_version_from_string" << std::endl;
}

void test_abi_version_compatibility() {
    ABIVersion v1{1, 0};
    ABIVersion v2{1, 3};
    ABIVersion v3{2, 0};

    // Same major = compatible
    assert(v1.IsCompatibleWith(v2));
    assert(v2.IsCompatibleWith(v1));
    // Different major = not compatible
    assert(!v1.IsCompatibleWith(v3));
    assert(!v3.IsCompatibleWith(v1));

    std::cout << "[PASS] test_abi_version_compatibility" << std::endl;
}

void test_abi_function_table_empty() {
    ABIFunctionTable table;
    assert(!table.IsComplete());
    assert(table.BoundCount() == 0);

    std::cout << "[PASS] test_abi_function_table_empty" << std::endl;
}

void test_abi_function_table_partial() {
    ABIFunctionTable table;
    table.RegisterSystem = [](const char*) { return true; };
    table.StepSimulation = []() {};
    assert(!table.IsComplete());
    assert(table.BoundCount() == 2);

    std::cout << "[PASS] test_abi_function_table_partial" << std::endl;
}

void test_abi_function_table_complete() {
    ABIFunctionTable table;
    table.RegisterSystem  = [](const char*) { return true; };
    table.StepSimulation  = []() {};
    table.GetCurrentTick  = []() -> uint64_t { return 0; };
    table.GetComponent    = [](uint32_t, uint32_t, void*, uint32_t) { return true; };
    table.SetComponent    = [](uint32_t, uint32_t, const void*, uint32_t) { return true; };
    table.SubmitInput     = [](const void*, uint32_t) {};
    table.EmitEvent       = [](uint32_t, const void*, uint32_t) {};
    table.GetWorldHash    = []() -> uint64_t { return 0; };
    assert(table.IsComplete());
    assert(table.BoundCount() == 8);

    std::cout << "[PASS] test_abi_function_table_complete" << std::endl;
}

void test_abi_capsule_creation() {
    ABIVersion ver{1, 0};
    ABICapsule capsule(ver, "Test capsule v1.0");
    assert(capsule.Version() == ver);
    assert(capsule.Description() == "Test capsule v1.0");
    assert(!capsule.IsSealed());
    assert(!capsule.IsReady());

    std::cout << "[PASS] test_abi_capsule_creation" << std::endl;
}

void test_abi_capsule_seal() {
    ABIVersion ver{1, 0};
    ABICapsule capsule(ver, "Sealed capsule");
    assert(!capsule.IsSealed());
    capsule.Seal();
    assert(capsule.IsSealed());

    std::cout << "[PASS] test_abi_capsule_seal" << std::endl;
}

// ============================================================
// ABIRegistry tests
// ============================================================

static std::shared_ptr<ABICapsule> make_capsule(uint32_t major, uint32_t minor,
                                                 const std::string& desc = "")
{
    return std::make_shared<ABICapsule>(ABIVersion{major, minor}, desc);
}

void test_abi_registry_register() {
    ABIRegistry reg;
    auto c = make_capsule(1, 0, "v1.0");
    reg.RegisterCapsule(c);
    assert(reg.HasVersion(ABIVersion{1, 0}));
    assert(reg.CapsuleCount() == 1);
    assert(!reg.HasVersion(ABIVersion{2, 0}));

    std::cout << "[PASS] test_abi_registry_register" << std::endl;
}

void test_abi_registry_get_capsule() {
    ABIRegistry reg;
    auto c = make_capsule(1, 0, "v1.0");
    reg.RegisterCapsule(c);
    auto got = reg.GetCapsule(ABIVersion{1, 0});
    assert(got != nullptr);
    assert(got->Version() == (ABIVersion{1, 0}));
    assert(got->Description() == "v1.0");

    // Non-existent version
    auto missing = reg.GetCapsule(ABIVersion{9, 9});
    assert(missing == nullptr);

    std::cout << "[PASS] test_abi_registry_get_capsule" << std::endl;
}

void test_abi_registry_find_compatible() {
    ABIRegistry reg;
    reg.RegisterCapsule(make_capsule(1, 0));
    reg.RegisterCapsule(make_capsule(1, 2));
    reg.RegisterCapsule(make_capsule(2, 0));

    // Request v1.2 -> should find v1.2
    auto c = reg.FindCompatible(ABIVersion{1, 2});
    assert(c != nullptr);
    assert(c->Version() == (ABIVersion{1, 2}));

    // Request v1.5 -> should find v1.2 (highest minor <= 5 with major 1)
    auto c2 = reg.FindCompatible(ABIVersion{1, 5});
    assert(c2 != nullptr);
    assert(c2->Version().major == 1);

    // Request v3.0 -> no major 3 available
    auto c3 = reg.FindCompatible(ABIVersion{3, 0});
    assert(c3 == nullptr);

    std::cout << "[PASS] test_abi_registry_find_compatible" << std::endl;
}

void test_abi_registry_bind_project() {
    ABIRegistry reg;
    reg.RegisterCapsule(make_capsule(1, 0, "v1.0"));

    ProjectABITarget target;
    target.projectName = "MyGame";
    target.targetABI = ABIVersion{1, 0};
    target.determinismProfile = "strict";

    assert(reg.BindProject(target));
    assert(reg.IsProjectBound("MyGame"));
    auto c = reg.GetProjectCapsule("MyGame");
    assert(c != nullptr);
    assert(c->Version() == (ABIVersion{1, 0}));

    std::cout << "[PASS] test_abi_registry_bind_project" << std::endl;
}

void test_abi_registry_unbind() {
    ABIRegistry reg;
    reg.RegisterCapsule(make_capsule(1, 0));

    ProjectABITarget target;
    target.projectName = "TestProject";
    target.targetABI = ABIVersion{1, 0};
    target.determinismProfile = "strict";

    reg.BindProject(target);
    assert(reg.IsProjectBound("TestProject"));
    reg.UnbindProject("TestProject");
    assert(!reg.IsProjectBound("TestProject"));
    assert(reg.GetProjectCapsule("TestProject") == nullptr);

    std::cout << "[PASS] test_abi_registry_unbind" << std::endl;
}

void test_abi_registry_versions() {
    ABIRegistry reg;
    reg.RegisterCapsule(make_capsule(1, 0));
    reg.RegisterCapsule(make_capsule(1, 1));
    reg.RegisterCapsule(make_capsule(2, 0));

    auto versions = reg.RegisteredVersions();
    assert(versions.size() == 3);

    std::cout << "[PASS] test_abi_registry_versions" << std::endl;
}

// ============================================================
// Runner
// ============================================================

void run_script_and_abi_tests() {
    int passed = 0;
    int total  = 0;
    auto RUN = [&](auto fn, [[maybe_unused]] const char* name) {
        fn(); ++passed; ++total;
    };

    std::cout << "\n--- ScriptVM Tests ---" << std::endl;
    RUN(test_script_value_types,        "test_script_value_types");
    RUN(test_script_value_equality,     "test_script_value_equality");
    RUN(test_script_value_truthy,       "test_script_value_truthy");
    RUN(test_vm_push_int,               "test_vm_push_int");
    RUN(test_vm_arithmetic,             "test_vm_arithmetic");
    RUN(test_vm_division_by_zero,       "test_vm_division_by_zero");
    RUN(test_vm_float_arithmetic,       "test_vm_float_arithmetic");
    RUN(test_vm_comparison,             "test_vm_comparison");
    RUN(test_vm_logic,                  "test_vm_logic");
    RUN(test_vm_variables,              "test_vm_variables");
    RUN(test_vm_set_get_variable,       "test_vm_set_get_variable");
    RUN(test_vm_jump,                   "test_vm_jump");
    RUN(test_vm_jump_if,                "test_vm_jump_if");
    RUN(test_vm_call_native,            "test_vm_call_native");
    RUN(test_vm_budget,                 "test_vm_budget");
    RUN(test_vm_state_hash,             "test_vm_state_hash");
    RUN(test_vm_reset,                  "test_vm_reset");
    RUN(test_vm_neg,                    "test_vm_neg");
    RUN(test_vm_string_push,            "test_vm_string_push");
    RUN(test_vm_ret,                    "test_vm_ret");

    std::cout << "\n--- ScriptSystem Tests ---" << std::endl;
    RUN(test_script_system_register,          "test_script_system_register");
    RUN(test_script_system_execute_tick,      "test_script_system_execute_tick");
    RUN(test_script_system_validate_contracts,"test_script_system_validate_contracts");
    RUN(test_script_system_combined_hash,     "test_script_system_combined_hash");
    RUN(test_script_system_unregister,        "test_script_system_unregister");

    std::cout << "\n--- ScriptSandbox Tests ---" << std::endl;
    RUN(test_sandbox_register_builtins,  "test_sandbox_register_builtins");
    RUN(test_sandbox_abs,                "test_sandbox_abs");
    RUN(test_sandbox_min_max,            "test_sandbox_min_max");
    RUN(test_sandbox_clamp,              "test_sandbox_clamp");
    RUN(test_sandbox_floor_ceil,         "test_sandbox_floor_ceil");
    RUN(test_sandbox_sqrt,               "test_sandbox_sqrt");
    RUN(test_sandbox_strlen,             "test_sandbox_strlen");
    RUN(test_sandbox_builtin_names,      "test_sandbox_builtin_names");

    std::cout << "\n--- ABICapsule Tests ---" << std::endl;
    RUN(test_abi_version_comparison,     "test_abi_version_comparison");
    RUN(test_abi_version_to_string,      "test_abi_version_to_string");
    RUN(test_abi_version_from_string,    "test_abi_version_from_string");
    RUN(test_abi_version_compatibility,  "test_abi_version_compatibility");
    RUN(test_abi_function_table_empty,   "test_abi_function_table_empty");
    RUN(test_abi_function_table_partial, "test_abi_function_table_partial");
    RUN(test_abi_function_table_complete, "test_abi_function_table_complete");
    RUN(test_abi_capsule_creation,       "test_abi_capsule_creation");
    RUN(test_abi_capsule_seal,           "test_abi_capsule_seal");

    std::cout << "\n--- ABIRegistry Tests ---" << std::endl;
    RUN(test_abi_registry_register,       "test_abi_registry_register");
    RUN(test_abi_registry_get_capsule,    "test_abi_registry_get_capsule");
    RUN(test_abi_registry_find_compatible,"test_abi_registry_find_compatible");
    RUN(test_abi_registry_bind_project,   "test_abi_registry_bind_project");
    RUN(test_abi_registry_unbind,         "test_abi_registry_unbind");
    RUN(test_abi_registry_versions,       "test_abi_registry_versions");

    std::cout << "\n  Script & ABI tests: " << passed << "/" << total << " passed" << std::endl;
}
