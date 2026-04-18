// S112 editor tests: CharacterCreatorEditor, CostumeEditor, MorphTargetEditor
#include <catch2/catch_test_macros.hpp>
#include "NF/Editor/Editor.h"
#include "NF/Editor/MorphTargetEditor.h"
#include "NF/Editor/CostumeEditor.h"
#include "NF/Editor/CharacterCreatorEditor.h"

using namespace NF;

// ── CharacterCreatorEditor ───────────────────────────────────────────────────

TEST_CASE("BodyPartCategory names", "[Editor][S112]") {
    REQUIRE(std::string(bodyPartCategoryName(BodyPartCategory::Head))  == "Head");
    REQUIRE(std::string(bodyPartCategoryName(BodyPartCategory::Torso)) == "Torso");
    REQUIRE(std::string(bodyPartCategoryName(BodyPartCategory::Arms))  == "Arms");
    REQUIRE(std::string(bodyPartCategoryName(BodyPartCategory::Legs))  == "Legs");
    REQUIRE(std::string(bodyPartCategoryName(BodyPartCategory::Feet))  == "Feet");
    REQUIRE(std::string(bodyPartCategoryName(BodyPartCategory::Hands)) == "Hands");
    REQUIRE(std::string(bodyPartCategoryName(BodyPartCategory::Face))  == "Face");
    REQUIRE(std::string(bodyPartCategoryName(BodyPartCategory::Hair))  == "Hair");
}

TEST_CASE("SkinTonePreset names", "[Editor][S112]") {
    REQUIRE(std::string(skinTonePresetName(SkinTonePreset::Pale))   == "Pale");
    REQUIRE(std::string(skinTonePresetName(SkinTonePreset::Fair))   == "Fair");
    REQUIRE(std::string(skinTonePresetName(SkinTonePreset::Medium)) == "Medium");
    REQUIRE(std::string(skinTonePresetName(SkinTonePreset::Tan))    == "Tan");
    REQUIRE(std::string(skinTonePresetName(SkinTonePreset::Brown))  == "Brown");
    REQUIRE(std::string(skinTonePresetName(SkinTonePreset::Dark))   == "Dark");
}

TEST_CASE("CharacterGender names", "[Editor][S112]") {
    REQUIRE(std::string(characterGenderName(CharacterGender::Male))      == "Male");
    REQUIRE(std::string(characterGenderName(CharacterGender::Female))    == "Female");
    REQUIRE(std::string(characterGenderName(CharacterGender::NonBinary)) == "NonBinary");
    REQUIRE(std::string(characterGenderName(CharacterGender::Undefined)) == "Undefined");
}

TEST_CASE("CharacterPreset defaults", "[Editor][S112]") {
    CharacterPreset p(1, "default_male", CharacterGender::Male);
    REQUIRE(p.id()           == 1u);
    REQUIRE(p.name()         == "default_male");
    REQUIRE(p.gender()       == CharacterGender::Male);
    REQUIRE(p.skinTone()     == SkinTonePreset::Medium);
    REQUIRE(p.height()       == 1.75f);
    REQUIRE(p.weight()       == 70.0f);
    REQUIRE(!p.isCustomized());
}

TEST_CASE("CharacterPreset mutation", "[Editor][S112]") {
    CharacterPreset p(2, "custom_female", CharacterGender::Female);
    p.setSkinTone(SkinTonePreset::Dark);
    p.setHeight(1.65f);
    p.setWeight(60.0f);
    p.setCustomized(true);
    REQUIRE(p.skinTone()     == SkinTonePreset::Dark);
    REQUIRE(p.height()       == 1.65f);
    REQUIRE(p.weight()       == 60.0f);
    REQUIRE(p.isCustomized());
}

TEST_CASE("CharacterCreatorEditor defaults", "[Editor][S112]") {
    CharacterCreatorEditor ed;
    REQUIRE(ed.activeBodyPart() == BodyPartCategory::Face);
    REQUIRE(ed.isShowSymmetry());
    REQUIRE(!ed.isShowGrid());
    REQUIRE(ed.zoom()          == 1.0f);
    REQUIRE(ed.presetCount()   == 0u);
}

TEST_CASE("CharacterCreatorEditor add/remove presets", "[Editor][S112]") {
    CharacterCreatorEditor ed;
    REQUIRE(ed.addPreset(CharacterPreset(1, "male_a",   CharacterGender::Male)));
    REQUIRE(ed.addPreset(CharacterPreset(2, "female_a", CharacterGender::Female)));
    REQUIRE(ed.addPreset(CharacterPreset(3, "nb_a",     CharacterGender::NonBinary)));
    REQUIRE(!ed.addPreset(CharacterPreset(1, "male_a",  CharacterGender::Male)));
    REQUIRE(ed.presetCount() == 3u);
    REQUIRE(ed.removePreset(2));
    REQUIRE(ed.presetCount() == 2u);
    REQUIRE(!ed.removePreset(99));
}

