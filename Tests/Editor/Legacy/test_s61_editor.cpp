#include <catch2/catch_test_macros.hpp>
#include <catch2/catch_approx.hpp>
#include "NF/Editor/Editor.h"

using namespace NF;
using Catch::Approx;

// ── SourceFileType ────────────────────────────────────────────────

TEST_CASE("SourceFileType enum values exist", "[Editor][S61]") {
    REQUIRE(SourceFileType::Header  != SourceFileType::Source);
    REQUIRE(SourceFileType::Shader  != SourceFileType::Script);
    REQUIRE(SourceFileType::Data    != SourceFileType::Config);
    REQUIRE(SourceFileType::Unknown != SourceFileType::Header);
}

// ── ProjectIndexer — file type classification (via indexFile) ────

TEST_CASE("ProjectIndexer indexFile header extension stored correctly", "[Editor][S61]") {
    ProjectIndexer indexer;
    indexer.indexFile("/src/core.h",   SourceFileType::Header, "Core");
    indexer.indexFile("/src/utils.hpp", SourceFileType::Header, "Core");
    auto headers = indexer.findFilesByType(SourceFileType::Header);
    REQUIRE(headers.size() == 2);
}

TEST_CASE("ProjectIndexer indexFile source extension stored correctly", "[Editor][S61]") {
    ProjectIndexer indexer;
    indexer.indexFile("/src/main.cpp", SourceFileType::Source, "App");
    indexer.indexFile("/src/win.cc",   SourceFileType::Source, "App");
    indexer.indexFile("/src/unix.cxx", SourceFileType::Source, "App");
    auto sources = indexer.findFilesByType(SourceFileType::Source);
    REQUIRE(sources.size() == 3);
}

TEST_CASE("ProjectIndexer indexFile shader extension stored correctly", "[Editor][S61]") {
    ProjectIndexer indexer;
    indexer.indexFile("/shaders/base.glsl", SourceFileType::Shader, "Renderer");
    indexer.indexFile("/shaders/post.hlsl", SourceFileType::Shader, "Renderer");
    auto shaders = indexer.findFilesByType(SourceFileType::Shader);
    REQUIRE(shaders.size() == 2);
}

TEST_CASE("ProjectIndexer indexFile script extension stored correctly", "[Editor][S61]") {
    ProjectIndexer indexer;
    indexer.indexFile("/scripts/ai.lua", SourceFileType::Script, "Game");
    indexer.indexFile("/scripts/tool.py", SourceFileType::Script, "Tools");
    auto scripts = indexer.findFilesByType(SourceFileType::Script);
    REQUIRE(scripts.size() == 2);
}

TEST_CASE("ProjectIndexer indexFile data and config extensions", "[Editor][S61]") {
    ProjectIndexer indexer;
    indexer.indexFile("/config/settings.json", SourceFileType::Data, "Config");
    indexer.indexFile("/config/app.cfg",       SourceFileType::Config, "Config");
    indexer.indexFile("/config/input.ini",     SourceFileType::Config, "Config");
    REQUIRE(indexer.findFilesByType(SourceFileType::Data).size()   == 1);
    REQUIRE(indexer.findFilesByType(SourceFileType::Config).size() == 2);
}

TEST_CASE("ProjectIndexer indexFile unknown extension stored as Unknown", "[Editor][S61]") {
    ProjectIndexer indexer;
    indexer.indexFile("/misc/readme.xyz", SourceFileType::Unknown, "Misc");
    auto unknowns = indexer.findFilesByType(SourceFileType::Unknown);
    REQUIRE(unknowns.size() == 1);
}

// ── ProjectIndexer ────────────────────────────────────────────────

TEST_CASE("ProjectIndexer starts with no indexed files", "[Editor][S61]") {
    ProjectIndexer indexer;
    REQUIRE(indexer.fileCount() == 0);
}

TEST_CASE("ProjectIndexer indexFile adds a file", "[Editor][S61]") {
    ProjectIndexer indexer;
    indexer.indexFile("/src/main.cpp", SourceFileType::Source, "Engine");
    REQUIRE(indexer.fileCount() == 1);
}

TEST_CASE("ProjectIndexer findFilesByType filters correctly", "[Editor][S61]") {
    ProjectIndexer indexer;
    indexer.indexFile("/src/a.cpp",     SourceFileType::Source, "Engine");
    indexer.indexFile("/src/b.h",       SourceFileType::Header, "Engine");
    indexer.indexFile("/src/c.glsl",    SourceFileType::Shader, "Renderer");
    auto headers = indexer.findFilesByType(SourceFileType::Header);
    REQUIRE(headers.size() == 1);
    REQUIRE(headers[0]->path == "/src/b.h");
}

TEST_CASE("ProjectIndexer findFilesByModule filters correctly", "[Editor][S61]") {
    ProjectIndexer indexer;
    indexer.indexFile("/src/a.cpp",  SourceFileType::Source, "Engine");
    indexer.indexFile("/src/b.cpp",  SourceFileType::Source, "Physics");
    indexer.indexFile("/src/c.h",    SourceFileType::Header, "Engine");
    auto engine = indexer.findFilesByModule("Engine");
    REQUIRE(engine.size() == 2);
}

TEST_CASE("ProjectIndexer indexDirectory non-existent path is safe", "[Editor][S61]") {
    ProjectIndexer indexer;
    indexer.indexDirectory("/nonexistent_path_xyz_12345");
    REQUIRE(indexer.fileCount() == 0);
}

// ── SymbolKind ───────────────────────────────────────────────────

