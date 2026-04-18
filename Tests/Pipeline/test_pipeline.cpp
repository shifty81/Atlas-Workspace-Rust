#include <catch2/catch_test_macros.hpp>
#include "NF/Pipeline/Pipeline.h"

#include <atomic>
#include <chrono>
#include <filesystem>
#include <fstream>
#include <string>
#include <thread>
#include <vector>

namespace fs = std::filesystem;

// Helper: create a unique temp directory for each test invocation.
// Uses a nanosecond timestamp + per-call counter to avoid collisions
// even when the test binary is run multiple times in quick succession.
static fs::path makeTempDir(std::string_view name) {
    static std::atomic<uint64_t> s_seq{0};
    auto ts  = static_cast<uint64_t>(
        std::chrono::steady_clock::now().time_since_epoch().count());
    auto idx = s_seq.fetch_add(1);
    std::string unique = std::string(name) + "_" +
                         std::to_string(ts)  + "_" +
                         std::to_string(idx);
    auto dir = fs::temp_directory_path() / "nf_pipeline_tests" / unique;
    std::error_code ec;
    fs::create_directories(dir, ec);
    return dir;
}

// ── ChangeEventType ───────────────────────────────────────────────────────

TEST_CASE("ChangeEventType names are non-empty for all values", "[Pipeline][ChangeEventType]") {
    using ET = NF::ChangeEventType;
    REQUIRE(std::string(NF::changeEventTypeName(ET::Unknown))           == "Unknown");
    REQUIRE(std::string(NF::changeEventTypeName(ET::AssetImported))     == "AssetImported");
    REQUIRE(std::string(NF::changeEventTypeName(ET::WorldChanged))      == "WorldChanged");
    REQUIRE(std::string(NF::changeEventTypeName(ET::ScriptUpdated))     == "ScriptUpdated");
    REQUIRE(std::string(NF::changeEventTypeName(ET::AnimationExported)) == "AnimationExported");
    REQUIRE(std::string(NF::changeEventTypeName(ET::ContractIssue))     == "ContractIssue");
    REQUIRE(std::string(NF::changeEventTypeName(ET::ReplayExported))    == "ReplayExported");
}

TEST_CASE("ChangeEventType fromString round-trips all known values", "[Pipeline][ChangeEventType]") {
    using ET = NF::ChangeEventType;
    REQUIRE(NF::changeEventTypeFromString("AssetImported")     == ET::AssetImported);
    REQUIRE(NF::changeEventTypeFromString("WorldChanged")      == ET::WorldChanged);
    REQUIRE(NF::changeEventTypeFromString("ScriptUpdated")     == ET::ScriptUpdated);
    REQUIRE(NF::changeEventTypeFromString("AnimationExported") == ET::AnimationExported);
    REQUIRE(NF::changeEventTypeFromString("ContractIssue")     == ET::ContractIssue);
    REQUIRE(NF::changeEventTypeFromString("ReplayExported")    == ET::ReplayExported);
    REQUIRE(NF::changeEventTypeFromString("garbage")           == ET::Unknown);
    REQUIRE(NF::changeEventTypeFromString("")                  == ET::Unknown);
}

// ── ChangeEvent JSON ──────────────────────────────────────────────────────

TEST_CASE("ChangeEvent toJson contains all five fields", "[Pipeline][ChangeEvent]") {
    NF::ChangeEvent ev;
    ev.tool      = "BlenderGenerator";
    ev.eventType = NF::ChangeEventType::AssetImported;
    ev.path      = "assets/cube.glb";
    ev.timestamp = 1712282858000LL;
    ev.metadata  = "source=blender";

    std::string j = ev.toJson();
    REQUIRE(j.find("BlenderGenerator") != std::string::npos);
    REQUIRE(j.find("AssetImported")    != std::string::npos);
    REQUIRE(j.find("assets/cube.glb")  != std::string::npos);
    REQUIRE(j.find("1712282858000")    != std::string::npos);
    REQUIRE(j.find("source=blender")   != std::string::npos);
}

TEST_CASE("ChangeEvent fromJson round-trip", "[Pipeline][ChangeEvent]") {
    NF::ChangeEvent original;
    original.tool      = "AtlasAI";
    original.eventType = NF::ChangeEventType::WorldChanged;
    original.path      = "worlds/sector01.json";
    original.timestamp = 9999LL;
    original.metadata  = "biome=desert";

    NF::ChangeEvent parsed;
    REQUIRE(NF::ChangeEvent::fromJson(original.toJson(), parsed));
    REQUIRE(parsed.tool              == "AtlasAI");
    REQUIRE(parsed.eventType         == NF::ChangeEventType::WorldChanged);
    REQUIRE(parsed.path              == "worlds/sector01.json");
    REQUIRE(parsed.timestamp         == 9999LL);
    REQUIRE(parsed.metadata          == "biome=desert");
}

TEST_CASE("ChangeEvent fromJson with unknown event_type produces Unknown", "[Pipeline][ChangeEvent]") {
    std::string json = R"({"tool":"x","event_type":"Bogus","path":"","timestamp":0,"metadata":""})";
    NF::ChangeEvent ev;
    REQUIRE(NF::ChangeEvent::fromJson(json, ev));
    REQUIRE(ev.eventType == NF::ChangeEventType::Unknown);
}

