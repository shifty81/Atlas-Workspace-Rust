// Phase 19: ProceduralGenerator, AIDebuggerPanel, HotReloadConfig tests

#include "../engine/ai/ProceduralGenerator.h"
#include "../editor/panels/AIDebuggerPanel.h"
#include "../engine/sim/HotReloadConfig.h"
#include <cassert>
#include <iostream>
#include <set>

using namespace atlas::ai;
using namespace atlas::editor;
using namespace atlas::sim;

// =============================================================
// ProceduralGenerator Tests
// =============================================================

void test_procedural_generator_default_seed() {
    ProceduralGenerator gen;
    assert(gen.GetSeed() == 42);
    std::cout << "  [PASS] test_procedural_generator_default_seed" << std::endl;
}

void test_procedural_generator_set_seed() {
    ProceduralGenerator gen;
    gen.SetSeed(12345);
    assert(gen.GetSeed() == 12345);

    // Different seed should produce different output
    ProceduralGenerator gen2;
    gen2.SetSeed(99999);
    auto name1 = gen.GenerateName("npc", NameCulture::Generic);
    auto name2 = gen2.GenerateName("npc", NameCulture::Generic);
    assert(name1.value != name2.value);
    std::cout << "  [PASS] test_procedural_generator_set_seed" << std::endl;
}

void test_procedural_generator_name_npc() {
    ProceduralGenerator gen;
    auto name = gen.GenerateName("npc", NameCulture::Nordic);
    assert(!name.value.empty());
    assert(name.category == "npc");
    assert(name.culture == NameCulture::Nordic);
    std::cout << "  [PASS] test_procedural_generator_name_npc" << std::endl;
}

void test_procedural_generator_name_place() {
    ProceduralGenerator gen;
    auto name = gen.GenerateName("place", NameCulture::Fantasy);
    assert(!name.value.empty());
    assert(name.category == "place");
    std::cout << "  [PASS] test_procedural_generator_name_place" << std::endl;
}

void test_procedural_generator_name_item() {
    ProceduralGenerator gen;
    auto name = gen.GenerateName("item", NameCulture::SciFi);
    assert(!name.value.empty());
    assert(name.category == "item");
    std::cout << "  [PASS] test_procedural_generator_name_item" << std::endl;
}

void test_procedural_generator_names_batch() {
    ProceduralGenerator gen;
    auto names = gen.GenerateNames("npc", NameCulture::Generic, 5);
    assert(names.size() == 5);
    for (const auto& n : names) {
        assert(!n.value.empty());
        assert(n.category == "npc");
    }
    std::cout << "  [PASS] test_procedural_generator_names_batch" << std::endl;
}

void test_procedural_generator_name_cultures() {
    ProceduralGenerator gen;
    std::set<std::string> seen;
    NameCulture cultures[] = {
        NameCulture::Generic, NameCulture::Nordic, NameCulture::Eastern,
        NameCulture::Latin, NameCulture::SciFi, NameCulture::Fantasy
    };

    for (auto c : cultures) {
        gen.SetSeed(42);
        auto name = gen.GenerateName("npc", c);
        assert(!name.value.empty());
        seen.insert(name.value);
    }
    // At least some cultures should produce different names
    assert(seen.size() > 1);
    std::cout << "  [PASS] test_procedural_generator_name_cultures" << std::endl;
}

void test_procedural_generator_biome_forest() {
    ProceduralGenerator gen;
    auto assets = gen.GenerateBiomeAssets(BiomeType::Forest);
    assert(assets.biome == BiomeType::Forest);
    assert(!assets.meshSuggestions.empty());
    assert(!assets.textureSuggestions.empty());
    std::cout << "  [PASS] test_procedural_generator_biome_forest" << std::endl;
}

