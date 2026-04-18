#include <catch2/catch_test_macros.hpp>
#include "NF/Editor/Editor.h"

using namespace NF;

// ── S16 Tests: Hot-Reload System ─────────────────────────────────────────────

TEST_CASE("HotReloadAssetType names", "[Editor][S16]") {
    REQUIRE(std::string(hotReloadAssetTypeName(HotReloadAssetType::Script))   == "Script");
    REQUIRE(std::string(hotReloadAssetTypeName(HotReloadAssetType::Shader))   == "Shader");
    REQUIRE(std::string(hotReloadAssetTypeName(HotReloadAssetType::Texture))  == "Texture");
    REQUIRE(std::string(hotReloadAssetTypeName(HotReloadAssetType::Mesh))     == "Mesh");
    REQUIRE(std::string(hotReloadAssetTypeName(HotReloadAssetType::Audio))    == "Audio");
    REQUIRE(std::string(hotReloadAssetTypeName(HotReloadAssetType::Config))   == "Config");
    REQUIRE(std::string(hotReloadAssetTypeName(HotReloadAssetType::Level))    == "Level");
    REQUIRE(std::string(hotReloadAssetTypeName(HotReloadAssetType::Material)) == "Material");
}

TEST_CASE("HotReloadEntry state helpers", "[Editor][S16]") {
    HotReloadEntry e;
    e.assetPath = "shaders/main.glsl";
    e.assetType = HotReloadAssetType::Shader;

    REQUIRE_FALSE(e.isPending());
    REQUIRE_FALSE(e.isReloading());
    REQUIRE_FALSE(e.hasError());
    REQUIRE_FALSE(e.isSuccess());

    e.markPending();
    REQUIRE(e.isPending());

    e.markSuccess();
    REQUIRE(e.isSuccess());
    REQUIRE(e.reloadCount == 1);
    REQUIRE_FALSE(e.hasError());
}

TEST_CASE("HotReloadEntry markFailed", "[Editor][S16]") {
    HotReloadEntry e;
    e.assetPath = "scripts/bad.lua";
    e.markFailed("parse error");
    REQUIRE(e.hasError());
    REQUIRE(e.errorMessage == "parse error");
    REQUIRE_FALSE(e.isSuccess());
}

TEST_CASE("HotReloadEntry markPending clears error", "[Editor][S16]") {
    HotReloadEntry e;
    e.markFailed("oops");
    REQUIRE(e.hasError());
    e.markPending();
    REQUIRE(e.errorMessage.empty());
    REQUIRE(e.isPending());
}

TEST_CASE("HotReloadWatcher watch + duplicate rejection", "[Editor][S16]") {
    HotReloadWatcher w;
    REQUIRE(w.watch("assets/tex.png", HotReloadAssetType::Texture));
    REQUIRE(w.entryCount() == 1);
    REQUIRE_FALSE(w.watch("assets/tex.png", HotReloadAssetType::Texture)); // duplicate
    REQUIRE(w.entryCount() == 1);
}

TEST_CASE("HotReloadWatcher unwatch", "[Editor][S16]") {
    HotReloadWatcher w;
    w.watch("mesh.obj", HotReloadAssetType::Mesh);
    REQUIRE(w.unwatch("mesh.obj"));
    REQUIRE(w.entryCount() == 0);
    REQUIRE_FALSE(w.unwatch("nonexistent"));
}

TEST_CASE("HotReloadWatcher findEntry", "[Editor][S16]") {
    HotReloadWatcher w;
    w.watch("audio/sfx.wav", HotReloadAssetType::Audio);
    REQUIRE(w.findEntry("audio/sfx.wav") != nullptr);
    REQUIRE(w.findEntry("audio/sfx.wav")->assetType == HotReloadAssetType::Audio);
    REQUIRE(w.findEntry("missing") == nullptr);
}

TEST_CASE("HotReloadWatcher triggerReload + pendingCount", "[Editor][S16]") {
    HotReloadWatcher w;
    w.watch("cfg/settings.json", HotReloadAssetType::Config);
    w.watch("shaders/lit.glsl", HotReloadAssetType::Shader);

    REQUIRE(w.triggerReload("cfg/settings.json"));
    REQUIRE(w.pendingCount() == 1);

    REQUIRE_FALSE(w.triggerReload("nonexistent"));
}

TEST_CASE("HotReloadDispatcher dispatchPending", "[Editor][S16]") {
    HotReloadWatcher w;
    w.watch("s1.lua", HotReloadAssetType::Script);
    w.watch("s2.lua", HotReloadAssetType::Script);
    w.triggerReload("s1.lua");
    w.triggerReload("s2.lua");
    REQUIRE(w.pendingCount() == 2);

    HotReloadDispatcher d;
    size_t dispatched = d.dispatchPending(w);
    REQUIRE(dispatched == 2);
    REQUIRE(d.totalDispatched() == 2);
    REQUIRE(w.pendingCount() == 0);

    // Both entries should be success now
    REQUIRE(w.findEntry("s1.lua")->isSuccess());
    REQUIRE(w.findEntry("s2.lua")->isSuccess());
    REQUIRE(w.findEntry("s1.lua")->reloadCount == 1);
}

TEST_CASE("HotReloadSystem init/shutdown", "[Editor][S16]") {
    HotReloadSystem sys;
    REQUIRE_FALSE(sys.isInitialized());
    sys.init();
    REQUIRE(sys.isInitialized());
    sys.shutdown();
    REQUIRE_FALSE(sys.isInitialized());
}

TEST_CASE("HotReloadSystem watch + triggerReload + tick dispatches", "[Editor][S16]") {
    HotReloadSystem sys;
    sys.init();

    REQUIRE(sys.watch("main.lua", HotReloadAssetType::Script));
    REQUIRE(sys.watchedCount() == 1);

    REQUIRE(sys.triggerReload("main.lua"));
    REQUIRE(sys.pendingCount() == 1);

    sys.tick(0.016f);
    REQUIRE(sys.pendingCount() == 0);
    REQUIRE(sys.totalDispatched() == 1);
    REQUIRE(sys.findEntry("main.lua")->isSuccess());
}

TEST_CASE("HotReloadSystem watch before init returns false", "[Editor][S16]") {
    HotReloadSystem sys;
    REQUIRE_FALSE(sys.watch("x.lua", HotReloadAssetType::Script));
}

TEST_CASE("HotReloadSystem unwatch", "[Editor][S16]") {
    HotReloadSystem sys;
    sys.init();
    sys.watch("level.json", HotReloadAssetType::Level);
    REQUIRE(sys.watchedCount() == 1);
    REQUIRE(sys.unwatch("level.json"));
    REQUIRE(sys.watchedCount() == 0);
}

TEST_CASE("HotReloadSystem multiple reloads increment reloadCount", "[Editor][S16]") {
    HotReloadSystem sys;
    sys.init();
    sys.watch("mat.glsl", HotReloadAssetType::Material);

    sys.triggerReload("mat.glsl");
    sys.tick(0.016f);

    sys.triggerReload("mat.glsl");
    sys.tick(0.016f);

    REQUIRE(sys.findEntry("mat.glsl")->reloadCount == 2);
    REQUIRE(sys.totalDispatched() == 2);
    REQUIRE(sys.tickCount() == 2);
}
