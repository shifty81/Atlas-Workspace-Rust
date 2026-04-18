// Tests/Workspace/test_workspace_project_state.cpp
//
// Tests for NF::WorkspaceProjectState — workspace-level project session owner.
//
// Validates Phase G normalization: the new spine between WorkspaceShell and
// per-document authoring units.
//
// Coverage:
//   — Default state (no project)
//   — Project load / unload lifecycle
//   — Document registry: open, close, has, find, count
//   — Active document context: set, clear, fallback on close
//   — Dirty tracking: per-document, aggregate, dirtyDocumentCount
//   — saveAll() / revertAll() coordination stubs
//   — Panel binding context
//   — Change listeners: fired on load, unload, open, close, dirty, context
//   — WorkspaceShell::projectState() accessor wiring
//   — WorkspaceShell::loadProject() notifies projectState
//   — WorkspaceShell::unloadProject() clears projectState

#include <catch2/catch_test_macros.hpp>
#include "NF/Workspace/WorkspaceProjectState.h"
#include "NF/Workspace/WorkspaceShell.h"

using namespace NF;

// ── Minimal stub adapter for tests ────────────────────────────────────────

namespace {

class StubAdapter final : public IGameProjectAdapter {
public:
    explicit StubAdapter(std::string id = "test.project",
                          std::string name = "Test Project")
        : m_id(std::move(id)), m_name(std::move(name)) {}

    [[nodiscard]] std::string projectId()          const override { return m_id; }
    [[nodiscard]] std::string projectDisplayName() const override { return m_name; }

    bool initialize() override { ++m_initCount; return m_initOk; }
    void shutdown()   override { ++m_shutCount; }

    [[nodiscard]] std::vector<GameplaySystemPanelDescriptor> panelDescriptors() const override { return {}; }

    int  m_initCount = 0;
    int  m_shutCount = 0;
    bool m_initOk    = true;

private:
    std::string m_id;
    std::string m_name;
};

// Helpers

OpenDocumentEntry makeEntry(const std::string& id,
                             const std::string& title,
                             DocumentKind kind = DocumentKind::Scene,
                             bool dirty = false) {
    OpenDocumentEntry e;
    e.documentId   = id;
    e.displayTitle = title;
    e.kind         = kind;
    e.isDirty      = dirty;
    return e;
}

} // namespace

// ═══════════════════════════════════════════════════════════════════════════
//  WorkspaceProjectState — standalone (no shell required)
// ═══════════════════════════════════════════════════════════════════════════

// ── Default state ──────────────────────────────────────────────────────────

TEST_CASE("WorkspaceProjectState default state has no project", "[ProjectState]") {
    WorkspaceProjectState state;
    REQUIRE_FALSE(state.hasProject());
    REQUIRE(state.adapter()       == nullptr);
    REQUIRE(state.projectId()     == "");
    REQUIRE(state.projectDisplayName() == "");
}

TEST_CASE("WorkspaceProjectState default state has no open documents", "[ProjectState]") {
    WorkspaceProjectState state;
    REQUIRE(state.openDocumentCount()  == 0);
    REQUIRE(state.allOpenDocuments().empty());
    REQUIRE(state.activeDocumentId()   == "");
    REQUIRE(state.activeDocument()     == nullptr);
}

TEST_CASE("WorkspaceProjectState default state has no unsaved changes", "[ProjectState]") {
    WorkspaceProjectState state;
    REQUIRE_FALSE(state.hasUnsavedChanges());
    REQUIRE(state.dirtyDocumentCount() == 0);
}

TEST_CASE("WorkspaceProjectState default state activePanelContext is empty", "[ProjectState]") {
    WorkspaceProjectState state;
    REQUIRE(state.activePanelContext() == "");
}

// ── Project load / unload ─────────────────────────────────────────────────

TEST_CASE("WorkspaceProjectState onProjectLoaded sets adapter", "[ProjectState][lifecycle]") {
    WorkspaceProjectState state;
    StubAdapter adapter;
    state.onProjectLoaded(&adapter, {});
    REQUIRE(state.hasProject());
    REQUIRE(state.adapter() == &adapter);
    REQUIRE(state.projectId()          == "test.project");
    REQUIRE(state.projectDisplayName() == "Test Project");
}

