#include <catch2/catch_test_macros.hpp>
#include "NF/Editor/Editor.h"

using namespace NF;

// ── S73: HotReload ───────────────────────────────────────────────

TEST_CASE("HotReloadAssetType names are correct", "[Editor][S73]") {
    REQUIRE(std::string(hotReloadAssetTypeName(HotReloadAssetType::Script))   == "Script");
    REQUIRE(std::string(hotReloadAssetTypeName(HotReloadAssetType::Shader))   == "Shader");
    REQUIRE(std::string(hotReloadAssetTypeName(HotReloadAssetType::Texture))  == "Texture");
    REQUIRE(std::string(hotReloadAssetTypeName(HotReloadAssetType::Mesh))     == "Mesh");
    REQUIRE(std::string(hotReloadAssetTypeName(HotReloadAssetType::Audio))    == "Audio");
    REQUIRE(std::string(hotReloadAssetTypeName(HotReloadAssetType::Config))   == "Config");
    REQUIRE(std::string(hotReloadAssetTypeName(HotReloadAssetType::Level))    == "Level");
    REQUIRE(std::string(hotReloadAssetTypeName(HotReloadAssetType::Material)) == "Material");
}

TEST_CASE("HotReloadEntry default status is Idle", "[Editor][S73]") {
    HotReloadEntry e;
    REQUIRE(e.status == HotReloadStatus::Idle);
    REQUIRE_FALSE(e.isPending());
    REQUIRE_FALSE(e.isReloading());
    REQUIRE_FALSE(e.hasError());
    REQUIRE_FALSE(e.isSuccess());
}

TEST_CASE("HotReloadEntry markPending sets Pending status", "[Editor][S73]") {
    HotReloadEntry e;
    e.markPending();
    REQUIRE(e.isPending());
    REQUIRE(e.errorMessage.empty());
}

TEST_CASE("HotReloadEntry markSuccess sets Success and increments reloadCount", "[Editor][S73]") {
    HotReloadEntry e;
    e.markSuccess();
    REQUIRE(e.isSuccess());
    REQUIRE(e.reloadCount == 1);
    REQUIRE(e.errorMessage.empty());
}

TEST_CASE("HotReloadEntry markFailed sets Failed with message", "[Editor][S73]") {
    HotReloadEntry e;
    e.markFailed("compile error");
    REQUIRE(e.hasError());
    REQUIRE(e.errorMessage == "compile error");
}

TEST_CASE("HotReloadEntry markPending clears error message", "[Editor][S73]") {
    HotReloadEntry e;
    e.markFailed("error");
    e.markPending();
    REQUIRE(e.errorMessage.empty());
}

TEST_CASE("HotReloadWatcher starts empty", "[Editor][S73]") {
    HotReloadWatcher w;
    REQUIRE(w.entryCount() == 0);
    REQUIRE(w.pendingCount() == 0);
}

TEST_CASE("HotReloadWatcher watch adds entry", "[Editor][S73]") {
    HotReloadWatcher w;
    REQUIRE(w.watch("main.lua", HotReloadAssetType::Script));
    REQUIRE(w.entryCount() == 1);
}

TEST_CASE("HotReloadWatcher watch rejects duplicate path", "[Editor][S73]") {
    HotReloadWatcher w;
    w.watch("main.lua", HotReloadAssetType::Script);
    REQUIRE_FALSE(w.watch("main.lua", HotReloadAssetType::Script));
    REQUIRE(w.entryCount() == 1);
}

TEST_CASE("HotReloadWatcher unwatch removes entry", "[Editor][S73]") {
    HotReloadWatcher w;
    w.watch("main.lua", HotReloadAssetType::Script);
    REQUIRE(w.unwatch("main.lua"));
    REQUIRE(w.entryCount() == 0);
}

TEST_CASE("HotReloadWatcher unwatch returns false for missing", "[Editor][S73]") {
    HotReloadWatcher w;
    REQUIRE_FALSE(w.unwatch("ghost.lua"));
}

TEST_CASE("HotReloadWatcher findEntry returns entry", "[Editor][S73]") {
    HotReloadWatcher w;
    w.watch("fx.glsl", HotReloadAssetType::Shader);
    REQUIRE(w.findEntry("fx.glsl") != nullptr);
    REQUIRE(w.findEntry("other") == nullptr);
}

TEST_CASE("HotReloadWatcher triggerReload marks entry Pending", "[Editor][S73]") {
    HotReloadWatcher w;
    w.watch("model.fbx", HotReloadAssetType::Mesh);
    REQUIRE(w.triggerReload("model.fbx"));
    REQUIRE(w.pendingCount() == 1);
    REQUIRE(w.findEntry("model.fbx")->isPending());
}

