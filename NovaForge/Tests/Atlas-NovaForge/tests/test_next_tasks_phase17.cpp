// Phase 17: Hot reload world migration, SimMirror, DesyncVisualizerPanel tests

#include "../engine/sim/ComponentMigration.h"
#include "../engine/sim/SimMirror.h"
#include "../editor/panels/DesyncVisualizerPanel.h"
#include <cassert>
#include <cstring>
#include <iostream>
#include <vector>

using namespace atlas::sim;
using namespace atlas::editor;

// ============================================================
// Component Migration Tests
// ============================================================

void test_remap_component_basic() {
    // Old schema: fields A(id=1) and B(id=2)
    ComponentSchema oldSchema;
    oldSchema.typeId = 100;
    oldSchema.version = 1;
    oldSchema.totalSize = 8;
    oldSchema.fields.push_back({1, 0, 4, FieldDesc::Type::Int32, "x"});
    oldSchema.fields.push_back({2, 4, 4, FieldDesc::Type::Int32, "y"});

    // New schema: same fields, same layout
    ComponentSchema newSchema = oldSchema;
    newSchema.version = 2;

    uint8_t oldData[8] = {};
    int32_t xVal = 42, yVal = 99;
    std::memcpy(oldData, &xVal, 4);
    std::memcpy(oldData + 4, &yVal, 4);

    uint8_t newData[8] = {};
    auto result = RemapComponent(oldSchema, newSchema, oldData, newData);

    assert(result.success);
    assert(result.fieldsRemapped == 2);
    assert(result.fieldsAdded == 0);
    assert(result.fieldsDropped == 0);

    int32_t outX = 0, outY = 0;
    std::memcpy(&outX, newData, 4);
    std::memcpy(&outY, newData + 4, 4);
    assert(outX == 42);
    assert(outY == 99);
    std::cout << "  [PASS] test_remap_component_basic" << std::endl;
}

void test_remap_component_field_added() {
    ComponentSchema oldSchema;
    oldSchema.typeId = 100;
    oldSchema.version = 1;
    oldSchema.totalSize = 4;
    oldSchema.fields.push_back({1, 0, 4, FieldDesc::Type::Int32, "x"});

    ComponentSchema newSchema;
    newSchema.typeId = 100;
    newSchema.version = 2;
    newSchema.totalSize = 8;
    newSchema.fields.push_back({1, 0, 4, FieldDesc::Type::Int32, "x"});
    newSchema.fields.push_back({3, 4, 4, FieldDesc::Type::Int32, "z"});

    int32_t xVal = 77;
    uint8_t oldData[4] = {};
    std::memcpy(oldData, &xVal, 4);

    uint8_t newData[8] = {};
    auto result = RemapComponent(oldSchema, newSchema, oldData, newData);

    assert(result.success);
    assert(result.fieldsRemapped == 1);
    assert(result.fieldsAdded == 1);

    int32_t outX = 0, outZ = 0;
    std::memcpy(&outX, newData, 4);
    std::memcpy(&outZ, newData + 4, 4);
    assert(outX == 77);
    assert(outZ == 0); // zero-initialized
    std::cout << "  [PASS] test_remap_component_field_added" << std::endl;
}

void test_remap_component_field_dropped() {
    ComponentSchema oldSchema;
    oldSchema.typeId = 100;
    oldSchema.version = 1;
    oldSchema.totalSize = 8;
    oldSchema.fields.push_back({1, 0, 4, FieldDesc::Type::Int32, "x"});
    oldSchema.fields.push_back({2, 4, 4, FieldDesc::Type::Int32, "y"});

    ComponentSchema newSchema;
    newSchema.typeId = 100;
    newSchema.version = 2;
    newSchema.totalSize = 4;
    newSchema.fields.push_back({1, 0, 4, FieldDesc::Type::Int32, "x"});

    int32_t xVal = 55, yVal = 66;
    uint8_t oldData[8] = {};
    std::memcpy(oldData, &xVal, 4);
    std::memcpy(oldData + 4, &yVal, 4);

    uint8_t newData[4] = {};
    auto result = RemapComponent(oldSchema, newSchema, oldData, newData);

    assert(result.success);
    assert(result.fieldsRemapped == 1);
    assert(result.fieldsDropped == 1);

    int32_t outX = 0;
    std::memcpy(&outX, newData, 4);
    assert(outX == 55);
    std::cout << "  [PASS] test_remap_component_field_dropped" << std::endl;
}