TEST_CASE("WorkspaceProjectState onProjectLoaded clears previous documents", "[ProjectState][lifecycle]") {
    WorkspaceProjectState state;
    StubAdapter adapter;
    state.onProjectLoaded(&adapter, {});
    state.openDocument(makeEntry("doc1", "Scene 1"));

    StubAdapter adapter2("test.project2", "Project 2");
    state.onProjectLoaded(&adapter2, {});
    REQUIRE(state.openDocumentCount() == 0);
    REQUIRE(state.activeDocumentId()  == "");
}

TEST_CASE("WorkspaceProjectState onProjectUnloaded clears everything", "[ProjectState][lifecycle]") {
    WorkspaceProjectState state;
    StubAdapter adapter;
    state.onProjectLoaded(&adapter, {});
    state.openDocument(makeEntry("doc1", "Scene 1"));
    state.setActivePanelContext("scene_editor");

    state.onProjectUnloaded();
    REQUIRE_FALSE(state.hasProject());
    REQUIRE(state.adapter()            == nullptr);
    REQUIRE(state.openDocumentCount()  == 0);
    REQUIRE(state.activeDocumentId()   == "");
    REQUIRE(state.activePanelContext() == "");
}

// ── Document registry ─────────────────────────────────────────────────────

TEST_CASE("WorkspaceProjectState openDocument registers entry", "[ProjectState][docs]") {
    WorkspaceProjectState state;
    StubAdapter adapter;
    state.onProjectLoaded(&adapter, {});

    bool ok = state.openDocument(makeEntry("scene1", "Main Scene", DocumentKind::Scene));
    REQUIRE(ok);
    REQUIRE(state.openDocumentCount() == 1);
    REQUIRE(state.hasDocument("scene1"));
}

TEST_CASE("WorkspaceProjectState openDocument sets active if first doc", "[ProjectState][docs]") {
    WorkspaceProjectState state;
    StubAdapter adapter;
    state.onProjectLoaded(&adapter, {});

    state.openDocument(makeEntry("doc1", "Doc 1"));
    REQUIRE(state.activeDocumentId() == "doc1");
    REQUIRE(state.activeDocument()   != nullptr);
    REQUIRE(state.activeDocument()->displayTitle == "Doc 1");
}

TEST_CASE("WorkspaceProjectState openDocument does not override existing active", "[ProjectState][docs]") {
    WorkspaceProjectState state;
    StubAdapter adapter;
    state.onProjectLoaded(&adapter, {});

    state.openDocument(makeEntry("doc1", "Doc 1"));
    state.openDocument(makeEntry("doc2", "Doc 2"));
    REQUIRE(state.activeDocumentId() == "doc1");
}

TEST_CASE("WorkspaceProjectState openDocument rejects duplicate id", "[ProjectState][docs]") {
    WorkspaceProjectState state;
    StubAdapter adapter;
    state.onProjectLoaded(&adapter, {});

    state.openDocument(makeEntry("doc1", "Scene 1"));
    bool second = state.openDocument(makeEntry("doc1", "Scene Duplicate"));
    REQUIRE_FALSE(second);
    REQUIRE(state.openDocumentCount() == 1);
}

TEST_CASE("WorkspaceProjectState openDocument without project returns false", "[ProjectState][docs]") {
    WorkspaceProjectState state;
    bool ok = state.openDocument(makeEntry("doc1", "Scene 1"));
    REQUIRE_FALSE(ok);
}

TEST_CASE("WorkspaceProjectState closeDocument removes entry", "[ProjectState][docs]") {
    WorkspaceProjectState state;
    StubAdapter adapter;
    state.onProjectLoaded(&adapter, {});

    state.openDocument(makeEntry("doc1", "Doc 1"));
    state.openDocument(makeEntry("doc2", "Doc 2"));
    REQUIRE(state.closeDocument("doc1"));
    REQUIRE_FALSE(state.hasDocument("doc1"));
    REQUIRE(state.openDocumentCount() == 1);
}