void test_procedural_generator_biome_desert() {
    ProceduralGenerator gen;
    auto assets = gen.GenerateBiomeAssets(BiomeType::Desert);
    assert(assets.biome == BiomeType::Desert);
    assert(!assets.meshSuggestions.empty());
    assert(!assets.textureSuggestions.empty());
    std::cout << "  [PASS] test_procedural_generator_biome_desert" << std::endl;
}

void test_procedural_generator_all_biomes() {
    ProceduralGenerator gen;
    auto allBiomes = gen.GenerateAllBiomeAssets();
    assert(allBiomes.size() == 9);
    for (const auto& b : allBiomes) {
        assert(!b.meshSuggestions.empty());
    }
    std::cout << "  [PASS] test_procedural_generator_all_biomes" << std::endl;
}

void test_procedural_generator_quest_combat() {
    ProceduralGenerator gen;
    auto quest = gen.GenerateQuest(QuestType::Combat, 3);
    assert(quest.type == QuestType::Combat);
    assert(!quest.title.empty());
    assert(!quest.description.empty());
    assert(!quest.objectives.empty());
    assert(quest.estimatedDifficulty == 3);
    std::cout << "  [PASS] test_procedural_generator_quest_combat" << std::endl;
}

void test_procedural_generator_quest_line() {
    ProceduralGenerator gen;
    auto quests = gen.GenerateQuestLine(4, 1);
    assert(quests.size() == 4);
    for (size_t i = 1; i < quests.size(); ++i) {
        assert(quests[i].estimatedDifficulty >= quests[i - 1].estimatedDifficulty);
    }
    std::cout << "  [PASS] test_procedural_generator_quest_line" << std::endl;
}

void test_procedural_generator_shaders() {
    ProceduralGenerator gen;
    BiomeType biomes[] = {
        BiomeType::Forest, BiomeType::Desert, BiomeType::Mountain,
        BiomeType::Ocean, BiomeType::Tundra, BiomeType::Swamp,
        BiomeType::Plains, BiomeType::Volcanic, BiomeType::Urban
    };
    for (auto b : biomes) {
        auto shaders = gen.SuggestShaders(b);
        assert(!shaders.empty());
        for (const auto& s : shaders) {
            assert(!s.name.empty());
        }
    }
    std::cout << "  [PASS] test_procedural_generator_shaders" << std::endl;
}

void test_procedural_generator_statistics() {
    ProceduralGenerator gen;
    assert(gen.GetGenerationCount() == 0);

    gen.GenerateName("npc", NameCulture::Generic);
    assert(gen.GetGenerationCount() > 0);
    uint32_t count = gen.GetGenerationCount();

    gen.GenerateBiomeAssets(BiomeType::Forest);
    assert(gen.GetGenerationCount() > count);

    gen.ResetStatistics();
    assert(gen.GetGenerationCount() == 0);
    std::cout << "  [PASS] test_procedural_generator_statistics" << std::endl;
}

void test_procedural_generator_deterministic() {
    ProceduralGenerator gen1;
    gen1.SetSeed(777);
    auto name1 = gen1.GenerateName("npc", NameCulture::Nordic);
    auto biome1 = gen1.GenerateBiomeAssets(BiomeType::Forest);

    ProceduralGenerator gen2;
    gen2.SetSeed(777);
    auto name2 = gen2.GenerateName("npc", NameCulture::Nordic);
    auto biome2 = gen2.GenerateBiomeAssets(BiomeType::Forest);

    assert(name1.value == name2.value);
    assert(biome1.meshSuggestions == biome2.meshSuggestions);
    assert(biome1.textureSuggestions == biome2.textureSuggestions);
    std::cout << "  [PASS] test_procedural_generator_deterministic" << std::endl;
}

// =============================================================
// AIDebuggerPanel Tests
// =============================================================

void test_ai_debugger_panel_name() {
    AIDebuggerPanel panel;
    assert(std::string(panel.Name()) == "AI Debugger");
    std::cout << "  [PASS] test_ai_debugger_panel_name" << std::endl;
}

