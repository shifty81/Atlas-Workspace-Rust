#pragma once
// NF::Game — Game layer: voxels, interaction loop, R.I.G. system, inventory
#include "NF/Core/Core.h"
#include "NF/Engine/Engine.h"
#include "NF/Renderer/Renderer.h"
#include "NF/Physics/Physics.h"
#include <map>
#include <unordered_map>
#include <vector>
#include <array>
#include <algorithm>
#include <cstdint>

namespace NF {

// ── Voxel types ──────────────────────────────────────────────────

enum class VoxelType : uint8_t {
    Air = 0,
    Stone,
    Dirt,
    Grass,
    Metal,
    Glass,
    Water,
    Ore_Iron,
    Ore_Gold,
    Ore_Crystal,
    Count
};

inline const char* voxelTypeName(VoxelType type) {
    switch (type) {
        case VoxelType::Air:         return "Air";
        case VoxelType::Stone:       return "Stone";
        case VoxelType::Dirt:        return "Dirt";
        case VoxelType::Grass:       return "Grass";
        case VoxelType::Metal:       return "Metal";
        case VoxelType::Glass:       return "Glass";
        case VoxelType::Water:       return "Water";
        case VoxelType::Ore_Iron:    return "Ore_Iron";
        case VoxelType::Ore_Gold:    return "Ore_Gold";
        case VoxelType::Ore_Crystal: return "Ore_Crystal";
        default:                     return "Unknown";
    }
}

inline VoxelType voxelTypeFromName(std::string_view name) {
    if (name == "Stone")       return VoxelType::Stone;
    if (name == "Dirt")        return VoxelType::Dirt;
    if (name == "Grass")       return VoxelType::Grass;
    if (name == "Metal")       return VoxelType::Metal;
    if (name == "Glass")       return VoxelType::Glass;
    if (name == "Water")       return VoxelType::Water;
    if (name == "Ore_Iron")    return VoxelType::Ore_Iron;
    if (name == "Ore_Gold")    return VoxelType::Ore_Gold;
    if (name == "Ore_Crystal") return VoxelType::Ore_Crystal;
    return VoxelType::Air;
}

constexpr int CHUNK_SIZE = 16;

struct Chunk {
    VoxelType voxels[CHUNK_SIZE][CHUNK_SIZE][CHUNK_SIZE] = {};
    int cx = 0, cy = 0, cz = 0;
    bool meshDirty = true;
    bool collisionDirty = true;
    bool meshed = false;

    VoxelType get(int x, int y, int z) const {
        if (x < 0 || x >= CHUNK_SIZE || y < 0 || y >= CHUNK_SIZE || z < 0 || z >= CHUNK_SIZE)
            return VoxelType::Air;
        return voxels[x][y][z];
    }

    void set(int x, int y, int z, VoxelType type) {
        if (x < 0 || x >= CHUNK_SIZE || y < 0 || y >= CHUNK_SIZE || z < 0 || z >= CHUNK_SIZE)
            return;
        voxels[x][y][z] = type;
        meshDirty = true;
        collisionDirty = true;
        meshed = false;
    }

    void markMeshClean() { meshDirty = false; meshed = true; }
    void markCollisionClean() { collisionDirty = false; }
    void markAllClean() { meshDirty = false; collisionDirty = false; meshed = true; }

    [[nodiscard]] bool isFullyAir() const {
        for (int x = 0; x < CHUNK_SIZE; ++x)
            for (int y = 0; y < CHUNK_SIZE; ++y)
                for (int z = 0; z < CHUNK_SIZE; ++z)
                    if (voxels[x][y][z] != VoxelType::Air) return false;
        return true;
    }

    [[nodiscard]] int solidCount() const {
        int count = 0;
        for (int x = 0; x < CHUNK_SIZE; ++x)
            for (int y = 0; y < CHUNK_SIZE; ++y)
                for (int z = 0; z < CHUNK_SIZE; ++z)
                    if (voxels[x][y][z] != VoxelType::Air) ++count;
        return count;
    }
};

// ── Chunk Serializer ─────────────────────────────────────────────

class ChunkSerializer {
public:
    static JsonValue toJson(const Chunk& chunk) {
        auto obj = JsonValue::object();
        obj.set("cx", JsonValue(chunk.cx));
        obj.set("cy", JsonValue(chunk.cy));
        obj.set("cz", JsonValue(chunk.cz));
        obj.set("size", JsonValue(CHUNK_SIZE));

        // RLE-encode the voxel data for compact storage
        auto voxelArray = JsonValue::array();
        for (int x = 0; x < CHUNK_SIZE; ++x) {
            for (int y = 0; y < CHUNK_SIZE; ++y) {
                for (int z = 0; z < CHUNK_SIZE; ++z) {
                    voxelArray.push(JsonValue(static_cast<int32_t>(chunk.voxels[x][y][z])));
                }
            }
        }
        obj.set("voxels", std::move(voxelArray));
        return obj;
    }

    static Chunk fromJson(const JsonValue& json) {
        Chunk chunk;
        chunk.cx = json["cx"].asInt();
        chunk.cy = json["cy"].asInt();
        chunk.cz = json["cz"].asInt();

        const auto& voxelArray = json["voxels"];
        if (voxelArray.isArray()) {
            size_t idx = 0;
            for (int x = 0; x < CHUNK_SIZE; ++x) {
                for (int y = 0; y < CHUNK_SIZE; ++y) {
                    for (int z = 0; z < CHUNK_SIZE; ++z) {
                        if (idx < voxelArray.size()) {
                            chunk.voxels[x][y][z] = static_cast<VoxelType>(voxelArray[idx].asInt());
                            ++idx;
                        }
                    }
                }
            }
        }

        chunk.meshDirty = true;
        chunk.collisionDirty = true;
        chunk.meshed = false;
        return chunk;
    }

    // Full round-trip: serialize to JSON string and back
    static std::string serialize(const Chunk& chunk) {
        return toJson(chunk).toJson(2);
    }

    static Chunk deserialize(const std::string& jsonStr) {
        auto parsed = JsonParser::parse(jsonStr);
        return fromJson(parsed);
    }
};

// ── Voxel Edit Command (undo-safe) ──────────────────────────────

class VoxelEditCommand : public ICommand {
public:
    VoxelEditCommand(Chunk* chunk, int x, int y, int z, VoxelType newType)
        : m_chunk(chunk), m_x(x), m_y(y), m_z(z),
          m_newType(newType), m_oldType(chunk->get(x, y, z)) {}

    void execute() override { m_chunk->set(m_x, m_y, m_z, m_newType); }
    void undo() override { m_chunk->set(m_x, m_y, m_z, m_oldType); }

    [[nodiscard]] std::string description() const override {
        return std::string("Set voxel (") + std::to_string(m_x) + "," +
               std::to_string(m_y) + "," + std::to_string(m_z) + ") to " +
               voxelTypeName(m_newType);
    }

private:
    Chunk* m_chunk;
    int m_x, m_y, m_z;
    VoxelType m_newType;
    VoxelType m_oldType;
};

// ── R.I.G. (Rig Interface Gear) ─────────────────────────────────

struct RigState {
    float health = 100.f;
    float maxHealth = 100.f;
    float energy = 100.f;
    float maxEnergy = 100.f;
    float oxygen = 100.f;
    float maxOxygen = 100.f;
    float stamina = 100.f;
    float maxStamina = 100.f;
    float energyRegenRate = 2.f;
    float staminaRegenRate = 5.f;
    float oxygenDrainRate = 1.f;
    int activeTool = 0;

    [[nodiscard]] bool isAlive() const { return health > 0.f; }

    void tick(float dt) {
        if (!isAlive()) return;
        energy = std::min(energy + energyRegenRate * dt, maxEnergy);
        stamina = std::min(stamina + staminaRegenRate * dt, maxStamina);
        oxygen = std::max(oxygen - oxygenDrainRate * dt, 0.f);
        if (oxygen <= 0.f) health = std::max(health - 10.f * dt, 0.f);
    }

    void takeDamage(float amount) {
        health = std::max(health - amount, 0.f);
    }

    void heal(float amount) {
        health = std::min(health + amount, maxHealth);
    }

    void consumeEnergy(float amount) {
        energy = std::max(energy - amount, 0.f);
    }

    void consumeStamina(float amount) {
        stamina = std::max(stamina - amount, 0.f);
    }
};

// ── Inventory ────────────────────────────────────────────────────

struct Inventory {
    int counts[static_cast<size_t>(VoxelType::Count)] = {};

    void add(VoxelType type, int amount = 1) {
        counts[static_cast<size_t>(type)] += amount;
    }

    bool remove(VoxelType type, int amount = 1) {
        auto& c = counts[static_cast<size_t>(type)];
        if (c < amount) return false;
        c -= amount;
        return true;
    }

    [[nodiscard]] int count(VoxelType type) const {
        return counts[static_cast<size_t>(type)];
    }
};

// ── Voxel color mapping ─────────────────────────────────────────

inline Vec4 voxelColor(VoxelType type) {
    switch (type) {
        case VoxelType::Stone:       return {0.6f, 0.6f, 0.6f, 1.f};
        case VoxelType::Dirt:        return {0.55f, 0.35f, 0.2f, 1.f};
        case VoxelType::Grass:       return {0.3f, 0.7f, 0.2f, 1.f};
        case VoxelType::Metal:       return {0.75f, 0.75f, 0.8f, 1.f};
        case VoxelType::Glass:       return {0.7f, 0.85f, 0.95f, 0.5f};
        case VoxelType::Water:       return {0.2f, 0.4f, 0.8f, 0.7f};
        case VoxelType::Ore_Iron:    return {0.65f, 0.5f, 0.4f, 1.f};
        case VoxelType::Ore_Gold:    return {0.9f, 0.8f, 0.2f, 1.f};
        case VoxelType::Ore_Crystal: return {0.6f, 0.2f, 0.9f, 1.f};
        default:                     return {1.f, 1.f, 1.f, 1.f};
    }
}

// ── Chunk Mesher ─────────────────────────────────────────────────
// Generates renderable geometry from chunk voxel data.
// Uses face-culling: only emits faces where a solid voxel is adjacent to air.

struct ChunkMeshResult {
    std::vector<Vertex> vertices;
    std::vector<uint32_t> indices;
    size_t faceCount = 0;
};

class ChunkMesher {
public:
    // Generate mesh for a chunk with face-culling.
    // worldOffsetX/Y/Z positions the chunk in world space.
    static ChunkMeshResult buildMesh(const Chunk& chunk) {
        ChunkMeshResult result;

        // 6 face directions: +X, -X, +Y, -Y, +Z, -Z
        struct FaceInfo {
            int dx, dy, dz;         // neighbor offset
            Vec3 normal;
            // 4 vertex offsets for a unit cube face (CCW winding)
            Vec3 v0, v1, v2, v3;
        };

        static const FaceInfo faces[6] = {
            // +X face (right)
            { 1, 0, 0, {1,0,0}, {1,0,0}, {1,1,0}, {1,1,1}, {1,0,1} },
            // -X face (left)
            {-1, 0, 0, {-1,0,0}, {0,0,1}, {0,1,1}, {0,1,0}, {0,0,0} },
            // +Y face (top)
            { 0, 1, 0, {0,1,0}, {0,1,0}, {0,1,1}, {1,1,1}, {1,1,0} },
            // -Y face (bottom)
            { 0,-1, 0, {0,-1,0}, {0,0,1}, {0,0,0}, {1,0,0}, {1,0,1} },
            // +Z face (front)
            { 0, 0, 1, {0,0,1}, {0,0,1}, {0,1,1}, {1,1,1}, {1,0,1} },
            // -Z face (back)
            { 0, 0,-1, {0,0,-1}, {1,0,0}, {1,1,0}, {0,1,0}, {0,0,0} },
        };

        float ox = static_cast<float>(chunk.cx * CHUNK_SIZE);
        float oy = static_cast<float>(chunk.cy * CHUNK_SIZE);
        float oz = static_cast<float>(chunk.cz * CHUNK_SIZE);

        for (int x = 0; x < CHUNK_SIZE; ++x) {
            for (int y = 0; y < CHUNK_SIZE; ++y) {
                for (int z = 0; z < CHUNK_SIZE; ++z) {
                    VoxelType vt = chunk.voxels[x][y][z];
                    if (vt == VoxelType::Air) continue;

                    Vec4 col = voxelColor(vt);

                    for (int f = 0; f < 6; ++f) {
                        const auto& face = faces[f];
                        int nx = x + face.dx;
                        int ny = y + face.dy;
                        int nz = z + face.dz;

                        // Emit face only if neighbor is air (exposed face)
                        if (chunk.get(nx, ny, nz) != VoxelType::Air) continue;

                        uint32_t baseIdx = static_cast<uint32_t>(result.vertices.size());
                        float fx = static_cast<float>(x) + ox;
                        float fy = static_cast<float>(y) + oy;
                        float fz = static_cast<float>(z) + oz;

                        auto makeVert = [&](const Vec3& off) -> Vertex {
                            return {{fx + off.x, fy + off.y, fz + off.z},
                                    face.normal, {0.f, 0.f}, col};
                        };

                        result.vertices.push_back(makeVert(face.v0));
                        result.vertices.push_back(makeVert(face.v1));
                        result.vertices.push_back(makeVert(face.v2));
                        result.vertices.push_back(makeVert(face.v3));

                        // Two triangles per face
                        result.indices.push_back(baseIdx + 0);
                        result.indices.push_back(baseIdx + 1);
                        result.indices.push_back(baseIdx + 2);
                        result.indices.push_back(baseIdx + 0);
                        result.indices.push_back(baseIdx + 2);
                        result.indices.push_back(baseIdx + 3);

                        ++result.faceCount;
                    }
                }
            }
        }
        return result;
    }

    // Build a Mesh object directly (convenience)
    static Mesh buildRenderMesh(const Chunk& chunk) {
        auto result = buildMesh(chunk);
        return Mesh(std::move(result.vertices), std::move(result.indices));
    }
};

// ── World State (multi-chunk container) ──────────────────────────

struct ChunkCoord {
    int x, y, z;

    bool operator==(const ChunkCoord& o) const { return x == o.x && y == o.y && z == o.z; }
    bool operator!=(const ChunkCoord& o) const { return !(*this == o); }
    bool operator<(const ChunkCoord& o) const {
        if (x != o.x) return x < o.x;
        if (y != o.y) return y < o.y;
        return z < o.z;
    }
};

} // namespace NF

// Hash specialization for ChunkCoord (must be before unordered_map usage)
template<> struct std::hash<NF::ChunkCoord> {
    size_t operator()(const NF::ChunkCoord& c) const noexcept {
        size_t h = std::hash<int>{}(c.x);
        h ^= std::hash<int>{}(c.y) + 0x9e3779b9 + (h << 6) + (h >> 2);
        h ^= std::hash<int>{}(c.z) + 0x9e3779b9 + (h << 6) + (h >> 2);
        return h;
    }
};

namespace NF {

class WorldState {
public:
    void clear() { m_chunks.clear(); }

    Chunk& getOrCreateChunk(int cx, int cy, int cz) {
        ChunkCoord coord{cx, cy, cz};
        auto it = m_chunks.find(coord);
        if (it == m_chunks.end()) {
            Chunk c;
            c.cx = cx; c.cy = cy; c.cz = cz;
            auto [newIt, _] = m_chunks.emplace(coord, std::move(c));
            return newIt->second;
        }
        return it->second;
    }

    Chunk* findChunk(int cx, int cy, int cz) {
        auto it = m_chunks.find({cx, cy, cz});
        return (it != m_chunks.end()) ? &it->second : nullptr;
    }

    const Chunk* findChunk(int cx, int cy, int cz) const {
        auto it = m_chunks.find({cx, cy, cz});
        return (it != m_chunks.end()) ? &it->second : nullptr;
    }

    void setChunk(const Chunk& chunk) {
        m_chunks[{chunk.cx, chunk.cy, chunk.cz}] = chunk;
    }

    void removeChunk(int cx, int cy, int cz) {
        m_chunks.erase({cx, cy, cz});
    }

    [[nodiscard]] size_t chunkCount() const { return m_chunks.size(); }

    // Iterate all chunks
    template<typename Fn>
    void forEach(Fn&& fn) {
        for (auto& [coord, chunk] : m_chunks) fn(chunk);
    }

    template<typename Fn>
    void forEach(Fn&& fn) const {
        for (const auto& [coord, chunk] : m_chunks) fn(chunk);
    }

    // Get voxel at world coordinates
    VoxelType getWorld(int wx, int wy, int wz) const {
        auto [cx, cy, cz, lx, ly, lz] = worldToLocal(wx, wy, wz);
        const Chunk* ch = findChunk(cx, cy, cz);
        return ch ? ch->get(lx, ly, lz) : VoxelType::Air;
    }

    // Set voxel at world coordinates (creates chunk if needed)
    void setWorld(int wx, int wy, int wz, VoxelType type) {
        auto [cx, cy, cz, lx, ly, lz] = worldToLocal(wx, wy, wz);
        getOrCreateChunk(cx, cy, cz).set(lx, ly, lz, type);
    }

private:
    // Convert world coordinates to chunk + local coordinates
    struct LocalCoord { int cx, cy, cz, lx, ly, lz; };
    static LocalCoord worldToLocal(int wx, int wy, int wz) {
        int cx = (wx >= 0) ? wx / CHUNK_SIZE : (wx - CHUNK_SIZE + 1) / CHUNK_SIZE;
        int cy = (wy >= 0) ? wy / CHUNK_SIZE : (wy - CHUNK_SIZE + 1) / CHUNK_SIZE;
        int cz = (wz >= 0) ? wz / CHUNK_SIZE : (wz - CHUNK_SIZE + 1) / CHUNK_SIZE;
        return {cx, cy, cz, wx - cx * CHUNK_SIZE, wy - cy * CHUNK_SIZE, wz - cz * CHUNK_SIZE};
    }
    std::unordered_map<ChunkCoord, Chunk> m_chunks;
};

// ── World Serializer ─────────────────────────────────────────────

class WorldSerializer {
public:
    static JsonValue toJson(const WorldState& world) {
        auto obj = JsonValue::object();
        obj.set("version", JsonValue(1));

        auto chunksArr = JsonValue::array();
        world.forEach([&](const Chunk& chunk) {
            chunksArr.push(ChunkSerializer::toJson(chunk));
        });
        obj.set("chunks", std::move(chunksArr));

        return obj;
    }

    static WorldState fromJson(const JsonValue& json) {
        WorldState world;
        const auto& chunksArr = json["chunks"];
        if (chunksArr.isArray()) {
            for (size_t i = 0; i < chunksArr.size(); ++i) {
                Chunk chunk = ChunkSerializer::fromJson(chunksArr[i]);
                world.setChunk(chunk);
            }
        }
        return world;
    }

    static std::string serialize(const WorldState& world) {
        return toJson(world).toJson(2);
    }

    static WorldState deserialize(const std::string& jsonStr) {
        auto parsed = JsonParser::parse(jsonStr);
        return fromJson(parsed);
    }
};

// ── Voxel Pick Service ──────────────────────────────────────────
// Raycast into a voxel world to find which voxel the ray hits.

struct VoxelHit {
    int wx, wy, wz;           // World-space voxel coordinates
    int face;                  // Which face was hit (0=+X, 1=-X, 2=+Y, 3=-Y, 4=+Z, 5=-Z)
    float distance;            // Distance along ray
    VoxelType type;            // Type of voxel hit
    Vec3 hitPoint;             // Exact hit point
    Vec3 normal;               // Hit face normal

    // Compute adjacent voxel position (for placing a new voxel)
    [[nodiscard]] ChunkCoord adjacentVoxel() const {
        static const int offsets[6][3] = {
            { 1, 0, 0}, {-1, 0, 0},
            { 0, 1, 0}, { 0,-1, 0},
            { 0, 0, 1}, { 0, 0,-1}
        };
        return {wx + offsets[face][0], wy + offsets[face][1], wz + offsets[face][2]};
    }
};

class VoxelPickService {
public:
    static constexpr float kEpsilon = 1e-8f;
    static constexpr float kMaxT    = 1e30f;

    // Cast a ray through the voxel world using DDA (Digital Differential Analyzer).
    // Returns the first solid voxel hit, or std::nullopt if nothing hit within maxDist.
    static std::optional<VoxelHit> raycast(const WorldState& world,
                                            const Vec3& origin, const Vec3& direction,
                                            float maxDist = 100.f) {
        // DDA voxel traversal
        Vec3 dir = direction.normalized();

        // Current voxel position
        int vx = static_cast<int>(std::floor(origin.x));
        int vy = static_cast<int>(std::floor(origin.y));
        int vz = static_cast<int>(std::floor(origin.z));

        // Step direction
        int stepX = (dir.x >= 0.f) ? 1 : -1;
        int stepY = (dir.y >= 0.f) ? 1 : -1;
        int stepZ = (dir.z >= 0.f) ? 1 : -1;

        // Distance to next voxel boundary
        float tMaxX = (std::abs(dir.x) > kEpsilon)
            ? ((stepX > 0 ? (vx + 1.f - origin.x) : (origin.x - vx)) / std::abs(dir.x))
            : kMaxT;
        float tMaxY = (std::abs(dir.y) > kEpsilon)
            ? ((stepY > 0 ? (vy + 1.f - origin.y) : (origin.y - vy)) / std::abs(dir.y))
            : kMaxT;
        float tMaxZ = (std::abs(dir.z) > kEpsilon)
            ? ((stepZ > 0 ? (vz + 1.f - origin.z) : (origin.z - vz)) / std::abs(dir.z))
            : kMaxT;

        // Distance between voxel boundaries
        float tDeltaX = (std::abs(dir.x) > kEpsilon) ? (1.f / std::abs(dir.x)) : kMaxT;
        float tDeltaY = (std::abs(dir.y) > kEpsilon) ? (1.f / std::abs(dir.y)) : kMaxT;
        float tDeltaZ = (std::abs(dir.z) > kEpsilon) ? (1.f / std::abs(dir.z)) : kMaxT;

        float t = 0.f;
        int lastFace = -1;

        // Check starting voxel
        VoxelType startType = world.getWorld(vx, vy, vz);
        if (startType != VoxelType::Air) {
            VoxelHit hit;
            hit.wx = vx; hit.wy = vy; hit.wz = vz;
            hit.face = 2; // default to +Y
            hit.distance = 0.f;
            hit.type = startType;
            hit.hitPoint = origin;
            hit.normal = {0, 1, 0};
            return hit;
        }

        while (t < maxDist) {
            // Step to next voxel
            if (tMaxX < tMaxY && tMaxX < tMaxZ) {
                t = tMaxX;
                vx += stepX;
                tMaxX += tDeltaX;
                lastFace = (stepX > 0) ? 1 : 0; // hit -X or +X face
            } else if (tMaxY < tMaxZ) {
                t = tMaxY;
                vy += stepY;
                tMaxY += tDeltaY;
                lastFace = (stepY > 0) ? 3 : 2; // hit -Y or +Y face
            } else {
                t = tMaxZ;
                vz += stepZ;
                tMaxZ += tDeltaZ;
                lastFace = (stepZ > 0) ? 5 : 4; // hit -Z or +Z face
            }

            if (t > maxDist) break;

            VoxelType vt = world.getWorld(vx, vy, vz);
            if (vt != VoxelType::Air) {
                static const Vec3 normals[6] = {
                    {1,0,0}, {-1,0,0}, {0,1,0}, {0,-1,0}, {0,0,1}, {0,0,-1}
                };
                VoxelHit hit;
                hit.wx = vx; hit.wy = vy; hit.wz = vz;
                hit.face = lastFace;
                hit.distance = t;
                hit.type = vt;
                hit.hitPoint = origin + dir * t;
                hit.normal = normals[lastFace];
                return hit;
            }
        }

        return std::nullopt;
    }
};

// ── G1: Resource Types ──────────────────────────────────────────

enum class ResourceType : uint8_t {
    RawStone = 0,
    RawDirt,
    RawIron,
    RawGold,
    RawCrystal,
    RefinedIron,
    RefinedGold,
    RefinedCrystal,
    SteelPlate,
    CircuitBoard,
    EnergyCell,
    Count
};

inline const char* resourceTypeName(ResourceType type) {
    switch (type) {
        case ResourceType::RawStone:       return "RawStone";
        case ResourceType::RawDirt:        return "RawDirt";
        case ResourceType::RawIron:        return "RawIron";
        case ResourceType::RawGold:        return "RawGold";
        case ResourceType::RawCrystal:     return "RawCrystal";
        case ResourceType::RefinedIron:    return "RefinedIron";
        case ResourceType::RefinedGold:    return "RefinedGold";
        case ResourceType::RefinedCrystal: return "RefinedCrystal";
        case ResourceType::SteelPlate:     return "SteelPlate";
        case ResourceType::CircuitBoard:   return "CircuitBoard";
        case ResourceType::EnergyCell:     return "EnergyCell";
        default:                           return "Unknown";
    }
}

inline ResourceType resourceTypeFromName(std::string_view name) {
    if (name == "RawStone")       return ResourceType::RawStone;
    if (name == "RawDirt")        return ResourceType::RawDirt;
    if (name == "RawIron")        return ResourceType::RawIron;
    if (name == "RawGold")        return ResourceType::RawGold;
    if (name == "RawCrystal")     return ResourceType::RawCrystal;
    if (name == "RefinedIron")    return ResourceType::RefinedIron;
    if (name == "RefinedGold")    return ResourceType::RefinedGold;
    if (name == "RefinedCrystal") return ResourceType::RefinedCrystal;
    if (name == "SteelPlate")     return ResourceType::SteelPlate;
    if (name == "CircuitBoard")   return ResourceType::CircuitBoard;
    if (name == "EnergyCell")     return ResourceType::EnergyCell;
    return ResourceType::RawStone;
}

// ── G1: Resource Drop Table ─────────────────────────────────────

struct ResourceDrop {
    ResourceType resource;
    int minAmount;
    int maxAmount;
};

inline std::vector<ResourceDrop> getResourceDrops(VoxelType type) {
    switch (type) {
        case VoxelType::Stone:       return {{ResourceType::RawStone, 1, 1}};
        case VoxelType::Dirt:        return {{ResourceType::RawDirt, 1, 1}};
        case VoxelType::Ore_Iron:    return {{ResourceType::RawIron, 1, 2}};
        case VoxelType::Ore_Gold:    return {{ResourceType::RawGold, 1, 2}};
        case VoxelType::Ore_Crystal: return {{ResourceType::RawCrystal, 1, 3}};
        default:                     return {};
    }
}

// ── G1: Resource Inventory ──────────────────────────────────────

struct ResourceInventory {
    int counts[static_cast<size_t>(ResourceType::Count)] = {};

    void add(ResourceType type, int amount = 1) {
        counts[static_cast<size_t>(type)] += amount;
    }

    bool remove(ResourceType type, int amount = 1) {
        auto& c = counts[static_cast<size_t>(type)];
        if (c < amount) return false;
        c -= amount;
        return true;
    }

    [[nodiscard]] int count(ResourceType type) const {
        return counts[static_cast<size_t>(type)];
    }

    [[nodiscard]] int totalItems() const {
        int total = 0;
        for (int i = 0; i < static_cast<int>(ResourceType::Count); ++i)
            total += counts[i];
        return total;
    }

    [[nodiscard]] bool isEmpty() const {
        return totalItems() == 0;
    }
};

// ── G1: Tool System ─────────────────────────────────────────────

enum class ToolType : uint8_t { MiningLaser = 0, PlacementTool, RepairTool, Scanner, Count };

inline const char* toolTypeName(ToolType type) {
    switch (type) {
        case ToolType::MiningLaser:   return "MiningLaser";
        case ToolType::PlacementTool: return "PlacementTool";
        case ToolType::RepairTool:    return "RepairTool";
        case ToolType::Scanner:       return "Scanner";
        default:                      return "Unknown";
    }
}

struct ToolState {
    ToolType type = ToolType::MiningLaser;
    float durability = 100.f;
    float energyCost = 5.f;
    float cooldown = 0.f;
    float cooldownRate = 0.5f;
    float miningDamage = 25.f;

    [[nodiscard]] bool isReady() const { return cooldown <= 0.f && durability > 0.f; }

    void tick(float dt) {
        if (cooldown > 0.f) cooldown = std::max(cooldown - dt, 0.f);
    }

    void use() {
        cooldown = cooldownRate;
        durability = std::max(durability - 1.f, 0.f);
    }
};

// ── G1: Tool Belt ───────────────────────────────────────────────

class ToolBelt {
    static constexpr int MAX_SLOTS = 4;
    ToolState m_slots[MAX_SLOTS];
    int m_activeSlot = 0;
public:
    void init() {
        m_slots[0] = {ToolType::MiningLaser,   100.f, 5.f, 0.f, 0.5f, 25.f};
        m_slots[1] = {ToolType::PlacementTool,  100.f, 2.f, 0.f, 0.3f, 0.f};
        m_slots[2] = {ToolType::RepairTool,     100.f, 8.f, 0.f, 1.0f, 0.f};
        m_slots[3] = {ToolType::Scanner,        100.f, 3.f, 0.f, 0.2f, 0.f};
        m_activeSlot = 0;
    }

    ToolState& activeTool() { return m_slots[m_activeSlot]; }
    const ToolState& activeTool() const { return m_slots[m_activeSlot]; }

    void selectSlot(int slot) {
        m_activeSlot = std::max(0, std::min(slot, MAX_SLOTS - 1));
    }

    [[nodiscard]] int activeSlot() const { return m_activeSlot; }

    void nextTool() { m_activeSlot = (m_activeSlot + 1) % MAX_SLOTS; }
    void prevTool() { m_activeSlot = (m_activeSlot + MAX_SLOTS - 1) % MAX_SLOTS; }

    ToolState& slot(int i) { return m_slots[std::max(0, std::min(i, MAX_SLOTS - 1))]; }
    const ToolState& slot(int i) const { return m_slots[std::max(0, std::min(i, MAX_SLOTS - 1))]; }

    [[nodiscard]] int slotCount() const { return MAX_SLOTS; }
};

// ── G1: HUD State ───────────────────────────────────────────────

struct HUDNotification {
    std::string message;
    float timeRemaining = 3.f;
};

struct HUDState {
    bool showCrosshair = true;
    bool targetLocked = false;
    VoxelType targetVoxel = VoxelType::Air;

    std::vector<HUDNotification> notifications;

    void addNotification(const std::string& msg, float duration = 3.f) {
        notifications.push_back({msg, duration});
    }

    void tick(float dt) {
        for (auto& n : notifications) n.timeRemaining -= dt;
        notifications.erase(
            std::remove_if(notifications.begin(), notifications.end(),
                           [](const HUDNotification& n) { return n.timeRemaining <= 0.f; }),
            notifications.end());
    }

    void clearNotifications() { notifications.clear(); }
};

// ── G1: Interaction System ──────────────────────────────────────

struct MineResult {
    bool success = false;
    VoxelType minedType = VoxelType::Air;
    std::vector<ResourceDrop> drops;
    float energyUsed = 0.f;
};

struct PlaceResult {
    bool success = false;
    VoxelType placedType = VoxelType::Air;
};

class InteractionSystem {
public:
    MineResult tryMine(WorldState& world, RigState& rig, ToolBelt& belt,
                       Inventory& voxelInv, ResourceInventory& resInv,
                       const Vec3& origin, const Vec3& direction) {
        MineResult result;
        auto& tool = belt.activeTool();
        if (tool.type != ToolType::MiningLaser) return result;
        if (!tool.isReady()) return result;
        if (rig.energy < tool.energyCost) return result;

        auto hit = VoxelPickService::raycast(world, origin, direction, m_maxReach);
        if (!hit.has_value()) return result;

        result.minedType = hit->type;
        world.setWorld(hit->wx, hit->wy, hit->wz, VoxelType::Air);

        rig.consumeEnergy(tool.energyCost);
        tool.use();

        voxelInv.add(result.minedType);

        auto drops = getResourceDrops(result.minedType);
        for (auto& drop : drops) {
            resInv.add(drop.resource, drop.minAmount);
            result.drops.push_back(drop);
        }

        result.success = true;
        result.energyUsed = tool.energyCost;
        return result;
    }

    PlaceResult tryPlace(WorldState& world, RigState& rig, ToolBelt& belt,
                         Inventory& voxelInv,
                         const Vec3& origin, const Vec3& direction,
                         VoxelType typeToPlace) {
        PlaceResult result;
        auto& tool = belt.activeTool();
        if (tool.type != ToolType::PlacementTool) return result;
        if (!tool.isReady()) return result;
        if (rig.energy < tool.energyCost) return result;
        if (!voxelInv.remove(typeToPlace)) return result;

        auto hit = VoxelPickService::raycast(world, origin, direction, m_maxReach);
        if (!hit.has_value()) {
            voxelInv.add(typeToPlace);
            return result;
        }

        auto adj = hit->adjacentVoxel();
        if (world.getWorld(adj.x, adj.y, adj.z) != VoxelType::Air) {
            voxelInv.add(typeToPlace);
            return result;
        }

        world.setWorld(adj.x, adj.y, adj.z, typeToPlace);
        rig.consumeEnergy(tool.energyCost);
        tool.use();

        result.success = true;
        result.placedType = typeToPlace;
        return result;
    }

    std::optional<VoxelHit> tryScan(const WorldState& world, const ToolBelt& belt,
                                     const Vec3& origin, const Vec3& direction) {
        const auto& tool = belt.activeTool();
        if (tool.type != ToolType::Scanner) return std::nullopt;
        if (!tool.isReady()) return std::nullopt;
        return VoxelPickService::raycast(world, origin, direction, m_maxReach);
    }

    [[nodiscard]] float maxReach() const { return m_maxReach; }
    void setMaxReach(float r) { m_maxReach = r; }

private:
    float m_maxReach = 8.f;
};

// ── G1: Game Session ────────────────────────────────────────────

class GameSession {
public:
    void init(uint32_t /*seed*/ = 42) {
        m_rig = RigState{};
        m_voxelInv = Inventory{};
        m_resourceInv = ResourceInventory{};
        m_toolBelt.init();
        m_world.clear();
        m_hud = HUDState{};
        m_interaction = InteractionSystem{};
        m_active = true;
    }

    void shutdown() { m_active = false; }

    void tick(float dt) {
        if (!m_active) return;
        m_rig.tick(dt);
        for (int i = 0; i < m_toolBelt.slotCount(); ++i)
            m_toolBelt.slot(i).tick(dt);
        m_hud.tick(dt);
    }

    RigState& rig() { return m_rig; }
    const RigState& rig() const { return m_rig; }
    Inventory& voxelInventory() { return m_voxelInv; }
    const Inventory& voxelInventory() const { return m_voxelInv; }
    ResourceInventory& resourceInventory() { return m_resourceInv; }
    const ResourceInventory& resourceInventory() const { return m_resourceInv; }
    ToolBelt& toolBelt() { return m_toolBelt; }
    const ToolBelt& toolBelt() const { return m_toolBelt; }
    WorldState& world() { return m_world; }
    const WorldState& world() const { return m_world; }
    HUDState& hud() { return m_hud; }
    const HUDState& hud() const { return m_hud; }
    InteractionSystem& interaction() { return m_interaction; }
    const InteractionSystem& interaction() const { return m_interaction; }

    [[nodiscard]] bool isActive() const { return m_active; }

private:
    RigState m_rig;
    Inventory m_voxelInv;
    ResourceInventory m_resourceInv;
    ToolBelt m_toolBelt;
    WorldState m_world;
    HUDState m_hud;
    InteractionSystem m_interaction;
    bool m_active = false;
};

// ── G2: Chunk Render Data ───────────────────────────────────────

struct ChunkRenderData {
    ChunkCoord coord;
    Mesh mesh;
    bool valid = false;
    uint32_t version = 0;
};

// ── G2: Chunk Render Cache ─────────────────────────────────────

class ChunkRenderCache {
public:
    void update(const Chunk& chunk) {
        ChunkCoord coord{chunk.cx, chunk.cy, chunk.cz};
        auto& data = m_cache[coord];
        data.coord = coord;
        data.mesh = ChunkMesher::buildRenderMesh(chunk);
        data.valid = true;
        ++data.version;
    }

    void remove(const ChunkCoord& coord) {
        m_cache.erase(coord);
    }

    ChunkRenderData* get(const ChunkCoord& coord) {
        auto it = m_cache.find(coord);
        return (it != m_cache.end()) ? &it->second : nullptr;
    }

    const ChunkRenderData* get(const ChunkCoord& coord) const {
        auto it = m_cache.find(coord);
        return (it != m_cache.end()) ? &it->second : nullptr;
    }

    void clear() { m_cache.clear(); }

    size_t cacheSize() const { return m_cache.size(); }

    int updateDirty(WorldState& world) {
        int count = 0;
        world.forEach([&](Chunk& chunk) {
            if (chunk.meshDirty) {
                update(chunk);
                chunk.markMeshClean();
                ++count;
            }
        });
        return count;
    }

private:
    std::unordered_map<ChunkCoord, ChunkRenderData> m_cache;
};

// ── G2: Chunk Renderer ─────────────────────────────────────────

class ChunkRenderer {
public:
    void init() {
        m_voxelMaterial = Material(StringID("voxel_material"));
        m_initialized = true;
    }

    void shutdown() {
        m_initialized = false;
    }

    int render(const WorldState& world, ChunkRenderCache& cache,
               RenderQueue& queue, const Camera& camera, float aspect) {
        Mat4 vp = camera.projectionMatrix(aspect) * camera.viewMatrix();
        Frustum frustum;
        frustum.extractFromVP(vp);

        int visibleCount = 0;
        world.forEach([&](const Chunk& chunk) {
            float bx = static_cast<float>(chunk.cx * CHUNK_SIZE);
            float by = static_cast<float>(chunk.cy * CHUNK_SIZE);
            float bz = static_cast<float>(chunk.cz * CHUNK_SIZE);
            Vec3 aabbMin{bx, by, bz};
            Vec3 aabbMax{bx + CHUNK_SIZE, by + CHUNK_SIZE, bz + CHUNK_SIZE};

            if (!frustum.testAABB(aabbMin, aabbMax)) return;

            ChunkCoord coord{chunk.cx, chunk.cy, chunk.cz};
            auto* rd = cache.get(coord);
            if (!rd || !rd->valid || rd->mesh.vertexCount() == 0) return;

            RenderCommand cmd;
            cmd.mesh = &rd->mesh;
            cmd.material = &m_voxelMaterial;
            cmd.transform = Mat4::translation(0.f, 0.f, 0.f);
            cmd.sortKey = (camera.position - Vec3{bx + CHUNK_SIZE * 0.5f,
                                                   by + CHUNK_SIZE * 0.5f,
                                                   bz + CHUNK_SIZE * 0.5f}).lengthSq();
            queue.submit(std::move(cmd));
            ++visibleCount;
        });

        return visibleCount;
    }

    int countVisible(const WorldState& world, const Camera& camera, float aspect) const {
        Mat4 vp = camera.projectionMatrix(aspect) * camera.viewMatrix();
        Frustum frustum;
        frustum.extractFromVP(vp);

        int count = 0;
        world.forEach([&](const Chunk& chunk) {
            float bx = static_cast<float>(chunk.cx * CHUNK_SIZE);
            float by = static_cast<float>(chunk.cy * CHUNK_SIZE);
            float bz = static_cast<float>(chunk.cz * CHUNK_SIZE);
            Vec3 aabbMin{bx, by, bz};
            Vec3 aabbMax{bx + CHUNK_SIZE, by + CHUNK_SIZE, bz + CHUNK_SIZE};

            if (frustum.testAABB(aabbMin, aabbMax)) ++count;
        });

        return count;
    }

    Material& voxelMaterial() { return m_voxelMaterial; }

private:
    Material m_voxelMaterial;
    bool m_initialized = false;
};

// ── G3: Movement & FPS Camera ────────────────────────────────────

class FPSCamera {
public:
    void init(const Vec3& position, float yaw = -90.f, float pitch = 0.f) {
        m_position = position;
        m_yaw = yaw;
        m_pitch = pitch;
        updateVectors();
    }

    void processMouseLook(float deltaX, float deltaY) {
        m_yaw += deltaX * m_sensitivity;
        m_pitch += deltaY * m_sensitivity;
        if (m_pitch > m_maxPitch) m_pitch = m_maxPitch;
        if (m_pitch < m_minPitch) m_pitch = m_minPitch;
        updateVectors();
    }

    [[nodiscard]] Vec3 position() const { return m_position; }
    [[nodiscard]] Vec3 forward() const { return m_forward; }
    [[nodiscard]] Vec3 right() const { return m_right; }
    [[nodiscard]] Vec3 up() const { return m_up; }
    [[nodiscard]] float yaw() const { return m_yaw; }
    [[nodiscard]] float pitch() const { return m_pitch; }

    void setPosition(const Vec3& pos) { m_position = pos; }
    void setSensitivity(float s) { m_sensitivity = s; }
    [[nodiscard]] float sensitivity() const { return m_sensitivity; }

    void setPitchLimits(float minPitch, float maxPitch) {
        m_minPitch = minPitch;
        m_maxPitch = maxPitch;
    }

    [[nodiscard]] Camera toCamera(float fov = 60.f, float near = 0.1f, float far = 1000.f) const {
        Camera cam;
        cam.position = m_position;
        cam.target = m_position + m_forward;
        cam.up = m_up;
        cam.fov = fov;
        cam.nearPlane = near;
        cam.farPlane = far;
        return cam;
    }

private:
    Vec3 m_position{0, 0, 0};
    float m_yaw = -90.f;
    float m_pitch = 0.f;
    float m_sensitivity = 0.1f;
    float m_minPitch = -89.f;
    float m_maxPitch = 89.f;
    Vec3 m_forward{0, 0, -1};
    Vec3 m_right{1, 0, 0};
    Vec3 m_up{0, 1, 0};

    void updateVectors() {
        constexpr float DEG2RAD = 3.14159265358979f / 180.f;
        float yawRad = m_yaw * DEG2RAD;
        float pitchRad = m_pitch * DEG2RAD;
        Vec3 fwd;
        fwd.x = std::cos(yawRad) * std::cos(pitchRad);
        fwd.y = std::sin(pitchRad);
        fwd.z = std::sin(yawRad) * std::cos(pitchRad);
        m_forward = fwd.normalized();
        const Vec3 worldUp{0.f, 1.f, 0.f};
        m_right = m_forward.cross(worldUp).normalized();
        m_up = m_right.cross(m_forward).normalized();
    }
};

struct MovementInput {
    bool forward = false;
    bool backward = false;
    bool left = false;
    bool right = false;
    bool jump = false;
    bool sprint = false;
    bool crouch = false;
};

class PlayerMovement {
public:
    void init(const Vec3& startPosition) {
        m_position = startPosition;
        m_velocity = {0, 0, 0};
        m_grounded = false;
        m_sprinting = false;
        m_crouching = false;
    }

    void update(float dt, const MovementInput& input, const FPSCamera& camera) {
        // Flatten camera vectors to XZ plane
        Vec3 camFwd = camera.forward();
        camFwd.y = 0.f;
        camFwd = camFwd.normalized();
        Vec3 camRight = camera.right();
        camRight.y = 0.f;
        camRight = camRight.normalized();

        Vec3 moveDir{0, 0, 0};
        if (input.forward)  moveDir = moveDir + camFwd;
        if (input.backward) moveDir = moveDir - camFwd;
        if (input.right)    moveDir = moveDir + camRight;
        if (input.left)     moveDir = moveDir - camRight;

        if (moveDir.lengthSq() > 1e-6f) moveDir = moveDir.normalized();

        m_sprinting = input.sprint && !input.crouch;
        m_crouching = input.crouch && !input.sprint;
        float speed = m_walkSpeed;
        if (m_sprinting) speed = m_sprintSpeed;
        else if (m_crouching) speed = m_crouchSpeed;

        m_velocity.x = moveDir.x * speed;
        m_velocity.z = moveDir.z * speed;

        // Gravity
        m_velocity.y += m_gravity * dt;

        // Jump
        if (m_grounded && input.jump) {
            m_velocity.y = m_jumpForce;
            m_grounded = false;
        }

        // Integrate
        m_position.x += m_velocity.x * dt;
        m_position.y += m_velocity.y * dt;
        m_position.z += m_velocity.z * dt;

        // Ground plane
        if (m_position.y < 0.f) {
            m_position.y = 0.f;
            m_velocity.y = 0.f;
            m_grounded = true;
        }
    }

    [[nodiscard]] Vec3 position() const { return m_position; }
    [[nodiscard]] Vec3 velocity() const { return m_velocity; }
    [[nodiscard]] bool isGrounded() const { return m_grounded; }
    [[nodiscard]] bool isSprinting() const { return m_sprinting; }
    [[nodiscard]] bool isCrouching() const { return m_crouching; }
    [[nodiscard]] float currentSpeed() const {
        return std::sqrt(m_velocity.x * m_velocity.x + m_velocity.z * m_velocity.z);
    }

    void setWalkSpeed(float s) { m_walkSpeed = s; }
    void setSprintSpeed(float s) { m_sprintSpeed = s; }
    void setCrouchSpeed(float s) { m_crouchSpeed = s; }
    void setJumpForce(float f) { m_jumpForce = f; }
    void setGravity(float g) { m_gravity = g; }
    [[nodiscard]] float walkSpeed() const { return m_walkSpeed; }
    [[nodiscard]] float sprintSpeed() const { return m_sprintSpeed; }
    [[nodiscard]] float crouchSpeed() const { return m_crouchSpeed; }
    [[nodiscard]] float jumpForce() const { return m_jumpForce; }
    [[nodiscard]] float gravity() const { return m_gravity; }

private:
    Vec3 m_position{0, 0, 0};
    Vec3 m_velocity{0, 0, 0};
    float m_walkSpeed = 5.f;
    float m_sprintSpeed = 8.f;
    float m_crouchSpeed = 2.5f;
    float m_jumpForce = 7.f;
    float m_gravity = -20.f;
    bool m_grounded = false;
    bool m_sprinting = false;
    bool m_crouching = false;
};

struct PlayerAABB {
    Vec3 min;
    Vec3 max;

    static PlayerAABB fromPosition(const Vec3& pos, float width = 0.6f, float height = 1.8f) {
        float hw = width * 0.5f;
        return {{pos.x - hw, pos.y, pos.z - hw},
                {pos.x + hw, pos.y + height, pos.z + hw}};
    }
};

class VoxelCollider {
public:
    Vec3 resolveCollision(const WorldState& world, const Vec3& position,
                          const Vec3& velocity, float dt,
                          float playerWidth = 0.6f, float playerHeight = 1.8f) {
        Vec3 pos = position;

        // X axis
        Vec3 tryX = pos;
        tryX.x += velocity.x * dt;
        if (aabbOverlapsSolid(world, PlayerAABB::fromPosition(tryX, playerWidth, playerHeight))) {
            tryX.x = pos.x;
        }
        pos = tryX;

        // Y axis
        Vec3 tryY = pos;
        tryY.y += velocity.y * dt;
        if (aabbOverlapsSolid(world, PlayerAABB::fromPosition(tryY, playerWidth, playerHeight))) {
            tryY.y = pos.y;
        }
        pos = tryY;

        // Z axis
        Vec3 tryZ = pos;
        tryZ.z += velocity.z * dt;
        if (aabbOverlapsSolid(world, PlayerAABB::fromPosition(tryZ, playerWidth, playerHeight))) {
            tryZ.z = pos.z;
        }
        pos = tryZ;

        return pos;
    }

    [[nodiscard]] bool wouldCollide(const WorldState& world, const Vec3& position,
                                    float playerWidth = 0.6f, float playerHeight = 1.8f) const {
        return aabbOverlapsSolid(world, PlayerAABB::fromPosition(position, playerWidth, playerHeight));
    }

    [[nodiscard]] bool isOnGround(const WorldState& world, const Vec3& position,
                                  float playerWidth = 0.6f) const {
        float hw = playerWidth * 0.5f;
        float belowY = position.y - 0.05f;
        int minVX = static_cast<int>(std::floor(position.x - hw));
        int maxVX = static_cast<int>(std::floor(position.x + hw));
        int minVZ = static_cast<int>(std::floor(position.z - hw));
        int maxVZ = static_cast<int>(std::floor(position.z + hw));
        int vy = static_cast<int>(std::floor(belowY));
        for (int vx = minVX; vx <= maxVX; ++vx)
            for (int vz = minVZ; vz <= maxVZ; ++vz)
                if (world.getWorld(vx, vy, vz) != VoxelType::Air) return true;
        return false;
    }

private:
    [[nodiscard]] bool aabbOverlapsSolid(const WorldState& world, const PlayerAABB& aabb) const {
        int minVX = static_cast<int>(std::floor(aabb.min.x));
        int maxVX = static_cast<int>(std::floor(aabb.max.x));
        int minVY = static_cast<int>(std::floor(aabb.min.y));
        int maxVY = static_cast<int>(std::floor(aabb.max.y));
        int minVZ = static_cast<int>(std::floor(aabb.min.z));
        int maxVZ = static_cast<int>(std::floor(aabb.max.z));
        for (int vx = minVX; vx <= maxVX; ++vx)
            for (int vy = minVY; vy <= maxVY; ++vy)
                for (int vz = minVZ; vz <= maxVZ; ++vz)
                    if (world.getWorld(vx, vy, vz) != VoxelType::Air) return true;
        return false;
    }
};

class PlayerController {
public:
    void init(const Vec3& startPosition) {
        m_movement.init(startPosition);
        m_camera.init(startPosition + Vec3{0, 1.6f, 0});
    }

    void update(float dt, const MovementInput& moveInput,
                float mouseDeltaX, float mouseDeltaY,
                WorldState& world) {
        m_camera.processMouseLook(mouseDeltaX, mouseDeltaY);
        m_movement.update(dt, moveInput, m_camera);

        Vec3 pos = m_movement.position();
        pos = m_collider.resolveCollision(world, pos, m_movement.velocity(), dt);

        m_camera.setPosition(pos + Vec3{0, 1.6f, 0});
    }

    FPSCamera& camera() { return m_camera; }
    const FPSCamera& camera() const { return m_camera; }
    PlayerMovement& movement() { return m_movement; }
    const PlayerMovement& movement() const { return m_movement; }
    VoxelCollider& collider() { return m_collider; }

    [[nodiscard]] Vec3 position() const { return m_movement.position(); }
    [[nodiscard]] Vec3 lookDirection() const { return m_camera.forward(); }

private:
    FPSCamera m_camera;
    PlayerMovement m_movement;
    VoxelCollider m_collider;
};

// ── G4: Ship Systems ─────────────────────────────────────────────

enum class ShipClass : uint8_t {
    Fighter,
    Corvette,
    Frigate,
    Cruiser,
    Freighter,
    Count
};

inline const char* shipClassName(ShipClass c) {
    switch (c) {
        case ShipClass::Fighter:   return "Fighter";
        case ShipClass::Corvette:  return "Corvette";
        case ShipClass::Frigate:   return "Frigate";
        case ShipClass::Cruiser:   return "Cruiser";
        case ShipClass::Freighter: return "Freighter";
        default:                   return "Unknown";
    }
}

enum class ModuleSlotType : uint8_t {
    Weapon,
    Shield,
    Engine,
    Reactor,
    Cargo,
    Utility,
    Count
};

inline const char* moduleSlotTypeName(ModuleSlotType t) {
    switch (t) {
        case ModuleSlotType::Weapon:  return "Weapon";
        case ModuleSlotType::Shield:  return "Shield";
        case ModuleSlotType::Engine:  return "Engine";
        case ModuleSlotType::Reactor: return "Reactor";
        case ModuleSlotType::Cargo:   return "Cargo";
        case ModuleSlotType::Utility: return "Utility";
        default:                      return "Unknown";
    }
}

struct ShipModule {
    StringID name;
    ModuleSlotType slotType = ModuleSlotType::Utility;
    int tier = 1;
    float health = 100.f;
    float maxHealth = 100.f;
    bool active = true;

    float damage = 0.f;
    float fireRate = 0.f;
    float shieldCapacity = 0.f;
    float shieldRegenRate = 0.f;
    float thrustPower = 0.f;
    float powerOutput = 0.f;
    float cargoCapacity = 0.f;

    bool isDestroyed() const { return health <= 0.f; }

    void takeDamage(float amount) {
        if (amount <= 0.f) return;
        health -= amount;
        if (health <= 0.f) {
            health = 0.f;
            active = false;
        }
    }

    void repair(float amount) {
        if (amount <= 0.f) return;
        health += amount;
        if (health > maxHealth) health = maxHealth;
        if (health > 0.f) active = true;
    }
};

struct ShipStats {
    float totalThrust = 0.f;
    float totalPowerOutput = 0.f;
    float totalPowerDraw = 0.f;
    float totalShieldCapacity = 0.f;
    float totalShieldRegen = 0.f;
    float totalCargoCapacity = 0.f;
    float maxWeaponDPS = 0.f;
    int activeModuleCount = 0;
    int destroyedModuleCount = 0;
};

class Ship {
public:
    void init(ShipClass shipClass, StringID shipName) {
        m_class = shipClass;
        m_name = shipName;
        m_modules.clear();
        switch (shipClass) {
            case ShipClass::Fighter:
                m_maxHull = 100.f; m_maxShield = 50.f; m_maxModules = 4; break;
            case ShipClass::Corvette:
                m_maxHull = 200.f; m_maxShield = 100.f; m_maxModules = 6; break;
            case ShipClass::Frigate:
                m_maxHull = 400.f; m_maxShield = 200.f; m_maxModules = 8; break;
            case ShipClass::Cruiser:
                m_maxHull = 800.f; m_maxShield = 400.f; m_maxModules = 12; break;
            case ShipClass::Freighter:
                m_maxHull = 300.f; m_maxShield = 75.f; m_maxModules = 10; break;
            default: break;
        }
        m_hull = m_maxHull;
        m_shield = m_maxShield;
        m_shieldRegenRate = 5.f;
    }

    bool addModule(const ShipModule& module) {
        if (static_cast<int>(m_modules.size()) >= m_maxModules) return false;
        m_modules.push_back(module);
        return true;
    }

    void removeModule(int index) {
        if (index >= 0 && index < static_cast<int>(m_modules.size()))
            m_modules.erase(m_modules.begin() + index);
    }

    ShipModule* module(int index) {
        if (index < 0 || index >= static_cast<int>(m_modules.size())) return nullptr;
        return &m_modules[static_cast<size_t>(index)];
    }

    const ShipModule* module(int index) const {
        if (index < 0 || index >= static_cast<int>(m_modules.size())) return nullptr;
        return &m_modules[static_cast<size_t>(index)];
    }

    int moduleCount() const { return static_cast<int>(m_modules.size()); }
    int maxModules() const { return m_maxModules; }

    float hull() const { return m_hull; }
    float maxHull() const { return m_maxHull; }

    void takeDamage(float amount) {
        if (amount <= 0.f) return;
        float remaining = amount;
        if (m_shield > 0.f) {
            if (m_shield >= remaining) {
                m_shield -= remaining;
                return;
            }
            remaining -= m_shield;
            m_shield = 0.f;
        }
        m_hull -= remaining;
        if (m_hull < 0.f) m_hull = 0.f;
    }

    void repair(float amount) {
        if (amount <= 0.f) return;
        m_hull += amount;
        if (m_hull > m_maxHull) m_hull = m_maxHull;
    }

    bool isDestroyed() const { return m_hull <= 0.f; }

    float shield() const { return m_shield; }
    float maxShield() const { return m_maxShield; }

    void rechargeShield(float dt) {
        float totalRegen = m_shieldRegenRate;
        for (const auto& mod : m_modules) {
            if (mod.active && mod.slotType == ModuleSlotType::Shield)
                totalRegen += mod.shieldRegenRate;
        }
        m_shield += totalRegen * dt;
        float maxS = m_maxShield;
        for (const auto& mod : m_modules) {
            if (mod.active && mod.slotType == ModuleSlotType::Shield)
                maxS += mod.shieldCapacity;
        }
        if (m_shield > maxS) m_shield = maxS;
    }

    ShipStats computeStats() const {
        ShipStats stats;
        for (const auto& mod : m_modules) {
            if (mod.isDestroyed()) {
                stats.destroyedModuleCount++;
                continue;
            }
            if (mod.active) {
                stats.activeModuleCount++;
                switch (mod.slotType) {
                    case ModuleSlotType::Engine:
                        stats.totalThrust += mod.thrustPower; break;
                    case ModuleSlotType::Reactor:
                        stats.totalPowerOutput += mod.powerOutput; break;
                    case ModuleSlotType::Shield:
                        stats.totalShieldCapacity += mod.shieldCapacity;
                        stats.totalShieldRegen += mod.shieldRegenRate; break;
                    case ModuleSlotType::Cargo:
                        stats.totalCargoCapacity += mod.cargoCapacity; break;
                    case ModuleSlotType::Weapon: {
                        float dps = (mod.fireRate > 0.f) ? mod.damage * mod.fireRate : 0.f;
                        if (dps > stats.maxWeaponDPS) stats.maxWeaponDPS = dps;
                        break;
                    }
                    default: break;
                }
            }
        }
        return stats;
    }

    ShipClass shipClass() const { return m_class; }
    StringID name() const { return m_name; }

private:
    ShipClass m_class = ShipClass::Fighter;
    StringID m_name;
    float m_hull = 100.f;
    float m_maxHull = 100.f;
    float m_shield = 50.f;
    float m_maxShield = 50.f;
    float m_shieldRegenRate = 5.f;
    std::vector<ShipModule> m_modules;
    int m_maxModules = 4;
};

// ── G4: Flight ───────────────────────────────────────────────────

struct FlightInput {
    float throttle = 0.f;
    float pitch = 0.f;
    float yaw = 0.f;
    float roll = 0.f;
    bool boost = false;
};

struct FlightState {
    Vec3 position{0, 0, 0};
    Vec3 velocity{0, 0, 0};
    Vec3 forward{0, 0, 1};
    Vec3 up{0, 1, 0};
    Vec3 right{1, 0, 0};
    float speed = 0.f;
    float maxSpeed = 100.f;
    float acceleration = 20.f;
    float turnRate = 90.f;
    float boostMultiplier = 2.f;
    bool boosting = false;
};

class FlightController {
public:
    void init(const Vec3& startPos) {
        m_state = FlightState{};
        m_state.position = startPos;
    }

    void update(float dt, const FlightInput& input, const Ship& ship) {
        constexpr float DEG2RAD = 3.14159265358979323846f / 180.f;
        float yawAngle   = -input.yaw   * m_state.turnRate * dt * DEG2RAD;
        float pitchAngle = -input.pitch  * m_state.turnRate * dt * DEG2RAD;

        // Yaw rotation around up axis
        if (std::fabs(yawAngle) > 1e-7f) {
            float cy = std::cos(yawAngle), sy = std::sin(yawAngle);
            Vec3 f = m_state.forward;
            m_state.forward = Vec3{
                f.x * cy + m_state.right.x * sy,
                f.y * cy + m_state.right.y * sy,
                f.z * cy + m_state.right.z * sy
            }.normalized();
            m_state.right = m_state.forward.cross(m_state.up).normalized();
        }

        // Pitch rotation around right axis
        if (std::fabs(pitchAngle) > 1e-7f) {
            float cp = std::cos(pitchAngle), sp = std::sin(pitchAngle);
            Vec3 f = m_state.forward;
            m_state.forward = Vec3{
                f.x * cp + m_state.up.x * sp,
                f.y * cp + m_state.up.y * sp,
                f.z * cp + m_state.up.z * sp
            }.normalized();
            m_state.up = m_state.right.cross(m_state.forward).normalized();
        }

        ShipStats stats = ship.computeStats();
        float thrust = stats.totalThrust > 0.f ? stats.totalThrust : m_state.acceleration;
        float accel = thrust * input.throttle;

        m_state.boosting = input.boost;
        if (m_state.boosting)
            accel *= m_state.boostMultiplier;

        float maxSpd = m_state.maxSpeed;
        if (m_state.boosting) maxSpd *= m_state.boostMultiplier;

        Vec3 targetVel = m_state.forward * (maxSpd * input.throttle);
        if (m_state.boosting)
            targetVel = m_state.forward * (maxSpd * std::fabs(input.throttle));

        Vec3 diff = targetVel - m_state.velocity;
        float diffLen = diff.length();
        float step = std::fabs(accel) * dt;
        if (diffLen > 0.f && step > 0.f) {
            if (step >= diffLen)
                m_state.velocity = targetVel;
            else
                m_state.velocity = m_state.velocity + diff.normalized() * step;
        }

        m_state.position = m_state.position + m_state.velocity * dt;
        m_state.speed = m_state.velocity.length();
    }

    const FlightState& state() const { return m_state; }
    FlightState& state() { return m_state; }
    Vec3 position() const { return m_state.position; }
    Vec3 velocity() const { return m_state.velocity; }
    float speed() const { return m_state.speed; }

private:
    FlightState m_state;
};

// ── G4: Combat ───────────────────────────────────────────────────

struct WeaponState {
    int moduleIndex = -1;
    float cooldown = 0.f;
    float range = 100.f;
    bool firing = false;

    bool isReady() const { return cooldown <= 0.f; }

    void tick(float dt) {
        if (cooldown > 0.f) {
            cooldown -= dt;
            if (cooldown < 0.f) cooldown = 0.f;
        }
    }
};

struct CombatTarget {
    Vec3 position;
    float distance = 0.f;
    bool inRange = false;
    bool inFiringArc = false;
};

class CombatSystem {
public:
    void init() { m_baseDamageVariance = 0.1f; }

    float calculateDamage(const Ship& ship, int weaponModuleIndex) const {
        const ShipModule* mod = ship.module(weaponModuleIndex);
        if (!mod || mod->slotType != ModuleSlotType::Weapon || !mod->active || mod->isDestroyed())
            return 0.f;
        return mod->damage;
    }

    void tickWeapons(float dt, std::vector<WeaponState>& weapons) {
        for (auto& w : weapons)
            w.tick(dt);
    }

    CombatTarget evaluateTarget(const Vec3& shipPos, const Vec3& shipForward,
                                const Vec3& targetPos, float weaponRange,
                                float firingArc = 30.f) const {
        CombatTarget ct;
        ct.position = targetPos;
        Vec3 toTarget = targetPos - shipPos;
        ct.distance = toTarget.length();
        ct.inRange = ct.distance <= weaponRange;

        if (ct.distance > 1e-7f) {
            Vec3 dir = toTarget.normalized();
            float dotVal = shipForward.dot(dir);
            if (dotVal > 1.f) dotVal = 1.f;
            if (dotVal < -1.f) dotVal = -1.f;
            float angle = std::acos(dotVal) * (180.f / 3.14159265358979323846f);
            ct.inFiringArc = angle <= firingArc;
        } else {
            ct.inFiringArc = true;
        }
        return ct;
    }

    float applyDamage(Ship& attacker, Ship& target, int weaponModuleIndex) {
        float dmg = calculateDamage(attacker, weaponModuleIndex);
        if (dmg <= 0.f) return 0.f;
        target.takeDamage(dmg);
        return dmg;
    }

private:
    float m_baseDamageVariance = 0.1f;
};

// ── G5: Fleet AI ─────────────────────────────────────────────────

enum class FormationType : uint8_t { Line, Wedge, Column, Spread, Defensive, Count };
inline const char* formationTypeName(FormationType t) {
    switch(t) {
        case FormationType::Line: return "Line";
        case FormationType::Wedge: return "Wedge";
        case FormationType::Column: return "Column";
        case FormationType::Spread: return "Spread";
        case FormationType::Defensive: return "Defensive";
        default: return "Unknown";
    }
}

struct FormationSlot {
    int shipIndex = -1;
    Vec3 offset{0,0,0};
    bool occupied = false;
};

class Formation {
public:
    void init(FormationType type, int shipCount, float spacing = 20.f) {
        m_type = type; m_spacing = spacing;
        m_slots.resize(shipCount);
        generateSlotOffsets();
    }
    FormationType type() const { return m_type; }
    int slotCount() const { return (int)m_slots.size(); }
    FormationSlot& slot(int i) { return m_slots[i]; }
    const FormationSlot& slot(int i) const { return m_slots[i]; }
    void setSpacing(float s) { m_spacing = s; generateSlotOffsets(); }
    float spacing() const { return m_spacing; }

    Vec3 getSlotWorldPosition(int idx, const Vec3& leaderPos, const Vec3& leaderFwd) const {
        if (idx < 0 || idx >= (int)m_slots.size()) return leaderPos;
        Vec3 up{0,1,0};
        Vec3 right = leaderFwd.cross(up).normalized();
        const Vec3& off = m_slots[idx].offset;
        return {leaderPos.x + right.x*off.x + leaderFwd.x*off.z,
                leaderPos.y + off.y,
                leaderPos.z + right.z*off.x + leaderFwd.z*off.z};
    }

private:
    FormationType m_type = FormationType::Line;
    std::vector<FormationSlot> m_slots;
    float m_spacing = 20.f;

    void generateSlotOffsets() {
        int n = (int)m_slots.size();
        for (int i = 0; i < n; ++i) {
            switch (m_type) {
                case FormationType::Line:
                    m_slots[i].offset = {(i - n/2) * m_spacing, 0, 0};
                    break;
                case FormationType::Wedge:
                    m_slots[i].offset = {(i - n/2) * m_spacing, 0, -(float)std::abs(i - n/2) * m_spacing};
                    break;
                case FormationType::Column:
                    m_slots[i].offset = {0, 0, -(float)i * m_spacing};
                    break;
                case FormationType::Spread:
                    m_slots[i].offset = {((i%3)-1) * m_spacing, 0, -(i/3) * m_spacing};
                    break;
                case FormationType::Defensive: {
                    float angle = (2.f * 3.14159265f * i) / (float)(n > 0 ? n : 1);
                    m_slots[i].offset = {std::cos(angle)*m_spacing, 0, std::sin(angle)*m_spacing};
                    break;
                }
                default: m_slots[i].offset = {0,0,0}; break;
            }
        }
    }
};

struct CaptainPersonality {
    float aggression = 0.5f;
    float caution = 0.5f;
    float loyalty = 0.5f;
    float initiative = 0.5f;
    float morale = 1.0f;
    float confidence = 1.0f;

    void adjustMorale(float delta) {
        morale += delta;
        if (morale < 0.f) morale = 0.f;
        if (morale > 1.f) morale = 1.f;
    }
    void adjustConfidence(float delta) {
        confidence += delta;
        if (confidence < 0.f) confidence = 0.f;
        if (confidence > 1.f) confidence = 1.f;
    }
    bool willFlee() const { return morale < 0.2f && caution > 0.6f; }
    bool willCharge() const { return aggression > 0.7f && confidence > 0.5f; }
};

enum class CaptainOrder : uint8_t {
    HoldPosition, AttackTarget, DefendTarget, FollowLeader, Patrol, Retreat, FreeEngage, Count
};
inline const char* captainOrderName(CaptainOrder o) {
    switch(o) {
        case CaptainOrder::HoldPosition: return "HoldPosition";
        case CaptainOrder::AttackTarget: return "AttackTarget";
        case CaptainOrder::DefendTarget: return "DefendTarget";
        case CaptainOrder::FollowLeader: return "FollowLeader";
        case CaptainOrder::Patrol: return "Patrol";
        case CaptainOrder::Retreat: return "Retreat";
        case CaptainOrder::FreeEngage: return "FreeEngage";
        default: return "Unknown";
    }
}

class AICaptain {
public:
    void init(StringID name, const CaptainPersonality& p) { m_name = name; m_personality = p; }
    StringID name() const { return m_name; }
    const CaptainPersonality& personality() const { return m_personality; }
    CaptainPersonality& personality() { return m_personality; }
    CaptainOrder currentOrder() const { return m_hasOverride ? m_overrideOrder : m_currentOrder; }
    void setOrder(CaptainOrder o) { m_currentOrder = o; }
    void overrideOrder(CaptainOrder o) { m_overrideOrder = o; m_hasOverride = true; }
    bool hasOverride() const { return m_hasOverride; }
    void clearOverride() { m_hasOverride = false; }

    CaptainOrder evaluate(float hullPct, float shieldPct, float enemyDist,
                          int alliesNearby, int enemiesNearby) const {
        if (m_personality.willFlee() && (hullPct < 0.3f || shieldPct < 0.2f))
            return CaptainOrder::Retreat;
        if (m_personality.willCharge() && enemyDist < 200.f) return CaptainOrder::AttackTarget;
        if (enemiesNearby > alliesNearby * 2 && m_personality.caution > 0.5f)
            return CaptainOrder::DefendTarget;
        return m_hasOverride ? m_overrideOrder : m_currentOrder;
    }

private:
    StringID m_name;
    CaptainPersonality m_personality;
    CaptainOrder m_currentOrder = CaptainOrder::FollowLeader;
    CaptainOrder m_overrideOrder = CaptainOrder::FollowLeader;
    bool m_hasOverride = false;
};

struct FleetShip {
    Ship ship;
    AICaptain captain;
    FlightController flight;
    int formationSlot = -1;
    bool active = true;
};

class Fleet {
public:
    void init(StringID name) { m_name = name; }

    int addShip(FleetShip s) {
        int idx = (int)m_ships.size();
        m_ships.push_back(std::move(s));
        return idx;
    }
    void removeShip(int idx) {
        if (idx >= 0 && idx < (int)m_ships.size()) m_ships[idx].active = false;
    }
    FleetShip* ship(int i) { return (i>=0&&i<(int)m_ships.size()) ? &m_ships[i] : nullptr; }
    const FleetShip* ship(int i) const { return (i>=0&&i<(int)m_ships.size()) ? &m_ships[i] : nullptr; }
    int shipCount() const { return (int)m_ships.size(); }
    int activeShipCount() const {
        int c = 0; for (auto& s : m_ships) if (s.active) ++c; return c;
    }

    void setFormation(FormationType t, float spacing = 20.f) {
        m_formation.init(t, (int)m_ships.size(), spacing);
    }
    const Formation& formation() const { return m_formation; }
    Formation& formation() { return m_formation; }

    void issueOrder(CaptainOrder o) { for (auto& s : m_ships) s.captain.setOrder(o); }
    void issueOrderTo(int idx, CaptainOrder o) {
        if (auto* s = ship(idx)) s->captain.setOrder(o);
    }

    void setLeader(int idx) { m_leaderIndex = idx; }
    int leaderIndex() const { return m_leaderIndex; }
    FleetShip* leader() { return ship(m_leaderIndex); }

    void tick(float /*dt*/) {
        for (auto& s : m_ships) {
            if (!s.active) continue;
            float hullPct = s.ship.hull() / s.ship.maxHull();
            float shieldPct = s.ship.shield() / s.ship.maxShield();
            CaptainOrder ord = s.captain.evaluate(hullPct, shieldPct, 500.f, 1, 0);
            s.captain.setOrder(ord);
        }
    }

    float fleetMorale() const {
        if (m_ships.empty()) return 1.f;
        float sum = 0.f; int c = 0;
        for (auto& s : m_ships) if (s.active) { sum += s.captain.personality().morale; ++c; }
        return c > 0 ? sum / c : 1.f;
    }
    float fleetStrength() const {
        float sum = 0.f;
        for (auto& s : m_ships)
            if (s.active) sum += s.ship.hull() / s.ship.maxHull();
        return sum;
    }

    StringID name() const { return m_name; }

private:
    StringID m_name;
    std::vector<FleetShip> m_ships;
    Formation m_formation;
    int m_leaderIndex = 0;
};

// ── G6: Economy ───────────────────────────────────────────────────

struct MarketItem {
    ResourceType resource;
    int quantity = 0;
    float buyPrice = 10.f;
    float sellPrice = 8.f;
};

class Market {
public:
    void init() { m_items.clear(); }

    void listItem(ResourceType res, int qty, float buyPrice, float sellPrice) {
        for (auto& item : m_items) {
            if (item.resource == res) { item.quantity += qty; return; }
        }
        m_items.push_back({res, qty, buyPrice, sellPrice});
    }

    bool buy(ResourceInventory& inv, ResourceType res, int amount, float& credits) {
        for (auto& item : m_items) {
            if (item.resource != res) continue;
            if (item.quantity < amount) return false;
            float cost = item.buyPrice * amount;
            if (credits < cost) return false;
            credits -= cost;
            item.quantity -= amount;
            inv.add(res, amount);
            return true;
        }
        return false;
    }

    bool sell(ResourceInventory& inv, ResourceType res, int amount, float& credits) {
        if (inv.count(res) < amount) return false;
        float gain = 0.f;
        for (auto& item : m_items) {
            if (item.resource == res) { gain = item.sellPrice * amount; break; }
        }
        if (gain == 0.f) {
            gain = 5.f * amount;
        }
        inv.remove(res, amount);
        credits += gain;
        return true;
    }

    const MarketItem* findItem(ResourceType res) const {
        for (auto& item : m_items) if (item.resource == res) return &item;
        return nullptr;
    }

    int itemCount() const { return (int)m_items.size(); }

private:
    std::vector<MarketItem> m_items;
};

struct RefiningRecipe {
    ResourceType input;
    int inputAmount;
    ResourceType output;
    int outputAmount;
    float timeRequired;
};

class Refinery {
public:
    void addRecipe(const RefiningRecipe& r) { m_recipes.push_back(r); }

    const RefiningRecipe* findRecipe(ResourceType input) const {
        for (auto& r : m_recipes) if (r.input == input) return &r;
        return nullptr;
    }

    float startRefining(ResourceInventory& inv, ResourceType input, int amount) {
        const RefiningRecipe* r = findRecipe(input);
        if (!r) return 0.f;
        int batches = amount / r->inputAmount;
        if (batches <= 0) return 0.f;
        if (!inv.remove(input, batches * r->inputAmount)) return 0.f;
        m_pendingOutput.push_back({r->output, batches * r->outputAmount});
        return r->timeRequired * batches;
    }

    void collectOutput(ResourceInventory& inv) {
        for (auto& p : m_pendingOutput) inv.add(p.first, p.second);
        m_pendingOutput.clear();
    }

    int recipeCount() const { return (int)m_recipes.size(); }

private:
    std::vector<RefiningRecipe> m_recipes;
    std::vector<std::pair<ResourceType, int>> m_pendingOutput;
};

struct ManufacturingRecipe {
    StringID name;
    std::vector<std::pair<ResourceType, int>> inputs;
    ResourceType output;
    int outputAmount;
    float timeRequired;
};

class Manufacturer {
public:
    void addRecipe(const ManufacturingRecipe& r) { m_recipes.push_back(r); }

    const ManufacturingRecipe* findRecipe(StringID name) const {
        for (auto& r : m_recipes) if (r.name == name) return &r;
        return nullptr;
    }

    bool canCraft(const ResourceInventory& inv, StringID recipeName) const {
        const ManufacturingRecipe* r = findRecipe(recipeName);
        if (!r) return false;
        for (auto& [res, amt] : r->inputs)
            if (inv.count(res) < amt) return false;
        return true;
    }

    float craft(ResourceInventory& inv, StringID recipeName) {
        const ManufacturingRecipe* r = findRecipe(recipeName);
        if (!r || !canCraft(inv, recipeName)) return 0.f;
        for (auto& [res, amt] : r->inputs) inv.remove(res, amt);
        inv.add(r->output, r->outputAmount);
        return r->timeRequired;
    }

    int recipeCount() const { return (int)m_recipes.size(); }

private:
    std::vector<ManufacturingRecipe> m_recipes;
};

// ── G7: Exploration ───────────────────────────────────────────────

enum class SectorType : uint8_t { Normal, Nebula, AsteroidField, DeepSpace, AncientRuins, Count };
inline const char* sectorTypeName(SectorType t) {
    switch(t) {
        case SectorType::Normal: return "Normal";
        case SectorType::Nebula: return "Nebula";
        case SectorType::AsteroidField: return "AsteroidField";
        case SectorType::DeepSpace: return "DeepSpace";
        case SectorType::AncientRuins: return "AncientRuins";
        default: return "Unknown";
    }
}

struct SectorInfo {
    StringID name;
    SectorType type = SectorType::Normal;
    Vec3 position{0,0,0};
    float scanProgress = 0.f;
    bool fullyScanned = false;
    bool hasWormhole = false;
    bool hasAncientTech = false;
};

class ProbeScanner {
public:
    void init(float scanRate = 0.1f) { m_scanRate = scanRate; m_active = false; }

    void startScan(SectorInfo& sector) {
        m_targetSector = &sector;
        m_active = true;
    }
    void stopScan() { m_active = false; m_targetSector = nullptr; }
    bool isActive() const { return m_active; }

    bool tick(float dt) {
        if (!m_active || !m_targetSector) return false;
        m_targetSector->scanProgress += m_scanRate * dt;
        if (m_targetSector->scanProgress >= 1.f) {
            m_targetSector->scanProgress = 1.f;
            m_targetSector->fullyScanned = true;
            m_active = false;
            m_targetSector = nullptr;
            return true;
        }
        return false;
    }

    float scanRate() const { return m_scanRate; }
    void setScanRate(float r) { m_scanRate = r; }

private:
    float m_scanRate = 0.1f;
    bool m_active = false;
    SectorInfo* m_targetSector = nullptr;
};

struct WormholeLink {
    StringID fromSector;
    StringID toSector;
    float stability = 1.f;
    bool twoWay = true;

    bool isTraversable() const { return stability > 0.1f; }
    void degrade(float amount) {
        stability -= amount;
        if (stability < 0.f) stability = 0.f;
    }
};

class StarMap {
public:
    void addSector(const SectorInfo& sector) {
        m_sectors[sector.name] = sector;
    }
    SectorInfo* findSector(StringID name) {
        auto it = m_sectors.find(name);
        return it != m_sectors.end() ? &it->second : nullptr;
    }
    const SectorInfo* findSector(StringID name) const {
        auto it = m_sectors.find(name);
        return it != m_sectors.end() ? &it->second : nullptr;
    }

    void addWormhole(const WormholeLink& link) { m_wormholes.push_back(link); }

    std::vector<StringID> getReachableSectors(StringID from) const {
        std::vector<StringID> result;
        for (auto& wh : m_wormholes) {
            if (!wh.isTraversable()) continue;
            if (wh.fromSector == from) result.push_back(wh.toSector);
            else if (wh.twoWay && wh.toSector == from) result.push_back(wh.fromSector);
        }
        return result;
    }

    int sectorCount() const { return (int)m_sectors.size(); }
    int wormholeCount() const { return (int)m_wormholes.size(); }

    std::vector<StringID> getAncientTechSectors() const {
        std::vector<StringID> result;
        for (auto& [name, s] : m_sectors)
            if (s.hasAncientTech) result.push_back(name);
        return result;
    }

private:
    std::map<StringID, SectorInfo> m_sectors;
    std::vector<WormholeLink> m_wormholes;
};

struct AncientTechFragment {
    StringID name;
    StringID sectorFound;
    int tier = 1;
    bool analyzed = false;
    float damageBonus = 0.f;
    float shieldBonus = 0.f;
    float speedBonus = 0.f;
};

class AncientTechRegistry {
public:
    void add(const AncientTechFragment& f) { m_fragments.push_back(f); }
    AncientTechFragment* find(StringID name) {
        for (auto& f : m_fragments) if (f.name == name) return &f;
        return nullptr;
    }
    void analyze(StringID name) {
        if (auto* f = find(name)) { f->analyzed = true; f->damageBonus = f->tier * 0.05f; }
    }
    int count() const { return (int)m_fragments.size(); }
    int analyzedCount() const {
        int c = 0; for (auto& f : m_fragments) if (f.analyzed) ++c; return c;
    }

private:
    std::vector<AncientTechFragment> m_fragments;
};

// ── G8: FPS Interiors ─────────────────────────────────────────────

enum class RoomType : uint8_t { Bridge, Engineering, MedBay, Cargo, Airlock, Corridor, Count };
inline const char* roomTypeName(RoomType t) {
    switch(t) {
        case RoomType::Bridge: return "Bridge";
        case RoomType::Engineering: return "Engineering";
        case RoomType::MedBay: return "MedBay";
        case RoomType::Cargo: return "Cargo";
        case RoomType::Airlock: return "Airlock";
        case RoomType::Corridor: return "Corridor";
        default: return "Unknown";
    }
}

struct ShipRoom {
    StringID name;
    RoomType type = RoomType::Corridor;
    float oxygenLevel = 1.f;
    float temperature = 20.f;
    bool pressurized = true;
    std::vector<StringID> connectedRooms;

    void connect(StringID other) { connectedRooms.push_back(other); }
    bool isConnectedTo(StringID other) const {
        for (auto& r : connectedRooms) if (r == other) return true;
        return false;
    }
    bool isHabitable() const { return pressurized && oxygenLevel > 0.2f && temperature > -10.f; }
};

class ShipInterior {
public:
    void addRoom(const ShipRoom& room) { m_rooms[room.name] = room; }
    ShipRoom* findRoom(StringID name) {
        auto it = m_rooms.find(name);
        return it != m_rooms.end() ? &it->second : nullptr;
    }
    const ShipRoom* findRoom(StringID name) const {
        auto it = m_rooms.find(name);
        return it != m_rooms.end() ? &it->second : nullptr;
    }

    int roomCount() const { return (int)m_rooms.size(); }

    void decompress(StringID roomName) {
        if (auto* r = findRoom(roomName)) { r->oxygenLevel = 0.f; r->pressurized = false; }
    }

    void repressurize(StringID roomName) {
        if (auto* r = findRoom(roomName)) { r->oxygenLevel = 1.f; r->pressurized = true; }
    }

    int habitableRoomCount() const {
        int c = 0;
        for (auto& [n, r] : m_rooms) if (r.isHabitable()) ++c;
        return c;
    }

private:
    std::map<StringID, ShipRoom> m_rooms;
};

struct EVAState {
    bool active = false;
    float suitIntegrity = 100.f;
    float oxygenSupply = 300.f;
    float jetpackFuel = 100.f;
    Vec3 velocity{0,0,0};

    bool isAlive() const { return suitIntegrity > 0.f && oxygenSupply > 0.f; }

    void tick(float dt) {
        if (!active) return;
        oxygenSupply -= dt;
        if (oxygenSupply < 0.f) oxygenSupply = 0.f;
    }

    void useThruster(const Vec3& impulse, float fuelCost) {
        if (jetpackFuel <= 0.f) return;
        velocity = velocity + impulse;
        jetpackFuel -= fuelCost;
        if (jetpackFuel < 0.f) jetpackFuel = 0.f;
    }

    void takeSuitDamage(float amount) {
        suitIntegrity -= amount;
        if (suitIntegrity < 0.f) suitIntegrity = 0.f;
    }
};

struct SurvivalStatus {
    float radiation = 0.f;
    float temperature = 37.f;
    bool inVacuum = false;
    bool onFire = false;

    bool isRadiationDangerous() const { return radiation > 50.f; }
    bool isHypothermic() const { return temperature < 35.f; }
    bool isHyperthermic() const { return temperature > 40.f; }
    bool isInDanger() const { return isRadiationDangerous() || isHypothermic() || isHyperthermic() || inVacuum || onFire; }

    void tick(float dt, const ShipRoom* currentRoom) {
        if (!currentRoom || !currentRoom->isHabitable()) {
            inVacuum = true;
            radiation += dt * 2.f;
            temperature -= dt * 5.f;
        } else {
            inVacuum = false;
        }
    }
};

// ── G9: Legend System ─────────────────────────────────────────────

enum class ReputationTier : uint8_t {
    Infamous = 0, Outlaw, Neutral, Trusted, Honored, Legend, Count
};
inline const char* reputationTierName(ReputationTier t) {
    switch(t) {
        case ReputationTier::Infamous: return "Infamous";
        case ReputationTier::Outlaw: return "Outlaw";
        case ReputationTier::Neutral: return "Neutral";
        case ReputationTier::Trusted: return "Trusted";
        case ReputationTier::Honored: return "Honored";
        case ReputationTier::Legend: return "Legend";
        default: return "Unknown";
    }
}

inline ReputationTier reputationTierFromScore(float score) {
    if (score < -500.f) return ReputationTier::Infamous;
    if (score < -100.f) return ReputationTier::Outlaw;
    if (score < 100.f)  return ReputationTier::Neutral;
    if (score < 500.f)  return ReputationTier::Trusted;
    if (score < 1000.f) return ReputationTier::Honored;
    return ReputationTier::Legend;
}

class PlayerReputation {
public:
    void adjustReputation(StringID faction, float delta) {
        m_scores[faction] += delta;
    }
    float getReputation(StringID faction) const {
        auto it = m_scores.find(faction);
        return it != m_scores.end() ? it->second : 0.f;
    }
    ReputationTier getTier(StringID faction) const {
        return reputationTierFromScore(getReputation(faction));
    }
    int factionCount() const { return (int)m_scores.size(); }
    float globalFame() const {
        float sum = 0.f;
        for (auto& [k, v] : m_scores) sum += std::abs(v);
        return sum;
    }

private:
    std::map<StringID, float> m_scores;
};

struct WorldBias {
    StringID sectorName;
    float economyModifier = 1.f;
    float dangerLevel = 0.f;
    float loyaltyToPlayer = 0.f;

    bool isFriendly() const { return loyaltyToPlayer > 0.3f; }
    bool isHostile() const { return loyaltyToPlayer < -0.3f; }
};

class WorldBiasMap {
public:
    void setBias(const WorldBias& bias) { m_biases[bias.sectorName] = bias; }
    WorldBias* getBias(StringID sectorName) {
        auto it = m_biases.find(sectorName);
        return it != m_biases.end() ? &it->second : nullptr;
    }
    const WorldBias* getBias(StringID sectorName) const {
        auto it = m_biases.find(sectorName);
        return it != m_biases.end() ? &it->second : nullptr;
    }

    void updateFromReputation(StringID /*faction*/, float reputationDelta) {
        for (auto& [name, bias] : m_biases)
            bias.loyaltyToPlayer += reputationDelta * 0.01f;
    }

    int biasCount() const { return (int)m_biases.size(); }

private:
    std::map<StringID, WorldBias> m_biases;
};

struct NPCMemoryEntry {
    StringID eventType;
    float timestamp = 0.f;
    float weight = 1.f;
    bool positive = true;
};

class NPCMemory {
public:
    void remember(const NPCMemoryEntry& entry) { m_entries.push_back(entry); }

    void decay(float dt, float decayRate = 0.01f) {
        for (auto& e : m_entries) {
            e.weight -= decayRate * dt;
            if (e.weight < 0.f) e.weight = 0.f;
        }
        m_entries.erase(
            std::remove_if(m_entries.begin(), m_entries.end(),
                           [](const NPCMemoryEntry& e){ return e.weight <= 0.f; }),
            m_entries.end());
    }

    float dispositionTowardPlayer() const {
        float sum = 0.f;
        for (auto& e : m_entries)
            sum += e.positive ? e.weight : -e.weight;
        return sum;
    }

    int entryCount() const { return (int)m_entries.size(); }
    bool remembers(StringID eventType) const {
        for (auto& e : m_entries) if (e.eventType == eventType) return true;
        return false;
    }

private:
    std::vector<NPCMemoryEntry> m_entries;
};

class LegendStatus {
public:
    void init() {}

    PlayerReputation& reputation() { return m_reputation; }
    const PlayerReputation& reputation() const { return m_reputation; }
    WorldBiasMap& worldBias() { return m_worldBias; }
    const WorldBiasMap& worldBias() const { return m_worldBias; }

    ReputationTier overallTier() const {
        float fame = m_reputation.globalFame();
        return reputationTierFromScore(fame - 500.f);
    }

    bool isLegend() const {
        return m_reputation.globalFame() >= 2000.f;
    }

private:
    PlayerReputation m_reputation;
    WorldBiasMap m_worldBias;
};

// ── G10: Quest & Mission System ──────────────────────────────────

enum class MissionObjectiveType {
    Kill,
    Collect,
    Deliver,
    Explore,
    Survive,
    Escort
};

inline const char* missionObjectiveTypeName(MissionObjectiveType t) {
    switch (t) {
        case MissionObjectiveType::Kill:    return "Kill";
        case MissionObjectiveType::Collect: return "Collect";
        case MissionObjectiveType::Deliver: return "Deliver";
        case MissionObjectiveType::Explore: return "Explore";
        case MissionObjectiveType::Survive: return "Survive";
        case MissionObjectiveType::Escort:  return "Escort";
        default:                            return "Unknown";
    }
}

struct MissionObjective {
    MissionObjectiveType type = MissionObjectiveType::Kill;
    StringID targetId;
    std::string description;
    int required = 1;
    int current = 0;

    bool isComplete() const { return current >= required; }
    void progress(int amount = 1) { current = std::min(current + amount, required); }
};

struct MissionReward {
    int credits = 0;
    std::map<ResourceType, int> resources;
    StringID reputationFactionId;
    float reputationAmount = 0.f;
};

enum class MissionStatus { Active, Completed, Failed };

class ActiveMission {
public:
    void init(StringID missionId, const std::string& title) {
        m_missionId = missionId;
        m_title = title;
        m_status = MissionStatus::Active;
    }

    StringID missionId() const { return m_missionId; }
    const std::string& title() const { return m_title; }
    MissionStatus status() const { return m_status; }

    void addObjective(const MissionObjective& obj) { m_objectives.push_back(obj); }
    int objectiveCount() const { return (int)m_objectives.size(); }
    MissionObjective& objective(int i) { return m_objectives[i]; }
    const MissionObjective& objective(int i) const { return m_objectives[i]; }

    void setReward(const MissionReward& r) { m_reward = r; }
    const MissionReward& reward() const { return m_reward; }

    bool allObjectivesComplete() const {
        for (auto& o : m_objectives) if (!o.isComplete()) return false;
        return !m_objectives.empty();
    }

    void complete() { if (m_status == MissionStatus::Active) m_status = MissionStatus::Completed; }
    void fail()     { if (m_status == MissionStatus::Active) m_status = MissionStatus::Failed; }

private:
    StringID m_missionId;
    std::string m_title;
    MissionStatus m_status = MissionStatus::Active;
    std::vector<MissionObjective> m_objectives;
    MissionReward m_reward;
};

class MissionLog {
public:
    void acceptMission(const ActiveMission& mission) {
        m_active.push_back(mission);
    }

    bool completeMission(StringID missionId) {
        for (auto it = m_active.begin(); it != m_active.end(); ++it) {
            if (it->missionId() == missionId) {
                it->complete();
                m_completed.push_back(*it);
                m_active.erase(it);
                return true;
            }
        }
        return false;
    }

    bool failMission(StringID missionId) {
        for (auto it = m_active.begin(); it != m_active.end(); ++it) {
            if (it->missionId() == missionId) {
                it->fail();
                m_failed.push_back(*it);
                m_active.erase(it);
                return true;
            }
        }
        return false;
    }

    ActiveMission* findActive(StringID missionId) {
        for (auto& m : m_active) if (m.missionId() == missionId) return &m;
        return nullptr;
    }

    int activeMissionCount()    const { return (int)m_active.size(); }
    int completedMissionCount() const { return (int)m_completed.size(); }
    int failedMissionCount()    const { return (int)m_failed.size(); }

    const std::vector<ActiveMission>& activeMissions()    const { return m_active; }
    const std::vector<ActiveMission>& completedMissions() const { return m_completed; }

private:
    std::vector<ActiveMission> m_active;
    std::vector<ActiveMission> m_completed;
    std::vector<ActiveMission> m_failed;
};

class QuestChain {
public:
    void init(const std::string& name) { m_name = name; m_currentIndex = 0; }

    const std::string& name() const { return m_name; }
    void addMission(StringID missionId) { m_missionIds.push_back(missionId); }
    int missionCount() const { return (int)m_missionIds.size(); }

    StringID currentMissionId() const {
        if (m_currentIndex < (int)m_missionIds.size())
            return m_missionIds[m_currentIndex];
        return StringID{};
    }

    bool advance() {
        if (m_currentIndex < (int)m_missionIds.size()) {
            ++m_currentIndex;
            return true;
        }
        return false;
    }

    bool isComplete() const { return m_currentIndex >= (int)m_missionIds.size(); }
    int currentIndex() const { return m_currentIndex; }

private:
    std::string m_name;
    std::vector<StringID> m_missionIds;
    int m_currentIndex = 0;
};

// ── G11: Dialogue System ─────────────────────────────────────────

enum class DialogueConditionType {
    Always,
    HasReputation,
    HasItem,
    MissionActive,
    MissionComplete
};

struct DialogueCondition {
    DialogueConditionType type = DialogueConditionType::Always;
    StringID factionId;
    float minReputation = 0.f;
    ResourceType itemType = ResourceType::RawStone;
    int itemAmount = 1;
    StringID missionId;

    bool evaluate(float reputation, int itemCount, bool missionActive, bool missionComplete) const {
        switch (type) {
            case DialogueConditionType::Always:          return true;
            case DialogueConditionType::HasReputation:   return reputation >= minReputation;
            case DialogueConditionType::HasItem:         return itemCount >= itemAmount;
            case DialogueConditionType::MissionActive:   return missionActive;
            case DialogueConditionType::MissionComplete: return missionComplete;
            default:                                     return false;
        }
    }
};

struct DialogueEffect {
    StringID reputationFactionId;
    float reputationDelta = 0.f;
    StringID startMissionId;
    ResourceType giveItemType = ResourceType::RawStone;
    int giveItemAmount = 0;
};

struct DialogueOption {
    std::string text;
    DialogueCondition condition;
    DialogueEffect effect;
    int nextNodeId = -1;
};

struct DialogueNode {
    int nodeId = 0;
    std::string speakerName;
    std::string text;
    std::vector<DialogueOption> options;
};

class DialogueGraph {
public:
    void setStartNodeId(int id) { m_startNodeId = id; }
    int startNodeId() const { return m_startNodeId; }

    void addNode(const DialogueNode& node) { m_nodes[node.nodeId] = node; }
    const DialogueNode* getNode(int id) const {
        auto it = m_nodes.find(id);
        return it != m_nodes.end() ? &it->second : nullptr;
    }
    int nodeCount() const { return (int)m_nodes.size(); }

private:
    int m_startNodeId = 0;
    std::map<int, DialogueNode> m_nodes;
};

class DialogueRunner {
public:
    void init(const DialogueGraph* graph) {
        m_graph = graph;
        m_currentNodeId = graph ? graph->startNodeId() : -1;
        m_complete = false;
    }

    const DialogueNode* currentNode() const {
        return m_graph ? m_graph->getNode(m_currentNodeId) : nullptr;
    }

    bool isComplete() const { return m_complete; }

    const DialogueEffect* selectOption(int optionIndex) {
        const DialogueNode* node = currentNode();
        if (!node || optionIndex < 0 || optionIndex >= (int)node->options.size())
            return nullptr;
        const DialogueOption& opt = node->options[optionIndex];
        m_lastEffect = opt.effect;
        if (opt.nextNodeId < 0) {
            m_complete = true;
        } else {
            m_currentNodeId = opt.nextNodeId;
        }
        return &m_lastEffect;
    }

    int currentNodeId() const { return m_currentNodeId; }

private:
    const DialogueGraph* m_graph = nullptr;
    int m_currentNodeId = -1;
    bool m_complete = false;
    DialogueEffect m_lastEffect;
};

// ── Game Phase G12 — Save/Load System ────────────────────────────

struct SaveSlot {
    int         slotIndex   = 0;
    std::string name;
    std::string timestamp;
    float       playtimeSeconds = 0.f;
    bool        isEmpty = true;
};

struct SaveData {
    // Player state
    Vec3  playerPosition{0.f, 0.f, 0.f};
    float playerHealth    = 100.f;
    float playerEnergy    = 100.f;
    float playerOxygen    = 100.f;
    float playtimeSeconds = 0.f;

    // Inventory
    std::map<std::string, int> inventory;

    // Active missions (by id)
    std::vector<std::string> activeMissionIds;
    std::vector<std::string> completedMissionIds;

    // Reputation per faction
    std::map<std::string, float> reputation;

    // Current sector name
    std::string currentSector;
};

class GameSaveSerializer {
public:
    static JsonValue toJson(const SaveData& data) {
        JsonValue root;
        // Player state
        root["playerHealth"]    = JsonValue(data.playerHealth);
        root["playerEnergy"]    = JsonValue(data.playerEnergy);
        root["playerOxygen"]    = JsonValue(data.playerOxygen);
        root["playtimeSeconds"] = JsonValue(data.playtimeSeconds);
        root["currentSector"]   = JsonValue(data.currentSector);

        // Position
        JsonValue pos;
        pos["x"] = JsonValue(data.playerPosition.x);
        pos["y"] = JsonValue(data.playerPosition.y);
        pos["z"] = JsonValue(data.playerPosition.z);
        root["position"] = pos;

        // Inventory
        JsonValue inv;
        for (const auto& [key, qty] : data.inventory)
            inv[key] = JsonValue(qty);
        root["inventory"] = inv;

        // Missions
        auto activeMissions = JsonValue::array();
        for (const auto& id : data.activeMissionIds)
            activeMissions.push(JsonValue(id));
        root["activeMissions"] = activeMissions;

        auto completedMissions = JsonValue::array();
        for (const auto& id : data.completedMissionIds)
            completedMissions.push(JsonValue(id));
        root["completedMissions"] = completedMissions;

        // Reputation
        JsonValue rep;
        for (const auto& [faction, val] : data.reputation)
            rep[faction] = JsonValue(val);
        root["reputation"] = rep;

        return root;
    }

    static SaveData fromJson(const JsonValue& j) {
        SaveData data;
        if (j.hasKey("playerHealth"))    data.playerHealth    = j["playerHealth"].asFloat();
        if (j.hasKey("playerEnergy"))    data.playerEnergy    = j["playerEnergy"].asFloat();
        if (j.hasKey("playerOxygen"))    data.playerOxygen    = j["playerOxygen"].asFloat();
        if (j.hasKey("playtimeSeconds")) data.playtimeSeconds = j["playtimeSeconds"].asFloat();
        if (j.hasKey("currentSector"))   data.currentSector   = j["currentSector"].asString();

        if (j.hasKey("position")) {
            const auto& pos = j["position"];
            data.playerPosition.x = pos.hasKey("x") ? pos["x"].asFloat() : 0.f;
            data.playerPosition.y = pos.hasKey("y") ? pos["y"].asFloat() : 0.f;
            data.playerPosition.z = pos.hasKey("z") ? pos["z"].asFloat() : 0.f;
        }

        if (j.hasKey("inventory")) {
            const auto& inv = j["inventory"];
            for (const auto& [key, val] : inv.members())
                data.inventory[key] = val.asInt();
        }

        if (j.hasKey("activeMissions")) {
            const auto& arr = j["activeMissions"];
            for (size_t i = 0; i < arr.size(); ++i)
                data.activeMissionIds.push_back(arr[i].asString());
        }

        if (j.hasKey("completedMissions")) {
            const auto& arr = j["completedMissions"];
            for (size_t i = 0; i < arr.size(); ++i)
                data.completedMissionIds.push_back(arr[i].asString());
        }

        if (j.hasKey("reputation")) {
            const auto& rep = j["reputation"];
            for (const auto& [key, val] : rep.members())
                data.reputation[key] = val.asFloat();
        }

        return data;
    }
};

class SaveSystem {
public:
    static constexpr int kMaxSlots = 5;

    void init() {
        m_slots.resize(kMaxSlots);
        for (int i = 0; i < kMaxSlots; ++i) {
            m_slots[i].slotIndex = i;
            m_slots[i].isEmpty   = true;
        }
        NF_LOG_INFO("SaveSystem", "SaveSystem initialized with " +
                    std::to_string(kMaxSlots) + " slots");
    }

    // Persist a SaveData into a slot. Returns false if slotIndex is out of range.
    bool saveGame(int slotIndex, const SaveData& data, const std::string& saveName) {
        if (slotIndex < 0 || slotIndex >= kMaxSlots) return false;
        m_saveData[slotIndex]          = data;
        m_slots[slotIndex].slotIndex   = slotIndex;
        m_slots[slotIndex].name        = saveName;
        m_slots[slotIndex].playtimeSeconds = data.playtimeSeconds;
        m_slots[slotIndex].isEmpty     = false;
        // Serialise to JsonValue (represents the on-disk representation)
        m_serialized[slotIndex]        = GameSaveSerializer::toJson(data);
        NF_LOG_INFO("SaveSystem", "Saved '" + saveName + "' to slot " +
                    std::to_string(slotIndex));
        return true;
    }

    // Load game data from slot. Returns nullptr if slot is empty.
    const SaveData* loadGame(int slotIndex) {
        if (slotIndex < 0 || slotIndex >= kMaxSlots) return nullptr;
        if (m_slots[slotIndex].isEmpty) return nullptr;
        // Re-deserialize from the stored json (simulates disk round-trip)
        m_saveData[slotIndex] = GameSaveSerializer::fromJson(m_serialized[slotIndex]);
        NF_LOG_INFO("SaveSystem", "Loaded slot " + std::to_string(slotIndex));
        return &m_saveData[slotIndex];
    }

    bool deleteSlot(int slotIndex) {
        if (slotIndex < 0 || slotIndex >= kMaxSlots) return false;
        m_slots[slotIndex].isEmpty = true;
        m_slots[slotIndex].name.clear();
        m_saveData.erase(slotIndex);
        m_serialized.erase(slotIndex);
        return true;
    }

    void enableAutoSave(bool enable) { m_autoSaveEnabled = enable; }
    bool isAutoSaveEnabled() const   { return m_autoSaveEnabled; }

    void tickAutoSave(float dt, const SaveData& data) {
        if (!m_autoSaveEnabled) return;
        m_autoSaveTimer += dt;
        if (m_autoSaveTimer >= m_autoSaveIntervalSeconds) {
            m_autoSaveTimer = 0.f;
            saveGame(kAutoSaveSlot, data, "AutoSave");
        }
    }

    [[nodiscard]] const SaveSlot& slot(int index) const {
        static SaveSlot empty;
        if (index < 0 || index >= kMaxSlots) return empty;
        return m_slots[index];
    }

    [[nodiscard]] std::vector<SaveSlot> listSlots() const { return m_slots; }

    int usedSlotCount() const {
        int count = 0;
        for (const auto& s : m_slots) if (!s.isEmpty) ++count;
        return count;
    }

    static constexpr int kAutoSaveSlot = 0;
    float autoSaveIntervalSeconds() const { return m_autoSaveIntervalSeconds; }
    void setAutoSaveInterval(float seconds) { m_autoSaveIntervalSeconds = seconds; }

private:
    std::vector<SaveSlot>          m_slots;
    std::map<int, SaveData>        m_saveData;
    std::map<int, JsonValue>       m_serialized;
    bool  m_autoSaveEnabled        = false;
    float m_autoSaveTimer          = 0.f;
    float m_autoSaveIntervalSeconds = 300.f;  // 5 minutes default
};

// ── Game Phase G13 — World Events System ─────────────────────────

enum class WorldEventType : uint8_t {
    AsteroidStorm,
    PirateRaid,
    TechDiscovery,
    FactionWar,
    TradeOpportunity,
    Plague,
    CelestialAnomaly
};

inline std::string worldEventTypeName(WorldEventType t) {
    switch (t) {
        case WorldEventType::AsteroidStorm:    return "AsteroidStorm";
        case WorldEventType::PirateRaid:       return "PirateRaid";
        case WorldEventType::TechDiscovery:    return "TechDiscovery";
        case WorldEventType::FactionWar:       return "FactionWar";
        case WorldEventType::TradeOpportunity: return "TradeOpportunity";
        case WorldEventType::Plague:           return "Plague";
        case WorldEventType::CelestialAnomaly: return "CelestialAnomaly";
        default: return "Unknown";
    }
}

struct EventEffect {
    float priceModifier      = 1.f;   // multiplier on buy/sell prices
    float dangerModifier     = 1.f;   // multiplier on encounter danger
    float reputationChange   = 0.f;   // flat reputation change on resolution
    float resourceBonus      = 0.f;   // extra resource yield while active
};

struct WorldEvent {
    int            eventId        = 0;
    WorldEventType type           = WorldEventType::AsteroidStorm;
    std::string    sectorId;
    std::string    description;
    float          duration       = 60.f;  // seconds until expiry
    float          elapsed        = 0.f;
    float          severity       = 1.f;   // 0-1
    bool           isActive       = true;
    EventEffect    effect;

    bool isExpired() const { return elapsed >= duration; }
    float remainingTime() const { return std::max(0.f, duration - elapsed); }
    void tick(float dt) { if (isActive) elapsed += dt; }
};

class WorldEventSystem {
public:
    void init() {
        m_nextEventId = 1;
        NF_LOG_INFO("WorldEvents", "WorldEventSystem initialized");
    }

    // Spawn a new event in the specified sector. Returns the assigned event ID.
    int spawnEvent(WorldEventType type, const std::string& sectorId,
                   float duration, float severity, const std::string& description) {
        WorldEvent ev;
        ev.eventId     = m_nextEventId++;
        ev.type        = type;
        ev.sectorId    = sectorId;
        ev.duration    = duration;
        ev.severity    = std::max(0.f, std::min(1.f, severity));
        ev.description = description;
        ev.isActive    = true;
        ev.effect      = buildEffect(type, ev.severity);
        m_events.push_back(ev);
        NF_LOG_INFO("WorldEvents", "Spawned " + worldEventTypeName(type) +
                    " in '" + sectorId + "' id=" + std::to_string(ev.eventId));
        return ev.eventId;
    }

    // Manually end an event before it expires.
    bool endEvent(int eventId) {
        for (auto& ev : m_events) {
            if (ev.eventId == eventId && ev.isActive) {
                ev.isActive = false;
                return true;
            }
        }
        return false;
    }

    // Advance all active events; expired events are marked inactive.
    void tick(float dt) {
        for (auto& ev : m_events) {
            if (!ev.isActive) continue;
            ev.tick(dt);
            if (ev.isExpired()) ev.isActive = false;
        }
        // Prune fully-expired events beyond a history window
        while (m_events.size() > kMaxHistorySize) {
            m_events.erase(m_events.begin());
        }
    }

    [[nodiscard]] std::vector<WorldEvent> getActiveEvents() const {
        std::vector<WorldEvent> out;
        for (const auto& ev : m_events)
            if (ev.isActive) out.push_back(ev);
        return out;
    }

    [[nodiscard]] std::vector<WorldEvent> getEventsInSector(const std::string& sectorId) const {
        std::vector<WorldEvent> out;
        for (const auto& ev : m_events)
            if (ev.sectorId == sectorId) out.push_back(ev);
        return out;
    }

    [[nodiscard]] const WorldEvent* findEvent(int eventId) const {
        for (const auto& ev : m_events)
            if (ev.eventId == eventId) return &ev;
        return nullptr;
    }

    int activeEventCount() const {
        int n = 0;
        for (const auto& ev : m_events) if (ev.isActive) ++n;
        return n;
    }

    int totalEventCount() const { return (int)m_events.size(); }

    static constexpr size_t kMaxHistorySize = 100;

private:
    static EventEffect buildEffect(WorldEventType type, float severity) {
        EventEffect e;
        switch (type) {
            case WorldEventType::AsteroidStorm:
                e.dangerModifier = 1.f + severity;
                break;
            case WorldEventType::PirateRaid:
                e.dangerModifier   = 1.f + severity * 2.f;
                e.reputationChange = -5.f * severity;
                break;
            case WorldEventType::TechDiscovery:
                e.reputationChange = 10.f * severity;
                e.resourceBonus    = 0.5f * severity;
                break;
            case WorldEventType::FactionWar:
                e.dangerModifier   = 1.5f + severity;
                e.priceModifier    = 1.f + 0.3f * severity;
                break;
            case WorldEventType::TradeOpportunity:
                e.priceModifier  = 1.f - 0.3f * severity;  // discounted prices
                e.resourceBonus  = 1.f * severity;
                break;
            case WorldEventType::Plague:
                e.dangerModifier   = 1.f + 0.5f * severity;
                e.reputationChange = -2.f * severity;
                break;
            case WorldEventType::CelestialAnomaly:
                e.resourceBonus = 2.f * severity;
                break;
            default:
                break;
        }
        return e;
    }

    std::vector<WorldEvent> m_events;
    int m_nextEventId = 1;
};

// ── Game Phase G14 — Tech Tree ────────────────────────────────────

enum class TechCategory : uint8_t {
    Weapons,
    Shields,
    Propulsion,
    Mining,
    Construction,
    Biology,
    Computing
};

inline std::string techCategoryName(TechCategory c) {
    switch (c) {
        case TechCategory::Weapons:      return "Weapons";
        case TechCategory::Shields:      return "Shields";
        case TechCategory::Propulsion:   return "Propulsion";
        case TechCategory::Mining:       return "Mining";
        case TechCategory::Construction: return "Construction";
        case TechCategory::Biology:      return "Biology";
        case TechCategory::Computing:    return "Computing";
        default: return "Unknown";
    }
}

struct TechNode {
    std::string          id;
    std::string          displayName;
    TechCategory         category   = TechCategory::Weapons;
    int                  tier       = 1;           // 1 = root, higher = deeper
    int                  cost       = 100;         // research points required
    std::vector<std::string> prerequisites;        // ids of required nodes
    bool                 researched = false;

    // Bonus values unlocked by this node (domain-specific)
    float damageBonus    = 0.f;
    float shieldBonus    = 0.f;
    float speedBonus     = 0.f;
    float miningBonus    = 0.f;
};

class TechTree {
public:
    void addNode(const TechNode& node) {
        m_nodes[node.id] = node;
    }

    // Returns true if the node exists and all its prerequisites are researched.
    [[nodiscard]] bool canResearch(const std::string& id) const {
        auto it = m_nodes.find(id);
        if (it == m_nodes.end()) return false;
        if (it->second.researched) return false;
        for (const auto& prereq : it->second.prerequisites) {
            auto pit = m_nodes.find(prereq);
            if (pit == m_nodes.end() || !pit->second.researched) return false;
        }
        return true;
    }

    // Unlock the given node. Returns false if prerequisites unmet or already researched.
    bool unlock(const std::string& id) {
        if (!canResearch(id)) return false;
        m_nodes[id].researched = true;
        m_researchedCount++;
        NF_LOG_INFO("TechTree", "Unlocked: " + id);
        return true;
    }

    [[nodiscard]] bool isUnlocked(const std::string& id) const {
        auto it = m_nodes.find(id);
        return it != m_nodes.end() && it->second.researched;
    }

    [[nodiscard]] const TechNode* findNode(const std::string& id) const {
        auto it = m_nodes.find(id);
        return it != m_nodes.end() ? &it->second : nullptr;
    }

    // Returns all nodes that can be researched right now.
    [[nodiscard]] std::vector<std::string> getAvailable() const {
        std::vector<std::string> out;
        for (const auto& [id, node] : m_nodes)
            if (canResearch(id)) out.push_back(id);
        return out;
    }

    // Returns all researched node ids.
    [[nodiscard]] std::vector<std::string> getResearched() const {
        std::vector<std::string> out;
        for (const auto& [id, node] : m_nodes)
            if (node.researched) out.push_back(id);
        return out;
    }

    // Returns nodes of a specific tier.
    [[nodiscard]] std::vector<std::string> getByTier(int tier) const {
        std::vector<std::string> out;
        for (const auto& [id, node] : m_nodes)
            if (node.tier == tier) out.push_back(id);
        return out;
    }

    // Compute aggregate bonuses from all researched nodes.
    struct AggregateBonus {
        float damage  = 0.f;
        float shield  = 0.f;
        float speed   = 0.f;
        float mining  = 0.f;
    };

    [[nodiscard]] AggregateBonus computeBonuses() const {
        AggregateBonus b;
        for (const auto& [id, node] : m_nodes) {
            if (!node.researched) continue;
            b.damage  += node.damageBonus;
            b.shield  += node.shieldBonus;
            b.speed   += node.speedBonus;
            b.mining  += node.miningBonus;
        }
        return b;
    }

    int nodeCount()       const { return (int)m_nodes.size(); }
    int researchedCount() const { return m_researchedCount; }

private:
    std::map<std::string, TechNode> m_nodes;
    int m_researchedCount = 0;
};

// ── Game Phase G15 — Player Progression ──────────────────────────

enum class XPSource : uint8_t {
    Combat,
    Mining,
    Exploration,
    Trade,
    Quest,
    Crafting
};

inline std::string xpSourceName(XPSource s) {
    switch (s) {
        case XPSource::Combat:      return "Combat";
        case XPSource::Mining:      return "Mining";
        case XPSource::Exploration: return "Exploration";
        case XPSource::Trade:       return "Trade";
        case XPSource::Quest:       return "Quest";
        case XPSource::Crafting:    return "Crafting";
        default: return "Unknown";
    }
}

class PlayerLevel {
public:
    static constexpr int kMaxLevel = 50;

    void init(int startLevel = 1) {
        m_level = std::max(1, std::min(startLevel, kMaxLevel));
        m_totalXP = xpForLevel(m_level);
        m_xpThisLevel = 0;
        NF_LOG_INFO("PlayerLevel", "Initialized at level " + std::to_string(m_level));
    }

    // Add XP; returns number of levels gained (0 if none).
    int addXP(int amount, XPSource /*source*/ = XPSource::Combat) {
        if (m_level >= kMaxLevel) return 0;
        m_totalXP     += amount;
        m_xpThisLevel += amount;
        int levelsGained = 0;
        while (m_level < kMaxLevel && m_xpThisLevel >= xpToNextLevel()) {
            m_xpThisLevel -= xpToNextLevel();
            ++m_level;
            ++levelsGained;
            NF_LOG_INFO("PlayerLevel", "Level up! Now level " + std::to_string(m_level));
        }
        return levelsGained;
    }

    [[nodiscard]] int  currentLevel()  const { return m_level; }
    [[nodiscard]] int  xpThisLevel()   const { return m_xpThisLevel; }
    [[nodiscard]] int  totalXP()       const { return m_totalXP; }
    [[nodiscard]] bool isMaxLevel()    const { return m_level >= kMaxLevel; }

    // XP needed to advance from current level to next.
    [[nodiscard]] int xpToNextLevel() const {
        if (m_level >= kMaxLevel) return 0;
        return xpForLevel(m_level + 1) - xpForLevel(m_level);
    }

    [[nodiscard]] float progressToNextLevel() const {
        int needed = xpToNextLevel();
        return needed > 0 ? static_cast<float>(m_xpThisLevel) / static_cast<float>(needed) : 1.f;
    }

private:
    // Cumulative XP required to *reach* `level` from level 1.
    static int xpForLevel(int level) {
        // Quadratic: level * (level - 1) * 50
        return level * (level - 1) * 50;
    }

    int m_level       = 1;
    int m_totalXP     = 0;
    int m_xpThisLevel = 0;
};

struct SkillNode {
    std::string id;
    std::string displayName;
    int         requiredLevel = 1;   // minimum player level to unlock
    int         pointCost     = 1;   // skill points required
    bool        unlocked      = false;

    // Passive bonuses granted when unlocked
    float healthBonus   = 0.f;
    float energyBonus   = 0.f;
    float damageBonus   = 0.f;
    float miningBonus   = 0.f;
};

class SkillTree {
public:
    void addSkill(const SkillNode& node) { m_skills[node.id] = node; }

    bool unlockSkill(const std::string& id, int playerLevel, int& availablePoints) {
        auto it = m_skills.find(id);
        if (it == m_skills.end()) return false;
        auto& sk = it->second;
        if (sk.unlocked) return false;
        if (playerLevel < sk.requiredLevel) return false;
        if (availablePoints < sk.pointCost) return false;
        availablePoints -= sk.pointCost;
        sk.unlocked = true;
        NF_LOG_INFO("SkillTree", "Skill unlocked: " + id);
        return true;
    }

    [[nodiscard]] bool isUnlocked(const std::string& id) const {
        auto it = m_skills.find(id);
        return it != m_skills.end() && it->second.unlocked;
    }

    [[nodiscard]] const SkillNode* findSkill(const std::string& id) const {
        auto it = m_skills.find(id);
        return it != m_skills.end() ? &it->second : nullptr;
    }

    // Returns ids of skills available to unlock at the given player level.
    [[nodiscard]] std::vector<std::string> getAvailable(int playerLevel) const {
        std::vector<std::string> out;
        for (const auto& [id, sk] : m_skills)
            if (!sk.unlocked && playerLevel >= sk.requiredLevel)
                out.push_back(id);
        return out;
    }

    struct SkillBonuses {
        float health = 0.f;
        float energy = 0.f;
        float damage = 0.f;
        float mining = 0.f;
    };

    [[nodiscard]] SkillBonuses computeBonuses() const {
        SkillBonuses b;
        for (const auto& [id, sk] : m_skills) {
            if (!sk.unlocked) continue;
            b.health += sk.healthBonus;
            b.energy += sk.energyBonus;
            b.damage += sk.damageBonus;
            b.mining += sk.miningBonus;
        }
        return b;
    }

    int skillCount()    const { return (int)m_skills.size(); }
    int unlockedCount() const {
        int n = 0;
        for (const auto& [id, sk] : m_skills) if (sk.unlocked) ++n;
        return n;
    }

private:
    std::map<std::string, SkillNode> m_skills;
};

class ProgressionSystem {
public:
    void init(int startLevel = 1) {
        m_level.init(startLevel);
        m_skillPoints = 0;
        NF_LOG_INFO("Progression", "ProgressionSystem initialized");
    }

    // Award XP, potentially granting skill points on level-up.
    void awardXP(int amount, XPSource source = XPSource::Combat) {
        int gained = m_level.addXP(amount, source);
        m_skillPoints += gained;  // 1 skill point per level gained
    }

    bool spendSkillPoint(const std::string& skillId) {
        return m_skillTree.unlockSkill(skillId, m_level.currentLevel(), m_skillPoints);
    }

    [[nodiscard]] PlayerLevel&       level()      { return m_level; }
    [[nodiscard]] const PlayerLevel& level() const { return m_level; }
    [[nodiscard]] SkillTree&         skillTree()  { return m_skillTree; }
    [[nodiscard]] const SkillTree&   skillTree()  const { return m_skillTree; }
    [[nodiscard]] int                skillPoints() const { return m_skillPoints; }

    // Combined stat modifiers from skills.
    [[nodiscard]] SkillTree::SkillBonuses bonuses() const {
        return m_skillTree.computeBonuses();
    }

private:
    PlayerLevel m_level;
    SkillTree   m_skillTree;
    int         m_skillPoints = 0;
};

// ── Game Phase G16 — Crafting System ──────────────────────────────

enum class CraftingCategory : uint8_t {
    Weapon,
    Armor,
    Tool,
    Component,
    Consumable,
    Fuel,
    Decoration
};

inline std::string craftingCategoryName(CraftingCategory c) {
    switch (c) {
        case CraftingCategory::Weapon:     return "Weapon";
        case CraftingCategory::Armor:      return "Armor";
        case CraftingCategory::Tool:       return "Tool";
        case CraftingCategory::Component:  return "Component";
        case CraftingCategory::Consumable: return "Consumable";
        case CraftingCategory::Fuel:       return "Fuel";
        case CraftingCategory::Decoration: return "Decoration";
        default: return "Unknown";
    }
}

struct CraftingIngredient {
    std::string itemId;
    int         quantity = 1;
};

struct CraftingRecipe {
    std::string                    recipeId;
    std::string                    outputItemId;
    int                            outputQuantity = 1;
    CraftingCategory               category       = CraftingCategory::Component;
    std::vector<CraftingIngredient> ingredients;
    float                          craftTime       = 5.f;   // seconds
    int                            requiredLevel   = 1;
};

struct CraftingJob {
    std::string recipeId;
    float       elapsed  = 0.f;
    float       duration = 5.f;
    bool        complete = false;

    void tick(float dt) {
        if (complete) return;
        elapsed += dt;
        if (elapsed >= duration) {
            elapsed = duration;
            complete = true;
        }
    }

    [[nodiscard]] float progress() const {
        return duration > 0.f ? std::min(elapsed / duration, 1.f) : 1.f;
    }
};

class CraftingQueue {
public:
    void enqueue(const std::string& recipeId, float duration) {
        CraftingJob job;
        job.recipeId = recipeId;
        job.duration = duration;
        m_jobs.push_back(job);
        NF_LOG_INFO("Crafting", "Enqueued: " + recipeId);
    }

    // Tick the frontmost incomplete job.
    void tick(float dt) {
        for (auto& job : m_jobs) {
            if (!job.complete) {
                job.tick(dt);
                break;   // only tick the head job
            }
        }
    }

    // Collect and remove all completed jobs, return their recipe ids.
    [[nodiscard]] std::vector<std::string> collectCompleted() {
        std::vector<std::string> out;
        auto it = m_jobs.begin();
        while (it != m_jobs.end()) {
            if (it->complete) {
                out.push_back(it->recipeId);
                it = m_jobs.erase(it);
            } else {
                break;   // queue is FIFO; stop at first incomplete
            }
        }
        return out;
    }

    [[nodiscard]] int  pendingCount() const { return static_cast<int>(m_jobs.size()); }
    [[nodiscard]] bool isEmpty()      const { return m_jobs.empty(); }

    [[nodiscard]] const CraftingJob* currentJob() const {
        for (const auto& j : m_jobs) {
            if (!j.complete) return &j;
        }
        return nullptr;
    }

private:
    std::vector<CraftingJob> m_jobs;
};

class CraftingSystem {
public:
    void registerRecipe(const CraftingRecipe& recipe) {
        m_recipes[recipe.recipeId] = recipe;
    }

    [[nodiscard]] const CraftingRecipe* findRecipe(const std::string& id) const {
        auto it = m_recipes.find(id);
        return it != m_recipes.end() ? &it->second : nullptr;
    }

    // Check if the player has enough resources (represented as a map<itemId, count>).
    [[nodiscard]] bool canCraft(const std::string& recipeId,
                                 const std::map<std::string, int>& inventory,
                                 int playerLevel = 1) const {
        auto it = m_recipes.find(recipeId);
        if (it == m_recipes.end()) return false;
        const auto& recipe = it->second;
        if (playerLevel < recipe.requiredLevel) return false;
        for (const auto& ing : recipe.ingredients) {
            auto iit = inventory.find(ing.itemId);
            if (iit == inventory.end() || iit->second < ing.quantity) return false;
        }
        return true;
    }

    // Deduct ingredients and enqueue the job. Returns false if cannot craft.
    bool enqueue(const std::string& recipeId,
                 std::map<std::string, int>& inventory,
                 int playerLevel = 1) {
        if (!canCraft(recipeId, inventory, playerLevel)) return false;
        const auto& recipe = m_recipes[recipeId];
        for (const auto& ing : recipe.ingredients) {
            inventory[ing.itemId] -= ing.quantity;
        }
        m_queue.enqueue(recipeId, recipe.craftTime);
        return true;
    }

    void tick(float dt) { m_queue.tick(dt); }

    [[nodiscard]] std::vector<std::string> collectCompleted() {
        return m_queue.collectCompleted();
    }

    [[nodiscard]] CraftingQueue&       queue()       { return m_queue; }
    [[nodiscard]] const CraftingQueue& queue() const { return m_queue; }
    [[nodiscard]] int recipeCount() const { return static_cast<int>(m_recipes.size()); }

    // Get all recipe ids for a given category.
    [[nodiscard]] std::vector<std::string> recipesByCategory(CraftingCategory cat) const {
        std::vector<std::string> out;
        for (const auto& [id, r] : m_recipes)
            if (r.category == cat) out.push_back(id);
        return out;
    }

private:
    std::map<std::string, CraftingRecipe> m_recipes;
    CraftingQueue m_queue;
};

// ── Game Phase G17 — Inventory & Equipment ───────────────────────

enum class ItemRarity : uint8_t {
    Common,
    Uncommon,
    Rare,
    Epic,
    Legendary
};

inline std::string itemRarityName(ItemRarity r) {
    switch (r) {
        case ItemRarity::Common:    return "Common";
        case ItemRarity::Uncommon:  return "Uncommon";
        case ItemRarity::Rare:      return "Rare";
        case ItemRarity::Epic:      return "Epic";
        case ItemRarity::Legendary: return "Legendary";
        default: return "Unknown";
    }
}

enum class ItemSlot : uint8_t {
    None,
    Head,
    Chest,
    Legs,
    Boots,
    Weapon,
    Shield,
    Accessory
};

inline std::string itemSlotName(ItemSlot s) {
    switch (s) {
        case ItemSlot::None:      return "None";
        case ItemSlot::Head:      return "Head";
        case ItemSlot::Chest:     return "Chest";
        case ItemSlot::Legs:      return "Legs";
        case ItemSlot::Boots:     return "Boots";
        case ItemSlot::Weapon:    return "Weapon";
        case ItemSlot::Shield:    return "Shield";
        case ItemSlot::Accessory: return "Accessory";
        default: return "Unknown";
    }
}

struct Item {
    std::string id;
    std::string displayName;
    ItemRarity  rarity    = ItemRarity::Common;
    ItemSlot    slot      = ItemSlot::None;
    int         stackMax  = 99;
    int         count     = 1;
    float       weight    = 1.f;

    // Stat bonuses (applicable when equipped)
    float damageBonus  = 0.f;
    float armorBonus   = 0.f;
    float speedBonus   = 0.f;
    float healthBonus  = 0.f;

    [[nodiscard]] bool isStackable() const { return stackMax > 1; }
    [[nodiscard]] bool canStack(int amount) const { return count + amount <= stackMax; }
};

class PlayerInventory {
public:
    explicit PlayerInventory(int capacity = 40) : m_capacity(capacity) {}

    // Add items. Returns the leftover count that didn't fit.
    int addItem(const Item& item) {
        int remaining = item.count;

        // Try stacking first
        if (item.isStackable()) {
            for (auto& existing : m_items) {
                if (existing.id == item.id && existing.canStack(remaining)) {
                    existing.count += remaining;
                    NF_LOG_INFO("Inventory", "Stacked " + std::to_string(remaining) + "x " + item.id);
                    return 0;
                } else if (existing.id == item.id) {
                    int space = existing.stackMax - existing.count;
                    existing.count = existing.stackMax;
                    remaining -= space;
                }
            }
        }

        // New slot(s)
        while (remaining > 0 && static_cast<int>(m_items.size()) < m_capacity) {
            Item newSlot = item;
            newSlot.count = std::min(remaining, item.stackMax);
            remaining -= newSlot.count;
            m_items.push_back(newSlot);
        }

        return remaining;   // 0 = all added
    }

    // Remove `count` of the given item. Returns amount actually removed.
    int removeItem(const std::string& id, int count) {
        int removed = 0;
        for (auto it = m_items.begin(); it != m_items.end() && removed < count; ) {
            if (it->id == id) {
                int take = std::min(it->count, count - removed);
                it->count -= take;
                removed += take;
                if (it->count <= 0) {
                    it = m_items.erase(it);
                    continue;
                }
            }
            ++it;
        }
        return removed;
    }

    [[nodiscard]] int countItem(const std::string& id) const {
        int total = 0;
        for (const auto& item : m_items)
            if (item.id == id) total += item.count;
        return total;
    }

    [[nodiscard]] const Item* findItem(const std::string& id) const {
        for (const auto& item : m_items)
            if (item.id == id) return &item;
        return nullptr;
    }

    [[nodiscard]] int usedSlots()  const { return static_cast<int>(m_items.size()); }
    [[nodiscard]] int freeSlots()  const { return m_capacity - usedSlots(); }
    [[nodiscard]] int capacity()   const { return m_capacity; }
    [[nodiscard]] bool isFull()    const { return usedSlots() >= m_capacity; }
    [[nodiscard]] const std::vector<Item>& items() const { return m_items; }

    // Convert to a simple map<itemId, count> (useful for crafting checks).
    [[nodiscard]] std::map<std::string, int> toCountMap() const {
        std::map<std::string, int> m;
        for (const auto& item : m_items) m[item.id] += item.count;
        return m;
    }

private:
    std::vector<Item> m_items;
    int m_capacity;
};

class EquipmentLoadout {
public:
    // Equip item in its designated slot. Returns the previously-equipped item id (empty if none).
    std::string equip(const Item& item) {
        if (item.slot == ItemSlot::None) return "";
        std::string prev;
        auto it = m_equipped.find(item.slot);
        if (it != m_equipped.end()) prev = it->second.id;
        m_equipped[item.slot] = item;
        NF_LOG_INFO("Equipment", "Equipped " + item.id + " in " + itemSlotName(item.slot));
        return prev;
    }

    // Unequip and return the item. Returns nullopt if slot is empty.
    std::string unequip(ItemSlot slot) {
        auto it = m_equipped.find(slot);
        if (it == m_equipped.end()) return "";
        std::string id = it->second.id;
        m_equipped.erase(it);
        NF_LOG_INFO("Equipment", "Unequipped " + id + " from " + itemSlotName(slot));
        return id;
    }

    [[nodiscard]] bool isSlotOccupied(ItemSlot slot) const {
        return m_equipped.find(slot) != m_equipped.end();
    }

    [[nodiscard]] const Item* getEquipped(ItemSlot slot) const {
        auto it = m_equipped.find(slot);
        return it != m_equipped.end() ? &it->second : nullptr;
    }

    struct EquipmentBonuses {
        float damage = 0.f;
        float armor  = 0.f;
        float speed  = 0.f;
        float health = 0.f;
    };

    [[nodiscard]] EquipmentBonuses computeBonuses() const {
        EquipmentBonuses b;
        for (const auto& [slot, item] : m_equipped) {
            b.damage += item.damageBonus;
            b.armor  += item.armorBonus;
            b.speed  += item.speedBonus;
            b.health += item.healthBonus;
        }
        return b;
    }

    [[nodiscard]] int equippedCount() const { return static_cast<int>(m_equipped.size()); }

private:
    std::map<ItemSlot, Item> m_equipped;
};

// ── G18 Status Effects ──────────────────────────────────────────

enum class StatusEffectType : uint8_t {
    Poison = 0,    // DoT, reduces health over time
    Burn,          // DoT, fire damage per tick
    Freeze,        // Slows movement and actions
    Radiation,     // Accumulates, causes long-term damage
    Bleed,         // DoT, bypasses shields
    Stun,          // Temporarily disables actions
    Blind,         // Reduces vision/accuracy
    Overcharge,    // Boosts output but risks burnout
    Count
};

inline const char* statusEffectTypeName(StatusEffectType t) {
    switch (t) {
        case StatusEffectType::Poison:     return "Poison";
        case StatusEffectType::Burn:       return "Burn";
        case StatusEffectType::Freeze:     return "Freeze";
        case StatusEffectType::Radiation:  return "Radiation";
        case StatusEffectType::Bleed:      return "Bleed";
        case StatusEffectType::Stun:       return "Stun";
        case StatusEffectType::Blind:      return "Blind";
        case StatusEffectType::Overcharge: return "Overcharge";
        default: return "Unknown";
    }
}

struct StatusEffect {
    StatusEffectType type      = StatusEffectType::Poison;
    float            damage    = 0.f;    // damage per tick (0 if non-damaging)
    float            duration  = 0.f;   // total duration in seconds
    float            elapsed   = 0.f;   // time elapsed
    float            tickRate  = 1.f;   // seconds between damage ticks
    float            tickTimer = 0.f;
    float            intensity = 1.f;   // 0..1 multiplier on all effects
    bool             active    = true;

    [[nodiscard]] bool isExpired() const { return elapsed >= duration; }
    [[nodiscard]] float remaining() const {
        return std::max(0.f, duration - elapsed);
    }

    // Returns damage dealt this tick (0 if not yet time)
    float tick(float dt) {
        if (!active || isExpired()) return 0.f;
        elapsed   += dt;
        tickTimer += dt;
        if (tickTimer >= tickRate) {
            tickTimer -= tickRate;
            return damage * intensity;
        }
        return 0.f;
    }
};

struct AilmentStack {
    std::vector<StatusEffect> effects;

    void apply(const StatusEffect& e) {
        // Refresh if same type already present, otherwise stack
        for (auto& ex : effects) {
            if (ex.type == e.type && ex.active) {
                ex.elapsed   = 0.f;
                ex.duration  = std::max(ex.duration, e.duration);
                ex.intensity = std::max(ex.intensity, e.intensity);
                return;
            }
        }
        effects.push_back(e);
    }

    void remove(StatusEffectType t) {
        for (auto& e : effects)
            if (e.type == t) e.active = false;
    }

    [[nodiscard]] bool has(StatusEffectType t) const {
        for (auto& e : effects)
            if (e.type == t && e.active && !e.isExpired()) return true;
        return false;
    }

    // Returns total damage dealt this tick from all effects
    float tick(float dt) {
        float total = 0.f;
        for (auto& e : effects) total += e.tick(dt);
        // Remove expired
        effects.erase(
            std::remove_if(effects.begin(), effects.end(),
                [](const StatusEffect& e){ return !e.active || e.isExpired(); }),
            effects.end());
        return total;
    }

    [[nodiscard]] size_t count() const { return effects.size(); }
    void clear() { effects.clear(); }
};

class StatusEffectSystem {
public:
    void applyEffect(int entityId, const StatusEffect& effect) {
        m_stacks[entityId].apply(effect);
    }

    void removeEffect(int entityId, StatusEffectType type) {
        auto it = m_stacks.find(entityId);
        if (it != m_stacks.end()) it->second.remove(type);
    }

    [[nodiscard]] bool hasEffect(int entityId, StatusEffectType type) const {
        auto it = m_stacks.find(entityId);
        return it != m_stacks.end() && it->second.has(type);
    }

    // Tick all stacks; returns map of entity→damageDealt
    std::unordered_map<int,float> tick(float dt) {
        std::unordered_map<int,float> dmg;
        for (auto& [id, stack] : m_stacks) {
            float d = stack.tick(dt);
            if (d > 0.f) dmg[id] = d;
        }
        // Clean up empty stacks
        for (auto it = m_stacks.begin(); it != m_stacks.end(); ) {
            if (it->second.count() == 0) it = m_stacks.erase(it);
            else ++it;
        }
        return dmg;
    }

    [[nodiscard]] const AilmentStack* getStack(int entityId) const {
        auto it = m_stacks.find(entityId);
        return (it != m_stacks.end()) ? &it->second : nullptr;
    }

    void clearEntity(int entityId) { m_stacks.erase(entityId); }
    void clearAll() { m_stacks.clear(); }
    [[nodiscard]] size_t entityCount() const { return m_stacks.size(); }

private:
    std::unordered_map<int, AilmentStack> m_stacks;
};

// ── G19 Contracts & Bounties ─────────────────────────────────────

enum class ContractType : uint8_t {
    Delivery = 0,   // Transport cargo from A to B
    Assassination,  // Eliminate a target
    Escort,         // Protect a subject
    Salvage,        // Recover items from a wreck
    Patrol,         // Hold a sector for a duration
    Mining,         // Extract a quantity of a resource
    Count
};

inline const char* contractTypeName(ContractType t) {
    switch(t){
        case ContractType::Delivery:      return "Delivery";
        case ContractType::Assassination: return "Assassination";
        case ContractType::Escort:        return "Escort";
        case ContractType::Salvage:       return "Salvage";
        case ContractType::Patrol:        return "Patrol";
        case ContractType::Mining:        return "Mining";
        default: return "Unknown";
    }
}

enum class ContractStatus : uint8_t {
    Available = 0,
    Accepted,
    InProgress,
    Completed,
    Failed,
    Expired
};

struct Contract {
    std::string    contractId;
    std::string    title;
    std::string    description;
    ContractType   type          = ContractType::Delivery;
    ContractStatus status        = ContractStatus::Available;
    std::string    issuingFaction;
    int            creditsReward = 0;
    int            reputationReward = 0;
    float          timeLimit     = 0.f;   // 0 = no time limit
    float          elapsed       = 0.f;
    int            requiredLevel = 0;

    void accept()   { if (status==ContractStatus::Available) status=ContractStatus::Accepted; }
    void start()    { if (status==ContractStatus::Accepted)  status=ContractStatus::InProgress; }
    void complete() { if (status==ContractStatus::InProgress) status=ContractStatus::Completed; }
    void fail()     { if (status==ContractStatus::InProgress) status=ContractStatus::Failed; }

    [[nodiscard]] bool isActive() const {
        return status==ContractStatus::Accepted || status==ContractStatus::InProgress;
    }
    [[nodiscard]] bool isExpired() const {
        return timeLimit > 0.f && elapsed >= timeLimit;
    }

    void tick(float dt) {
        if (!isActive()) return;
        elapsed += dt;
        if (isExpired()) status = ContractStatus::Expired;
    }
};

struct BountyTarget {
    std::string targetId;
    std::string targetName;
    std::string factionId;
    int         creditsReward  = 0;
    int         reputationCost = 0;   // rep cost with target's faction
    bool        deadOrAlive    = true; // true = dead or alive, false = alive only
    bool        claimed        = false;

    void claim() { claimed = true; }
    [[nodiscard]] bool isClaimable() const { return !claimed; }
};

class ContractBoard {
public:
    void addContract(Contract c) {
        m_contracts.push_back(std::move(c));
    }

    void addBounty(BountyTarget b) {
        m_bounties.push_back(std::move(b));
    }

    [[nodiscard]] Contract* findContract(const std::string& id) {
        for (auto& c : m_contracts)
            if (c.contractId == id) return &c;
        return nullptr;
    }

    [[nodiscard]] BountyTarget* findBounty(const std::string& targetId) {
        for (auto& b : m_bounties)
            if (b.targetId == targetId) return &b;
        return nullptr;
    }

    [[nodiscard]] std::vector<const Contract*> availableContracts(int playerLevel) const {
        std::vector<const Contract*> out;
        for (auto& c : m_contracts)
            if (c.status==ContractStatus::Available && c.requiredLevel<=playerLevel)
                out.push_back(&c);
        return out;
    }

    [[nodiscard]] std::vector<const BountyTarget*> activeBounties() const {
        std::vector<const BountyTarget*> out;
        for (auto& b : m_bounties)
            if (!b.claimed) out.push_back(&b);
        return out;
    }

    void tick(float dt) {
        for (auto& c : m_contracts) c.tick(dt);
    }

    [[nodiscard]] size_t contractCount() const { return m_contracts.size(); }
    [[nodiscard]] size_t bountyCount()   const { return m_bounties.size(); }

    void removeExpired() {
        m_contracts.erase(
            std::remove_if(m_contracts.begin(), m_contracts.end(),
                [](const Contract& c){ return c.status==ContractStatus::Expired; }),
            m_contracts.end());
    }

private:
    std::vector<Contract>     m_contracts;
    std::vector<BountyTarget> m_bounties;
};

// ── G20 Companion System ─────────────────────────────────────────

enum class CompanionRole : uint8_t {
    Combat = 0,    // Front-line fighter
    Engineer,      // Repairs, hacks, constructs
    Medic,         // Heals, revives, buffs
    Scout,         // Reconnaissance, early warning
    Pilot,         // Improved ship handling
    Trader,        // Better prices, extra inventory
    Count
};

inline const char* companionRoleName(CompanionRole r) {
    switch(r){
        case CompanionRole::Combat:    return "Combat";
        case CompanionRole::Engineer:  return "Engineer";
        case CompanionRole::Medic:     return "Medic";
        case CompanionRole::Scout:     return "Scout";
        case CompanionRole::Pilot:     return "Pilot";
        case CompanionRole::Trader:    return "Trader";
        default: return "Unknown";
    }
}

struct CompanionPersonality {
    float loyalty      = 0.5f;  // 0..1, affects willingness to follow orders
    float bravery      = 0.5f;  // 0..1, affects combat engagement threshold
    float curiosity    = 0.5f;  // 0..1, affects exploration behavior
    float morale       = 1.0f;  // 0..1, affected by events
    int   trust        = 0;     // accumulated trust points
    int   trustToLoyalThreshold = 50;

    void gainTrust(int amount) { trust += amount; }
    void loseTrust(int amount) { trust -= amount; }
    [[nodiscard]] bool isLoyal() const { return trust >= trustToLoyalThreshold; }

    void adjustMorale(float delta) {
        morale = std::clamp(morale + delta, 0.f, 1.f);
    }
};

struct CompanionAbility {
    std::string name;
    std::string description;
    float       cooldown        = 30.f;  // seconds
    float       cooldownElapsed = 0.f;
    bool        passive         = false; // true = always active, no cooldown

    [[nodiscard]] bool isReady() const {
        return passive || cooldownElapsed >= cooldown;
    }

    void use() {
        if (!passive) cooldownElapsed = 0.f;
    }

    void tick(float dt) {
        if (!passive && cooldownElapsed < cooldown)
            cooldownElapsed += dt;
    }
};

class Companion {
public:
    void init(const std::string& name, CompanionRole role) {
        m_name = name;
        m_role = role;
        m_health = m_maxHealth = 100.f;
        m_active = true;
    }

    [[nodiscard]] const std::string&  name()        const { return m_name; }
    [[nodiscard]] CompanionRole       role()         const { return m_role; }
    [[nodiscard]] float               health()       const { return m_health; }
    [[nodiscard]] float               maxHealth()    const { return m_maxHealth; }
    [[nodiscard]] bool                isActive()     const { return m_active; }
    [[nodiscard]] bool                isAlive()      const { return m_health > 0.f; }
    [[nodiscard]] CompanionPersonality& personality()      { return m_personality; }
    [[nodiscard]] const CompanionPersonality& personality() const { return m_personality; }

    void addAbility(const CompanionAbility& ab) { m_abilities.push_back(ab); }
    [[nodiscard]] size_t abilityCount() const { return m_abilities.size(); }
    [[nodiscard]] CompanionAbility* findAbility(const std::string& abName) {
        for (auto& ab : m_abilities) if (ab.name == abName) return &ab;
        return nullptr;
    }

    void takeDamage(float dmg) {
        m_health = std::max(0.f, m_health - dmg);
        if (m_health <= 0.f) m_active = false;
        m_personality.adjustMorale(-0.05f);
    }

    void heal(float amount) {
        m_health = std::min(m_maxHealth, m_health + amount);
        if (m_health > 0.f) m_active = true;
    }

    void dismiss()  { m_active = false; }
    void recall()   { if (isAlive()) m_active = true; }

    void tick(float dt) {
        for (auto& ab : m_abilities) ab.tick(dt);
    }

private:
    std::string           m_name;
    CompanionRole         m_role    = CompanionRole::Combat;
    float                 m_health  = 100.f;
    float                 m_maxHealth = 100.f;
    bool                  m_active  = true;
    CompanionPersonality  m_personality;
    std::vector<CompanionAbility> m_abilities;
};

class CompanionManager {
public:
    static constexpr int kMaxCompanions = 4;

    void addCompanion(Companion c) {
        if (static_cast<int>(m_companions.size()) < kMaxCompanions)
            m_companions.push_back(std::move(c));
    }

    void removeCompanion(const std::string& name) {
        m_companions.erase(
            std::remove_if(m_companions.begin(), m_companions.end(),
                [&](const Companion& c){ return c.name()==name; }),
            m_companions.end());
    }

    [[nodiscard]] Companion* findCompanion(const std::string& name) {
        for (auto& c : m_companions) if (c.name()==name) return &c;
        return nullptr;
    }

    [[nodiscard]] size_t companionCount()  const { return m_companions.size(); }
    [[nodiscard]] size_t activeCount()     const {
        size_t n=0; for(auto& c: m_companions) if(c.isActive()) ++n; return n;
    }

    void tick(float dt) { for (auto& c : m_companions) c.tick(dt); }

    void healAll(float amount) { for (auto& c : m_companions) c.heal(amount); }

    [[nodiscard]] bool hasRole(CompanionRole role) const {
        for (auto& c : m_companions)
            if (c.role()==role && c.isActive()) return true;
        return false;
    }

    [[nodiscard]] float averageMorale() const {
        if (m_companions.empty()) return 0.f;
        float sum=0.f;
        for (auto& c : m_companions) sum += c.personality().morale;
        return sum / static_cast<float>(m_companions.size());
    }

private:
    std::vector<Companion> m_companions;
};

// ── G21 Faction System ────────────────────────────────────────

enum class FactionType : uint8_t {
    Military = 0,
    Corporate,
    Scientific,
    Religious,
    Criminal,
    Pirate,
    Colonial,
    Independent,
    Count
};

inline const char* factionTypeName(FactionType t) {
    switch(t){
        case FactionType::Military:    return "Military";
        case FactionType::Corporate:   return "Corporate";
        case FactionType::Scientific:  return "Scientific";
        case FactionType::Religious:   return "Religious";
        case FactionType::Criminal:    return "Criminal";
        case FactionType::Pirate:      return "Pirate";
        case FactionType::Colonial:    return "Colonial";
        case FactionType::Independent: return "Independent";
        default: return "Unknown";
    }
}

enum class FactionStanding : uint8_t {
    Hostile = 0,
    Unfriendly,
    Neutral,
    Friendly,
    Allied
};

inline const char* factionStandingName(FactionStanding s) {
    switch(s){
        case FactionStanding::Hostile:    return "Hostile";
        case FactionStanding::Unfriendly: return "Unfriendly";
        case FactionStanding::Neutral:    return "Neutral";
        case FactionStanding::Friendly:   return "Friendly";
        case FactionStanding::Allied:     return "Allied";
        default: return "Unknown";
    }
}

struct FactionTerritory {
    std::string sectorId;
    std::string sectorName;
    float       controlStrength = 1.0f;
    int         resourceOutput  = 0;
    bool        contested       = false;

    void erode(float amount)    { controlStrength = std::clamp(controlStrength - amount, 0.f, 1.f); }
    void reinforce(float amount){ controlStrength = std::clamp(controlStrength + amount, 0.f, 1.f); }
    [[nodiscard]] bool isLost() const { return controlStrength <= 0.f; }
};

class Faction {
public:
    void init(const std::string& id, const std::string& name, FactionType type) {
        m_factionId = id;
        m_name      = name;
        m_type      = type;
        m_influence  = 0.5f;
        m_wealth     = 0;
        m_militaryPower = 0;
        m_territories.clear();
        m_wealthAccum = 0.f;
    }

    [[nodiscard]] const std::string& factionId()     const { return m_factionId; }
    [[nodiscard]] const std::string& name()          const { return m_name; }
    [[nodiscard]] FactionType        type()          const { return m_type; }
    [[nodiscard]] float              influence()     const { return m_influence; }
    [[nodiscard]] int                wealth()        const { return m_wealth; }
    [[nodiscard]] int                militaryPower() const { return m_militaryPower; }

    void addTerritory(FactionTerritory t) { m_territories.push_back(std::move(t)); }

    void removeTerritory(const std::string& sectorId) {
        m_territories.erase(
            std::remove_if(m_territories.begin(), m_territories.end(),
                [&](const FactionTerritory& t){ return t.sectorId==sectorId; }),
            m_territories.end());
    }

    [[nodiscard]] FactionTerritory* findTerritory(const std::string& sectorId) {
        for (auto& t : m_territories) if (t.sectorId==sectorId) return &t;
        return nullptr;
    }

    [[nodiscard]] size_t territoryCount() const { return m_territories.size(); }

    [[nodiscard]] int totalResourceOutput() const {
        int sum = 0;
        for (auto& t : m_territories) sum += t.resourceOutput;
        return sum;
    }

    void adjustInfluence(float delta) { m_influence = std::clamp(m_influence + delta, 0.f, 1.f); }
    void addWealth(int amount)        { m_wealth += amount; }
    [[nodiscard]] bool spendWealth(int amount) {
        if (m_wealth < amount) return false;
        m_wealth -= amount;
        return true;
    }

    void setMilitaryPower(int v) { m_militaryPower = v; }

    void tick(float dt) {
        m_wealthAccum += dt;
        while (m_wealthAccum >= 1.f) {
            m_wealthAccum -= 1.f;
            m_wealth += totalResourceOutput();
        }
    }

private:
    std::string  m_factionId;
    std::string  m_name;
    FactionType  m_type         = FactionType::Independent;
    float        m_influence    = 0.5f;
    int          m_wealth       = 0;
    int          m_militaryPower = 0;
    float        m_wealthAccum  = 0.f;
    std::vector<FactionTerritory> m_territories;
};

struct GameFactionRelation {
    std::string     factionA;
    std::string     factionB;
    FactionStanding standing   = FactionStanding::Neutral;
    int             reputation = 0;
    bool            atWar      = false;
    bool            hasTreaty  = false;
    std::string     treatyType;

    void improve(int amount) {
        reputation += amount;
        updateStanding();
    }

    void degrade(int amount) {
        reputation -= amount;
        updateStanding();
    }

    void declareWar() {
        atWar    = true;
        standing = FactionStanding::Hostile;
        breakTreaty();
    }

    void declarePeace() {
        atWar    = false;
        standing = FactionStanding::Unfriendly;
    }

    void signTreaty(const std::string& type) {
        hasTreaty  = true;
        treatyType = type;
    }

    void breakTreaty() {
        hasTreaty  = false;
        treatyType.clear();
    }

private:
    void updateStanding() {
        if      (reputation < -75) standing = FactionStanding::Hostile;
        else if (reputation < -25) standing = FactionStanding::Unfriendly;
        else if (reputation <  25) standing = FactionStanding::Neutral;
        else if (reputation <  75) standing = FactionStanding::Friendly;
        else                       standing = FactionStanding::Allied;
    }
};

class GameFactionManager {
public:
    static constexpr int kMaxFactions = 16;

    void addFaction(Faction f) {
        if (static_cast<int>(m_factions.size()) < kMaxFactions)
            m_factions.push_back(std::move(f));
    }

    void removeFaction(const std::string& id) {
        m_factions.erase(
            std::remove_if(m_factions.begin(), m_factions.end(),
                [&](const Faction& f){ return f.factionId()==id; }),
            m_factions.end());
    }

    [[nodiscard]] Faction* findFaction(const std::string& id) {
        for (auto& f : m_factions) if (f.factionId()==id) return &f;
        return nullptr;
    }

    [[nodiscard]] size_t factionCount() const { return m_factions.size(); }

    void setRelation(const std::string& a, const std::string& b, GameFactionRelation rel) {
        auto key = makeKey(a, b);
        rel.factionA = key.first;
        rel.factionB = key.second;
        for (auto& r : m_relations)
            if (r.factionA==key.first && r.factionB==key.second) { r = std::move(rel); return; }
        m_relations.push_back(std::move(rel));
    }

    [[nodiscard]] GameFactionRelation* getRelation(const std::string& a, const std::string& b) {
        auto key = makeKey(a, b);
        for (auto& r : m_relations)
            if (r.factionA==key.first && r.factionB==key.second) return &r;
        return nullptr;
    }

    [[nodiscard]] std::vector<const Faction*> alliedFactions(const std::string& factionId) const {
        std::vector<const Faction*> out;
        for (auto& r : m_relations) {
            if (r.standing != FactionStanding::Allied) continue;
            std::string otherId;
            if      (r.factionA == factionId) otherId = r.factionB;
            else if (r.factionB == factionId) otherId = r.factionA;
            else continue;
            for (auto& f : m_factions) if (f.factionId()==otherId) { out.push_back(&f); break; }
        }
        return out;
    }

    [[nodiscard]] std::vector<const Faction*> hostileFactions(const std::string& factionId) const {
        std::vector<const Faction*> out;
        for (auto& r : m_relations) {
            if (r.standing != FactionStanding::Hostile) continue;
            std::string otherId;
            if      (r.factionA == factionId) otherId = r.factionB;
            else if (r.factionB == factionId) otherId = r.factionA;
            else continue;
            for (auto& f : m_factions) if (f.factionId()==otherId) { out.push_back(&f); break; }
        }
        return out;
    }

    void tick(float dt) { for (auto& f : m_factions) f.tick(dt); }

private:
    std::vector<Faction>              m_factions;
    std::vector<GameFactionRelation>  m_relations;

    [[nodiscard]] static std::pair<std::string,std::string> makeKey(const std::string& a, const std::string& b) {
        return a < b ? std::make_pair(a, b) : std::make_pair(b, a);
    }
};

// ── SP1 Voxel Material Table ──────────────────────────────────

struct VoxelMaterialDef {
    VoxelType   type        = VoxelType::Air;
    float       density     = 0.f;    // kg/m³ ÷ 1000
    float       hardness    = 0.f;    // 0-10 mining resistance
    bool        isLoose     = false;  // subject to collapse physics
    bool        canCollapse = false;  // falls when unsupported
    std::string yieldMaterial;        // raw material produced
    int         yieldQuantity = 0;    // units per voxel
    float       structuralStrength = 0.f; // load-bearing capacity
    float       blastResistance    = 0.f; // explosion resistance

    [[nodiscard]] bool isSolid()     const { return type != VoxelType::Air && type != VoxelType::Water; }
    [[nodiscard]] bool isMineable()  const { return isSolid() && hardness > 0.f; }
    [[nodiscard]] bool yieldsItem()  const { return !yieldMaterial.empty() && yieldQuantity > 0; }
};

class VoxelMaterialTable {
public:
    void registerMaterial(const VoxelMaterialDef& def) {
        auto idx = static_cast<size_t>(def.type);
        if (idx < m_defs.size()) m_defs[idx] = def;
    }

    [[nodiscard]] const VoxelMaterialDef& get(VoxelType t) const {
        auto idx = static_cast<size_t>(t);
        if (idx < m_defs.size()) return m_defs[idx];
        return m_defs[0]; // Air fallback
    }

    void loadDefaults() {
        registerMaterial({VoxelType::Air, 0.f, 0.f, false, false, "", 0, 0.f, 0.f});
        registerMaterial({VoxelType::Stone, 2.5f, 8.f, false, false, "gravel", 2, 10.f, 6.f});
        registerMaterial({VoxelType::Dirt, 1.5f, 2.f, true, true, "soil", 2, 1.f, 0.5f});
        registerMaterial({VoxelType::Grass, 1.3f, 2.f, true, true, "soil", 1, 1.f, 0.5f});
        registerMaterial({VoxelType::Metal, 7.8f, 9.f, false, false, "scrap_metal", 1, 15.f, 9.f});
        registerMaterial({VoxelType::Glass, 2.2f, 3.f, false, false, "silica", 1, 2.f, 1.f});
        registerMaterial({VoxelType::Water, 1.0f, 0.f, true, false, "", 0, 0.f, 0.f});
        registerMaterial({VoxelType::Ore_Iron, 5.0f, 7.f, false, false, "raw_iron", 1, 8.f, 5.f});
        registerMaterial({VoxelType::Ore_Gold, 8.0f, 6.f, false, false, "raw_gold", 1, 7.f, 4.f});
        registerMaterial({VoxelType::Ore_Crystal, 3.0f, 5.f, false, false, "raw_crystal", 1, 5.f, 3.f});
    }

    [[nodiscard]] size_t materialCount() const {
        return static_cast<size_t>(VoxelType::Count);
    }

private:
    std::array<VoxelMaterialDef, static_cast<size_t>(VoxelType::Count)> m_defs{};
};

// ── SP2 Centrifuge System ─────────────────────────────────────

enum class CentrifugeState : uint8_t {
    Idle = 0, Loading, Processing, Complete, PowerStall
};

inline const char* centrifugeStateName(CentrifugeState s) {
    switch (s) {
    case CentrifugeState::Idle:       return "Idle";
    case CentrifugeState::Loading:    return "Loading";
    case CentrifugeState::Processing: return "Processing";
    case CentrifugeState::Complete:   return "Complete";
    case CentrifugeState::PowerStall: return "PowerStall";
    default: return "Unknown";
    }
}

struct CentrifugeJob {
    std::string inputMaterial;
    int         inputQuantity  = 0;
    std::string outputMaterial;
    int         outputQuantity = 0;
    float       processingTime = 10.f;
    float       elapsed        = 0.f;
    float       powerRequired  = 3.f;

    [[nodiscard]] bool isComplete() const { return elapsed >= processingTime; }
    [[nodiscard]] float progress()  const {
        return processingTime > 0.f ? std::clamp(elapsed / processingTime, 0.f, 1.f) : 1.f;
    }
};

class CentrifugeSystem {
public:
    static constexpr int kMaxQueueSize = 8;

    void setTier(int tier) {
        m_tier = std::clamp(tier, 1, 3);
        switch (m_tier) {
        case 1: m_speedMult = 1.0f; m_maxQueue = 4; break;
        case 2: m_speedMult = 1.5f; m_maxQueue = 6; break;
        case 3: m_speedMult = 2.5f; m_maxQueue = 8; break;
        }
    }

    [[nodiscard]] int tier() const { return m_tier; }

    bool addJob(CentrifugeJob job) {
        if (static_cast<int>(m_queue.size()) >= m_maxQueue) return false;
        m_queue.push_back(std::move(job));
        if (m_state == CentrifugeState::Idle) m_state = CentrifugeState::Loading;
        return true;
    }

    void tick(float dt, float availablePower) {
        if (m_queue.empty()) { m_state = CentrifugeState::Idle; return; }
        auto& current = m_queue.front();
        if (m_state == CentrifugeState::Loading) m_state = CentrifugeState::Processing;
        if (m_state == CentrifugeState::Processing || m_state == CentrifugeState::PowerStall) {
            if (availablePower < current.powerRequired) {
                m_state = CentrifugeState::PowerStall; return;
            }
            m_state = CentrifugeState::Processing;
            current.elapsed += dt * m_speedMult;
            if (current.isComplete()) m_state = CentrifugeState::Complete;
        }
    }

    void collectOutput() {
        if (m_state != CentrifugeState::Complete || m_queue.empty()) return;
        m_queue.erase(m_queue.begin());
        m_state = m_queue.empty() ? CentrifugeState::Idle : CentrifugeState::Loading;
    }

    [[nodiscard]] CentrifugeState     state()       const { return m_state; }
    [[nodiscard]] size_t              queueSize()   const { return m_queue.size(); }
    [[nodiscard]] int                 maxQueueSize() const { return m_maxQueue; }
    [[nodiscard]] const CentrifugeJob* currentJob() const {
        return m_queue.empty() ? nullptr : &m_queue.front();
    }
    [[nodiscard]] float currentProgress() const {
        return m_queue.empty() ? 0.f : m_queue.front().progress();
    }

private:
    int                        m_tier      = 1;
    float                      m_speedMult = 1.0f;
    int                        m_maxQueue  = 4;
    CentrifugeState            m_state     = CentrifugeState::Idle;
    std::vector<CentrifugeJob> m_queue;
};

// ── SP3 Interface Port ────────────────────────────────────────

enum class LinkState : uint8_t {
    Idle = 0, Contact, Linking, Linked, Control, LinkFailed
};

inline const char* linkStateName(LinkState s) {
    switch (s) {
    case LinkState::Idle:       return "Idle";
    case LinkState::Contact:    return "Contact";
    case LinkState::Linking:    return "Linking";
    case LinkState::Linked:     return "Linked";
    case LinkState::Control:    return "Control";
    case LinkState::LinkFailed: return "LinkFailed";
    default: return "Unknown";
    }
}

class InterfacePort {
public:
    void beginContact(const std::string& targetId) {
        if (m_state != LinkState::Idle) return;
        m_targetId = targetId; m_state = LinkState::Contact;
    }
    void attemptLink() {
        if (m_state != LinkState::Contact) return;
        m_state = LinkState::Linking; m_linkQuality = 0.85f; m_state = LinkState::Linked;
    }
    void enterControl() {
        if (m_state != LinkState::Linked) return;
        m_state = LinkState::Control;
    }
    void disconnect() {
        m_state = LinkState::Idle; m_targetId.clear(); m_linkQuality = 0.f;
    }
    void failLink() { m_state = LinkState::LinkFailed; }
    void retryFromFail() {
        if (m_state != LinkState::LinkFailed) return;
        m_state = LinkState::Contact;
    }

    [[nodiscard]] LinkState          state()         const { return m_state; }
    [[nodiscard]] const std::string& currentTarget() const { return m_targetId; }
    [[nodiscard]] float              linkQuality()   const { return m_linkQuality; }
    [[nodiscard]] bool               isLinked()      const { return m_state == LinkState::Linked || m_state == LinkState::Control; }
    [[nodiscard]] bool               hasControl()    const { return m_state == LinkState::Control; }

private:
    LinkState   m_state       = LinkState::Idle;
    std::string m_targetId;
    float       m_linkQuality = 0.f;
};

// ── SP4 Sand Physics System ──────────────────────────────────

struct CollapseEvent {
    int x = 0, y = 0, z = 0;
    VoxelType material = VoxelType::Air;
    int fallDistance = 0;
};

class SandPhysicsSystem {
public:
    void setMaterialTable(const VoxelMaterialTable* table) { m_materials = table; }

    [[nodiscard]] bool wouldCollapse(const Chunk& chunk, int x, int y, int z) const {
        if (!m_materials) return false;
        auto type = chunk.get(x, y, z);
        const auto& mat = m_materials->get(type);
        if (!mat.canCollapse) return false;
        if (y <= 0) return false;
        return chunk.get(x, y - 1, z) == VoxelType::Air || chunk.get(x, y - 1, z) == VoxelType::Water;
    }

    int simulateStep(Chunk& chunk) {
        m_lastEvents.clear();
        if (!m_materials) return 0;
        int moved = 0;
        for (int y = 1; y < CHUNK_SIZE; ++y) {
            for (int z = 0; z < CHUNK_SIZE; ++z) {
                for (int x = 0; x < CHUNK_SIZE; ++x) {
                    if (!wouldCollapse(chunk, x, y, z)) continue;
                    auto type = chunk.get(x, y, z);
                    int targetY = y - 1;
                    while (targetY > 0 && chunk.get(x, targetY - 1, z) == VoxelType::Air) --targetY;
                    chunk.set(x, y, z, VoxelType::Air);
                    chunk.set(x, targetY, z, type);
                    m_lastEvents.push_back({x, y, z, type, y - targetY});
                    ++moved;
                }
            }
        }
        return moved;
    }

    int simulateUntilStable(Chunk& chunk, int maxIterations = 64) {
        int total = 0;
        for (int i = 0; i < maxIterations; ++i) {
            int moved = simulateStep(chunk);
            if (moved == 0) break;
            total += moved;
        }
        return total;
    }

    [[nodiscard]] const std::vector<CollapseEvent>& lastEvents() const { return m_lastEvents; }

private:
    const VoxelMaterialTable* m_materials = nullptr;
    std::vector<CollapseEvent> m_lastEvents;
};

// ── SP5 Breach Minigame ──────────────────────────────────────

enum class BreachState : uint8_t {
    Inactive = 0, Initiating, Active, Success, Failure, Partial, Cooldown
};

inline const char* breachStateName(BreachState s) {
    switch (s) {
    case BreachState::Inactive:   return "Inactive";
    case BreachState::Initiating: return "Initiating";
    case BreachState::Active:     return "Active";
    case BreachState::Success:    return "Success";
    case BreachState::Failure:    return "Failure";
    case BreachState::Partial:    return "Partial";
    case BreachState::Cooldown:   return "Cooldown";
    default: return "Unknown";
    }
}

struct BreachGrid {
    int   width       = 6;
    int   height      = 6;
    float timeLimit   = 30.f;
    int   iceNodes    = 3;
    int   dataNodes   = 2;
};

class BreachMinigame {
public:
    void initiate(const BreachGrid& grid) {
        m_grid = grid; m_timeRemaining = grid.timeLimit;
        m_dataCollected = 0; m_traceX = 0; m_traceY = 0;
        m_state = BreachState::Initiating;
    }
    void start() {
        if (m_state != BreachState::Initiating) return;
        m_state = BreachState::Active;
    }
    void tick(float dt) {
        if (m_state != BreachState::Active) return;
        m_timeRemaining -= dt;
        if (m_timeRemaining <= 0.f) {
            m_timeRemaining = 0.f;
            m_state = (m_dataCollected > 0) ? BreachState::Partial : BreachState::Failure;
        }
    }
    void moveTrace(int dx, int dy) {
        if (m_state != BreachState::Active) return;
        int nx = m_traceX + dx, ny = m_traceY + dy;
        if (nx < 0 || nx >= m_grid.width || ny < 0 || ny >= m_grid.height) return;
        m_traceX = nx; m_traceY = ny;
    }
    void collectDataNode() {
        if (m_state != BreachState::Active) return;
        ++m_dataCollected;
        if (m_dataCollected >= m_grid.dataNodes) m_state = BreachState::Success;
    }
    void hitIce() {
        if (m_state != BreachState::Active) return;
        m_state = (m_dataCollected > 0) ? BreachState::Partial : BreachState::Failure;
    }
    void reset() { m_state = BreachState::Inactive; }

    [[nodiscard]] BreachState       state()         const { return m_state; }
    [[nodiscard]] float             timeRemaining() const { return m_timeRemaining; }
    [[nodiscard]] int               dataCollected() const { return m_dataCollected; }
    [[nodiscard]] int               traceX()        const { return m_traceX; }
    [[nodiscard]] int               traceY()        const { return m_traceY; }
    [[nodiscard]] const BreachGrid& grid()          const { return m_grid; }

private:
    BreachState m_state         = BreachState::Inactive;
    BreachGrid  m_grid;
    float       m_timeRemaining = 0.f;
    int         m_dataCollected = 0;
    int         m_traceX        = 0;
    int         m_traceY        = 0;
};

// ── SP6 R.I.G. AI Event System ────────────────────────────────

enum class RigAIEvent : uint8_t {
    PowerChanged = 0, HealthChanged, OxygenLow, ScanResult, ThreatDetected,
    EnvironmentChanged, ModuleInstalled, ModuleRemoved, DroneStatus, SystemFailure,
    Count
};

inline const char* rigAIEventName(RigAIEvent e) {
    switch (e) {
    case RigAIEvent::PowerChanged:       return "PowerChanged";
    case RigAIEvent::HealthChanged:      return "HealthChanged";
    case RigAIEvent::OxygenLow:          return "OxygenLow";
    case RigAIEvent::ScanResult:         return "ScanResult";
    case RigAIEvent::ThreatDetected:     return "ThreatDetected";
    case RigAIEvent::EnvironmentChanged: return "EnvironmentChanged";
    case RigAIEvent::ModuleInstalled:    return "ModuleInstalled";
    case RigAIEvent::ModuleRemoved:      return "ModuleRemoved";
    case RigAIEvent::DroneStatus:        return "DroneStatus";
    case RigAIEvent::SystemFailure:      return "SystemFailure";
    default: return "Unknown";
    }
}

struct RigAIFeatures {
    bool vitals        = true;
    bool scanning      = false;
    bool mapping       = false;
    bool navigation    = false;
    bool droneControl  = false;
    bool healingControl= false;
    bool fleetCommand  = false;

    [[nodiscard]] int enabledCount() const {
        int n = 0;
        if (vitals)         ++n;
        if (scanning)       ++n;
        if (mapping)        ++n;
        if (navigation)     ++n;
        if (droneControl)   ++n;
        if (healingControl) ++n;
        if (fleetCommand)   ++n;
        return n;
    }
};

struct RigAIAlert {
    RigAIEvent  event;
    std::string message;
    float       severity = 0.f;
};

class RigAICore {
public:
    void init(const RigAIFeatures& features) { m_features = features; m_initialized = true; }

    void onEvent(RigAIEvent event, float value = 0.f) {
        if (!m_initialized) return;
        switch (event) {
        case RigAIEvent::ScanResult:  if (!m_features.scanning) return; break;
        case RigAIEvent::DroneStatus: if (!m_features.droneControl) return; break;
        default: break;
        }
        float sev = (event == RigAIEvent::OxygenLow || event == RigAIEvent::SystemFailure) ? 1.0f :
                     (event == RigAIEvent::ThreatDetected) ? 0.8f : 0.3f;
        m_alerts.push_back({event, rigAIEventName(event), sev});
        (void)value;
    }

    void tick(float /*dt*/) {}
    void enableFeature(const std::string& f) {
        if      (f == "scanning")       m_features.scanning = true;
        else if (f == "mapping")        m_features.mapping = true;
        else if (f == "navigation")     m_features.navigation = true;
        else if (f == "droneControl")   m_features.droneControl = true;
        else if (f == "healingControl") m_features.healingControl = true;
        else if (f == "fleetCommand")   m_features.fleetCommand = true;
    }
    void disableFeature(const std::string& f) {
        if      (f == "scanning")       m_features.scanning = false;
        else if (f == "mapping")        m_features.mapping = false;
        else if (f == "navigation")     m_features.navigation = false;
        else if (f == "droneControl")   m_features.droneControl = false;
        else if (f == "healingControl") m_features.healingControl = false;
        else if (f == "fleetCommand")   m_features.fleetCommand = false;
    }

    [[nodiscard]] const RigAIFeatures&        features()      const { return m_features; }
    [[nodiscard]] bool                         isInitialized() const { return m_initialized; }
    [[nodiscard]] size_t                       pendingAlerts() const { return m_alerts.size(); }
    [[nodiscard]] const std::vector<RigAIAlert>& alerts()      const { return m_alerts; }
    void clearAlerts() { m_alerts.clear(); }

private:
    RigAIFeatures           m_features;
    bool                    m_initialized = false;
    std::vector<RigAIAlert> m_alerts;
};

// ── G22 Weather System ────────────────────────────────────────────

enum class WeatherType : uint8_t {
    Clear      = 0,
    Rain       = 1,
    Storm      = 2,
    Snow       = 3,
    Fog        = 4,
    Sandstorm  = 5,
    AcidRain   = 6,
    SolarFlare = 7
};

inline const char* weatherTypeName(WeatherType t) {
    switch (t) {
        case WeatherType::Clear:      return "Clear";
        case WeatherType::Rain:       return "Rain";
        case WeatherType::Storm:      return "Storm";
        case WeatherType::Snow:       return "Snow";
        case WeatherType::Fog:        return "Fog";
        case WeatherType::Sandstorm:  return "Sandstorm";
        case WeatherType::AcidRain:   return "Acid Rain";
        case WeatherType::SolarFlare: return "Solar Flare";
    }
    return "Unknown";
}

/// Active weather condition with intensity and duration tracking.
struct WeatherCondition {
    WeatherType type      = WeatherType::Clear;
    float intensity       = 0.f;    // 0.0 = calm, 1.0 = extreme
    float duration        = 0.f;    // total duration in seconds (0 = indefinite)
    float elapsed         = 0.f;    // time elapsed in this condition
    float transitionTime  = 2.f;    // fade-in/out period

    [[nodiscard]] bool isExpired() const {
        return duration > 0.f && elapsed >= duration;
    }

    [[nodiscard]] float progress() const {
        return duration > 0.f ? std::min(elapsed / duration, 1.f) : 0.f;
    }

    /// Effective intensity accounting for fade-in at start and fade-out near end.
    [[nodiscard]] float effectiveIntensity() const {
        float t = intensity;
        // Fade in
        if (elapsed < transitionTime)
            t *= (elapsed / transitionTime);
        // Fade out near end
        if (duration > 0.f) {
            float remaining = duration - elapsed;
            if (remaining < transitionTime)
                t *= (remaining / transitionTime);
        }
        return std::max(t, 0.f);
    }
};

/// Environmental effects applied by the weather system to gameplay.
struct WeatherEffects {
    float visibilityMultiplier  = 1.f;   // 1.0 = full, 0.0 = zero visibility
    float movementMultiplier    = 1.f;   // affects player/ship speed
    float damagePerSecond       = 0.f;   // AcidRain / SolarFlare damage
    float miningMultiplier      = 1.f;   // affects mining output
    bool  disablesScanner       = false; // Storm/SolarFlare blocks scans
    bool  disablesNavigation    = false; // Severe conditions block nav

    static WeatherEffects forCondition(const WeatherCondition& c) {
        WeatherEffects fx;
        float i = c.effectiveIntensity();
        switch (c.type) {
            case WeatherType::Clear:
                break;
            case WeatherType::Rain:
                fx.visibilityMultiplier = 1.f - 0.3f * i;
                fx.movementMultiplier   = 1.f - 0.1f * i;
                break;
            case WeatherType::Storm:
                fx.visibilityMultiplier = 1.f - 0.6f * i;
                fx.movementMultiplier   = 1.f - 0.3f * i;
                fx.disablesScanner      = i > 0.7f;
                break;
            case WeatherType::Snow:
                fx.visibilityMultiplier = 1.f - 0.4f * i;
                fx.movementMultiplier   = 1.f - 0.2f * i;
                fx.miningMultiplier     = 1.f - 0.15f * i;
                break;
            case WeatherType::Fog:
                fx.visibilityMultiplier = 1.f - 0.7f * i;
                break;
            case WeatherType::Sandstorm:
                fx.visibilityMultiplier = 1.f - 0.8f * i;
                fx.movementMultiplier   = 1.f - 0.4f * i;
                fx.miningMultiplier     = 1.f - 0.3f * i;
                fx.damagePerSecond      = 2.f * i;
                fx.disablesNavigation   = i > 0.8f;
                break;
            case WeatherType::AcidRain:
                fx.visibilityMultiplier = 1.f - 0.3f * i;
                fx.damagePerSecond      = 5.f * i;
                break;
            case WeatherType::SolarFlare:
                fx.visibilityMultiplier = 1.f - 0.2f * i;
                fx.damagePerSecond      = 8.f * i;
                fx.disablesScanner      = true;
                fx.disablesNavigation   = i > 0.5f;
                break;
        }
        return fx;
    }
};

/// Forecast entry — a pending future weather event.
struct WeatherForecastEntry {
    WeatherType type      = WeatherType::Clear;
    float intensity       = 0.5f;
    float duration        = 60.f;
    float delayUntilStart = 30.f;  // seconds until this event begins
};

/// Central weather system managing active weather, transitions, and forecasting.
class WeatherSystem {
public:
    static constexpr int kMaxForecast = 8;

    /// Set the active weather condition directly (immediate change).
    void setWeather(WeatherType type, float intensity, float duration = 0.f) {
        m_active.type      = type;
        m_active.intensity = std::max(0.f, std::min(intensity, 1.f));
        m_active.duration  = duration;
        m_active.elapsed   = 0.f;
    }

    /// Queue a weather event in the forecast.
    void addForecast(const WeatherForecastEntry& entry) {
        if (static_cast<int>(m_forecast.size()) >= kMaxForecast) return;
        m_forecast.push_back(entry);
    }

    /// Clear all forecast entries.
    void clearForecast() { m_forecast.clear(); }

    /// Advance the weather system by dt seconds.
    void tick(float dt) {
        m_active.elapsed += dt;

        // If the current weather has expired, transition to next forecast or clear.
        if (m_active.isExpired()) {
            if (!m_forecast.empty()) {
                auto next = m_forecast.front();
                m_forecast.erase(m_forecast.begin());
                setWeather(next.type, next.intensity, next.duration);
            } else {
                setWeather(WeatherType::Clear, 0.f);
            }
        }

        // Tick forecast delays
        for (auto& f : m_forecast)
            f.delayUntilStart = std::max(0.f, f.delayUntilStart - dt);

        // Check if any forecast entry is ready (delay reached zero and current is clear)
        if (m_active.type == WeatherType::Clear && !m_forecast.empty()) {
            if (m_forecast.front().delayUntilStart <= 0.f) {
                auto next = m_forecast.front();
                m_forecast.erase(m_forecast.begin());
                setWeather(next.type, next.intensity, next.duration);
            }
        }
    }

    [[nodiscard]] const WeatherCondition& activeWeather() const { return m_active; }
    [[nodiscard]] WeatherType currentType() const { return m_active.type; }
    [[nodiscard]] float currentIntensity() const { return m_active.effectiveIntensity(); }
    [[nodiscard]] bool isClear() const { return m_active.type == WeatherType::Clear; }

    /// Compute the current gameplay effects of the active weather.
    [[nodiscard]] WeatherEffects currentEffects() const {
        return WeatherEffects::forCondition(m_active);
    }

    [[nodiscard]] const std::vector<WeatherForecastEntry>& forecast() const { return m_forecast; }
    [[nodiscard]] size_t forecastCount() const { return m_forecast.size(); }

    /// Force clear weather immediately.
    void clearWeather() {
        setWeather(WeatherType::Clear, 0.f);
    }

private:
    WeatherCondition m_active;
    std::vector<WeatherForecastEntry> m_forecast;
};

// ── G23 Trading System ────────────────────────────────────────────

enum class TradeGoodCategory : uint8_t {
    Raw         = 0,
    Refined     = 1,
    Component   = 2,
    Consumable  = 3,
    Tech        = 4,
    Luxury      = 5,
    Contraband  = 6,
    Data        = 7
};

inline const char* tradeGoodCategoryName(TradeGoodCategory c) {
    switch (c) {
        case TradeGoodCategory::Raw:        return "Raw";
        case TradeGoodCategory::Refined:    return "Refined";
        case TradeGoodCategory::Component:  return "Component";
        case TradeGoodCategory::Consumable: return "Consumable";
        case TradeGoodCategory::Tech:       return "Tech";
        case TradeGoodCategory::Luxury:     return "Luxury";
        case TradeGoodCategory::Contraband: return "Contraband";
        case TradeGoodCategory::Data:       return "Data";
    }
    return "Unknown";
}

/// Description of a tradeable good.
struct TradeGood {
    std::string         id;
    std::string         name;
    TradeGoodCategory   category    = TradeGoodCategory::Raw;
    float               basePrice   = 10.f;    // credits per unit
    float               weight      = 1.f;     // kg per unit
    bool                legal       = true;     // false = contraband

    [[nodiscard]] bool isContraband() const { return !legal; }
};

/// A pending buy or sell offer at a trading post.
struct TradeOffer {
    std::string goodId;
    int         quantity     = 0;
    float       pricePerUnit = 0.f;
    bool        isBuyOffer   = true;  // true = the post wants to buy, false = selling
};

/// A trade route connecting two posts.
struct TradeRoute {
    std::string originId;
    std::string destinationId;
    std::string goodId;
    float       profitMargin = 0.f;  // percentage 0-1
    float       riskLevel    = 0.f;  // 0 = safe, 1 = very dangerous
    float       distance     = 0.f;  // abstract distance units
};

/// A single trading post — manages local inventory, pricing, and offers.
class TradingPost {
public:
    TradingPost() = default;
    TradingPost(const std::string& id, const std::string& name)
        : m_id(id), m_name(name) {}

    [[nodiscard]] const std::string& postId() const { return m_id; }
    [[nodiscard]] const std::string& name() const { return m_name; }

    /// Add stock of a good.
    void addStock(const std::string& goodId, int quantity, float pricePerUnit) {
        for (auto& s : m_stock) {
            if (s.goodId == goodId) {
                s.quantity += quantity;
                s.pricePerUnit = pricePerUnit;
                return;
            }
        }
        m_stock.push_back({goodId, quantity, pricePerUnit, false});
    }

    /// Remove stock (e.g. after sale).  Returns actual amount removed.
    int removeStock(const std::string& goodId, int quantity) {
        for (auto& s : m_stock) {
            if (s.goodId == goodId) {
                int removed = std::min(s.quantity, quantity);
                s.quantity -= removed;
                return removed;
            }
        }
        return 0;
    }

    /// Query current stock for a good.
    [[nodiscard]] int stockOf(const std::string& goodId) const {
        for (auto& s : m_stock) if (s.goodId == goodId) return s.quantity;
        return 0;
    }

    /// Get the current price for a good.
    [[nodiscard]] float priceOf(const std::string& goodId) const {
        for (auto& s : m_stock) if (s.goodId == goodId) return s.pricePerUnit;
        return 0.f;
    }

    /// Execute a buy: player buys 'quantity' of goodId from the post.
    /// Returns total cost (0 if insufficient stock).
    float buy(const std::string& goodId, int quantity) {
        for (auto& s : m_stock) {
            if (s.goodId != goodId) continue;
            if (s.quantity < quantity) return 0.f;  // insufficient stock
            float cost = s.pricePerUnit * static_cast<float>(quantity);
            s.quantity -= quantity;
            m_totalSales += cost;
            return cost;
        }
        return 0.f;
    }

    /// Execute a sell: player sells 'quantity' of goodId to the post.
    /// Returns revenue (buy-back is at 80% of current price).
    float sell(const std::string& goodId, int quantity, float basePrice) {
        float sellPrice = basePrice * 0.8f;  // 80% buy-back rate
        addStock(goodId, quantity, basePrice);
        float revenue = sellPrice * static_cast<float>(quantity);
        m_totalPurchases += revenue;
        return revenue;
    }

    /// Apply supply/demand price fluctuation per tick.
    void tickPrices(float dt) {
        for (auto& s : m_stock) {
            // Low stock → price rises, high stock → price falls
            float supplyFactor = 1.f;
            if (s.quantity < 10)       supplyFactor = 1.05f;
            else if (s.quantity > 100) supplyFactor = 0.97f;
            s.pricePerUnit *= std::pow(supplyFactor, dt);
            s.pricePerUnit = std::max(0.01f, s.pricePerUnit);  // price floor
        }
    }

    [[nodiscard]] const std::vector<TradeOffer>& stock() const { return m_stock; }
    [[nodiscard]] size_t stockCount() const { return m_stock.size(); }
    [[nodiscard]] float totalSales() const { return m_totalSales; }
    [[nodiscard]] float totalPurchases() const { return m_totalPurchases; }

    /// Set a tax rate (0-1).
    void setTaxRate(float rate) { m_taxRate = std::max(0.f, std::min(rate, 1.f)); }
    [[nodiscard]] float taxRate() const { return m_taxRate; }

private:
    std::string m_id;
    std::string m_name;
    std::vector<TradeOffer> m_stock;
    float m_totalSales     = 0.f;
    float m_totalPurchases = 0.f;
    float m_taxRate        = 0.05f;  // 5% default
};

/// Central trading system managing all posts, routes, and global economics.
class TradingSystem {
public:
    static constexpr int kMaxPosts  = 32;
    static constexpr int kMaxRoutes = 64;
    static constexpr int kMaxGoods  = 128;

    /// Register a trade good definition.
    void registerGood(const TradeGood& good) {
        if (static_cast<int>(m_goods.size()) >= kMaxGoods) return;
        for (auto& g : m_goods) if (g.id == good.id) return;  // no duplicates
        m_goods.push_back(good);
    }

    /// Add a trading post.
    void addPost(TradingPost post) {
        if (static_cast<int>(m_posts.size()) >= kMaxPosts) return;
        m_posts.push_back(std::move(post));
    }

    /// Remove a post by ID.
    bool removePost(const std::string& id) {
        for (auto it = m_posts.begin(); it != m_posts.end(); ++it) {
            if (it->postId() == id) { m_posts.erase(it); return true; }
        }
        return false;
    }

    /// Find a post by ID.
    [[nodiscard]] TradingPost* findPost(const std::string& id) {
        for (auto& p : m_posts) if (p.postId() == id) return &p;
        return nullptr;
    }

    /// Add a trade route.
    void addRoute(const TradeRoute& route) {
        if (static_cast<int>(m_routes.size()) >= kMaxRoutes) return;
        m_routes.push_back(route);
    }

    /// Find the trade good definition.
    [[nodiscard]] const TradeGood* findGood(const std::string& id) const {
        for (auto& g : m_goods) if (g.id == id) return &g;
        return nullptr;
    }

    /// Execute a trade: player buys from a post.
    float executeBuy(const std::string& postId, const std::string& goodId, int qty) {
        auto* post = findPost(postId);
        if (!post) return 0.f;
        float cost = post->buy(goodId, qty);
        if (cost > 0.f) m_totalVolume += cost;
        return cost;
    }

    /// Execute a trade: player sells to a post.
    float executeSell(const std::string& postId, const std::string& goodId, int qty) {
        auto* post = findPost(postId);
        if (!post) return 0.f;
        const auto* good = findGood(goodId);
        float basePrice = good ? good->basePrice : 10.f;
        float revenue = post->sell(goodId, qty, basePrice);
        if (revenue > 0.f) m_totalVolume += revenue;
        return revenue;
    }

    /// Advance all post prices by dt seconds.
    void tick(float dt) {
        for (auto& p : m_posts)
            p.tickPrices(dt);
    }

    /// Calculate profit margin for a route (buy at origin, sell at destination).
    [[nodiscard]] float routeProfitMargin(const TradeRoute& route) const {
        const TradingPost* origin = nullptr;
        const TradingPost* dest = nullptr;
        for (auto& p : m_posts) {
            if (p.postId() == route.originId) origin = &p;
            if (p.postId() == route.destinationId) dest = &p;
        }
        if (!origin || !dest) return 0.f;

        float buyPrice  = origin->priceOf(route.goodId);
        float sellPrice = dest->priceOf(route.goodId) * 0.8f;  // sell at 80%
        if (buyPrice <= 0.f) return 0.f;
        return (sellPrice - buyPrice) / buyPrice;
    }

    [[nodiscard]] const std::vector<TradeGood>& goods() const { return m_goods; }
    [[nodiscard]] const std::vector<TradingPost>& posts() const { return m_posts; }
    [[nodiscard]] const std::vector<TradeRoute>& routes() const { return m_routes; }
    [[nodiscard]] size_t postCount() const { return m_posts.size(); }
    [[nodiscard]] size_t routeCount() const { return m_routes.size(); }
    [[nodiscard]] size_t goodCount() const { return m_goods.size(); }
    [[nodiscard]] float totalVolume() const { return m_totalVolume; }

private:
    std::vector<TradeGood>    m_goods;
    std::vector<TradingPost>  m_posts;
    std::vector<TradeRoute>   m_routes;
    float m_totalVolume = 0.f;
};

// ── G24 Base Building System ──────────────────────────────────────

enum class BasePartCategory : uint8_t {
    Foundation = 0,
    Wall       = 1,
    Floor      = 2,
    Ceiling    = 3,
    Door       = 4,
    Window     = 5,
    Utility    = 6,
    Decoration = 7
};

inline const char* basePartCategoryName(BasePartCategory c) {
    switch (c) {
        case BasePartCategory::Foundation: return "Foundation";
        case BasePartCategory::Wall:       return "Wall";
        case BasePartCategory::Floor:      return "Floor";
        case BasePartCategory::Ceiling:    return "Ceiling";
        case BasePartCategory::Door:       return "Door";
        case BasePartCategory::Window:     return "Window";
        case BasePartCategory::Utility:    return "Utility";
        case BasePartCategory::Decoration: return "Decoration";
    }
    return "Unknown";
}

/// A single buildable part in a base.
struct BasePart {
    std::string       id;
    std::string       name;
    BasePartCategory  category   = BasePartCategory::Foundation;
    float             hitPoints  = 100.f;
    float             buildCost  = 10.f;   // resource units
    float             powerDraw  = 0.f;    // watts consumed
    float             weight     = 1.f;    // kg

    [[nodiscard]] bool requiresPower() const { return powerDraw > 0.f; }
};

/// Grid coordinates for part placement.
struct BaseGridPos {
    int x = 0;
    int y = 0;
    int z = 0;

    bool operator==(const BaseGridPos& o) const { return x == o.x && y == o.y && z == o.z; }
    bool operator!=(const BaseGridPos& o) const { return !(*this == o); }
};

/// A placed part instance within a base layout.
struct PlacedBasePart {
    std::string partId;
    BaseGridPos position;
    float       currentHP = 100.f;
    bool        powered   = true;
};

/// Grid-based base layout managing part placement and adjacency.
class BaseLayout {
public:
    static constexpr int kMaxParts = 256;

    /// Place a part at the given grid position. Returns false if occupied or at limit.
    bool placePart(const std::string& partId, BaseGridPos pos, float hp = 100.f) {
        if (static_cast<int>(m_parts.size()) >= kMaxParts) return false;
        // Check for collision
        for (auto& p : m_parts) {
            if (p.position == pos) return false;
        }
        PlacedBasePart pp;
        pp.partId = partId;
        pp.position = pos;
        pp.currentHP = hp;
        m_parts.push_back(pp);
        return true;
    }

    /// Remove a part at the given position.
    bool removePart(BaseGridPos pos) {
        for (auto it = m_parts.begin(); it != m_parts.end(); ++it) {
            if (it->position == pos) { m_parts.erase(it); return true; }
        }
        return false;
    }

    /// Find a part at the given position.
    [[nodiscard]] PlacedBasePart* partAt(BaseGridPos pos) {
        for (auto& p : m_parts) if (p.position == pos) return &p;
        return nullptr;
    }

    /// Count adjacent parts (Manhattan distance = 1 on same plane or directly above/below).
    [[nodiscard]] int adjacentCount(BaseGridPos pos) const {
        int count = 0;
        for (auto& p : m_parts) {
            int dx = std::abs(p.position.x - pos.x);
            int dy = std::abs(p.position.y - pos.y);
            int dz = std::abs(p.position.z - pos.z);
            if (dx + dy + dz == 1) ++count;
        }
        return count;
    }

    /// Check structural integrity: every non-foundation part must be adjacent to at least one other part.
    [[nodiscard]] bool isStructurallySound(
        const std::vector<BasePart>& partDefs) const {
        for (auto& placed : m_parts) {
            // Find the part definition
            const BasePart* def = nullptr;
            for (auto& d : partDefs) {
                if (d.id == placed.partId) { def = &d; break; }
            }
            if (!def) continue;
            // Foundations are always valid
            if (def->category == BasePartCategory::Foundation) continue;
            // Other parts need at least one neighbor
            if (adjacentCount(placed.position) == 0) return false;
        }
        return true;
    }

    /// Calculate total power draw of all parts.
    [[nodiscard]] float totalPowerDraw(const std::vector<BasePart>& partDefs) const {
        float total = 0.f;
        for (auto& placed : m_parts) {
            for (auto& d : partDefs) {
                if (d.id == placed.partId) { total += d.powerDraw; break; }
            }
        }
        return total;
    }

    [[nodiscard]] const std::vector<PlacedBasePart>& parts() const { return m_parts; }
    [[nodiscard]] size_t partCount() const { return m_parts.size(); }
    void clear() { m_parts.clear(); }

private:
    std::vector<PlacedBasePart> m_parts;
};

/// Defensive capabilities of a base.
struct BaseDefense {
    int   turretSlots     = 0;
    float shieldStrength  = 0.f;    // max shield HP
    float hullArmor       = 0.f;    // damage reduction percentage 0-1
    float currentShield   = 0.f;

    /// Take damage — shields absorb first, then hull pass-through.
    float takeDamage(float damage) {
        if (currentShield > 0.f) {
            if (damage <= currentShield) {
                currentShield -= damage;
                return 0.f;
            }
            damage -= currentShield;
            currentShield = 0.f;
        }
        // Apply armor reduction
        return damage * (1.f - hullArmor);
    }

    /// Regenerate shields by amount, capped at max.
    void regenShield(float amount) {
        currentShield = std::min(currentShield + amount, shieldStrength);
    }

    /// Reset shields to full.
    void resetShields() { currentShield = shieldStrength; }
};

/// Central base system managing power, life support, storage, and defenses.
class BaseSystem {
public:
    static constexpr int kMaxBases = 8;
    static constexpr float kDefaultPowerOutput = 100.f;

    /// Register a part definition.
    void registerPart(const BasePart& part) {
        for (auto& p : m_partDefs) if (p.id == part.id) return;
        m_partDefs.push_back(part);
    }

    /// Create a new base with given name. Returns base index or -1 if at limit.
    int createBase(const std::string& name) {
        if (static_cast<int>(m_baseNames.size()) >= kMaxBases) return -1;
        m_baseNames.push_back(name);
        m_layouts.emplace_back();
        m_defenses.emplace_back();
        m_powerOutputs.push_back(kDefaultPowerOutput);
        return static_cast<int>(m_baseNames.size()) - 1;
    }

    /// Remove a base by index.
    bool removeBase(int index) {
        if (index < 0 || index >= static_cast<int>(m_baseNames.size())) return false;
        m_baseNames.erase(m_baseNames.begin() + index);
        m_layouts.erase(m_layouts.begin() + index);
        m_defenses.erase(m_defenses.begin() + index);
        m_powerOutputs.erase(m_powerOutputs.begin() + index);
        return true;
    }

    /// Access base layout by index.
    [[nodiscard]] BaseLayout* layout(int index) {
        if (index < 0 || index >= static_cast<int>(m_layouts.size())) return nullptr;
        return &m_layouts[static_cast<size_t>(index)];
    }

    /// Access base defense by index.
    [[nodiscard]] BaseDefense* defense(int index) {
        if (index < 0 || index >= static_cast<int>(m_defenses.size())) return nullptr;
        return &m_defenses[static_cast<size_t>(index)];
    }

    /// Get base name.
    [[nodiscard]] const std::string& baseName(int index) const {
        static const std::string empty;
        if (index < 0 || index >= static_cast<int>(m_baseNames.size())) return empty;
        return m_baseNames[static_cast<size_t>(index)];
    }

    /// Set power output for a base.
    void setPowerOutput(int index, float watts) {
        if (index >= 0 && index < static_cast<int>(m_powerOutputs.size()))
            m_powerOutputs[static_cast<size_t>(index)] = watts;
    }

    /// Check if a base has sufficient power (output >= draw).
    [[nodiscard]] bool hasSufficientPower(int index) const {
        if (index < 0 || index >= static_cast<int>(m_layouts.size())) return false;
        float draw = m_layouts[static_cast<size_t>(index)].totalPowerDraw(m_partDefs);
        return m_powerOutputs[static_cast<size_t>(index)] >= draw;
    }

    /// Get available power (output - draw).
    [[nodiscard]] float availablePower(int index) const {
        if (index < 0 || index >= static_cast<int>(m_powerOutputs.size())) return 0.f;
        float draw = m_layouts[static_cast<size_t>(index)].totalPowerDraw(m_partDefs);
        return m_powerOutputs[static_cast<size_t>(index)] - draw;
    }

    [[nodiscard]] const std::vector<BasePart>& partDefinitions() const { return m_partDefs; }
    [[nodiscard]] size_t baseCount() const { return m_baseNames.size(); }
    [[nodiscard]] size_t partDefCount() const { return m_partDefs.size(); }

    /// Find part definition by ID.
    [[nodiscard]] const BasePart* findPart(const std::string& id) const {
        for (auto& p : m_partDefs) if (p.id == id) return &p;
        return nullptr;
    }

private:
    std::vector<BasePart>     m_partDefs;
    std::vector<std::string>  m_baseNames;
    std::vector<BaseLayout>   m_layouts;
    std::vector<BaseDefense>  m_defenses;
    std::vector<float>        m_powerOutputs;
};

// ── G25 Habitat System ────────────────────────────────────────────

enum class HabitatZoneType : uint8_t {
    Living      = 0,
    Engineering = 1,
    Medical     = 2,
    Command     = 3,
    Cargo       = 4,
    Recreation  = 5,
    Hydroponics = 6,
    Airlock     = 7
};

inline const char* habitatZoneTypeName(HabitatZoneType t) {
    switch (t) {
        case HabitatZoneType::Living:      return "Living";
        case HabitatZoneType::Engineering: return "Engineering";
        case HabitatZoneType::Medical:     return "Medical";
        case HabitatZoneType::Command:     return "Command";
        case HabitatZoneType::Cargo:       return "Cargo";
        case HabitatZoneType::Recreation:  return "Recreation";
        case HabitatZoneType::Hydroponics: return "Hydroponics";
        case HabitatZoneType::Airlock:     return "Airlock";
    }
    return "Unknown";
}

/// A single zone in a habitat (room/section).
struct HabitatZone {
    std::string     id;
    std::string     name;
    HabitatZoneType type        = HabitatZoneType::Living;
    int             capacity    = 4;        // max crew occupancy
    int             occupants   = 0;
    float           oxygenLevel = 1.f;      // 0–1 (fraction of normal)
    float           temperature = 22.f;     // Celsius
    float           pressure    = 1.f;      // atmospheres
    bool            sealed      = true;     // false if breached

    [[nodiscard]] bool isHabitable() const {
        return sealed && oxygenLevel > 0.15f && pressure > 0.5f
               && temperature > 5.f && temperature < 45.f;
    }

    [[nodiscard]] bool isBreached() const { return !sealed; }

    [[nodiscard]] float availableCapacity() const {
        return static_cast<float>(capacity - occupants);
    }
};

/// Life support module — maintains atmosphere in connected zones.
struct LifeSupportModule {
    std::string id;
    float oxygenGenRate  = 0.1f;   // O₂ per second
    float co2ScrubRate   = 0.08f;  // CO₂ removed per second
    float tempTarget     = 22.f;   // target temperature °C
    float tempAdjustRate = 0.5f;   // °C per second of convergence
    float powerDraw      = 15.f;   // watts
    bool  active         = true;

    [[nodiscard]] bool isOperational() const { return active; }
};

/// Habitat layout — manages zones and connectivity.
class HabitatLayout {
public:
    static constexpr int kMaxZones = 32;

    /// Add a zone. Returns false if at capacity or duplicate ID.
    bool addZone(const HabitatZone& zone) {
        if (static_cast<int>(m_zones.size()) >= kMaxZones) return false;
        for (auto& z : m_zones) if (z.id == zone.id) return false;
        m_zones.push_back(zone);
        return true;
    }

    /// Remove a zone by ID.
    bool removeZone(const std::string& id) {
        for (auto it = m_zones.begin(); it != m_zones.end(); ++it) {
            if (it->id == id) {
                // Remove any connections referencing this zone
                auto cit = m_connections.begin();
                while (cit != m_connections.end()) {
                    if (cit->first == id || cit->second == id)
                        cit = m_connections.erase(cit);
                    else
                        ++cit;
                }
                m_zones.erase(it);
                return true;
            }
        }
        return false;
    }

    /// Find zone by ID (mutable).
    [[nodiscard]] HabitatZone* findZone(const std::string& id) {
        for (auto& z : m_zones) if (z.id == id) return &z;
        return nullptr;
    }

    /// Find zone by ID (const).
    [[nodiscard]] const HabitatZone* findZone(const std::string& id) const {
        for (auto& z : m_zones) if (z.id == id) return &z;
        return nullptr;
    }

    /// Connect two zones (bidirectional passage).
    bool connect(const std::string& a, const std::string& b) {
        if (!findZone(a) || !findZone(b)) return false;
        // Avoid duplicates
        for (auto& c : m_connections)
            if ((c.first == a && c.second == b) || (c.first == b && c.second == a))
                return false;
        m_connections.emplace_back(a, b);
        return true;
    }

    /// Count neighbors of a zone.
    [[nodiscard]] int neighborCount(const std::string& id) const {
        int n = 0;
        for (auto& c : m_connections)
            if (c.first == id || c.second == id) ++n;
        return n;
    }

    /// Get all zones connected to a given zone.
    [[nodiscard]] std::vector<std::string> neighbors(const std::string& id) const {
        std::vector<std::string> result;
        for (auto& c : m_connections) {
            if (c.first == id) result.push_back(c.second);
            else if (c.second == id) result.push_back(c.first);
        }
        return result;
    }

    /// Count habitable zones.
    [[nodiscard]] int habitableCount() const {
        int n = 0;
        for (auto& z : m_zones) if (z.isHabitable()) ++n;
        return n;
    }

    /// Total crew capacity across habitable zones.
    [[nodiscard]] int totalCapacity() const {
        int c = 0;
        for (auto& z : m_zones)
            if (z.isHabitable()) c += z.capacity;
        return c;
    }

    [[nodiscard]] const std::vector<HabitatZone>& zones() const { return m_zones; }
    [[nodiscard]] size_t zoneCount() const { return m_zones.size(); }
    void clear() { m_zones.clear(); m_connections.clear(); }

private:
    std::vector<HabitatZone> m_zones;
    std::vector<std::pair<std::string, std::string>> m_connections;
};

/// Central habitat system — manages habitats, life support, and atmosphere.
class HabitatSystem {
public:
    static constexpr int kMaxHabitats = 4;

    /// Create a new habitat. Returns index or -1 if at limit.
    int createHabitat(const std::string& name) {
        if (static_cast<int>(m_names.size()) >= kMaxHabitats) return -1;
        m_names.push_back(name);
        m_layouts.emplace_back();
        m_lifeSupportModules.emplace_back();
        return static_cast<int>(m_names.size()) - 1;
    }

    /// Add a life support module to a habitat.
    bool addLifeSupport(int habIdx, const LifeSupportModule& mod) {
        if (habIdx < 0 || habIdx >= static_cast<int>(m_lifeSupportModules.size()))
            return false;
        m_lifeSupportModules[static_cast<size_t>(habIdx)].push_back(mod);
        return true;
    }

    /// Access layout for a habitat.
    [[nodiscard]] HabitatLayout* layout(int index) {
        if (index < 0 || index >= static_cast<int>(m_layouts.size())) return nullptr;
        return &m_layouts[static_cast<size_t>(index)];
    }

    [[nodiscard]] const HabitatLayout* layout(int index) const {
        if (index < 0 || index >= static_cast<int>(m_layouts.size())) return nullptr;
        return &m_layouts[static_cast<size_t>(index)];
    }

    /// Get habitat name.
    [[nodiscard]] const std::string& habitatName(int index) const {
        static const std::string empty;
        if (index < 0 || index >= static_cast<int>(m_names.size())) return empty;
        return m_names[static_cast<size_t>(index)];
    }

    /// Simulate atmosphere tick for a habitat.
    /// Oxygen generation, CO₂ scrubbing, and temperature convergence.
    void tickAtmosphere(int habIdx, float dt) {
        if (habIdx < 0 || habIdx >= static_cast<int>(m_layouts.size())) return;

        auto& layout = m_layouts[static_cast<size_t>(habIdx)];
        auto& modules = m_lifeSupportModules[static_cast<size_t>(habIdx)];

        // Aggregate life support rates
        float totalO2Gen = 0.f;
        float totalTempTarget = 0.f;
        float totalTempRate = 0.f;
        int activeCount = 0;
        for (auto& mod : modules) {
            if (!mod.active) continue;
            totalO2Gen += mod.oxygenGenRate;
            totalTempTarget += mod.tempTarget;
            totalTempRate += mod.tempAdjustRate;
            ++activeCount;
        }
        if (activeCount > 0)
            totalTempTarget /= static_cast<float>(activeCount);

        // Apply to each sealed zone
        for (auto& zone : const_cast<std::vector<HabitatZone>&>(layout.zones())) {
            if (!zone.sealed) continue;

            // Oxygen: generate toward 1.0, reduced by occupants
            float o2Drain = static_cast<float>(zone.occupants) * 0.02f * dt;
            zone.oxygenLevel += (totalO2Gen * dt - o2Drain);
            zone.oxygenLevel = std::clamp(zone.oxygenLevel, 0.f, 1.f);

            // Temperature: converge toward target
            if (activeCount > 0) {
                float diff = totalTempTarget - zone.temperature;
                float adj = totalTempRate * dt;
                if (std::abs(diff) <= adj)
                    zone.temperature = totalTempTarget;
                else
                    zone.temperature += (diff > 0.f ? adj : -adj);
            }
        }
    }

    /// Breach a zone — sets sealed=false, vents atmosphere.
    bool breachZone(int habIdx, const std::string& zoneId) {
        auto* lay = layout(habIdx);
        if (!lay) return false;
        auto* zone = lay->findZone(zoneId);
        if (!zone) return false;
        zone->sealed = false;
        zone->oxygenLevel = 0.f;
        zone->pressure = 0.f;
        return true;
    }

    /// Repair a breach — re-seals the zone.
    bool repairBreach(int habIdx, const std::string& zoneId) {
        auto* lay = layout(habIdx);
        if (!lay) return false;
        auto* zone = lay->findZone(zoneId);
        if (!zone) return false;
        zone->sealed = true;
        zone->pressure = 1.f;
        return true;
    }

    /// Total life support power draw for a habitat.
    [[nodiscard]] float lifeSupportPowerDraw(int habIdx) const {
        if (habIdx < 0 || habIdx >= static_cast<int>(m_lifeSupportModules.size()))
            return 0.f;
        float total = 0.f;
        for (auto& mod : m_lifeSupportModules[static_cast<size_t>(habIdx)])
            if (mod.active) total += mod.powerDraw;
        return total;
    }

    [[nodiscard]] size_t habitatCount() const { return m_names.size(); }

private:
    std::vector<std::string>                    m_names;
    std::vector<HabitatLayout>                  m_layouts;
    std::vector<std::vector<LifeSupportModule>> m_lifeSupportModules;
};

// ---------------------------------------------------------------------------
// G26 — Power Grid System
// ---------------------------------------------------------------------------

enum class PowerSourceType : uint8_t {
    Solar, Nuclear, Fusion, Geothermal, Wind, Battery, FuelCell, Antimatter
};

inline const char* powerSourceTypeName(PowerSourceType t) {
    switch (t) {
        case PowerSourceType::Solar:       return "Solar";
        case PowerSourceType::Nuclear:     return "Nuclear";
        case PowerSourceType::Fusion:      return "Fusion";
        case PowerSourceType::Geothermal:  return "Geothermal";
        case PowerSourceType::Wind:        return "Wind";
        case PowerSourceType::Battery:     return "Battery";
        case PowerSourceType::FuelCell:    return "Fuel Cell";
        case PowerSourceType::Antimatter:  return "Antimatter";
    }
    return "Unknown";
}

struct PowerNode {
    std::string id;
    std::string name;
    PowerSourceType sourceType = PowerSourceType::Solar;
    float generationRate = 0.f;
    float consumptionRate = 0.f;
    int priority = 0;
    bool online = true;

    [[nodiscard]] float netPower() const { return online ? (generationRate - consumptionRate) : 0.f; }
    [[nodiscard]] bool isGenerator() const { return generationRate > consumptionRate; }
    [[nodiscard]] bool isConsumer() const { return consumptionRate > generationRate; }
};

struct PowerConduit {
    std::string id;
    std::string fromNodeId;
    std::string toNodeId;
    float maxCapacity = 100.f;
    float currentLoad = 0.f;
    float efficiency = 0.95f;
    bool active = true;

    [[nodiscard]] float availableCapacity() const { return active ? std::max(0.f, maxCapacity - currentLoad) : 0.f; }
    [[nodiscard]] float loadFraction() const { return maxCapacity > 0.f ? currentLoad / maxCapacity : 0.f; }
    [[nodiscard]] bool isOverloaded() const { return currentLoad > maxCapacity; }
};

class PowerGrid {
public:
    static constexpr size_t kMaxNodes = 64;
    static constexpr size_t kMaxConduits = 128;

    bool addNode(PowerNode node) {
        if (m_nodes.size() >= kMaxNodes) return false;
        for (const auto& n : m_nodes) {
            if (n.id == node.id) return false;
        }
        m_nodes.push_back(std::move(node));
        return true;
    }

    bool removeNode(const std::string& id) {
        auto it = std::find_if(m_nodes.begin(), m_nodes.end(),
                               [&](const PowerNode& n) { return n.id == id; });
        if (it == m_nodes.end()) return false;
        m_nodes.erase(it);
        m_conduits.erase(
            std::remove_if(m_conduits.begin(), m_conduits.end(),
                           [&](const PowerConduit& c) {
                               return c.fromNodeId == id || c.toNodeId == id;
                           }),
            m_conduits.end());
        return true;
    }

    [[nodiscard]] const PowerNode* findNode(const std::string& id) const {
        for (const auto& n : m_nodes) {
            if (n.id == id) return &n;
        }
        return nullptr;
    }

    PowerNode* findNode(const std::string& id) {
        for (auto& n : m_nodes) {
            if (n.id == id) return &n;
        }
        return nullptr;
    }

    bool addConduit(PowerConduit conduit) {
        if (m_conduits.size() >= kMaxConduits) return false;
        for (const auto& c : m_conduits) {
            if (c.id == conduit.id) return false;
        }
        m_conduits.push_back(std::move(conduit));
        return true;
    }

    bool removeConduit(const std::string& id) {
        auto it = std::find_if(m_conduits.begin(), m_conduits.end(),
                               [&](const PowerConduit& c) { return c.id == id; });
        if (it == m_conduits.end()) return false;
        m_conduits.erase(it);
        return true;
    }

    [[nodiscard]] const PowerConduit* findConduit(const std::string& id) const {
        for (const auto& c : m_conduits) {
            if (c.id == id) return &c;
        }
        return nullptr;
    }

    PowerConduit* findConduit(const std::string& id) {
        for (auto& c : m_conduits) {
            if (c.id == id) return &c;
        }
        return nullptr;
    }

    [[nodiscard]] float totalGeneration() const {
        float sum = 0.f;
        for (const auto& n : m_nodes) {
            if (n.online) sum += n.generationRate;
        }
        return sum;
    }

    [[nodiscard]] float totalConsumption() const {
        float sum = 0.f;
        for (const auto& n : m_nodes) {
            if (n.online) sum += n.consumptionRate;
        }
        return sum;
    }

    [[nodiscard]] float netPower() const { return totalGeneration() - totalConsumption(); }
    [[nodiscard]] bool isDeficit() const { return netPower() < 0.f; }
    [[nodiscard]] size_t nodeCount() const { return m_nodes.size(); }
    [[nodiscard]] size_t conduitCount() const { return m_conduits.size(); }
    [[nodiscard]] const std::vector<PowerNode>& nodes() const { return m_nodes; }
    [[nodiscard]] const std::vector<PowerConduit>& conduits() const { return m_conduits; }

    [[nodiscard]] size_t generatorCount() const {
        size_t count = 0;
        for (const auto& n : m_nodes) {
            if (n.isGenerator()) ++count;
        }
        return count;
    }

    [[nodiscard]] size_t consumerCount() const {
        size_t count = 0;
        for (const auto& n : m_nodes) {
            if (n.isConsumer()) ++count;
        }
        return count;
    }

    void clear() {
        m_nodes.clear();
        m_conduits.clear();
    }

    std::vector<PowerNode>& nodesMutable() { return m_nodes; }
    std::vector<PowerConduit>& conduitsMutable() { return m_conduits; }

private:
    std::vector<PowerNode> m_nodes;
    std::vector<PowerConduit> m_conduits;
};

class PowerGridSystem {
public:
    static constexpr size_t kMaxGrids = 8;

    int createGrid(const std::string& name) {
        if (m_grids.size() >= kMaxGrids) return -1;
        int idx = static_cast<int>(m_grids.size());
        m_grids.emplace_back();
        m_gridNames.push_back(name);
        return idx;
    }

    PowerGrid* grid(int index) {
        if (index < 0 || static_cast<size_t>(index) >= m_grids.size()) return nullptr;
        return &m_grids[static_cast<size_t>(index)];
    }

    [[nodiscard]] const PowerGrid* grid(int index) const {
        if (index < 0 || static_cast<size_t>(index) >= m_grids.size()) return nullptr;
        return &m_grids[static_cast<size_t>(index)];
    }

    [[nodiscard]] const std::string& gridName(int index) const {
        return m_gridNames.at(static_cast<size_t>(index));
    }

    [[nodiscard]] size_t gridCount() const { return m_grids.size(); }

    void tickDistribution(int gridIdx, float /*dt*/) {
        PowerGrid* g = grid(gridIdx);
        if (!g) return;

        for (auto& c : g->conduitsMutable()) {
            c.currentLoad = 0.f;
        }

        float totalGen = g->totalGeneration();
        float totalCons = g->totalConsumption();
        if (totalGen <= 0.f || totalCons <= 0.f) return;

        auto& conduits = g->conduitsMutable();
        for (size_t i = 0; i < conduits.size(); ++i) {
            if (!conduits[i].active) continue;
            const PowerNode* toNode = g->findNode(conduits[i].toNodeId);
            if (!toNode || !toNode->online || !toNode->isConsumer()) continue;

            float share = toNode->consumptionRate / totalCons;
            float load = share * std::min(totalGen, totalCons);
            load = std::min(load, conduits[i].maxCapacity * conduits[i].efficiency);
            conduits[i].currentLoad = load;
        }
    }

    int shedLoad(int gridIdx) {
        PowerGrid* g = grid(gridIdx);
        if (!g) return 0;

        struct ConsumerInfo {
            size_t index;
            int priority;
            std::string id;
        };
        std::vector<ConsumerInfo> consumers;
        const auto& nodes = g->nodes();
        for (size_t i = 0; i < nodes.size(); ++i) {
            if (nodes[i].isConsumer() && nodes[i].online) {
                consumers.push_back({i, nodes[i].priority, nodes[i].id});
            }
        }
        std::sort(consumers.begin(), consumers.end(),
                  [](const ConsumerInfo& a, const ConsumerInfo& b) {
                      return a.priority < b.priority;
                  });

        int shed = 0;
        for (const auto& ci : consumers) {
            if (g->netPower() >= 0.f) break;
            PowerNode* node = g->findNode(ci.id);
            if (node) {
                node->online = false;
                ++shed;
            }
        }
        return shed;
    }

    int restoreAll(int gridIdx) {
        PowerGrid* g = grid(gridIdx);
        if (!g) return 0;
        int restored = 0;
        for (auto& n : g->nodesMutable()) {
            if (!n.online) {
                n.online = true;
                ++restored;
            }
        }
        return restored;
    }

    [[nodiscard]] float totalSystemPower() const {
        float sum = 0.f;
        for (const auto& g : m_grids) {
            sum += g.netPower();
        }
        return sum;
    }

private:
    std::vector<PowerGrid>   m_grids;
    std::vector<std::string> m_gridNames;
};

// ---------------------------------------------------------------------------
// G27 — Vehicle System
// ---------------------------------------------------------------------------

enum class VehicleType : uint8_t {
    Rover, Hoverbike, Mech, Shuttle, Crawler, Speeder, Tank, Dropship
};

inline const char* vehicleTypeName(VehicleType t) {
    switch (t) {
        case VehicleType::Rover:     return "Rover";
        case VehicleType::Hoverbike: return "Hoverbike";
        case VehicleType::Mech:      return "Mech";
        case VehicleType::Shuttle:   return "Shuttle";
        case VehicleType::Crawler:   return "Crawler";
        case VehicleType::Speeder:   return "Speeder";
        case VehicleType::Tank:      return "Tank";
        case VehicleType::Dropship:  return "Dropship";
    }
    return "Unknown";
}

struct VehicleSeat {
    std::string id;
    std::string label;
    bool isDriver = false;
    bool occupied = false;
    std::string occupantId;

    void enter(const std::string& entityId) {
        occupantId = entityId;
        occupied = true;
    }

    void exit() {
        occupantId.clear();
        occupied = false;
    }
};

struct VehicleComponent {
    std::string id;
    std::string name;
    float health = 100.f;
    float maxHealth = 100.f;
    bool functional = true;

    [[nodiscard]] float healthFraction() const { return maxHealth > 0.f ? health / maxHealth : 0.f; }

    bool applyDamage(float amount) {
        health = std::max(0.f, health - amount);
        if (health <= 0.f) functional = false;
        return functional;
    }

    void repair(float amount) {
        health = std::min(maxHealth, health + amount);
        if (health > 0.f) functional = true;
    }

    [[nodiscard]] bool isDestroyed() const { return health <= 0.f; }
};

class Vehicle {
public:
    void setId(const std::string& vid) { m_id = vid; }
    [[nodiscard]] const std::string& id() const { return m_id; }

    void setName(const std::string& n) { m_name = n; }
    [[nodiscard]] const std::string& name() const { return m_name; }

    void setType(VehicleType type) { m_type = type; }
    [[nodiscard]] VehicleType type() const { return m_type; }

    // --- Seats (max 8) ---
    static constexpr size_t kMaxSeats = 8;

    bool addSeat(const VehicleSeat& seat) {
        if (m_seats.size() >= kMaxSeats) return false;
        for (const auto& s : m_seats) {
            if (s.id == seat.id) return false;
        }
        m_seats.push_back(seat);
        return true;
    }

    bool removeSeat(const std::string& sid) {
        for (auto it = m_seats.begin(); it != m_seats.end(); ++it) {
            if (it->id == sid) { m_seats.erase(it); return true; }
        }
        return false;
    }

    VehicleSeat* findSeat(const std::string& sid) {
        for (auto& s : m_seats) { if (s.id == sid) return &s; }
        return nullptr;
    }

    const VehicleSeat* findSeat(const std::string& sid) const {
        for (const auto& s : m_seats) { if (s.id == sid) return &s; }
        return nullptr;
    }

    [[nodiscard]] size_t seatCount() const { return m_seats.size(); }
    [[nodiscard]] const std::vector<VehicleSeat>& seats() const { return m_seats; }

    [[nodiscard]] size_t occupantCount() const {
        size_t count = 0;
        for (const auto& s : m_seats) { if (s.occupied) ++count; }
        return count;
    }

    [[nodiscard]] bool hasDriver() const {
        for (const auto& s : m_seats) {
            if (s.isDriver && s.occupied) return true;
        }
        return false;
    }

    // --- Components (max 16) ---
    static constexpr size_t kMaxComponents = 16;

    bool addComponent(const VehicleComponent& comp) {
        if (m_components.size() >= kMaxComponents) return false;
        for (const auto& c : m_components) {
            if (c.id == comp.id) return false;
        }
        m_components.push_back(comp);
        return true;
    }

    bool removeComponent(const std::string& cid) {
        for (auto it = m_components.begin(); it != m_components.end(); ++it) {
            if (it->id == cid) { m_components.erase(it); return true; }
        }
        return false;
    }

    VehicleComponent* findComponent(const std::string& cid) {
        for (auto& c : m_components) { if (c.id == cid) return &c; }
        return nullptr;
    }

    const VehicleComponent* findComponent(const std::string& cid) const {
        for (const auto& c : m_components) { if (c.id == cid) return &c; }
        return nullptr;
    }

    [[nodiscard]] size_t componentCount() const { return m_components.size(); }
    [[nodiscard]] const std::vector<VehicleComponent>& components() const { return m_components; }
    std::vector<VehicleComponent>& componentsMutable() { return m_components; }

    [[nodiscard]] bool isOperational() const {
        for (const auto& c : m_components) {
            if (!c.functional) return false;
        }
        return true;
    }

    // --- Fuel ---
    void setFuel(float fuel) { m_fuel = std::max(0.f, fuel); }
    void setMaxFuel(float maxFuel) { m_maxFuel = std::max(0.f, maxFuel); }
    [[nodiscard]] float fuel() const { return m_fuel; }
    [[nodiscard]] float maxFuel() const { return m_maxFuel; }
    [[nodiscard]] float fuelFraction() const { return m_maxFuel > 0.f ? m_fuel / m_maxFuel : 0.f; }
    void consumeFuel(float amount) { m_fuel = std::max(0.f, m_fuel - amount); }
    [[nodiscard]] bool hasFuel() const { return m_fuel > 0.f; }

    // --- Speed / physics ---
    void setSpeed(float speed) { m_speed = speed; }
    [[nodiscard]] float speed() const { return m_speed; }
    void setMaxSpeed(float maxSpeed) { m_maxSpeed = maxSpeed; }
    [[nodiscard]] float maxSpeed() const { return m_maxSpeed; }
    [[nodiscard]] float speedFraction() const { return m_maxSpeed > 0.f ? m_speed / m_maxSpeed : 0.f; }

    // --- Position ---
    void setPosition(const Vec3& pos) { m_position = pos; }
    [[nodiscard]] const Vec3& position() const { return m_position; }

    // --- Engine ---
    void setEngineActive(bool active) { m_engineActive = active; }
    [[nodiscard]] bool engineActive() const { return m_engineActive; }

    [[nodiscard]] bool canOperate() const {
        return m_engineActive && hasFuel() && isOperational() && hasDriver();
    }

private:
    std::string m_id;
    std::string m_name;
    VehicleType m_type = VehicleType::Rover;
    std::vector<VehicleSeat> m_seats;
    std::vector<VehicleComponent> m_components;
    float m_fuel = 100.f;
    float m_maxFuel = 100.f;
    float m_speed = 0.f;
    float m_maxSpeed = 50.f;
    Vec3 m_position;
    bool m_engineActive = false;
};

class VehicleSystem {
public:
    static constexpr size_t kMaxVehicles = 16;

    int createVehicle(const std::string& vname, VehicleType type) {
        if (m_vehicles.size() >= kMaxVehicles) return -1;
        int idx = static_cast<int>(m_vehicles.size());
        Vehicle v;
        v.setId("vehicle_" + std::to_string(idx));
        v.setName(vname);
        v.setType(type);
        m_vehicles.push_back(std::move(v));
        return idx;
    }

    Vehicle* vehicle(int index) {
        if (index < 0 || static_cast<size_t>(index) >= m_vehicles.size()) return nullptr;
        return &m_vehicles[static_cast<size_t>(index)];
    }

    const Vehicle* vehicle(int index) const {
        if (index < 0 || static_cast<size_t>(index) >= m_vehicles.size()) return nullptr;
        return &m_vehicles[static_cast<size_t>(index)];
    }

    [[nodiscard]] size_t vehicleCount() const { return m_vehicles.size(); }

    void tickVehicle(int index, float dt) {
        auto* v = vehicle(index);
        if (!v || !v->canOperate()) return;
        float fuelRate = 0.5f * (v->speed() / std::max(1.f, v->maxSpeed()));
        v->consumeFuel(fuelRate * dt);
        Vec3 pos = v->position();
        pos.x += v->speed() * dt;
        v->setPosition(pos);
    }

    int applyDamage(int index, float amount) {
        auto* v = vehicle(index);
        if (!v) return 0;
        int damaged = 0;
        for (auto& c : v->componentsMutable()) {
            c.applyDamage(amount);
            ++damaged;
        }
        return damaged;
    }

    int repairAll(int index, float amount) {
        auto* v = vehicle(index);
        if (!v) return 0;
        int repaired = 0;
        for (auto& c : v->componentsMutable()) {
            c.repair(amount);
            ++repaired;
        }
        return repaired;
    }

    [[nodiscard]] size_t operationalCount() const {
        size_t count = 0;
        for (const auto& v : m_vehicles) {
            if (v.isOperational()) ++count;
        }
        return count;
    }

private:
    std::vector<Vehicle> m_vehicles;
};

// ── G28 — Research System ────────────────────────────────────────

enum class ResearchCategory : uint8_t {
    Physics, Biology, Engineering, Computing, Materials, Energy, Weapons, Xenotech
};

inline const char* researchCategoryName(ResearchCategory c) {
    switch (c) {
        case ResearchCategory::Physics:     return "Physics";
        case ResearchCategory::Biology:     return "Biology";
        case ResearchCategory::Engineering: return "Engineering";
        case ResearchCategory::Computing:   return "Computing";
        case ResearchCategory::Materials:   return "Materials";
        case ResearchCategory::Energy:      return "Energy";
        case ResearchCategory::Weapons:     return "Weapons";
        case ResearchCategory::Xenotech:    return "Xenotech";
    }
    return "Unknown";
}

struct ResearchProject {
    std::string id;
    std::string name;
    ResearchCategory category = ResearchCategory::Physics;
    float cost = 100.f;
    float progress = 0.f;
    float durationSeconds = 60.f;
    std::vector<std::string> prerequisites;
    bool completed = false;

    [[nodiscard]] float progressFraction() const {
        return cost > 0.f ? std::min(1.f, progress / cost) : 0.f;
    }

    [[nodiscard]] bool isComplete() const {
        return completed || progress >= cost;
    }

    void addProgress(float points) {
        progress = std::min(cost, progress + points);
        if (progress >= cost) completed = true;
    }
};

class ResearchLab {
public:
    void setId(const std::string& id) { m_id = id; }
    [[nodiscard]] const std::string& id() const { return m_id; }

    void setName(const std::string& name) { m_name = name; }
    [[nodiscard]] const std::string& name() const { return m_name; }

    void setResearchRate(float rate) { m_researchRate = std::max(0.f, rate); }
    [[nodiscard]] float researchRate() const { return m_researchRate; }

    bool assignProject(const std::string& projectId) {
        if (m_activeProjectId == projectId) return false;
        m_activeProjectId = projectId;
        return true;
    }

    void clearProject() { m_activeProjectId.clear(); }

    [[nodiscard]] const std::string& activeProjectId() const { return m_activeProjectId; }
    [[nodiscard]] bool hasActiveProject() const { return !m_activeProjectId.empty(); }

    void markCompleted(const std::string& projectId) {
        m_completedProjects.push_back(projectId);
    }

    [[nodiscard]] size_t completedCount() const { return m_completedProjects.size(); }

    [[nodiscard]] bool hasCompleted(const std::string& projectId) const {
        for (const auto& p : m_completedProjects)
            if (p == projectId) return true;
        return false;
    }

    [[nodiscard]] const std::vector<std::string>& completedProjects() const { return m_completedProjects; }

    void setBudget(float budget) { m_budget = std::max(0.f, budget); }
    [[nodiscard]] float budget() const { return m_budget; }
    void spendBudget(float amount) { m_budget = std::max(0.f, m_budget - amount); }
    [[nodiscard]] bool hasBudget() const { return m_budget > 0.f; }

private:
    std::string m_id;
    std::string m_name;
    float m_researchRate = 1.f;
    std::string m_activeProjectId;
    std::vector<std::string> m_completedProjects;
    float m_budget = 1000.f;
};

class ResearchTree {
public:
    bool addProject(const ResearchProject& project) {
        if (m_projects.size() >= kMaxProjects) return false;
        for (const auto& p : m_projects) {
            if (p.id == project.id) return false;
        }
        m_projects.push_back(project);
        return true;
    }

    bool removeProject(const std::string& id) {
        auto it = std::find_if(m_projects.begin(), m_projects.end(),
            [&id](const ResearchProject& p) { return p.id == id; });
        if (it == m_projects.end()) return false;
        m_projects.erase(it);
        return true;
    }

    ResearchProject* findProject(const std::string& id) {
        for (auto& p : m_projects) if (p.id == id) return &p;
        return nullptr;
    }

    const ResearchProject* findProject(const std::string& id) const {
        for (const auto& p : m_projects) if (p.id == id) return &p;
        return nullptr;
    }

    [[nodiscard]] size_t projectCount() const { return m_projects.size(); }
    [[nodiscard]] const std::vector<ResearchProject>& projects() const { return m_projects; }

    [[nodiscard]] bool prerequisitesMet(const std::string& projectId, const std::vector<std::string>& completed) const {
        const auto* proj = findProject(projectId);
        if (!proj) return false;
        for (const auto& prereq : proj->prerequisites) {
            bool found = false;
            for (const auto& c : completed) {
                if (c == prereq) { found = true; break; }
            }
            if (!found) return false;
        }
        return true;
    }

    [[nodiscard]] std::vector<const ResearchProject*> projectsInCategory(ResearchCategory cat) const {
        std::vector<const ResearchProject*> result;
        for (const auto& p : m_projects) {
            if (p.category == cat) result.push_back(&p);
        }
        return result;
    }

    [[nodiscard]] size_t completedCount() const {
        size_t c = 0;
        for (const auto& p : m_projects) if (p.isComplete()) ++c;
        return c;
    }

    void clear() { m_projects.clear(); }

    static constexpr size_t kMaxProjects = 128;

private:
    std::vector<ResearchProject> m_projects;
};

class ResearchSystem {
public:
    int createLab(const std::string& name) {
        if (m_labs.size() >= kMaxLabs) return -1;
        ResearchLab lab;
        lab.setId("lab_" + std::to_string(m_labs.size()));
        lab.setName(name);
        m_labs.push_back(std::move(lab));
        return static_cast<int>(m_labs.size() - 1);
    }

    ResearchLab* lab(int index) {
        if (index < 0 || index >= static_cast<int>(m_labs.size())) return nullptr;
        return &m_labs[static_cast<size_t>(index)];
    }

    const ResearchLab* lab(int index) const {
        if (index < 0 || index >= static_cast<int>(m_labs.size())) return nullptr;
        return &m_labs[static_cast<size_t>(index)];
    }

    [[nodiscard]] size_t labCount() const { return m_labs.size(); }

    ResearchTree& tree() { return m_tree; }
    const ResearchTree& tree() const { return m_tree; }

    void tick(float dt) {
        for (auto& lab : m_labs) {
            if (!lab.hasActiveProject() || !lab.hasBudget()) continue;

            ResearchProject* proj = m_tree.findProject(lab.activeProjectId());
            if (!proj || proj->isComplete()) {
                if (proj && proj->isComplete()) {
                    lab.markCompleted(proj->id);
                    lab.clearProject();
                }
                continue;
            }

            float points = lab.researchRate() * dt;
            float budgetCost = points * 0.1f;
            if (lab.budget() < budgetCost) continue;

            proj->addProgress(points);
            lab.spendBudget(budgetCost);

            if (proj->isComplete()) {
                lab.markCompleted(proj->id);
                lab.clearProject();
                ++m_discoveries;
            }
        }
    }

    bool assignProject(int labIndex, const std::string& projectId) {
        auto* l = lab(labIndex);
        if (!l) return false;

        const auto* proj = m_tree.findProject(projectId);
        if (!proj || proj->isComplete()) return false;

        std::vector<std::string> allCompleted;
        for (const auto& lab : m_labs) {
            for (const auto& c : lab.completedProjects())
                allCompleted.push_back(c);
        }

        if (!m_tree.prerequisitesMet(projectId, allCompleted)) return false;

        return l->assignProject(projectId);
    }

    [[nodiscard]] size_t discoveries() const { return m_discoveries; }

    [[nodiscard]] size_t activeLabCount() const {
        size_t count = 0;
        for (const auto& l : m_labs) {
            if (l.hasActiveProject()) ++count;
        }
        return count;
    }

    static constexpr size_t kMaxLabs = 8;

private:
    std::vector<ResearchLab> m_labs;
    ResearchTree m_tree;
    size_t m_discoveries = 0;
};

// ── G29 — Diplomacy System ──────────────────────────────────────

enum class DiplomacyAction : uint8_t {
    TradeAgreement,
    NonAggression,
    MilitaryAlliance,
    TechSharing,
    TerritoryExchange,
    Embargo,
    WarDeclaration,
    PeaceTreaty
};

inline const char* diplomacyActionName(DiplomacyAction a) noexcept {
    switch (a) {
        case DiplomacyAction::TradeAgreement:    return "TradeAgreement";
        case DiplomacyAction::NonAggression:     return "NonAggression";
        case DiplomacyAction::MilitaryAlliance:  return "MilitaryAlliance";
        case DiplomacyAction::TechSharing:       return "TechSharing";
        case DiplomacyAction::TerritoryExchange: return "TerritoryExchange";
        case DiplomacyAction::Embargo:           return "Embargo";
        case DiplomacyAction::WarDeclaration:    return "WarDeclaration";
        case DiplomacyAction::PeaceTreaty:       return "PeaceTreaty";
        default:                                  return "Unknown";
    }
}

enum class DiplomaticStance : uint8_t {
    Hostile,
    Unfriendly,
    Neutral,
    Friendly,
    Allied
};

inline const char* diplomaticStanceName(DiplomaticStance s) noexcept {
    switch (s) {
        case DiplomaticStance::Hostile:    return "Hostile";
        case DiplomaticStance::Unfriendly: return "Unfriendly";
        case DiplomaticStance::Neutral:    return "Neutral";
        case DiplomaticStance::Friendly:   return "Friendly";
        case DiplomaticStance::Allied:     return "Allied";
        default:                            return "Unknown";
    }
}

struct DiplomaticRelation {
    std::string factionA;
    std::string factionB;
    float opinion = 0.f;
    DiplomaticStance stance = DiplomaticStance::Neutral;
    size_t treatiesCount = 0;
    bool atWar = false;

    void adjustOpinion(float delta) {
        opinion = std::max(-100.f, std::min(100.f, opinion + delta));
        updateStance();
    }

    void updateStance() {
        if (opinion <= -51.f)       stance = DiplomaticStance::Hostile;
        else if (opinion <= -1.f)   stance = DiplomaticStance::Unfriendly;
        else if (opinion <= 24.f)   stance = DiplomaticStance::Neutral;
        else if (opinion <= 74.f)   stance = DiplomaticStance::Friendly;
        else                        stance = DiplomaticStance::Allied;
    }

    void declareWar() { atWar = true; opinion = -100.f; stance = DiplomaticStance::Hostile; }
    void declarePeace() { atWar = false; adjustOpinion(20.f); }

    [[nodiscard]] bool isHostile() const { return stance == DiplomaticStance::Hostile; }
    [[nodiscard]] bool isAllied() const { return stance == DiplomaticStance::Allied; }
};

struct Treaty {
    std::string id;
    DiplomacyAction action;
    std::string factionA;
    std::string factionB;
    float durationSeconds = 0.f;
    float elapsedSeconds = 0.f;
    bool active = true;
    bool expired = false;

    void tick(float dt) {
        if (!active || expired) return;
        elapsedSeconds += dt;
        if (durationSeconds > 0.f && elapsedSeconds >= durationSeconds) {
            expired = true;
            active = false;
        }
    }

    [[nodiscard]] bool isActive() const { return active && !expired; }
    [[nodiscard]] float remainingSeconds() const {
        if (durationSeconds <= 0.f) return -1.f;
        return std::max(0.f, durationSeconds - elapsedSeconds);
    }
    [[nodiscard]] bool isPermanent() const { return durationSeconds <= 0.f; }
    void revoke() { active = false; }
};

class DiplomaticChannel {
public:
    explicit DiplomaticChannel(const std::string& ownerFaction)
        : m_owner(ownerFaction) {}

    bool addRelation(const std::string& otherFaction, float initialOpinion = 0.f) {
        if (otherFaction == m_owner || m_relations.size() >= kMaxRelations) return false;
        for (const auto& r : m_relations) {
            if (r.factionB == otherFaction) return false;
        }
        DiplomaticRelation rel;
        rel.factionA = m_owner;
        rel.factionB = otherFaction;
        rel.adjustOpinion(initialOpinion);
        m_relations.push_back(rel);
        return true;
    }

    [[nodiscard]] DiplomaticRelation* findRelation(const std::string& otherFaction) {
        for (auto& r : m_relations) {
            if (r.factionB == otherFaction) return &r;
        }
        return nullptr;
    }

    [[nodiscard]] const DiplomaticRelation* findRelation(const std::string& otherFaction) const {
        for (const auto& r : m_relations) {
            if (r.factionB == otherFaction) return &r;
        }
        return nullptr;
    }

    bool proposeTreaty(const std::string& otherFaction, DiplomacyAction action,
                       float duration = 0.f) {
        auto* rel = findRelation(otherFaction);
        if (!rel) return false;
        if (rel->atWar && action != DiplomacyAction::PeaceTreaty) return false;
        if (m_treaties.size() >= kMaxTreaties) return false;

        Treaty treaty;
        treaty.id = "treaty_" + std::to_string(m_nextTreatyId++);
        treaty.action = action;
        treaty.factionA = m_owner;
        treaty.factionB = otherFaction;
        treaty.durationSeconds = duration;
        m_treaties.push_back(treaty);
        rel->treatiesCount++;

        switch (action) {
            case DiplomacyAction::TradeAgreement:    rel->adjustOpinion(10.f); break;
            case DiplomacyAction::NonAggression:     rel->adjustOpinion(15.f); break;
            case DiplomacyAction::MilitaryAlliance:  rel->adjustOpinion(25.f); break;
            case DiplomacyAction::TechSharing:       rel->adjustOpinion(10.f); break;
            case DiplomacyAction::TerritoryExchange: rel->adjustOpinion(5.f);  break;
            case DiplomacyAction::Embargo:           rel->adjustOpinion(-20.f); break;
            case DiplomacyAction::WarDeclaration:    rel->declareWar(); break;
            case DiplomacyAction::PeaceTreaty:       rel->declarePeace(); break;
        }
        return true;
    }

    void tickTreaties(float dt) {
        for (auto& t : m_treaties) t.tick(dt);
    }

    [[nodiscard]] size_t activeTreatyCount() const {
        size_t count = 0;
        for (const auto& t : m_treaties) {
            if (t.isActive()) ++count;
        }
        return count;
    }

    [[nodiscard]] size_t relationCount() const { return m_relations.size(); }
    [[nodiscard]] const std::string& owner() const { return m_owner; }
    [[nodiscard]] const std::vector<DiplomaticRelation>& relations() const { return m_relations; }
    [[nodiscard]] const std::vector<Treaty>& treaties() const { return m_treaties; }

    [[nodiscard]] size_t alliedCount() const {
        size_t count = 0;
        for (const auto& r : m_relations) {
            if (r.isAllied()) ++count;
        }
        return count;
    }

    [[nodiscard]] size_t hostileCount() const {
        size_t count = 0;
        for (const auto& r : m_relations) {
            if (r.isHostile()) ++count;
        }
        return count;
    }

    static constexpr size_t kMaxRelations = 32;
    static constexpr size_t kMaxTreaties = 64;

private:
    std::string m_owner;
    std::vector<DiplomaticRelation> m_relations;
    std::vector<Treaty> m_treaties;
    size_t m_nextTreatyId = 1;
};

class DiplomacySystem {
public:
    int createChannel(const std::string& factionName) {
        if (m_channels.size() >= kMaxChannels) return -1;
        for (const auto& ch : m_channels) {
            if (ch.owner() == factionName) return -1;
        }
        m_channels.emplace_back(factionName);
        return static_cast<int>(m_channels.size()) - 1;
    }

    [[nodiscard]] DiplomaticChannel* channel(int index) {
        if (index < 0 || index >= static_cast<int>(m_channels.size())) return nullptr;
        return &m_channels[static_cast<size_t>(index)];
    }

    [[nodiscard]] DiplomaticChannel* channelByName(const std::string& factionName) {
        for (auto& ch : m_channels) {
            if (ch.owner() == factionName) return &ch;
        }
        return nullptr;
    }

    bool establishRelation(const std::string& factionA, const std::string& factionB,
                           float initialOpinion = 0.f) {
        auto* chA = channelByName(factionA);
        auto* chB = channelByName(factionB);
        if (!chA || !chB) return false;
        bool a = chA->addRelation(factionB, initialOpinion);
        bool b = chB->addRelation(factionA, initialOpinion);
        return a && b;
    }

    bool proposeTreaty(const std::string& from, const std::string& to,
                       DiplomacyAction action, float duration = 0.f) {
        auto* ch = channelByName(from);
        if (!ch) return false;
        return ch->proposeTreaty(to, action, duration);
    }

    void tick(float dt) {
        for (auto& ch : m_channels) ch.tickTreaties(dt);
        m_tickCount++;
    }

    [[nodiscard]] size_t channelCount() const { return m_channels.size(); }
    [[nodiscard]] size_t tickCount() const { return m_tickCount; }

    [[nodiscard]] size_t totalActiveTreaties() const {
        size_t count = 0;
        for (const auto& ch : m_channels) count += ch.activeTreatyCount();
        return count;
    }

    [[nodiscard]] size_t totalAlliances() const {
        size_t count = 0;
        for (const auto& ch : m_channels) count += ch.alliedCount();
        return count;
    }

    static constexpr size_t kMaxChannels = 16;

private:
    std::vector<DiplomaticChannel> m_channels;
    size_t m_tickCount = 0;
};

// ── G30 — Espionage System ───────────────────────────────────────

enum class EspionageMissionType : uint8_t {
    Infiltration,
    Sabotage,
    Surveillance,
    DataTheft,
    Assassination,
    Recruitment,
    CounterIntel,
    Extraction
};

inline const char* espionageMissionTypeName(EspionageMissionType t) noexcept {
    switch (t) {
        case EspionageMissionType::Infiltration: return "Infiltration";
        case EspionageMissionType::Sabotage:     return "Sabotage";
        case EspionageMissionType::Surveillance:  return "Surveillance";
        case EspionageMissionType::DataTheft:    return "DataTheft";
        case EspionageMissionType::Assassination: return "Assassination";
        case EspionageMissionType::Recruitment:  return "Recruitment";
        case EspionageMissionType::CounterIntel: return "CounterIntel";
        case EspionageMissionType::Extraction:   return "Extraction";
        default:                                  return "Unknown";
    }
}

struct SpyAgent {
    std::string id;
    std::string name;
    float skillLevel = 1.f;
    float loyalty = 1.f;
    float coverStrength = 1.f;
    size_t missionsCompleted = 0;
    bool deployed = false;
    bool compromised = false;
    bool captured = false;

    [[nodiscard]] bool isAvailable() const { return !deployed && !compromised && !captured; }
    [[nodiscard]] bool isActive() const { return deployed && !compromised && !captured; }
    void deploy() { deployed = true; }
    void recall() { deployed = false; }
    void compromise() { compromised = true; coverStrength = 0.f; }
    void capture() { captured = true; deployed = false; }
    void rescue() { captured = false; compromised = false; coverStrength = 0.3f; }
};

struct EspionageMission {
    std::string id;
    EspionageMissionType type = EspionageMissionType::Infiltration;
    std::string targetFaction;
    std::string agentId;
    float difficulty = 1.f;
    float progress = 0.f;
    float durationSeconds = 0.f;
    float elapsedSeconds = 0.f;
    bool completed = false;
    bool failed = false;

    [[nodiscard]] float progressFraction() const {
        return durationSeconds > 0.f ? std::min(1.f, elapsedSeconds / durationSeconds) : 0.f;
    }

    [[nodiscard]] bool isComplete() const { return completed; }
    [[nodiscard]] bool isFailed() const { return failed; }
    [[nodiscard]] bool isInProgress() const { return !completed && !failed && elapsedSeconds > 0.f; }

    void advance(float dt) {
        if (completed || failed) return;
        elapsedSeconds += dt;
        progress = progressFraction();
        if (durationSeconds > 0.f && elapsedSeconds >= durationSeconds) {
            completed = true;
            progress = 1.f;
        }
    }

    void fail() { failed = true; }
};

class IntelligenceNetwork {
public:
    explicit IntelligenceNetwork(const std::string& ownerFaction)
        : m_owner(ownerFaction) {}

    bool addAgent(const SpyAgent& agent) {
        if (m_agents.size() >= kMaxAgents) return false;
        for (const auto& a : m_agents) {
            if (a.id == agent.id) return false;
        }
        m_agents.push_back(agent);
        return true;
    }

    bool removeAgent(const std::string& agentId) {
        for (auto it = m_agents.begin(); it != m_agents.end(); ++it) {
            if (it->id == agentId && !it->deployed) {
                m_agents.erase(it);
                return true;
            }
        }
        return false;
    }

    [[nodiscard]] SpyAgent* findAgent(const std::string& agentId) {
        for (auto& a : m_agents) {
            if (a.id == agentId) return &a;
        }
        return nullptr;
    }

    [[nodiscard]] const SpyAgent* findAgent(const std::string& agentId) const {
        for (const auto& a : m_agents) {
            if (a.id == agentId) return &a;
        }
        return nullptr;
    }

    bool launchMission(const std::string& agentId, EspionageMissionType type,
                       const std::string& targetFaction, float duration) {
        auto* agent = findAgent(agentId);
        if (!agent || !agent->isAvailable()) return false;
        if (m_missions.size() >= kMaxMissions) return false;

        EspionageMission mission;
        mission.id = "mission_" + std::to_string(m_nextMissionId++);
        mission.type = type;
        mission.targetFaction = targetFaction;
        mission.agentId = agentId;
        mission.difficulty = 1.f;
        mission.durationSeconds = duration;
        agent->deploy();
        m_missions.push_back(mission);
        return true;
    }

    void tick(float dt) {
        for (auto& mission : m_missions) {
            if (mission.isInProgress() || (!mission.completed && !mission.failed && mission.elapsedSeconds == 0.f)) {
                mission.advance(dt);
                if (mission.isComplete()) {
                    auto* agent = findAgent(mission.agentId);
                    if (agent) {
                        agent->recall();
                        agent->missionsCompleted++;
                        m_intelGathered++;
                    }
                }
            }
        }
    }

    [[nodiscard]] size_t agentCount() const { return m_agents.size(); }
    [[nodiscard]] size_t missionCount() const { return m_missions.size(); }
    [[nodiscard]] size_t intelGathered() const { return m_intelGathered; }
    [[nodiscard]] const std::string& owner() const { return m_owner; }
    [[nodiscard]] const std::vector<SpyAgent>& agents() const { return m_agents; }
    [[nodiscard]] const std::vector<EspionageMission>& missions() const { return m_missions; }

    [[nodiscard]] size_t availableAgentCount() const {
        size_t count = 0;
        for (const auto& a : m_agents) {
            if (a.isAvailable()) ++count;
        }
        return count;
    }

    [[nodiscard]] size_t activeMissionCount() const {
        size_t count = 0;
        for (const auto& m : m_missions) {
            if (m.isInProgress() || (!m.completed && !m.failed && m.elapsedSeconds == 0.f)) ++count;
        }
        return count;
    }

    [[nodiscard]] size_t completedMissionCount() const {
        size_t count = 0;
        for (const auto& m : m_missions) {
            if (m.isComplete()) ++count;
        }
        return count;
    }

    static constexpr size_t kMaxAgents = 16;
    static constexpr size_t kMaxMissions = 64;

private:
    std::string m_owner;
    std::vector<SpyAgent> m_agents;
    std::vector<EspionageMission> m_missions;
    size_t m_nextMissionId = 1;
    size_t m_intelGathered = 0;
};

class EspionageSystem {
public:
    int createNetwork(const std::string& factionName) {
        if (m_networks.size() >= kMaxNetworks) return -1;
        for (const auto& n : m_networks) {
            if (n.owner() == factionName) return -1;
        }
        m_networks.emplace_back(factionName);
        return static_cast<int>(m_networks.size()) - 1;
    }

    [[nodiscard]] IntelligenceNetwork* network(int index) {
        if (index < 0 || index >= static_cast<int>(m_networks.size())) return nullptr;
        return &m_networks[static_cast<size_t>(index)];
    }

    [[nodiscard]] IntelligenceNetwork* networkByName(const std::string& factionName) {
        for (auto& n : m_networks) {
            if (n.owner() == factionName) return &n;
        }
        return nullptr;
    }

    bool recruitAgent(const std::string& factionName, const SpyAgent& agent) {
        auto* net = networkByName(factionName);
        if (!net) return false;
        return net->addAgent(agent);
    }

    bool launchMission(const std::string& factionName, const std::string& agentId,
                       EspionageMissionType type, const std::string& targetFaction,
                       float duration) {
        auto* net = networkByName(factionName);
        if (!net) return false;
        return net->launchMission(agentId, type, targetFaction, duration);
    }

    void tick(float dt) {
        for (auto& n : m_networks) n.tick(dt);
        m_tickCount++;
    }

    [[nodiscard]] size_t networkCount() const { return m_networks.size(); }
    [[nodiscard]] size_t tickCount() const { return m_tickCount; }

    [[nodiscard]] size_t totalActiveMissions() const {
        size_t count = 0;
        for (const auto& n : m_networks) count += n.activeMissionCount();
        return count;
    }

    [[nodiscard]] size_t totalIntelGathered() const {
        size_t count = 0;
        for (const auto& n : m_networks) count += n.intelGathered();
        return count;
    }

    static constexpr size_t kMaxNetworks = 8;

private:
    std::vector<IntelligenceNetwork> m_networks;
    size_t m_tickCount = 0;
};

// ── G31 — Colony Management ─────────────────────────────────────

enum class ColonyRole : uint8_t {
    Governor,
    Engineer,
    Scientist,
    Miner,
    Farmer,
    Guard,
    Medic,
    Explorer
};

inline const char* colonyRoleName(ColonyRole r) noexcept {
    switch (r) {
        case ColonyRole::Governor:  return "Governor";
        case ColonyRole::Engineer:  return "Engineer";
        case ColonyRole::Scientist: return "Scientist";
        case ColonyRole::Miner:     return "Miner";
        case ColonyRole::Farmer:    return "Farmer";
        case ColonyRole::Guard:     return "Guard";
        case ColonyRole::Medic:     return "Medic";
        case ColonyRole::Explorer:  return "Explorer";
        default:                    return "Unknown";
    }
}

struct Colonist {
    std::string id;
    std::string name;
    ColonyRole role = ColonyRole::Farmer;
    float morale = 1.f;
    float health = 1.f;
    float productivity = 1.f;
    bool assigned = false;

    [[nodiscard]] bool isHealthy() const { return health > 0.25f; }
    [[nodiscard]] bool isProductive() const { return isHealthy() && morale > 0.25f; }
    [[nodiscard]] float effectiveOutput() const { return productivity * morale * health; }
    void takeDamage(float amount) { health = std::max(0.f, health - amount); }
    void heal(float amount) { health = std::min(1.f, health + amount); }
    void boostMorale(float amount) { morale = std::min(1.f, morale + amount); }
    void reduceMorale(float amount) { morale = std::max(0.f, morale - amount); }
};

struct ColonyBuilding {
    std::string id;
    std::string name;
    size_t capacity = 0;
    size_t occupants = 0;
    float operationalLevel = 1.f;
    bool powered = true;
    bool damaged = false;

    [[nodiscard]] bool isOperational() const { return powered && !damaged && operationalLevel > 0.f; }
    [[nodiscard]] bool isFull() const { return occupants >= capacity; }
    [[nodiscard]] size_t availableSlots() const { return capacity > occupants ? capacity - occupants : 0; }
    void damage() { damaged = true; operationalLevel *= 0.5f; }
    void repair() { damaged = false; operationalLevel = 1.f; }
};

class Colony {
public:
    explicit Colony(const std::string& colonyName)
        : m_name(colonyName) {}

    bool addColonist(const Colonist& colonist) {
        if (m_colonists.size() >= kMaxColonists) return false;
        for (const auto& c : m_colonists) {
            if (c.id == colonist.id) return false;
        }
        m_colonists.push_back(colonist);
        return true;
    }

    bool removeColonist(const std::string& colonistId) {
        for (auto it = m_colonists.begin(); it != m_colonists.end(); ++it) {
            if (it->id == colonistId) {
                m_colonists.erase(it);
                return true;
            }
        }
        return false;
    }

    [[nodiscard]] Colonist* findColonist(const std::string& colonistId) {
        for (auto& c : m_colonists) {
            if (c.id == colonistId) return &c;
        }
        return nullptr;
    }

    [[nodiscard]] const Colonist* findColonist(const std::string& colonistId) const {
        for (const auto& c : m_colonists) {
            if (c.id == colonistId) return &c;
        }
        return nullptr;
    }

    bool addBuilding(const ColonyBuilding& building) {
        if (m_buildings.size() >= kMaxBuildings) return false;
        for (const auto& b : m_buildings) {
            if (b.id == building.id) return false;
        }
        m_buildings.push_back(building);
        return true;
    }

    bool removeBuilding(const std::string& buildingId) {
        for (auto it = m_buildings.begin(); it != m_buildings.end(); ++it) {
            if (it->id == buildingId && it->occupants == 0) {
                m_buildings.erase(it);
                return true;
            }
        }
        return false;
    }

    [[nodiscard]] ColonyBuilding* findBuilding(const std::string& buildingId) {
        for (auto& b : m_buildings) {
            if (b.id == buildingId) return &b;
        }
        return nullptr;
    }

    void tick(float /*dt*/) {
        float totalMorale = 0.f;
        size_t productiveCount = 0;
        for (const auto& c : m_colonists) {
            totalMorale += c.morale;
            if (c.isProductive()) ++productiveCount;
        }
        m_avgMorale = m_colonists.empty() ? 0.f : totalMorale / static_cast<float>(m_colonists.size());
        m_productiveCount = productiveCount;

        float totalOutput = 0.f;
        for (const auto& c : m_colonists) {
            if (c.isProductive()) totalOutput += c.effectiveOutput();
        }
        m_resourceOutput += totalOutput;
    }

    [[nodiscard]] size_t population() const { return m_colonists.size(); }
    [[nodiscard]] size_t buildingCount() const { return m_buildings.size(); }
    [[nodiscard]] float averageMorale() const { return m_avgMorale; }
    [[nodiscard]] size_t productiveCount() const { return m_productiveCount; }
    [[nodiscard]] float resourceOutput() const { return m_resourceOutput; }
    [[nodiscard]] const std::string& name() const { return m_name; }
    [[nodiscard]] const std::vector<Colonist>& colonists() const { return m_colonists; }
    [[nodiscard]] const std::vector<ColonyBuilding>& buildings() const { return m_buildings; }

    [[nodiscard]] size_t operationalBuildingCount() const {
        size_t count = 0;
        for (const auto& b : m_buildings) {
            if (b.isOperational()) ++count;
        }
        return count;
    }

    [[nodiscard]] size_t colonistsWithRole(ColonyRole role) const {
        size_t count = 0;
        for (const auto& c : m_colonists) {
            if (c.role == role) ++count;
        }
        return count;
    }

    static constexpr size_t kMaxColonists = 128;
    static constexpr size_t kMaxBuildings = 64;

private:
    std::string m_name;
    std::vector<Colonist> m_colonists;
    std::vector<ColonyBuilding> m_buildings;
    float m_avgMorale = 0.f;
    size_t m_productiveCount = 0;
    float m_resourceOutput = 0.f;
};

class ColonySystem {
public:
    int createColony(const std::string& colonyName) {
        if (m_colonies.size() >= kMaxColonies) return -1;
        for (const auto& c : m_colonies) {
            if (c.name() == colonyName) return -1;
        }
        m_colonies.emplace_back(colonyName);
        return static_cast<int>(m_colonies.size()) - 1;
    }

    [[nodiscard]] Colony* colony(int index) {
        if (index < 0 || index >= static_cast<int>(m_colonies.size())) return nullptr;
        return &m_colonies[static_cast<size_t>(index)];
    }

    [[nodiscard]] Colony* colonyByName(const std::string& name) {
        for (auto& c : m_colonies) {
            if (c.name() == name) return &c;
        }
        return nullptr;
    }

    bool addColonistToColony(const std::string& colonyName, const Colonist& colonist) {
        auto* col = colonyByName(colonyName);
        if (!col) return false;
        return col->addColonist(colonist);
    }

    void tick(float dt) {
        for (auto& c : m_colonies) c.tick(dt);
        m_tickCount++;
    }

    [[nodiscard]] size_t colonyCount() const { return m_colonies.size(); }
    [[nodiscard]] size_t tickCount() const { return m_tickCount; }

    [[nodiscard]] size_t totalPopulation() const {
        size_t count = 0;
        for (const auto& c : m_colonies) count += c.population();
        return count;
    }

    [[nodiscard]] size_t totalBuildings() const {
        size_t count = 0;
        for (const auto& c : m_colonies) count += c.buildingCount();
        return count;
    }

    [[nodiscard]] float totalResourceOutput() const {
        float total = 0.f;
        for (const auto& c : m_colonies) total += c.resourceOutput();
        return total;
    }

    static constexpr size_t kMaxColonies = 16;

private:
    std::vector<Colony> m_colonies;
    size_t m_tickCount = 0;
};

// ── G32 — Archaeology System ────────────────────────────────────

enum class ArtifactRarity : uint8_t {
    Common,
    Uncommon,
    Rare,
    Epic,
    Legendary,
    Ancient,
    Mythic,
    Unique
};

inline const char* artifactRarityName(ArtifactRarity r) noexcept {
    switch (r) {
        case ArtifactRarity::Common:    return "Common";
        case ArtifactRarity::Uncommon:  return "Uncommon";
        case ArtifactRarity::Rare:      return "Rare";
        case ArtifactRarity::Epic:      return "Epic";
        case ArtifactRarity::Legendary: return "Legendary";
        case ArtifactRarity::Ancient:   return "Ancient";
        case ArtifactRarity::Mythic:    return "Mythic";
        case ArtifactRarity::Unique:    return "Unique";
        default:                        return "Unknown";
    }
}

struct Artifact {
    std::string id;
    std::string name;
    ArtifactRarity rarity = ArtifactRarity::Common;
    std::string origin;
    float researchProgress = 0.f;
    bool decoded = false;

    [[nodiscard]] bool isDecoded() const { return decoded; }
    [[nodiscard]] bool isResearching() const { return researchProgress > 0.f && !decoded; }
    [[nodiscard]] float progressFraction() const { return std::min(1.f, researchProgress); }

    void advanceResearch(float amount) {
        if (decoded) return;
        researchProgress = std::min(1.f, researchProgress + amount);
        if (researchProgress >= 1.f) decoded = true;
    }
};

struct ExcavationSite {
    std::string id;
    std::string location;
    float difficulty = 1.f;
    float progress = 0.f;
    float durationSeconds = 0.f;
    float elapsedSeconds = 0.f;
    size_t artifactsFound = 0;
    bool completed = false;
    bool active = false;

    [[nodiscard]] float progressFraction() const {
        return durationSeconds > 0.f ? std::min(1.f, elapsedSeconds / durationSeconds) : 0.f;
    }

    [[nodiscard]] bool isActive() const { return active && !completed; }
    [[nodiscard]] bool isComplete() const { return completed; }

    void activate() { active = true; }

    void advance(float dt) {
        if (!active || completed) return;
        elapsedSeconds += dt;
        progress = progressFraction();
        if (durationSeconds > 0.f && elapsedSeconds >= durationSeconds) {
            completed = true;
            progress = 1.f;
        }
    }
};

class ArtifactCollection {
public:
    explicit ArtifactCollection(const std::string& ownerName)
        : m_owner(ownerName) {}

    bool addArtifact(const Artifact& artifact) {
        if (m_artifacts.size() >= kMaxArtifacts) return false;
        for (const auto& a : m_artifacts) {
            if (a.id == artifact.id) return false;
        }
        m_artifacts.push_back(artifact);
        return true;
    }

    bool removeArtifact(const std::string& artifactId) {
        for (auto it = m_artifacts.begin(); it != m_artifacts.end(); ++it) {
            if (it->id == artifactId) {
                m_artifacts.erase(it);
                return true;
            }
        }
        return false;
    }

    [[nodiscard]] Artifact* findArtifact(const std::string& artifactId) {
        for (auto& a : m_artifacts) {
            if (a.id == artifactId) return &a;
        }
        return nullptr;
    }

    [[nodiscard]] const Artifact* findArtifact(const std::string& artifactId) const {
        for (const auto& a : m_artifacts) {
            if (a.id == artifactId) return &a;
        }
        return nullptr;
    }

    [[nodiscard]] size_t artifactCount() const { return m_artifacts.size(); }
    [[nodiscard]] const std::string& owner() const { return m_owner; }
    [[nodiscard]] const std::vector<Artifact>& artifacts() const { return m_artifacts; }

    [[nodiscard]] size_t decodedCount() const {
        size_t count = 0;
        for (const auto& a : m_artifacts) {
            if (a.isDecoded()) ++count;
        }
        return count;
    }

    [[nodiscard]] size_t rarityCount(ArtifactRarity rarity) const {
        size_t count = 0;
        for (const auto& a : m_artifacts) {
            if (a.rarity == rarity) ++count;
        }
        return count;
    }

    static constexpr size_t kMaxArtifacts = 256;

private:
    std::string m_owner;
    std::vector<Artifact> m_artifacts;
};

class ArchaeologySystem {
public:
    int createSite(const std::string& siteId, const std::string& location, float duration) {
        if (m_sites.size() >= kMaxSites) return -1;
        for (const auto& s : m_sites) {
            if (s.id == siteId) return -1;
        }
        ExcavationSite site;
        site.id = siteId;
        site.location = location;
        site.durationSeconds = duration;
        m_sites.push_back(site);
        return static_cast<int>(m_sites.size()) - 1;
    }

    bool activateSite(const std::string& siteId) {
        for (auto& s : m_sites) {
            if (s.id == siteId && !s.active) {
                s.activate();
                return true;
            }
        }
        return false;
    }

    [[nodiscard]] ExcavationSite* findSite(const std::string& siteId) {
        for (auto& s : m_sites) {
            if (s.id == siteId) return &s;
        }
        return nullptr;
    }

    bool addCollection(const std::string& ownerName) {
        if (m_collections.size() >= kMaxCollections) return false;
        for (const auto& c : m_collections) {
            if (c.owner() == ownerName) return false;
        }
        m_collections.emplace_back(ownerName);
        return true;
    }

    [[nodiscard]] ArtifactCollection* collectionByOwner(const std::string& ownerName) {
        for (auto& c : m_collections) {
            if (c.owner() == ownerName) return &c;
        }
        return nullptr;
    }

    void tick(float dt) {
        for (auto& s : m_sites) {
            bool wasDone = s.isComplete();
            s.advance(dt);
            if (!wasDone && s.isComplete()) {
                s.artifactsFound++;
                m_totalArtifactsFound++;
            }
        }
        m_tickCount++;
    }

    [[nodiscard]] size_t siteCount() const { return m_sites.size(); }
    [[nodiscard]] size_t collectionCount() const { return m_collections.size(); }
    [[nodiscard]] size_t tickCount() const { return m_tickCount; }
    [[nodiscard]] size_t totalArtifactsFound() const { return m_totalArtifactsFound; }
    [[nodiscard]] const std::vector<ExcavationSite>& sites() const { return m_sites; }

    [[nodiscard]] size_t activeSiteCount() const {
        size_t count = 0;
        for (const auto& s : m_sites) {
            if (s.isActive()) ++count;
        }
        return count;
    }

    [[nodiscard]] size_t completedSiteCount() const {
        size_t count = 0;
        for (const auto& s : m_sites) {
            if (s.isComplete()) ++count;
        }
        return count;
    }

    [[nodiscard]] size_t totalDecodedArtifacts() const {
        size_t count = 0;
        for (const auto& c : m_collections) count += c.decodedCount();
        return count;
    }

    static constexpr size_t kMaxSites = 32;
    static constexpr size_t kMaxCollections = 8;

private:
    std::vector<ExcavationSite> m_sites;
    std::vector<ArtifactCollection> m_collections;
    size_t m_tickCount = 0;
    size_t m_totalArtifactsFound = 0;
};

// ─────────────────────────────────────────────────────────────────────────────
// G33 — Migration System
// ─────────────────────────────────────────────────────────────────────────────

enum class MigrationTrigger : uint8_t {
    Economic = 0,
    Environmental,
    Political,
    Cultural,
    War,
    Famine,
    Disease,
    Opportunity
};

inline const char* migrationTriggerName(MigrationTrigger t) {
    switch (t) {
        case MigrationTrigger::Economic:      return "Economic";
        case MigrationTrigger::Environmental: return "Environmental";
        case MigrationTrigger::Political:     return "Political";
        case MigrationTrigger::Cultural:      return "Cultural";
        case MigrationTrigger::War:           return "War";
        case MigrationTrigger::Famine:        return "Famine";
        case MigrationTrigger::Disease:       return "Disease";
        case MigrationTrigger::Opportunity:   return "Opportunity";
        default:                              return "Unknown";
    }
}

struct Migrant {
    std::string id;
    std::string name;
    std::string originRegion;
    std::string destinationRegion;
    MigrationTrigger trigger = MigrationTrigger::Economic;
    float journeyProgress = 0.f; // 0..1
    bool arrived = false;

    [[nodiscard]] bool isInTransit() const { return !arrived && journeyProgress > 0.f; }
    [[nodiscard]] bool hasArrived()  const { return arrived; }

    void advance(float amount) {
        if (arrived) return;
        journeyProgress += amount;
        if (journeyProgress >= 1.f) {
            journeyProgress = 1.f;
            arrived = true;
        }
    }
};

struct MigrationWave {
    std::string id;
    std::string originRegion;
    std::string destinationRegion;
    MigrationTrigger trigger = MigrationTrigger::Economic;
    size_t totalMigrants  = 0;
    size_t arrivedCount   = 0;
    float  speed          = 0.1f; // progress per tick

    [[nodiscard]] bool isComplete() const { return totalMigrants > 0 && arrivedCount >= totalMigrants; }
    [[nodiscard]] float completionFraction() const {
        return totalMigrants > 0 ? static_cast<float>(arrivedCount) / static_cast<float>(totalMigrants) : 0.f;
    }
};

class MigrationRoute {
public:
    static constexpr size_t kMaxMigrants = 256;

    explicit MigrationRoute(const std::string& origin, const std::string& destination)
        : m_origin(origin), m_destination(destination) {}

    [[nodiscard]] const std::string& origin()      const { return m_origin; }
    [[nodiscard]] const std::string& destination() const { return m_destination; }

    bool addMigrant(const Migrant& migrant) {
        if (m_migrants.size() >= kMaxMigrants) return false;
        for (const auto& m : m_migrants) {
            if (m.id == migrant.id) return false;
        }
        m_migrants.push_back(migrant);
        return true;
    }

    bool removeMigrant(const std::string& migrantId) {
        for (auto it = m_migrants.begin(); it != m_migrants.end(); ++it) {
            if (it->id == migrantId) { m_migrants.erase(it); return true; }
        }
        return false;
    }

    [[nodiscard]] Migrant* findMigrant(const std::string& migrantId) {
        for (auto& m : m_migrants) { if (m.id == migrantId) return &m; }
        return nullptr;
    }

    [[nodiscard]] size_t migrantCount()  const { return m_migrants.size(); }
    [[nodiscard]] size_t arrivedCount()  const {
        size_t count = 0;
        for (const auto& m : m_migrants) { if (m.arrived) ++count; }
        return count;
    }
    [[nodiscard]] size_t inTransitCount() const {
        size_t count = 0;
        for (const auto& m : m_migrants) { if (m.isInTransit()) ++count; }
        return count;
    }

    void tick(float dt) {
        for (auto& m : m_migrants) m.advance(dt * 0.1f);
        m_tickCount++;
    }

    [[nodiscard]] size_t tickCount() const { return m_tickCount; }

private:
    std::string            m_origin;
    std::string            m_destination;
    std::vector<Migrant>   m_migrants;
    size_t                 m_tickCount = 0;
};

class MigrationSystem {
public:
    static constexpr size_t kMaxRoutes = 32;
    static constexpr size_t kMaxWaves  = 16;

    int createRoute(const std::string& origin, const std::string& destination) {
        if (m_routes.size() >= kMaxRoutes) return -1;
        for (const auto& r : m_routes) {
            if (r.origin() == origin && r.destination() == destination) return -1;
        }
        m_routes.emplace_back(origin, destination);
        return static_cast<int>(m_routes.size()) - 1;
    }

    [[nodiscard]] MigrationRoute* route(int index) {
        if (index < 0 || index >= static_cast<int>(m_routes.size())) return nullptr;
        return &m_routes[static_cast<size_t>(index)];
    }

    [[nodiscard]] MigrationRoute* routeByEndpoints(const std::string& origin, const std::string& destination) {
        for (auto& r : m_routes) {
            if (r.origin() == origin && r.destination() == destination) return &r;
        }
        return nullptr;
    }

    bool addWave(const MigrationWave& wave) {
        if (m_waves.size() >= kMaxWaves) return false;
        for (const auto& w : m_waves) {
            if (w.id == wave.id) return false;
        }
        m_waves.push_back(wave);
        return true;
    }

    [[nodiscard]] MigrationWave* waveById(const std::string& id) {
        for (auto& w : m_waves) { if (w.id == id) return &w; }
        return nullptr;
    }

    void tick(float dt) {
        for (auto& r : m_routes) r.tick(dt);
        for (auto& w : m_waves) {
            if (!w.isComplete()) {
                w.arrivedCount += static_cast<size_t>(w.speed * dt);
                if (w.arrivedCount > w.totalMigrants) w.arrivedCount = w.totalMigrants;
            }
        }
        m_tickCount++;
    }

    [[nodiscard]] size_t routeCount() const { return m_routes.size(); }
    [[nodiscard]] size_t waveCount()  const { return m_waves.size(); }
    [[nodiscard]] size_t tickCount()  const { return m_tickCount; }

    [[nodiscard]] size_t completedWaveCount() const {
        size_t count = 0;
        for (const auto& w : m_waves) { if (w.isComplete()) ++count; }
        return count;
    }

    [[nodiscard]] size_t totalMigrantsInTransit() const {
        size_t count = 0;
        for (const auto& r : m_routes) count += r.inTransitCount();
        return count;
    }

private:
    std::vector<MigrationRoute> m_routes;
    std::vector<MigrationWave>  m_waves;
    size_t                      m_tickCount = 0;
};

// ─────────────────────────────────────────────────────────────────────────────
// G34 — Insurgency System
// ─────────────────────────────────────────────────────────────────────────────

enum class InsurgencyType : uint8_t {
    Political   = 0,
    Religious   = 1,
    Economic    = 2,
    Military    = 3,
    Cultural    = 4,
    Ecological  = 5,
    Corporate   = 6,
    Territorial = 7
};

inline const char* insurgencyTypeName(InsurgencyType t) {
    switch (t) {
        case InsurgencyType::Political:   return "Political";
        case InsurgencyType::Religious:   return "Religious";
        case InsurgencyType::Economic:    return "Economic";
        case InsurgencyType::Military:    return "Military";
        case InsurgencyType::Cultural:    return "Cultural";
        case InsurgencyType::Ecological:  return "Ecological";
        case InsurgencyType::Corporate:   return "Corporate";
        case InsurgencyType::Territorial: return "Territorial";
        default:                          return "Unknown";
    }
}

enum class InsurgentStatus : uint8_t {
    Active      = 0,
    Captured    = 1,
    Eliminated  = 2,
    Underground = 3
};

struct Insurgent {
    std::string     id;
    std::string     name;
    InsurgencyType  type      = InsurgencyType::Political;
    InsurgentStatus status    = InsurgentStatus::Active;
    float           loyalty   = 1.f;
    float           influence = 0.f;

    [[nodiscard]] bool isActive()      const { return status == InsurgentStatus::Active; }
    [[nodiscard]] bool isCaptured()    const { return status == InsurgentStatus::Captured; }
    [[nodiscard]] bool isEliminated()  const { return status == InsurgentStatus::Eliminated; }
    [[nodiscard]] bool isUnderground() const { return status == InsurgentStatus::Underground; }

    bool capture() {
        if (status != InsurgentStatus::Active && status != InsurgentStatus::Underground) return false;
        status = InsurgentStatus::Captured;
        return true;
    }

    bool eliminate() {
        if (status == InsurgentStatus::Eliminated) return false;
        status = InsurgentStatus::Eliminated;
        return true;
    }

    bool goUnderground() {
        if (status != InsurgentStatus::Active) return false;
        status = InsurgentStatus::Underground;
        return true;
    }
};

struct InsurgencyCell {
    std::string    id;
    std::string    region;
    InsurgencyType type             = InsurgencyType::Political;
    size_t         memberCount      = 0;
    float          resourcePool     = 0.f;
    float          operationalLevel = 0.f;

    [[nodiscard]] bool  isOperational() const { return operationalLevel > 0.f && memberCount > 0; }
    [[nodiscard]] bool  isFunded()      const { return resourcePool > 0.f; }
    [[nodiscard]] float totalStrength() const { return static_cast<float>(memberCount) * operationalLevel; }

    void addMembers(size_t count) { memberCount += count; }
    bool removeMembers(size_t count) {
        if (count > memberCount) return false;
        memberCount -= count;
        return true;
    }

    void addResources(float amount)  { if (amount > 0.f) resourcePool += amount; }
    bool drainResources(float amount) {
        if (amount > resourcePool) return false;
        resourcePool -= amount;
        return true;
    }
};

class InsurgencyMovement {
public:
    static constexpr size_t kMaxCells = 32;

    explicit InsurgencyMovement(const std::string& name) : m_name(name) {}

    [[nodiscard]] const std::string& name() const { return m_name; }

    bool addCell(const InsurgencyCell& cell) {
        if (m_cells.size() >= kMaxCells) return false;
        for (const auto& c : m_cells) { if (c.id == cell.id) return false; }
        m_cells.push_back(cell);
        return true;
    }

    bool removeCell(const std::string& cellId) {
        for (auto it = m_cells.begin(); it != m_cells.end(); ++it) {
            if (it->id == cellId) { m_cells.erase(it); return true; }
        }
        return false;
    }

    [[nodiscard]] InsurgencyCell* findCell(const std::string& cellId) {
        for (auto& c : m_cells) { if (c.id == cellId) return &c; }
        return nullptr;
    }

    [[nodiscard]] size_t cellCount()       const { return m_cells.size(); }
    [[nodiscard]] size_t activeCellCount() const {
        size_t count = 0;
        for (const auto& c : m_cells) { if (c.isOperational()) ++count; }
        return count;
    }
    [[nodiscard]] size_t totalMembers() const {
        size_t total = 0;
        for (const auto& c : m_cells) { total += c.memberCount; }
        return total;
    }

    void tick(float /*dt*/) { m_tickCount++; }
    [[nodiscard]] size_t tickCount() const { return m_tickCount; }

private:
    std::string                 m_name;
    std::vector<InsurgencyCell> m_cells;
    size_t                      m_tickCount = 0;
};

class InsurgencySystem {
public:
    static constexpr size_t kMaxMovements  = 8;
    static constexpr size_t kMaxInsurgents = 256;

    InsurgencyMovement* createMovement(const std::string& name) {
        if (m_movements.size() >= kMaxMovements) return nullptr;
        for (const auto& mv : m_movements) { if (mv.name() == name) return nullptr; }
        m_movements.emplace_back(name);
        return &m_movements.back();
    }

    [[nodiscard]] InsurgencyMovement* movementByName(const std::string& name) {
        for (auto& mv : m_movements) { if (mv.name() == name) return &mv; }
        return nullptr;
    }

    bool addInsurgent(const Insurgent& insurgent) {
        if (m_insurgents.size() >= kMaxInsurgents) return false;
        for (const auto& i : m_insurgents) { if (i.id == insurgent.id) return false; }
        m_insurgents.push_back(insurgent);
        return true;
    }

    [[nodiscard]] Insurgent* findInsurgent(const std::string& id) {
        for (auto& i : m_insurgents) { if (i.id == id) return &i; }
        return nullptr;
    }

    void tick(float dt) {
        for (auto& mv : m_movements) mv.tick(dt);
        m_tickCount++;
    }

    [[nodiscard]] size_t movementCount()       const { return m_movements.size(); }
    [[nodiscard]] size_t totalInsurgentCount() const { return m_insurgents.size(); }
    [[nodiscard]] size_t tickCount()           const { return m_tickCount; }

    [[nodiscard]] size_t activeInsurgentCount() const {
        size_t count = 0;
        for (const auto& i : m_insurgents) { if (i.isActive()) ++count; }
        return count;
    }

    [[nodiscard]] size_t capturedInsurgentCount() const {
        size_t count = 0;
        for (const auto& i : m_insurgents) { if (i.isCaptured()) ++count; }
        return count;
    }

    [[nodiscard]] size_t totalCells() const {
        size_t count = 0;
        for (const auto& mv : m_movements) count += mv.cellCount();
        return count;
    }

private:
    std::vector<InsurgencyMovement> m_movements;
    std::vector<Insurgent>          m_insurgents;
    size_t                          m_tickCount = 0;
};

// ─────────────────────────────────────────────────────────────────────────────
// G35 — Plague System
// ─────────────────────────────────────────────────────────────────────────────

enum class PlagueType : uint8_t {
    Bacterial = 0,
    Viral     = 1,
    Fungal    = 2,
    Parasitic = 3,
    Prion     = 4,
    Genetic   = 5,
    Chemical  = 6,
    Radiation = 7
};

inline const char* plagueTypeName(PlagueType t) {
    switch (t) {
        case PlagueType::Bacterial: return "Bacterial";
        case PlagueType::Viral:     return "Viral";
        case PlagueType::Fungal:    return "Fungal";
        case PlagueType::Parasitic: return "Parasitic";
        case PlagueType::Prion:     return "Prion";
        case PlagueType::Genetic:   return "Genetic";
        case PlagueType::Chemical:  return "Chemical";
        case PlagueType::Radiation: return "Radiation";
        default:                    return "Unknown";
    }
}

enum class InfectionStatus : uint8_t {
    Healthy    = 0,
    Exposed    = 1,
    Infected   = 2,
    Recovering = 3,
    Immune     = 4
};

struct PlagueCarrier {
    std::string     id;
    std::string     name;
    InfectionStatus status        = InfectionStatus::Healthy;
    float           infectivity   = 0.f;  // 0–1
    float           immunity      = 0.f;  // 0–1
    int             daysInfected  = 0;

    [[nodiscard]] bool isHealthy()    const { return status == InfectionStatus::Healthy; }
    [[nodiscard]] bool isExposed()    const { return status == InfectionStatus::Exposed; }
    [[nodiscard]] bool isInfected()   const { return status == InfectionStatus::Infected; }
    [[nodiscard]] bool isRecovering() const { return status == InfectionStatus::Recovering; }
    [[nodiscard]] bool isImmune()     const { return status == InfectionStatus::Immune; }

    bool expose() {
        if (status != InfectionStatus::Healthy) return false;
        if (immunity >= 1.f) return false; // fully immune, cannot be exposed
        status = InfectionStatus::Exposed;
        return true;
    }

    bool infect() {
        if (status != InfectionStatus::Exposed) return false;
        status = InfectionStatus::Infected;
        return true;
    }

    bool recover() {
        if (status != InfectionStatus::Infected && status != InfectionStatus::Recovering) return false;
        status = InfectionStatus::Recovering;
        return true;
    }

    bool becomeImmune() {
        if (status == InfectionStatus::Healthy || status == InfectionStatus::Recovering
            || status == InfectionStatus::Immune) {
            status = InfectionStatus::Immune;
            immunity = 1.f;
            return true;
        }
        return false;
    }
};

struct PlagueStat {
    std::string id;
    std::string region;
    PlagueType  type            = PlagueType::Viral;
    float       transmissionRate = 0.f;  // R0 factor
    float       mortalityRate    = 0.f;  // 0–1
    float       incubationDays   = 0.f;
    bool        contained        = false;

    [[nodiscard]] bool isLethal()    const { return mortalityRate > 0.f; }
    [[nodiscard]] bool isContained() const { return contained; }
    [[nodiscard]] bool isSpreading() const { return !contained && transmissionRate > 1.f; }

    void contain()  { contained = true; }
    void release()  { contained = false; }
};

class PlagueRegion {
public:
    static constexpr size_t kMaxCarriers = 512;

    explicit PlagueRegion(const std::string& name) : m_name(name) {}

    [[nodiscard]] const std::string& name() const { return m_name; }

    bool addCarrier(const PlagueCarrier& carrier) {
        if (m_carriers.size() >= kMaxCarriers) return false;
        for (const auto& c : m_carriers) { if (c.id == carrier.id) return false; }
        m_carriers.push_back(carrier);
        return true;
    }

    bool removeCarrier(const std::string& carrierId) {
        for (auto it = m_carriers.begin(); it != m_carriers.end(); ++it) {
            if (it->id == carrierId) { m_carriers.erase(it); return true; }
        }
        return false;
    }

    [[nodiscard]] PlagueCarrier* findCarrier(const std::string& carrierId) {
        for (auto& c : m_carriers) { if (c.id == carrierId) return &c; }
        return nullptr;
    }

    [[nodiscard]] size_t carrierCount()   const { return m_carriers.size(); }
    [[nodiscard]] size_t infectedCount()  const {
        size_t n = 0;
        for (const auto& c : m_carriers) { if (c.isInfected()) ++n; }
        return n;
    }
    [[nodiscard]] size_t immuneCount() const {
        size_t n = 0;
        for (const auto& c : m_carriers) { if (c.isImmune()) ++n; }
        return n;
    }
    [[nodiscard]] size_t healthyCount() const {
        size_t n = 0;
        for (const auto& c : m_carriers) { if (c.isHealthy()) ++n; }
        return n;
    }

    void tick(float /*dt*/) { m_tickCount++; }
    [[nodiscard]] size_t tickCount() const { return m_tickCount; }

private:
    std::string               m_name;
    std::vector<PlagueCarrier> m_carriers;
    size_t                    m_tickCount = 0;
};

class PlagueSystem {
public:
    static constexpr size_t kMaxRegions = 32;
    static constexpr size_t kMaxPlagueStats = 16;

    PlagueRegion* createRegion(const std::string& name) {
        if (m_regions.size() >= kMaxRegions) return nullptr;
        for (const auto& r : m_regions) { if (r.name() == name) return nullptr; }
        m_regions.emplace_back(name);
        return &m_regions.back();
    }

    [[nodiscard]] PlagueRegion* regionByName(const std::string& name) {
        for (auto& r : m_regions) { if (r.name() == name) return &r; }
        return nullptr;
    }

    bool addPlagueStat(const PlagueStat& stat) {
        if (m_plagues.size() >= kMaxPlagueStats) return false;
        for (const auto& p : m_plagues) { if (p.id == stat.id) return false; }
        m_plagues.push_back(stat);
        return true;
    }

    [[nodiscard]] PlagueStat* findPlague(const std::string& id) {
        for (auto& p : m_plagues) { if (p.id == id) return &p; }
        return nullptr;
    }

    void tick(float dt) {
        for (auto& r : m_regions) r.tick(dt);
        m_tickCount++;
    }

    [[nodiscard]] size_t regionCount()     const { return m_regions.size(); }
    [[nodiscard]] size_t plagueCount()     const { return m_plagues.size(); }
    [[nodiscard]] size_t tickCount()       const { return m_tickCount; }

    [[nodiscard]] size_t totalInfected() const {
        size_t n = 0;
        for (const auto& r : m_regions) n += r.infectedCount();
        return n;
    }

    [[nodiscard]] size_t totalImmune() const {
        size_t n = 0;
        for (const auto& r : m_regions) n += r.immuneCount();
        return n;
    }

    [[nodiscard]] size_t activePlagueCount() const {
        size_t n = 0;
        for (const auto& p : m_plagues) { if (!p.isContained()) ++n; }
        return n;
    }

private:
    std::vector<PlagueRegion> m_regions;
    std::vector<PlagueStat>   m_plagues;
    size_t                    m_tickCount = 0;
};

// ============================================================
// G36 — Famine System
// ============================================================

enum class FamineType : uint8_t {
    Drought   = 0,
    Blight    = 1,
    Flood     = 2,
    Pest      = 3,
    War       = 4,
    Blockade  = 5,
    Economic  = 6,
    Climate   = 7,
};

inline const char* famineTypeName(FamineType t) {
    switch (t) {
        case FamineType::Drought:  return "Drought";
        case FamineType::Blight:   return "Blight";
        case FamineType::Flood:    return "Flood";
        case FamineType::Pest:     return "Pest";
        case FamineType::War:      return "War";
        case FamineType::Blockade: return "Blockade";
        case FamineType::Economic: return "Economic";
        case FamineType::Climate:  return "Climate";
        default:                   return "Unknown";
    }
}

enum class FamineSeverity : uint8_t {
    None         = 0,
    Mild         = 1,
    Moderate     = 2,
    Severe       = 3,
    Catastrophic = 4,
};

struct FamineEvent {
    std::string    id;
    std::string    region;
    FamineType     type     = FamineType::Drought;
    FamineSeverity severity = FamineSeverity::None;
    float          duration = 0.f;
    bool           resolved = false;

    [[nodiscard]] bool isActive()   const { return !resolved; }
    [[nodiscard]] bool isCritical() const { return severity >= FamineSeverity::Severe; }

    void resolve() { resolved = true; }
    void advanceDuration(float dt) { if (!resolved) duration += dt; }
};

class FamineRegion {
public:
    explicit FamineRegion(std::string name, float pop = 1000.f, float food = 500.f, float rate = 1.f)
        : m_name(std::move(name)), m_population(pop), m_foodSupply(food), m_consumptionRate(rate) {}

    [[nodiscard]] const std::string& name()            const { return m_name; }
    [[nodiscard]] float              population()      const { return m_population; }
    [[nodiscard]] float              foodSupply()      const { return m_foodSupply; }
    [[nodiscard]] float              consumptionRate() const { return m_consumptionRate; }
    [[nodiscard]] size_t             tickCount()       const { return m_tickCount; }

    [[nodiscard]] FamineSeverity severity() const {
        if (m_population <= 0.f) return FamineSeverity::None;
        float ratio = m_foodSupply / m_population;
        if (ratio >= 0.5f)  return FamineSeverity::None;
        if (ratio >= 0.35f) return FamineSeverity::Mild;
        if (ratio >= 0.2f)  return FamineSeverity::Moderate;
        if (ratio >= 0.05f) return FamineSeverity::Severe;
        return FamineSeverity::Catastrophic;
    }

    void addAid(float amount) {
        if (amount > 0.f) m_foodSupply += amount;
    }

    void tick(float dt) {
        float consumed = m_consumptionRate * m_population * dt;
        m_foodSupply -= consumed;
        if (m_foodSupply < 0.f) m_foodSupply = 0.f;
        m_tickCount++;
    }

private:
    std::string m_name;
    float       m_population;
    float       m_foodSupply;
    float       m_consumptionRate;
    size_t      m_tickCount = 0;
};

class FamineSystem {
public:
    static constexpr size_t MAX_REGIONS = 32;
    static constexpr size_t MAX_EVENTS  = 64;

    FamineRegion* createRegion(const std::string& name, float pop = 1000.f,
                                float food = 500.f, float rate = 1.f) {
        if (m_regions.size() >= MAX_REGIONS) return nullptr;
        for (auto& r : m_regions) if (r.name() == name) return nullptr;
        m_regions.emplace_back(name, pop, food, rate);
        return &m_regions.back();
    }

    [[nodiscard]] FamineRegion* regionByName(const std::string& name) {
        for (auto& r : m_regions) if (r.name() == name) return &r;
        return nullptr;
    }

    bool addEvent(const FamineEvent& ev) {
        if (m_events.size() >= MAX_EVENTS) return false;
        for (auto& e : m_events) if (e.id == ev.id) return false;
        m_events.push_back(ev);
        return true;
    }

    [[nodiscard]] FamineEvent* findEvent(const std::string& id) {
        for (auto& e : m_events) if (e.id == id) return &e;
        return nullptr;
    }

    void tick(float dt) {
        for (auto& r : m_regions) r.tick(dt);
        for (auto& e : m_events)  e.advanceDuration(dt);
        m_tickCount++;
    }

    [[nodiscard]] size_t regionCount()       const { return m_regions.size(); }
    [[nodiscard]] size_t eventCount()        const { return m_events.size(); }
    [[nodiscard]] size_t tickCount()         const { return m_tickCount; }

    [[nodiscard]] size_t activeEventCount() const {
        size_t c = 0;
        for (auto& e : m_events) if (e.isActive()) c++;
        return c;
    }

    [[nodiscard]] size_t resolvedEventCount() const {
        size_t c = 0;
        for (auto& e : m_events) if (!e.isActive()) c++;
        return c;
    }

    [[nodiscard]] size_t criticalRegionCount() const {
        size_t c = 0;
        for (auto& r : m_regions)
            if (r.severity() >= FamineSeverity::Severe) c++;
        return c;
    }

private:
    std::vector<FamineRegion> m_regions;
    std::vector<FamineEvent>  m_events;
    size_t                    m_tickCount = 0;
};

// ============================================================
// G37 — Refugee System
// ============================================================

enum class RefugeeOrigin : uint8_t {
    War        = 0,
    Famine     = 1,
    Plague     = 2,
    Disaster   = 3,
    Political  = 4,
    Economic   = 5,
    Religious  = 6,
    Climate    = 7,
};

inline const char* refugeeOriginName(RefugeeOrigin o) {
    switch (o) {
        case RefugeeOrigin::War:       return "War";
        case RefugeeOrigin::Famine:    return "Famine";
        case RefugeeOrigin::Plague:    return "Plague";
        case RefugeeOrigin::Disaster:  return "Disaster";
        case RefugeeOrigin::Political: return "Political";
        case RefugeeOrigin::Economic:  return "Economic";
        case RefugeeOrigin::Religious: return "Religious";
        case RefugeeOrigin::Climate:   return "Climate";
        default:                       return "Unknown";
    }
}

enum class RefugeeStatus : uint8_t {
    InTransit  = 0,
    Sheltered  = 1,
    Settled    = 2,
    Displaced  = 3,
    Returned   = 4,
};

struct Refugee {
    std::string    id;
    std::string    name;
    RefugeeOrigin  origin  = RefugeeOrigin::War;
    RefugeeStatus  status  = RefugeeStatus::InTransit;
    float          health  = 1.0f;  // 0..1

    [[nodiscard]] bool isInTransit() const { return status == RefugeeStatus::InTransit; }
    [[nodiscard]] bool isSheltered() const { return status == RefugeeStatus::Sheltered; }
    [[nodiscard]] bool isSettled()   const { return status == RefugeeStatus::Settled;   }
    [[nodiscard]] bool isDisplaced() const { return status == RefugeeStatus::Displaced; }
    [[nodiscard]] bool isReturned()  const { return status == RefugeeStatus::Returned;  }

    bool shelter() {
        if (status != RefugeeStatus::InTransit && status != RefugeeStatus::Displaced)
            return false;
        status = RefugeeStatus::Sheltered;
        return true;
    }

    bool settle() {
        if (status != RefugeeStatus::Sheltered) return false;
        status = RefugeeStatus::Settled;
        return true;
    }

    bool displace() {
        if (status == RefugeeStatus::Returned || status == RefugeeStatus::Settled)
            return false;
        status = RefugeeStatus::Displaced;
        return true;
    }

    bool sendHome() {
        if (status != RefugeeStatus::Sheltered && status != RefugeeStatus::Settled)
            return false;
        status = RefugeeStatus::Returned;
        return true;
    }
};

class RefugeeCamp {
public:
    static constexpr size_t MAX_REFUGEES = 512;

    explicit RefugeeCamp(std::string name, size_t capacity = 100)
        : m_name(std::move(name)), m_capacity(capacity) {}

    [[nodiscard]] const std::string& name()     const { return m_name; }
    [[nodiscard]] size_t capacity()             const { return m_capacity; }
    [[nodiscard]] size_t refugeeCount()         const { return m_refugees.size(); }
    [[nodiscard]] size_t tickCount()            const { return m_tickCount; }

    bool addRefugee(const Refugee& r) {
        if (m_refugees.size() >= MAX_REFUGEES) return false;
        if (m_refugees.size() >= m_capacity)   return false;
        for (auto& ref : m_refugees) if (ref.id == r.id) return false;
        m_refugees.push_back(r);
        return true;
    }

    bool removeRefugee(const std::string& id) {
        for (auto it = m_refugees.begin(); it != m_refugees.end(); ++it) {
            if (it->id == id) { m_refugees.erase(it); return true; }
        }
        return false;
    }

    [[nodiscard]] Refugee* findRefugee(const std::string& id) {
        for (auto& r : m_refugees) if (r.id == id) return &r;
        return nullptr;
    }

    [[nodiscard]] size_t shelteredCount() const {
        size_t c = 0;
        for (auto& r : m_refugees) if (r.isSheltered()) c++;
        return c;
    }

    [[nodiscard]] size_t settledCount() const {
        size_t c = 0;
        for (auto& r : m_refugees) if (r.isSettled()) c++;
        return c;
    }

    [[nodiscard]] bool isFull() const { return m_refugees.size() >= m_capacity; }

    void tick(float /*dt*/) { m_tickCount++; }

private:
    std::string         m_name;
    size_t              m_capacity;
    std::vector<Refugee> m_refugees;
    size_t              m_tickCount = 0;
};

class RefugeeSystem {
public:
    static constexpr size_t MAX_CAMPS = 32;

    RefugeeCamp* createCamp(const std::string& name, size_t capacity = 100) {
        if (m_camps.size() >= MAX_CAMPS) return nullptr;
        for (auto& c : m_camps) if (c.name() == name) return nullptr;
        m_camps.emplace_back(name, capacity);
        return &m_camps.back();
    }

    [[nodiscard]] RefugeeCamp* campByName(const std::string& name) {
        for (auto& c : m_camps) if (c.name() == name) return &c;
        return nullptr;
    }

    void tick(float dt) {
        for (auto& c : m_camps) c.tick(dt);
        m_tickCount++;
    }

    [[nodiscard]] size_t campCount() const { return m_camps.size(); }
    [[nodiscard]] size_t tickCount() const { return m_tickCount; }

    [[nodiscard]] size_t totalRefugees() const {
        size_t c = 0;
        for (auto& camp : m_camps) c += camp.refugeeCount();
        return c;
    }

    [[nodiscard]] size_t totalSheltered() const {
        size_t c = 0;
        for (auto& camp : m_camps) c += camp.shelteredCount();
        return c;
    }

    [[nodiscard]] size_t totalSettled() const {
        size_t c = 0;
        for (auto& camp : m_camps) c += camp.settledCount();
        return c;
    }

    [[nodiscard]] size_t fullCampCount() const {
        size_t c = 0;
        for (auto& camp : m_camps) if (camp.isFull()) c++;
        return c;
    }

private:
    std::vector<RefugeeCamp> m_camps;
    size_t                   m_tickCount = 0;
};

// ============================================================
// G38 — Storm System
// ============================================================

enum class StormType : uint8_t {
    Thunderstorm  = 0,
    Hurricane     = 1,
    Blizzard      = 2,
    Sandstorm     = 3,
    Firestorm     = 4,
    Hailstorm     = 5,
    Tornado       = 6,
    ElectricStorm = 7,
};

inline const char* stormTypeName(StormType t) {
    switch (t) {
        case StormType::Thunderstorm:  return "Thunderstorm";
        case StormType::Hurricane:     return "Hurricane";
        case StormType::Blizzard:      return "Blizzard";
        case StormType::Sandstorm:     return "Sandstorm";
        case StormType::Firestorm:     return "Firestorm";
        case StormType::Hailstorm:     return "Hailstorm";
        case StormType::Tornado:       return "Tornado";
        case StormType::ElectricStorm: return "ElectricStorm";
        default:                       return "Unknown";
    }
}

enum class StormSeverity : uint8_t {
    None         = 0,
    Mild         = 1,
    Moderate     = 2,
    Severe       = 3,
    Catastrophic = 4,
};

struct Storm {
    std::string   id;
    std::string   region;
    StormType     type     = StormType::Thunderstorm;
    StormSeverity severity = StormSeverity::Mild;
    uint32_t      duration = 0;    // ticks remaining
    bool          active   = true;

    [[nodiscard]] bool isActive()   const { return active && duration > 0; }
    [[nodiscard]] bool isCritical() const { return severity >= StormSeverity::Severe; }

    void dissipate() { active = false; duration = 0; }

    void advanceDuration() {
        if (!active) return;
        if (duration > 0) duration--;
        if (duration == 0) active = false;
    }
};

class StormRegion {
public:
    explicit StormRegion(std::string name) : m_name(std::move(name)) {}

    [[nodiscard]] const std::string& name()  const { return m_name; }
    [[nodiscard]] size_t tickCount()         const { return m_tickCount; }

    [[nodiscard]] StormSeverity currentSeverity() const {
        StormSeverity worst = StormSeverity::None;
        for (auto& s : m_storms) {
            if (s.isActive() && s.severity > worst)
                worst = s.severity;
        }
        return worst;
    }

    bool addStorm(const Storm& s) {
        for (auto& existing : m_storms) if (existing.id == s.id) return false;
        m_storms.push_back(s);
        return true;
    }

    [[nodiscard]] Storm* findStorm(const std::string& id) {
        for (auto& s : m_storms) if (s.id == id) return &s;
        return nullptr;
    }

    [[nodiscard]] size_t stormCount()  const { return m_storms.size(); }

    [[nodiscard]] size_t activeStormCount() const {
        size_t c = 0;
        for (auto& s : m_storms) if (s.isActive()) c++;
        return c;
    }

    void tick() {
        for (auto& s : m_storms) s.advanceDuration();
        m_tickCount++;
    }

private:
    std::string        m_name;
    std::vector<Storm> m_storms;
    size_t             m_tickCount = 0;
};

class StormSystem {
public:
    static constexpr size_t MAX_REGIONS = 32;
    static constexpr size_t MAX_STORMS  = 128;

    StormRegion* createRegion(const std::string& name) {
        if (m_regions.size() >= MAX_REGIONS) return nullptr;
        for (auto& r : m_regions) if (r.name() == name) return nullptr;
        m_regions.emplace_back(name);
        return &m_regions.back();
    }

    [[nodiscard]] StormRegion* regionByName(const std::string& name) {
        for (auto& r : m_regions) if (r.name() == name) return &r;
        return nullptr;
    }

    bool addStorm(const Storm& s) {
        if (m_stormCount >= MAX_STORMS) return false;
        auto* region = regionByName(s.region);
        if (!region) return false;
        if (region->addStorm(s)) { m_stormCount++; return true; }
        return false;
    }

    void tick() {
        for (auto& r : m_regions) r.tick();
        m_tickCount++;
    }

    [[nodiscard]] size_t regionCount()      const { return m_regions.size(); }
    [[nodiscard]] size_t tickCount()        const { return m_tickCount; }
    [[nodiscard]] size_t totalStormCount()  const { return m_stormCount; }

    [[nodiscard]] size_t activeStormCount() const {
        size_t c = 0;
        for (auto& r : m_regions) c += r.activeStormCount();
        return c;
    }

    [[nodiscard]] size_t criticalRegionCount() const {
        size_t c = 0;
        for (auto& r : m_regions)
            if (r.currentSeverity() >= StormSeverity::Severe) c++;
        return c;
    }

private:
    std::vector<StormRegion> m_regions;
    size_t                   m_tickCount  = 0;
    size_t                   m_stormCount = 0;
};

// ============================================================
// G39 — Earthquake System
// ============================================================

enum class EarthquakeScale : uint8_t {
    Micro       = 0,
    Minor       = 1,
    Light       = 2,
    Moderate    = 3,
    Strong      = 4,
    Major       = 5,
    Great       = 6,
    Catastrophic = 7,
};

inline const char* earthquakeScaleName(EarthquakeScale s) {
    switch (s) {
        case EarthquakeScale::Micro:        return "Micro";
        case EarthquakeScale::Minor:        return "Minor";
        case EarthquakeScale::Light:        return "Light";
        case EarthquakeScale::Moderate:     return "Moderate";
        case EarthquakeScale::Strong:       return "Strong";
        case EarthquakeScale::Major:        return "Major";
        case EarthquakeScale::Great:        return "Great";
        case EarthquakeScale::Catastrophic: return "Catastrophic";
        default:                            return "Unknown";
    }
}

enum class EarthquakeStatus : uint8_t {
    Pending    = 0,
    Active     = 1,
    Aftershock = 2,
    Resolved   = 3,
};

struct Earthquake {
    std::string      id;
    std::string      region;
    EarthquakeScale  scale    = EarthquakeScale::Minor;
    EarthquakeStatus status   = EarthquakeStatus::Pending;
    float            magnitude = 0.0f;
    uint32_t         depth     = 0;    // km
    uint32_t         duration  = 0;    // ticks

    [[nodiscard]] bool isActive()     const { return status == EarthquakeStatus::Active;     }
    [[nodiscard]] bool isPending()    const { return status == EarthquakeStatus::Pending;    }
    [[nodiscard]] bool isResolved()   const { return status == EarthquakeStatus::Resolved;   }
    [[nodiscard]] bool isAftershock() const { return status == EarthquakeStatus::Aftershock; }
    [[nodiscard]] bool isMajor()      const { return scale >= EarthquakeScale::Major;        }

    void activate()  { if (status == EarthquakeStatus::Pending)    status = EarthquakeStatus::Active;     }
    void resolve()   { status = EarthquakeStatus::Resolved;   }
    void toAftershock() { if (status == EarthquakeStatus::Active) status = EarthquakeStatus::Aftershock; }
};

class FaultLine {
public:
    explicit FaultLine(std::string name) : m_name(std::move(name)) {}

    [[nodiscard]] const std::string& name()  const { return m_name; }
    [[nodiscard]] size_t tickCount()         const { return m_tickCount; }

    bool addEarthquake(const Earthquake& eq) {
        for (auto& e : m_earthquakes) if (e.id == eq.id) return false;
        m_earthquakes.push_back(eq);
        return true;
    }

    [[nodiscard]] Earthquake* find(const std::string& id) {
        for (auto& e : m_earthquakes) if (e.id == id) return &e;
        return nullptr;
    }

    [[nodiscard]] size_t earthquakeCount() const { return m_earthquakes.size(); }

    [[nodiscard]] size_t activeCount() const {
        size_t c = 0;
        for (auto& e : m_earthquakes) if (e.isActive() || e.isAftershock()) c++;
        return c;
    }

    [[nodiscard]] size_t majorCount() const {
        size_t c = 0;
        for (auto& e : m_earthquakes) if (e.isMajor()) c++;
        return c;
    }

    void tick() { m_tickCount++; }

private:
    std::string            m_name;
    std::vector<Earthquake> m_earthquakes;
    size_t                 m_tickCount = 0;
};

class EarthquakeSystem {
public:
    static constexpr size_t MAX_FAULTS      = 32;
    static constexpr size_t MAX_EARTHQUAKES = 256;

    FaultLine* createFaultLine(const std::string& name) {
        if (m_faults.size() >= MAX_FAULTS) return nullptr;
        for (auto& f : m_faults) if (f.name() == name) return nullptr;
        m_faults.emplace_back(name);
        return &m_faults.back();
    }

    [[nodiscard]] FaultLine* faultByName(const std::string& name) {
        for (auto& f : m_faults) if (f.name() == name) return &f;
        return nullptr;
    }

    bool addEarthquake(const Earthquake& eq) {
        if (m_eqCount >= MAX_EARTHQUAKES) return false;
        auto* fault = faultByName(eq.region);
        if (!fault) return false;
        if (fault->addEarthquake(eq)) { m_eqCount++; return true; }
        return false;
    }

    void tick() {
        for (auto& f : m_faults) f.tick();
        m_tickCount++;
    }

    [[nodiscard]] size_t faultCount()       const { return m_faults.size(); }
    [[nodiscard]] size_t tickCount()        const { return m_tickCount;     }
    [[nodiscard]] size_t earthquakeCount()  const { return m_eqCount;       }

    [[nodiscard]] size_t activeEarthquakeCount() const {
        size_t c = 0;
        for (auto& f : m_faults) c += f.activeCount();
        return c;
    }

    [[nodiscard]] size_t majorEarthquakeCount() const {
        size_t c = 0;
        for (auto& f : m_faults) c += f.majorCount();
        return c;
    }

private:
    std::vector<FaultLine> m_faults;
    size_t                 m_tickCount = 0;
    size_t                 m_eqCount   = 0;
};

// ============================================================
// G40 — Volcano System
// ============================================================

enum class VolcanoActivity : uint8_t {
    Dormant      = 0,
    Restless     = 1,
    Elevated     = 2,
    Unrest       = 3,
    Minor        = 4,
    Moderate     = 5,
    Major        = 6,
    Catastrophic = 7,
};

inline const char* volcanoActivityName(VolcanoActivity a) {
    switch (a) {
        case VolcanoActivity::Dormant:      return "Dormant";
        case VolcanoActivity::Restless:     return "Restless";
        case VolcanoActivity::Elevated:     return "Elevated";
        case VolcanoActivity::Unrest:       return "Unrest";
        case VolcanoActivity::Minor:        return "Minor";
        case VolcanoActivity::Moderate:     return "Moderate";
        case VolcanoActivity::Major:        return "Major";
        case VolcanoActivity::Catastrophic: return "Catastrophic";
        default:                            return "Unknown";
    }
}

enum class VolcanoStatus : uint8_t {
    Inactive  = 0,
    Monitoring = 1,
    Erupting  = 2,
    Subsiding = 3,
};

struct VolcanicEvent {
    std::string     id;
    VolcanoActivity activity = VolcanoActivity::Minor;
    uint32_t        duration = 0;    // ticks
    uint32_t        ashfall  = 0;    // km radius
    bool            resolved = false;

    void resolve() { resolved = true; }
    [[nodiscard]] bool isResolved()     const { return resolved;                          }
    [[nodiscard]] bool isMajor()        const { return activity >= VolcanoActivity::Major; }
    [[nodiscard]] bool isCatastrophic() const { return activity == VolcanoActivity::Catastrophic; }
};

class Volcano {
public:
    explicit Volcano(std::string name) : m_name(std::move(name)) {}

    [[nodiscard]] const std::string& name()    const { return m_name;    }
    [[nodiscard]] VolcanoStatus      status()   const { return m_status;  }
    [[nodiscard]] VolcanoActivity    activity() const { return m_activity; }
    [[nodiscard]] size_t             tickCount() const { return m_tickCount; }

    void setActivity(VolcanoActivity a) { m_activity = a; }
    void startEruption()  { m_status = VolcanoStatus::Erupting;   }
    void beginSubsiding() { if (m_status == VolcanoStatus::Erupting) m_status = VolcanoStatus::Subsiding; }
    void deactivate()     { m_status = VolcanoStatus::Inactive;   }
    void monitor()        { m_status = VolcanoStatus::Monitoring;  }

    [[nodiscard]] bool isErupting()  const { return m_status == VolcanoStatus::Erupting;  }
    [[nodiscard]] bool isInactive()  const { return m_status == VolcanoStatus::Inactive;  }
    [[nodiscard]] bool isSubsiding() const { return m_status == VolcanoStatus::Subsiding; }

    bool addEvent(const VolcanicEvent& ev) {
        for (auto& e : m_events) if (e.id == ev.id) return false;
        m_events.push_back(ev);
        return true;
    }

    [[nodiscard]] VolcanicEvent* findEvent(const std::string& id) {
        for (auto& e : m_events) if (e.id == id) return &e;
        return nullptr;
    }

    [[nodiscard]] size_t eventCount()   const { return m_events.size(); }
    [[nodiscard]] size_t majorEvents()  const {
        size_t c = 0;
        for (auto& e : m_events) if (e.isMajor()) c++;
        return c;
    }

    void tick() { m_tickCount++; }

private:
    std::string               m_name;
    VolcanoStatus             m_status   = VolcanoStatus::Inactive;
    VolcanoActivity           m_activity = VolcanoActivity::Dormant;
    std::vector<VolcanicEvent> m_events;
    size_t                    m_tickCount = 0;
};

class VolcanoSystem {
public:
    static constexpr size_t MAX_VOLCANOES = 64;
    static constexpr size_t MAX_EVENTS    = 512;

    Volcano* createVolcano(const std::string& name) {
        if (m_volcanoes.size() >= MAX_VOLCANOES) return nullptr;
        for (auto& v : m_volcanoes) if (v.name() == name) return nullptr;
        m_volcanoes.emplace_back(name);
        return &m_volcanoes.back();
    }

    [[nodiscard]] Volcano* byName(const std::string& name) {
        for (auto& v : m_volcanoes) if (v.name() == name) return &v;
        return nullptr;
    }

    bool addEvent(const std::string& volcanoName, const VolcanicEvent& ev) {
        if (m_eventCount >= MAX_EVENTS) return false;
        auto* v = byName(volcanoName);
        if (!v) return false;
        if (v->addEvent(ev)) { m_eventCount++; return true; }
        return false;
    }

    void tick() {
        for (auto& v : m_volcanoes) v.tick();
        m_tickCount++;
    }

    [[nodiscard]] size_t volcanoCount()  const { return m_volcanoes.size(); }
    [[nodiscard]] size_t eventCount()    const { return m_eventCount;       }
    [[nodiscard]] size_t tickCount()     const { return m_tickCount;        }

    [[nodiscard]] size_t eruptingCount() const {
        size_t c = 0;
        for (auto& v : m_volcanoes) if (v.isErupting()) c++;
        return c;
    }

    [[nodiscard]] size_t majorEventCount() const {
        size_t c = 0;
        for (auto& v : m_volcanoes) c += v.majorEvents();
        return c;
    }

private:
    std::vector<Volcano> m_volcanoes;
    size_t               m_tickCount  = 0;
    size_t               m_eventCount = 0;
};

// ============================================================
// G41 — Tsunami System
// ============================================================

enum class TsunamiCause : uint8_t {
    Earthquake   = 0,
    Landslide    = 1,
    Volcanic     = 2,
    Meteorite    = 3,
    Submarine    = 4,
    Glacial      = 5,
    Nuclear      = 6,
    Unknown      = 7,
};

inline const char* tsunamiCauseName(TsunamiCause c) {
    switch (c) {
        case TsunamiCause::Earthquake:  return "Earthquake";
        case TsunamiCause::Landslide:   return "Landslide";
        case TsunamiCause::Volcanic:    return "Volcanic";
        case TsunamiCause::Meteorite:   return "Meteorite";
        case TsunamiCause::Submarine:   return "Submarine";
        case TsunamiCause::Glacial:     return "Glacial";
        case TsunamiCause::Nuclear:     return "Nuclear";
        case TsunamiCause::Unknown:     return "Unknown";
        default:                        return "Unknown";
    }
}

enum class TsunamiStatus : uint8_t {
    Forming   = 0,
    Traveling = 1,
    Striking  = 2,
    Receding  = 3,
};

struct TsunamiWave {
    float    heightMeters  = 0.f;
    float    speedKmh      = 0.f;
    float    periodSeconds = 0.f;
    bool     broke         = false;

    void breakWave() { broke = true; }
    [[nodiscard]] bool hasBroken()    const { return broke;                  }
    [[nodiscard]] bool isDevastating() const { return heightMeters >= 10.f;  }
    [[nodiscard]] bool isFast()        const { return speedKmh >= 800.f;     }
};

class Tsunami {
public:
    explicit Tsunami(std::string id) : m_id(std::move(id)) {}

    [[nodiscard]] const std::string& id()      const { return m_id;     }
    [[nodiscard]] TsunamiStatus      status()   const { return m_status; }
    [[nodiscard]] TsunamiCause       cause()    const { return m_cause;  }
    [[nodiscard]] size_t             waveCount() const { return m_waves.size(); }
    [[nodiscard]] size_t             tickCount() const { return m_tickCount;    }

    void setCause(TsunamiCause c) { m_cause = c; }
    void advance() {
        switch (m_status) {
            case TsunamiStatus::Forming:   m_status = TsunamiStatus::Traveling; break;
            case TsunamiStatus::Traveling: m_status = TsunamiStatus::Striking;  break;
            case TsunamiStatus::Striking:  m_status = TsunamiStatus::Receding;  break;
            default: break;
        }
    }

    bool addWave(const TsunamiWave& w) {
        m_waves.push_back(w);
        return true;
    }

    [[nodiscard]] float maxWaveHeight() const {
        float max = 0.f;
        for (auto& w : m_waves) if (w.heightMeters > max) max = w.heightMeters;
        return max;
    }

    [[nodiscard]] bool isDevastating() const { return maxWaveHeight() >= 10.f; }
    [[nodiscard]] bool isReceding()    const { return m_status == TsunamiStatus::Receding;  }
    [[nodiscard]] bool isStriking()    const { return m_status == TsunamiStatus::Striking;  }

    void tick() { m_tickCount++; }

private:
    std::string          m_id;
    TsunamiStatus        m_status    = TsunamiStatus::Forming;
    TsunamiCause         m_cause     = TsunamiCause::Unknown;
    std::vector<TsunamiWave> m_waves;
    size_t               m_tickCount = 0;
};

class TsunamiSystem {
public:
    static constexpr size_t MAX_TSUNAMIS = 64;

    Tsunami* create(const std::string& id) {
        if (m_tsunamis.size() >= MAX_TSUNAMIS) return nullptr;
        for (auto& t : m_tsunamis) if (t.id() == id) return nullptr;
        m_tsunamis.emplace_back(id);
        return &m_tsunamis.back();
    }

    [[nodiscard]] Tsunami* find(const std::string& id) {
        for (auto& t : m_tsunamis) if (t.id() == id) return &t;
        return nullptr;
    }

    void tick() {
        for (auto& t : m_tsunamis) t.tick();
        m_tickCount++;
    }

    [[nodiscard]] size_t count()       const { return m_tsunamis.size(); }
    [[nodiscard]] size_t tickCount()   const { return m_tickCount;       }

    [[nodiscard]] size_t strikingCount() const {
        size_t c = 0;
        for (auto& t : m_tsunamis) if (t.isStriking()) c++;
        return c;
    }

    [[nodiscard]] size_t devastatingCount() const {
        size_t c = 0;
        for (auto& t : m_tsunamis) if (t.isDevastating()) c++;
        return c;
    }

private:
    std::vector<Tsunami> m_tsunamis;
    size_t               m_tickCount = 0;
};

// ============================================================
// G42 — Wildfire System
// ============================================================

enum class WildfireType : uint8_t {
    Forest, Grassland, Shrub, Peat, Urban, Agricultural, Desert, Tropical
};

[[nodiscard]] inline const char* wildfireTypeName(WildfireType t) {
    switch (t) {
        case WildfireType::Forest:       return "Forest";
        case WildfireType::Grassland:    return "Grassland";
        case WildfireType::Shrub:        return "Shrub";
        case WildfireType::Peat:         return "Peat";
        case WildfireType::Urban:        return "Urban";
        case WildfireType::Agricultural: return "Agricultural";
        case WildfireType::Desert:       return "Desert";
        case WildfireType::Tropical:     return "Tropical";
    }
    return "Unknown";
}

enum class WildfireSeverity : uint8_t {
    Minor, Moderate, Significant, Major, Catastrophic
};

[[nodiscard]] inline const char* wildfireSeverityName(WildfireSeverity s) {
    switch (s) {
        case WildfireSeverity::Minor:        return "Minor";
        case WildfireSeverity::Moderate:     return "Moderate";
        case WildfireSeverity::Significant:  return "Significant";
        case WildfireSeverity::Major:        return "Major";
        case WildfireSeverity::Catastrophic: return "Catastrophic";
    }
    return "Unknown";
}

struct WildfireFront {
    std::string      id;
    float            widthKm         = 0.f;
    float            advanceRateKmh  = 0.f;
    bool             contained       = false;
    WildfireSeverity severity        = WildfireSeverity::Minor;

    void contain()          { contained = true; }
    void spread(float rate) { advanceRateKmh = rate; }

    [[nodiscard]] bool isContained()    const { return contained; }
    [[nodiscard]] bool isSpreading()    const { return advanceRateKmh > 0.f && !contained; }
    [[nodiscard]] bool isCatastrophic() const { return severity == WildfireSeverity::Catastrophic; }
};

class WildfireZone {
public:
    explicit WildfireZone(const std::string& name) : m_name(name) {}

    void setType(WildfireType t)         { m_type = t; }
    void setSeverity(WildfireSeverity s) { m_severity = s; }

    bool addFront(const WildfireFront& f) {
        for (auto& existing : m_fronts) if (existing.id == f.id) return false;
        m_fronts.push_back(f);
        return true;
    }

    void containAll() { for (auto& f : m_fronts) f.contain(); }

    [[nodiscard]] size_t frontCount() const { return m_fronts.size(); }

    [[nodiscard]] size_t containedFronts() const {
        size_t c = 0;
        for (auto& f : m_fronts) if (f.isContained()) c++;
        return c;
    }

    [[nodiscard]] const std::string& name()     const { return m_name;     }
    [[nodiscard]] WildfireType       type()     const { return m_type;     }
    [[nodiscard]] WildfireSeverity   severity() const { return m_severity; }

    [[nodiscard]] bool isActive() const {
        for (auto& f : m_fronts) if (f.isSpreading()) return true;
        return false;
    }

    void tick() { m_tickCount++; }

private:
    std::string                m_name;
    WildfireType               m_type      = WildfireType::Forest;
    WildfireSeverity           m_severity  = WildfireSeverity::Minor;
    std::vector<WildfireFront> m_fronts;
    size_t                     m_tickCount = 0;
};

class WildfireSystem {
public:
    static constexpr size_t MAX_ZONES = 64;

    WildfireZone* createZone(const std::string& name) {
        if (m_zones.size() >= MAX_ZONES) return nullptr;
        for (auto& z : m_zones) if (z.name() == name) return nullptr;
        m_zones.emplace_back(name);
        return &m_zones.back();
    }

    [[nodiscard]] WildfireZone* byName(const std::string& name) {
        for (auto& z : m_zones) if (z.name() == name) return &z;
        return nullptr;
    }

    void tick() {
        for (auto& z : m_zones) z.tick();
        m_tickCount++;
    }

    [[nodiscard]] size_t zoneCount()  const { return m_zones.size(); }
    [[nodiscard]] size_t tickCount()  const { return m_tickCount;    }

    [[nodiscard]] size_t activeCount() const {
        size_t c = 0;
        for (auto& z : m_zones) if (z.isActive()) c++;
        return c;
    }

    [[nodiscard]] size_t catastrophicCount() const {
        size_t c = 0;
        for (auto& z : m_zones) if (z.severity() == WildfireSeverity::Catastrophic) c++;
        return c;
    }

private:
    std::vector<WildfireZone> m_zones;
    size_t                    m_tickCount = 0;
};

// ============================================================
// G43 — Flood System
// ============================================================

enum class FloodType : uint8_t { River, Coastal, Flash, Urban, Groundwater, Dam, Snowmelt, Tropical };

inline const char* floodTypeName(FloodType t) {
    switch (t) {
        case FloodType::River:       return "River";
        case FloodType::Coastal:     return "Coastal";
        case FloodType::Flash:       return "Flash";
        case FloodType::Urban:       return "Urban";
        case FloodType::Groundwater: return "Groundwater";
        case FloodType::Dam:         return "Dam";
        case FloodType::Snowmelt:    return "Snowmelt";
        case FloodType::Tropical:    return "Tropical";
        default:                     return "Unknown";
    }
}

enum class FloodSeverity : uint8_t { Minor, Moderate, Significant, Major, Catastrophic };

inline const char* floodSeverityName(FloodSeverity s) {
    switch (s) {
        case FloodSeverity::Minor:        return "Minor";
        case FloodSeverity::Moderate:     return "Moderate";
        case FloodSeverity::Significant:  return "Significant";
        case FloodSeverity::Major:        return "Major";
        case FloodSeverity::Catastrophic: return "Catastrophic";
        default:                          return "Unknown";
    }
}

struct FloodWaterLevel {
    std::string id;
    float       depthMeters           = 0.f;
    float       riseRateMetersPerHour = 0.f;
    bool        receding              = false;

    void startReceding()  { receding = true; }
    void rise(float rate) { riseRateMetersPerHour = rate; }

    [[nodiscard]] bool isRising()       const { return riseRateMetersPerHour > 0.f && !receding; }
    [[nodiscard]] bool isDangerous()    const { return depthMeters >= 1.0f; }
    [[nodiscard]] bool isCatastrophic() const { return depthMeters >= 5.0f; }
};

class FloodZone {
public:
    explicit FloodZone(const std::string& name) : m_name(name) {}

    void setType(FloodType t)         { m_type = t;     }
    void setSeverity(FloodSeverity s) { m_severity = s; }

    bool addLevel(const FloodWaterLevel& l) {
        for (auto& existing : m_levels) if (existing.id == l.id) return false;
        m_levels.push_back(l);
        return true;
    }

    void recessAll() { for (auto& l : m_levels) l.startReceding(); }

    [[nodiscard]] size_t levelCount() const { return m_levels.size(); }

    [[nodiscard]] size_t recedingLevels() const {
        size_t c = 0;
        for (auto& l : m_levels) if (l.receding) c++;
        return c;
    }

    [[nodiscard]] const std::string& name()     const { return m_name;     }
    [[nodiscard]] FloodType          type()     const { return m_type;     }
    [[nodiscard]] FloodSeverity      severity() const { return m_severity; }

    [[nodiscard]] bool isFlooding() const {
        for (auto& l : m_levels) if (l.isRising()) return true;
        return false;
    }

    void tick() { m_tickCount++; }

private:
    std::string                  m_name;
    FloodType                    m_type      = FloodType::River;
    FloodSeverity                m_severity  = FloodSeverity::Minor;
    std::vector<FloodWaterLevel> m_levels;
    size_t                       m_tickCount = 0;
};

class FloodSystem {
public:
    static constexpr size_t MAX_ZONES = 64;

    FloodZone* createZone(const std::string& name) {
        if (m_zones.size() >= MAX_ZONES) return nullptr;
        for (auto& z : m_zones) if (z.name() == name) return nullptr;
        m_zones.emplace_back(name);
        return &m_zones.back();
    }

    [[nodiscard]] FloodZone* byName(const std::string& name) {
        for (auto& z : m_zones) if (z.name() == name) return &z;
        return nullptr;
    }

    void tick() {
        for (auto& z : m_zones) z.tick();
        m_tickCount++;
    }

    [[nodiscard]] size_t zoneCount()  const { return m_zones.size(); }
    [[nodiscard]] size_t tickCount()  const { return m_tickCount;    }

    [[nodiscard]] size_t floodingCount() const {
        size_t c = 0;
        for (auto& z : m_zones) if (z.isFlooding()) c++;
        return c;
    }

    [[nodiscard]] size_t catastrophicCount() const {
        size_t c = 0;
        for (auto& z : m_zones) if (z.severity() == FloodSeverity::Catastrophic) c++;
        return c;
    }

private:
    std::vector<FloodZone> m_zones;
    size_t                 m_tickCount = 0;
};

// ============================================================
// G44 — Landslide System
// ============================================================

enum class LandslideType : uint8_t { Debris, Rockfall, Mudflow, Slump, Creep, Avalanche, Earthflow, Topple };

inline const char* landslideTypeName(LandslideType t) {
    switch (t) {
        case LandslideType::Debris:    return "Debris";
        case LandslideType::Rockfall:  return "Rockfall";
        case LandslideType::Mudflow:   return "Mudflow";
        case LandslideType::Slump:     return "Slump";
        case LandslideType::Creep:     return "Creep";
        case LandslideType::Avalanche: return "Avalanche";
        case LandslideType::Earthflow: return "Earthflow";
        case LandslideType::Topple:    return "Topple";
        default:                       return "Unknown";
    }
}

enum class LandslideSeverity : uint8_t { Minor, Moderate, Significant, Major, Catastrophic };

inline const char* landslideSeverityName(LandslideSeverity s) {
    switch (s) {
        case LandslideSeverity::Minor:        return "Minor";
        case LandslideSeverity::Moderate:     return "Moderate";
        case LandslideSeverity::Significant:  return "Significant";
        case LandslideSeverity::Major:        return "Major";
        case LandslideSeverity::Catastrophic: return "Catastrophic";
        default:                              return "Unknown";
    }
}

struct LandslideDebrisFlow {
    std::string id;
    float       volumeCubicMeters  = 0.f;
    float       speedMetersPerSec  = 0.f;
    bool        halted             = false;

    void halt()                    { halted = true; }
    void accelerate(float speed)   { speedMetersPerSec = speed; }

    [[nodiscard]] bool isMoving()       const { return speedMetersPerSec > 0.f && !halted; }
    [[nodiscard]] bool isDangerous()    const { return volumeCubicMeters >= 1000.f;         }
    [[nodiscard]] bool isCatastrophic() const { return volumeCubicMeters >= 100000.f;       }
};

class LandslideZone {
public:
    explicit LandslideZone(const std::string& name) : m_name(name) {}

    void setType(LandslideType t)         { m_type = t;     }
    void setSeverity(LandslideSeverity s) { m_severity = s; }

    bool addFlow(const LandslideDebrisFlow& f) {
        for (auto& existing : m_flows) if (existing.id == f.id) return false;
        m_flows.push_back(f);
        return true;
    }

    void haltAll() { for (auto& f : m_flows) f.halt(); }

    [[nodiscard]] size_t flowCount() const { return m_flows.size(); }

    [[nodiscard]] size_t movingFlows() const {
        size_t c = 0;
        for (auto& f : m_flows) if (f.isMoving()) c++;
        return c;
    }

    [[nodiscard]] const std::string&  name()     const { return m_name;     }
    [[nodiscard]] LandslideType       type()     const { return m_type;     }
    [[nodiscard]] LandslideSeverity   severity() const { return m_severity; }

    [[nodiscard]] bool isActive() const {
        for (auto& f : m_flows) if (f.isMoving()) return true;
        return false;
    }

    void tick() { m_tickCount++; }

private:
    std::string                      m_name;
    LandslideType                    m_type      = LandslideType::Debris;
    LandslideSeverity                m_severity  = LandslideSeverity::Minor;
    std::vector<LandslideDebrisFlow> m_flows;
    size_t                           m_tickCount = 0;
};

class LandslideSystem {
public:
    static constexpr size_t MAX_ZONES = 64;

    LandslideZone* createZone(const std::string& name) {
        if (m_zones.size() >= MAX_ZONES) return nullptr;
        for (auto& z : m_zones) if (z.name() == name) return nullptr;
        m_zones.emplace_back(name);
        return &m_zones.back();
    }

    [[nodiscard]] LandslideZone* byName(const std::string& name) {
        for (auto& z : m_zones) if (z.name() == name) return &z;
        return nullptr;
    }

    void tick() {
        for (auto& z : m_zones) z.tick();
        m_tickCount++;
    }

    [[nodiscard]] size_t zoneCount()  const { return m_zones.size(); }
    [[nodiscard]] size_t tickCount()  const { return m_tickCount;    }

    [[nodiscard]] size_t activeCount() const {
        size_t c = 0;
        for (auto& z : m_zones) if (z.isActive()) c++;
        return c;
    }

    [[nodiscard]] size_t catastrophicCount() const {
        size_t c = 0;
        for (auto& z : m_zones) if (z.severity() == LandslideSeverity::Catastrophic) c++;
        return c;
    }

private:
    std::vector<LandslideZone> m_zones;
    size_t                     m_tickCount = 0;
};

// ============================================================
// G45 — Drought System
// ============================================================

enum class DroughtType : uint8_t { Agricultural, Hydrological, Meteorological, Socioeconomic, Groundwater, Ecological, Coastal, Urban };

inline const char* droughtTypeName(DroughtType t) {
    switch (t) {
        case DroughtType::Agricultural:   return "Agricultural";
        case DroughtType::Hydrological:   return "Hydrological";
        case DroughtType::Meteorological: return "Meteorological";
        case DroughtType::Socioeconomic:  return "Socioeconomic";
        case DroughtType::Groundwater:    return "Groundwater";
        case DroughtType::Ecological:     return "Ecological";
        case DroughtType::Coastal:        return "Coastal";
        case DroughtType::Urban:          return "Urban";
        default:                          return "Unknown";
    }
}

enum class DroughtIntensity : uint8_t { Mild, Moderate, Severe, Extreme, Exceptional };

inline const char* droughtIntensityName(DroughtIntensity i) {
    switch (i) {
        case DroughtIntensity::Mild:        return "Mild";
        case DroughtIntensity::Moderate:    return "Moderate";
        case DroughtIntensity::Severe:      return "Severe";
        case DroughtIntensity::Extreme:     return "Extreme";
        case DroughtIntensity::Exceptional: return "Exceptional";
        default:                            return "Unknown";
    }
}

struct DroughtRegion {
    std::string id;
    float       waterReservePercent = 100.0f;
    float       precipitationMm     = 0.f;
    bool        active              = false;

    void deplete(float amount) {
        waterReservePercent -= amount;
        if (waterReservePercent < 0.f) waterReservePercent = 0.f;
    }

    void replenish(float amount) {
        waterReservePercent += amount;
        if (waterReservePercent > 100.f) waterReservePercent = 100.f;
    }

    void activate()   { active = true;  }
    void deactivate() { active = false; }

    [[nodiscard]] bool isArid()      const { return waterReservePercent < 25.0f; }
    [[nodiscard]] bool isExhausted() const { return waterReservePercent <= 0.f;  }
    [[nodiscard]] bool isCritical()  const { return waterReservePercent < 10.0f; }
};

class DroughtZone {
public:
    explicit DroughtZone(const std::string& name) : m_name(name) {}

    void setType(DroughtType t)           { m_type = t;      }
    void setIntensity(DroughtIntensity i) { m_intensity = i; }

    bool addRegion(DroughtRegion r) {
        for (auto& existing : m_regions) if (existing.id == r.id) return false;
        m_regions.push_back(std::move(r));
        return true;
    }

    void depleteAll(float amount)   { for (auto& r : m_regions) r.deplete(amount);   }
    void replenishAll(float amount) { for (auto& r : m_regions) r.replenish(amount); }

    [[nodiscard]] size_t regionCount() const { return m_regions.size(); }

    [[nodiscard]] size_t aridCount() const {
        size_t c = 0;
        for (auto& r : m_regions) if (r.isArid()) c++;
        return c;
    }

    [[nodiscard]] size_t exhaustedCount() const {
        size_t c = 0;
        for (auto& r : m_regions) if (r.isExhausted()) c++;
        return c;
    }

    [[nodiscard]] const std::string& name()      const { return m_name;      }
    [[nodiscard]] DroughtType        type()      const { return m_type;      }
    [[nodiscard]] DroughtIntensity   intensity() const { return m_intensity; }

    [[nodiscard]] bool isCritical() const {
        for (auto& r : m_regions) if (r.isCritical()) return true;
        return false;
    }

    void tick() { m_tickCount++; }

private:
    std::string                m_name;
    DroughtType                m_type      = DroughtType::Agricultural;
    DroughtIntensity           m_intensity = DroughtIntensity::Mild;
    std::vector<DroughtRegion> m_regions;
    size_t                     m_tickCount = 0;
};

class DroughtSystem {
public:
    static constexpr size_t MAX_ZONES = 64;

    DroughtZone* createZone(const std::string& name) {
        if (m_zones.size() >= MAX_ZONES) return nullptr;
        for (auto& z : m_zones) if (z.name() == name) return nullptr;
        m_zones.emplace_back(name);
        return &m_zones.back();
    }

    [[nodiscard]] DroughtZone* byName(const std::string& name) {
        for (auto& z : m_zones) if (z.name() == name) return &z;
        return nullptr;
    }

    void tick() {
        for (auto& z : m_zones) z.tick();
        m_tickCount++;
    }

    [[nodiscard]] size_t zoneCount()  const { return m_zones.size(); }
    [[nodiscard]] size_t tickCount()  const { return m_tickCount;    }

    [[nodiscard]] size_t criticalCount() const {
        size_t c = 0;
        for (auto& z : m_zones) if (z.isCritical()) c++;
        return c;
    }

    [[nodiscard]] size_t exhaustedRegionCount() const {
        size_t c = 0;
        for (auto& z : m_zones) c += z.exhaustedCount();
        return c;
    }

private:
    std::vector<DroughtZone> m_zones;
    size_t                   m_tickCount = 0;
};

// ============================================================
// G46 — Epidemic System
// ============================================================

enum class EpidemicType : uint8_t { Viral, Bacterial, Fungal, Parasitic, Prion, Zoonotic, Waterborne, Airborne };

inline const char* epidemicTypeName(EpidemicType t) {
    switch (t) {
        case EpidemicType::Viral:      return "Viral";
        case EpidemicType::Bacterial:  return "Bacterial";
        case EpidemicType::Fungal:     return "Fungal";
        case EpidemicType::Parasitic:  return "Parasitic";
        case EpidemicType::Prion:      return "Prion";
        case EpidemicType::Zoonotic:   return "Zoonotic";
        case EpidemicType::Waterborne: return "Waterborne";
        case EpidemicType::Airborne:   return "Airborne";
        default:                       return "Unknown";
    }
}

enum class EpidemicPhase : uint8_t { Outbreak, Epidemic, Endemic, Pandemic, Resolved };

inline const char* epidemicPhaseName(EpidemicPhase p) {
    switch (p) {
        case EpidemicPhase::Outbreak:  return "Outbreak";
        case EpidemicPhase::Epidemic:  return "Epidemic";
        case EpidemicPhase::Endemic:   return "Endemic";
        case EpidemicPhase::Pandemic:  return "Pandemic";
        case EpidemicPhase::Resolved:  return "Resolved";
        default:                       return "Unknown";
    }
}

struct EpidemicVector {
    std::string id;
    size_t      infectedCount  = 0;
    size_t      populationSize = 1000;
    bool        active         = false;

    void infect(size_t count) {
        infectedCount = (infectedCount + count > populationSize) ? populationSize : infectedCount + count;
    }

    void recover(size_t count) {
        infectedCount = (count > infectedCount) ? 0 : infectedCount - count;
    }

    void activate()   { active = true;  }
    void deactivate() { active = false; }

    [[nodiscard]] float infectionRate() const {
        if (populationSize == 0) return 0.f;
        return static_cast<float>(infectedCount) / static_cast<float>(populationSize);
    }

    [[nodiscard]] bool isContained() const { return infectionRate() < 0.05f; }
    [[nodiscard]] bool isCritical()  const { return infectionRate() >= 0.5f; }
};

class EpidemicZone {
public:
    explicit EpidemicZone(const std::string& name) : m_name(name) {}

    void setType(EpidemicType t)   { m_type  = t; }
    void setPhase(EpidemicPhase p) { m_phase = p; }

    bool addVector(EpidemicVector v) {
        for (auto& existing : m_vectors) if (existing.id == v.id) return false;
        m_vectors.push_back(std::move(v));
        return true;
    }

    void infectAll(size_t count)  { for (auto& v : m_vectors) v.infect(count);  }
    void recoverAll(size_t count) { for (auto& v : m_vectors) v.recover(count); }

    [[nodiscard]] size_t vectorCount() const { return m_vectors.size(); }

    [[nodiscard]] size_t criticalCount() const {
        size_t c = 0;
        for (auto& v : m_vectors) if (v.isCritical()) c++;
        return c;
    }

    [[nodiscard]] const std::string& name()  const { return m_name;  }
    [[nodiscard]] EpidemicType       type()  const { return m_type;  }
    [[nodiscard]] EpidemicPhase      phase() const { return m_phase; }

    [[nodiscard]] bool isContained() const {
        for (auto& v : m_vectors) if (!v.isContained()) return false;
        return true;
    }

    void tick() { m_tickCount++; }

private:
    std::string                 m_name;
    EpidemicType                m_type      = EpidemicType::Viral;
    EpidemicPhase               m_phase     = EpidemicPhase::Outbreak;
    std::vector<EpidemicVector> m_vectors;
    size_t                      m_tickCount = 0;
};

class EpidemicSystem {
public:
    static constexpr size_t MAX_ZONES = 64;

    EpidemicZone* createZone(const std::string& name) {
        if (m_zones.size() >= MAX_ZONES) return nullptr;
        for (auto& z : m_zones) if (z.name() == name) return nullptr;
        m_zones.emplace_back(name);
        return &m_zones.back();
    }

    [[nodiscard]] EpidemicZone* byName(const std::string& name) {
        for (auto& z : m_zones) if (z.name() == name) return &z;
        return nullptr;
    }

    void tick() {
        for (auto& z : m_zones) z.tick();
        m_tickCount++;
    }

    [[nodiscard]] size_t zoneCount()  const { return m_zones.size(); }
    [[nodiscard]] size_t tickCount()  const { return m_tickCount;    }

    [[nodiscard]] size_t criticalZoneCount() const {
        size_t c = 0;
        for (auto& z : m_zones) if (z.criticalCount() > 0) c++;
        return c;
    }

    [[nodiscard]] size_t containedZoneCount() const {
        size_t c = 0;
        for (auto& z : m_zones) if (z.isContained()) c++;
        return c;
    }

private:
    std::vector<EpidemicZone> m_zones;
    size_t                    m_tickCount = 0;
};

// ============================================================
// G47 — Solar Flare System
// ============================================================

enum class SolarFlareClass : uint8_t { A, B, C, M, X, S, N, Z };

inline const char* solarFlareClassName(SolarFlareClass c) {
    switch (c) {
        case SolarFlareClass::A: return "A";
        case SolarFlareClass::B: return "B";
        case SolarFlareClass::C: return "C";
        case SolarFlareClass::M: return "M";
        case SolarFlareClass::X: return "X";
        case SolarFlareClass::S: return "S";
        case SolarFlareClass::N: return "N";
        case SolarFlareClass::Z: return "Z";
        default:                 return "Unknown";
    }
}

enum class SolarFlareEffect : uint8_t { RadioBlackout, RadiationStorm, GeomagneticStorm, PowerGridDisruption, SatelliteDamage, CommunicationLoss };

inline const char* solarFlareEffectName(SolarFlareEffect e) {
    switch (e) {
        case SolarFlareEffect::RadioBlackout:        return "RadioBlackout";
        case SolarFlareEffect::RadiationStorm:       return "RadiationStorm";
        case SolarFlareEffect::GeomagneticStorm:     return "GeomagneticStorm";
        case SolarFlareEffect::PowerGridDisruption:  return "PowerGridDisruption";
        case SolarFlareEffect::SatelliteDamage:      return "SatelliteDamage";
        case SolarFlareEffect::CommunicationLoss:    return "CommunicationLoss";
        default:                                     return "Unknown";
    }
}

struct SolarFlareEvent {
    std::string     id;
    SolarFlareClass flareClass  = SolarFlareClass::C;
    float           intensity   = 1.0f;
    float           duration    = 60.f;
    bool            active      = false;

    void activate()   { active = true;  }
    void deactivate() { active = false; }

    [[nodiscard]] bool isMajor()    const { return flareClass >= SolarFlareClass::M; }
    [[nodiscard]] bool isExtreme()  const { return flareClass >= SolarFlareClass::X; }
    [[nodiscard]] float energyOutput() const { return intensity * duration; }
};

class SolarFlareRegion {
public:
    explicit SolarFlareRegion(const std::string& name) : m_name(name) {}

    bool addEvent(SolarFlareEvent e) {
        for (auto& existing : m_events) if (existing.id == e.id) return false;
        m_events.push_back(std::move(e));
        return true;
    }

    bool removeEvent(const std::string& id) {
        for (auto it = m_events.begin(); it != m_events.end(); ++it) {
            if (it->id == id) { m_events.erase(it); return true; }
        }
        return false;
    }

    [[nodiscard]] SolarFlareEvent* findEvent(const std::string& id) {
        for (auto& e : m_events) if (e.id == id) return &e;
        return nullptr;
    }

    void activateAll()   { for (auto& e : m_events) e.activate();   }
    void deactivateAll() { for (auto& e : m_events) e.deactivate(); }

    [[nodiscard]] size_t eventCount()  const { return m_events.size(); }

    [[nodiscard]] size_t activeCount() const {
        size_t c = 0;
        for (auto& e : m_events) if (e.active) c++;
        return c;
    }

    [[nodiscard]] size_t majorCount() const {
        size_t c = 0;
        for (auto& e : m_events) if (e.isMajor()) c++;
        return c;
    }

    [[nodiscard]] const std::string& name() const { return m_name; }

    void tick() { m_tickCount++; }
    [[nodiscard]] size_t tickCount() const { return m_tickCount; }

private:
    std::string                  m_name;
    std::vector<SolarFlareEvent> m_events;
    size_t                       m_tickCount = 0;
};

class SolarFlareSystem {
public:
    static constexpr size_t MAX_REGIONS = 32;

    SolarFlareRegion* createRegion(const std::string& name) {
        if (m_regions.size() >= MAX_REGIONS) return nullptr;
        for (auto& r : m_regions) if (r.name() == name) return nullptr;
        m_regions.emplace_back(name);
        return &m_regions.back();
    }

    [[nodiscard]] SolarFlareRegion* byName(const std::string& name) {
        for (auto& r : m_regions) if (r.name() == name) return &r;
        return nullptr;
    }

    void tick() {
        for (auto& r : m_regions) r.tick();
        m_tickCount++;
    }

    [[nodiscard]] size_t regionCount() const { return m_regions.size(); }
    [[nodiscard]] size_t tickCount()   const { return m_tickCount;      }

    [[nodiscard]] size_t activeEventCount() const {
        size_t c = 0;
        for (auto& r : m_regions) c += r.activeCount();
        return c;
    }

    [[nodiscard]] size_t majorEventCount() const {
        size_t c = 0;
        for (auto& r : m_regions) c += r.majorCount();
        return c;
    }

private:
    std::vector<SolarFlareRegion> m_regions;
    size_t                        m_tickCount = 0;
};

// ============================================================
// G48 — Meteor Shower System
// ============================================================

enum class MeteorShowerClass : uint8_t {
    Sporadic  = 0,
    Minor     = 1,
    Moderate  = 2,
    Major     = 3,
    Annual    = 4,
    Periodic  = 5,
    Storm     = 6,
    Outburst  = 7
};

inline const char* meteorShowerClassName(MeteorShowerClass c) {
    switch (c) {
        case MeteorShowerClass::Sporadic:  return "Sporadic";
        case MeteorShowerClass::Minor:     return "Minor";
        case MeteorShowerClass::Moderate:  return "Moderate";
        case MeteorShowerClass::Major:     return "Major";
        case MeteorShowerClass::Annual:    return "Annual";
        case MeteorShowerClass::Periodic:  return "Periodic";
        case MeteorShowerClass::Storm:     return "Storm";
        case MeteorShowerClass::Outburst:  return "Outburst";
        default:                           return "Unknown";
    }
}

enum class MeteorImpactType : uint8_t {
    Airburst      = 0,
    CraterFormation = 1,
    Graze         = 2,
    Ablation      = 3,
    Penetrating   = 4,
    Fragmentation = 5
};

inline const char* meteorImpactTypeName(MeteorImpactType t) {
    switch (t) {
        case MeteorImpactType::Airburst:       return "Airburst";
        case MeteorImpactType::CraterFormation:return "CraterFormation";
        case MeteorImpactType::Graze:          return "Graze";
        case MeteorImpactType::Ablation:       return "Ablation";
        case MeteorImpactType::Penetrating:    return "Penetrating";
        case MeteorImpactType::Fragmentation:  return "Fragmentation";
        default:                               return "Unknown";
    }
}

struct MeteorEvent {
    std::string       id;
    MeteorShowerClass showerClass = MeteorShowerClass::Minor;
    float             intensity   = 1.f;
    float             duration    = 1.f;
    MeteorImpactType  impactType  = MeteorImpactType::Ablation;
    bool              active      = false;

    void activate()   { active = true;  }
    void deactivate() { active = false; }

    [[nodiscard]] bool isMinor() const {
        return static_cast<uint8_t>(showerClass) < static_cast<uint8_t>(MeteorShowerClass::Moderate);
    }

    [[nodiscard]] bool isMajor() const {
        return static_cast<uint8_t>(showerClass) >= static_cast<uint8_t>(MeteorShowerClass::Major);
    }

    [[nodiscard]] float radiationFlux() const { return intensity * duration; }
};

class MeteorShowerRegion {
public:
    explicit MeteorShowerRegion(const std::string& name) : m_name(name) {}

    bool addEvent(const MeteorEvent& ev) {
        for (auto& existing : m_events) if (existing.id == ev.id) return false;
        m_events.push_back(ev);
        return true;
    }

    bool removeEvent(const std::string& id) {
        for (auto it = m_events.begin(); it != m_events.end(); ++it) {
            if (it->id == id) { m_events.erase(it); return true; }
        }
        return false;
    }

    [[nodiscard]] MeteorEvent* findEvent(const std::string& id) {
        for (auto& ev : m_events) if (ev.id == id) return &ev;
        return nullptr;
    }

    void activateAll()   { for (auto& ev : m_events) ev.activate();   }
    void deactivateAll() { for (auto& ev : m_events) ev.deactivate(); }

    [[nodiscard]] size_t eventCount() const { return m_events.size(); }

    [[nodiscard]] size_t activeCount() const {
        size_t c = 0;
        for (auto& ev : m_events) if (ev.active) c++;
        return c;
    }

    [[nodiscard]] size_t majorCount() const {
        size_t c = 0;
        for (auto& ev : m_events) if (ev.isMajor()) c++;
        return c;
    }

    [[nodiscard]] const std::string& name()      const { return m_name;      }
    [[nodiscard]] size_t             tickCount() const { return m_tickCount; }

    void tick() { m_tickCount++; }

private:
    std::string              m_name;
    std::vector<MeteorEvent> m_events;
    size_t                   m_tickCount = 0;
};

class MeteorShowerSystem {
public:
    static constexpr size_t MAX_REGIONS = 32;

    MeteorShowerRegion* createRegion(const std::string& name) {
        if (m_regions.size() >= MAX_REGIONS) return nullptr;
        for (auto& r : m_regions) if (r.name() == name) return nullptr;
        m_regions.emplace_back(name);
        return &m_regions.back();
    }

    [[nodiscard]] MeteorShowerRegion* byName(const std::string& name) {
        for (auto& r : m_regions) if (r.name() == name) return &r;
        return nullptr;
    }

    void tick() {
        for (auto& r : m_regions) r.tick();
        m_tickCount++;
    }

    [[nodiscard]] size_t regionCount() const { return m_regions.size();  }
    [[nodiscard]] size_t tickCount()   const { return m_tickCount;        }

    [[nodiscard]] size_t activeEventCount() const {
        size_t c = 0;
        for (auto& r : m_regions) c += r.activeCount();
        return c;
    }

    [[nodiscard]] size_t majorEventCount() const {
        size_t c = 0;
        for (auto& r : m_regions) c += r.majorCount();
        return c;
    }

private:
    std::vector<MeteorShowerRegion> m_regions;
    size_t                          m_tickCount = 0;
};

// G49 — Aurora System

enum class AuroraType : uint8_t {
    Borealis, Australis, Polar, Substorm, Diffuse, Discrete, Pulsating, Custom
};

inline const char* auroraTypeName(AuroraType t) {
    switch (t) {
        case AuroraType::Borealis:  return "Borealis";
        case AuroraType::Australis: return "Australis";
        case AuroraType::Polar:     return "Polar";
        case AuroraType::Substorm:  return "Substorm";
        case AuroraType::Diffuse:   return "Diffuse";
        case AuroraType::Discrete:  return "Discrete";
        case AuroraType::Pulsating: return "Pulsating";
        case AuroraType::Custom:    return "Custom";
    }
    return "Unknown";
}

enum class AuroraIntensity : uint8_t {
    Faint, Quiet, Active, Storm, Severe, Extreme
};

inline const char* auroraIntensityName(AuroraIntensity i) {
    switch (i) {
        case AuroraIntensity::Faint:   return "Faint";
        case AuroraIntensity::Quiet:   return "Quiet";
        case AuroraIntensity::Active:  return "Active";
        case AuroraIntensity::Storm:   return "Storm";
        case AuroraIntensity::Severe:  return "Severe";
        case AuroraIntensity::Extreme: return "Extreme";
    }
    return "Unknown";
}

struct AuroraEvent {
    std::string     id;
    AuroraType      type       = AuroraType::Borealis;
    AuroraIntensity intensity  = AuroraIntensity::Quiet;
    float           duration   = 1.0f;
    float           colorShift = 0.0f;
    bool            active     = false;

    void activate()   { active = true;  }
    void deactivate() { active = false; }

    [[nodiscard]] bool isVisible() const {
        return static_cast<uint8_t>(intensity) >= static_cast<uint8_t>(AuroraIntensity::Active);
    }
    [[nodiscard]] bool isStorm() const {
        return static_cast<uint8_t>(intensity) >= static_cast<uint8_t>(AuroraIntensity::Storm);
    }
    [[nodiscard]] float radiationIndex() const {
        return (static_cast<float>(static_cast<uint8_t>(intensity)) + 1.0f) * duration;
    }
};

class AuroraRegion {
public:
    explicit AuroraRegion(const std::string& name) : m_name(name) {}

    [[nodiscard]] bool addEvent(const AuroraEvent& ev) {
        for (auto& e : m_events) if (e.id == ev.id) return false;
        m_events.push_back(ev);
        return true;
    }

    [[nodiscard]] bool removeEvent(const std::string& id) {
        for (auto it = m_events.begin(); it != m_events.end(); ++it) {
            if (it->id == id) { m_events.erase(it); return true; }
        }
        return false;
    }

    [[nodiscard]] AuroraEvent* findEvent(const std::string& id) {
        for (auto& e : m_events) if (e.id == id) return &e;
        return nullptr;
    }

    void activateAll()   { for (auto& e : m_events) e.activate();   }
    void deactivateAll() { for (auto& e : m_events) e.deactivate(); }

    [[nodiscard]] size_t eventCount()  const { return m_events.size(); }
    [[nodiscard]] size_t activeCount() const {
        size_t c = 0; for (auto& e : m_events) if (e.active) ++c; return c;
    }
    [[nodiscard]] size_t stormCount() const {
        size_t c = 0; for (auto& e : m_events) if (e.active && e.isStorm()) ++c; return c;
    }

    void tick()  { ++m_tickCount; }
    [[nodiscard]] const std::string& name()      const { return m_name; }
    [[nodiscard]] size_t             tickCount() const { return m_tickCount; }

private:
    std::string              m_name;
    std::vector<AuroraEvent> m_events;
    size_t                   m_tickCount = 0;
};

class AuroraSystem {
public:
    static constexpr size_t MAX_REGIONS = 32;

    AuroraRegion* createRegion(const std::string& name) {
        for (auto& r : m_regions) if (r.name() == name) return nullptr;
        if (m_regions.size() >= MAX_REGIONS) return nullptr;
        m_regions.emplace_back(name);
        return &m_regions.back();
    }

    [[nodiscard]] AuroraRegion* byName(const std::string& name) {
        for (auto& r : m_regions) if (r.name() == name) return &r;
        return nullptr;
    }

    void tick() {
        ++m_tickCount;
        for (auto& r : m_regions) r.tick();
    }

    [[nodiscard]] size_t regionCount() const { return m_regions.size(); }
    [[nodiscard]] size_t tickCount()   const { return m_tickCount; }

    [[nodiscard]] size_t activeEventCount() const {
        size_t c = 0;
        for (auto& r : m_regions) c += r.activeCount();
        return c;
    }

    [[nodiscard]] size_t stormEventCount() const {
        size_t c = 0;
        for (auto& r : m_regions) c += r.stormCount();
        return c;
    }

private:
    std::vector<AuroraRegion> m_regions;
    size_t                    m_tickCount = 0;
};

// G50 — Heatwave System

enum class HeatwaveType : uint8_t {
    Regional, Urban, Coastal, Continental, Desert, Tropical, Polar, Custom
};

inline const char* heatwaveTypeName(HeatwaveType t) {
    switch (t) {
        case HeatwaveType::Regional:     return "Regional";
        case HeatwaveType::Urban:        return "Urban";
        case HeatwaveType::Coastal:      return "Coastal";
        case HeatwaveType::Continental:  return "Continental";
        case HeatwaveType::Desert:       return "Desert";
        case HeatwaveType::Tropical:     return "Tropical";
        case HeatwaveType::Polar:        return "Polar";
        case HeatwaveType::Custom:       return "Custom";
    }
    return "Unknown";
}

enum class HeatwaveSeverity : uint8_t {
    Mild, Moderate, Severe, Extreme, Catastrophic, Unprecedented
};

inline const char* heatwaveSeverityName(HeatwaveSeverity s) {
    switch (s) {
        case HeatwaveSeverity::Mild:          return "Mild";
        case HeatwaveSeverity::Moderate:      return "Moderate";
        case HeatwaveSeverity::Severe:        return "Severe";
        case HeatwaveSeverity::Extreme:       return "Extreme";
        case HeatwaveSeverity::Catastrophic:  return "Catastrophic";
        case HeatwaveSeverity::Unprecedented: return "Unprecedented";
    }
    return "Unknown";
}

struct HeatwaveEvent {
    std::string      id;
    HeatwaveType     type       = HeatwaveType::Regional;
    HeatwaveSeverity severity   = HeatwaveSeverity::Mild;
    float            duration   = 1.0f;
    float            peakTemp   = 30.0f;
    bool             active     = false;

    void activate()   { active = true;  }
    void deactivate() { active = false; }

    [[nodiscard]] bool isDangerous() const {
        return static_cast<uint8_t>(severity) >= static_cast<uint8_t>(HeatwaveSeverity::Severe);
    }
    [[nodiscard]] bool isExtreme() const {
        return static_cast<uint8_t>(severity) >= static_cast<uint8_t>(HeatwaveSeverity::Extreme);
    }
    [[nodiscard]] float heatIndex() const {
        return (static_cast<float>(static_cast<uint8_t>(severity)) + 1.0f) * peakTemp;
    }
};

class HeatwaveRegion {
public:
    explicit HeatwaveRegion(const std::string& name) : m_name(name) {}

    [[nodiscard]] bool addEvent(const HeatwaveEvent& ev) {
        for (auto& e : m_events) if (e.id == ev.id) return false;
        m_events.push_back(ev);
        return true;
    }

    [[nodiscard]] bool removeEvent(const std::string& id) {
        for (auto it = m_events.begin(); it != m_events.end(); ++it) {
            if (it->id == id) { m_events.erase(it); return true; }
        }
        return false;
    }

    [[nodiscard]] HeatwaveEvent* findEvent(const std::string& id) {
        for (auto& e : m_events) if (e.id == id) return &e;
        return nullptr;
    }

    void activateAll()   { for (auto& e : m_events) e.activate();   }
    void deactivateAll() { for (auto& e : m_events) e.deactivate(); }

    [[nodiscard]] size_t eventCount()     const { return m_events.size(); }
    [[nodiscard]] size_t activeCount()    const {
        size_t c = 0; for (auto& e : m_events) if (e.active) ++c; return c;
    }
    [[nodiscard]] size_t extremeCount()   const {
        size_t c = 0; for (auto& e : m_events) if (e.active && e.isExtreme()) ++c; return c;
    }

    void tick()  { ++m_tickCount; }
    [[nodiscard]] const std::string& name()      const { return m_name; }
    [[nodiscard]] size_t             tickCount() const { return m_tickCount; }

private:
    std::string               m_name;
    std::vector<HeatwaveEvent> m_events;
    size_t                    m_tickCount = 0;
};

class HeatwaveSystem {
public:
    static constexpr size_t MAX_REGIONS = 32;

    HeatwaveRegion* createRegion(const std::string& name) {
        for (auto& r : m_regions) if (r.name() == name) return nullptr;
        if (m_regions.size() >= MAX_REGIONS) return nullptr;
        m_regions.emplace_back(name);
        return &m_regions.back();
    }

    [[nodiscard]] HeatwaveRegion* byName(const std::string& name) {
        for (auto& r : m_regions) if (r.name() == name) return &r;
        return nullptr;
    }

    void tick() {
        ++m_tickCount;
        for (auto& r : m_regions) r.tick();
    }

    [[nodiscard]] size_t regionCount() const { return m_regions.size(); }
    [[nodiscard]] size_t tickCount()   const { return m_tickCount; }

    [[nodiscard]] size_t activeEventCount() const {
        size_t c = 0;
        for (auto& r : m_regions) c += r.activeCount();
        return c;
    }

    [[nodiscard]] size_t extremeEventCount() const {
        size_t c = 0;
        for (auto& r : m_regions) c += r.extremeCount();
        return c;
    }

private:
    std::vector<HeatwaveRegion> m_regions;
    size_t                      m_tickCount = 0;
};

// G51 — Blizzard System

enum class BlizzardType : uint8_t {
    Arctic, Alpine, Coastal, Prairie, Lake, Polar, Nor_easter, Custom
};

inline const char* blizzardTypeName(BlizzardType t) {
    switch (t) {
        case BlizzardType::Arctic:    return "Arctic";
        case BlizzardType::Alpine:    return "Alpine";
        case BlizzardType::Coastal:   return "Coastal";
        case BlizzardType::Prairie:   return "Prairie";
        case BlizzardType::Lake:      return "Lake";
        case BlizzardType::Polar:     return "Polar";
        case BlizzardType::Nor_easter: return "Nor'easter";
        case BlizzardType::Custom:    return "Custom";
    }
    return "Unknown";
}

enum class BlizzardIntensity : uint8_t {
    Light, Moderate, Heavy, Severe, Extreme, Whiteout
};

inline const char* blizzardIntensityName(BlizzardIntensity i) {
    switch (i) {
        case BlizzardIntensity::Light:    return "Light";
        case BlizzardIntensity::Moderate: return "Moderate";
        case BlizzardIntensity::Heavy:    return "Heavy";
        case BlizzardIntensity::Severe:   return "Severe";
        case BlizzardIntensity::Extreme:  return "Extreme";
        case BlizzardIntensity::Whiteout: return "Whiteout";
    }
    return "Unknown";
}

struct BlizzardEvent {
    std::string       id;
    BlizzardType      type      = BlizzardType::Arctic;
    BlizzardIntensity intensity = BlizzardIntensity::Light;
    float             duration  = 1.0f;
    float             windSpeed = 0.0f;
    bool              active    = false;

    void activate()   { active = true;  }
    void deactivate() { active = false; }

    [[nodiscard]] bool isSevere() const {
        return static_cast<uint8_t>(intensity) >= static_cast<uint8_t>(BlizzardIntensity::Severe);
    }
    [[nodiscard]] bool isWhiteout() const {
        return intensity == BlizzardIntensity::Whiteout;
    }
    [[nodiscard]] float visibilityIndex() const {
        float base = static_cast<float>(static_cast<uint8_t>(BlizzardIntensity::Whiteout)
                                        - static_cast<uint8_t>(intensity));
        return base;
    }
};

class BlizzardRegion {
public:
    explicit BlizzardRegion(const std::string& name) : m_name(name) {}

    [[nodiscard]] bool addEvent(const BlizzardEvent& ev) {
        for (auto& e : m_events) if (e.id == ev.id) return false;
        m_events.push_back(ev);
        return true;
    }

    [[nodiscard]] bool removeEvent(const std::string& id) {
        for (auto it = m_events.begin(); it != m_events.end(); ++it) {
            if (it->id == id) { m_events.erase(it); return true; }
        }
        return false;
    }

    [[nodiscard]] BlizzardEvent* findEvent(const std::string& id) {
        for (auto& e : m_events) if (e.id == id) return &e;
        return nullptr;
    }

    void activateAll()   { for (auto& e : m_events) e.activate();   }
    void deactivateAll() { for (auto& e : m_events) e.deactivate(); }

    [[nodiscard]] size_t eventCount()   const { return m_events.size(); }
    [[nodiscard]] size_t activeCount()  const {
        size_t c = 0; for (auto& e : m_events) if (e.active) ++c; return c;
    }
    [[nodiscard]] size_t severeCount()  const {
        size_t c = 0; for (auto& e : m_events) if (e.active && e.isSevere()) ++c; return c;
    }
    [[nodiscard]] size_t whiteoutCount() const {
        size_t c = 0; for (auto& e : m_events) if (e.active && e.isWhiteout()) ++c; return c;
    }

    void tick()  { ++m_tickCount; }
    [[nodiscard]] const std::string& name()      const { return m_name; }
    [[nodiscard]] size_t             tickCount() const { return m_tickCount; }

private:
    std::string               m_name;
    std::vector<BlizzardEvent> m_events;
    size_t                    m_tickCount = 0;
};

class BlizzardSystem {
public:
    static constexpr size_t MAX_REGIONS = 32;

    BlizzardRegion* createRegion(const std::string& name) {
        for (auto& r : m_regions) if (r.name() == name) return nullptr;
        if (m_regions.size() >= MAX_REGIONS) return nullptr;
        m_regions.emplace_back(name);
        return &m_regions.back();
    }

    [[nodiscard]] BlizzardRegion* byName(const std::string& name) {
        for (auto& r : m_regions) if (r.name() == name) return &r;
        return nullptr;
    }

    void tick() {
        ++m_tickCount;
        for (auto& r : m_regions) r.tick();
    }

    [[nodiscard]] size_t regionCount() const { return m_regions.size(); }
    [[nodiscard]] size_t tickCount()   const { return m_tickCount; }

    [[nodiscard]] size_t activeEventCount() const {
        size_t c = 0;
        for (auto& r : m_regions) c += r.activeCount();
        return c;
    }

    [[nodiscard]] size_t severeEventCount() const {
        size_t c = 0;
        for (auto& r : m_regions) c += r.severeCount();
        return c;
    }

    [[nodiscard]] size_t whiteoutEventCount() const {
        size_t c = 0;
        for (auto& r : m_regions) c += r.whiteoutCount();
        return c;
    }

private:
    std::vector<BlizzardRegion> m_regions;
    size_t                      m_tickCount = 0;
};

// ── G52 — Sandstorm System ────────────────────────────────────────

enum class SandstormType : uint8_t {
    Haboob, Simoom, Khamsin, Shamal, Harmattan, Sirocco, Zonda, Custom
};

inline const char* sandstormTypeName(SandstormType t) {
    switch (t) {
        case SandstormType::Haboob:   return "Haboob";
        case SandstormType::Simoom:   return "Simoom";
        case SandstormType::Khamsin:  return "Khamsin";
        case SandstormType::Shamal:   return "Shamal";
        case SandstormType::Harmattan: return "Harmattan";
        case SandstormType::Sirocco:  return "Sirocco";
        case SandstormType::Zonda:    return "Zonda";
        case SandstormType::Custom:   return "Custom";
    }
    return "Unknown";
}

enum class SandstormSeverity : uint8_t {
    Dust, Moderate, Strong, Severe, Violent, Catastrophic
};

inline const char* sandstormSeverityName(SandstormSeverity s) {
    switch (s) {
        case SandstormSeverity::Dust:         return "Dust";
        case SandstormSeverity::Moderate:     return "Moderate";
        case SandstormSeverity::Strong:       return "Strong";
        case SandstormSeverity::Severe:       return "Severe";
        case SandstormSeverity::Violent:      return "Violent";
        case SandstormSeverity::Catastrophic: return "Catastrophic";
    }
    return "Unknown";
}

struct SandstormEvent {
    std::string       id;
    SandstormType     type      = SandstormType::Haboob;
    SandstormSeverity severity  = SandstormSeverity::Dust;
    float             duration  = 1.0f;
    float             windSpeed = 0.0f;
    bool              active    = false;

    void activate()   { active = true;  }
    void deactivate() { active = false; }

    [[nodiscard]] bool isSevere() const {
        return static_cast<uint8_t>(severity) >= static_cast<uint8_t>(SandstormSeverity::Severe);
    }
    [[nodiscard]] bool isCatastrophic() const {
        return severity == SandstormSeverity::Catastrophic;
    }
    [[nodiscard]] float particleDensity() const {
        return static_cast<float>(static_cast<uint8_t>(severity) + 1) * windSpeed;
    }
};

class SandstormRegion {
public:
    explicit SandstormRegion(const std::string& name) : m_name(name) {}

    [[nodiscard]] bool addEvent(const SandstormEvent& ev) {
        for (auto& e : m_events) if (e.id == ev.id) return false;
        m_events.push_back(ev);
        return true;
    }

    [[nodiscard]] bool removeEvent(const std::string& id) {
        for (auto it = m_events.begin(); it != m_events.end(); ++it) {
            if (it->id == id) { m_events.erase(it); return true; }
        }
        return false;
    }

    [[nodiscard]] SandstormEvent* findEvent(const std::string& id) {
        for (auto& e : m_events) if (e.id == id) return &e;
        return nullptr;
    }

    void activateAll()   { for (auto& e : m_events) e.activate();   }
    void deactivateAll() { for (auto& e : m_events) e.deactivate(); }

    [[nodiscard]] size_t eventCount()        const { return m_events.size(); }
    [[nodiscard]] size_t activeCount()       const {
        size_t c = 0; for (auto& e : m_events) if (e.active) ++c; return c;
    }
    [[nodiscard]] size_t severeCount()       const {
        size_t c = 0; for (auto& e : m_events) if (e.active && e.isSevere()) ++c; return c;
    }
    [[nodiscard]] size_t catastrophicCount() const {
        size_t c = 0; for (auto& e : m_events) if (e.active && e.isCatastrophic()) ++c; return c;
    }

    void tick()  { ++m_tickCount; }
    [[nodiscard]] const std::string& name()      const { return m_name; }
    [[nodiscard]] size_t             tickCount() const { return m_tickCount; }

private:
    std::string                m_name;
    std::vector<SandstormEvent> m_events;
    size_t                     m_tickCount = 0;
};

class SandstormSystem {
public:
    static constexpr size_t MAX_REGIONS = 32;

    SandstormRegion* createRegion(const std::string& name) {
        for (auto& r : m_regions) if (r.name() == name) return nullptr;
        if (m_regions.size() >= MAX_REGIONS) return nullptr;
        m_regions.emplace_back(name);
        return &m_regions.back();
    }

    [[nodiscard]] SandstormRegion* byName(const std::string& name) {
        for (auto& r : m_regions) if (r.name() == name) return &r;
        return nullptr;
    }

    void tick() {
        ++m_tickCount;
        for (auto& r : m_regions) r.tick();
    }

    [[nodiscard]] size_t regionCount() const { return m_regions.size(); }
    [[nodiscard]] size_t tickCount()   const { return m_tickCount; }

    [[nodiscard]] size_t activeEventCount() const {
        size_t c = 0; for (auto& r : m_regions) c += r.activeCount(); return c;
    }
    [[nodiscard]] size_t severeEventCount() const {
        size_t c = 0; for (auto& r : m_regions) c += r.severeCount(); return c;
    }
    [[nodiscard]] size_t catastrophicEventCount() const {
        size_t c = 0; for (auto& r : m_regions) c += r.catastrophicCount(); return c;
    }

private:
    std::vector<SandstormRegion> m_regions;
    size_t                       m_tickCount = 0;
};

// ── G53 — Cyclone System ──────────────────────────────────────────

enum class CycloneCategory : uint8_t {
    TropicalDepression, TropicalStorm, Cat1, Cat2, Cat3, Cat4
};

inline const char* cycloneCategoryName(CycloneCategory c) {
    switch (c) {
        case CycloneCategory::TropicalDepression: return "TropicalDepression";
        case CycloneCategory::TropicalStorm:      return "TropicalStorm";
        case CycloneCategory::Cat1:               return "Cat1";
        case CycloneCategory::Cat2:               return "Cat2";
        case CycloneCategory::Cat3:               return "Cat3";
        case CycloneCategory::Cat4:               return "Cat4";
    }
    return "Unknown";
}

enum class CycloneStage : uint8_t {
    Forming, Intensifying, Peak, Weakening, Dissipating, Remnant
};

inline const char* cycloneStageName(CycloneStage s) {
    switch (s) {
        case CycloneStage::Forming:      return "Forming";
        case CycloneStage::Intensifying: return "Intensifying";
        case CycloneStage::Peak:         return "Peak";
        case CycloneStage::Weakening:    return "Weakening";
        case CycloneStage::Dissipating:  return "Dissipating";
        case CycloneStage::Remnant:      return "Remnant";
    }
    return "Unknown";
}

struct CycloneEvent {
    std::string    id;
    CycloneCategory category = CycloneCategory::TropicalDepression;
    CycloneStage    stage    = CycloneStage::Forming;
    float           windSpeed = 0.0f;
    float           pressure  = 1013.0f;
    bool            active    = false;

    void activate()   { active = true;  }
    void deactivate() { active = false; }

    [[nodiscard]] bool isMajor() const {
        return static_cast<uint8_t>(category) >= static_cast<uint8_t>(CycloneCategory::Cat3);
    }
    [[nodiscard]] bool isAtPeak() const { return stage == CycloneStage::Peak; }
    [[nodiscard]] float stormSurge() const {
        return (static_cast<float>(static_cast<uint8_t>(category) + 1)) * 0.5f * windSpeed / 100.0f;
    }
};

class CycloneRegion {
public:
    explicit CycloneRegion(const std::string& name) : m_name(name) {}

    [[nodiscard]] bool addEvent(const CycloneEvent& ev) {
        for (auto& e : m_events) if (e.id == ev.id) return false;
        m_events.push_back(ev);
        return true;
    }

    [[nodiscard]] bool removeEvent(const std::string& id) {
        for (auto it = m_events.begin(); it != m_events.end(); ++it) {
            if (it->id == id) { m_events.erase(it); return true; }
        }
        return false;
    }

    [[nodiscard]] CycloneEvent* findEvent(const std::string& id) {
        for (auto& e : m_events) if (e.id == id) return &e;
        return nullptr;
    }

    void activateAll()   { for (auto& e : m_events) e.activate();   }
    void deactivateAll() { for (auto& e : m_events) e.deactivate(); }

    [[nodiscard]] size_t eventCount()  const { return m_events.size(); }
    [[nodiscard]] size_t activeCount() const {
        size_t c = 0; for (auto& e : m_events) if (e.active) ++c; return c;
    }
    [[nodiscard]] size_t majorCount()  const {
        size_t c = 0; for (auto& e : m_events) if (e.active && e.isMajor()) ++c; return c;
    }
    [[nodiscard]] size_t peakCount()   const {
        size_t c = 0; for (auto& e : m_events) if (e.active && e.isAtPeak()) ++c; return c;
    }

    void tick() { ++m_tickCount; }
    [[nodiscard]] const std::string& name()      const { return m_name; }
    [[nodiscard]] size_t             tickCount() const { return m_tickCount; }

private:
    std::string               m_name;
    std::vector<CycloneEvent> m_events;
    size_t                    m_tickCount = 0;
};

class CycloneSystem {
public:
    static constexpr size_t MAX_REGIONS = 32;

    CycloneRegion* createRegion(const std::string& name) {
        for (auto& r : m_regions) if (r.name() == name) return nullptr;
        if (m_regions.size() >= MAX_REGIONS) return nullptr;
        m_regions.emplace_back(name);
        return &m_regions.back();
    }

    [[nodiscard]] CycloneRegion* byName(const std::string& name) {
        for (auto& r : m_regions) if (r.name() == name) return &r;
        return nullptr;
    }

    void tick() {
        ++m_tickCount;
        for (auto& r : m_regions) r.tick();
    }

    [[nodiscard]] size_t regionCount() const { return m_regions.size(); }
    [[nodiscard]] size_t tickCount()   const { return m_tickCount; }

    [[nodiscard]] size_t activeEventCount() const {
        size_t c = 0; for (auto& r : m_regions) c += r.activeCount(); return c;
    }
    [[nodiscard]] size_t majorEventCount()  const {
        size_t c = 0; for (auto& r : m_regions) c += r.majorCount(); return c;
    }
    [[nodiscard]] size_t peakEventCount()   const {
        size_t c = 0; for (auto& r : m_regions) c += r.peakCount(); return c;
    }

private:
    std::vector<CycloneRegion> m_regions;
    size_t                     m_tickCount = 0;
};

// ── G54 — Tornado System ──────────────────────────────────────────

enum class TornadoScale : uint8_t {
    EF0, EF1, EF2, EF3, EF4, EF5
};

inline const char* tornadoScaleName(TornadoScale s) {
    switch (s) {
        case TornadoScale::EF0: return "EF0";
        case TornadoScale::EF1: return "EF1";
        case TornadoScale::EF2: return "EF2";
        case TornadoScale::EF3: return "EF3";
        case TornadoScale::EF4: return "EF4";
        case TornadoScale::EF5: return "EF5";
    }
    return "Unknown";
}

enum class TornadoStage : uint8_t {
    Organizing, Mature, Shrinking, Rope, Dissipating, Remnant
};

inline const char* tornadoStageName(TornadoStage s) {
    switch (s) {
        case TornadoStage::Organizing:  return "Organizing";
        case TornadoStage::Mature:      return "Mature";
        case TornadoStage::Shrinking:   return "Shrinking";
        case TornadoStage::Rope:        return "Rope";
        case TornadoStage::Dissipating: return "Dissipating";
        case TornadoStage::Remnant:     return "Remnant";
    }
    return "Unknown";
}

struct TornadoEvent {
    std::string  id;
    TornadoScale scale    = TornadoScale::EF0;
    TornadoStage stage    = TornadoStage::Organizing;
    float        windSpeed = 0.0f;
    float        pathLength = 0.0f;
    bool         active   = false;

    void activate()   { active = true;  }
    void deactivate() { active = false; }

    [[nodiscard]] bool isViolent() const {
        return static_cast<uint8_t>(scale) >= static_cast<uint8_t>(TornadoScale::EF4);
    }
    [[nodiscard]] bool isAtPeak() const { return stage == TornadoStage::Mature; }
    [[nodiscard]] float destructionIndex() const {
        return (static_cast<float>(static_cast<uint8_t>(scale) + 1)) * windSpeed / 100.0f;
    }
};

class TornadoRegion {
public:
    explicit TornadoRegion(const std::string& name) : m_name(name) {}

    [[nodiscard]] bool addEvent(const TornadoEvent& ev) {
        for (auto& e : m_events) if (e.id == ev.id) return false;
        m_events.push_back(ev);
        return true;
    }

    [[nodiscard]] bool removeEvent(const std::string& id) {
        for (auto it = m_events.begin(); it != m_events.end(); ++it) {
            if (it->id == id) { m_events.erase(it); return true; }
        }
        return false;
    }

    [[nodiscard]] TornadoEvent* findEvent(const std::string& id) {
        for (auto& e : m_events) if (e.id == id) return &e;
        return nullptr;
    }

    void activateAll()   { for (auto& e : m_events) e.activate();   }
    void deactivateAll() { for (auto& e : m_events) e.deactivate(); }

    [[nodiscard]] size_t eventCount()    const { return m_events.size(); }
    [[nodiscard]] size_t activeCount()   const {
        size_t c = 0; for (auto& e : m_events) if (e.active) ++c; return c;
    }
    [[nodiscard]] size_t violentCount()  const {
        size_t c = 0; for (auto& e : m_events) if (e.active && e.isViolent()) ++c; return c;
    }
    [[nodiscard]] size_t peakCount()     const {
        size_t c = 0; for (auto& e : m_events) if (e.active && e.isAtPeak()) ++c; return c;
    }

    void tick() { ++m_tickCount; }
    [[nodiscard]] const std::string& name()      const { return m_name; }
    [[nodiscard]] size_t             tickCount() const { return m_tickCount; }

private:
    std::string               m_name;
    std::vector<TornadoEvent> m_events;
    size_t                    m_tickCount = 0;
};

class TornadoSystem {
public:
    static constexpr size_t MAX_REGIONS = 32;

    TornadoRegion* createRegion(const std::string& name) {
        for (auto& r : m_regions) if (r.name() == name) return nullptr;
        if (m_regions.size() >= MAX_REGIONS) return nullptr;
        m_regions.emplace_back(name);
        return &m_regions.back();
    }

    [[nodiscard]] TornadoRegion* byName(const std::string& name) {
        for (auto& r : m_regions) if (r.name() == name) return &r;
        return nullptr;
    }

    void tick() {
        ++m_tickCount;
        for (auto& r : m_regions) r.tick();
    }

    [[nodiscard]] size_t regionCount() const { return m_regions.size(); }
    [[nodiscard]] size_t tickCount()   const { return m_tickCount; }

    [[nodiscard]] size_t activeEventCount()  const {
        size_t c = 0; for (auto& r : m_regions) c += r.activeCount(); return c;
    }
    [[nodiscard]] size_t violentEventCount() const {
        size_t c = 0; for (auto& r : m_regions) c += r.violentCount(); return c;
    }
    [[nodiscard]] size_t peakEventCount()    const {
        size_t c = 0; for (auto& r : m_regions) c += r.peakCount(); return c;
    }

private:
    std::vector<TornadoRegion> m_regions;
    size_t                     m_tickCount = 0;
};

// ── G55 — Dust Storm System ───────────────────────────────────────

enum class DustDensity : uint8_t {
    Trace, Light, Moderate, Heavy, Extreme
};

inline const char* dustDensityName(DustDensity d) {
    switch (d) {
        case DustDensity::Trace:    return "Trace";
        case DustDensity::Light:    return "Light";
        case DustDensity::Moderate: return "Moderate";
        case DustDensity::Heavy:    return "Heavy";
        case DustDensity::Extreme:  return "Extreme";
    }
    return "Unknown";
}

enum class DustStormPhase : uint8_t {
    Forming, Advancing, Peak, Retreating, Clearing
};

inline const char* dustStormPhaseName(DustStormPhase p) {
    switch (p) {
        case DustStormPhase::Forming:    return "Forming";
        case DustStormPhase::Advancing:  return "Advancing";
        case DustStormPhase::Peak:       return "Peak";
        case DustStormPhase::Retreating: return "Retreating";
        case DustStormPhase::Clearing:   return "Clearing";
    }
    return "Unknown";
}

struct DustStormEvent {
    std::string    id;
    DustDensity    density   = DustDensity::Light;
    DustStormPhase phase     = DustStormPhase::Forming;
    float          windSpeed = 0.0f;
    float          visibility = 1000.0f; // metres
    bool           active    = false;

    void activate()   { active = true;  }
    void deactivate() { active = false; }

    [[nodiscard]] bool isHazardous() const {
        return static_cast<uint8_t>(density) >= static_cast<uint8_t>(DustDensity::Heavy);
    }
    [[nodiscard]] bool isAtPeak() const { return phase == DustStormPhase::Peak; }
    [[nodiscard]] bool isLowVisibility() const { return visibility < 200.0f; }
    [[nodiscard]] float hazardScore() const {
        return (static_cast<float>(static_cast<uint8_t>(density) + 1)) * windSpeed / 50.0f;
    }
};

class DustStormRegion {
public:
    explicit DustStormRegion(const std::string& name) : m_name(name) {}

    [[nodiscard]] bool addEvent(const DustStormEvent& ev) {
        for (auto& e : m_events) if (e.id == ev.id) return false;
        m_events.push_back(ev);
        return true;
    }

    [[nodiscard]] bool removeEvent(const std::string& id) {
        for (auto it = m_events.begin(); it != m_events.end(); ++it) {
            if (it->id == id) { m_events.erase(it); return true; }
        }
        return false;
    }

    [[nodiscard]] DustStormEvent* findEvent(const std::string& id) {
        for (auto& e : m_events) if (e.id == id) return &e;
        return nullptr;
    }

    void activateAll()   { for (auto& e : m_events) e.activate();   }
    void deactivateAll() { for (auto& e : m_events) e.deactivate(); }

    [[nodiscard]] size_t eventCount()     const { return m_events.size(); }
    [[nodiscard]] size_t activeCount()    const {
        size_t c = 0; for (auto& e : m_events) if (e.active) ++c; return c;
    }
    [[nodiscard]] size_t hazardousCount() const {
        size_t c = 0; for (auto& e : m_events) if (e.active && e.isHazardous()) ++c; return c;
    }
    [[nodiscard]] size_t peakCount()      const {
        size_t c = 0; for (auto& e : m_events) if (e.active && e.isAtPeak()) ++c; return c;
    }
    [[nodiscard]] size_t lowVisCount()    const {
        size_t c = 0; for (auto& e : m_events) if (e.active && e.isLowVisibility()) ++c; return c;
    }

    void tick() { ++m_tickCount; }
    [[nodiscard]] const std::string& name()      const { return m_name;      }
    [[nodiscard]] size_t             tickCount() const { return m_tickCount; }

private:
    std::string                  m_name;
    std::vector<DustStormEvent>  m_events;
    size_t                       m_tickCount = 0;
};

class DustStormSystem {
public:
    static constexpr size_t MAX_REGIONS = 32;

    DustStormRegion* createRegion(const std::string& name) {
        for (auto& r : m_regions) if (r.name() == name) return nullptr;
        if (m_regions.size() >= MAX_REGIONS) return nullptr;
        m_regions.emplace_back(name);
        return &m_regions.back();
    }

    [[nodiscard]] DustStormRegion* byName(const std::string& name) {
        for (auto& r : m_regions) if (r.name() == name) return &r;
        return nullptr;
    }

    void tick() {
        ++m_tickCount;
        for (auto& r : m_regions) r.tick();
    }

    [[nodiscard]] size_t regionCount()      const { return m_regions.size(); }
    [[nodiscard]] size_t tickCount()        const { return m_tickCount;      }
    [[nodiscard]] size_t activeEventCount() const {
        size_t c = 0; for (auto& r : m_regions) c += r.activeCount(); return c;
    }
    [[nodiscard]] size_t hazardousEventCount() const {
        size_t c = 0; for (auto& r : m_regions) c += r.hazardousCount(); return c;
    }
    [[nodiscard]] size_t peakEventCount()   const {
        size_t c = 0; for (auto& r : m_regions) c += r.peakCount(); return c;
    }

private:
    std::vector<DustStormRegion> m_regions;
    size_t                       m_tickCount = 0;
};

// ── G56 — Hail Storm System ───────────────────────────────────────

enum class HailSize : uint8_t {
    Pea, Marble, Golf, Baseball, Grapefruit
};

inline const char* hailSizeName(HailSize h) {
    switch (h) {
        case HailSize::Pea:        return "Pea";
        case HailSize::Marble:     return "Marble";
        case HailSize::Golf:       return "Golf";
        case HailSize::Baseball:   return "Baseball";
        case HailSize::Grapefruit: return "Grapefruit";
    }
    return "Unknown";
}

enum class HailStormPhase : uint8_t {
    Developing, Intensifying, Peak, Weakening, Dissipating
};

inline const char* hailStormPhaseName(HailStormPhase p) {
    switch (p) {
        case HailStormPhase::Developing:  return "Developing";
        case HailStormPhase::Intensifying:return "Intensifying";
        case HailStormPhase::Peak:        return "Peak";
        case HailStormPhase::Weakening:   return "Weakening";
        case HailStormPhase::Dissipating: return "Dissipating";
    }
    return "Unknown";
}

struct HailStormEvent {
    std::string     id;
    HailSize        hailSize  = HailSize::Pea;
    HailStormPhase  phase     = HailStormPhase::Developing;
    float           intensity = 0.0f;   // 0–100
    float           coverage  = 0.0f;   // km²
    bool            active    = false;

    void activate()   { active = true;  }
    void deactivate() { active = false; }

    [[nodiscard]] bool isSevere()     const {
        return static_cast<uint8_t>(hailSize) >= static_cast<uint8_t>(HailSize::Golf);
    }
    [[nodiscard]] bool isAtPeak()     const { return phase == HailStormPhase::Peak; }
    [[nodiscard]] bool isWidespread() const { return coverage >= 500.0f; }
    [[nodiscard]] float damageScore() const {
        return (static_cast<float>(static_cast<uint8_t>(hailSize) + 1)) * intensity / 100.0f;
    }
};

class HailStormRegion {
public:
    explicit HailStormRegion(const std::string& name) : m_name(name) {}

    [[nodiscard]] bool addEvent(const HailStormEvent& ev) {
        for (auto& e : m_events) if (e.id == ev.id) return false;
        m_events.push_back(ev);
        return true;
    }

    [[nodiscard]] bool removeEvent(const std::string& id) {
        for (auto it = m_events.begin(); it != m_events.end(); ++it) {
            if (it->id == id) { m_events.erase(it); return true; }
        }
        return false;
    }

    [[nodiscard]] HailStormEvent* findEvent(const std::string& id) {
        for (auto& e : m_events) if (e.id == id) return &e;
        return nullptr;
    }

    void activateAll()   { for (auto& e : m_events) e.activate();   }
    void deactivateAll() { for (auto& e : m_events) e.deactivate(); }

    [[nodiscard]] size_t eventCount()      const { return m_events.size(); }
    [[nodiscard]] size_t activeCount()     const {
        size_t c = 0; for (auto& e : m_events) if (e.active) ++c; return c;
    }
    [[nodiscard]] size_t severeCount()     const {
        size_t c = 0; for (auto& e : m_events) if (e.active && e.isSevere()) ++c; return c;
    }
    [[nodiscard]] size_t peakCount()       const {
        size_t c = 0; for (auto& e : m_events) if (e.active && e.isAtPeak()) ++c; return c;
    }
    [[nodiscard]] size_t widespreadCount() const {
        size_t c = 0; for (auto& e : m_events) if (e.active && e.isWidespread()) ++c; return c;
    }

    void tick() { ++m_tickCount; }
    [[nodiscard]] const std::string& name()      const { return m_name;      }
    [[nodiscard]] size_t             tickCount() const { return m_tickCount; }

private:
    std::string                  m_name;
    std::vector<HailStormEvent>  m_events;
    size_t                       m_tickCount = 0;
};

class HailStormSystem {
public:
    static constexpr size_t MAX_REGIONS = 32;

    HailStormRegion* createRegion(const std::string& name) {
        for (auto& r : m_regions) if (r.name() == name) return nullptr;
        if (m_regions.size() >= MAX_REGIONS) return nullptr;
        m_regions.emplace_back(name);
        return &m_regions.back();
    }

    [[nodiscard]] HailStormRegion* byName(const std::string& name) {
        for (auto& r : m_regions) if (r.name() == name) return &r;
        return nullptr;
    }

    void tick() {
        ++m_tickCount;
        for (auto& r : m_regions) r.tick();
    }

    [[nodiscard]] size_t regionCount()       const { return m_regions.size(); }
    [[nodiscard]] size_t tickCount()         const { return m_tickCount;      }
    [[nodiscard]] size_t activeEventCount()  const {
        size_t c = 0; for (auto& r : m_regions) c += r.activeCount(); return c;
    }
    [[nodiscard]] size_t severeEventCount()  const {
        size_t c = 0; for (auto& r : m_regions) c += r.severeCount(); return c;
    }
    [[nodiscard]] size_t peakEventCount()    const {
        size_t c = 0; for (auto& r : m_regions) c += r.peakCount(); return c;
    }
    [[nodiscard]] size_t widespreadEventCount() const {
        size_t c = 0; for (auto& r : m_regions) c += r.widespreadCount(); return c;
    }

private:
    std::vector<HailStormRegion> m_regions;
    size_t                       m_tickCount = 0;
};

// ── G57 — Ice Storm System ────────────────────────────────────────

enum class IceThickness : uint8_t {
    Glaze, Light, Moderate, Heavy, Extreme
};

inline const char* iceThicknessName(IceThickness t) {
    switch (t) {
        case IceThickness::Glaze:    return "Glaze";
        case IceThickness::Light:    return "Light";
        case IceThickness::Moderate: return "Moderate";
        case IceThickness::Heavy:    return "Heavy";
        case IceThickness::Extreme:  return "Extreme";
    }
    return "Unknown";
}

enum class IceStormPhase : uint8_t {
    Forming, Spreading, Intensifying, Freezing, Dissipating
};

inline const char* iceStormPhaseName(IceStormPhase p) {
    switch (p) {
        case IceStormPhase::Forming:      return "Forming";
        case IceStormPhase::Spreading:    return "Spreading";
        case IceStormPhase::Intensifying: return "Intensifying";
        case IceStormPhase::Freezing:     return "Freezing";
        case IceStormPhase::Dissipating:  return "Dissipating";
    }
    return "Unknown";
}

struct IceStormEvent {
    std::string    id;
    IceThickness   thickness = IceThickness::Glaze;
    IceStormPhase  phase     = IceStormPhase::Forming;
    float          intensity = 0.0f;   // 0–100
    float          coverage  = 0.0f;   // km²
    bool           active    = false;

    void activate()   { active = true;  }
    void deactivate() { active = false; }

    [[nodiscard]] bool isSevere()      const {
        return static_cast<uint8_t>(thickness) >= static_cast<uint8_t>(IceThickness::Heavy);
    }
    [[nodiscard]] bool isFreezing()    const { return phase == IceStormPhase::Freezing;     }
    [[nodiscard]] bool isWidespread()  const { return coverage >= 400.0f;                   }
    [[nodiscard]] float hazardScore()  const {
        return (static_cast<float>(static_cast<uint8_t>(thickness) + 1)) * intensity / 100.0f;
    }
};

class IceStormRegion {
public:
    explicit IceStormRegion(const std::string& name) : m_name(name) {}

    [[nodiscard]] bool addEvent(const IceStormEvent& ev) {
        for (auto& e : m_events) if (e.id == ev.id) return false;
        m_events.push_back(ev);
        return true;
    }

    [[nodiscard]] bool removeEvent(const std::string& id) {
        for (auto it = m_events.begin(); it != m_events.end(); ++it) {
            if (it->id == id) { m_events.erase(it); return true; }
        }
        return false;
    }

    [[nodiscard]] IceStormEvent* findEvent(const std::string& id) {
        for (auto& e : m_events) if (e.id == id) return &e;
        return nullptr;
    }

    void activateAll()   { for (auto& e : m_events) e.activate();   }
    void deactivateAll() { for (auto& e : m_events) e.deactivate(); }

    [[nodiscard]] size_t eventCount()      const { return m_events.size(); }
    [[nodiscard]] size_t activeCount()     const {
        size_t c = 0; for (auto& e : m_events) if (e.active) ++c; return c;
    }
    [[nodiscard]] size_t severeCount()     const {
        size_t c = 0; for (auto& e : m_events) if (e.active && e.isSevere())     ++c; return c;
    }
    [[nodiscard]] size_t freezingCount()   const {
        size_t c = 0; for (auto& e : m_events) if (e.active && e.isFreezing())   ++c; return c;
    }
    [[nodiscard]] size_t widespreadCount() const {
        size_t c = 0; for (auto& e : m_events) if (e.active && e.isWidespread()) ++c; return c;
    }

    void tick() { ++m_tickCount; }
    [[nodiscard]] const std::string& name()      const { return m_name;      }
    [[nodiscard]] size_t             tickCount() const { return m_tickCount; }

private:
    std::string                 m_name;
    std::vector<IceStormEvent>  m_events;
    size_t                      m_tickCount = 0;
};

class IceStormSystem {
public:
    static constexpr size_t MAX_REGIONS = 32;

    IceStormRegion* createRegion(const std::string& name) {
        for (auto& r : m_regions) if (r.name() == name) return nullptr;
        if (m_regions.size() >= MAX_REGIONS) return nullptr;
        m_regions.emplace_back(name);
        return &m_regions.back();
    }

    [[nodiscard]] IceStormRegion* byName(const std::string& name) {
        for (auto& r : m_regions) if (r.name() == name) return &r;
        return nullptr;
    }

    void tick() {
        ++m_tickCount;
        for (auto& r : m_regions) r.tick();
    }

    [[nodiscard]] size_t regionCount()          const { return m_regions.size(); }
    [[nodiscard]] size_t tickCount()            const { return m_tickCount;      }
    [[nodiscard]] size_t activeEventCount()     const {
        size_t c = 0; for (auto& r : m_regions) c += r.activeCount();     return c;
    }
    [[nodiscard]] size_t severeEventCount()     const {
        size_t c = 0; for (auto& r : m_regions) c += r.severeCount();     return c;
    }
    [[nodiscard]] size_t freezingEventCount()   const {
        size_t c = 0; for (auto& r : m_regions) c += r.freezingCount();   return c;
    }
    [[nodiscard]] size_t widespreadEventCount() const {
        size_t c = 0; for (auto& r : m_regions) c += r.widespreadCount(); return c;
    }

private:
    std::vector<IceStormRegion> m_regions;
    size_t                      m_tickCount = 0;
};

// ── G58 — Monsoon System ─────────────────────────────────────────

enum class MonsoonIntensity : uint8_t {
    Light, Moderate, Heavy, Severe, Extreme
};
inline const char* monsoonIntensityName(MonsoonIntensity m) {
    switch (m) {
        case MonsoonIntensity::Light:    return "Light";
        case MonsoonIntensity::Moderate: return "Moderate";
        case MonsoonIntensity::Heavy:    return "Heavy";
        case MonsoonIntensity::Severe:   return "Severe";
        case MonsoonIntensity::Extreme:  return "Extreme";
    }
    return "Unknown";
}

enum class MonsoonPhase : uint8_t {
    Onset, Active, Peak, Retreating, Withdrawal
};
inline const char* monsoonPhaseName(MonsoonPhase p) {
    switch (p) {
        case MonsoonPhase::Onset:      return "Onset";
        case MonsoonPhase::Active:     return "Active";
        case MonsoonPhase::Peak:       return "Peak";
        case MonsoonPhase::Retreating: return "Retreating";
        case MonsoonPhase::Withdrawal: return "Withdrawal";
    }
    return "Unknown";
}

struct MonsoonEvent {
    std::string       id;
    MonsoonIntensity  intensity  = MonsoonIntensity::Light;
    MonsoonPhase      phase      = MonsoonPhase::Onset;
    float             rainfallMm = 0.0f;
    float             windSpeed  = 0.0f;
    bool              active     = false;

    void activate()   { active = true;  }
    void deactivate() { active = false; }

    [[nodiscard]] bool isSevere()    const { return intensity >= MonsoonIntensity::Severe; }
    [[nodiscard]] bool isPeak()      const { return phase == MonsoonPhase::Peak; }
    [[nodiscard]] bool isFlooding()  const { return rainfallMm >= 200.0f; }
    [[nodiscard]] float hazardScore() const {
        return (static_cast<float>(static_cast<uint8_t>(intensity)) + 1.0f) * rainfallMm / 100.0f;
    }
};

class MonsoonRegion {
public:
    explicit MonsoonRegion(const std::string& name) : m_name(name) {}

    [[nodiscard]] bool addEvent(const MonsoonEvent& ev) {
        for (auto& e : m_events) if (e.id == ev.id) return false;
        m_events.push_back(ev);
        return true;
    }
    [[nodiscard]] bool removeEvent(const std::string& id) {
        for (auto it = m_events.begin(); it != m_events.end(); ++it) {
            if (it->id == id) { m_events.erase(it); return true; }
        }
        return false;
    }
    [[nodiscard]] MonsoonEvent* findEvent(const std::string& id) {
        for (auto& e : m_events) if (e.id == id) return &e;
        return nullptr;
    }
    void activateAll()   { for (auto& e : m_events) e.activate();   }
    void deactivateAll() { for (auto& e : m_events) e.deactivate(); }

    void tick() { ++m_tickCount; }
    [[nodiscard]] const std::string& name()        const { return m_name; }
    [[nodiscard]] size_t eventCount()    const { return m_events.size(); }
    [[nodiscard]] size_t tickCount()     const { return m_tickCount; }
    [[nodiscard]] size_t activeCount()   const {
        size_t c = 0; for (auto& e : m_events) if (e.active)       ++c; return c;
    }
    [[nodiscard]] size_t severeCount()   const {
        size_t c = 0; for (auto& e : m_events) if (e.active && e.isSevere())   ++c; return c;
    }
    [[nodiscard]] size_t peakCount()     const {
        size_t c = 0; for (auto& e : m_events) if (e.active && e.isPeak())     ++c; return c;
    }
    [[nodiscard]] size_t floodingCount() const {
        size_t c = 0; for (auto& e : m_events) if (e.active && e.isFlooding()) ++c; return c;
    }

private:
    std::string               m_name;
    std::vector<MonsoonEvent> m_events;
    size_t                    m_tickCount = 0;
};

class MonsoonSystem {
public:
    static constexpr size_t MAX_REGIONS = 32;

    MonsoonRegion* createRegion(const std::string& name) {
        if (m_regions.size() >= MAX_REGIONS) return nullptr;
        for (auto& r : m_regions) if (r.name() == name) return nullptr;
        m_regions.emplace_back(name);
        return &m_regions.back();
    }
    [[nodiscard]] MonsoonRegion* byName(const std::string& name) {
        for (auto& r : m_regions) if (r.name() == name) return &r;
        return nullptr;
    }
    void tick() {
        ++m_tickCount;
        for (auto& r : m_regions) r.tick();
    }

    [[nodiscard]] size_t regionCount()        const { return m_regions.size(); }
    [[nodiscard]] size_t tickCount()          const { return m_tickCount; }
    [[nodiscard]] size_t activeEventCount()   const {
        size_t c = 0; for (auto& r : m_regions) c += r.activeCount();   return c;
    }
    [[nodiscard]] size_t severeEventCount()   const {
        size_t c = 0; for (auto& r : m_regions) c += r.severeCount();   return c;
    }
    [[nodiscard]] size_t peakEventCount()     const {
        size_t c = 0; for (auto& r : m_regions) c += r.peakCount();     return c;
    }
    [[nodiscard]] size_t floodingEventCount() const {
        size_t c = 0; for (auto& r : m_regions) c += r.floodingCount(); return c;
    }

private:
    std::vector<MonsoonRegion> m_regions;
    size_t                     m_tickCount = 0;
};

// ── G59 — Lightning Storm System ────────────────────────────────

enum class LightningIntensity : uint8_t {
    Scattered, Moderate, Frequent, Severe, Extreme
};
inline const char* lightningIntensityName(LightningIntensity l) {
    switch (l) {
        case LightningIntensity::Scattered: return "Scattered";
        case LightningIntensity::Moderate:  return "Moderate";
        case LightningIntensity::Frequent:  return "Frequent";
        case LightningIntensity::Severe:    return "Severe";
        case LightningIntensity::Extreme:   return "Extreme";
    }
    return "Unknown";
}

enum class LightningStormPhase : uint8_t {
    Building, Active, Peak, Weakening, Dissipating
};
inline const char* lightningStormPhaseName(LightningStormPhase p) {
    switch (p) {
        case LightningStormPhase::Building:    return "Building";
        case LightningStormPhase::Active:      return "Active";
        case LightningStormPhase::Peak:        return "Peak";
        case LightningStormPhase::Weakening:   return "Weakening";
        case LightningStormPhase::Dissipating: return "Dissipating";
    }
    return "Unknown";
}

struct LightningEvent {
    std::string          id;
    LightningIntensity   intensity    = LightningIntensity::Scattered;
    LightningStormPhase  phase        = LightningStormPhase::Building;
    float                strikeRate   = 0.0f;  // strikes per minute
    float                coverage     = 0.0f;  // km²
    bool                 active       = false;

    void activate()   { active = true;  }
    void deactivate() { active = false; }

    [[nodiscard]] bool isSevere()     const { return intensity >= LightningIntensity::Severe; }
    [[nodiscard]] bool isPeak()       const { return phase == LightningStormPhase::Peak; }
    [[nodiscard]] bool isWidespread() const { return coverage >= 300.0f; }
    [[nodiscard]] float hazardScore() const {
        return (static_cast<float>(static_cast<uint8_t>(intensity)) + 1.0f) * strikeRate / 10.0f;
    }
};

class LightningRegion {
public:
    explicit LightningRegion(const std::string& name) : m_name(name) {}

    [[nodiscard]] bool addEvent(const LightningEvent& ev) {
        for (auto& e : m_events) if (e.id == ev.id) return false;
        m_events.push_back(ev);
        return true;
    }
    [[nodiscard]] bool removeEvent(const std::string& id) {
        for (auto it = m_events.begin(); it != m_events.end(); ++it) {
            if (it->id == id) { m_events.erase(it); return true; }
        }
        return false;
    }
    [[nodiscard]] LightningEvent* findEvent(const std::string& id) {
        for (auto& e : m_events) if (e.id == id) return &e;
        return nullptr;
    }
    void activateAll()   { for (auto& e : m_events) e.activate();   }
    void deactivateAll() { for (auto& e : m_events) e.deactivate(); }

    void tick() { ++m_tickCount; }
    [[nodiscard]] const std::string& name()      const { return m_name; }
    [[nodiscard]] size_t eventCount()   const { return m_events.size(); }
    [[nodiscard]] size_t tickCount()    const { return m_tickCount; }
    [[nodiscard]] size_t activeCount()  const {
        size_t c = 0; for (auto& e : m_events) if (e.active)                     ++c; return c;
    }
    [[nodiscard]] size_t severeCount()  const {
        size_t c = 0; for (auto& e : m_events) if (e.active && e.isSevere())     ++c; return c;
    }
    [[nodiscard]] size_t peakCount()    const {
        size_t c = 0; for (auto& e : m_events) if (e.active && e.isPeak())       ++c; return c;
    }
    [[nodiscard]] size_t widespreadCount() const {
        size_t c = 0; for (auto& e : m_events) if (e.active && e.isWidespread()) ++c; return c;
    }

private:
    std::string                m_name;
    std::vector<LightningEvent> m_events;
    size_t                     m_tickCount = 0;
};

class LightningSystem {
public:
    static constexpr size_t MAX_REGIONS = 32;

    LightningRegion* createRegion(const std::string& name) {
        if (m_regions.size() >= MAX_REGIONS) return nullptr;
        for (auto& r : m_regions) if (r.name() == name) return nullptr;
        m_regions.emplace_back(name);
        return &m_regions.back();
    }
    [[nodiscard]] LightningRegion* byName(const std::string& name) {
        for (auto& r : m_regions) if (r.name() == name) return &r;
        return nullptr;
    }
    void tick() {
        ++m_tickCount;
        for (auto& r : m_regions) r.tick();
    }

    [[nodiscard]] size_t regionCount()          const { return m_regions.size(); }
    [[nodiscard]] size_t tickCount()            const { return m_tickCount; }
    [[nodiscard]] size_t activeEventCount()     const {
        size_t c = 0; for (auto& r : m_regions) c += r.activeCount();     return c;
    }
    [[nodiscard]] size_t severeEventCount()     const {
        size_t c = 0; for (auto& r : m_regions) c += r.severeCount();     return c;
    }
    [[nodiscard]] size_t peakEventCount()       const {
        size_t c = 0; for (auto& r : m_regions) c += r.peakCount();       return c;
    }
    [[nodiscard]] size_t widespreadEventCount() const {
        size_t c = 0; for (auto& r : m_regions) c += r.widespreadCount(); return c;
    }

private:
    std::vector<LightningRegion> m_regions;
    size_t                       m_tickCount = 0;
};

// ── G60 — Geomagnetic Storm System ──────────────────────────────

enum class GeoStormKpIndex : uint8_t {
    Quiet, Unsettled, Active, Storm, Extreme
};
inline const char* geoStormKpIndexName(GeoStormKpIndex k) {
    switch (k) {
        case GeoStormKpIndex::Quiet:     return "Quiet";
        case GeoStormKpIndex::Unsettled: return "Unsettled";
        case GeoStormKpIndex::Active:    return "Active";
        case GeoStormKpIndex::Storm:     return "Storm";
        case GeoStormKpIndex::Extreme:   return "Extreme";
    }
    return "Unknown";
}

enum class GeoStormPhase : uint8_t {
    Onset, Initial, Main, Recovery, Quiescent
};
inline const char* geoStormPhaseName(GeoStormPhase p) {
    switch (p) {
        case GeoStormPhase::Onset:      return "Onset";
        case GeoStormPhase::Initial:    return "Initial";
        case GeoStormPhase::Main:       return "Main";
        case GeoStormPhase::Recovery:   return "Recovery";
        case GeoStormPhase::Quiescent:  return "Quiescent";
    }
    return "Unknown";
}

struct GeoStormEvent {
    std::string      id;
    GeoStormKpIndex  kp          = GeoStormKpIndex::Quiet;
    GeoStormPhase    phase       = GeoStormPhase::Quiescent;
    float            disturbance = 0.0f;  // nT field change
    float            coverage    = 0.0f;  // % of globe affected
    bool             active      = false;

    void activate()   { active = true;  }
    void deactivate() { active = false; }

    [[nodiscard]] bool isSevere()     const { return kp >= GeoStormKpIndex::Storm; }
    [[nodiscard]] bool isMainPhase()  const { return phase == GeoStormPhase::Main; }
    [[nodiscard]] bool isGlobal()     const { return coverage >= 50.0f; }
    [[nodiscard]] float hazardScore() const {
        return (static_cast<float>(static_cast<uint8_t>(kp)) + 1.0f) * coverage / 10.0f;
    }
};

class GeoStormRegion {
public:
    explicit GeoStormRegion(const std::string& name) : m_name(name) {}

    [[nodiscard]] bool addEvent(const GeoStormEvent& ev) {
        for (auto& e : m_events) if (e.id == ev.id) return false;
        m_events.push_back(ev);
        return true;
    }
    [[nodiscard]] bool removeEvent(const std::string& id) {
        for (auto it = m_events.begin(); it != m_events.end(); ++it) {
            if (it->id == id) { m_events.erase(it); return true; }
        }
        return false;
    }
    [[nodiscard]] GeoStormEvent* findEvent(const std::string& id) {
        for (auto& e : m_events) if (e.id == id) return &e;
        return nullptr;
    }
    void activateAll()   { for (auto& e : m_events) e.activate();   }
    void deactivateAll() { for (auto& e : m_events) e.deactivate(); }

    void tick() { ++m_tickCount; }
    [[nodiscard]] const std::string& name()       const { return m_name; }
    [[nodiscard]] size_t eventCount()    const { return m_events.size(); }
    [[nodiscard]] size_t tickCount()     const { return m_tickCount; }
    [[nodiscard]] size_t activeCount()   const {
        size_t c = 0; for (auto& e : m_events) if (e.active)                    ++c; return c;
    }
    [[nodiscard]] size_t severeCount()   const {
        size_t c = 0; for (auto& e : m_events) if (e.active && e.isSevere())    ++c; return c;
    }
    [[nodiscard]] size_t mainPhaseCount()const {
        size_t c = 0; for (auto& e : m_events) if (e.active && e.isMainPhase()) ++c; return c;
    }
    [[nodiscard]] size_t globalCount()   const {
        size_t c = 0; for (auto& e : m_events) if (e.active && e.isGlobal())    ++c; return c;
    }

private:
    std::string               m_name;
    std::vector<GeoStormEvent> m_events;
    size_t                    m_tickCount = 0;
};

class GeoStormSystem {
public:
    static constexpr size_t MAX_REGIONS = 32;

    GeoStormRegion* createRegion(const std::string& name) {
        if (m_regions.size() >= MAX_REGIONS) return nullptr;
        for (auto& r : m_regions) if (r.name() == name) return nullptr;
        m_regions.emplace_back(name);
        return &m_regions.back();
    }
    [[nodiscard]] GeoStormRegion* byName(const std::string& name) {
        for (auto& r : m_regions) if (r.name() == name) return &r;
        return nullptr;
    }
    void tick() {
        ++m_tickCount;
        for (auto& r : m_regions) r.tick();
    }

    [[nodiscard]] size_t regionCount()        const { return m_regions.size(); }
    [[nodiscard]] size_t tickCount()          const { return m_tickCount; }
    [[nodiscard]] size_t activeEventCount()   const {
        size_t c = 0; for (auto& r : m_regions) c += r.activeCount();    return c;
    }
    [[nodiscard]] size_t severeEventCount()   const {
        size_t c = 0; for (auto& r : m_regions) c += r.severeCount();    return c;
    }
    [[nodiscard]] size_t mainPhaseEventCount()const {
        size_t c = 0; for (auto& r : m_regions) c += r.mainPhaseCount(); return c;
    }
    [[nodiscard]] size_t globalEventCount()   const {
        size_t c = 0; for (auto& r : m_regions) c += r.globalCount();    return c;
    }

private:
    std::vector<GeoStormRegion> m_regions;
    size_t                      m_tickCount = 0;
};

// ── G61 — Cosmic Ray System ─────────────────────────────────────

enum class CosmicRayType : uint8_t {
    Proton, Alpha, HeavyIon, Electron, Positron
};
inline const char* cosmicRayTypeName(CosmicRayType t) {
    switch (t) {
        case CosmicRayType::Proton:   return "Proton";
        case CosmicRayType::Alpha:    return "Alpha";
        case CosmicRayType::HeavyIon: return "HeavyIon";
        case CosmicRayType::Electron: return "Electron";
        case CosmicRayType::Positron: return "Positron";
    }
    return "Unknown";
}

enum class CosmicRayIntensity : uint8_t {
    Background, Elevated, High, Severe, Extreme
};
inline const char* cosmicRayIntensityName(CosmicRayIntensity i) {
    switch (i) {
        case CosmicRayIntensity::Background: return "Background";
        case CosmicRayIntensity::Elevated:   return "Elevated";
        case CosmicRayIntensity::High:       return "High";
        case CosmicRayIntensity::Severe:     return "Severe";
        case CosmicRayIntensity::Extreme:    return "Extreme";
    }
    return "Unknown";
}

struct CosmicRayEvent {
    std::string         id;
    CosmicRayType       type      = CosmicRayType::Proton;
    CosmicRayIntensity  intensity = CosmicRayIntensity::Background;
    float               dosageRate = 0.0f;  // mSv/hr
    float               coverage   = 0.0f;  // % of area affected
    bool                active     = false;

    void activate()   { active = true;  }
    void deactivate() { active = false; }

    [[nodiscard]] bool isSevere()       const { return intensity >= CosmicRayIntensity::Severe; }
    [[nodiscard]] bool isHighEnergy()   const { return type <= CosmicRayType::HeavyIon; }
    [[nodiscard]] bool isWidespread()   const { return coverage >= 50.0f; }
    [[nodiscard]] float hazardScore()   const {
        return (static_cast<float>(static_cast<uint8_t>(intensity)) + 1.0f) * coverage / 10.0f;
    }
};

class CosmicRayRegion {
public:
    explicit CosmicRayRegion(const std::string& name) : m_name(name) {}

    [[nodiscard]] bool addEvent(const CosmicRayEvent& ev) {
        for (auto& e : m_events) if (e.id == ev.id) return false;
        m_events.push_back(ev);
        return true;
    }
    [[nodiscard]] bool removeEvent(const std::string& id) {
        for (auto it = m_events.begin(); it != m_events.end(); ++it) {
            if (it->id == id) { m_events.erase(it); return true; }
        }
        return false;
    }
    [[nodiscard]] CosmicRayEvent* findEvent(const std::string& id) {
        for (auto& e : m_events) if (e.id == id) return &e;
        return nullptr;
    }
    void activateAll()   { for (auto& e : m_events) e.activate();   }
    void deactivateAll() { for (auto& e : m_events) e.deactivate(); }

    void tick() { ++m_tickCount; }
    [[nodiscard]] const std::string& name()          const { return m_name; }
    [[nodiscard]] size_t eventCount()       const { return m_events.size(); }
    [[nodiscard]] size_t tickCount()        const { return m_tickCount; }
    [[nodiscard]] size_t activeCount()      const {
        size_t c = 0; for (auto& e : m_events) if (e.active)                      ++c; return c;
    }
    [[nodiscard]] size_t severeCount()      const {
        size_t c = 0; for (auto& e : m_events) if (e.active && e.isSevere())      ++c; return c;
    }
    [[nodiscard]] size_t highEnergyCount()  const {
        size_t c = 0; for (auto& e : m_events) if (e.active && e.isHighEnergy())  ++c; return c;
    }
    [[nodiscard]] size_t widespreadCount()  const {
        size_t c = 0; for (auto& e : m_events) if (e.active && e.isWidespread())  ++c; return c;
    }

private:
    std::string                  m_name;
    std::vector<CosmicRayEvent>  m_events;
    size_t                       m_tickCount = 0;
};

class CosmicRaySystem {
public:
    static constexpr size_t MAX_REGIONS = 32;

    CosmicRayRegion* createRegion(const std::string& name) {
        if (m_regions.size() >= MAX_REGIONS) return nullptr;
        for (auto& r : m_regions) if (r.name() == name) return nullptr;
        m_regions.emplace_back(name);
        return &m_regions.back();
    }
    [[nodiscard]] CosmicRayRegion* byName(const std::string& name) {
        for (auto& r : m_regions) if (r.name() == name) return &r;
        return nullptr;
    }
    void tick() {
        ++m_tickCount;
        for (auto& r : m_regions) r.tick();
    }

    [[nodiscard]] size_t regionCount()           const { return m_regions.size(); }
    [[nodiscard]] size_t tickCount()             const { return m_tickCount; }
    [[nodiscard]] size_t activeEventCount()      const {
        size_t c = 0; for (auto& r : m_regions) c += r.activeCount();     return c;
    }
    [[nodiscard]] size_t severeEventCount()      const {
        size_t c = 0; for (auto& r : m_regions) c += r.severeCount();     return c;
    }
    [[nodiscard]] size_t highEnergyEventCount()  const {
        size_t c = 0; for (auto& r : m_regions) c += r.highEnergyCount(); return c;
    }
    [[nodiscard]] size_t widespreadEventCount()  const {
        size_t c = 0; for (auto& r : m_regions) c += r.widespreadCount(); return c;
    }

private:
    std::vector<CosmicRayRegion> m_regions;
    size_t                       m_tickCount = 0;
};

// ── G62 — Solar Wind System ─────────────────────────────────────

enum class SolarWindType : uint8_t {
    Slow, Fast, Transient, Coronal, Interplanetary
};
inline const char* solarWindTypeName(SolarWindType t) {
    switch (t) {
        case SolarWindType::Slow:           return "Slow";
        case SolarWindType::Fast:           return "Fast";
        case SolarWindType::Transient:      return "Transient";
        case SolarWindType::Coronal:        return "Coronal";
        case SolarWindType::Interplanetary: return "Interplanetary";
    }
    return "Unknown";
}

enum class SolarWindIntensity : uint8_t {
    Calm, Moderate, Strong, Violent, Extreme
};
inline const char* solarWindIntensityName(SolarWindIntensity i) {
    switch (i) {
        case SolarWindIntensity::Calm:     return "Calm";
        case SolarWindIntensity::Moderate: return "Moderate";
        case SolarWindIntensity::Strong:   return "Strong";
        case SolarWindIntensity::Violent:  return "Violent";
        case SolarWindIntensity::Extreme:  return "Extreme";
    }
    return "Unknown";
}

struct SolarWindEvent {
    std::string          id;
    SolarWindType        type      = SolarWindType::Slow;
    SolarWindIntensity   intensity = SolarWindIntensity::Calm;
    float                velocity  = 0.0f;   // km/s
    float                coverage  = 0.0f;   // % of area affected
    bool                 active    = false;

    void activate()   { active = true;  }
    void deactivate() { active = false; }

    [[nodiscard]] bool isSevere()       const { return intensity >= SolarWindIntensity::Violent; }
    [[nodiscard]] bool isFastStream()   const { return type == SolarWindType::Fast || type == SolarWindType::Transient; }
    [[nodiscard]] bool isWidespread()   const { return coverage >= 50.0f; }
    [[nodiscard]] float hazardScore()   const {
        return (static_cast<float>(static_cast<uint8_t>(intensity)) + 1.0f) * coverage / 10.0f;
    }
};

class SolarWindRegion {
public:
    explicit SolarWindRegion(const std::string& name) : m_name(name) {}

    [[nodiscard]] bool addEvent(const SolarWindEvent& ev) {
        for (auto& e : m_events) if (e.id == ev.id) return false;
        m_events.push_back(ev);
        return true;
    }
    [[nodiscard]] bool removeEvent(const std::string& id) {
        for (auto it = m_events.begin(); it != m_events.end(); ++it) {
            if (it->id == id) { m_events.erase(it); return true; }
        }
        return false;
    }
    [[nodiscard]] SolarWindEvent* findEvent(const std::string& id) {
        for (auto& e : m_events) if (e.id == id) return &e;
        return nullptr;
    }
    void activateAll()   { for (auto& e : m_events) e.activate();   }
    void deactivateAll() { for (auto& e : m_events) e.deactivate(); }

    void tick() { ++m_tickCount; }
    [[nodiscard]] const std::string& name()           const { return m_name; }
    [[nodiscard]] size_t eventCount()        const { return m_events.size(); }
    [[nodiscard]] size_t tickCount()         const { return m_tickCount; }
    [[nodiscard]] size_t activeCount()       const {
        size_t c = 0; for (auto& e : m_events) if (e.active)                       ++c; return c;
    }
    [[nodiscard]] size_t severeCount()       const {
        size_t c = 0; for (auto& e : m_events) if (e.active && e.isSevere())       ++c; return c;
    }
    [[nodiscard]] size_t fastStreamCount()   const {
        size_t c = 0; for (auto& e : m_events) if (e.active && e.isFastStream())   ++c; return c;
    }
    [[nodiscard]] size_t widespreadCount()   const {
        size_t c = 0; for (auto& e : m_events) if (e.active && e.isWidespread())   ++c; return c;
    }

private:
    std::string                   m_name;
    std::vector<SolarWindEvent>   m_events;
    size_t                        m_tickCount = 0;
};

class SolarWindSystem {
public:
    static constexpr size_t MAX_REGIONS = 32;

    SolarWindRegion* createRegion(const std::string& name) {
        if (m_regions.size() >= MAX_REGIONS) return nullptr;
        for (auto& r : m_regions) if (r.name() == name) return nullptr;
        m_regions.emplace_back(name);
        return &m_regions.back();
    }
    [[nodiscard]] SolarWindRegion* byName(const std::string& name) {
        for (auto& r : m_regions) if (r.name() == name) return &r;
        return nullptr;
    }
    void tick() {
        ++m_tickCount;
        for (auto& r : m_regions) r.tick();
    }

    [[nodiscard]] size_t regionCount()            const { return m_regions.size(); }
    [[nodiscard]] size_t tickCount()              const { return m_tickCount; }
    [[nodiscard]] size_t activeEventCount()       const {
        size_t c = 0; for (auto& r : m_regions) c += r.activeCount();      return c;
    }
    [[nodiscard]] size_t severeEventCount()       const {
        size_t c = 0; for (auto& r : m_regions) c += r.severeCount();      return c;
    }
    [[nodiscard]] size_t fastStreamEventCount()   const {
        size_t c = 0; for (auto& r : m_regions) c += r.fastStreamCount();  return c;
    }
    [[nodiscard]] size_t widespreadEventCount()   const {
        size_t c = 0; for (auto& r : m_regions) c += r.widespreadCount();  return c;
    }

private:
    std::vector<SolarWindRegion> m_regions;
    size_t                       m_tickCount = 0;
};

} // namespace NF