TEST_CASE("WorkspaceProjectState closeDocument updates active when active is closed", "[ProjectState][docs]") {
    WorkspaceProjectState state;
    StubAdapter adapter;
    state.onProjectLoaded(&adapter, {});

    state.openDocument(makeEntry("doc1", "Doc 1"));
    state.openDocument(makeEntry("doc2", "Doc 2"));
    REQUIRE(state.activeDocumentId() == "doc1");

    state.closeDocument("doc1");
    // Active must have moved to another open doc (doc2)
    REQUIRE_FALSE(state.activeDocumentId().empty());
    REQUIRE(state.hasDocument(state.activeDocumentId()));
}

TEST_CASE("WorkspaceProjectState closeDocument clears active when last doc closed", "[ProjectState][docs]") {
    WorkspaceProjectState state;
    StubAdapter adapter;
    state.onProjectLoaded(&adapter, {});

    state.openDocument(makeEntry("doc1", "Only Doc"));
    state.closeDocument("doc1");
    REQUIRE(state.activeDocumentId() == "");
    REQUIRE(state.activeDocument()   == nullptr);
}

TEST_CASE("WorkspaceProjectState closeDocument unknown id returns false", "[ProjectState][docs]") {
    WorkspaceProjectState state;
    StubAdapter adapter;
    state.onProjectLoaded(&adapter, {});
    REQUIRE_FALSE(state.closeDocument("nonexistent"));
}

TEST_CASE("WorkspaceProjectState allOpenDocuments returns all entries", "[ProjectState][docs]") {
    WorkspaceProjectState state;
    StubAdapter adapter;
    state.onProjectLoaded(&adapter, {});

    state.openDocument(makeEntry("s1", "Scene 1"));
    state.openDocument(makeEntry("a1", "Asset 1"));
    state.openDocument(makeEntry("m1", "Mat 1"));

    auto all = state.allOpenDocuments();
    REQUIRE(all.size() == 3);
}

TEST_CASE("WorkspaceProjectState findDocument returns entry", "[ProjectState][docs]") {
    WorkspaceProjectState state;
    StubAdapter adapter;
    state.onProjectLoaded(&adapter, {});

    state.openDocument(makeEntry("mat1", "Metal Mat", DocumentKind::Material));
    const auto* e = state.findDocument("mat1");
    REQUIRE(e != nullptr);
    REQUIRE(e->displayTitle == "Metal Mat");
    REQUIRE(e->kind         == DocumentKind::Material);
}

TEST_CASE("WorkspaceProjectState findDocument unknown returns nullptr", "[ProjectState][docs]") {
    WorkspaceProjectState state;
    StubAdapter adapter;
    state.onProjectLoaded(&adapter, {});
    REQUIRE(state.findDocument("missing") == nullptr);
}

// ── Active document context ───────────────────────────────────────────────

TEST_CASE("WorkspaceProjectState setActiveDocument switches active", "[ProjectState][active]") {
    WorkspaceProjectState state;
    StubAdapter adapter;
    state.onProjectLoaded(&adapter, {});

    state.openDocument(makeEntry("d1", "Doc 1"));
    state.openDocument(makeEntry("d2", "Doc 2"));

    REQUIRE(state.setActiveDocument("d2"));
    REQUIRE(state.activeDocumentId() == "d2");
}

TEST_CASE("WorkspaceProjectState setActiveDocument on non-open returns false", "[ProjectState][active]") {
    WorkspaceProjectState state;
    StubAdapter adapter;
    state.onProjectLoaded(&adapter, {});

    state.openDocument(makeEntry("d1", "Doc 1"));
    REQUIRE_FALSE(state.setActiveDocument("d999"));
    REQUIRE(state.activeDocumentId() == "d1");
}