void test_remap_component_type_mismatch() {
    ComponentSchema a;
    a.typeId = 100;
    ComponentSchema b;
    b.typeId = 200;

    uint8_t data[4] = {};
    auto result = RemapComponent(a, b, data, data);
    assert(!result.success);
    assert(!result.error.empty());
    std::cout << "  [PASS] test_remap_component_type_mismatch" << std::endl;
}

void test_remap_component_null_data() {
    ComponentSchema schema;
    schema.typeId = 100;
    auto result = RemapComponent(schema, schema, nullptr, nullptr);
    assert(!result.success);
    std::cout << "  [PASS] test_remap_component_null_data" << std::endl;
}

void test_schemas_compatible() {
    ComponentSchema a;
    a.typeId = 100;
    ComponentSchema b;
    b.typeId = 100;
    assert(SchemasCompatible(a, b));

    ComponentSchema c;
    c.typeId = 200;
    assert(!SchemasCompatible(a, c));
    std::cout << "  [PASS] test_schemas_compatible" << std::endl;
}

void test_migration_manager_register_and_get() {
    ComponentMigrationManager mgr;
    ComponentSchema s;
    s.typeId = 10;
    s.version = 1;
    s.totalSize = 4;
    mgr.RegisterSchema(s);

    assert(mgr.SchemaCount() == 1);
    assert(mgr.GetSchema(10, 1) != nullptr);
    assert(mgr.GetLatestSchema(10) != nullptr);
    assert(mgr.GetLatestSchema(10)->version == 1);
    assert(mgr.GetSchema(10, 2) == nullptr);
    assert(mgr.GetLatestSchema(999) == nullptr);
    std::cout << "  [PASS] test_migration_manager_register_and_get" << std::endl;
}

void test_migration_manager_needs_migration() {
    ComponentMigrationManager mgr;
    ComponentSchema s1;
    s1.typeId = 10;
    s1.version = 1;
    s1.totalSize = 4;
    mgr.RegisterSchema(s1);

    ComponentSchema s2 = s1;
    s2.version = 2;
    mgr.RegisterSchema(s2);

    assert(mgr.NeedsMigration(10, 1));
    assert(!mgr.NeedsMigration(10, 2));
    assert(!mgr.NeedsMigration(999, 1)); // unknown type
    std::cout << "  [PASS] test_migration_manager_needs_migration" << std::endl;
}

void test_migration_manager_migrate_to_latest() {
    ComponentMigrationManager mgr;

    ComponentSchema s1;
    s1.typeId = 10;
    s1.version = 1;
    s1.totalSize = 4;
    s1.fields.push_back({1, 0, 4, FieldDesc::Type::Int32, "x"});
    mgr.RegisterSchema(s1);

    ComponentSchema s2;
    s2.typeId = 10;
    s2.version = 2;
    s2.totalSize = 8;
    s2.fields.push_back({1, 0, 4, FieldDesc::Type::Int32, "x"});
    s2.fields.push_back({2, 4, 4, FieldDesc::Type::Float, "speed"});
    mgr.RegisterSchema(s2);

    int32_t xVal = 123;
    uint8_t oldData[4] = {};
    std::memcpy(oldData, &xVal, 4);

    uint8_t newData[8] = {};
    auto result = mgr.MigrateToLatest(10, 1, oldData, newData);
    assert(result.success);
    assert(result.fieldsRemapped == 1);
    assert(result.fieldsAdded == 1);

    int32_t outX = 0;
    std::memcpy(&outX, newData, 4);
    assert(outX == 123);
    std::cout << "  [PASS] test_migration_manager_migrate_to_latest" << std::endl;
}

