#include <catch2/catch_test_macros.hpp>
#include "NF/Editor/Editor.h"
#include "NF/Editor/ShaderGraphEditor.h"
#include "NF/Editor/ParticleEditor.h"
#include "NF/Editor/IconEditor.h"

using namespace NF;

// ── S84: IconEditor + ParticleEditor + ShaderGraphEditor ─────────

// ── IconEditor ───────────────────────────────────────────────────

TEST_CASE("IconSize names are correct", "[Editor][S84]") {
    REQUIRE(std::string(iconSizeName(IconSize::XSmall)) == "XSmall");
    REQUIRE(std::string(iconSizeName(IconSize::Small))  == "Small");
    REQUIRE(std::string(iconSizeName(IconSize::Medium)) == "Medium");
    REQUIRE(std::string(iconSizeName(IconSize::Large))  == "Large");
    REQUIRE(std::string(iconSizeName(IconSize::XLarge)) == "XLarge");
}

TEST_CASE("IconTheme names are correct", "[Editor][S84]") {
    REQUIRE(std::string(iconThemeName(IconTheme::Light))        == "Light");
    REQUIRE(std::string(iconThemeName(IconTheme::Dark))         == "Dark");
    REQUIRE(std::string(iconThemeName(IconTheme::HighContrast)) == "HighContrast");
    REQUIRE(std::string(iconThemeName(IconTheme::Monochrome))   == "Monochrome");
}

TEST_CASE("IconState names are correct", "[Editor][S84]") {
    REQUIRE(std::string(iconStateName(IconState::Normal))   == "Normal");
    REQUIRE(std::string(iconStateName(IconState::Hover))    == "Hover");
    REQUIRE(std::string(iconStateName(IconState::Pressed))  == "Pressed");
    REQUIRE(std::string(iconStateName(IconState::Disabled)) == "Disabled");
    REQUIRE(std::string(iconStateName(IconState::Selected)) == "Selected");
}

TEST_CASE("IconAsset default size is Medium, theme is Light", "[Editor][S84]") {
    IconAsset icon("save_icon");
    REQUIRE(icon.name() == "save_icon");
    REQUIRE(icon.size() == IconSize::Medium);
    REQUIRE(icon.theme() == IconTheme::Light);
    REQUIRE(icon.state() == IconState::Normal);
    REQUIRE_FALSE(icon.isScalable());
    REQUIRE_FALSE(icon.isDirty());
    REQUIRE(icon.pixelDensity() == 1.0f);
}

TEST_CASE("IconAsset isDisabled and isSelected predicates", "[Editor][S84]") {
    IconAsset icon("close");
    icon.setState(IconState::Disabled);
    REQUIRE(icon.isDisabled());
    REQUIRE_FALSE(icon.isSelected());
    icon.setState(IconState::Selected);
    REQUIRE(icon.isSelected());
}

TEST_CASE("IconAsset isHighDPI when pixelDensity >= 2", "[Editor][S84]") {
    IconAsset icon("star");
    REQUIRE_FALSE(icon.isHighDPI());
    icon.setPixelDensity(2.0f);
    REQUIRE(icon.isHighDPI());
}

TEST_CASE("IconEditor addIcon and iconCount", "[Editor][S84]") {
    IconEditor editor;
    IconAsset icon("folder", IconSize::Large);
    REQUIRE(editor.addIcon(icon));
    REQUIRE(editor.iconCount() == 1);
}

TEST_CASE("IconEditor addIcon rejects duplicate name", "[Editor][S84]") {
    IconEditor editor;
    IconAsset icon("folder");
    editor.addIcon(icon);
    REQUIRE_FALSE(editor.addIcon(icon));
}

TEST_CASE("IconEditor removeIcon removes entry", "[Editor][S84]") {
    IconEditor editor;
    IconAsset icon("trash");
    editor.addIcon(icon);
    REQUIRE(editor.removeIcon("trash"));
    REQUIRE(editor.iconCount() == 0);
}