TEST_CASE("WorkspaceProjectState setActiveDocument same id is no-op but ok", "[ProjectState][active]") {
    WorkspaceProjectState state;
    StubAdapter adapter;
    state.onProjectLoaded(&adapter, {});

    state.openDocument(makeEntry("d1", "Doc 1"));
    REQUIRE(state.setActiveDocument("d1")); // already active
    REQUIRE(state.activeDocumentId() == "d1");
}

// ── Dirty tracking ────────────────────────────────────────────────────────

TEST_CASE("WorkspaceProjectState openDocument with dirty=true makes hasUnsavedChanges true",
          "[ProjectState][dirty]") {
    WorkspaceProjectState state;
    StubAdapter adapter;
    state.onProjectLoaded(&adapter, {});

    state.openDocument(makeEntry("d1", "Doc 1", DocumentKind::Scene, /*dirty=*/true));
    REQUIRE(state.hasUnsavedChanges());
    REQUIRE(state.dirtyDocumentCount() == 1);
}

TEST_CASE("WorkspaceProjectState clean docs: hasUnsavedChanges false", "[ProjectState][dirty]") {
    WorkspaceProjectState state;
    StubAdapter adapter;
    state.onProjectLoaded(&adapter, {});

    state.openDocument(makeEntry("d1", "Doc 1", DocumentKind::Scene, false));
    REQUIRE_FALSE(state.hasUnsavedChanges());
    REQUIRE(state.dirtyDocumentCount() == 0);
}

TEST_CASE("WorkspaceProjectState notifyDocumentDirtyChanged marks dirty", "[ProjectState][dirty]") {
    WorkspaceProjectState state;
    StubAdapter adapter;
    state.onProjectLoaded(&adapter, {});

    state.openDocument(makeEntry("d1", "Doc 1"));
    REQUIRE_FALSE(state.hasUnsavedChanges());

    REQUIRE(state.notifyDocumentDirtyChanged("d1", true));
    REQUIRE(state.hasUnsavedChanges());
    REQUIRE(state.dirtyDocumentCount() == 1);
}

TEST_CASE("WorkspaceProjectState notifyDocumentDirtyChanged clears dirty", "[ProjectState][dirty]") {
    WorkspaceProjectState state;
    StubAdapter adapter;
    state.onProjectLoaded(&adapter, {});

    state.openDocument(makeEntry("d1", "Doc 1", DocumentKind::Scene, true));
    REQUIRE(state.hasUnsavedChanges());

    REQUIRE(state.notifyDocumentDirtyChanged("d1", false));
    REQUIRE_FALSE(state.hasUnsavedChanges());
}

TEST_CASE("WorkspaceProjectState notifyDocumentDirtyChanged on unknown doc returns false",
          "[ProjectState][dirty]") {
    WorkspaceProjectState state;
    StubAdapter adapter;
    state.onProjectLoaded(&adapter, {});
    REQUIRE_FALSE(state.notifyDocumentDirtyChanged("ghost", true));
}

TEST_CASE("WorkspaceProjectState dirtyDocumentCount counts correctly", "[ProjectState][dirty]") {
    WorkspaceProjectState state;
    StubAdapter adapter;
    state.onProjectLoaded(&adapter, {});

    state.openDocument(makeEntry("d1", "Doc 1", DocumentKind::Scene, true));
    state.openDocument(makeEntry("d2", "Doc 2", DocumentKind::Material, false));
    state.openDocument(makeEntry("d3", "Doc 3", DocumentKind::Animation, true));

    REQUIRE(state.dirtyDocumentCount() == 2);
}

TEST_CASE("OpenDocumentEntry dirtyTitle has asterisk when dirty", "[ProjectState][dirty]") {
    OpenDocumentEntry e = makeEntry("x", "My Scene", DocumentKind::Scene, true);
    REQUIRE(e.dirtyTitle() == "* My Scene");
}

TEST_CASE("OpenDocumentEntry dirtyTitle no asterisk when clean", "[ProjectState][dirty]") {
    OpenDocumentEntry e = makeEntry("x", "My Scene", DocumentKind::Scene, false);
    REQUIRE(e.dirtyTitle() == "My Scene");
}