TEST_CASE("CharacterCreatorEditor counts and find", "[Editor][S112]") {
    CharacterCreatorEditor ed;
    CharacterPreset p1(1, "m1", CharacterGender::Male);
    CharacterPreset p2(2, "m2", CharacterGender::Male);
    CharacterPreset p3(3, "f1", CharacterGender::Female); p3.setCustomized(true);
    CharacterPreset p4(4, "nb", CharacterGender::NonBinary); p4.setCustomized(true);
    ed.addPreset(p1); ed.addPreset(p2); ed.addPreset(p3); ed.addPreset(p4);
    REQUIRE(ed.countByGender(CharacterGender::Male)      == 2u);
    REQUIRE(ed.countByGender(CharacterGender::Female)    == 1u);
    REQUIRE(ed.countByGender(CharacterGender::NonBinary) == 1u);
    REQUIRE(ed.countByGender(CharacterGender::Undefined) == 0u);
    REQUIRE(ed.countCustomized()                         == 2u);
    auto* found = ed.findPreset(3);
    REQUIRE(found != nullptr);
    REQUIRE(found->gender() == CharacterGender::Female);
    REQUIRE(ed.findPreset(99) == nullptr);
}

TEST_CASE("CharacterCreatorEditor settings mutation", "[Editor][S112]") {
    CharacterCreatorEditor ed;
    ed.setActiveBodyPart(BodyPartCategory::Hair);
    ed.setShowSymmetry(false);
    ed.setShowGrid(true);
    ed.setZoom(2.0f);
    REQUIRE(ed.activeBodyPart() == BodyPartCategory::Hair);
    REQUIRE(!ed.isShowSymmetry());
    REQUIRE(ed.isShowGrid());
    REQUIRE(ed.zoom() == 2.0f);
}

// ── CostumeEditor ────────────────────────────────────────────────────────────

TEST_CASE("CostumeSlot names", "[Editor][S112]") {
    REQUIRE(std::string(costumeSlotName(CostumeSlot::Hat))      == "Hat");
    REQUIRE(std::string(costumeSlotName(CostumeSlot::Top))      == "Top");
    REQUIRE(std::string(costumeSlotName(CostumeSlot::Bottom))   == "Bottom");
    REQUIRE(std::string(costumeSlotName(CostumeSlot::Shoes))    == "Shoes");
    REQUIRE(std::string(costumeSlotName(CostumeSlot::Gloves))   == "Gloves");
    REQUIRE(std::string(costumeSlotName(CostumeSlot::Belt))     == "Belt");
    REQUIRE(std::string(costumeSlotName(CostumeSlot::Necklace)) == "Necklace");
    REQUIRE(std::string(costumeSlotName(CostumeSlot::Earring))  == "Earring");
    REQUIRE(std::string(costumeSlotName(CostumeSlot::Backpack)) == "Backpack");
    REQUIRE(std::string(costumeSlotName(CostumeSlot::Cape))     == "Cape");
}

TEST_CASE("CostumeMaterial names", "[Editor][S112]") {
    REQUIRE(std::string(costumeMaterialName(CostumeMaterial::Cloth))     == "Cloth");
    REQUIRE(std::string(costumeMaterialName(CostumeMaterial::Leather))   == "Leather");
    REQUIRE(std::string(costumeMaterialName(CostumeMaterial::Metal))     == "Metal");
    REQUIRE(std::string(costumeMaterialName(CostumeMaterial::Silk))      == "Silk");
    REQUIRE(std::string(costumeMaterialName(CostumeMaterial::Fur))       == "Fur");
    REQUIRE(std::string(costumeMaterialName(CostumeMaterial::Synthetic)) == "Synthetic");
}

TEST_CASE("CostumeRarity names", "[Editor][S112]") {
    REQUIRE(std::string(costumeRarityName(CostumeRarity::Common))    == "Common");
    REQUIRE(std::string(costumeRarityName(CostumeRarity::Uncommon))  == "Uncommon");
    REQUIRE(std::string(costumeRarityName(CostumeRarity::Rare))      == "Rare");
    REQUIRE(std::string(costumeRarityName(CostumeRarity::Epic))      == "Epic");
    REQUIRE(std::string(costumeRarityName(CostumeRarity::Legendary)) == "Legendary");
}

TEST_CASE("CostumePiece defaults", "[Editor][S112]") {
    CostumePiece cp(1, "iron_helm", CostumeSlot::Hat);
    REQUIRE(cp.id()         == 1u);
    REQUIRE(cp.name()       == "iron_helm");
    REQUIRE(cp.slot()       == CostumeSlot::Hat);
    REQUIRE(cp.material()   == CostumeMaterial::Cloth);
    REQUIRE(cp.rarity()     == CostumeRarity::Common);
    REQUIRE(!cp.isEquipped());
    REQUIRE(!cp.isLocked());
    REQUIRE(cp.isDyeable());
}