TEST_CASE("IconEditor setActiveIcon updates active", "[Editor][S84]") {
    IconEditor editor;
    IconAsset icon("home");
    editor.addIcon(icon);
    REQUIRE(editor.setActiveIcon("home"));
    REQUIRE(editor.activeIcon() == "home");
}

TEST_CASE("IconEditor countByTheme and countBySize", "[Editor][S84]") {
    IconEditor editor;
    IconAsset a("a"); a.setTheme(IconTheme::Dark); a.setSize(IconSize::Large);
    IconAsset b("b"); b.setTheme(IconTheme::Dark); b.setSize(IconSize::Small);
    IconAsset c("c"); c.setTheme(IconTheme::Light); c.setSize(IconSize::Large);
    editor.addIcon(a); editor.addIcon(b); editor.addIcon(c);
    REQUIRE(editor.countByTheme(IconTheme::Dark) == 2);
    REQUIRE(editor.countBySize(IconSize::Large) == 2);
}

TEST_CASE("IconEditor scalableCount and highDPICount", "[Editor][S84]") {
    IconEditor editor;
    IconAsset a("a"); a.setScalable(true); a.setPixelDensity(2.0f);
    IconAsset b("b");
    editor.addIcon(a); editor.addIcon(b);
    REQUIRE(editor.scalableCount() == 1);
    REQUIRE(editor.highDPICount() == 1);
}

TEST_CASE("IconEditor MAX_ICONS is 256", "[Editor][S84]") {
    REQUIRE(IconEditor::MAX_ICONS == 256);
}

// ── ParticleEditor ───────────────────────────────────────────────

TEST_CASE("ParticleEmitterShape names are correct", "[Editor][S84]") {
    REQUIRE(std::string(particleEmitterShapeName(ParticleEmitterShape::Point))     == "Point");
    REQUIRE(std::string(particleEmitterShapeName(ParticleEmitterShape::Circle))    == "Circle");
    REQUIRE(std::string(particleEmitterShapeName(ParticleEmitterShape::Rectangle)) == "Rectangle");
    REQUIRE(std::string(particleEmitterShapeName(ParticleEmitterShape::Cone))      == "Cone");
    REQUIRE(std::string(particleEmitterShapeName(ParticleEmitterShape::Sphere))    == "Sphere");
    REQUIRE(std::string(particleEmitterShapeName(ParticleEmitterShape::Ring))      == "Ring");
    REQUIRE(std::string(particleEmitterShapeName(ParticleEmitterShape::Line))      == "Line");
    REQUIRE(std::string(particleEmitterShapeName(ParticleEmitterShape::Custom))    == "Custom");
}

TEST_CASE("ParticleBlendMode names are correct", "[Editor][S84]") {
    REQUIRE(std::string(particleBlendModeName(ParticleBlendMode::Additive)) == "Additive");
    REQUIRE(std::string(particleBlendModeName(ParticleBlendMode::Alpha))    == "Alpha");
    REQUIRE(std::string(particleBlendModeName(ParticleBlendMode::Multiply)) == "Multiply");
    REQUIRE(std::string(particleBlendModeName(ParticleBlendMode::Screen))   == "Screen");
}

TEST_CASE("ParticleEmitterConfig isValid with positive values", "[Editor][S84]") {
    ParticleEmitterConfig cfg;
    cfg.id = "smoke";
    REQUIRE(cfg.isValid());
    cfg.setEmitRate(-1.0f);
    REQUIRE_FALSE(cfg.isValid());
}

TEST_CASE("ParticleEffectLayer addEmitter and emitterCount", "[Editor][S84]") {
    ParticleEffectLayer layer("fire");
    ParticleEmitterConfig cfg; cfg.id = "sparks"; cfg.setEmitRate(20.f);
    REQUIRE(layer.addEmitter(cfg));
    REQUIRE(layer.emitterCount() == 1);
}