// ── ChangeEvent file I/O ──────────────────────────────────────────────────

TEST_CASE("ChangeEvent writeToFile creates a .change.json file", "[Pipeline][ChangeEvent]") {
    auto dir = makeTempDir("write_file");

    NF::ChangeEvent ev;
    ev.tool      = "ContractScanner";
    ev.eventType = NF::ChangeEventType::ContractIssue;
    ev.path      = "src/Foo.cpp";
    ev.timestamp = 1000LL;

    REQUIRE(ev.writeToFile(dir));

    bool found = false;
    for (const auto& entry : fs::directory_iterator(dir)) {
        if (entry.path().extension() == ".json") { found = true; break; }
    }
    REQUIRE(found);
}

TEST_CASE("ChangeEvent readFromFile round-trip", "[Pipeline][ChangeEvent]") {
    auto dir = makeTempDir("read_file");

    NF::ChangeEvent original;
    original.tool      = "ReplayMinimizer";
    original.eventType = NF::ChangeEventType::ReplayExported;
    original.path      = "replays/run42.replay.json";
    original.timestamp = 42000LL;
    original.metadata  = "frames=300";

    REQUIRE(original.writeToFile(dir));

    // Find the written file.
    fs::path written;
    for (const auto& entry : fs::directory_iterator(dir)) {
        if (entry.path().extension() == ".json") { written = entry.path(); break; }
    }
    REQUIRE(!written.empty());

    NF::ChangeEvent parsed;
    REQUIRE(NF::ChangeEvent::readFromFile(written, parsed));
    REQUIRE(parsed.tool      == "ReplayMinimizer");
    REQUIRE(parsed.eventType == NF::ChangeEventType::ReplayExported);
    REQUIRE(parsed.path      == "replays/run42.replay.json");
    REQUIRE(parsed.timestamp == 42000LL);
    REQUIRE(parsed.metadata  == "frames=300");
}

// ── Manifest ──────────────────────────────────────────────────────────────

TEST_CASE("Manifest registerAsset generates unique GUIDs", "[Pipeline][Manifest]") {
    NF::Manifest m;
    auto g1 = m.registerAsset({"", "mesh",    "a/mesh.glb",    0, ""});
    auto g2 = m.registerAsset({"", "texture", "a/tex.png",     0, ""});
    auto g3 = m.registerAsset({"", "animation","a/run.glb",    0, ""});
    REQUIRE(g1 != g2);
    REQUIRE(g1 != g3);
    REQUIRE(g2 != g3);
    REQUIRE(m.recordCount() == 3);
}

TEST_CASE("Manifest findByGuid returns registered record", "[Pipeline][Manifest]") {
    NF::Manifest m;
    auto guid = m.registerAsset({"", "mesh", "models/ship.glb", 1000LL, "abc"});
    const auto* rec = m.findByGuid(guid);
    REQUIRE(rec != nullptr);
    REQUIRE(rec->type      == "mesh");
    REQUIRE(rec->path      == "models/ship.glb");
    REQUIRE(rec->importDate == 1000LL);
    REQUIRE(rec->checksum  == "abc");
}

TEST_CASE("Manifest findByPath returns registered record", "[Pipeline][Manifest]") {
    NF::Manifest m;
    m.registerAsset({"", "texture", "textures/rock.png", 0, ""});
    const auto* rec = m.findByPath("textures/rock.png");
    REQUIRE(rec != nullptr);
    REQUIRE(rec->type == "texture");
}

TEST_CASE("Manifest removeAsset removes by GUID", "[Pipeline][Manifest]") {
    NF::Manifest m;
    auto g = m.registerAsset({"", "script", "scripts/door.graph", 0, ""});
    REQUIRE(m.recordCount() == 1);
    REQUIRE(m.removeAsset(g));
    REQUIRE(m.recordCount() == 0);
    REQUIRE(m.findByGuid(g) == nullptr);
    REQUIRE(!m.removeAsset(g)); // second remove returns false
}

TEST_CASE("Manifest save and load round-trip", "[Pipeline][Manifest]") {
    auto dir  = makeTempDir("manifest_roundtrip");
    auto path = dir / "manifest.json";

    NF::Manifest original;
    original.projectName = "TestProject";
    original.modules     = {"Core", "Engine", "Pipeline"};
    original.registerAsset({"", "mesh",    "models/cube.glb",    100LL, "sha1"});
    original.registerAsset({"", "texture", "textures/dirt.png",  200LL, "sha2"});

    REQUIRE(original.save(path));

    NF::Manifest loaded;
    REQUIRE(loaded.load(path));
    REQUIRE(loaded.projectName   == "TestProject");
    REQUIRE(loaded.modules.size() == 3);
    REQUIRE(loaded.recordCount() == 2);

    const auto* mesh = loaded.findByPath("models/cube.glb");
    REQUIRE(mesh != nullptr);
    REQUIRE(mesh->type      == "mesh");
    REQUIRE(mesh->importDate == 100LL);
    REQUIRE(mesh->checksum  == "sha1");

    const auto* tex = loaded.findByPath("textures/dirt.png");
    REQUIRE(tex != nullptr);
    REQUIRE(tex->type == "texture");
}

