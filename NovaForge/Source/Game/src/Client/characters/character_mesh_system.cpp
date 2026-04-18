#include "characters/character_mesh_system.h"
#include <algorithm>

namespace atlas {

AstronautCharacter CharacterMeshSystem::generateCharacter(int seed, bool isFemale) {
    AstronautCharacter character;
    character.characterId = seed;
    character.isFemale = isFemale;
    character.position = glm::vec3(0.0f);
    character.rotation = glm::quat(1.0f, 0.0f, 0.0f, 0.0f);

    // Default body meshes for each part
    character.bodyMeshes = {
        {"Head_Default.obj",     "astronaut_base", glm::vec3(1.0f), glm::vec3(0.0f, 1.6f, 0.0f)},
        {"Torso_Default.obj",    "astronaut_base", glm::vec3(1.0f), glm::vec3(0.0f, 1.1f, 0.0f)},
        {"ArmLeft_Default.obj",  "astronaut_base", glm::vec3(1.0f), glm::vec3(-0.35f, 1.2f, 0.0f)},
        {"ArmRight_Default.obj", "astronaut_base", glm::vec3(1.0f), glm::vec3(0.35f, 1.2f, 0.0f)},
        {"LegLeft_Default.obj",  "astronaut_base", glm::vec3(1.0f), glm::vec3(-0.12f, 0.5f, 0.0f)},
        {"LegRight_Default.obj", "astronaut_base", glm::vec3(1.0f), glm::vec3(0.12f, 0.5f, 0.0f)}
    };

    // Default body sliders
    character.sliders = {
        {"height",     0.85f, 1.15f, 1.0f, BodyPart::Torso},
        {"torsoWidth", 0.8f,  1.2f,  1.0f, BodyPart::Torso},
        {"armLength",  0.85f, 1.15f, 1.0f, BodyPart::ArmLeft},
        {"legLength",  0.85f, 1.15f, 1.0f, BodyPart::LegLeft}
    };

    // No accessories by default
    character.accessories.clear();

    // Carry reference mesh archive if one has been configured.
    character.referenceMeshArchive = referenceMeshArchive_;
    character.uniformScale = 1.0f;

    // Seed initial morph weights from default slider values.
    for (const auto& sl : character.sliders) {
        float normalized = (sl.currentValue - sl.minValue) / (sl.maxValue - sl.minValue);
        character.morphWeights[sl.name] = normalized;
    }

    return character;
}

void CharacterMeshSystem::applySlider(AstronautCharacter& character,
                                       const std::string& sliderName, float value) {
    for (auto& slider : character.sliders) {
        if (slider.name != sliderName)
            continue;

        slider.currentValue = std::clamp(value, slider.minValue, slider.maxValue);

        // Update the corresponding mesh scale
        auto partIndex = static_cast<size_t>(slider.targetPart);
        if (partIndex < character.bodyMeshes.size()) {
            auto& mesh = character.bodyMeshes[partIndex];
            if (sliderName == "height") {
                mesh.scale.y = slider.currentValue;
            } else if (sliderName == "torsoWidth") {
                mesh.scale.x = slider.currentValue;
                mesh.scale.z = slider.currentValue;
            } else if (sliderName == "armLength") {
                mesh.scale.y = slider.currentValue;
                // Mirror to right arm
                auto rightIndex = static_cast<size_t>(BodyPart::ArmRight);
                if (rightIndex < character.bodyMeshes.size()) {
                    character.bodyMeshes[rightIndex].scale.y = slider.currentValue;
                }
            } else if (sliderName == "legLength") {
                mesh.scale.y = slider.currentValue;
                // Mirror to right leg
                auto rightIndex = static_cast<size_t>(BodyPart::LegRight);
                if (rightIndex < character.bodyMeshes.size()) {
                    character.bodyMeshes[rightIndex].scale.y = slider.currentValue;
                }
            }
        }

        // Update the morph weight that corresponds to this slider.
        float normalized = (slider.currentValue - slider.minValue)
                         / (slider.maxValue - slider.minValue);
        character.morphWeights[sliderName] = normalized;

        // Recompute uniform scale from the height slider relative to
        // the default value (1.0) so it matches the server convention.
        if (sliderName == "height") {
            constexpr float DEFAULT_HEIGHT_VALUE = 1.0f;
            character.uniformScale = slider.currentValue / DEFAULT_HEIGHT_VALUE;
        }

        return;
    }
}

void CharacterMeshSystem::assembleMeshes(AstronautCharacter& /*character*/) {
    // Stub: actual mesh combining happens in the render layer
}

void CharacterMeshSystem::setReferenceMeshArchive(const std::string& archivePath) {
    referenceMeshArchive_ = archivePath;
}

const std::string& CharacterMeshSystem::referenceMeshArchive() const {
    return referenceMeshArchive_;
}

} // namespace atlas