void test_migration_manager_hot_reload_safe() {
    ComponentMigrationManager mgr;

    ComponentSchema s1;
    s1.typeId = 10;
    s1.version = 1;
    s1.totalSize = 4;
    s1.fields.push_back({1, 0, 4, FieldDesc::Type::Int32, "x"});
    mgr.RegisterSchema(s1);

    // Adding a field is safe
    ComponentSchema s2;
    s2.typeId = 10;
    s2.version = 2;
    s2.totalSize = 8;
    s2.fields.push_back({1, 0, 4, FieldDesc::Type::Int32, "x"});
    s2.fields.push_back({2, 4, 4, FieldDesc::Type::Float, "speed"});
    mgr.RegisterSchema(s2);

    assert(mgr.IsHotReloadSafe(10, 1));

    // Changing a field type is unsafe
    ComponentSchema s3;
    s3.typeId = 20;
    s3.version = 1;
    s3.totalSize = 4;
    s3.fields.push_back({1, 0, 4, FieldDesc::Type::Int32, "x"});
    mgr.RegisterSchema(s3);

    ComponentSchema s4;
    s4.typeId = 20;
    s4.version = 2;
    s4.totalSize = 4;
    s4.fields.push_back({1, 0, 4, FieldDesc::Type::Float, "x"}); // type changed
    mgr.RegisterSchema(s4);

    assert(!mgr.IsHotReloadSafe(20, 1));
    std::cout << "  [PASS] test_migration_manager_hot_reload_safe" << std::endl;
}

// ============================================================
// SimMirror Tests
// ============================================================

class TestSimulation : public ISimulation {
public:
    explicit TestSimulation(uint64_t hashOffset = 0) : m_hashOffset(hashOffset) {}

    void Step(const std::vector<uint8_t>& inputFrame) override {
        m_tick++;
        // Deterministic hash based on tick and input size
        m_hash = m_tick * 1000 + inputFrame.size() + m_hashOffset;
    }

    uint64_t WorldHash() const override { return m_hash; }
    uint64_t CurrentTick() const override { return m_tick; }

    void SetHashOffset(uint64_t offset) { m_hashOffset = offset; }

private:
    uint64_t m_tick = 0;
    uint64_t m_hash = 0;
    uint64_t m_hashOffset = 0;
};

void test_sim_mirror_no_desync() {
    TestSimulation server;
    TestSimulation client;

    SimMirrorController mirror;
    mirror.SetServer(&server);
    mirror.SetClient(&client);

    std::vector<uint8_t> input = {1, 2, 3};
    assert(mirror.Step(input));
    assert(mirror.Step(input));
    assert(mirror.Step(input));
    assert(!mirror.HasDesync());
    assert(mirror.FrameCount() == 3);
    std::cout << "  [PASS] test_sim_mirror_no_desync" << std::endl;
}

void test_sim_mirror_detects_desync() {
    TestSimulation server(0);
    TestSimulation client(1); // different hash offset

    SimMirrorController mirror;
    mirror.SetServer(&server);
    mirror.SetClient(&client);

    std::vector<uint8_t> input = {1};
    assert(!mirror.Step(input)); // should detect mismatch
    assert(mirror.HasDesync());
    assert(mirror.Desyncs().size() == 1);
    assert(mirror.FirstDesync() != nullptr);
    assert(mirror.FirstDesync()->serverHash != mirror.FirstDesync()->clientHash);
    std::cout << "  [PASS] test_sim_mirror_detects_desync" << std::endl;
}

void test_sim_mirror_run_frames() {
    TestSimulation server;
    TestSimulation client;

    SimMirrorController mirror;
    mirror.SetServer(&server);
    mirror.SetClient(&client);

    std::vector<std::vector<uint8_t>> inputs = {{1}, {2}, {3}, {4}, {5}};
    uint64_t desyncTick = mirror.RunFrames(inputs);
    assert(desyncTick == 0); // no desync
    assert(mirror.FrameCount() == 5);
    std::cout << "  [PASS] test_sim_mirror_run_frames" << std::endl;
}