// ── saveAll / revertAll ───────────────────────────────────────────────────

TEST_CASE("WorkspaceProjectState saveAll with no project returns NoProjectLoaded",
          "[ProjectState][save]") {
    WorkspaceProjectState state;
    auto result = state.saveAll();
    REQUIRE(result.status == ProjectSaveStatus::NoProjectLoaded);
    REQUIRE_FALSE(result.ok());
}

TEST_CASE("WorkspaceProjectState saveAll with no dirty docs returns NothingDirty",
          "[ProjectState][save]") {
    WorkspaceProjectState state;
    StubAdapter adapter;
    state.onProjectLoaded(&adapter, {});
    state.openDocument(makeEntry("d1", "Doc 1", DocumentKind::Scene, false));

    auto result = state.saveAll();
    REQUIRE(result.status == ProjectSaveStatus::NothingDirty);
    REQUIRE(result.ok());
}

TEST_CASE("WorkspaceProjectState saveAll saves dirty docs and clears dirty state",
          "[ProjectState][save]") {
    WorkspaceProjectState state;
    StubAdapter adapter;
    state.onProjectLoaded(&adapter, {});

    // Documents with save delegates can be saved even without file paths.
    auto stubSave = [](const std::string&, const std::string&) { return true; };
    state.openDocument(makeEntry("d1", "Doc 1", DocumentKind::Scene, true), stubSave);
    state.openDocument(makeEntry("d2", "Doc 2", DocumentKind::Material, true), stubSave);
    state.openDocument(makeEntry("d3", "Doc 3", DocumentKind::Animation, false));

    auto result = state.saveAll();
    REQUIRE(result.status     == ProjectSaveStatus::Ok);
    REQUIRE(result.savedCount == 2);
    REQUIRE(result.ok());
    REQUIRE_FALSE(state.hasUnsavedChanges());
}

TEST_CASE("WorkspaceProjectState revertAll clears all dirty state", "[ProjectState][save]") {
    WorkspaceProjectState state;
    StubAdapter adapter;
    state.onProjectLoaded(&adapter, {});
    state.openDocument(makeEntry("d1", "Doc 1", DocumentKind::Scene, true));
    state.openDocument(makeEntry("d2", "Doc 2", DocumentKind::Material, true));

    REQUIRE(state.hasUnsavedChanges());
    state.revertAll();
    REQUIRE_FALSE(state.hasUnsavedChanges());
}

// ── Panel binding context ─────────────────────────────────────────────────

TEST_CASE("WorkspaceProjectState setActivePanelContext updates context", "[ProjectState][context]") {
    WorkspaceProjectState state;
    state.setActivePanelContext("scene_editor");
    REQUIRE(state.activePanelContext() == "scene_editor");
}

TEST_CASE("WorkspaceProjectState setActivePanelContext same value is no-op", "[ProjectState][context]") {
    WorkspaceProjectState state;
    state.setActivePanelContext("material_editor");
    state.setActivePanelContext("material_editor"); // same
    REQUIRE(state.activePanelContext() == "material_editor");
}

TEST_CASE("WorkspaceProjectState setActivePanelContext survives project unload", "[ProjectState][context]") {
    WorkspaceProjectState state;
    StubAdapter adapter;
    state.onProjectLoaded(&adapter, {});
    state.setActivePanelContext("scene_editor");

    state.onProjectUnloaded();
    // After unload, panel context cleared to prevent stale binding
    REQUIRE(state.activePanelContext() == "");
}

// ── Change listeners ──────────────────────────────────────────────────────

TEST_CASE("WorkspaceProjectState change listener fires on project load", "[ProjectState][listeners]") {
    WorkspaceProjectState state;
    int count = 0;
    state.addChangeListener([&] { ++count; });

    StubAdapter adapter;
    state.onProjectLoaded(&adapter, {});
    REQUIRE(count == 1);
}

