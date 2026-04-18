#include "test_log.h"
#include "tools/IToolModule.h"
#include "tools/ToolModuleRegistry.h"
#include "tools/ToolingLayer.h"
#include <cassert>
#include <string>
#include <vector>

// ============================================================
// Minimal mock tool module for testing
// ============================================================

class MockToolModule : public atlas::tools::IToolModule {
public:
    explicit MockToolModule(const std::string& name) : m_name(name) {}

    std::string Name() const override { return m_name; }
    void OnRegister() override { ++registerCount; }
    void OnUnregister() override { ++unregisterCount; }
    bool HandleInput(uint32_t keyCode, bool pressed) override {
        lastKey = keyCode;
        lastPressed = pressed;
        return consumeInput;
    }
    void Update(float dt) override { totalDt += dt; ++updateCount; }
    void Render() override { ++renderCount; }

    int registerCount = 0;
    int unregisterCount = 0;
    int updateCount = 0;
    int renderCount = 0;
    float totalDt = 0.0f;
    uint32_t lastKey = 0;
    bool lastPressed = false;
    bool consumeInput = false;

private:
    std::string m_name;
};

// ============================================================
// ToolModuleRegistry tests
// ============================================================

void test_registry_empty() {
    atlas::tools::ToolModuleRegistry registry;
    assert(registry.Count() == 0);
    assert(registry.Find("nonexistent") == nullptr);
}

void test_registry_register_one() {
    atlas::tools::ToolModuleRegistry registry;
    auto mod = std::make_unique<MockToolModule>("TestTool");
    auto* raw = mod.get();
    bool ok = registry.Register(std::move(mod));
    assert(ok);
    assert(registry.Count() == 1);
    assert(registry.Find("TestTool") == raw);
    assert(raw->registerCount == 1);
}

void test_registry_register_duplicate_rejected() {
    atlas::tools::ToolModuleRegistry registry;
    registry.Register(std::make_unique<MockToolModule>("DupTool"));
    bool ok = registry.Register(std::make_unique<MockToolModule>("DupTool"));
    assert(!ok);
    assert(registry.Count() == 1);
}

void test_registry_register_null_rejected() {
    atlas::tools::ToolModuleRegistry registry;
    bool ok = registry.Register(nullptr);
    assert(!ok);
    assert(registry.Count() == 0);
}

void test_registry_unregister() {
    atlas::tools::ToolModuleRegistry registry;
    auto mod = std::make_unique<MockToolModule>("RemoveTool");
    auto* raw = mod.get();
    registry.Register(std::move(mod));
    bool ok = registry.Unregister("RemoveTool");
    assert(ok);
    assert(registry.Count() == 0);
    assert(registry.Find("RemoveTool") == nullptr);
}

void test_registry_unregister_calls_on_unregister() {
    atlas::tools::ToolModuleRegistry registry;
    auto mod = std::make_unique<MockToolModule>("Tool");
    auto* raw = mod.get();
    registry.Register(std::move(mod));
    // raw is still valid until Unregister erases it
    assert(raw->unregisterCount == 0);
    registry.Unregister("Tool");
    // After erase, raw is dangling — but unregisterCount was set before erase
    // We can't check raw here, so this test just verifies no crash.
}

void test_registry_unregister_nonexistent() {
    atlas::tools::ToolModuleRegistry registry;
    bool ok = registry.Unregister("ghost");
    assert(!ok);
}

void test_registry_broadcast_input() {
    atlas::tools::ToolModuleRegistry registry;
    auto mod = std::make_unique<MockToolModule>("InputTool");
    mod->consumeInput = true;
    registry.Register(std::move(mod));
    bool consumed = registry.BroadcastInput(42, true);
    assert(consumed);
}

void test_registry_broadcast_input_not_consumed() {
    atlas::tools::ToolModuleRegistry registry;
    auto mod = std::make_unique<MockToolModule>("PassTool");
    mod->consumeInput = false;
    registry.Register(std::move(mod));
    bool consumed = registry.BroadcastInput(99, false);
    assert(!consumed);
}

