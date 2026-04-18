// Tests/Workspace/test_phase62_variable_store.cpp
// Phase 62 — WorkspaceVariableStore: VariableType, Variable,
//             VariableScope, VariableStore
#include <catch2/catch_test_macros.hpp>
#include "NF/Workspace/WorkspaceVariableStore.h"

// ═════════════════════════════════════════════════════════════════
// VariableType
// ═════════════════════════════════════════════════════════════════

TEST_CASE("VariableType: name helpers", "[varstore][type]") {
    REQUIRE(std::string(NF::variableTypeName(NF::VariableType::String)) == "String");
    REQUIRE(std::string(NF::variableTypeName(NF::VariableType::Int))    == "Int");
    REQUIRE(std::string(NF::variableTypeName(NF::VariableType::Float))  == "Float");
    REQUIRE(std::string(NF::variableTypeName(NF::VariableType::Bool))   == "Bool");
    REQUIRE(std::string(NF::variableTypeName(NF::VariableType::Custom)) == "Custom");
}

// ═════════════════════════════════════════════════════════════════
// Variable
// ═════════════════════════════════════════════════════════════════

static NF::Variable makeVar(const std::string& key, const std::string& value,
                              NF::VariableType type = NF::VariableType::String,
                              const std::string& scope = "global") {
    NF::Variable v;
    v.key   = key;
    v.value = value;
    v.type  = type;
    v.scope = scope;
    return v;
}

TEST_CASE("Variable: default invalid", "[varstore][var]") {
    NF::Variable v;
    REQUIRE_FALSE(v.isValid());
}

TEST_CASE("Variable: valid with key", "[varstore][var]") {
    auto v = makeVar("myKey", "myValue");
    REQUIRE(v.isValid());
}

TEST_CASE("Variable: equality by key and scope", "[varstore][var]") {
    auto a = makeVar("k", "val1", NF::VariableType::String, "global");
    auto b = makeVar("k", "val2", NF::VariableType::Int, "global");
    auto c = makeVar("k", "val1", NF::VariableType::String, "local");
    REQUIRE(a == b);
    REQUIRE(a != c);
}

TEST_CASE("Variable: readOnly default false", "[varstore][var]") {
    auto v = makeVar("k", "v");
    REQUIRE_FALSE(v.readOnly);
}

// ═════════════════════════════════════════════════════════════════
// VariableScope
// ═════════════════════════════════════════════════════════════════

static NF::VariableScope makeScope(const std::string& id, const std::string& name,
                                    bool persistent = true) {
    NF::VariableScope s;
    s.id         = id;
    s.name       = name;
    s.persistent = persistent;
    return s;
}

TEST_CASE("VariableScope: default empty", "[varstore][scope]") {
    auto s = makeScope("global", "Global");
    REQUIRE(s.isValid());
    REQUIRE(s.empty());
    REQUIRE(s.count() == 0);
}

TEST_CASE("VariableScope: invalid without id", "[varstore][scope]") {
    NF::VariableScope s;
    s.name = "Global";
    REQUIRE_FALSE(s.isValid());
}

TEST_CASE("VariableScope: set and get", "[varstore][scope]") {
    auto s = makeScope("global", "Global");
    REQUIRE(s.set(makeVar("color", "red")));
    const auto* val = s.get("color");
    REQUIRE(val != nullptr);
    REQUIRE(*val == "red");
}

TEST_CASE("VariableScope: set updates existing", "[varstore][scope]") {
    auto s = makeScope("global", "Global");
    s.set(makeVar("x", "10", NF::VariableType::Int));
    REQUIRE(s.set(makeVar("x", "20", NF::VariableType::Int)));
    REQUIRE(*s.get("x") == "20");
    REQUIRE(s.count() == 1);
}

TEST_CASE("VariableScope: set rejects invalid", "[varstore][scope]") {
    auto s = makeScope("global", "Global");
    NF::Variable v;
    REQUIRE_FALSE(s.set(v));
}

