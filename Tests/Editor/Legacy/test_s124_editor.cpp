// S124 editor tests: SandboxEditor, SceneIsolationEditor, PlaymodeEditor
#include <catch2/catch_test_macros.hpp>
#include "NF/Editor/Editor.h"
#include "NF/Editor/PlaymodeEditor.h"
#include "NF/Editor/SceneIsolationEditor.h"
#include "NF/Editor/SandboxEditor.h"

using namespace NF;

// ── SandboxEditor ─────────────────────────────────────────────────────────────

TEST_CASE("SandboxMode names", "[Editor][S124]") {
    REQUIRE(std::string(sandboxModeName(SandboxMode::Isolated))   == "Isolated");
    REQUIRE(std::string(sandboxModeName(SandboxMode::Networked))  == "Networked");
    REQUIRE(std::string(sandboxModeName(SandboxMode::Headless))   == "Headless");
    REQUIRE(std::string(sandboxModeName(SandboxMode::FullStack))  == "FullStack");
    REQUIRE(std::string(sandboxModeName(SandboxMode::Custom))     == "Custom");
}

TEST_CASE("SandboxState names", "[Editor][S124]") {
    REQUIRE(std::string(sandboxStateName(SandboxState::Inactive))  == "Inactive");
    REQUIRE(std::string(sandboxStateName(SandboxState::Launching)) == "Launching");
    REQUIRE(std::string(sandboxStateName(SandboxState::Active))    == "Active");
    REQUIRE(std::string(sandboxStateName(SandboxState::Paused))    == "Paused");
    REQUIRE(std::string(sandboxStateName(SandboxState::Crashed))   == "Crashed");
    REQUIRE(std::string(sandboxStateName(SandboxState::Destroyed)) == "Destroyed");
}

TEST_CASE("SandboxEntry defaults", "[Editor][S124]") {
    SandboxEntry e(1, "sandbox_a", SandboxMode::Isolated);
    REQUIRE(e.id()           == 1u);
    REQUIRE(e.name()         == "sandbox_a");
    REQUIRE(e.mode()         == SandboxMode::Isolated);
    REQUIRE(e.state()        == SandboxState::Inactive);
    REQUIRE(!e.isRecordable());
    REQUIRE(e.timeoutSec()   == 30.0f);
    REQUIRE(e.isEnabled());
}

TEST_CASE("SandboxEntry mutation", "[Editor][S124]") {
    SandboxEntry e(2, "sandbox_b", SandboxMode::Networked);
    e.setState(SandboxState::Active);
    e.setIsRecordable(true);
    e.setTimeoutSec(60.0f);
    e.setIsEnabled(false);
    REQUIRE(e.state()        == SandboxState::Active);
    REQUIRE(e.isRecordable());
    REQUIRE(e.timeoutSec()   == 60.0f);
    REQUIRE(!e.isEnabled());
}

TEST_CASE("SandboxEditor defaults", "[Editor][S124]") {
    SandboxEditor ed;
    REQUIRE(ed.isShowInactive());
    REQUIRE(!ed.isGroupByMode());
    REQUIRE(ed.maxConcurrent() == 4u);
    REQUIRE(ed.entryCount()    == 0u);
}

TEST_CASE("SandboxEditor add/remove entries", "[Editor][S124]") {
    SandboxEditor ed;
    REQUIRE(ed.addEntry(SandboxEntry(1, "sb_a", SandboxMode::Isolated)));
    REQUIRE(ed.addEntry(SandboxEntry(2, "sb_b", SandboxMode::Networked)));
    REQUIRE(ed.addEntry(SandboxEntry(3, "sb_c", SandboxMode::Headless)));
    REQUIRE(!ed.addEntry(SandboxEntry(1, "sb_a", SandboxMode::Isolated)));
    REQUIRE(ed.entryCount() == 3u);
    REQUIRE(ed.removeEntry(2));
    REQUIRE(ed.entryCount() == 2u);
    REQUIRE(!ed.removeEntry(99));
}