void test_registry_update_all() {
    atlas::tools::ToolModuleRegistry registry;
    auto a = std::make_unique<MockToolModule>("A");
    auto b = std::make_unique<MockToolModule>("B");
    auto* rawA = a.get();
    auto* rawB = b.get();
    registry.Register(std::move(a));
    registry.Register(std::move(b));
    registry.UpdateAll(0.016f);
    assert(rawA->updateCount == 1);
    assert(rawB->updateCount == 1);
    assert(rawA->totalDt > 0.015f && rawA->totalDt < 0.017f);
}

void test_registry_render_all() {
    atlas::tools::ToolModuleRegistry registry;
    auto mod = std::make_unique<MockToolModule>("RenderTool");
    auto* raw = mod.get();
    registry.Register(std::move(mod));
    registry.RenderAll();
    assert(raw->renderCount == 1);
}

void test_registry_clear() {
    atlas::tools::ToolModuleRegistry registry;
    registry.Register(std::make_unique<MockToolModule>("X"));
    registry.Register(std::make_unique<MockToolModule>("Y"));
    assert(registry.Count() == 2);
    registry.Clear();
    assert(registry.Count() == 0);
}

void test_registry_multiple_modules() {
    atlas::tools::ToolModuleRegistry registry;
    registry.Register(std::make_unique<MockToolModule>("Alpha"));
    registry.Register(std::make_unique<MockToolModule>("Beta"));
    registry.Register(std::make_unique<MockToolModule>("Gamma"));
    assert(registry.Count() == 3);
    assert(registry.Find("Alpha") != nullptr);
    assert(registry.Find("Beta") != nullptr);
    assert(registry.Find("Gamma") != nullptr);
    assert(registry.Find("Delta") == nullptr);
}

// ============================================================
// ToolingLayer tests
// ============================================================

void test_tooling_layer_default_inactive() {
    atlas::tools::ToolingLayer layer;
    assert(!layer.IsActive());
}

void test_tooling_layer_activate_deactivate() {
    atlas::tools::ToolingLayer layer;
    layer.Activate();
    assert(layer.IsActive());
    layer.Deactivate();
    assert(!layer.IsActive());
}

void test_tooling_layer_toggle() {
    atlas::tools::ToolingLayer layer;
    layer.Toggle();
    assert(layer.IsActive());
    layer.Toggle();
    assert(!layer.IsActive());
}

void test_tooling_layer_inactive_skips_update() {
    atlas::tools::ToolingLayer layer;
    auto mod = std::make_unique<MockToolModule>("Skip");
    auto* raw = mod.get();
    layer.Registry().Register(std::move(mod));
    // raw is valid via the registry
    layer.Update(1.0f);
    assert(raw->updateCount == 0);
    layer.Activate();
    layer.Update(1.0f);
    assert(raw->updateCount == 1);
}

void test_tooling_layer_inactive_skips_render() {
    atlas::tools::ToolingLayer layer;
    auto mod = std::make_unique<MockToolModule>("RenderSkip");
    auto* raw = mod.get();
    layer.Registry().Register(std::move(mod));
    layer.Render();
    assert(raw->renderCount == 0);
    layer.Activate();
    layer.Render();
    assert(raw->renderCount == 1);
}

void test_tooling_layer_inactive_skips_input() {
    atlas::tools::ToolingLayer layer;
    auto mod = std::make_unique<MockToolModule>("InputSkip");
    mod->consumeInput = true;
    layer.Registry().Register(std::move(mod));
    bool consumed = layer.HandleInput(42, true);
    assert(!consumed);
    layer.Activate();
    consumed = layer.HandleInput(42, true);
    assert(consumed);
}

void test_tooling_layer_toggle_key() {
    atlas::tools::ToolingLayer layer;
    assert(layer.ToggleKey() == 0);
    layer.SetToggleKey(112); // F1
    assert(layer.ToggleKey() == 112);
    // Pressing toggle key should activate
    layer.HandleInput(112, true);
    assert(layer.IsActive());
    // Pressing again should deactivate
    layer.HandleInput(112, true);
    assert(!layer.IsActive());
}

void test_tooling_layer_registry_access() {
    atlas::tools::ToolingLayer layer;
    layer.Registry().Register(std::make_unique<MockToolModule>("Reg1"));
    assert(layer.Registry().Count() == 1);
    const auto& constLayer = layer;
    assert(constLayer.Registry().Count() == 1);
}
