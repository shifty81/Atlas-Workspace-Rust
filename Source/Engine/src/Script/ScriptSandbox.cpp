#include "ScriptSandbox.h"

#include <cmath>

namespace atlas::script {

// ============================================================
// Helpers
// ============================================================

static double ToFloat(const ScriptValue& v) {
    switch (v.type) {
        case ScriptValue::Type::Int:   return static_cast<double>(v.intVal);
        case ScriptValue::Type::Float: return v.floatVal;
        case ScriptValue::Type::Bool:  return static_cast<double>(v.intVal);
        default: return 0.0;
    }
}

static int64_t ToInt(const ScriptValue& v) {
    switch (v.type) {
        case ScriptValue::Type::Int:   return v.intVal;
        case ScriptValue::Type::Float: return static_cast<int64_t>(v.floatVal);
        case ScriptValue::Type::Bool:  return v.intVal;
        default: return 0;
    }
}

static bool IsFloat(const ScriptValue& v) {
    return v.type == ScriptValue::Type::Float;
}

// ============================================================
// Builtins
// ============================================================

ScriptValue ScriptSandbox::Fn_atlas_abs(const std::vector<ScriptValue>& args) {
    if (args.empty()) return ScriptValue{};
    const auto& a = args[0];
    if (IsFloat(a))
        return ScriptValue(std::fabs(a.floatVal));
    return ScriptValue(a.intVal < 0 ? -a.intVal : a.intVal);
}

ScriptValue ScriptSandbox::Fn_atlas_min(const std::vector<ScriptValue>& args) {
    if (args.size() < 2) return args.empty() ? ScriptValue{} : args[0];
    const auto& a = args[0];
    const auto& b = args[1];
    if (IsFloat(a) || IsFloat(b)) {
        double fa = ToFloat(a), fb = ToFloat(b);
        return ScriptValue(fa < fb ? fa : fb);
    }
    int64_t ia = ToInt(a), ib = ToInt(b);
    return ScriptValue(ia < ib ? ia : ib);
}

ScriptValue ScriptSandbox::Fn_atlas_max(const std::vector<ScriptValue>& args) {
    if (args.size() < 2) return args.empty() ? ScriptValue{} : args[0];
    const auto& a = args[0];
    const auto& b = args[1];
    if (IsFloat(a) || IsFloat(b)) {
        double fa = ToFloat(a), fb = ToFloat(b);
        return ScriptValue(fa > fb ? fa : fb);
    }
    int64_t ia = ToInt(a), ib = ToInt(b);
    return ScriptValue(ia > ib ? ia : ib);
}

ScriptValue ScriptSandbox::Fn_atlas_clamp(const std::vector<ScriptValue>& args) {
    if (args.size() < 3) return args.empty() ? ScriptValue{} : args[0];
    const auto& val = args[0];
    const auto& lo  = args[1];
    const auto& hi  = args[2];
    if (IsFloat(val) || IsFloat(lo) || IsFloat(hi)) {
        double fv = ToFloat(val), fl = ToFloat(lo), fh = ToFloat(hi);
        if (fv < fl) return ScriptValue(fl);
        if (fv > fh) return ScriptValue(fh);
        return ScriptValue(fv);
    }
    int64_t iv = ToInt(val), il = ToInt(lo), ih = ToInt(hi);
    if (iv < il) return ScriptValue(il);
    if (iv > ih) return ScriptValue(ih);
    return ScriptValue(iv);
}

ScriptValue ScriptSandbox::Fn_atlas_floor(const std::vector<ScriptValue>& args) {
    if (args.empty()) return ScriptValue{};
    return ScriptValue(static_cast<int64_t>(std::floor(ToFloat(args[0]))));
}

ScriptValue ScriptSandbox::Fn_atlas_ceil(const std::vector<ScriptValue>& args) {
    if (args.empty()) return ScriptValue{};
    return ScriptValue(static_cast<int64_t>(std::ceil(ToFloat(args[0]))));
}

ScriptValue ScriptSandbox::Fn_atlas_sqrt(const std::vector<ScriptValue>& args) {
    if (args.empty()) return ScriptValue{};
    double v = ToFloat(args[0]);
    return ScriptValue(v >= 0.0 ? std::sqrt(v) : 0.0);
}

ScriptValue ScriptSandbox::Fn_atlas_strlen(const std::vector<ScriptValue>& args) {
    if (args.empty()) return ScriptValue(int64_t{0});
    if (args[0].type == ScriptValue::Type::String)
        return ScriptValue(static_cast<int64_t>(args[0].strVal.size()));
    return ScriptValue(int64_t{0});
}

// ============================================================
// Registration
// ============================================================

void ScriptSandbox::RegisterBuiltins(ScriptVM& vm) {
    vm.RegisterFunction("atlas_abs",    Fn_atlas_abs);
    vm.RegisterFunction("atlas_min",    Fn_atlas_min);
    vm.RegisterFunction("atlas_max",    Fn_atlas_max);
    vm.RegisterFunction("atlas_clamp",  Fn_atlas_clamp);
    vm.RegisterFunction("atlas_floor",  Fn_atlas_floor);
    vm.RegisterFunction("atlas_ceil",   Fn_atlas_ceil);
    vm.RegisterFunction("atlas_sqrt",   Fn_atlas_sqrt);
    vm.RegisterFunction("atlas_strlen", Fn_atlas_strlen);
}

std::vector<std::string> ScriptSandbox::BuiltinNames() {
    return {
        "atlas_abs",
        "atlas_min",
        "atlas_max",
        "atlas_clamp",
        "atlas_floor",
        "atlas_ceil",
        "atlas_sqrt",
        "atlas_strlen"
    };
}

}  // namespace atlas::script