TEST_CASE("SandboxEditor counts and find", "[Editor][S124]") {
    SandboxEditor ed;
    SandboxEntry e1(1, "sb_a", SandboxMode::Isolated);
    SandboxEntry e2(2, "sb_b", SandboxMode::Isolated);  e2.setState(SandboxState::Active);
    SandboxEntry e3(3, "sb_c", SandboxMode::Networked); e3.setIsEnabled(false);
    SandboxEntry e4(4, "sb_d", SandboxMode::Headless);  e4.setState(SandboxState::Active); e4.setIsEnabled(false);
    ed.addEntry(e1); ed.addEntry(e2); ed.addEntry(e3); ed.addEntry(e4);
    REQUIRE(ed.countByMode(SandboxMode::Isolated)   == 2u);
    REQUIRE(ed.countByMode(SandboxMode::Networked)  == 1u);
    REQUIRE(ed.countByMode(SandboxMode::Custom)     == 0u);
    REQUIRE(ed.countByState(SandboxState::Inactive) == 2u);
    REQUIRE(ed.countByState(SandboxState::Active)   == 2u);
    REQUIRE(ed.countEnabled()                       == 2u);
    auto* found = ed.findEntry(3);
    REQUIRE(found != nullptr);
    REQUIRE(found->mode() == SandboxMode::Networked);
    REQUIRE(ed.findEntry(99) == nullptr);
}

TEST_CASE("SandboxEditor settings mutation", "[Editor][S124]") {
    SandboxEditor ed;
    ed.setIsShowInactive(false);
    ed.setIsGroupByMode(true);
    ed.setMaxConcurrent(8u);
    REQUIRE(!ed.isShowInactive());
    REQUIRE(ed.isGroupByMode());
    REQUIRE(ed.maxConcurrent() == 8u);
}

// ── SceneIsolationEditor ──────────────────────────────────────────────────────

TEST_CASE("IsolationScope names", "[Editor][S124]") {
    REQUIRE(std::string(isolationScopeName(IsolationScope::World))    == "World");
    REQUIRE(std::string(isolationScopeName(IsolationScope::SubScene)) == "SubScene");
    REQUIRE(std::string(isolationScopeName(IsolationScope::Prefab))   == "Prefab");
    REQUIRE(std::string(isolationScopeName(IsolationScope::Layer))    == "Layer");
    REQUIRE(std::string(isolationScopeName(IsolationScope::Custom))   == "Custom");
}

TEST_CASE("IsolationPolicy names", "[Editor][S124]") {
    REQUIRE(std::string(isolationPolicyName(IsolationPolicy::ReadOnly))    == "ReadOnly");
    REQUIRE(std::string(isolationPolicyName(IsolationPolicy::CopyOnWrite)) == "CopyOnWrite");
    REQUIRE(std::string(isolationPolicyName(IsolationPolicy::Exclusive))   == "Exclusive");
    REQUIRE(std::string(isolationPolicyName(IsolationPolicy::Shared))      == "Shared");
}

TEST_CASE("IsolatedScene defaults", "[Editor][S124]") {
    IsolatedScene sc(1, "world_a", IsolationScope::World, IsolationPolicy::ReadOnly);
    REQUIRE(sc.id()             == 1u);
    REQUIRE(sc.name()           == "world_a");
    REQUIRE(sc.scope()          == IsolationScope::World);
    REQUIRE(sc.policy()         == IsolationPolicy::ReadOnly);
    REQUIRE(!sc.isActive());
    REQUIRE(sc.memoryBudgetMB() == 256u);
    REQUIRE(sc.isEnabled());
}

TEST_CASE("IsolatedScene mutation", "[Editor][S124]") {
    IsolatedScene sc(2, "prefab_b", IsolationScope::Prefab, IsolationPolicy::CopyOnWrite);
    sc.setIsActive(true);
    sc.setMemoryBudgetMB(512u);
    sc.setIsEnabled(false);
    REQUIRE(sc.isActive());
    REQUIRE(sc.memoryBudgetMB() == 512u);
    REQUIRE(!sc.isEnabled());
}

TEST_CASE("SceneIsolationEditor defaults", "[Editor][S124]") {
    SceneIsolationEditor ed;
    REQUIRE(ed.isShowInactive());
    REQUIRE(!ed.isGroupByScope());
    REQUIRE(ed.defaultBudgetMB() == 512u);
    REQUIRE(ed.sceneCount()      == 0u);
}

