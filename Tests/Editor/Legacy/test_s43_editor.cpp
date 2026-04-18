#include <catch2/catch_test_macros.hpp>
#include <catch2/catch_approx.hpp>
#include "NF/Editor/Editor.h"
#include "NF/Editor/AnimBlueprintEditor.h"

using namespace NF;

TEST_CASE("AnimBPNodeType names cover all 5 values", "[Editor][S43]") {
    REQUIRE(std::string(animBPNodeTypeName(AnimBPNodeType::StateMachine)) == "StateMachine");
    REQUIRE(std::string(animBPNodeTypeName(AnimBPNodeType::BlendSpace))   == "BlendSpace");
    REQUIRE(std::string(animBPNodeTypeName(AnimBPNodeType::Selector))     == "Selector");
    REQUIRE(std::string(animBPNodeTypeName(AnimBPNodeType::Sequence))     == "Sequence");
    REQUIRE(std::string(animBPNodeTypeName(AnimBPNodeType::Pose))         == "Pose");
}

TEST_CASE("AnimBPState names cover all 5 values", "[Editor][S43]") {
    REQUIRE(std::string(animBPStateName(AnimBPState::Inactive))  == "Inactive");
    REQUIRE(std::string(animBPStateName(AnimBPState::Compiling)) == "Compiling");
    REQUIRE(std::string(animBPStateName(AnimBPState::Ready))     == "Ready");
    REQUIRE(std::string(animBPStateName(AnimBPState::Running))   == "Running");
    REQUIRE(std::string(animBPStateName(AnimBPState::Error))     == "Error");
}

TEST_CASE("AnimBPBlendMode names cover all 5 values", "[Editor][S43]") {
    REQUIRE(std::string(animBPBlendModeName(AnimBPBlendMode::Override)) == "Override");
    REQUIRE(std::string(animBPBlendModeName(AnimBPBlendMode::Additive)) == "Additive");
    REQUIRE(std::string(animBPBlendModeName(AnimBPBlendMode::Layered))  == "Layered");
    REQUIRE(std::string(animBPBlendModeName(AnimBPBlendMode::Masked))   == "Masked");
    REQUIRE(std::string(animBPBlendModeName(AnimBPBlendMode::Blended))  == "Blended");
}

TEST_CASE("AnimBlueprintAsset default values", "[Editor][S43]") {
    AnimBlueprintAsset bp("locomotion");
    REQUIRE(bp.name()       == "locomotion");
    REQUIRE(bp.nodeCount()  == 0u);
    REQUIRE(bp.layerCount() == 0u);
    REQUIRE(bp.nodeType()   == AnimBPNodeType::StateMachine);
    REQUIRE(bp.state()      == AnimBPState::Inactive);
    REQUIRE(bp.blendMode()  == AnimBPBlendMode::Override);
    REQUIRE_FALSE(bp.isLooping());
    REQUIRE_FALSE(bp.isOptimized());
    REQUIRE_FALSE(bp.isDirty());
    REQUIRE_FALSE(bp.isRunning());
    REQUIRE_FALSE(bp.hasError());
    REQUIRE_FALSE(bp.isReady());
    REQUIRE_FALSE(bp.isComplex());
}

TEST_CASE("AnimBlueprintAsset setters round-trip", "[Editor][S43]") {
    AnimBlueprintAsset bp("combat", 5, 2);
    bp.setNodeType(AnimBPNodeType::BlendSpace);
    bp.setState(AnimBPState::Running);
    bp.setBlendMode(AnimBPBlendMode::Additive);
    bp.setNodeCount(25);
    bp.setLayerCount(4);
    bp.setLooping(true);
    bp.setOptimized(true);
    bp.setDirty(true);

    REQUIRE(bp.nodeType()   == AnimBPNodeType::BlendSpace);
    REQUIRE(bp.state()      == AnimBPState::Running);
    REQUIRE(bp.blendMode()  == AnimBPBlendMode::Additive);
    REQUIRE(bp.nodeCount()  == 25u);
    REQUIRE(bp.layerCount() == 4u);
    REQUIRE(bp.isLooping());
    REQUIRE(bp.isOptimized());
    REQUIRE(bp.isDirty());
    REQUIRE(bp.isRunning());
    REQUIRE_FALSE(bp.hasError());
    REQUIRE_FALSE(bp.isReady());
    REQUIRE(bp.isComplex());
}

TEST_CASE("AnimBlueprintAsset isComplex requires nodeCount>=20", "[Editor][S43]") {
    AnimBlueprintAsset bp("simple");
    bp.setNodeCount(19);
    REQUIRE_FALSE(bp.isComplex());

    bp.setNodeCount(20);
    REQUIRE(bp.isComplex());
}

