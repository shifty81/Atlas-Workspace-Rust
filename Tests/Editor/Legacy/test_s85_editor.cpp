#include <catch2/catch_test_macros.hpp>
#include "NF/Editor/Editor.h"
#include "NF/Editor/TimelineEditor.h"
#include "NF/Editor/TilemapEditor.h"
#include "NF/Editor/SpriteEditor.h"

using namespace NF;

// ── S85: SpriteEditor + TilemapEditor + TimelineEditor ──────────

// ── SpriteEditor ─────────────────────────────────────────────────

TEST_CASE("SpriteOrigin names are correct", "[Editor][S85]") {
    REQUIRE(std::string(spriteOriginName(SpriteOrigin::TopLeft))      == "TopLeft");
    REQUIRE(std::string(spriteOriginName(SpriteOrigin::TopCenter))    == "TopCenter");
    REQUIRE(std::string(spriteOriginName(SpriteOrigin::Center))       == "Center");
    REQUIRE(std::string(spriteOriginName(SpriteOrigin::BottomLeft))   == "BottomLeft");
    REQUIRE(std::string(spriteOriginName(SpriteOrigin::BottomCenter)) == "BottomCenter");
}

TEST_CASE("SpriteBlendMode names are correct", "[Editor][S85]") {
    REQUIRE(std::string(spriteBlendModeName(SpriteBlendMode::Normal))   == "Normal");
    REQUIRE(std::string(spriteBlendModeName(SpriteBlendMode::Additive)) == "Additive");
    REQUIRE(std::string(spriteBlendModeName(SpriteBlendMode::Multiply)) == "Multiply");
    REQUIRE(std::string(spriteBlendModeName(SpriteBlendMode::Screen))   == "Screen");
    REQUIRE(std::string(spriteBlendModeName(SpriteBlendMode::Overlay))  == "Overlay");
}

TEST_CASE("SpriteAnimState names are correct", "[Editor][S85]") {
    REQUIRE(std::string(spriteAnimStateName(SpriteAnimState::Idle))     == "Idle");
    REQUIRE(std::string(spriteAnimStateName(SpriteAnimState::Playing))  == "Playing");
    REQUIRE(std::string(spriteAnimStateName(SpriteAnimState::Paused))   == "Paused");
    REQUIRE(std::string(spriteAnimStateName(SpriteAnimState::Stopped))  == "Stopped");
    REQUIRE(std::string(spriteAnimStateName(SpriteAnimState::Finished)) == "Finished");
}

TEST_CASE("SpriteAsset default dimensions are 32x32", "[Editor][S85]") {
    SpriteAsset sprite("hero");
    REQUIRE(sprite.name() == "hero");
    REQUIRE(sprite.width() == 32);
    REQUIRE(sprite.height() == 32);
    REQUIRE(sprite.area() == 1024);
    REQUIRE(sprite.origin() == SpriteOrigin::Center);
    REQUIRE(sprite.blendMode() == SpriteBlendMode::Normal);
    REQUIRE(sprite.animState() == SpriteAnimState::Idle);
}

TEST_CASE("SpriteAsset isAnimated when frameCount > 1", "[Editor][S85]") {
    SpriteAsset sprite("run", 64, 64);
    REQUIRE_FALSE(sprite.isAnimated());
    sprite.setFrameCount(8);
    REQUIRE(sprite.isAnimated());
}

TEST_CASE("SpriteAsset isPlaying isPaused isFinished predicates", "[Editor][S85]") {
    SpriteAsset sprite("idle");
    sprite.setAnimState(SpriteAnimState::Playing);
    REQUIRE(sprite.isPlaying());
    sprite.setAnimState(SpriteAnimState::Paused);
    REQUIRE(sprite.isPaused());
    sprite.setAnimState(SpriteAnimState::Finished);
    REQUIRE(sprite.isFinished());
}

TEST_CASE("SpriteAsset flip flags", "[Editor][S85]") {
    SpriteAsset sprite("enemy");
    REQUIRE_FALSE(sprite.isFlippedH());
    REQUIRE_FALSE(sprite.isFlippedV());
    sprite.setFlippedH(true);
    sprite.setFlippedV(true);
    REQUIRE(sprite.isFlippedH());
    REQUIRE(sprite.isFlippedV());
}

TEST_CASE("SpriteEditor addSprite and spriteCount", "[Editor][S85]") {
    SpriteEditor editor;
    SpriteAsset sprite("hero", 32, 32);
    REQUIRE(editor.addSprite(sprite));
    REQUIRE(editor.spriteCount() == 1);
}