TEST_CASE("VariableScope: set respects readOnly", "[varstore][scope]") {
    auto s = makeScope("global", "Global");
    auto v = makeVar("locked", "original");
    v.readOnly = true;
    s.set(v);
    REQUIRE_FALSE(s.set(makeVar("locked", "modified")));
    REQUIRE(*s.get("locked") == "original");
}

TEST_CASE("VariableScope: setReadOnly", "[varstore][scope]") {
    auto s = makeScope("global", "Global");
    s.set(makeVar("k", "v"));
    REQUIRE(s.setReadOnly("k", true));
    REQUIRE(s.find("k")->readOnly == true);
    REQUIRE(s.setReadOnly("k", false));
    REQUIRE(s.find("k")->readOnly == false);
}

TEST_CASE("VariableScope: setReadOnly unknown", "[varstore][scope]") {
    auto s = makeScope("global", "Global");
    REQUIRE_FALSE(s.setReadOnly("missing", true));
}

TEST_CASE("VariableScope: remove", "[varstore][scope]") {
    auto s = makeScope("global", "Global");
    s.set(makeVar("k", "v"));
    REQUIRE(s.remove("k"));
    REQUIRE(s.empty());
}

TEST_CASE("VariableScope: remove unknown", "[varstore][scope]") {
    auto s = makeScope("global", "Global");
    REQUIRE_FALSE(s.remove("missing"));
}

TEST_CASE("VariableScope: remove readOnly fails", "[varstore][scope]") {
    auto s = makeScope("global", "Global");
    auto v = makeVar("locked", "val");
    v.readOnly = true;
    s.set(v);
    REQUIRE_FALSE(s.remove("locked"));
    REQUIRE(s.contains("locked"));
}

TEST_CASE("VariableScope: filterByType", "[varstore][scope]") {
    auto s = makeScope("global", "Global");
    s.set(makeVar("a", "1", NF::VariableType::Int));
    s.set(makeVar("b", "2", NF::VariableType::Int));
    s.set(makeVar("c", "hello", NF::VariableType::String));
    auto ints = s.filterByType(NF::VariableType::Int);
    REQUIRE(ints.size() == 2);
}

TEST_CASE("VariableScope: clear", "[varstore][scope]") {
    auto s = makeScope("global", "Global");
    s.set(makeVar("a", "1"));
    s.set(makeVar("b", "2"));
    s.clear();
    REQUIRE(s.empty());
}

// ═════════════════════════════════════════════════════════════════
// VariableStore
// ═════════════════════════════════════════════════════════════════

TEST_CASE("VariableStore: default empty", "[varstore][main]") {
    NF::VariableStore store;
    REQUIRE(store.scopeCount() == 0);
    REQUIRE(store.totalVariables() == 0);
}

TEST_CASE("VariableStore: addScope", "[varstore][main]") {
    NF::VariableStore store;
    REQUIRE(store.addScope(makeScope("global", "Global")));
    REQUIRE(store.scopeCount() == 1);
    REQUIRE(store.hasScope("global"));
}

TEST_CASE("VariableStore: addScope rejects invalid", "[varstore][main]") {
    NF::VariableStore store;
    NF::VariableScope s;
    REQUIRE_FALSE(store.addScope(s));
}

TEST_CASE("VariableStore: addScope rejects duplicate", "[varstore][main]") {
    NF::VariableStore store;
    store.addScope(makeScope("global", "Global"));
    REQUIRE_FALSE(store.addScope(makeScope("global", "Global2")));
}

TEST_CASE("VariableStore: removeScope", "[varstore][main]") {
    NF::VariableStore store;
    store.addScope(makeScope("global", "Global"));
    REQUIRE(store.removeScope("global"));
    REQUIRE(store.scopeCount() == 0);
}

TEST_CASE("VariableStore: removeScope unknown", "[varstore][main]") {
    NF::VariableStore store;
    REQUIRE_FALSE(store.removeScope("missing"));
}

TEST_CASE("VariableStore: set and get", "[varstore][main]") {
    NF::VariableStore store;
    store.addScope(makeScope("global", "Global"));
    REQUIRE(store.set("global", makeVar("theme", "dark")));
    const auto* val = store.get("global", "theme");
    REQUIRE(val != nullptr);
    REQUIRE(*val == "dark");
}

