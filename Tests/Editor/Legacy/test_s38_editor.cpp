#include <catch2/catch_test_macros.hpp>
#include <catch2/catch_approx.hpp>
#include "NF/Editor/Editor.h"
#include "NF/Editor/SpriteEditor.h"

using namespace NF;

TEST_CASE("SpriteOrigin names cover all 5 values", "[Editor][S38]") {
    REQUIRE(std::string(spriteOriginName(SpriteOrigin::TopLeft))      == "TopLeft");
    REQUIRE(std::string(spriteOriginName(SpriteOrigin::TopCenter))    == "TopCenter");
    REQUIRE(std::string(spriteOriginName(SpriteOrigin::Center))       == "Center");
    REQUIRE(std::string(spriteOriginName(SpriteOrigin::BottomLeft))   == "BottomLeft");
    REQUIRE(std::string(spriteOriginName(SpriteOrigin::BottomCenter)) == "BottomCenter");
}

TEST_CASE("SpriteBlendMode names cover all 5 values", "[Editor][S38]") {
    REQUIRE(std::string(spriteBlendModeName(SpriteBlendMode::Normal))   == "Normal");
    REQUIRE(std::string(spriteBlendModeName(SpriteBlendMode::Additive)) == "Additive");
    REQUIRE(std::string(spriteBlendModeName(SpriteBlendMode::Multiply)) == "Multiply");
    REQUIRE(std::string(spriteBlendModeName(SpriteBlendMode::Screen))   == "Screen");
    REQUIRE(std::string(spriteBlendModeName(SpriteBlendMode::Overlay))  == "Overlay");
}

TEST_CASE("SpriteAnimState names cover all 5 values", "[Editor][S38]") {
    REQUIRE(std::string(spriteAnimStateName(SpriteAnimState::Idle))     == "Idle");
    REQUIRE(std::string(spriteAnimStateName(SpriteAnimState::Playing))  == "Playing");
    REQUIRE(std::string(spriteAnimStateName(SpriteAnimState::Paused))   == "Paused");
    REQUIRE(std::string(spriteAnimStateName(SpriteAnimState::Stopped))  == "Stopped");
    REQUIRE(std::string(spriteAnimStateName(SpriteAnimState::Finished)) == "Finished");
}

TEST_CASE("SpriteAsset default values", "[Editor][S38]") {
    SpriteAsset s("hero");
    REQUIRE(s.name()        == "hero");
    REQUIRE(s.width()       == 32u);
    REQUIRE(s.height()      == 32u);
    REQUIRE(s.origin()      == SpriteOrigin::Center);
    REQUIRE(s.blendMode()   == SpriteBlendMode::Normal);
    REQUIRE(s.animState()   == SpriteAnimState::Idle);
    REQUIRE(s.frameCount()  == 1u);
    REQUIRE(s.frameRate()   == Catch::Approx(24.0f));
    REQUIRE_FALSE(s.isLooping());
    REQUIRE_FALSE(s.isFlippedH());
    REQUIRE_FALSE(s.isFlippedV());
    REQUIRE_FALSE(s.isDirty());
    REQUIRE_FALSE(s.isAnimated());
    REQUIRE(s.area() == 32u * 32u);
}

TEST_CASE("SpriteAsset setters round-trip", "[Editor][S38]") {
    SpriteAsset s("enemy", 64, 128);
    s.setOrigin(SpriteOrigin::BottomCenter);
    s.setBlendMode(SpriteBlendMode::Additive);
    s.setAnimState(SpriteAnimState::Playing);
    s.setFrameCount(8);
    s.setFrameRate(30.0f);
    s.setLooping(true);
    s.setFlippedH(true);
    s.setFlippedV(false);
    s.setDirty(true);

    REQUIRE(s.origin()     == SpriteOrigin::BottomCenter);
    REQUIRE(s.blendMode()  == SpriteBlendMode::Additive);
    REQUIRE(s.animState()  == SpriteAnimState::Playing);
    REQUIRE(s.frameCount() == 8u);
    REQUIRE(s.frameRate()  == Catch::Approx(30.0f));
    REQUIRE(s.isLooping());
    REQUIRE(s.isFlippedH());
    REQUIRE_FALSE(s.isFlippedV());
    REQUIRE(s.isDirty());
    REQUIRE(s.isAnimated());
    REQUIRE(s.isPlaying());
    REQUIRE_FALSE(s.isPaused());
    REQUIRE_FALSE(s.isFinished());
    REQUIRE(s.area() == 64u * 128u);
}

TEST_CASE("SpriteAsset isAnimated requires frameCount > 1", "[Editor][S38]") {
    SpriteAsset s("tile");
    REQUIRE_FALSE(s.isAnimated());
    s.setFrameCount(2);
    REQUIRE(s.isAnimated());
}