void test_ai_debugger_panel_empty() {
    AIDebuggerPanel panel;
    assert(panel.DiagnosticCount() == 0);
    assert(panel.GetDiagnostics().empty());
    std::cout << "  [PASS] test_ai_debugger_panel_empty" << std::endl;
}

void test_ai_debugger_panel_add_diagnostic() {
    AIDebuggerPanel panel;
    AIDiagnosticEntry entry;
    entry.systemName = "Physics";
    entry.issue = "Object falling through floor";
    entry.suggestedFix = "Increase collision margin";
    entry.severity = DiagnosticSeverity::Warning;

    panel.AddDiagnostic(entry);
    assert(panel.DiagnosticCount() == 1);

    panel.AddDiagnostic(entry);
    assert(panel.DiagnosticCount() == 2);
    std::cout << "  [PASS] test_ai_debugger_panel_add_diagnostic" << std::endl;
}

void test_ai_debugger_panel_clear() {
    AIDebuggerPanel panel;
    AIDiagnosticEntry entry;
    entry.systemName = "AI";
    entry.issue = "NPC stuck";
    panel.AddDiagnostic(entry);
    panel.AddDiagnostic(entry);
    assert(panel.DiagnosticCount() == 2);

    panel.ClearDiagnostics();
    assert(panel.DiagnosticCount() == 0);
    std::cout << "  [PASS] test_ai_debugger_panel_clear" << std::endl;
}

void test_ai_debugger_panel_select() {
    AIDebuggerPanel panel;
    assert(!panel.HasSelection());

    AIDiagnosticEntry entry;
    entry.systemName = "Render";
    entry.issue = "Flickering shadows";
    panel.AddDiagnostic(entry);
    panel.AddDiagnostic(entry);

    panel.SelectEntry(1);
    assert(panel.HasSelection());
    assert(panel.SelectedIndex() == 1);
    std::cout << "  [PASS] test_ai_debugger_panel_select" << std::endl;
}

void test_ai_debugger_panel_severity_filter() {
    AIDebuggerPanel panel;

    AIDiagnosticEntry info;
    info.systemName = "UI";
    info.issue = "Minor layout shift";
    info.severity = DiagnosticSeverity::Info;

    AIDiagnosticEntry warning;
    warning.systemName = "Physics";
    warning.issue = "Jittery collision";
    warning.severity = DiagnosticSeverity::Warning;

    AIDiagnosticEntry error;
    error.systemName = "AI";
    error.issue = "NPC crash";
    error.severity = DiagnosticSeverity::Error;

    panel.AddDiagnostic(info);
    panel.AddDiagnostic(warning);
    panel.AddDiagnostic(error);

    panel.SetSeverityFilter(DiagnosticSeverity::Warning);
    auto filtered = panel.GetFilteredDiagnostics();
    assert(filtered.size() == 2);
    for (const auto& d : filtered) {
        assert(d.severity >= DiagnosticSeverity::Warning);
    }
    std::cout << "  [PASS] test_ai_debugger_panel_severity_filter" << std::endl;
}

void test_ai_debugger_panel_apply_fix() {
    AIDebuggerPanel panel;
    AIDiagnosticEntry entry;
    entry.systemName = "Render";
    entry.issue = "Shadow acne";
    entry.suggestedFix = "Increase bias";
    panel.AddDiagnostic(entry);

    assert(panel.AppliedFixCount() == 0);
    bool ok = panel.ApplyFix(0);
    assert(ok);
    assert(panel.AppliedFixCount() == 1);
    assert(panel.GetDiagnostics()[0].fixApplied);
    std::cout << "  [PASS] test_ai_debugger_panel_apply_fix" << std::endl;
}

