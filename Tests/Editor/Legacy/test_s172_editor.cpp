// S172 editor tests: ScriptingConsoleV1, IDEIntegrationV1, PluginSystemV1
#include <catch2/catch_test_macros.hpp>
#include <catch2/catch_approx.hpp>
#include "NF/Editor/Editor.h"
#include "NF/Editor/ScriptingConsoleV1.h"
#include "NF/Editor/IDEIntegrationV1.h"
#include "NF/Editor/PluginSystemV1.h"

using namespace NF;
using Catch::Approx;

// ── ScriptingConsoleV1 ───────────────────────────────────────────────────────

TEST_CASE("Scv1ScriptEntry validity", "[Editor][S172]") {
    Scv1ScriptEntry e;
    REQUIRE(!e.isValid());
    e.id = 1; e.name = "HelloScript";
    REQUIRE(e.isValid());
}

TEST_CASE("ScriptingConsoleV1 addEntry and entryCount", "[Editor][S172]") {
    ScriptingConsoleV1 sc;
    REQUIRE(sc.entryCount() == 0);
    Scv1ScriptEntry e; e.id = 1; e.name = "Script1";
    REQUIRE(sc.addEntry(e));
    REQUIRE(sc.entryCount() == 1);
}

TEST_CASE("ScriptingConsoleV1 addEntry invalid fails", "[Editor][S172]") {
    ScriptingConsoleV1 sc;
    REQUIRE(!sc.addEntry(Scv1ScriptEntry{}));
}

TEST_CASE("ScriptingConsoleV1 addEntry duplicate fails", "[Editor][S172]") {
    ScriptingConsoleV1 sc;
    Scv1ScriptEntry e; e.id = 1; e.name = "A";
    sc.addEntry(e);
    REQUIRE(!sc.addEntry(e));
}

TEST_CASE("ScriptingConsoleV1 removeEntry", "[Editor][S172]") {
    ScriptingConsoleV1 sc;
    Scv1ScriptEntry e; e.id = 2; e.name = "B";
    sc.addEntry(e);
    REQUIRE(sc.removeEntry(2));
    REQUIRE(sc.entryCount() == 0);
    REQUIRE(!sc.removeEntry(2));
}

TEST_CASE("ScriptingConsoleV1 executeScript and executeCount", "[Editor][S172]") {
    ScriptingConsoleV1 sc;
    Scv1ScriptEntry e; e.id = 1; e.name = "Run";
    sc.addEntry(e);
    REQUIRE(sc.executeScript(1));
    REQUIRE(sc.executeCount() == 1);
    REQUIRE(sc.findEntry(1)->isRunning());
    REQUIRE(sc.findEntry(1)->executeCount == 1);
}

TEST_CASE("ScriptingConsoleV1 executeScript unknown fails", "[Editor][S172]") {
    ScriptingConsoleV1 sc;
    REQUIRE(!sc.executeScript(99));
}

TEST_CASE("ScriptingConsoleV1 clearHistory resets output", "[Editor][S172]") {
    ScriptingConsoleV1 sc;
    Scv1ScriptEntry e; e.id = 1; e.name = "A";
    sc.addEntry(e);
    sc.executeScript(1);
    sc.clearHistory(1);
    REQUIRE(sc.findEntry(1)->executeCount == 0);
    REQUIRE(sc.findEntry(1)->output.empty());
}

TEST_CASE("ScriptingConsoleV1 setState and findEntry", "[Editor][S172]") {
    ScriptingConsoleV1 sc;
    Scv1ScriptEntry e; e.id = 1; e.name = "A";
    sc.addEntry(e);
    REQUIRE(sc.setState(1, Scv1ExecutionState::Complete));
    REQUIRE(sc.findEntry(1)->isComplete());
}

TEST_CASE("ScriptingConsoleV1 setState error", "[Editor][S172]") {
    ScriptingConsoleV1 sc;
    Scv1ScriptEntry e; e.id = 1; e.name = "A";
    sc.addEntry(e);
    REQUIRE(sc.setState(1, Scv1ExecutionState::Error));
    REQUIRE(sc.findEntry(1)->hasError());
}