TEST_CASE("SpriteEditor addSprite rejects duplicate name", "[Editor][S85]") {
    SpriteEditor editor;
    SpriteAsset sprite("hero");
    editor.addSprite(sprite);
    REQUIRE_FALSE(editor.addSprite(sprite));
}

TEST_CASE("SpriteEditor removeSprite removes entry", "[Editor][S85]") {
    SpriteEditor editor;
    SpriteAsset sprite("hero");
    editor.addSprite(sprite);
    REQUIRE(editor.removeSprite("hero"));
    REQUIRE(editor.spriteCount() == 0);
}

TEST_CASE("SpriteEditor setActiveSprite updates active", "[Editor][S85]") {
    SpriteEditor editor;
    SpriteAsset sprite("enemy");
    editor.addSprite(sprite);
    REQUIRE(editor.setActiveSprite("enemy"));
    REQUIRE(editor.activeSprite() == "enemy");
}

TEST_CASE("SpriteEditor animatedCount and playingCount", "[Editor][S85]") {
    SpriteEditor editor;
    SpriteAsset a("a"); a.setFrameCount(4);
    SpriteAsset b("b"); b.setFrameCount(1);
    SpriteAsset c("c"); c.setFrameCount(8); c.setAnimState(SpriteAnimState::Playing);
    editor.addSprite(a); editor.addSprite(b); editor.addSprite(c);
    REQUIRE(editor.animatedCount() == 2);
    REQUIRE(editor.playingCount() == 1);
}

TEST_CASE("SpriteEditor countByBlendMode and countByOrigin", "[Editor][S85]") {
    SpriteEditor editor;
    SpriteAsset a("a"); a.setBlendMode(SpriteBlendMode::Additive); a.setOrigin(SpriteOrigin::TopLeft);
    SpriteAsset b("b"); b.setBlendMode(SpriteBlendMode::Normal);   b.setOrigin(SpriteOrigin::TopLeft);
    SpriteAsset c("c"); c.setBlendMode(SpriteBlendMode::Additive); c.setOrigin(SpriteOrigin::Center);
    editor.addSprite(a); editor.addSprite(b); editor.addSprite(c);
    REQUIRE(editor.countByOrigin(SpriteOrigin::TopLeft) == 2);
}

TEST_CASE("SpriteEditor MAX_SPRITES is 512", "[Editor][S85]") {
    REQUIRE(SpriteEditor::MAX_SPRITES == 512);
}

// ── TilemapEditor ────────────────────────────────────────────────

TEST_CASE("TileFlipMode names are correct", "[Editor][S85]") {
    REQUIRE(std::string(tileFlipModeName(TileFlipMode::None))       == "None");
    REQUIRE(std::string(tileFlipModeName(TileFlipMode::Horizontal)) == "Horizontal");
    REQUIRE(std::string(tileFlipModeName(TileFlipMode::Vertical))   == "Vertical");
    REQUIRE(std::string(tileFlipModeName(TileFlipMode::Both))       == "Both");
    REQUIRE(std::string(tileFlipModeName(TileFlipMode::Rotate90))   == "Rotate90");
}

TEST_CASE("TileLayerType names are correct", "[Editor][S85]") {
    REQUIRE(std::string(tileLayerTypeName(TileLayerType::Background)) == "Background");
    REQUIRE(std::string(tileLayerTypeName(TileLayerType::Midground))  == "Midground");
    REQUIRE(std::string(tileLayerTypeName(TileLayerType::Foreground)) == "Foreground");
    REQUIRE(std::string(tileLayerTypeName(TileLayerType::Object))     == "Object");
    REQUIRE(std::string(tileLayerTypeName(TileLayerType::Collision))  == "Collision");
}

TEST_CASE("TileAnimMode names are correct", "[Editor][S85]") {
    REQUIRE(std::string(tileAnimModeName(TileAnimMode::Static))   == "Static");
    REQUIRE(std::string(tileAnimModeName(TileAnimMode::Loop))     == "Loop");
    REQUIRE(std::string(tileAnimModeName(TileAnimMode::PingPong)) == "PingPong");
    REQUIRE(std::string(tileAnimModeName(TileAnimMode::Once))     == "Once");
    REQUIRE(std::string(tileAnimModeName(TileAnimMode::Random))   == "Random");
}

TEST_CASE("TileAsset default tileWidth and tileHeight are 16", "[Editor][S85]") {
    TileAsset tile("grass");
    REQUIRE(tile.name() == "grass");
    REQUIRE(tile.tileWidth() == 16);
    REQUIRE(tile.tileHeight() == 16);
    REQUIRE(tile.area() == 256);
    REQUIRE(tile.layerType() == TileLayerType::Background);
    REQUIRE(tile.flipMode() == TileFlipMode::None);
    REQUIRE(tile.animMode() == TileAnimMode::Static);
}