TEST_CASE("WorkspaceProjectState change listener fires on project unload", "[ProjectState][listeners]") {
    WorkspaceProjectState state;
    StubAdapter adapter;
    state.onProjectLoaded(&adapter, {});

    int count = 0;
    state.addChangeListener([&] { ++count; });
    state.onProjectUnloaded();
    REQUIRE(count == 1);
}

TEST_CASE("WorkspaceProjectState change listener fires on openDocument", "[ProjectState][listeners]") {
    WorkspaceProjectState state;
    StubAdapter adapter;
    state.onProjectLoaded(&adapter, {});

    int count = 0;
    state.addChangeListener([&] { ++count; });
    state.openDocument(makeEntry("d1", "Doc 1"));
    REQUIRE(count == 1);
}

TEST_CASE("WorkspaceProjectState change listener fires on closeDocument", "[ProjectState][listeners]") {
    WorkspaceProjectState state;
    StubAdapter adapter;
    state.onProjectLoaded(&adapter, {});
    state.openDocument(makeEntry("d1", "Doc 1"));

    int count = 0;
    state.addChangeListener([&] { ++count; });
    state.closeDocument("d1");
    REQUIRE(count == 1);
}

TEST_CASE("WorkspaceProjectState change listener fires on dirty change", "[ProjectState][listeners]") {
    WorkspaceProjectState state;
    StubAdapter adapter;
    state.onProjectLoaded(&adapter, {});
    state.openDocument(makeEntry("d1", "Doc 1"));

    int count = 0;
    state.addChangeListener([&] { ++count; });
    state.notifyDocumentDirtyChanged("d1", true);
    REQUIRE(count == 1);
}

TEST_CASE("WorkspaceProjectState change listener fires on setActiveDocument", "[ProjectState][listeners]") {
    WorkspaceProjectState state;
    StubAdapter adapter;
    state.onProjectLoaded(&adapter, {});
    state.openDocument(makeEntry("d1", "Doc 1"));
    state.openDocument(makeEntry("d2", "Doc 2"));

    int count = 0;
    state.addChangeListener([&] { ++count; });
    state.setActiveDocument("d2");
    REQUIRE(count == 1);
}

TEST_CASE("WorkspaceProjectState change listener fires on panel context change",
          "[ProjectState][listeners]") {
    WorkspaceProjectState state;
    int count = 0;
    state.addChangeListener([&] { ++count; });
    state.setActivePanelContext("animation_editor");
    REQUIRE(count == 1);
}

TEST_CASE("WorkspaceProjectState multiple listeners all fire", "[ProjectState][listeners]") {
    WorkspaceProjectState state;
    int a = 0, b = 0;
    state.addChangeListener([&] { ++a; });
    state.addChangeListener([&] { ++b; });

    StubAdapter adapter;
    state.onProjectLoaded(&adapter, {});
    REQUIRE(a == 1);
    REQUIRE(b == 1);
}

TEST_CASE("WorkspaceProjectState clearChangeListeners removes callbacks", "[ProjectState][listeners]") {
    WorkspaceProjectState state;
    int count = 0;
    state.addChangeListener([&] { ++count; });
    state.clearChangeListeners();

    StubAdapter adapter;
    state.onProjectLoaded(&adapter, {});
    REQUIRE(count == 0);
}

// ── DocumentKind helpers ──────────────────────────────────────────────────

TEST_CASE("documentKindName returns expected strings", "[ProjectState][kinds]") {
    CHECK(std::string(documentKindName(DocumentKind::Scene))       == "Scene");
    CHECK(std::string(documentKindName(DocumentKind::Asset))       == "Asset");
    CHECK(std::string(documentKindName(DocumentKind::Material))    == "Material");
    CHECK(std::string(documentKindName(DocumentKind::Animation))   == "Animation");
    CHECK(std::string(documentKindName(DocumentKind::VisualLogic)) == "VisualLogic");
    CHECK(std::string(documentKindName(DocumentKind::DataTable))   == "DataTable");
    CHECK(std::string(documentKindName(DocumentKind::NovaForge))   == "NovaForge");
    CHECK(std::string(documentKindName(DocumentKind::Unknown))     == "Unknown");
}