void test_ai_debugger_panel_apply_fix_callback() {
    AIDebuggerPanel panel;
    bool callbackFired = false;
    std::string fixedSystem;

    panel.SetOnFixApplied([&](const AIDiagnosticEntry& e) {
        callbackFired = true;
        fixedSystem = e.systemName;
    });

    AIDiagnosticEntry entry;
    entry.systemName = "WorldGen";
    entry.issue = "Missing chunk";
    entry.suggestedFix = "Regenerate chunk";
    panel.AddDiagnostic(entry);

    panel.ApplyFix(0);
    assert(callbackFired);
    assert(fixedSystem == "WorldGen");
    std::cout << "  [PASS] test_ai_debugger_panel_apply_fix_callback" << std::endl;
}

void test_ai_debugger_panel_draw() {
    AIDebuggerPanel panel;
    panel.SetBounds(0, 0, 400, 300);

    AIDiagnosticEntry entry;
    entry.systemName = "AI";
    entry.issue = "Stuck NPC";
    panel.AddDiagnostic(entry);

    panel.Draw();
    const auto& drawList = panel.GetDrawList();
    assert(drawList.CommandCount() > 0);
    std::cout << "  [PASS] test_ai_debugger_panel_draw" << std::endl;
}

void test_ai_debugger_panel_llm_status() {
    AIDebuggerPanel panel;
    assert(!panel.IsLLMConnected());
    assert(panel.GetLLMBackend() == nullptr);
    std::cout << "  [PASS] test_ai_debugger_panel_llm_status" << std::endl;
}

// =============================================================
// HotReloadConfig Tests
// =============================================================

void test_hotreload_config_register() {
    HotReloadConfig config;
    config.RegisterParam("speed", 10, "Movement speed");
    assert(config.HasParam("speed"));
    assert(!config.HasParam("gravity"));
    assert(config.ParamCount() == 1);
    std::cout << "  [PASS] test_hotreload_config_register" << std::endl;
}

void test_hotreload_config_get_set() {
    HotReloadConfig config;
    config.RegisterParam("health", 100);
    auto result = config.SetValue("health", 200);
    assert(result.success);
    auto val = config.GetValue("health");
    assert(std::get<int32_t>(val) == 200);
    std::cout << "  [PASS] test_hotreload_config_get_set" << std::endl;
}

void test_hotreload_config_typed_get() {
    HotReloadConfig config;
    config.RegisterParam("count", int32_t(42));
    int32_t val = config.GetAs<int32_t>("count");
    assert(val == 42);
    std::cout << "  [PASS] test_hotreload_config_typed_get" << std::endl;
}

void test_hotreload_config_lock() {
    HotReloadConfig config;
    config.RegisterParam("gravity", 9.8f);
    config.LockParam("gravity");
    assert(config.IsLocked("gravity"));

    auto result = config.SetValue("gravity", 0.0f);
    assert(!result.success);

    // Value unchanged
    float val = config.GetAs<float>("gravity");
    assert(val == 9.8f);
    std::cout << "  [PASS] test_hotreload_config_lock" << std::endl;
}

void test_hotreload_config_tick_gating() {
    HotReloadConfig config;
    config.RegisterParam("step", int32_t(1));

    config.BeginTick();
    assert(config.IsInTick());

    auto result = config.SetValue("step", int32_t(2));
    assert(!result.success);

    config.EndTick();
    assert(!config.IsInTick());

    result = config.SetValue("step", int32_t(2));
    assert(result.success);
    std::cout << "  [PASS] test_hotreload_config_tick_gating" << std::endl;
}

void test_hotreload_config_versioning() {
    HotReloadConfig config;
    config.RegisterParam("val", int32_t(0));
    uint32_t v0 = config.GetGlobalVersion();

    config.SetValue("val", int32_t(1));
    uint32_t v1 = config.GetGlobalVersion();
    assert(v1 > v0);

    config.SetValue("val", int32_t(2));
    uint32_t v2 = config.GetGlobalVersion();
    assert(v2 > v1);

    assert(config.GetParamVersion("val") >= 2);
    std::cout << "  [PASS] test_hotreload_config_versioning" << std::endl;
}