void test_sim_mirror_run_frames_with_desync() {
    TestSimulation server(0);
    TestSimulation client(1);

    SimMirrorController mirror;
    mirror.SetServer(&server);
    mirror.SetClient(&client);

    std::vector<std::vector<uint8_t>> inputs = {{1}, {2}, {3}};
    uint64_t desyncTick = mirror.RunFrames(inputs);
    assert(desyncTick > 0);
    std::cout << "  [PASS] test_sim_mirror_run_frames_with_desync" << std::endl;
}

void test_sim_mirror_reset() {
    TestSimulation server;
    TestSimulation client;

    SimMirrorController mirror;
    mirror.SetServer(&server);
    mirror.SetClient(&client);
    mirror.Step({1});

    mirror.Reset();
    assert(!mirror.HasDesync());
    assert(mirror.FrameCount() == 0);
    std::cout << "  [PASS] test_sim_mirror_reset" << std::endl;
}

void test_sim_mirror_callback() {
    TestSimulation server(0);
    TestSimulation client(1);

    SimMirrorController mirror;
    mirror.SetServer(&server);
    mirror.SetClient(&client);

    bool callbackFired = false;
    mirror.SetDesyncCallback([&](const MirrorDesyncEvent& evt) {
        callbackFired = true;
        assert(evt.serverHash != evt.clientHash);
    });

    mirror.Step({1});
    assert(callbackFired);
    std::cout << "  [PASS] test_sim_mirror_callback" << std::endl;
}

void test_sim_mirror_disabled() {
    TestSimulation server(0);
    TestSimulation client(1);

    SimMirrorController mirror;
    mirror.SetServer(&server);
    mirror.SetClient(&client);
    mirror.SetEnabled(false);
    assert(!mirror.IsEnabled());

    // With mirror disabled, Step always returns true
    assert(mirror.Step({1}));
    assert(!mirror.HasDesync());
    std::cout << "  [PASS] test_sim_mirror_disabled" << std::endl;
}

void test_sim_mirror_null_sims() {
    SimMirrorController mirror;
    // No sims set, should not crash
    assert(mirror.Step({1}));
    assert(!mirror.HasDesync());
    std::cout << "  [PASS] test_sim_mirror_null_sims" << std::endl;
}

// ============================================================
// DesyncVisualizerPanel Tests
// ============================================================

void test_desync_panel_empty() {
    DesyncVisualizerPanel panel;
    panel.Draw();

    assert(panel.GetDrawList().CommandCount() > 0);
    bool foundNoDesync = false;
    for (const auto& cmd : panel.GetDrawList().Commands()) {
        if (cmd.kind == atlas::ui::UIDrawCmd::Kind::Text &&
            cmd.text.find("No desyncs") != std::string::npos) {
            foundNoDesync = true;
        }
    }
    assert(foundNoDesync);
    std::cout << "  [PASS] test_desync_panel_empty" << std::endl;
}

void test_desync_panel_add_event() {
    DesyncVisualizerPanel panel;

    DesyncDisplayEvent evt;
    evt.tick = 42;
    evt.serverHash = 0xAAAA;
    evt.clientHash = 0xBBBB;
    panel.AddEvent(evt);

    assert(panel.Events().size() == 1);
    assert(panel.Events()[0].tick == 42);
    std::cout << "  [PASS] test_desync_panel_add_event" << std::endl;
}

void test_desync_panel_draw_with_events() {
    DesyncVisualizerPanel panel;

    DesyncDisplayEvent evt;
    evt.tick = 100;
    evt.serverHash = 0x1234;
    evt.clientHash = 0x5678;
    panel.AddEvent(evt);
    panel.Draw();

    bool foundTick = false;
    bool foundTitle = false;
    for (const auto& cmd : panel.GetDrawList().Commands()) {
        if (cmd.kind == atlas::ui::UIDrawCmd::Kind::Text) {
            if (cmd.text == "Desync Visualizer") foundTitle = true;
            if (cmd.text == "100") foundTick = true;
        }
    }
    assert(foundTitle);
    assert(foundTick);
    std::cout << "  [PASS] test_desync_panel_draw_with_events" << std::endl;
}

