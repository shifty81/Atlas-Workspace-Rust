#include <catch2/catch_test_macros.hpp>
#include <set>
#include <string>

// Include the 10 primary tool module headers
#include "Tools/ConsoleToolModule.h"
#include "Tools/ProfilerToolModule.h"
#include "Tools/ECSInspectorToolModule.h"
#include "Tools/MaterialEditorToolModule.h"
#include "Tools/MeshViewerToolModule.h"
#include "Tools/PrefabEditorToolModule.h"
#include "Tools/AIDebuggerToolModule.h"
#include "Tools/WorldGraphToolModule.h"
#include "Tools/SceneGraphToolModule.h"
#include "Tools/PhysicsTunerToolModule.h"

// Include existing tool infrastructure
#include "Tools/IEditorToolModule.h"

// ── Tool module Name() tests ─────────────────────────────────────

TEST_CASE("ConsoleToolModule Name", "[Editor][Tools]") {
    atlas::editor::ConsoleToolModule tool;
    REQUIRE(tool.Name() == "Console");
}

TEST_CASE("ProfilerToolModule Name", "[Editor][Tools]") {
    atlas::editor::ProfilerToolModule tool;
    REQUIRE(tool.Name() == "Profiler");
}

TEST_CASE("ECSInspectorToolModule Name", "[Editor][Tools]") {
    atlas::editor::ECSInspectorToolModule tool;
    REQUIRE(tool.Name() == "ECS Inspector");
}

TEST_CASE("MaterialEditorToolModule Name", "[Editor][Tools]") {
    atlas::editor::MaterialEditorToolModule tool;
    REQUIRE(tool.Name() == "Material Editor");
}

TEST_CASE("MeshViewerToolModule Name", "[Editor][Tools]") {
    atlas::editor::MeshViewerToolModule tool;
    REQUIRE(tool.Name() == "Mesh Viewer");
}

TEST_CASE("PrefabEditorToolModule Name", "[Editor][Tools]") {
    atlas::editor::PrefabEditorToolModule tool;
    REQUIRE(tool.Name() == "Prefab Editor");
}

TEST_CASE("AIDebuggerToolModule Name", "[Editor][Tools]") {
    atlas::editor::AIDebuggerToolModule tool;
    REQUIRE(tool.Name() == "AI Debugger");
}

TEST_CASE("WorldGraphToolModule Name", "[Editor][Tools]") {
    atlas::editor::WorldGraphToolModule tool;
    REQUIRE(tool.Name() == "World Graph");
}

TEST_CASE("SceneGraphToolModule Name", "[Editor][Tools]") {
    atlas::editor::SceneGraphToolModule tool;
    REQUIRE(tool.Name() == "Scene Graph");
}

TEST_CASE("PhysicsTunerToolModule Name", "[Editor][Tools]") {
    atlas::editor::PhysicsTunerToolModule tool;
    REQUIRE(tool.Name() == "Physics Tuner");
}

// ── IEditorToolModule interface compliance ───────────────────────

TEST_CASE("Tool modules implement IEditorToolModule interface", "[Editor][Tools]") {
    // Verify each tool can be used through the IEditorToolModule interface
    atlas::editor::ConsoleToolModule console;
    atlas::editor::ProfilerToolModule profiler;
    atlas::editor::ECSInspectorToolModule ecsInspector;

    atlas::editor::IEditorToolModule* tools[] = {
        &console, &profiler, &ecsInspector
    };

    for (auto* tool : tools) {
        REQUIRE_FALSE(tool->Name().empty());

        // These should not throw
        tool->OnRegister();
        tool->RegisterPanels();
        tool->RegisterMenus();
        tool->RegisterModes();
        tool->Update(0.016f);
        tool->Render();
        REQUIRE_FALSE(tool->HandleInput(0, false));
        tool->OnUnregister();
    }
}

TEST_CASE("All 10 tool modules have unique names", "[Editor][Tools]") {
    atlas::editor::ConsoleToolModule console;
    atlas::editor::ProfilerToolModule profiler;
    atlas::editor::ECSInspectorToolModule ecsInspector;
    atlas::editor::MaterialEditorToolModule materialEditor;
    atlas::editor::MeshViewerToolModule meshViewer;
    atlas::editor::PrefabEditorToolModule prefabEditor;
    atlas::editor::AIDebuggerToolModule aiDebugger;
    atlas::editor::WorldGraphToolModule worldGraph;
    atlas::editor::SceneGraphToolModule sceneGraph;
    atlas::editor::PhysicsTunerToolModule physicsTuner;

    std::set<std::string> names;
    atlas::tools::IToolModule* tools[] = {
        &console, &profiler, &ecsInspector, &materialEditor,
        &meshViewer, &prefabEditor, &aiDebugger,
        &worldGraph, &sceneGraph, &physicsTuner
    };

    for (auto* tool : tools) {
        auto result = names.insert(tool->Name());
        INFO("Duplicate tool name: " << tool->Name());
        REQUIRE(result.second); // insertion succeeded = name was unique
    }

    REQUIRE(names.size() == 10);
}

TEST_CASE("All tool modules survive full lifecycle", "[Editor][Tools]") {
    atlas::editor::MaterialEditorToolModule materialEditor;
    atlas::editor::WorldGraphToolModule worldGraph;
    atlas::editor::SceneGraphToolModule sceneGraph;
    atlas::editor::PhysicsTunerToolModule physicsTuner;

    atlas::editor::IEditorToolModule* tools[] = {
        &materialEditor, &worldGraph, &sceneGraph, &physicsTuner
    };

    for (auto* tool : tools) {
        // Full lifecycle: register → panels/menus/modes → use → unregister
        tool->OnRegister();
        tool->RegisterPanels();
        tool->RegisterMenus();
        tool->RegisterModes();

        for (int i = 0; i < 3; ++i) {
            tool->Update(0.016f);
            tool->Render();
        }

        REQUIRE_FALSE(tool->HandleInput(42, true));
        REQUIRE_FALSE(tool->HandleInput(42, false));

        tool->OnUnregister();
    }
}