TEST_CASE("TileAsset collider and animated flags", "[Editor][S85]") {
    TileAsset tile("stone");
    REQUIRE_FALSE(tile.isAnimated());
    REQUIRE_FALSE(tile.hasCollider());
    tile.setAnimated(true);
    tile.setCollider(true);
    REQUIRE(tile.isAnimated());
    REQUIRE(tile.hasCollider());
}

TEST_CASE("TilemapEditor addTile and tileCount", "[Editor][S85]") {
    TilemapEditor editor;
    TileAsset tile("grass");
    REQUIRE(editor.addTile(tile));
    REQUIRE(editor.tileCount() == 1);
}

TEST_CASE("TilemapEditor addTile rejects duplicate name", "[Editor][S85]") {
    TilemapEditor editor;
    TileAsset tile("rock");
    editor.addTile(tile);
    REQUIRE_FALSE(editor.addTile(tile));
}

TEST_CASE("TilemapEditor removeTile removes entry", "[Editor][S85]") {
    TilemapEditor editor;
    TileAsset tile("water");
    editor.addTile(tile);
    REQUIRE(editor.removeTile("water"));
    REQUIRE(editor.tileCount() == 0);
}

TEST_CASE("TilemapEditor setActiveTile updates active", "[Editor][S85]") {
    TilemapEditor editor;
    TileAsset tile("sand");
    editor.addTile(tile);
    REQUIRE(editor.setActiveTile("sand"));
    REQUIRE(editor.activeTile() == "sand");
}

TEST_CASE("TilemapEditor animatedCount and colliderCount", "[Editor][S85]") {
    TilemapEditor editor;
    TileAsset a("a"); a.setAnimated(true);
    TileAsset b("b"); b.setCollider(true);
    TileAsset c("c"); c.setAnimated(true); c.setCollider(true);
    editor.addTile(a); editor.addTile(b); editor.addTile(c);
    REQUIRE(editor.animatedCount() == 2);
    REQUIRE(editor.colliderCount() == 2);
}

TEST_CASE("TilemapEditor countByLayerType", "[Editor][S85]") {
    TilemapEditor editor;
    TileAsset a("a"); a.setLayerType(TileLayerType::Foreground);
    TileAsset b("b"); b.setLayerType(TileLayerType::Background);
    TileAsset c("c"); c.setLayerType(TileLayerType::Foreground);
    editor.addTile(a); editor.addTile(b); editor.addTile(c);
    REQUIRE(editor.countByLayerType(TileLayerType::Foreground) == 2);
    REQUIRE(editor.countByLayerType(TileLayerType::Background) == 1);
}

TEST_CASE("TilemapEditor MAX_TILES is 256", "[Editor][S85]") {
    REQUIRE(TilemapEditor::MAX_TILES == 256);
}

// ── TimelineEditor ───────────────────────────────────────────────

TEST_CASE("TimelineEventType names are correct", "[Editor][S85]") {
    REQUIRE(std::string(timelineEventTypeName(TimelineEventType::Keyframe)) == "Keyframe");
    REQUIRE(std::string(timelineEventTypeName(TimelineEventType::Marker))   == "Marker");
    REQUIRE(std::string(timelineEventTypeName(TimelineEventType::Clip))     == "Clip");
    REQUIRE(std::string(timelineEventTypeName(TimelineEventType::Trigger))  == "Trigger");
    REQUIRE(std::string(timelineEventTypeName(TimelineEventType::Label))    == "Label");
    REQUIRE(std::string(timelineEventTypeName(TimelineEventType::Camera))   == "Camera");
    REQUIRE(std::string(timelineEventTypeName(TimelineEventType::Audio))    == "Audio");
    REQUIRE(std::string(timelineEventTypeName(TimelineEventType::Custom))   == "Custom");
}

TEST_CASE("TimelineTrackKind names are correct", "[Editor][S85]") {
    REQUIRE(std::string(timelineTrackKindName(TimelineTrackKind::Animation)) == "Animation");
    REQUIRE(std::string(timelineTrackKindName(TimelineTrackKind::Audio))     == "Audio");
    REQUIRE(std::string(timelineTrackKindName(TimelineTrackKind::Event))     == "Event");
    REQUIRE(std::string(timelineTrackKindName(TimelineTrackKind::Camera))    == "Camera");
}