void test_desync_panel_select_event() {
    DesyncVisualizerPanel panel;

    DesyncDisplayEvent e1;
    e1.tick = 10;
    DesyncDisplayEvent e2;
    e2.tick = 20;
    panel.AddEvent(e1);
    panel.AddEvent(e2);

    assert(panel.SelectedIndex() == 0);
    panel.SelectEvent(1);
    assert(panel.SelectedIndex() == 1);
    std::cout << "  [PASS] test_desync_panel_select_event" << std::endl;
}

void test_desync_panel_clear_events() {
    DesyncVisualizerPanel panel;
    DesyncDisplayEvent evt;
    evt.tick = 1;
    panel.AddEvent(evt);
    assert(!panel.Events().empty());
    panel.ClearEvents();
    assert(panel.Events().empty());
    std::cout << "  [PASS] test_desync_panel_clear_events" << std::endl;
}

void test_desync_panel_with_field_details() {
    DesyncVisualizerPanel panel;

    DesyncDisplayEvent evt;
    evt.tick = 50;
    evt.serverHash = 0xAA;
    evt.clientHash = 0xBB;

    DesyncFieldDetail detail;
    detail.entityName = "Ship";
    detail.componentName = "Transform";
    detail.fieldName = "position.x";
    detail.serverValue = "124.0";
    detail.clientValue = "123.9375";
    evt.details.push_back(detail);

    panel.AddEvent(evt);
    panel.SelectEvent(0);
    panel.Draw();

    bool foundField = false;
    for (const auto& cmd : panel.GetDrawList().Commands()) {
        if (cmd.kind == atlas::ui::UIDrawCmd::Kind::Text &&
            cmd.text.find("Ship.Transform.position.x") != std::string::npos) {
            foundField = true;
        }
    }
    assert(foundField);
    std::cout << "  [PASS] test_desync_panel_with_field_details" << std::endl;
}

void test_desync_panel_mirror_controller_sync() {
    TestSimulation server(0);
    TestSimulation client(1);

    SimMirrorController mirror;
    mirror.SetServer(&server);
    mirror.SetClient(&client);
    mirror.Step({1}); // causes desync

    DesyncVisualizerPanel panel;
    panel.SetMirrorController(&mirror);
    panel.Draw(); // syncs from controller

    assert(!panel.Events().empty());
    assert(panel.Events()[0].tick == 1);
    std::cout << "  [PASS] test_desync_panel_mirror_controller_sync" << std::endl;
}

void test_desync_panel_name() {
    DesyncVisualizerPanel panel;
    assert(std::string(panel.Name()) == "Desync Visualizer");
    std::cout << "  [PASS] test_desync_panel_name" << std::endl;
}

// ---------------------------------------------------------------
// Runner
// ---------------------------------------------------------------

void register_next_tasks_phase17() {
    std::cout << "\n--- Phase 17: Component Migration ---" << std::endl;
    test_remap_component_basic();
    test_remap_component_field_added();
    test_remap_component_field_dropped();
    test_remap_component_type_mismatch();
    test_remap_component_null_data();
    test_schemas_compatible();
    test_migration_manager_register_and_get();
    test_migration_manager_needs_migration();
    test_migration_manager_migrate_to_latest();
    test_migration_manager_hot_reload_safe();

    std::cout << "\n--- Phase 17: SimMirror ---" << std::endl;
    test_sim_mirror_no_desync();
    test_sim_mirror_detects_desync();
    test_sim_mirror_run_frames();
    test_sim_mirror_run_frames_with_desync();
    test_sim_mirror_reset();
    test_sim_mirror_callback();
    test_sim_mirror_disabled();
    test_sim_mirror_null_sims();

    std::cout << "\n--- Phase 17: DesyncVisualizerPanel ---" << std::endl;
    test_desync_panel_empty();
    test_desync_panel_add_event();
    test_desync_panel_draw_with_events();
    test_desync_panel_select_event();
    test_desync_panel_clear_events();
    test_desync_panel_with_field_details();
    test_desync_panel_mirror_controller_sync();
    test_desync_panel_name();
}