TEST_CASE("AnimBlueprintAsset hasError and isReady states", "[Editor][S43]") {
    AnimBlueprintAsset bp("test");
    bp.setState(AnimBPState::Ready);
    REQUIRE(bp.isReady());
    REQUIRE_FALSE(bp.isRunning());

    bp.setState(AnimBPState::Error);
    REQUIRE(bp.hasError());
    REQUIRE_FALSE(bp.isReady());
}

TEST_CASE("AnimBlueprintEditor addBlueprint and duplicate rejection", "[Editor][S43]") {
    AnimBlueprintEditor editor;
    AnimBlueprintAsset a("a"), b("b"), dup("a");
    REQUIRE(editor.addBlueprint(a));
    REQUIRE(editor.addBlueprint(b));
    REQUIRE_FALSE(editor.addBlueprint(dup));
    REQUIRE(editor.blueprintCount() == 2);
}

TEST_CASE("AnimBlueprintEditor removeBlueprint clears activeBlueprint", "[Editor][S43]") {
    AnimBlueprintEditor editor;
    AnimBlueprintAsset bp("idle");
    editor.addBlueprint(bp);
    editor.setActiveBlueprint("idle");
    REQUIRE(editor.activeBlueprint() == "idle");

    editor.removeBlueprint("idle");
    REQUIRE(editor.blueprintCount() == 0);
    REQUIRE(editor.activeBlueprint().empty());
}

TEST_CASE("AnimBlueprintEditor findBlueprint returns pointer or nullptr", "[Editor][S43]") {
    AnimBlueprintEditor editor;
    AnimBlueprintAsset bp("walk");
    editor.addBlueprint(bp);

    REQUIRE(editor.findBlueprint("walk") != nullptr);
    REQUIRE(editor.findBlueprint("walk")->name() == "walk");
    REQUIRE(editor.findBlueprint("missing") == nullptr);
}

TEST_CASE("AnimBlueprintEditor aggregate counts", "[Editor][S43]") {
    AnimBlueprintEditor editor;

    AnimBlueprintAsset a("a"); a.setDirty(true); a.setState(AnimBPState::Running); a.setLooping(true); a.setOptimized(true); a.setNodeCount(30);
    AnimBlueprintAsset b("b"); b.setDirty(true); b.setNodeCount(5);
    AnimBlueprintAsset c("c"); c.setLooping(true); c.setOptimized(true); c.setNodeCount(25);

    editor.addBlueprint(a); editor.addBlueprint(b); editor.addBlueprint(c);

    REQUIRE(editor.dirtyCount()     == 2);
    REQUIRE(editor.runningCount()   == 1);
    REQUIRE(editor.loopingCount()   == 2);
    REQUIRE(editor.optimizedCount() == 2);
    REQUIRE(editor.complexCount()   == 2); // a and c
}

TEST_CASE("AnimBlueprintEditor countByNodeType and countByState", "[Editor][S43]") {
    AnimBlueprintEditor editor;

    AnimBlueprintAsset a("a"); a.setNodeType(AnimBPNodeType::BlendSpace); a.setState(AnimBPState::Running);
    AnimBlueprintAsset b("b"); b.setNodeType(AnimBPNodeType::BlendSpace); b.setState(AnimBPState::Inactive);
    AnimBlueprintAsset c("c"); c.setNodeType(AnimBPNodeType::Selector);   c.setState(AnimBPState::Running);

    editor.addBlueprint(a); editor.addBlueprint(b); editor.addBlueprint(c);

    REQUIRE(editor.countByNodeType(AnimBPNodeType::BlendSpace) == 2);
    REQUIRE(editor.countByNodeType(AnimBPNodeType::Selector)   == 1);
    REQUIRE(editor.countByState(AnimBPState::Running)          == 2);
    REQUIRE(editor.countByState(AnimBPState::Inactive)         == 1);
}

TEST_CASE("AnimBlueprintEditor setActiveBlueprint returns false for missing", "[Editor][S43]") {
    AnimBlueprintEditor editor;
    REQUIRE_FALSE(editor.setActiveBlueprint("ghost"));
    REQUIRE(editor.activeBlueprint().empty());
}

TEST_CASE("AnimBlueprintEditor MAX_BLUEPRINTS limit enforced", "[Editor][S43]") {
    AnimBlueprintEditor editor;
    for (size_t i = 0; i < AnimBlueprintEditor::MAX_BLUEPRINTS; ++i) {
        AnimBlueprintAsset bp("BP" + std::to_string(i));
        REQUIRE(editor.addBlueprint(bp));
    }
    AnimBlueprintAsset overflow("Overflow");
    REQUIRE_FALSE(editor.addBlueprint(overflow));
    REQUIRE(editor.blueprintCount() == AnimBlueprintEditor::MAX_BLUEPRINTS);
}