// ═══════════════════════════════════════════════════════════════════════════
//  WorkspaceShell::projectState() integration
// ═══════════════════════════════════════════════════════════════════════════

TEST_CASE("WorkspaceShell exposes projectState accessor", "[ProjectState][shell]") {
    WorkspaceShell shell;
    shell.initialize();

    // Accessor is valid even before project load
    WorkspaceProjectState& state = shell.projectState();
    REQUIRE_FALSE(state.hasProject());
    shell.shutdown();
}

TEST_CASE("WorkspaceShell loadProject notifies projectState", "[ProjectState][shell]") {
    WorkspaceShell shell;
    shell.initialize();

    auto adapter = std::make_unique<StubAdapter>("novaforge", "NovaForge");
    REQUIRE(shell.loadProject(std::move(adapter)));

    const WorkspaceProjectState& state = shell.projectState();
    REQUIRE(state.hasProject());
    REQUIRE(state.projectId()          == "novaforge");
    REQUIRE(state.projectDisplayName() == "NovaForge");

    shell.shutdown();
}

TEST_CASE("WorkspaceShell unloadProject clears projectState", "[ProjectState][shell]") {
    WorkspaceShell shell;
    shell.initialize();

    auto adapter = std::make_unique<StubAdapter>();
    shell.loadProject(std::move(adapter));
    REQUIRE(shell.projectState().hasProject());

    shell.unloadProject();
    REQUIRE_FALSE(shell.projectState().hasProject());

    shell.shutdown();
}

TEST_CASE("WorkspaceShell projectState survives multiple load/unload cycles", "[ProjectState][shell]") {
    WorkspaceShell shell;
    shell.initialize();

    for (int i = 0; i < 3; ++i) {
        auto adapter = std::make_unique<StubAdapter>(
            "proj." + std::to_string(i), "Project " + std::to_string(i));
        REQUIRE(shell.loadProject(std::move(adapter)));
        REQUIRE(shell.projectState().hasProject());
        shell.unloadProject();
        REQUIRE_FALSE(shell.projectState().hasProject());
    }
    shell.shutdown();
}

TEST_CASE("WorkspaceShell projectState change listener survives shell reload", "[ProjectState][shell]") {
    WorkspaceShell shell;
    shell.initialize();

    int loadEvents   = 0;
    int unloadEvents = 0;
    shell.projectState().addChangeListener([&] {
        if (shell.projectState().hasProject()) ++loadEvents;
        else ++unloadEvents;
    });

    auto adapter = std::make_unique<StubAdapter>();
    shell.loadProject(std::move(adapter));
    shell.unloadProject();

    REQUIRE(loadEvents   == 1);
    REQUIRE(unloadEvents == 1);

    shell.shutdown();
}

// ═══════════════════════════════════════════════════════════════════════════
//  saveAll with delegates — verifies real save pipeline
// ═══════════════════════════════════════════════════════════════════════════

TEST_CASE("WorkspaceProjectState saveAll calls registered save delegate",
          "[ProjectState][save][delegate]") {
    WorkspaceProjectState state;
    StubAdapter adapter;
    state.onProjectLoaded(&adapter, {});

    int saveCalls = 0;
    auto saveDelegate = [&](const std::string& docId,
                            const std::string& filePath) -> bool {
        (void)filePath;
        ++saveCalls;
        return docId != "fail_doc"; // simulate failure for "fail_doc"
    };

    state.openDocument(makeEntry("ok_doc", "OK Document", DocumentKind::Scene, true),
                       saveDelegate);
    state.openDocument(makeEntry("clean_doc", "Clean", DocumentKind::Asset, false));

    auto result = state.saveAll();
    REQUIRE(result.status     == ProjectSaveStatus::Ok);
    REQUIRE(result.savedCount == 1);
    REQUIRE(saveCalls         == 1);
    REQUIRE_FALSE(state.hasUnsavedChanges());
}

