#pragma once

#include <string>
#include <vector>
#include <map>
#include <cstdint>
#include <glm/glm.hpp>
#include <glm/gtc/quaternion.hpp>

namespace atlas {

enum class BodyPart : uint32_t { Head, Torso, ArmLeft, ArmRight, LegLeft, LegRight };

struct BodySlider {
    std::string name;
    float minValue;
    float maxValue;
    float currentValue;
    BodyPart targetPart;
};

struct MeshPiece {
    std::string meshFile;
    std::string material;
    glm::vec3 scale;
    glm::vec3 offset;
};

struct AstronautCharacter {
    int characterId{0};
    bool isFemale{false};
    glm::vec3 position{0.0f};
    glm::quat rotation{1.0f, 0.0f, 0.0f, 0.0f};
    std::vector<MeshPiece> bodyMeshes;
    std::vector<MeshPiece> accessories;
    std::vector<BodySlider> sliders;

    /// Path to a zip archive containing a reference base mesh (e.g. human.zip).
    std::string referenceMeshArchive;
    /// Uniform scale applied to the whole character mesh.
    float uniformScale{1.0f};
    /// Named morph-target weights (0..1) derived from sliders.
    std::map<std::string, float> morphWeights;
};

class CharacterMeshSystem {
public:
    AstronautCharacter generateCharacter(int seed, bool isFemale);
    void applySlider(AstronautCharacter& character, const std::string& sliderName, float value);
    void assembleMeshes(AstronautCharacter& character);

    /** Set a zip archive containing a reference base mesh for human-type characters. */
    void setReferenceMeshArchive(const std::string& archivePath);

    /** @return Current reference mesh archive path (empty if none). */
    const std::string& referenceMeshArchive() const;

private:
    std::string referenceMeshArchive_;
};

} // namespace atlas