TEST_CASE("SceneIsolationEditor add/remove scenes", "[Editor][S124]") {
    SceneIsolationEditor ed;
    REQUIRE(ed.addScene(IsolatedScene(1, "w_a",  IsolationScope::World,    IsolationPolicy::ReadOnly)));
    REQUIRE(ed.addScene(IsolatedScene(2, "ss_a", IsolationScope::SubScene, IsolationPolicy::Exclusive)));
    REQUIRE(ed.addScene(IsolatedScene(3, "p_a",  IsolationScope::Prefab,   IsolationPolicy::Shared)));
    REQUIRE(!ed.addScene(IsolatedScene(1, "w_a", IsolationScope::World,    IsolationPolicy::ReadOnly)));
    REQUIRE(ed.sceneCount() == 3u);
    REQUIRE(ed.removeScene(2));
    REQUIRE(ed.sceneCount() == 2u);
    REQUIRE(!ed.removeScene(99));
}

TEST_CASE("SceneIsolationEditor counts and find", "[Editor][S124]") {
    SceneIsolationEditor ed;
    IsolatedScene s1(1, "w_a",  IsolationScope::World,    IsolationPolicy::ReadOnly);
    IsolatedScene s2(2, "w_b",  IsolationScope::World,    IsolationPolicy::ReadOnly);  s2.setIsActive(true);
    IsolatedScene s3(3, "ss_a", IsolationScope::SubScene, IsolationPolicy::Exclusive); s3.setIsEnabled(false);
    IsolatedScene s4(4, "p_a",  IsolationScope::Prefab,   IsolationPolicy::Shared);    s4.setIsActive(true); s4.setIsEnabled(false);
    ed.addScene(s1); ed.addScene(s2); ed.addScene(s3); ed.addScene(s4);
    REQUIRE(ed.countByScope(IsolationScope::World)              == 2u);
    REQUIRE(ed.countByScope(IsolationScope::SubScene)           == 1u);
    REQUIRE(ed.countByScope(IsolationScope::Layer)              == 0u);
    REQUIRE(ed.countByPolicy(IsolationPolicy::ReadOnly)         == 2u);
    REQUIRE(ed.countByPolicy(IsolationPolicy::Exclusive)        == 1u);
    REQUIRE(ed.countActive()                                    == 2u);
    auto* found = ed.findScene(3);
    REQUIRE(found != nullptr);
    REQUIRE(found->scope() == IsolationScope::SubScene);
    REQUIRE(ed.findScene(99) == nullptr);
}

TEST_CASE("SceneIsolationEditor settings mutation", "[Editor][S124]") {
    SceneIsolationEditor ed;
    ed.setIsShowInactive(false);
    ed.setIsGroupByScope(true);
    ed.setDefaultBudgetMB(1024u);
    REQUIRE(!ed.isShowInactive());
    REQUIRE(ed.isGroupByScope());
    REQUIRE(ed.defaultBudgetMB() == 1024u);
}

// ── PlaymodeEditor ────────────────────────────────────────────────────────────

TEST_CASE("PlaymodeState names", "[Editor][S124]") {
    REQUIRE(std::string(playmodeStateName(PlaymodeState::Stopped))  == "Stopped");
    REQUIRE(std::string(playmodeStateName(PlaymodeState::Entering)) == "Entering");
    REQUIRE(std::string(playmodeStateName(PlaymodeState::Running))  == "Running");
    REQUIRE(std::string(playmodeStateName(PlaymodeState::Paused))   == "Paused");
    REQUIRE(std::string(playmodeStateName(PlaymodeState::Exiting))  == "Exiting");
}

TEST_CASE("PlaymodeTarget names", "[Editor][S124]") {
    REQUIRE(std::string(playmodeTargetName(PlaymodeTarget::Editor))           == "Editor");
    REQUIRE(std::string(playmodeTargetName(PlaymodeTarget::StandaloneWindow)) == "StandaloneWindow");
    REQUIRE(std::string(playmodeTargetName(PlaymodeTarget::DevicePreview))    == "DevicePreview");
    REQUIRE(std::string(playmodeTargetName(PlaymodeTarget::Headless))         == "Headless");
    REQUIRE(std::string(playmodeTargetName(PlaymodeTarget::Custom))           == "Custom");
}