TEST_CASE("CostumePiece mutation", "[Editor][S112]") {
    CostumePiece cp(2, "legendary_cape", CostumeSlot::Cape);
    cp.setMaterial(CostumeMaterial::Silk);
    cp.setRarity(CostumeRarity::Legendary);
    cp.setEquipped(true);
    cp.setLocked(true);
    cp.setDyeable(false);
    REQUIRE(cp.material()   == CostumeMaterial::Silk);
    REQUIRE(cp.rarity()     == CostumeRarity::Legendary);
    REQUIRE(cp.isEquipped());
    REQUIRE(cp.isLocked());
    REQUIRE(!cp.isDyeable());
}

TEST_CASE("CostumeEditor defaults", "[Editor][S112]") {
    CostumeEditor ed;
    REQUIRE(ed.isShowPreview());
    REQUIRE(ed.isShowStats());
    REQUIRE(!ed.isGridView());
    REQUIRE(ed.pieceCount() == 0u);
}

TEST_CASE("CostumeEditor add/remove pieces", "[Editor][S112]") {
    CostumeEditor ed;
    REQUIRE(ed.addPiece(CostumePiece(1, "hat1",    CostumeSlot::Hat)));
    REQUIRE(ed.addPiece(CostumePiece(2, "top1",    CostumeSlot::Top)));
    REQUIRE(ed.addPiece(CostumePiece(3, "bottom1", CostumeSlot::Bottom)));
    REQUIRE(!ed.addPiece(CostumePiece(1, "hat1",   CostumeSlot::Hat)));
    REQUIRE(ed.pieceCount() == 3u);
    REQUIRE(ed.removePiece(2));
    REQUIRE(ed.pieceCount() == 2u);
    REQUIRE(!ed.removePiece(99));
}

TEST_CASE("CostumeEditor counts and find", "[Editor][S112]") {
    CostumeEditor ed;
    CostumePiece c1(1, "hat_a",   CostumeSlot::Hat);
    CostumePiece c2(2, "hat_b",   CostumeSlot::Hat);  c2.setRarity(CostumeRarity::Epic);
    CostumePiece c3(3, "top_a",   CostumeSlot::Top);  c3.setEquipped(true);
    CostumePiece c4(4, "shoes_a", CostumeSlot::Shoes); c4.setEquipped(true); c4.setRarity(CostumeRarity::Rare);
    ed.addPiece(c1); ed.addPiece(c2); ed.addPiece(c3); ed.addPiece(c4);
    REQUIRE(ed.countBySlot(CostumeSlot::Hat)         == 2u);
    REQUIRE(ed.countBySlot(CostumeSlot::Top)         == 1u);
    REQUIRE(ed.countBySlot(CostumeSlot::Cape)        == 0u);
    REQUIRE(ed.countByRarity(CostumeRarity::Common)  == 2u);
    REQUIRE(ed.countByRarity(CostumeRarity::Epic)    == 1u);
    REQUIRE(ed.countByRarity(CostumeRarity::Rare)    == 1u);
    REQUIRE(ed.countEquipped()                       == 2u);
    auto* found = ed.findPiece(3);
    REQUIRE(found != nullptr);
    REQUIRE(found->slot() == CostumeSlot::Top);
    REQUIRE(ed.findPiece(99) == nullptr);
}

TEST_CASE("CostumeEditor settings mutation", "[Editor][S112]") {
    CostumeEditor ed;
    ed.setShowPreview(false);
    ed.setShowStats(false);
    ed.setGridView(true);
    REQUIRE(!ed.isShowPreview());
    REQUIRE(!ed.isShowStats());
    REQUIRE(ed.isGridView());
}

// ── MorphTargetEditor ────────────────────────────────────────────────────────

TEST_CASE("MorphCategory names", "[Editor][S112]") {
    REQUIRE(std::string(morphCategoryName(MorphCategory::Expression)) == "Expression");
    REQUIRE(std::string(morphCategoryName(MorphCategory::BodyShape))  == "BodyShape");
    REQUIRE(std::string(morphCategoryName(MorphCategory::Facial))     == "Facial");
    REQUIRE(std::string(morphCategoryName(MorphCategory::Corrective)) == "Corrective");
    REQUIRE(std::string(morphCategoryName(MorphCategory::Blend))      == "Blend");
    REQUIRE(std::string(morphCategoryName(MorphCategory::Custom))     == "Custom");
}