TEST_CASE("ParticleEffectLayer addEmitter rejects duplicate id", "[Editor][S84]") {
    ParticleEffectLayer layer("smoke");
    ParticleEmitterConfig cfg; cfg.id = "emitter1";
    layer.addEmitter(cfg);
    REQUIRE_FALSE(layer.addEmitter(cfg));
}

TEST_CASE("ParticleEffectLayer removeEmitter removes entry", "[Editor][S84]") {
    ParticleEffectLayer layer("fire");
    ParticleEmitterConfig cfg; cfg.id = "sparks";
    layer.addEmitter(cfg);
    REQUIRE(layer.removeEmitter("sparks"));
    REQUIRE(layer.emitterCount() == 0);
}

TEST_CASE("ParticleEffectLayer visible default true", "[Editor][S84]") {
    ParticleEffectLayer layer("base");
    REQUIRE(layer.visible());
    layer.setVisible(false);
    REQUIRE_FALSE(layer.visible());
}

TEST_CASE("ParticleEffectEditor addLayer and layerCount", "[Editor][S84]") {
    ParticleEffectEditor editor;
    ParticleEffectLayer layer("fire");
    REQUIRE(editor.addLayer(layer));
    REQUIRE(editor.layerCount() == 1);
}

TEST_CASE("ParticleEffectEditor addLayer rejects duplicate name", "[Editor][S84]") {
    ParticleEffectEditor editor;
    ParticleEffectLayer layer("fire");
    editor.addLayer(layer);
    REQUIRE_FALSE(editor.addLayer(layer));
}

TEST_CASE("ParticleEffectEditor preview and stopPreview", "[Editor][S84]") {
    ParticleEffectEditor editor;
    REQUIRE_FALSE(editor.isPreviewing());
    editor.preview();
    REQUIRE(editor.isPreviewing());
    editor.stopPreview();
    REQUIRE_FALSE(editor.isPreviewing());
}

TEST_CASE("ParticleEffectEditor totalEmitterCount sums all layers", "[Editor][S84]") {
    ParticleEffectEditor editor;
    ParticleEffectLayer layer1("fire");
    ParticleEmitterConfig e1; e1.id = "a";
    ParticleEmitterConfig e2; e2.id = "b";
    layer1.addEmitter(e1); layer1.addEmitter(e2);
    ParticleEffectLayer layer2("smoke");
    ParticleEmitterConfig e3; e3.id = "c";
    layer2.addEmitter(e3);
    editor.addLayer(layer1); editor.addLayer(layer2);
    REQUIRE(editor.totalEmitterCount() == 3);
}

// ── ShaderGraphEditor ────────────────────────────────────────────

TEST_CASE("ShaderNodeType names are correct", "[Editor][S84]") {
    REQUIRE(std::string(shaderNodeTypeName(ShaderNodeType::Input))   == "Input");
    REQUIRE(std::string(shaderNodeTypeName(ShaderNodeType::Output))  == "Output");
    REQUIRE(std::string(shaderNodeTypeName(ShaderNodeType::Math))    == "Math");
    REQUIRE(std::string(shaderNodeTypeName(ShaderNodeType::Texture)) == "Texture");
    REQUIRE(std::string(shaderNodeTypeName(ShaderNodeType::Color))   == "Color");
    REQUIRE(std::string(shaderNodeTypeName(ShaderNodeType::Vector))  == "Vector");
    REQUIRE(std::string(shaderNodeTypeName(ShaderNodeType::Blend))   == "Blend");
    REQUIRE(std::string(shaderNodeTypeName(ShaderNodeType::Custom))  == "Custom");
}