void test_hotreload_config_reset() {
    HotReloadConfig config;
    config.RegisterParam("damage", int32_t(50));
    config.SetValue("damage", int32_t(999));
    assert(config.GetAs<int32_t>("damage") == 999);

    config.ResetToDefault("damage");
    assert(config.GetAs<int32_t>("damage") == 50);
    std::cout << "  [PASS] test_hotreload_config_reset" << std::endl;
}

void test_hotreload_config_callback() {
    HotReloadConfig config;
    config.RegisterParam("volume", 0.5f);

    bool fired = false;
    std::string changedKey;
    config.SetChangeCallback([&](const std::string& key, const ConfigValue&, const ConfigValue& newVal) {
        fired = true;
        changedKey = key;
        assert(std::get<float>(newVal) == 0.8f);
    });

    config.SetValue("volume", 0.8f);
    assert(fired);
    assert(changedKey == "volume");
    std::cout << "  [PASS] test_hotreload_config_callback" << std::endl;
}

void test_hotreload_config_changed_keys() {
    HotReloadConfig config;
    config.RegisterParam("a", int32_t(1));
    config.RegisterParam("b", int32_t(2));
    config.RegisterParam("c", int32_t(3));

    config.SetValue("a", int32_t(10));
    config.SetValue("c", int32_t(30));

    auto changed = config.GetChangedKeys();
    assert(changed.size() == 2);

    config.ClearChangedKeys();
    assert(config.GetChangedKeys().empty());
    std::cout << "  [PASS] test_hotreload_config_changed_keys" << std::endl;
}

void test_hotreload_config_all_params() {
    HotReloadConfig config;
    config.RegisterParam("x", int32_t(1));
    config.RegisterParam("y", int32_t(2));
    config.RegisterParam("z", int32_t(3));

    auto all = config.GetAllParams();
    assert(all.size() == 3);
    std::cout << "  [PASS] test_hotreload_config_all_params" << std::endl;
}

// =============================================================
// Runner
// =============================================================

void register_next_tasks_phase19() {
    std::cout << "\n--- Phase 19: ProceduralGenerator ---" << std::endl;
    test_procedural_generator_default_seed();
    test_procedural_generator_set_seed();
    test_procedural_generator_name_npc();
    test_procedural_generator_name_place();
    test_procedural_generator_name_item();
    test_procedural_generator_names_batch();
    test_procedural_generator_name_cultures();
    test_procedural_generator_biome_forest();
    test_procedural_generator_biome_desert();
    test_procedural_generator_all_biomes();
    test_procedural_generator_quest_combat();
    test_procedural_generator_quest_line();
    test_procedural_generator_shaders();
    test_procedural_generator_statistics();
    test_procedural_generator_deterministic();

    std::cout << "\n--- Phase 19: AIDebuggerPanel ---" << std::endl;
    test_ai_debugger_panel_name();
    test_ai_debugger_panel_empty();
    test_ai_debugger_panel_add_diagnostic();
    test_ai_debugger_panel_clear();
    test_ai_debugger_panel_select();
    test_ai_debugger_panel_severity_filter();
    test_ai_debugger_panel_apply_fix();
    test_ai_debugger_panel_apply_fix_callback();
    test_ai_debugger_panel_draw();
    test_ai_debugger_panel_llm_status();

    std::cout << "\n--- Phase 19: HotReloadConfig ---" << std::endl;
    test_hotreload_config_register();
    test_hotreload_config_get_set();
    test_hotreload_config_typed_get();
    test_hotreload_config_lock();
    test_hotreload_config_tick_gating();
    test_hotreload_config_versioning();
    test_hotreload_config_reset();
    test_hotreload_config_callback();
    test_hotreload_config_changed_keys();
    test_hotreload_config_all_params();
}