TEST_CASE("ScriptingConsoleV1 countByChannel", "[Editor][S172]") {
    ScriptingConsoleV1 sc;
    Scv1ScriptEntry e1; e1.id = 1; e1.name = "A"; e1.channel = Scv1ChannelType::Debug;
    Scv1ScriptEntry e2; e2.id = 2; e2.name = "B"; e2.channel = Scv1ChannelType::Error;
    sc.addEntry(e1); sc.addEntry(e2);
    REQUIRE(sc.countByChannel(Scv1ChannelType::Debug) == 1);
    REQUIRE(sc.countByChannel(Scv1ChannelType::Error) == 1);
}

TEST_CASE("Scv1OutputLine validity", "[Editor][S172]") {
    Scv1OutputLine line;
    REQUIRE(!line.isValid());
    line.text = "Hello";
    REQUIRE(line.isValid());
}

TEST_CASE("Scv1ScriptEntry addOutput", "[Editor][S172]") {
    Scv1ScriptEntry e; e.id = 1; e.name = "A";
    Scv1OutputLine line; line.text = "output text";
    e.addOutput(line);
    REQUIRE(e.output.size() == 1);
}

TEST_CASE("scv1ChannelTypeName covers all values", "[Editor][S172]") {
    REQUIRE(std::string(scv1ChannelTypeName(Scv1ChannelType::Debug))   == "Debug");
    REQUIRE(std::string(scv1ChannelTypeName(Scv1ChannelType::Script))  == "Script");
}

TEST_CASE("scv1ExecutionStateName covers all values", "[Editor][S172]") {
    REQUIRE(std::string(scv1ExecutionStateName(Scv1ExecutionState::Idle))     == "Idle");
    REQUIRE(std::string(scv1ExecutionStateName(Scv1ExecutionState::Complete)) == "Complete");
}

TEST_CASE("ScriptingConsoleV1 findEntry returns nullptr when missing", "[Editor][S172]") {
    ScriptingConsoleV1 sc;
    REQUIRE(sc.findEntry(42) == nullptr);
}

TEST_CASE("ScriptingConsoleV1 onExecute callback", "[Editor][S172]") {
    ScriptingConsoleV1 sc;
    uint64_t notified = 0;
    sc.setOnExecute([&](uint64_t id) { notified = id; });
    Scv1ScriptEntry e; e.id = 7; e.name = "X";
    sc.addEntry(e);
    sc.executeScript(7);
    REQUIRE(notified == 7);
}

// ── IDEIntegrationV1 ─────────────────────────────────────────────────────────

TEST_CASE("Idev1ProjectFile validity", "[Editor][S172]") {
    Idev1ProjectFile f;
    REQUIRE(!f.isValid());
    f.id = 1; f.path = "Source/Main.cpp";
    REQUIRE(f.isValid());
}

TEST_CASE("IDEIntegrationV1 addFile and fileCount", "[Editor][S172]") {
    IDEIntegrationV1 ide;
    REQUIRE(ide.fileCount() == 0);
    Idev1ProjectFile f; f.id = 1; f.path = "A.cpp";
    REQUIRE(ide.addFile(f));
    REQUIRE(ide.fileCount() == 1);
}

TEST_CASE("IDEIntegrationV1 addFile invalid fails", "[Editor][S172]") {
    IDEIntegrationV1 ide;
    REQUIRE(!ide.addFile(Idev1ProjectFile{}));
}

TEST_CASE("IDEIntegrationV1 addFile duplicate fails", "[Editor][S172]") {
    IDEIntegrationV1 ide;
    Idev1ProjectFile f; f.id = 1; f.path = "A.cpp";
    ide.addFile(f);
    REQUIRE(!ide.addFile(f));
}

TEST_CASE("IDEIntegrationV1 removeFile", "[Editor][S172]") {
    IDEIntegrationV1 ide;
    Idev1ProjectFile f; f.id = 2; f.path = "B.cpp";
    ide.addFile(f);
    REQUIRE(ide.removeFile(2));
    REQUIRE(ide.fileCount() == 0);
    REQUIRE(!ide.removeFile(2));
}