// ── WatchLog ──────────────────────────────────────────────────────────────

TEST_CASE("WatchLog appendLine creates file with expected content", "[Pipeline][WatchLog]") {
    auto dir  = makeTempDir("watchlog");
    auto path = dir / "watch.log";

    NF::WatchLog log(path);
    log.appendLine("hello pipeline");
    log.appendLine("second line");

    std::ifstream ifs(path);
    REQUIRE(ifs.is_open());
    std::string content((std::istreambuf_iterator<char>(ifs)),
                         std::istreambuf_iterator<char>());
    REQUIRE(content.find("hello pipeline") != std::string::npos);
    REQUIRE(content.find("second line")    != std::string::npos);
}

TEST_CASE("WatchLog concurrent appends are thread-safe", "[Pipeline][WatchLog]") {
    auto dir  = makeTempDir("watchlog_threads");
    auto path = dir / "watch.log";
    NF::WatchLog log(path);

    constexpr int kThreads = 4;
    constexpr int kLines   = 20;

    std::vector<std::thread> threads;
    threads.reserve(kThreads);
    for (int t = 0; t < kThreads; ++t) {
        threads.emplace_back([&log, t] {
            for (int i = 0; i < kLines; ++i)
                log.appendLine("thread " + std::to_string(t) +
                                " line "  + std::to_string(i));
        });
    }
    for (auto& th : threads) th.join();

    std::ifstream ifs(path);
    REQUIRE(ifs.is_open());
    int lineCount = 0;
    std::string line;
    while (std::getline(ifs, line)) ++lineCount;
    REQUIRE(lineCount == kThreads * kLines);
}

// ── PipelineDirectories ───────────────────────────────────────────────────

TEST_CASE("PipelineDirectories fromRoot derives correct sub-paths", "[Pipeline][PipelineDirectories]") {
    fs::path root = "/workspace/mygame";
    auto d = NF::PipelineDirectories::fromRoot(root);

    REQUIRE(d.root                  == root);
    REQUIRE(d.dotAtlas              == root / ".atlas");
    REQUIRE(d.changes               == root / ".atlas" / "pipeline" / "changes");
    REQUIRE(d.assets                == root / ".atlas" / "pipeline" / "assets");
    REQUIRE(d.worlds                == root / ".atlas" / "pipeline" / "worlds");
    REQUIRE(d.scripts               == root / ".atlas" / "pipeline" / "scripts");
    REQUIRE(d.animations            == root / ".atlas" / "pipeline" / "animations");
    REQUIRE(d.sessions              == root / ".atlas" / "pipeline" / "sessions");
    REQUIRE(d.manifestFile          == root / ".atlas" / "manifest.json");
    REQUIRE(d.watchLogFile          == root / ".atlas" / "watch.log");
}

TEST_CASE("PipelineDirectories ensureCreated makes all directories", "[Pipeline][PipelineDirectories]") {
    auto tmpRoot = makeTempDir("pipedirs_ensure") / "workspace";
    auto d = NF::PipelineDirectories::fromRoot(tmpRoot);
    REQUIRE(d.ensureCreated());

    REQUIRE(fs::exists(d.changes));
    REQUIRE(fs::exists(d.assets));
    REQUIRE(fs::exists(d.worlds));
    REQUIRE(fs::exists(d.scripts));
    REQUIRE(fs::exists(d.animations));
    REQUIRE(fs::exists(d.sessions));
}

// ── PipelineWatcher ───────────────────────────────────────────────────────

TEST_CASE("PipelineWatcher poll detects new .change.json and fires callback",
          "[Pipeline][PipelineWatcher]") {
    auto watchDir = makeTempDir("watcher_poll");
    NF::PipelineWatcher watcher(watchDir);

    std::vector<NF::ChangeEvent> received;
    watcher.subscribe([&received](const NF::ChangeEvent& ev) {
        received.push_back(ev);
    });

    // No files yet — poll should return 0.
    REQUIRE(watcher.poll() == 0);
    REQUIRE(received.empty());

    // Write a .change.json file.
    NF::ChangeEvent ev;
    ev.tool      = "AtlasAI";
    ev.eventType = NF::ChangeEventType::WorldChanged;
    ev.path      = "worlds/sector02.json";
    ev.timestamp = 7777LL;
    REQUIRE(ev.writeToFile(watchDir));

    // Poll should pick it up.
    int found = watcher.poll();
    REQUIRE(found == 1);
    REQUIRE(received.size() == 1);
    REQUIRE(received[0].tool      == "AtlasAI");
    REQUIRE(received[0].eventType == NF::ChangeEventType::WorldChanged);

    // Polling again must not re-fire the same file.
    found = watcher.poll();
    REQUIRE(found == 0);
    REQUIRE(received.size() == 1);
}