TEST_CASE("WorkspaceProjectState saveAll reports PartialFailure when delegate fails",
          "[ProjectState][save][delegate]") {
    WorkspaceProjectState state;
    StubAdapter adapter;
    state.onProjectLoaded(&adapter, {});

    auto okSave   = [](const std::string&, const std::string&) { return true;  };
    auto failSave = [](const std::string&, const std::string&) { return false; };

    state.openDocument(makeEntry("ok_doc", "OK", DocumentKind::Scene, true), okSave);
    state.openDocument(makeEntry("fail_doc", "Fail", DocumentKind::Material, true), failSave);

    auto result = state.saveAll();
    REQUIRE(result.status      == ProjectSaveStatus::PartialFailure);
    REQUIRE(result.savedCount  == 1);
    REQUIRE(result.failedCount == 1);
    // The failed document should still be dirty
    REQUIRE(state.findDocument("fail_doc")->isDirty);
    REQUIRE_FALSE(state.findDocument("ok_doc")->isDirty);
}

TEST_CASE("WorkspaceProjectState saveAll without delegate skips pathless dirty docs",
          "[ProjectState][save][delegate]") {
    WorkspaceProjectState state;
    StubAdapter adapter;
    state.onProjectLoaded(&adapter, {});

    // No delegate, no filePath → cannot be saved
    state.openDocument(makeEntry("orphan", "Orphan Doc", DocumentKind::Scene, true));

    auto result = state.saveAll();
    REQUIRE(result.status == ProjectSaveStatus::NothingDirty);
    REQUIRE(result.savedCount  == 0);
    REQUIRE(result.failedCount == 0);
    // Document is still dirty (we couldn't save it)
    REQUIRE(state.findDocument("orphan")->isDirty);
}

TEST_CASE("WorkspaceProjectState setSaveDelegate after openDocument",
          "[ProjectState][save][delegate]") {
    WorkspaceProjectState state;
    StubAdapter adapter;
    state.onProjectLoaded(&adapter, {});

    state.openDocument(makeEntry("doc1", "Doc", DocumentKind::Scene, true));
    // Initially no delegate — cannot save
    auto result1 = state.saveAll();
    REQUIRE(result1.status == ProjectSaveStatus::NothingDirty);
    REQUIRE(state.findDocument("doc1")->isDirty);

    // Wire in a delegate after the fact
    state.setSaveDelegate("doc1", [](const std::string&, const std::string&) { return true; });
    auto result2 = state.saveAll();
    REQUIRE(result2.status     == ProjectSaveStatus::Ok);
    REQUIRE(result2.savedCount == 1);
    REQUIRE_FALSE(state.findDocument("doc1")->isDirty);
}

TEST_CASE("WorkspaceProjectState closeDocument clears save delegate",
          "[ProjectState][save][delegate]") {
    WorkspaceProjectState state;
    StubAdapter adapter;
    state.onProjectLoaded(&adapter, {});

    int saveCalls = 0;
    state.openDocument(
        makeEntry("doc1", "Doc", DocumentKind::Scene, true),
        [&](const std::string&, const std::string&) { ++saveCalls; return true; });
    state.closeDocument("doc1");

    // Re-open without delegate
    state.openDocument(makeEntry("doc1", "Doc", DocumentKind::Scene, true));
    state.saveAll();
    REQUIRE(saveCalls == 0); // old delegate was cleaned up
}

TEST_CASE("WorkspaceProjectState onProjectUnloaded clears save delegates",
          "[ProjectState][save][delegate]") {
    WorkspaceProjectState state;
    StubAdapter adapter;
    state.onProjectLoaded(&adapter, {});

    state.openDocument(
        makeEntry("doc1", "Doc", DocumentKind::Scene, true),
        [](const std::string&, const std::string&) { return true; });

    state.onProjectUnloaded();
    REQUIRE(state.openDocumentCount() == 0);

    // Reload — delegate should not carry over
    state.onProjectLoaded(&adapter, {});
    state.openDocument(makeEntry("doc1", "Doc", DocumentKind::Scene, true));
    auto result = state.saveAll();
    REQUIRE(result.status == ProjectSaveStatus::NothingDirty); // no delegate, no path
}
