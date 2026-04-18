#pragma once
#include "ProceduralMeshGraph.h"
#include <cstdint>
#include <vector>
#include <string>
#include <unordered_map>

namespace atlas::procedural {

// ---------- Data Structures ----------

struct Vec3 {
    float x = 0.0f, y = 0.0f, z = 0.0f;
};

/// A draggable control point that defines the ship hull silhouette.
/// The editor allows users to click-and-drag these to reshape the hull.
struct HullControlPoint {
    uint32_t id = 0;
    Vec3 position;
    float radius = 0.5f;   // cross-section radius at this point
    float weight = 1.0f;    // spline weight for curvature control
};

/// Where a turret or weapon can be mounted (visible in FPS view).
struct TurretHardpoint {
    uint32_t id = 0;
    Vec3 position;
    Vec3 direction;         // forward aim direction
    std::string size;       // "small", "medium", "large", "capital"
    std::string weaponType; // "turret", "launcher", "utility"
};

/// A point light or spot light inside/outside the ship.
struct ShipLight {
    uint32_t id = 0;
    Vec3 position;
    Vec3 color;             // RGB 0-1
    float intensity = 1.0f;
    float range = 10.0f;
    std::string type;       // "point", "spot", "ambient"
    bool interior = true;   // true = interior light, false = exterior nav light
};

/// A named interior section (bridge, cargo bay, engine room, etc.).
/// Players walk through these in FPS mode.
struct InteriorSection {
    uint32_t id = 0;
    std::string name;       // "cockpit", "bridge", "cargo_bay", "engine_room", "crew_quarters"
    Vec3 boundsMin;
    Vec3 boundsMax;
    bool hasCockpit = false; // true if this section has an FPS flight cockpit view
};

/// Visual feature attached to the hull (fins, antennae, vents, etc.)
struct VisualFeature {
    uint32_t id = 0;
    std::string type;       // "fin", "antenna", "vent", "intake", "wing", "nacelle"
    Vec3 position;
    Vec3 scale;
    float rotation = 0.0f;  // degrees around local Y axis
};

/// Complete ship hull definition combining all elements.
struct ShipHullData {
    std::string shipId;
    std::string shipName;
    std::string shipClass;
    uint64_t generationSeed = 0;

    std::vector<HullControlPoint> controlPoints;
    std::vector<TurretHardpoint> turretHardpoints;
    std::vector<ShipLight> lights;
    std::vector<InteriorSection> interiors;
    std::vector<VisualFeature> visualFeatures;

    MeshData generatedMesh;

    bool IsValid() const {
        return !controlPoints.empty() && !shipId.empty();
    }
};

/// Records a single user edit for PCG learning feedback.
struct PCGEditRecord {
    std::string editType;   // "move_control_point", "add_hardpoint", "add_light", etc.
    std::string shipClass;
    uint32_t targetId = 0;
    Vec3 oldValue;
    Vec3 newValue;
    uint64_t timestamp = 0;
};

/// Aggregated statistics from user edits that the PCG uses to generate
/// better defaults over time.
struct PCGLearningProfile {
    std::string shipClass;
    float avgControlPointCount = 0.0f;
    float avgHullLength = 0.0f;
    float avgHullWidth = 0.0f;
    float avgTurretCount = 0.0f;
    float avgLightCount = 0.0f;
    float avgInteriorCount = 0.0f;
    uint32_t sampleCount = 0;

    void Incorporate(const ShipHullData& hull);
};

// ---------- Ship Hull Graph ----------

enum class ShipNodeType : uint8_t {
    HullSpline,       // generates hull mesh from control points
    TurretMount,      // places turret hardpoint geometry
    LightFixture,     // places a light source
    InteriorVolume,   // defines an interior walkable space
    VisualAttachment, // attaches visual features
    Merge,            // merges two meshes
    Output            // final output node
};

struct ShipGraphNode {
    uint32_t id = 0;
    ShipNodeType type;
    std::vector<std::pair<std::string, std::string>> properties;

    std::string GetProperty(const std::string& key, const std::string& def = "") const {
        for (const auto& p : properties) {
            if (p.first == key) return p.second;
        }
        return def;
    }
};

struct ShipGraphEdge {
    uint32_t fromNode;
    uint16_t fromPort;
    uint32_t toNode;
    uint16_t toPort;
};

class ShipHullGraph {
public:
    // -- Node management --
    uint32_t AddNode(ShipNodeType type);
    void SetNodeProperty(uint32_t id, const std::string& key, const std::string& value);
    void RemoveNode(uint32_t id);
    void AddEdge(const ShipGraphEdge& edge);

    // -- Compilation & execution --
    bool Compile();
    bool Execute();
    bool IsCompiled() const;
    size_t NodeCount() const;
    const MeshData* GetOutput() const;

    // -- Hull control points (editor-editable) --
    uint32_t AddControlPoint(const HullControlPoint& cp);
    void MoveControlPoint(uint32_t id, const Vec3& newPos);
    void SetControlPointRadius(uint32_t id, float radius);
    void RemoveControlPoint(uint32_t id);
    const std::vector<HullControlPoint>& ControlPoints() const;
    const HullControlPoint* GetControlPoint(uint32_t id) const;

    // -- Turret hardpoints --
    uint32_t AddTurretHardpoint(const TurretHardpoint& hp);
    void RemoveTurretHardpoint(uint32_t id);
    void MoveTurretHardpoint(uint32_t id, const Vec3& newPos);
    const std::vector<TurretHardpoint>& TurretHardpoints() const;

    // -- Lights --
    uint32_t AddLight(const ShipLight& light);
    void RemoveLight(uint32_t id);
    void MoveLight(uint32_t id, const Vec3& newPos);
    const std::vector<ShipLight>& Lights() const;

    // -- Interior sections --
    uint32_t AddInterior(const InteriorSection& section);
    void RemoveInterior(uint32_t id);
    const std::vector<InteriorSection>& Interiors() const;

    // -- Visual features --
    uint32_t AddVisualFeature(const VisualFeature& feature);
    void RemoveVisualFeature(uint32_t id);
    const std::vector<VisualFeature>& VisualFeatures() const;

    // -- PCG seed-based generation --
    void GenerateFromSeed(uint64_t seed, const std::string& shipClass);

    // -- PCG learning --
    void RecordEdit(const PCGEditRecord& record);
    const std::vector<PCGEditRecord>& EditHistory() const;
    void ApplyLearningProfile(const PCGLearningProfile& profile);
    ShipHullData ExportHullData() const;

private:
    uint32_t m_nextNodeID = 1;
    uint32_t m_nextElementID = 1;
    std::unordered_map<uint32_t, ShipGraphNode> m_nodes;
    std::vector<ShipGraphEdge> m_edges;
    std::vector<uint32_t> m_executionOrder;
    bool m_compiled = false;

    std::vector<HullControlPoint> m_controlPoints;
    std::vector<TurretHardpoint> m_turretHardpoints;
    std::vector<ShipLight> m_lights;
    std::vector<InteriorSection> m_interiors;
    std::vector<VisualFeature> m_visualFeatures;

    std::vector<PCGEditRecord> m_editHistory;

    // Per-node intermediate mesh results
    std::unordered_map<uint64_t, MeshData> m_outputs;

    bool HasCycle() const;
    void ExecuteNode(const ShipGraphNode& node);
};

}
