#include "ShipHullNodes.h"
#include <cmath>
#include <algorithm>

namespace atlas::procedural {

static const float PI = 3.14159265358979323846f;

// Deterministic xorshift64 RNG
static uint64_t Xorshift64(uint64_t& state) {
    state ^= state << 13;
    state ^= state >> 7;
    state ^= state << 17;
    return state;
}

static float RandFloat(uint64_t& state) {
    return static_cast<float>(Xorshift64(state) & 0xFFFFu) / 65535.0f;
}

static float RandRange(uint64_t& state, float lo, float hi) {
    return lo + RandFloat(state) * (hi - lo);
}

static float SafeStofLocal(const std::string& s, float def) {
    try { return s.empty() ? def : std::stof(s); }
    catch (...) { return def; }
}

MeshData GenerateHullFromControlPoints(const std::vector<HullControlPoint>& points,
                                       int segments) {
    MeshData mesh;
    if (points.size() < 2 || segments < 3) return mesh;

    int n = static_cast<int>(points.size());
    int segs = segments;

    // For each control point, generate a ring of vertices
    for (int i = 0; i < n; ++i) {
        const auto& cp = points[i];
        float r = cp.radius;

        for (int s = 0; s < segs; ++s) {
            float theta = 2.0f * PI * static_cast<float>(s) / static_cast<float>(segs);
            float cy = std::cos(theta) * r;
            float cz = std::sin(theta) * r;

            mesh.vertices.push_back(cp.position.x);
            mesh.vertices.push_back(cp.position.y + cy);
            mesh.vertices.push_back(cp.position.z + cz);

            // Normal points outward from spine
            float len = std::sqrt(cy * cy + cz * cz);
            if (len > 0.0f) {
                mesh.normals.push_back(0.0f);
                mesh.normals.push_back(cy / len);
                mesh.normals.push_back(cz / len);
            } else {
                mesh.normals.push_back(0.0f);
                mesh.normals.push_back(1.0f);
                mesh.normals.push_back(0.0f);
            }
        }
    }

    // Connect adjacent rings with quads (two triangles each)
    for (int i = 0; i < n - 1; ++i) {
        for (int s = 0; s < segs; ++s) {
            uint32_t curr = static_cast<uint32_t>(i * segs + s);
            uint32_t next = static_cast<uint32_t>(i * segs + ((s + 1) % segs));
            uint32_t currNext = static_cast<uint32_t>((i + 1) * segs + s);
            uint32_t nextNext = static_cast<uint32_t>((i + 1) * segs + ((s + 1) % segs));

            mesh.indices.push_back(curr);
            mesh.indices.push_back(currNext);
            mesh.indices.push_back(next);

            mesh.indices.push_back(next);
            mesh.indices.push_back(currNext);
            mesh.indices.push_back(nextNext);
        }
    }

    // Cap the front (first ring) with a fan
    uint32_t frontCenter = static_cast<uint32_t>(mesh.VertexCount());
    mesh.vertices.push_back(points.front().position.x);
    mesh.vertices.push_back(points.front().position.y);
    mesh.vertices.push_back(points.front().position.z);
    mesh.normals.push_back(-1.0f);
    mesh.normals.push_back(0.0f);
    mesh.normals.push_back(0.0f);

    for (int s = 0; s < segs; ++s) {
        uint32_t a = static_cast<uint32_t>(s);
        uint32_t b = static_cast<uint32_t>((s + 1) % segs);
        mesh.indices.push_back(frontCenter);
        mesh.indices.push_back(b);
        mesh.indices.push_back(a);
    }

    // Cap the back (last ring) with a fan
    uint32_t backCenter = static_cast<uint32_t>(mesh.VertexCount());
    mesh.vertices.push_back(points.back().position.x);
    mesh.vertices.push_back(points.back().position.y);
    mesh.vertices.push_back(points.back().position.z);
    mesh.normals.push_back(1.0f);
    mesh.normals.push_back(0.0f);
    mesh.normals.push_back(0.0f);

    int lastRingStart = (n - 1) * segs;
    for (int s = 0; s < segs; ++s) {
        uint32_t a = static_cast<uint32_t>(lastRingStart + s);
        uint32_t b = static_cast<uint32_t>(lastRingStart + (s + 1) % segs);
        mesh.indices.push_back(backCenter);
        mesh.indices.push_back(a);
        mesh.indices.push_back(b);
    }

    return mesh;
}

MeshData GenerateTurretMount(const Vec3& position, const Vec3& direction,
                             const std::string& size) {
    MeshData mesh;
    float baseRadius = 0.15f;
    float height = 0.2f;

    if (size == "medium") { baseRadius = 0.25f; height = 0.3f; }
    else if (size == "large") { baseRadius = 0.4f; height = 0.45f; }
    else if (size == "capital") { baseRadius = 0.6f; height = 0.6f; }

    int segs = 6;
    // Bottom ring
    for (int i = 0; i < segs; ++i) {
        float theta = 2.0f * PI * static_cast<float>(i) / static_cast<float>(segs);
        float cx = std::cos(theta) * baseRadius;
        float cz = std::sin(theta) * baseRadius;
        mesh.vertices.push_back(position.x + cx);
        mesh.vertices.push_back(position.y);
        mesh.vertices.push_back(position.z + cz);
        mesh.normals.push_back(cx);
        mesh.normals.push_back(0.0f);
        mesh.normals.push_back(cz);
    }

    // Top ring (slightly smaller for taper)
    float topRadius = baseRadius * 0.7f;
    for (int i = 0; i < segs; ++i) {
        float theta = 2.0f * PI * static_cast<float>(i) / static_cast<float>(segs);
        float cx = std::cos(theta) * topRadius;
        float cz = std::sin(theta) * topRadius;
        mesh.vertices.push_back(position.x + cx);
        mesh.vertices.push_back(position.y + height);
        mesh.vertices.push_back(position.z + cz);
        mesh.normals.push_back(cx);
        mesh.normals.push_back(0.3f);
        mesh.normals.push_back(cz);
    }

    // Side triangles
    for (int i = 0; i < segs; ++i) {
        uint32_t b0 = static_cast<uint32_t>(i);
        uint32_t b1 = static_cast<uint32_t>((i + 1) % segs);
        uint32_t t0 = static_cast<uint32_t>(segs + i);
        uint32_t t1 = static_cast<uint32_t>(segs + (i + 1) % segs);

        mesh.indices.push_back(b0);
        mesh.indices.push_back(b1);
        mesh.indices.push_back(t0);

        mesh.indices.push_back(t0);
        mesh.indices.push_back(b1);
        mesh.indices.push_back(t1);
    }

    // Barrel (small cylinder pointing in aim direction)
    uint32_t barrelBase = static_cast<uint32_t>(mesh.VertexCount());
    float barrelRadius = topRadius * 0.3f;
    float barrelLen = height * 1.5f;
    // Use direction to orient barrel
    float dx = direction.x, dy = direction.y, dz = direction.z;
    float dLen = std::sqrt(dx * dx + dy * dy + dz * dz);
    if (dLen > 0.0f) { dx /= dLen; dy /= dLen; dz /= dLen; }
    else { dx = 0.0f; dy = 1.0f; dz = 0.0f; }

    // Barrel tip
    mesh.vertices.push_back(position.x + dx * barrelLen);
    mesh.vertices.push_back(position.y + height + dy * barrelLen);
    mesh.vertices.push_back(position.z + dz * barrelLen);
    mesh.normals.push_back(dx);
    mesh.normals.push_back(dy);
    mesh.normals.push_back(dz);

    // Connect top ring to barrel tip
    for (int i = 0; i < segs; ++i) {
        uint32_t t0 = static_cast<uint32_t>(segs + i);
        uint32_t t1 = static_cast<uint32_t>(segs + (i + 1) % segs);
        mesh.indices.push_back(t0);
        mesh.indices.push_back(t1);
        mesh.indices.push_back(barrelBase);
    }

    return mesh;
}

MeshData GenerateLightFixture(const Vec3& position, float range, bool interior) {
    MeshData mesh;
    // Small diamond shape to represent light position
    float s = interior ? 0.05f : 0.08f;

    // 6 vertices: +X, -X, +Y, -Y, +Z, -Z
    float verts[] = {
        position.x + s, position.y,     position.z,
        position.x - s, position.y,     position.z,
        position.x,     position.y + s, position.z,
        position.x,     position.y - s, position.z,
        position.x,     position.y,     position.z + s,
        position.x,     position.y,     position.z - s
    };
    mesh.vertices.assign(verts, verts + 18);

    float norms[] = {
         1, 0, 0,  -1, 0, 0,
         0, 1, 0,   0,-1, 0,
         0, 0, 1,   0, 0,-1
    };
    mesh.normals.assign(norms, norms + 18);

    // 8 triangles forming octahedron
    uint32_t idx[] = {
        0, 2, 4,  0, 4, 3,  0, 3, 5,  0, 5, 2,
        1, 4, 2,  1, 3, 4,  1, 5, 3,  1, 2, 5
    };
    mesh.indices.assign(idx, idx + 24);

    return mesh;
}

MeshData GenerateInteriorVolume(const Vec3& boundsMin, const Vec3& boundsMax) {
    MeshData mesh;
    float x0 = boundsMin.x, y0 = boundsMin.y, z0 = boundsMin.z;
    float x1 = boundsMax.x, y1 = boundsMax.y, z1 = boundsMax.z;

    // 8 corners of the box
    float verts[] = {
        x0, y0, z0,   x1, y0, z0,   x1, y1, z0,  x0, y1, z0,
        x0, y0, z1,   x1, y0, z1,   x1, y1, z1,  x0, y1, z1
    };
    mesh.vertices.assign(verts, verts + 24);

    // Inward-facing normals for interior walls
    mesh.normals.resize(24);
    for (size_t i = 0; i < 24; i += 3) {
        float cx = (x0 + x1) * 0.5f;
        float cy = (y0 + y1) * 0.5f;
        float cz = (z0 + z1) * 0.5f;
        float nx = cx - mesh.vertices[i];
        float ny = cy - mesh.vertices[i + 1];
        float nz = cz - mesh.vertices[i + 2];
        float len = std::sqrt(nx * nx + ny * ny + nz * nz);
        if (len > 0.0f) { nx /= len; ny /= len; nz /= len; }
        mesh.normals[i] = nx;
        mesh.normals[i + 1] = ny;
        mesh.normals[i + 2] = nz;
    }

    // 12 triangles (6 faces × 2)
    uint32_t idx[] = {
        0, 2, 1,  0, 3, 2,   // back
        4, 5, 6,  4, 6, 7,   // front
        0, 1, 5,  0, 5, 4,   // bottom
        3, 6, 2,  3, 7, 6,   // top
        0, 4, 7,  0, 7, 3,   // left
        1, 2, 6,  1, 6, 5    // right
    };
    mesh.indices.assign(idx, idx + 36);

    return mesh;
}

MeshData GenerateVisualFeature(const VisualFeature& feature) {
    MeshData mesh;
    float sx = feature.scale.x > 0.0f ? feature.scale.x : 0.3f;
    float sy = feature.scale.y > 0.0f ? feature.scale.y : 0.1f;
    float sz = feature.scale.z > 0.0f ? feature.scale.z : 0.5f;

    float px = feature.position.x;
    float py = feature.position.y;
    float pz = feature.position.z;

    // Simple scaled wedge shape for most features
    // Front is tapered, back is full width
    float verts[] = {
        px,          py,          pz - sz,      // 0: front tip bottom
        px,          py + sy,     pz - sz,      // 1: front tip top
        px - sx / 2, py,          pz + sz,      // 2: back left bottom
        px + sx / 2, py,          pz + sz,      // 3: back right bottom
        px - sx / 2, py + sy,     pz + sz,      // 4: back left top
        px + sx / 2, py + sy,     pz + sz       // 5: back right top
    };
    mesh.vertices.assign(verts, verts + 18);

    // Approximate normals
    float norms[] = {
         0,  0, -1,
         0,  0, -1,
        -0.7f, 0, 0.7f,
         0.7f, 0, 0.7f,
        -0.7f, 0, 0.7f,
         0.7f, 0, 0.7f
    };
    mesh.normals.assign(norms, norms + 18);

    uint32_t idx[] = {
        0, 3, 2,    // bottom
        0, 1, 5,  0, 5, 3,    // right side
        0, 2, 4,  0, 4, 1,    // left side
        1, 4, 5,    // top
        2, 3, 5,  2, 5, 4     // back
    };
    mesh.indices.assign(idx, idx + 24);

    return mesh;
}

std::vector<HullControlPoint> GenerateDefaultControlPoints(uint64_t seed,
                                                            const std::string& shipClass) {
    std::vector<HullControlPoint> points;
    uint64_t state = seed ^ 0x5DEECE66DULL;
    if (state == 0) state = 1;

    int numPoints = 5;
    float length = 10.0f;
    float maxRadius = 1.5f;

    if (shipClass == "Frigate") { numPoints = 5; length = 8.0f; maxRadius = 1.0f; }
    else if (shipClass == "Destroyer") { numPoints = 6; length = 12.0f; maxRadius = 1.3f; }
    else if (shipClass == "Cruiser") { numPoints = 7; length = 18.0f; maxRadius = 2.0f; }
    else if (shipClass == "Battlecruiser") { numPoints = 8; length = 24.0f; maxRadius = 2.5f; }
    else if (shipClass == "Battleship") { numPoints = 9; length = 32.0f; maxRadius = 3.5f; }
    else if (shipClass == "Capital") { numPoints = 12; length = 60.0f; maxRadius = 8.0f; }

    uint32_t cpId = 1;
    for (int i = 0; i < numPoints; ++i) {
        HullControlPoint cp;
        cp.id = cpId++;
        float t = static_cast<float>(i) / static_cast<float>(numPoints - 1);
        cp.position.x = -length / 2.0f + t * length;
        cp.position.y = RandRange(state, -0.2f, 0.2f);
        cp.position.z = RandRange(state, -0.1f, 0.1f);

        // Tapered at front and back, widest in middle
        float profile = 4.0f * t * (1.0f - t); // parabolic [0,1] peak at 0.5
        cp.radius = maxRadius * profile * RandRange(state, 0.8f, 1.2f);
        if (cp.radius < 0.1f) cp.radius = 0.1f;

        cp.weight = 1.0f;
        points.push_back(cp);
    }

    return points;
}

std::vector<TurretHardpoint> GenerateDefaultHardpoints(uint64_t seed,
                                                        const std::string& shipClass,
                                                        int turretSlots,
                                                        int launcherSlots) {
    std::vector<TurretHardpoint> hardpoints;
    uint64_t state = seed ^ 0xCAFEBABEull;
    if (state == 0) state = 1;

    std::string turretSize = "small";
    if (shipClass == "Cruiser" || shipClass == "Battlecruiser") turretSize = "medium";
    else if (shipClass == "Battleship") turretSize = "large";
    else if (shipClass == "Capital") turretSize = "capital";

    uint32_t hpId = 1;
    float spacing = 2.0f;

    // Turrets along dorsal (top) of hull
    for (int i = 0; i < turretSlots; ++i) {
        TurretHardpoint hp;
        hp.id = hpId++;
        float t = (turretSlots > 1)
            ? static_cast<float>(i) / static_cast<float>(turretSlots - 1)
            : 0.5f;
        hp.position.x = -spacing * (turretSlots - 1) / 2.0f + t * spacing * (turretSlots - 1);
        hp.position.y = 1.0f + RandRange(state, 0.0f, 0.3f);
        hp.position.z = RandRange(state, -0.2f, 0.2f);
        hp.direction = {0.0f, 1.0f, 0.0f};
        hp.size = turretSize;
        hp.weaponType = "turret";
        hardpoints.push_back(hp);
    }

    // Launchers along ventral (bottom) or sides
    for (int i = 0; i < launcherSlots; ++i) {
        TurretHardpoint hp;
        hp.id = hpId++;
        hp.position.x = RandRange(state, -1.0f, 1.0f);
        hp.position.y = -0.5f;
        hp.position.z = RandRange(state, -0.5f, 0.5f);
        hp.direction = {1.0f, 0.0f, 0.0f};
        hp.size = turretSize;
        hp.weaponType = "launcher";
        hardpoints.push_back(hp);
    }

    return hardpoints;
}

std::vector<InteriorSection> GenerateDefaultInteriors(uint64_t seed,
                                                       const std::string& shipClass) {
    std::vector<InteriorSection> interiors;
    uint64_t state = seed ^ 0xDEADBEEFull;
    if (state == 0) state = 1;

    uint32_t intId = 1;

    // Every ship gets a cockpit/bridge for FPS flight
    {
        InteriorSection cockpit;
        cockpit.id = intId++;
        cockpit.name = "cockpit";
        cockpit.hasCockpit = true;
        float length = (shipClass == "Frigate") ? 2.0f : 4.0f;
        cockpit.boundsMin = {-length / 2.0f - 3.0f, -0.5f, -1.0f};
        cockpit.boundsMax = {-length / 2.0f, 1.5f, 1.0f};
        interiors.push_back(cockpit);
    }

    // Engine room at the back
    {
        InteriorSection engine;
        engine.id = intId++;
        engine.name = "engine_room";
        float length = (shipClass == "Frigate") ? 2.0f : 5.0f;
        engine.boundsMin = {length / 2.0f - 3.0f, -1.0f, -1.5f};
        engine.boundsMax = {length / 2.0f, 1.0f, 1.5f};
        interiors.push_back(engine);
    }

    // Larger ships get additional rooms
    if (shipClass != "Frigate") {
        InteriorSection cargo;
        cargo.id = intId++;
        cargo.name = "cargo_bay";
        cargo.boundsMin = {-1.0f, -1.5f, -2.0f};
        cargo.boundsMax = {3.0f, 0.5f, 2.0f};
        interiors.push_back(cargo);
    }

    if (shipClass == "Cruiser" || shipClass == "Battlecruiser" ||
        shipClass == "Battleship" || shipClass == "Capital") {
        InteriorSection crew;
        crew.id = intId++;
        crew.name = "crew_quarters";
        crew.boundsMin = {-3.0f, -0.5f, -1.5f};
        crew.boundsMax = {-1.0f, 1.5f, 1.5f};
        interiors.push_back(crew);
    }

    if (shipClass == "Battleship" || shipClass == "Capital") {
        InteriorSection bridge;
        bridge.id = intId++;
        bridge.name = "bridge";
        bridge.hasCockpit = true;
        bridge.boundsMin = {-5.0f, 2.0f, -2.0f};
        bridge.boundsMax = {-2.0f, 4.0f, 2.0f};
        interiors.push_back(bridge);
    }

    return interiors;
}

std::vector<ShipLight> GenerateDefaultLights(uint64_t seed,
                                              const std::string& shipClass) {
    std::vector<ShipLight> lights;
    uint64_t state = seed ^ 0xF00DCAFEull;
    if (state == 0) state = 1;

    uint32_t lightId = 1;

    // Interior cockpit light
    {
        ShipLight l;
        l.id = lightId++;
        l.position = {-3.0f, 1.0f, 0.0f};
        l.color = {0.9f, 0.9f, 1.0f};
        l.intensity = 0.8f;
        l.range = 3.0f;
        l.type = "point";
        l.interior = true;
        lights.push_back(l);
    }

    // Interior engine room light (warm)
    {
        ShipLight l;
        l.id = lightId++;
        l.position = {3.0f, 0.5f, 0.0f};
        l.color = {1.0f, 0.7f, 0.4f};
        l.intensity = 0.6f;
        l.range = 4.0f;
        l.type = "point";
        l.interior = true;
        lights.push_back(l);
    }

    // Exterior nav lights
    {
        ShipLight l;
        l.id = lightId++;
        l.position = {0.0f, 0.0f, -1.5f};
        l.color = {1.0f, 0.0f, 0.0f};
        l.intensity = 1.0f;
        l.range = 15.0f;
        l.type = "point";
        l.interior = false;
        lights.push_back(l);
    }
    {
        ShipLight l;
        l.id = lightId++;
        l.position = {0.0f, 0.0f, 1.5f};
        l.color = {0.0f, 1.0f, 0.0f};
        l.intensity = 1.0f;
        l.range = 15.0f;
        l.type = "point";
        l.interior = false;
        lights.push_back(l);
    }

    // Larger ships get more lights
    if (shipClass != "Frigate") {
        ShipLight l;
        l.id = lightId++;
        l.position = {0.0f, 0.5f, 0.0f};
        l.color = {0.8f, 0.8f, 0.9f};
        l.intensity = 0.5f;
        l.range = 5.0f;
        l.type = "ambient";
        l.interior = true;
        lights.push_back(l);
    }

    return lights;
}

}