TEST_CASE("SymbolKind enum values exist", "[Editor][S61]") {
    REQUIRE(SymbolKind::Function  != SymbolKind::Class);
    REQUIRE(SymbolKind::Struct    != SymbolKind::Enum);
    REQUIRE(SymbolKind::Variable  != SymbolKind::Namespace);
    REQUIRE(SymbolKind::Macro     != SymbolKind::Type);
    REQUIRE(SymbolKind::Unknown   != SymbolKind::Function);
}

// ── GizmoMode + GizmoAxis ─────────────────────────────────────────

TEST_CASE("GizmoMode enum values exist", "[Editor][S61]") {
    REQUIRE(GizmoMode::Translate != GizmoMode::Rotate);
    REQUIRE(GizmoMode::Rotate    != GizmoMode::Scale);
    REQUIRE(GizmoMode::Translate != GizmoMode::Scale);
}

TEST_CASE("GizmoAxis enum values exist", "[Editor][S61]") {
    REQUIRE(GizmoAxis::None != GizmoAxis::X);
    REQUIRE(GizmoAxis::X    != GizmoAxis::Y);
    REQUIRE(GizmoAxis::Y    != GizmoAxis::Z);
    REQUIRE(GizmoAxis::XY   != GizmoAxis::YZ);
    REQUIRE(GizmoAxis::XZ   != GizmoAxis::XYZ);
}

// ── GizmoState ───────────────────────────────────────────────────

TEST_CASE("GizmoState defaults", "[Editor][S61]") {
    GizmoState gs;
    REQUIRE(gs.mode       == GizmoMode::Translate);
    REQUIRE(gs.activeAxis == GizmoAxis::None);
    REQUIRE_FALSE(gs.isDragging);
    REQUIRE_FALSE(gs.snapEnabled);
    REQUIRE(gs.snapValue  == Approx(0.25f));
}

TEST_CASE("GizmoState activate sets axis and dragging", "[Editor][S61]") {
    GizmoState gs;
    gs.activate(GizmoAxis::X);
    REQUIRE(gs.activeAxis  == GizmoAxis::X);
    REQUIRE(gs.isDragging);
}

TEST_CASE("GizmoState deactivate clears axis and dragging", "[Editor][S61]") {
    GizmoState gs;
    gs.activate(GizmoAxis::Y);
    gs.deactivate();
    REQUIRE(gs.activeAxis == GizmoAxis::None);
    REQUIRE_FALSE(gs.isDragging);
}

TEST_CASE("GizmoState setMode changes mode and deactivates", "[Editor][S61]") {
    GizmoState gs;
    gs.activate(GizmoAxis::Z);
    gs.setMode(GizmoMode::Rotate);
    REQUIRE(gs.mode == GizmoMode::Rotate);
    REQUIRE(gs.activeAxis == GizmoAxis::None);
    REQUIRE_FALSE(gs.isDragging);
}

// ── EditorCameraOrbit ─────────────────────────────────────────────

TEST_CASE("EditorCameraOrbit defaults", "[Editor][S61]") {
    EditorCameraOrbit cam;
    REQUIRE(cam.distance  == Approx(10.f));
    REQUIRE(cam.yaw       == Approx(-90.f));
    REQUIRE(cam.pitch     == Approx(30.f));
    REQUIRE(cam.fovDeg    == Approx(60.f));
    REQUIRE(cam.nearPlane == Approx(0.1f));
    REQUIRE(cam.farPlane  == Approx(1000.f));
}

TEST_CASE("EditorCameraOrbit orbit changes yaw and pitch", "[Editor][S61]") {
    EditorCameraOrbit cam;
    float oldYaw = cam.yaw;
    float oldPitch = cam.pitch;
    cam.orbit(10.f, 5.f);
    REQUIRE(cam.yaw   == Approx(oldYaw + 10.f));
    REQUIRE(cam.pitch == Approx(oldPitch + 5.f));
}

TEST_CASE("EditorCameraOrbit pitch is clamped to [-89, 89]", "[Editor][S61]") {
    EditorCameraOrbit cam;
    cam.orbit(0.f, 200.f);
    REQUIRE(cam.pitch <= 89.f);
    cam.orbit(0.f, -400.f);
    REQUIRE(cam.pitch >= -89.f);
}

TEST_CASE("EditorCameraOrbit zoom decreases distance", "[Editor][S61]") {
    EditorCameraOrbit cam;
    float d0 = cam.distance;
    cam.zoom(3.f);
    REQUIRE(cam.distance == Approx(d0 - 3.f));
}

TEST_CASE("EditorCameraOrbit zoom is clamped to minimum 0.5", "[Editor][S61]") {
    EditorCameraOrbit cam;
    cam.zoom(9999.f);
    REQUIRE(cam.distance >= 0.5f);
}

TEST_CASE("EditorCameraOrbit computePosition returns non-zero vector", "[Editor][S61]") {
    EditorCameraOrbit cam;
    Vec3 pos = cam.computePosition();
    float len = std::sqrt(pos.x*pos.x + pos.y*pos.y + pos.z*pos.z);
    REQUIRE(len > 0.f);
}

TEST_CASE("EditorCameraOrbit buildCamera returns camera with correct fov", "[Editor][S61]") {
    EditorCameraOrbit cam;
    cam.fovDeg = 75.f;
    Camera c = cam.buildCamera(16.f / 9.f);
    REQUIRE(c.fov == Approx(75.f));
}

// ── SnapSettings ─────────────────────────────────────────────────

TEST_CASE("SnapSettings defaults", "[Editor][S61]") {
    SnapSettings s;
    REQUIRE(s.gridSize  == Approx(0.25f));
    REQUIRE(s.angleStep == Approx(15.f));
    REQUIRE(s.scaleStep == Approx(0.1f));
    REQUIRE_FALSE(s.enabled);
}