TEST_CASE("PlaymodeSession defaults", "[Editor][S124]") {
    PlaymodeSession ps(1, "session_a", PlaymodeTarget::Editor);
    REQUIRE(ps.id()          == 1u);
    REQUIRE(ps.name()        == "session_a");
    REQUIRE(ps.target()      == PlaymodeTarget::Editor);
    REQUIRE(ps.state()       == PlaymodeState::Stopped);
    REQUIRE(ps.frameCount()  == 0u);
    REQUIRE(!ps.isRecording());
    REQUIRE(ps.isEnabled());
}

TEST_CASE("PlaymodeSession mutation", "[Editor][S124]") {
    PlaymodeSession ps(2, "session_b", PlaymodeTarget::StandaloneWindow);
    ps.setState(PlaymodeState::Running);
    ps.setFrameCount(1200u);
    ps.setIsRecording(true);
    ps.setIsEnabled(false);
    REQUIRE(ps.state()       == PlaymodeState::Running);
    REQUIRE(ps.frameCount()  == 1200u);
    REQUIRE(ps.isRecording());
    REQUIRE(!ps.isEnabled());
}

TEST_CASE("PlaymodeEditor defaults", "[Editor][S124]") {
    PlaymodeEditor ed;
    REQUIRE(ed.isAutoRecompile());
    REQUIRE(ed.isRestoreOnExit());
    REQUIRE(ed.targetFPS()     == 60u);
    REQUIRE(ed.sessionCount()  == 0u);
}

TEST_CASE("PlaymodeEditor add/remove sessions", "[Editor][S124]") {
    PlaymodeEditor ed;
    REQUIRE(ed.addSession(PlaymodeSession(1, "sess_a", PlaymodeTarget::Editor)));
    REQUIRE(ed.addSession(PlaymodeSession(2, "sess_b", PlaymodeTarget::StandaloneWindow)));
    REQUIRE(ed.addSession(PlaymodeSession(3, "sess_c", PlaymodeTarget::Headless)));
    REQUIRE(!ed.addSession(PlaymodeSession(1, "sess_a", PlaymodeTarget::Editor)));
    REQUIRE(ed.sessionCount() == 3u);
    REQUIRE(ed.removeSession(2));
    REQUIRE(ed.sessionCount() == 2u);
    REQUIRE(!ed.removeSession(99));
}

TEST_CASE("PlaymodeEditor counts and find", "[Editor][S124]") {
    PlaymodeEditor ed;
    PlaymodeSession p1(1, "s_a", PlaymodeTarget::Editor);
    PlaymodeSession p2(2, "s_b", PlaymodeTarget::Editor);         p2.setState(PlaymodeState::Running);
    PlaymodeSession p3(3, "s_c", PlaymodeTarget::Headless);       p3.setIsRecording(true);
    PlaymodeSession p4(4, "s_d", PlaymodeTarget::DevicePreview);  p4.setState(PlaymodeState::Running); p4.setIsRecording(true);
    ed.addSession(p1); ed.addSession(p2); ed.addSession(p3); ed.addSession(p4);
    REQUIRE(ed.countByTarget(PlaymodeTarget::Editor)        == 2u);
    REQUIRE(ed.countByTarget(PlaymodeTarget::Headless)      == 1u);
    REQUIRE(ed.countByTarget(PlaymodeTarget::Custom)        == 0u);
    REQUIRE(ed.countByState(PlaymodeState::Stopped)         == 2u);
    REQUIRE(ed.countByState(PlaymodeState::Running)         == 2u);
    REQUIRE(ed.countRecording()                             == 2u);
    auto* found = ed.findSession(3);
    REQUIRE(found != nullptr);
    REQUIRE(found->target() == PlaymodeTarget::Headless);
    REQUIRE(ed.findSession(99) == nullptr);
}

TEST_CASE("PlaymodeEditor settings mutation", "[Editor][S124]") {
    PlaymodeEditor ed;
    ed.setIsAutoRecompile(false);
    ed.setIsRestoreOnExit(false);
    ed.setTargetFPS(30u);
    REQUIRE(!ed.isAutoRecompile());
    REQUIRE(!ed.isRestoreOnExit());
    REQUIRE(ed.targetFPS() == 30u);
}