TEST_CASE("SpriteAsset isPaused and isFinished states", "[Editor][S38]") {
    SpriteAsset s("coin");
    s.setAnimState(SpriteAnimState::Paused);
    REQUIRE(s.isPaused());
    REQUIRE_FALSE(s.isPlaying());

    s.setAnimState(SpriteAnimState::Finished);
    REQUIRE(s.isFinished());
    REQUIRE_FALSE(s.isPaused());
}

TEST_CASE("SpriteEditor addSprite and duplicate rejection", "[Editor][S38]") {
    SpriteEditor editor;
    SpriteAsset a("a"), b("b"), dup("a");
    REQUIRE(editor.addSprite(a));
    REQUIRE(editor.addSprite(b));
    REQUIRE_FALSE(editor.addSprite(dup));
    REQUIRE(editor.spriteCount() == 2);
}

TEST_CASE("SpriteEditor removeSprite clears activeSprite", "[Editor][S38]") {
    SpriteEditor editor;
    SpriteAsset s("hero");
    editor.addSprite(s);
    editor.setActiveSprite("hero");
    REQUIRE(editor.activeSprite() == "hero");

    editor.removeSprite("hero");
    REQUIRE(editor.spriteCount() == 0);
    REQUIRE(editor.activeSprite().empty());
}

TEST_CASE("SpriteEditor findSprite returns pointer or nullptr", "[Editor][S38]") {
    SpriteEditor editor;
    SpriteAsset s("gem");
    editor.addSprite(s);

    REQUIRE(editor.findSprite("gem") != nullptr);
    REQUIRE(editor.findSprite("gem")->name() == "gem");
    REQUIRE(editor.findSprite("missing") == nullptr);
}

TEST_CASE("SpriteEditor dirtyCount, animatedCount, playingCount, loopingCount", "[Editor][S38]") {
    SpriteEditor editor;

    SpriteAsset a("a"); a.setDirty(true); a.setFrameCount(4); a.setAnimState(SpriteAnimState::Playing); a.setLooping(true);
    SpriteAsset b("b"); b.setDirty(true); b.setFrameCount(1);
    SpriteAsset c("c"); c.setFrameCount(8); c.setLooping(true);

    editor.addSprite(a); editor.addSprite(b); editor.addSprite(c);

    REQUIRE(editor.dirtyCount()    == 2);
    REQUIRE(editor.animatedCount() == 2); // a and c
    REQUIRE(editor.playingCount()  == 1); // only a
    REQUIRE(editor.loopingCount()  == 2); // a and c
}

TEST_CASE("SpriteEditor countByBlendMode and countByOrigin", "[Editor][S38]") {
    SpriteEditor editor;

    SpriteAsset a("a"); a.setBlendMode(SpriteBlendMode::Additive); a.setOrigin(SpriteOrigin::TopLeft);
    SpriteAsset b("b"); b.setBlendMode(SpriteBlendMode::Additive); b.setOrigin(SpriteOrigin::Center);
    SpriteAsset c("c"); c.setBlendMode(SpriteBlendMode::Normal);   c.setOrigin(SpriteOrigin::TopLeft);

    editor.addSprite(a); editor.addSprite(b); editor.addSprite(c);

    REQUIRE(editor.countByBlendMode(SpriteBlendMode::Additive) == 2);
    REQUIRE(editor.countByBlendMode(SpriteBlendMode::Normal)   == 1);
    REQUIRE(editor.countByOrigin(SpriteOrigin::TopLeft)        == 2);
    REQUIRE(editor.countByOrigin(SpriteOrigin::Center)         == 1);
}

TEST_CASE("SpriteEditor setActiveSprite returns false for missing", "[Editor][S38]") {
    SpriteEditor editor;
    REQUIRE_FALSE(editor.setActiveSprite("ghost"));
    REQUIRE(editor.activeSprite().empty());
}

TEST_CASE("SpriteEditor MAX_SPRITES limit enforced", "[Editor][S38]") {
    SpriteEditor editor;
    for (size_t i = 0; i < SpriteEditor::MAX_SPRITES; ++i) {
        SpriteAsset s("Sprite" + std::to_string(i));
        REQUIRE(editor.addSprite(s));
    }
    SpriteAsset overflow("Overflow");
    REQUIRE_FALSE(editor.addSprite(overflow));
    REQUIRE(editor.spriteCount() == SpriteEditor::MAX_SPRITES);
}

TEST_CASE("SpriteAsset area calculation", "[Editor][S38]") {
    SpriteAsset s("bg", 800, 600);
    REQUIRE(s.area() == 800u * 600u);
}
