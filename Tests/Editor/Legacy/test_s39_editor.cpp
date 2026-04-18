#include <catch2/catch_test_macros.hpp>
#include <catch2/catch_approx.hpp>
#include "NF/Editor/Editor.h"
#include "NF/Editor/TilemapEditor.h"

using namespace NF;

TEST_CASE("TileFlipMode names cover all 5 values", "[Editor][S39]") {
    REQUIRE(std::string(tileFlipModeName(TileFlipMode::None))       == "None");
    REQUIRE(std::string(tileFlipModeName(TileFlipMode::Horizontal)) == "Horizontal");
    REQUIRE(std::string(tileFlipModeName(TileFlipMode::Vertical))   == "Vertical");
    REQUIRE(std::string(tileFlipModeName(TileFlipMode::Both))       == "Both");
    REQUIRE(std::string(tileFlipModeName(TileFlipMode::Rotate90))   == "Rotate90");
}

TEST_CASE("TileLayerType names cover all 5 values", "[Editor][S39]") {
    REQUIRE(std::string(tileLayerTypeName(TileLayerType::Background)) == "Background");
    REQUIRE(std::string(tileLayerTypeName(TileLayerType::Midground))  == "Midground");
    REQUIRE(std::string(tileLayerTypeName(TileLayerType::Foreground)) == "Foreground");
    REQUIRE(std::string(tileLayerTypeName(TileLayerType::Object))     == "Object");
    REQUIRE(std::string(tileLayerTypeName(TileLayerType::Collision))  == "Collision");
}

TEST_CASE("TileAnimMode names cover all 5 values", "[Editor][S39]") {
    REQUIRE(std::string(tileAnimModeName(TileAnimMode::Static))   == "Static");
    REQUIRE(std::string(tileAnimModeName(TileAnimMode::Loop))     == "Loop");
    REQUIRE(std::string(tileAnimModeName(TileAnimMode::PingPong)) == "PingPong");
    REQUIRE(std::string(tileAnimModeName(TileAnimMode::Once))     == "Once");
    REQUIRE(std::string(tileAnimModeName(TileAnimMode::Random))   == "Random");
}

TEST_CASE("TileAsset default values", "[Editor][S39]") {
    TileAsset t("grass");
    REQUIRE(t.name()        == "grass");
    REQUIRE(t.tileWidth()   == 16u);
    REQUIRE(t.tileHeight()  == 16u);
    REQUIRE(t.tileId()      == 0u);
    REQUIRE(t.tilesetName() == "");
    REQUIRE(t.flipMode()    == TileFlipMode::None);
    REQUIRE(t.layerType()   == TileLayerType::Background);
    REQUIRE(t.animMode()    == TileAnimMode::Static);
    REQUIRE_FALSE(t.isAnimated());
    REQUIRE_FALSE(t.hasCollider());
    REQUIRE_FALSE(t.isDirty());
    REQUIRE(t.area() == 16u * 16u);
}

TEST_CASE("TileAsset setters round-trip", "[Editor][S39]") {
    TileAsset t("wall", 32, 32);
    t.setTilesetName("dungeon");
    t.setTileId(42);
    t.setFlipMode(TileFlipMode::Horizontal);
    t.setLayerType(TileLayerType::Collision);
    t.setAnimMode(TileAnimMode::Loop);
    t.setAnimated(true);
    t.setCollider(true);
    t.setDirty(true);

    REQUIRE(t.tilesetName() == "dungeon");
    REQUIRE(t.tileId()      == 42u);
    REQUIRE(t.flipMode()    == TileFlipMode::Horizontal);
    REQUIRE(t.layerType()   == TileLayerType::Collision);
    REQUIRE(t.animMode()    == TileAnimMode::Loop);
    REQUIRE(t.isAnimated());
    REQUIRE(t.hasCollider());
    REQUIRE(t.isDirty());
    REQUIRE(t.area() == 32u * 32u);
}

TEST_CASE("TileAsset area calculation with custom dimensions", "[Editor][S39]") {
    TileAsset t("bg", 64, 48);
    REQUIRE(t.area() == 64u * 48u);
}

TEST_CASE("TilemapEditor addTile and duplicate rejection", "[Editor][S39]") {
    TilemapEditor editor;
    TileAsset a("a"), b("b"), dup("a");
    REQUIRE(editor.addTile(a));
    REQUIRE(editor.addTile(b));
    REQUIRE_FALSE(editor.addTile(dup));
    REQUIRE(editor.tileCount() == 2);
}

TEST_CASE("TilemapEditor removeTile clears activeTile", "[Editor][S39]") {
    TilemapEditor editor;
    TileAsset t("rock");
    editor.addTile(t);
    editor.setActiveTile("rock");
    REQUIRE(editor.activeTile() == "rock");

    editor.removeTile("rock");
    REQUIRE(editor.tileCount() == 0);
    REQUIRE(editor.activeTile().empty());
}

TEST_CASE("TilemapEditor findTile returns pointer or nullptr", "[Editor][S39]") {
    TilemapEditor editor;
    TileAsset t("water");
    editor.addTile(t);

    REQUIRE(editor.findTile("water") != nullptr);
    REQUIRE(editor.findTile("water")->name() == "water");
    REQUIRE(editor.findTile("missing") == nullptr);
}

TEST_CASE("TilemapEditor setActiveTile returns false for missing", "[Editor][S39]") {
    TilemapEditor editor;
    REQUIRE_FALSE(editor.setActiveTile("ghost"));
    REQUIRE(editor.activeTile().empty());
}

TEST_CASE("TilemapEditor dirtyCount, animatedCount, colliderCount", "[Editor][S39]") {
    TilemapEditor editor;

    TileAsset a("a"); a.setDirty(true); a.setAnimated(true); a.setCollider(true);
    TileAsset b("b"); b.setDirty(true);
    TileAsset c("c"); c.setAnimated(true);

    editor.addTile(a); editor.addTile(b); editor.addTile(c);

    REQUIRE(editor.dirtyCount()    == 2);
    REQUIRE(editor.animatedCount() == 2);
    REQUIRE(editor.colliderCount() == 1);
}

TEST_CASE("TilemapEditor countByLayerType and countByFlipMode", "[Editor][S39]") {
    TilemapEditor editor;

    TileAsset a("a"); a.setLayerType(TileLayerType::Foreground); a.setFlipMode(TileFlipMode::Horizontal);
    TileAsset b("b"); b.setLayerType(TileLayerType::Foreground); b.setFlipMode(TileFlipMode::None);
    TileAsset c("c"); c.setLayerType(TileLayerType::Background); c.setFlipMode(TileFlipMode::Horizontal);

    editor.addTile(a); editor.addTile(b); editor.addTile(c);

    REQUIRE(editor.countByLayerType(TileLayerType::Foreground) == 2);
    REQUIRE(editor.countByLayerType(TileLayerType::Background) == 1);
    REQUIRE(editor.countByFlipMode(TileFlipMode::Horizontal)   == 2);
    REQUIRE(editor.countByFlipMode(TileFlipMode::None)         == 1);
}

TEST_CASE("TilemapEditor MAX_TILES limit enforced", "[Editor][S39]") {
    TilemapEditor editor;
    for (size_t i = 0; i < TilemapEditor::MAX_TILES; ++i) {
        TileAsset t("Tile" + std::to_string(i));
        REQUIRE(editor.addTile(t));
    }
    TileAsset overflow("Overflow");
    REQUIRE_FALSE(editor.addTile(overflow));
    REQUIRE(editor.tileCount() == TilemapEditor::MAX_TILES);
}