TEST_CASE("TimelineEvent select deselect setTime setDuration", "[Editor][S85]") {
    TimelineEvent ev;
    ev.id = "cut1";
    ev.setTime(3.0f);
    ev.setDuration(2.0f);
    REQUIRE(ev.time == 3.0f);
    REQUIRE(ev.duration == 2.0f);
    REQUIRE_FALSE(ev.selected);
    ev.select();
    REQUIRE(ev.selected);
    ev.deselect();
    REQUIRE_FALSE(ev.selected);
}

TEST_CASE("TimelineTrack addEvent and eventCount", "[Editor][S85]") {
    TimelineTrack track("Anim", TimelineTrackKind::Animation);
    TimelineEvent ev; ev.id = "e1"; ev.setTime(0.f);
    REQUIRE(track.addEvent(ev));
    REQUIRE(track.eventCount() == 1);
}

TEST_CASE("TimelineTrack addEvent rejects duplicate id", "[Editor][S85]") {
    TimelineTrack track("Audio", TimelineTrackKind::Audio);
    TimelineEvent ev; ev.id = "ev1";
    track.addEvent(ev);
    REQUIRE_FALSE(track.addEvent(ev));
}

TEST_CASE("TimelineTrack removeEvent removes entry", "[Editor][S85]") {
    TimelineTrack track("Cam", TimelineTrackKind::Camera);
    TimelineEvent ev; ev.id = "cut1";
    track.addEvent(ev);
    REQUIRE(track.removeEvent("cut1"));
    REQUIRE(track.eventCount() == 0);
}

TEST_CASE("TimelineTrack selectAll deselectAll", "[Editor][S85]") {
    TimelineTrack track("Events", TimelineTrackKind::Event);
    TimelineEvent e1; e1.id = "e1";
    TimelineEvent e2; e2.id = "e2";
    track.addEvent(e1); track.addEvent(e2);
    track.selectAll();
    REQUIRE(track.selectedCount() == 2);
    track.deselectAll();
    REQUIRE(track.selectedCount() == 0);
}

TEST_CASE("TimelineTrack duration is max event end time", "[Editor][S85]") {
    TimelineTrack track("Anim", TimelineTrackKind::Animation);
    TimelineEvent e1; e1.id = "e1"; e1.setTime(0.f); e1.setDuration(3.f);
    TimelineEvent e2; e2.id = "e2"; e2.setTime(5.f); e2.setDuration(1.f);
    track.addEvent(e1); track.addEvent(e2);
    REQUIRE(track.duration() == 6.0f);
}

TEST_CASE("TimelineTrack muted flag", "[Editor][S85]") {
    TimelineTrack track("Audio", TimelineTrackKind::Audio);
    REQUIRE_FALSE(track.muted());
    track.setMuted(true);
    REQUIRE(track.muted());
}

TEST_CASE("TimelineEditorPanel addTrack and trackCount", "[Editor][S85]") {
    TimelineEditorPanel panel;
    TimelineTrack track("AnimTrack", TimelineTrackKind::Animation);
    REQUIRE(panel.addTrack(track));
    REQUIRE(panel.trackCount() == 1);
}

TEST_CASE("TimelineEditorPanel addTrack rejects duplicate name", "[Editor][S85]") {
    TimelineEditorPanel panel;
    TimelineTrack track("AnimTrack", TimelineTrackKind::Animation);
    panel.addTrack(track);
    REQUIRE_FALSE(panel.addTrack(track));
}

TEST_CASE("TimelineEditorPanel setActiveTrack updates active", "[Editor][S85]") {
    TimelineEditorPanel panel;
    TimelineTrack track("CamTrack", TimelineTrackKind::Camera);
    panel.addTrack(track);
    REQUIRE(panel.setActiveTrack("CamTrack"));
    REQUIRE(panel.activeTrack() == "CamTrack");
}

TEST_CASE("TimelineEditorPanel play pause stop and playhead", "[Editor][S85]") {
    TimelineEditorPanel panel;
    panel.setPlayhead(5.0f);
    REQUIRE_FALSE(panel.isPlaying());
    panel.play();
    REQUIRE(panel.isPlaying());
    panel.pause();
    REQUIRE_FALSE(panel.isPlaying());
    REQUIRE(panel.playhead() == 5.0f);
    panel.play();
    panel.stop();
    REQUIRE_FALSE(panel.isPlaying());
    REQUIRE(panel.playhead() == 0.0f);
}

TEST_CASE("TimelineEditorPanel MAX_TRACKS is 64", "[Editor][S85]") {
    REQUIRE(TimelineEditorPanel::MAX_TRACKS == 64);
}
