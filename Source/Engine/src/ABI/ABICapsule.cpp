#include "ABICapsule.h"

namespace atlas::abi {

// ---------------------------------------------------------------------------
// ABIVersion
// ---------------------------------------------------------------------------

bool ABIVersion::operator==(const ABIVersion& other) const {
    return major == other.major && minor == other.minor;
}

bool ABIVersion::operator!=(const ABIVersion& other) const {
    return !(*this == other);
}

bool ABIVersion::operator<(const ABIVersion& other) const {
    if (major != other.major)
        return major < other.major;
    return minor < other.minor;
}

std::string ABIVersion::ToString() const {
    return "atlas_abi_v" + std::to_string(major) + "_" + std::to_string(minor);
}

ABIVersion ABIVersion::FromString(const std::string& str) {
    ABIVersion version;

    const std::string prefix = "atlas_abi_v";
    if (!str.starts_with(prefix))
        return version;

    std::string remainder = str.substr(prefix.size());
    auto sep = remainder.find('_');

    try {
        if (sep == std::string::npos) {
            version.major = static_cast<uint32_t>(std::stoul(remainder));
            version.minor = 0;
        } else {
            version.major = static_cast<uint32_t>(std::stoul(remainder.substr(0, sep)));
            version.minor = static_cast<uint32_t>(std::stoul(remainder.substr(sep + 1)));
        }
    } catch (...) {
        return ABIVersion{};
    }

    return version;
}

bool ABIVersion::IsCompatibleWith(const ABIVersion& other) const {
    return major == other.major;
}

// ---------------------------------------------------------------------------
// ABIFunctionTable
// ---------------------------------------------------------------------------

bool ABIFunctionTable::IsComplete() const {
    return RegisterSystem
        && StepSimulation
        && GetCurrentTick
        && GetComponent
        && SetComponent
        && SubmitInput
        && EmitEvent
        && GetWorldHash;
}

uint32_t ABIFunctionTable::BoundCount() const {
    uint32_t count = 0;
    if (RegisterSystem)  ++count;
    if (StepSimulation)  ++count;
    if (GetCurrentTick)  ++count;
    if (GetComponent)    ++count;
    if (SetComponent)    ++count;
    if (SubmitInput)     ++count;
    if (EmitEvent)       ++count;
    if (GetWorldHash)    ++count;
    return count;
}

// ---------------------------------------------------------------------------
// ABICapsule
// ---------------------------------------------------------------------------

ABICapsule::ABICapsule(ABIVersion version, const std::string& description)
    : m_version(version)
    , m_description(description) {
}

const ABIVersion& ABICapsule::Version() const {
    return m_version;
}

const std::string& ABICapsule::Description() const {
    return m_description;
}

ABIFunctionTable& ABICapsule::Functions() {
    return m_functions;
}

const ABIFunctionTable& ABICapsule::Functions() const {
    return m_functions;
}

bool ABICapsule::IsReady() const {
    return m_functions.IsComplete();
}

void ABICapsule::Seal() {
    m_sealed = true;
}

bool ABICapsule::IsSealed() const {
    return m_sealed;
}

} // namespace atlas::abi
