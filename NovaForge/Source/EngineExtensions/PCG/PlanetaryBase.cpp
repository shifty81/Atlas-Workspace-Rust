#include "PlanetaryBase.h"
#include <algorithm>
#include <cmath>

namespace atlas::procedural {

// ---- BaseZone ----

float BaseZone::Area() const {
    static constexpr float PI = 3.14159265358979323846f;
    return PI * radius * radius;
}

// ---- PlanetaryBase ----

void PlanetaryBase::Init(const PlanetaryBaseConfig& cfg) {
    m_config = cfg;
    m_zones.clear();
    m_nextId = 1;
}

uint32_t PlanetaryBase::AddZone(BaseZoneType type, float x, float y, float radius) {
    if (m_zones.size() >= m_config.maxZones) return 0;
    if (radius <= 0.0f) return 0;

    // Check within base radius.
    float dist = std::sqrt(x * x + y * y);
    if (dist + radius > m_config.baseRadius) return 0;

    if (!IsZoneSpacingValid(x, y, radius)) return 0;

    BaseZone zone;
    zone.id = m_nextId++;
    zone.type = type;
    zone.x = x;
    zone.y = y;
    zone.radius = radius;
    zone.level = 1;
    zone.operational = true;

    // Auto-generate name from type.
    static const char* typeNames[] = {
        "Landing", "Habitat", "Mining", "Refinery", "Defense",
        "Research", "Power", "Communication", "Storage", "Medical"
    };
    uint8_t idx = static_cast<uint8_t>(type);
    zone.name = (idx < 10) ? typeNames[idx] : "Zone";
    zone.name += " " + std::to_string(zone.id);

    m_zones.push_back(zone);
    return zone.id;
}

void PlanetaryBase::RemoveZone(uint32_t zoneId) {
    m_zones.erase(
        std::remove_if(m_zones.begin(), m_zones.end(),
                       [zoneId](const BaseZone& z) { return z.id == zoneId; }),
        m_zones.end());
}

const BaseZone* PlanetaryBase::GetZone(uint32_t zoneId) const {
    for (auto& z : m_zones) {
        if (z.id == zoneId) return &z;
    }
    return nullptr;
}

std::vector<uint32_t> PlanetaryBase::FindZonesByType(BaseZoneType type) const {
    std::vector<uint32_t> result;
    for (auto& z : m_zones) {
        if (z.type == type) result.push_back(z.id);
    }
    return result;
}

void PlanetaryBase::UpgradeZone(uint32_t zoneId) {
    for (auto& z : m_zones) {
        if (z.id == zoneId) {
            if (z.level < 5) ++z.level;
            return;
        }
    }
}

bool PlanetaryBase::HasRequiredZones() const {
    bool hasLanding = false, hasPower = false, hasHabitat = false;
    for (auto& z : m_zones) {
        if (z.type == BaseZoneType::Landing) hasLanding = true;
        if (z.type == BaseZoneType::Power) hasPower = true;
        if (z.type == BaseZoneType::Habitat) hasHabitat = true;
    }
    return hasLanding && hasPower && hasHabitat;
}

bool PlanetaryBase::IsZoneSpacingValid(float x, float y, float radius) const {
    for (auto& z : m_zones) {
        float dx = x - z.x;
        float dy = y - z.y;
        float dist = std::sqrt(dx * dx + dy * dy);
        float minDist = radius + z.radius + m_config.minZoneSpacing;
        if (dist < minDist) return false;
    }
    return true;
}

void PlanetaryBase::Generate(uint64_t seed) {
    Clear();
    DeterministicRNG rng(seed);

    float maxR = m_config.baseRadius;
    float zoneRadius = maxR * 0.08f;

    // Place required zones first: Landing at center, Power and Habitat nearby.
    AddZone(BaseZoneType::Landing, 0.0f, 0.0f, zoneRadius);

    auto placeNear = [&](BaseZoneType type, float distFromCenter) {
        for (int attempt = 0; attempt < 50; ++attempt) {
            float angle = rng.NextFloat() * 6.28318530718f;
            float r = distFromCenter + rng.NextFloatRange(-zoneRadius, zoneRadius);
            float px = std::cos(angle) * r;
            float py = std::sin(angle) * r;
            if (AddZone(type, px, py, zoneRadius) != 0) return;
        }
    };

    placeNear(BaseZoneType::Power, zoneRadius * 4.0f);
    placeNear(BaseZoneType::Habitat, zoneRadius * 4.0f);

    // Fill remaining zones randomly.
    static const BaseZoneType extraTypes[] = {
        BaseZoneType::Mining, BaseZoneType::Refinery, BaseZoneType::Defense,
        BaseZoneType::Research, BaseZoneType::Communication,
        BaseZoneType::Storage, BaseZoneType::Medical
    };
    uint32_t extraTypeCount = 7;

    while (m_zones.size() < m_config.maxZones) {
        bool placed = false;
        for (int attempt = 0; attempt < 100; ++attempt) {
            float angle = rng.NextFloat() * 6.28318530718f;
            float dist = rng.NextFloatRange(zoneRadius * 2.0f, maxR - zoneRadius);
            float px = std::cos(angle) * dist;
            float py = std::sin(angle) * dist;
            BaseZoneType t = extraTypes[rng.NextU32(extraTypeCount)];
            if (AddZone(t, px, py, zoneRadius) != 0) { placed = true; break; }
        }
        if (!placed) break;  // Could not find valid placement.
    }
}

void PlanetaryBase::Clear() {
    m_zones.clear();
    m_nextId = 1;
}

size_t PlanetaryBase::OperationalZoneCount() const {
    size_t count = 0;
    for (auto& z : m_zones) {
        if (z.operational) ++count;
    }
    return count;
}

float PlanetaryBase::TotalArea() const {
    float total = 0.0f;
    for (auto& z : m_zones) {
        total += z.Area();
    }
    return total;
}

} // namespace atlas::procedural
