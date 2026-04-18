#pragma once

#include <cstdint>
#include <string>

namespace NF::Patch {

enum class RigMechState : std::uint8_t {
    IdleRig,
    ArmedForDeploy,
    Transforming,
    MechActive,
    MechDamaged,
    Retracting
};

struct RigMechRuntimeState {
    RigMechState state = RigMechState::IdleRig;
    float transformationCharge = 0.0f;
    float mechHull = 0.0f;
    float mechHeat = 0.0f;
    bool emergencyEjectAvailable = false;
};

} // namespace NF::Patch