TEST_CASE("HotReloadWatcher triggerReload fails for missing path", "[Editor][S73]") {
    HotReloadWatcher w;
    REQUIRE_FALSE(w.triggerReload("ghost.fbx"));
}

TEST_CASE("HotReloadWatcher kMaxEntries is 256", "[Editor][S73]") {
    REQUIRE(HotReloadWatcher::kMaxEntries == 256);
}

TEST_CASE("HotReloadDispatcher dispatchPending processes pending entries", "[Editor][S73]") {
    HotReloadWatcher w;
    w.watch("a.lua", HotReloadAssetType::Script);
    w.watch("b.lua", HotReloadAssetType::Script);
    w.triggerReload("a.lua");
    HotReloadDispatcher d;
    size_t count = d.dispatchPending(w);
    REQUIRE(count == 1);
    REQUIRE(d.totalDispatched() == 1);
}

TEST_CASE("HotReloadDispatcher dispatchPending marks entry Success", "[Editor][S73]") {
    HotReloadWatcher w;
    w.watch("shader.glsl", HotReloadAssetType::Shader);
    w.triggerReload("shader.glsl");
    HotReloadDispatcher d;
    d.dispatchPending(w);
    REQUIRE(w.findEntry("shader.glsl")->isSuccess());
}

TEST_CASE("HotReloadDispatcher dispatchPending returns 0 when nothing pending", "[Editor][S73]") {
    HotReloadWatcher w;
    w.watch("a.lua", HotReloadAssetType::Script);
    HotReloadDispatcher d;
    REQUIRE(d.dispatchPending(w) == 0);
}

TEST_CASE("HotReloadSystem starts uninitialized", "[Editor][S73]") {
    HotReloadSystem sys;
    REQUIRE_FALSE(sys.isInitialized());
    REQUIRE(sys.watchedCount() == 0);
}

TEST_CASE("HotReloadSystem init sets initialized", "[Editor][S73]") {
    HotReloadSystem sys;
    sys.init();
    REQUIRE(sys.isInitialized());
}

TEST_CASE("HotReloadSystem watch before init returns false", "[Editor][S73]") {
    HotReloadSystem sys;
    REQUIRE_FALSE(sys.watch("a.lua", HotReloadAssetType::Script));
}

TEST_CASE("HotReloadSystem watch after init succeeds", "[Editor][S73]") {
    HotReloadSystem sys;
    sys.init();
    REQUIRE(sys.watch("main.lua", HotReloadAssetType::Script));
    REQUIRE(sys.watchedCount() == 1);
}

TEST_CASE("HotReloadSystem unwatch removes entry", "[Editor][S73]") {
    HotReloadSystem sys;
    sys.init();
    sys.watch("a.lua", HotReloadAssetType::Script);
    REQUIRE(sys.unwatch("a.lua"));
    REQUIRE(sys.watchedCount() == 0);
}

TEST_CASE("HotReloadSystem triggerReload marks pending", "[Editor][S73]") {
    HotReloadSystem sys;
    sys.init();
    sys.watch("fx.glsl", HotReloadAssetType::Shader);
    REQUIRE(sys.triggerReload("fx.glsl"));
    REQUIRE(sys.pendingCount() == 1);
}

TEST_CASE("HotReloadSystem tick dispatches pending and increments tickCount", "[Editor][S73]") {
    HotReloadSystem sys;
    sys.init();
    sys.watch("a.lua", HotReloadAssetType::Script);
    sys.triggerReload("a.lua");
    sys.tick(0.016f);
    REQUIRE(sys.tickCount() == 1);
    REQUIRE(sys.totalDispatched() == 1);
    REQUIRE(sys.pendingCount() == 0);
}

TEST_CASE("HotReloadSystem tick does nothing when uninitialized", "[Editor][S73]") {
    HotReloadSystem sys;
    sys.tick(0.016f);
    REQUIRE(sys.tickCount() == 0);
}

TEST_CASE("HotReloadSystem findEntry returns entry after watch", "[Editor][S73]") {
    HotReloadSystem sys;
    sys.init();
    sys.watch("cfg.json", HotReloadAssetType::Config);
    REQUIRE(sys.findEntry("cfg.json") != nullptr);
}

TEST_CASE("HotReloadSystem findEntry returns nullptr for missing", "[Editor][S73]") {
    HotReloadSystem sys;
    sys.init();
    REQUIRE(sys.findEntry("ghost") == nullptr);
}

TEST_CASE("HotReloadSystem shutdown clears state", "[Editor][S73]") {
    HotReloadSystem sys;
    sys.init();
    sys.watch("a.lua", HotReloadAssetType::Script);
    sys.shutdown();
    REQUIRE_FALSE(sys.isInitialized());
    REQUIRE(sys.watchedCount() == 0);
    REQUIRE(sys.tickCount() == 0);
}