TEST_CASE("ShaderPortKind names are correct", "[Editor][S84]") {
    REQUIRE(std::string(shaderPortKindName(ShaderPortKind::Float))   == "Float");
    REQUIRE(std::string(shaderPortKindName(ShaderPortKind::Vector2)) == "Vector2");
    REQUIRE(std::string(shaderPortKindName(ShaderPortKind::Vector3)) == "Vector3");
    REQUIRE(std::string(shaderPortKindName(ShaderPortKind::Vector4)) == "Vector4");
}

TEST_CASE("ShaderNode select deselect and setPosition", "[Editor][S84]") {
    ShaderNode node;
    node.id = "n1";
    node.setPosition(100.f, 200.f);
    REQUIRE(node.posX == 100.f);
    REQUIRE(node.posY == 200.f);
    REQUIRE_FALSE(node.selected);
    node.select();
    REQUIRE(node.selected);
    node.deselect();
    REQUIRE_FALSE(node.selected);
}

TEST_CASE("ShaderGraphEditor addNode and nodeCount", "[Editor][S84]") {
    ShaderGraphEditor editor;
    ShaderNode node; node.id = "input1"; node.type = ShaderNodeType::Input;
    REQUIRE(editor.addNode(node));
    REQUIRE(editor.nodeCount() == 1);
}

TEST_CASE("ShaderGraphEditor addNode rejects duplicate id", "[Editor][S84]") {
    ShaderGraphEditor editor;
    ShaderNode node; node.id = "output1";
    editor.addNode(node);
    REQUIRE_FALSE(editor.addNode(node));
}

TEST_CASE("ShaderGraphEditor removeNode removes entry and cleans edges", "[Editor][S84]") {
    ShaderGraphEditor editor;
    ShaderNode n1; n1.id = "n1";
    ShaderNode n2; n2.id = "n2";
    editor.addNode(n1); editor.addNode(n2);
    ShaderGraphEdge edge; edge.id = "e1"; edge.fromNode = "n1"; edge.toNode = "n2";
    editor.addEdge(edge);
    REQUIRE(editor.edgeCount() == 1);
    REQUIRE(editor.removeNode("n1"));
    REQUIRE(editor.nodeCount() == 1);
    REQUIRE(editor.edgeCount() == 0); // edges cleaned up
}

TEST_CASE("ShaderGraphEditor addEdge fails when nodes missing", "[Editor][S84]") {
    ShaderGraphEditor editor;
    ShaderNode n; n.id = "n1";
    editor.addNode(n);
    ShaderGraphEdge edge; edge.id = "e1"; edge.fromNode = "n1"; edge.toNode = "n2"; // n2 missing
    REQUIRE_FALSE(editor.addEdge(edge));
}

TEST_CASE("ShaderGraphEditor removeEdge removes edge", "[Editor][S84]") {
    ShaderGraphEditor editor;
    ShaderNode n1; n1.id = "n1";
    ShaderNode n2; n2.id = "n2";
    editor.addNode(n1); editor.addNode(n2);
    ShaderGraphEdge edge; edge.id = "e1"; edge.fromNode = "n1"; edge.toNode = "n2";
    editor.addEdge(edge);
    REQUIRE(editor.removeEdge("e1"));
    REQUIRE(editor.edgeCount() == 0);
}

TEST_CASE("ShaderGraphEditor selectAll and deselectAll", "[Editor][S84]") {
    ShaderGraphEditor editor;
    ShaderNode n1; n1.id = "n1";
    ShaderNode n2; n2.id = "n2";
    editor.addNode(n1); editor.addNode(n2);
    editor.selectAll();
    REQUIRE(editor.selectedCount() == 2);
    editor.deselectAll();
    REQUIRE(editor.selectedCount() == 0);
}

TEST_CASE("ShaderGraphEditor MAX_NODES and MAX_EDGES constants", "[Editor][S84]") {
    REQUIRE(ShaderGraphEditor::MAX_NODES == 256);
    REQUIRE(ShaderGraphEditor::MAX_EDGES == 512);
}