TEST_CASE("VariableStore: set unknown scope", "[varstore][main]") {
    NF::VariableStore store;
    REQUIRE_FALSE(store.set("missing", makeVar("k", "v")));
}

TEST_CASE("VariableStore: set respects readOnly", "[varstore][main]") {
    NF::VariableStore store;
    store.addScope(makeScope("global", "Global"));
    auto v = makeVar("locked", "value");
    v.readOnly = true;
    store.set("global", v);
    REQUIRE_FALSE(store.set("global", makeVar("locked", "new_value")));
}

TEST_CASE("VariableStore: get unknown scope", "[varstore][main]") {
    NF::VariableStore store;
    REQUIRE(store.get("missing", "k") == nullptr);
}

TEST_CASE("VariableStore: remove", "[varstore][main]") {
    NF::VariableStore store;
    store.addScope(makeScope("global", "Global"));
    store.set("global", makeVar("k", "v"));
    REQUIRE(store.remove("global", "k"));
    REQUIRE(store.totalVariables() == 0);
}

TEST_CASE("VariableStore: contains", "[varstore][main]") {
    NF::VariableStore store;
    store.addScope(makeScope("global", "Global"));
    REQUIRE_FALSE(store.contains("global", "k"));
    store.set("global", makeVar("k", "v"));
    REQUIRE(store.contains("global", "k"));
}

TEST_CASE("VariableStore: searchByKey", "[varstore][main]") {
    NF::VariableStore store;
    store.addScope(makeScope("global", "Global"));
    store.addScope(makeScope("local", "Local"));
    store.set("global", makeVar("editor.theme", "dark"));
    store.set("global", makeVar("editor.fontSize", "14"));
    store.set("local", makeVar("editor.zoom", "1.0"));

    auto results = store.searchByKey("editor");
    REQUIRE(results.size() == 3);
}

TEST_CASE("VariableStore: searchByKey empty", "[varstore][main]") {
    NF::VariableStore store;
    store.addScope(makeScope("global", "Global"));
    store.set("global", makeVar("k", "v"));
    REQUIRE(store.searchByKey("").empty());
}

TEST_CASE("VariableStore: observer on set (new)", "[varstore][main]") {
    NF::VariableStore store;
    store.addScope(makeScope("global", "Global"));

    std::string observedScope, observedKey, observedOld, observedNew;
    store.addObserver([&](const std::string& sc, const std::string& k,
                          const std::string& o, const std::string& n) {
        observedScope = sc; observedKey = k; observedOld = o; observedNew = n;
    });

    store.set("global", makeVar("color", "blue"));
    REQUIRE(observedScope == "global");
    REQUIRE(observedKey == "color");
    REQUIRE(observedOld.empty());
    REQUIRE(observedNew == "blue");
}

TEST_CASE("VariableStore: observer on set (update)", "[varstore][main]") {
    NF::VariableStore store;
    store.addScope(makeScope("global", "Global"));
    store.set("global", makeVar("x", "10"));

    std::string observedOld, observedNew;
    store.addObserver([&](const std::string&, const std::string&,
                          const std::string& o, const std::string& n) {
        observedOld = o; observedNew = n;
    });

    store.set("global", makeVar("x", "20"));
    REQUIRE(observedOld == "10");
    REQUIRE(observedNew == "20");
}

TEST_CASE("VariableStore: clearObservers", "[varstore][main]") {
    NF::VariableStore store;
    store.addScope(makeScope("global", "Global"));
    int calls = 0;
    store.addObserver([&](const std::string&, const std::string&,
                          const std::string&, const std::string&) { ++calls; });
    store.clearObservers();
    store.set("global", makeVar("k", "v"));
    REQUIRE(calls == 0);
}

TEST_CASE("VariableStore: serialize empty", "[varstore][serial]") {
    NF::VariableStore store;
    REQUIRE(store.serialize().empty());
}