TEST_CASE("IDEIntegrationV1 setState syncedCount", "[Editor][S172]") {
    IDEIntegrationV1 ide;
    Idev1ProjectFile f; f.id = 1; f.path = "A.cpp";
    ide.addFile(f);
    REQUIRE(ide.setState(1, Idev1FileState::Synced));
    REQUIRE(ide.syncedCount() == 1);
    REQUIRE(ide.findFile(1)->isSynced());
}

TEST_CASE("IDEIntegrationV1 modifiedCount", "[Editor][S172]") {
    IDEIntegrationV1 ide;
    Idev1ProjectFile f; f.id = 1; f.path = "A.cpp";
    ide.addFile(f);
    ide.setState(1, Idev1FileState::Modified);
    REQUIRE(ide.modifiedCount() == 1);
    REQUIRE(ide.findFile(1)->isModified());
}

TEST_CASE("IDEIntegrationV1 conflict state", "[Editor][S172]") {
    IDEIntegrationV1 ide;
    Idev1ProjectFile f; f.id = 1; f.path = "A.cpp";
    ide.addFile(f);
    ide.setState(1, Idev1FileState::Conflict);
    REQUIRE(ide.findFile(1)->hasConflict());
}

TEST_CASE("IDEIntegrationV1 connectServer and serverState", "[Editor][S172]") {
    IDEIntegrationV1 ide;
    Idev1LanguageServer srv; srv.name = "clangd"; srv.endpoint = "127.0.0.1:7890";
    REQUIRE(ide.connectServer(srv));
    REQUIRE(ide.serverState() == Idev1ServerState::Connected);
}

TEST_CASE("IDEIntegrationV1 connectServer invalid fails", "[Editor][S172]") {
    IDEIntegrationV1 ide;
    REQUIRE(!ide.connectServer(Idev1LanguageServer{}));
}

TEST_CASE("IDEIntegrationV1 disconnectServer", "[Editor][S172]") {
    IDEIntegrationV1 ide;
    Idev1LanguageServer srv; srv.name = "clangd"; srv.endpoint = "127.0.0.1:7890";
    ide.connectServer(srv);
    REQUIRE(ide.disconnectServer());
    REQUIRE(ide.serverState() == Idev1ServerState::Disconnected);
}

TEST_CASE("idev1FileStateName covers all values", "[Editor][S172]") {
    REQUIRE(std::string(idev1FileStateName(Idev1FileState::Unknown))  == "Unknown");
    REQUIRE(std::string(idev1FileStateName(Idev1FileState::Conflict)) == "Conflict");
}

TEST_CASE("idev1ServerStateName covers all values", "[Editor][S172]") {
    REQUIRE(std::string(idev1ServerStateName(Idev1ServerState::Disconnected)) == "Disconnected");
    REQUIRE(std::string(idev1ServerStateName(Idev1ServerState::Error))        == "Error");
}

TEST_CASE("IDEIntegrationV1 onSync callback", "[Editor][S172]") {
    IDEIntegrationV1 ide;
    uint64_t notified = 0;
    ide.setOnSync([&](uint64_t id) { notified = id; });
    Idev1ProjectFile f; f.id = 5; f.path = "E.cpp";
    ide.addFile(f);
    ide.setState(5, Idev1FileState::Synced);
    REQUIRE(notified == 5);
}

// ── PluginSystemV1 ───────────────────────────────────────────────────────────

TEST_CASE("Psyv1PluginDescriptor validity", "[Editor][S172]") {
    Psyv1PluginDescriptor p;
    REQUIRE(!p.isValid());
    p.id = 1; p.name = "MeshImporter";
    REQUIRE(p.isValid());
}

TEST_CASE("PluginSystemV1 registerPlugin and pluginCount", "[Editor][S172]") {
    PluginSystemV1 ps;
    REQUIRE(ps.pluginCount() == 0);
    Psyv1PluginDescriptor p; p.id = 1; p.name = "P1";
    REQUIRE(ps.registerPlugin(p));
    REQUIRE(ps.pluginCount() == 1);
}

TEST_CASE("PluginSystemV1 registerPlugin invalid fails", "[Editor][S172]") {
    PluginSystemV1 ps;
    REQUIRE(!ps.registerPlugin(Psyv1PluginDescriptor{}));
}