TEST_CASE("MorphBlendMode names", "[Editor][S112]") {
    REQUIRE(std::string(morphBlendModeName(MorphBlendMode::Additive))  == "Additive");
    REQUIRE(std::string(morphBlendModeName(MorphBlendMode::Replace))   == "Replace");
    REQUIRE(std::string(morphBlendModeName(MorphBlendMode::Multiply))  == "Multiply");
    REQUIRE(std::string(morphBlendModeName(MorphBlendMode::Average))   == "Average");
}

TEST_CASE("MorphTarget defaults", "[Editor][S112]") {
    MorphTarget mt(1, "smile", MorphCategory::Expression);
    REQUIRE(mt.id()          == 1u);
    REQUIRE(mt.name()        == "smile");
    REQUIRE(mt.category()    == MorphCategory::Expression);
    REQUIRE(mt.blendMode()   == MorphBlendMode::Additive);
    REQUIRE(mt.weight()      == 0.0f);
    REQUIRE(mt.minWeight()   == -1.0f);
    REQUIRE(mt.maxWeight()   == 1.0f);
    REQUIRE(!mt.isSymmetric());
}

TEST_CASE("MorphTarget mutation", "[Editor][S112]") {
    MorphTarget mt(2, "bulge_arm", MorphCategory::BodyShape);
    mt.setBlendMode(MorphBlendMode::Replace);
    mt.setWeight(0.5f);
    mt.setMinWeight(0.0f);
    mt.setMaxWeight(2.0f);
    mt.setSymmetric(true);
    REQUIRE(mt.blendMode()   == MorphBlendMode::Replace);
    REQUIRE(mt.weight()      == 0.5f);
    REQUIRE(mt.minWeight()   == 0.0f);
    REQUIRE(mt.maxWeight()   == 2.0f);
    REQUIRE(mt.isSymmetric());
}

TEST_CASE("MorphTargetEditor defaults", "[Editor][S112]") {
    MorphTargetEditor ed;
    REQUIRE(ed.isLivePreview());
    REQUIRE(ed.isShowMesh());
    REQUIRE(!ed.isNormalize());
    REQUIRE(ed.previewFrameRate() == 30.0f);
    REQUIRE(ed.targetCount()      == 0u);
}

TEST_CASE("MorphTargetEditor add/remove targets", "[Editor][S112]") {
    MorphTargetEditor ed;
    REQUIRE(ed.addTarget(MorphTarget(1, "smile",     MorphCategory::Expression)));
    REQUIRE(ed.addTarget(MorphTarget(2, "frown",     MorphCategory::Expression)));
    REQUIRE(ed.addTarget(MorphTarget(3, "arm_bulge", MorphCategory::BodyShape)));
    REQUIRE(!ed.addTarget(MorphTarget(1, "smile",    MorphCategory::Expression)));
    REQUIRE(ed.targetCount() == 3u);
    REQUIRE(ed.removeTarget(2));
    REQUIRE(ed.targetCount() == 2u);
    REQUIRE(!ed.removeTarget(99));
}

TEST_CASE("MorphTargetEditor counts and find", "[Editor][S112]") {
    MorphTargetEditor ed;
    MorphTarget t1(1, "smile",      MorphCategory::Expression);
    MorphTarget t2(2, "frown",      MorphCategory::Expression);
    MorphTarget t3(3, "brow_raise", MorphCategory::Facial);   t3.setBlendMode(MorphBlendMode::Replace);
    MorphTarget t4(4, "body_slim",  MorphCategory::BodyShape); t4.setBlendMode(MorphBlendMode::Replace);
    ed.addTarget(t1); ed.addTarget(t2); ed.addTarget(t3); ed.addTarget(t4);
    REQUIRE(ed.countByCategory(MorphCategory::Expression) == 2u);
    REQUIRE(ed.countByCategory(MorphCategory::Facial)     == 1u);
    REQUIRE(ed.countByCategory(MorphCategory::Corrective) == 0u);
    REQUIRE(ed.countByBlendMode(MorphBlendMode::Additive) == 2u);
    REQUIRE(ed.countByBlendMode(MorphBlendMode::Replace)  == 2u);
    auto* found = ed.findTarget(3);
    REQUIRE(found != nullptr);
    REQUIRE(found->category() == MorphCategory::Facial);
    REQUIRE(ed.findTarget(99) == nullptr);
}

TEST_CASE("MorphTargetEditor settings mutation", "[Editor][S112]") {
    MorphTargetEditor ed;
    ed.setLivePreview(false);
    ed.setShowMesh(false);
    ed.setNormalize(true);
    ed.setPreviewFrameRate(60.0f);
    REQUIRE(!ed.isLivePreview());
    REQUIRE(!ed.isShowMesh());
    REQUIRE(ed.isNormalize());
    REQUIRE(ed.previewFrameRate() == 60.0f);
}