TEST_CASE("VariableStore: serialize round-trip", "[varstore][serial]") {
    NF::VariableStore store;
    store.addScope(makeScope("global", "Global", true));
    store.addScope(makeScope("session", "Session", false));
    store.set("global", makeVar("theme", "dark", NF::VariableType::String));
    auto v = makeVar("debug", "true", NF::VariableType::Bool);
    v.readOnly = true;
    v.description = "Enable debug";
    store.set("global", v);
    store.set("session", makeVar("zoom", "1.5", NF::VariableType::Float));

    std::string data = store.serialize();
    REQUIRE_FALSE(data.empty());

    NF::VariableStore store2;
    REQUIRE(store2.deserialize(data));
    REQUIRE(store2.scopeCount() == 2);
    REQUIRE(store2.totalVariables() == 3);

    REQUIRE(*store2.get("global", "theme") == "dark");
    REQUIRE(*store2.get("session", "zoom") == "1.5");

    const auto* debugScope = store2.findScope("global");
    REQUIRE(debugScope != nullptr);
    const auto* debugVar = debugScope->find("debug");
    REQUIRE(debugVar != nullptr);
    REQUIRE(debugVar->readOnly == true);
    REQUIRE(debugVar->description == "Enable debug");

    const auto* sessionScope = store2.findScope("session");
    REQUIRE(sessionScope != nullptr);
    REQUIRE(sessionScope->persistent == false);
}

TEST_CASE("VariableStore: serialize pipe in value", "[varstore][serial]") {
    NF::VariableStore store;
    store.addScope(makeScope("global", "Global"));
    store.set("global", makeVar("opts", "a|b|c"));

    std::string data = store.serialize();
    NF::VariableStore store2;
    store2.deserialize(data);
    REQUIRE(*store2.get("global", "opts") == "a|b|c");
}

TEST_CASE("VariableStore: deserialize empty", "[varstore][serial]") {
    NF::VariableStore store;
    REQUIRE(store.deserialize(""));
    REQUIRE(store.scopeCount() == 0);
}

TEST_CASE("VariableStore: clear", "[varstore][main]") {
    NF::VariableStore store;
    store.addScope(makeScope("global", "Global"));
    store.set("global", makeVar("k", "v"));
    store.clear();
    REQUIRE(store.scopeCount() == 0);
    REQUIRE(store.totalVariables() == 0);
}

// ═════════════════════════════════════════════════════════════════
// Integration
// ═════════════════════════════════════════════════════════════════

TEST_CASE("Integration: multi-scope variable store", "[varstore][integration]") {
    NF::VariableStore store;
    store.addScope(makeScope("global", "Global"));
    store.addScope(makeScope("project", "Project"));
    store.addScope(makeScope("user", "User"));

    store.set("global", makeVar("app.theme", "dark"));
    store.set("global", makeVar("app.language", "en-US"));
    store.set("project", makeVar("build.target", "Debug"));
    store.set("project", makeVar("build.platform", "Windows"));
    store.set("user", makeVar("user.name", "Alice"));

    REQUIRE(store.totalVariables() == 5);
    REQUIRE(store.searchByKey("build").size() == 2);
    REQUIRE(store.searchByKey("app").size() == 2);
    REQUIRE(store.searchByKey("user").size() == 1);
}

TEST_CASE("Integration: serialize/deserialize preserves scope and variable data", "[varstore][integration]") {
    NF::VariableStore store;
    store.addScope(makeScope("global", "Global", true));
    store.addScope(makeScope("temp", "Temp", false));

    store.set("global", makeVar("a", "alpha"));
    store.set("global", makeVar("b", "beta|gamma"));
    store.set("temp", makeVar("t", "transient"));

    std::string data = store.serialize();
    NF::VariableStore store2;
    store2.deserialize(data);

    REQUIRE(store2.scopeCount() == 2);
    REQUIRE(*store2.get("global", "a") == "alpha");
    REQUIRE(*store2.get("global", "b") == "beta|gamma");
    REQUIRE(*store2.get("temp", "t") == "transient");
    REQUIRE(store2.findScope("temp")->persistent == false);
}