TEST_CASE("PluginSystemV1 registerPlugin duplicate fails", "[Editor][S172]") {
    PluginSystemV1 ps;
    Psyv1PluginDescriptor p; p.id = 1; p.name = "A";
    ps.registerPlugin(p);
    REQUIRE(!ps.registerPlugin(p));
}

TEST_CASE("PluginSystemV1 unregisterPlugin", "[Editor][S172]") {
    PluginSystemV1 ps;
    Psyv1PluginDescriptor p; p.id = 2; p.name = "B";
    ps.registerPlugin(p);
    REQUIRE(ps.unregisterPlugin(2));
    REQUIRE(ps.pluginCount() == 0);
    REQUIRE(!ps.unregisterPlugin(2));
}

TEST_CASE("PluginSystemV1 loadPlugin and loadedCount", "[Editor][S172]") {
    PluginSystemV1 ps;
    Psyv1PluginDescriptor p; p.id = 1; p.name = "A";
    ps.registerPlugin(p);
    REQUIRE(ps.loadPlugin(1));
    REQUIRE(ps.loadedCount() == 1);
    REQUIRE(ps.findPlugin(1)->isLoaded());
}

TEST_CASE("PluginSystemV1 loadPlugin disabled fails", "[Editor][S172]") {
    PluginSystemV1 ps;
    Psyv1PluginDescriptor p; p.id = 1; p.name = "A"; p.state = Psyv1PluginState::Disabled;
    ps.registerPlugin(p);
    REQUIRE(!ps.loadPlugin(1));
}

TEST_CASE("PluginSystemV1 unloadPlugin", "[Editor][S172]") {
    PluginSystemV1 ps;
    Psyv1PluginDescriptor p; p.id = 1; p.name = "A";
    ps.registerPlugin(p);
    ps.loadPlugin(1);
    REQUIRE(ps.unloadPlugin(1));
    REQUIRE(ps.loadedCount() == 0);
}

TEST_CASE("PluginSystemV1 setState failedCount", "[Editor][S172]") {
    PluginSystemV1 ps;
    Psyv1PluginDescriptor p; p.id = 1; p.name = "A";
    ps.registerPlugin(p);
    REQUIRE(ps.setState(1, Psyv1PluginState::Failed));
    REQUIRE(ps.failedCount() == 1);
    REQUIRE(ps.findPlugin(1)->hasFailed());
}

TEST_CASE("PluginSystemV1 countByCategory", "[Editor][S172]") {
    PluginSystemV1 ps;
    Psyv1PluginDescriptor p1; p1.id = 1; p1.name = "A"; p1.category = Psyv1PluginCategory::Importer;
    Psyv1PluginDescriptor p2; p2.id = 2; p2.name = "B"; p2.category = Psyv1PluginCategory::Theme;
    ps.registerPlugin(p1); ps.registerPlugin(p2);
    REQUIRE(ps.countByCategory(Psyv1PluginCategory::Importer) == 1);
    REQUIRE(ps.countByCategory(Psyv1PluginCategory::Theme)    == 1);
}

TEST_CASE("psyv1PluginStateName covers all values", "[Editor][S172]") {
    REQUIRE(std::string(psyv1PluginStateName(Psyv1PluginState::Unloaded)) == "Unloaded");
    REQUIRE(std::string(psyv1PluginStateName(Psyv1PluginState::Disabled)) == "Disabled");
}

TEST_CASE("psyv1PluginCategoryName covers all values", "[Editor][S172]") {
    REQUIRE(std::string(psyv1PluginCategoryName(Psyv1PluginCategory::Importer))  == "Importer");
    REQUIRE(std::string(psyv1PluginCategoryName(Psyv1PluginCategory::Extension)) == "Extension");
}

TEST_CASE("PluginSystemV1 findPlugin returns nullptr when missing", "[Editor][S172]") {
    PluginSystemV1 ps;
    REQUIRE(ps.findPlugin(42) == nullptr);
}

TEST_CASE("PluginSystemV1 onStateChange callback", "[Editor][S172]") {
    PluginSystemV1 ps;
    uint64_t notified = 0;
    ps.setOnStateChange([&](uint64_t id) { notified = id; });
    Psyv1PluginDescriptor p; p.id = 9; p.name = "Z";
    ps.registerPlugin(p);
    ps.loadPlugin(9);
    REQUIRE(notified == 9);
}
