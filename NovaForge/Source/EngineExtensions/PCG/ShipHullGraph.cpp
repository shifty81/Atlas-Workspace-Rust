#include "ShipHullGraph.h"
#include "ShipHullNodes.h"
#include <algorithm>
#include <queue>
#include <cmath>

namespace atlas::procedural {

static float SafeStofGraph(const std::string& s, float def) {
    try { return s.empty() ? def : std::stof(s); }
    catch (...) { return def; }
}

static int SafeStoiGraph(const std::string& s, int def) {
    try { return s.empty() ? def : std::stoi(s); }
    catch (...) { return def; }
}

static uint64_t SafeStoullGraph(const std::string& s, uint64_t def) {
    try { return s.empty() ? def : std::stoull(s); }
    catch (...) { return def; }
}

// ---------- PCGLearningProfile ----------

void PCGLearningProfile::Incorporate(const ShipHullData& hull) {
    float n = static_cast<float>(sampleCount);
    float n1 = n + 1.0f;

    avgControlPointCount = (avgControlPointCount * n + static_cast<float>(hull.controlPoints.size())) / n1;
    avgTurretCount = (avgTurretCount * n + static_cast<float>(hull.turretHardpoints.size())) / n1;
    avgLightCount = (avgLightCount * n + static_cast<float>(hull.lights.size())) / n1;
    avgInteriorCount = (avgInteriorCount * n + static_cast<float>(hull.interiors.size())) / n1;

    // Compute hull extents from control points
    if (!hull.controlPoints.empty()) {
        float minX = hull.controlPoints.front().position.x;
        float maxX = minX;
        float maxR = 0.0f;
        for (const auto& cp : hull.controlPoints) {
            if (cp.position.x < minX) minX = cp.position.x;
            if (cp.position.x > maxX) maxX = cp.position.x;
            if (cp.radius > maxR) maxR = cp.radius;
        }
        avgHullLength = (avgHullLength * n + (maxX - minX)) / n1;
        avgHullWidth = (avgHullWidth * n + maxR * 2.0f) / n1;
    }

    sampleCount++;
}

// ---------- Node management ----------

uint32_t ShipHullGraph::AddNode(ShipNodeType type) {
    uint32_t id = m_nextNodeID++;
    ShipGraphNode node;
    node.id = id;
    node.type = type;
    m_nodes[id] = std::move(node);
    m_compiled = false;
    return id;
}

void ShipHullGraph::SetNodeProperty(uint32_t id, const std::string& key, const std::string& value) {
    auto it = m_nodes.find(id);
    if (it == m_nodes.end()) return;
    for (auto& p : it->second.properties) {
        if (p.first == key) {
            p.second = value;
            m_compiled = false;
            return;
        }
    }
    it->second.properties.push_back({key, value});
    m_compiled = false;
}

void ShipHullGraph::RemoveNode(uint32_t id) {
    m_nodes.erase(id);
    m_edges.erase(
        std::remove_if(m_edges.begin(), m_edges.end(),
            [id](const ShipGraphEdge& e) {
                return e.fromNode == id || e.toNode == id;
            }),
        m_edges.end()
    );
    m_compiled = false;
}

void ShipHullGraph::AddEdge(const ShipGraphEdge& edge) {
    m_edges.push_back(edge);
    m_compiled = false;
}

// ---------- Compile ----------

bool ShipHullGraph::HasCycle() const {
    std::unordered_map<uint32_t, int> inDegree;
    for (const auto& [id, _] : m_nodes) {
        inDegree[id] = 0;
    }
    for (const auto& e : m_edges) {
        if (inDegree.count(e.toNode)) {
            inDegree[e.toNode]++;
        }
    }

    std::queue<uint32_t> q;
    for (const auto& [id, deg] : inDegree) {
        if (deg == 0) q.push(id);
    }

    int visited = 0;
    while (!q.empty()) {
        uint32_t n = q.front();
        q.pop();
        visited++;
        for (const auto& e : m_edges) {
            if (e.fromNode == n) {
                if (--inDegree[e.toNode] == 0) {
                    q.push(e.toNode);
                }
            }
        }
    }
    return visited != static_cast<int>(m_nodes.size());
}

bool ShipHullGraph::Compile() {
    m_compiled = false;
    m_executionOrder.clear();

    if (HasCycle()) return false;

    std::unordered_map<uint32_t, int> inDegree;
    for (const auto& [id, _] : m_nodes) {
        inDegree[id] = 0;
    }
    for (const auto& e : m_edges) {
        inDegree[e.toNode]++;
    }

    std::queue<uint32_t> q;
    for (const auto& [id, deg] : inDegree) {
        if (deg == 0) q.push(id);
    }

    while (!q.empty()) {
        uint32_t n = q.front();
        q.pop();
        m_executionOrder.push_back(n);
        for (const auto& e : m_edges) {
            if (e.fromNode == n) {
                if (--inDegree[e.toNode] == 0) {
                    q.push(e.toNode);
                }
            }
        }
    }

    m_compiled = (m_executionOrder.size() == m_nodes.size());
    return m_compiled;
}

// ---------- Execute ----------

void ShipHullGraph::ExecuteNode(const ShipGraphNode& node) {
    MeshData input0, input1;
    for (const auto& e : m_edges) {
        if (e.toNode == node.id) {
            uint64_t key = (static_cast<uint64_t>(e.fromNode) << 16) | e.fromPort;
            auto outIt = m_outputs.find(key);
            if (outIt != m_outputs.end()) {
                if (e.toPort == 0) input0 = outIt->second;
                else if (e.toPort == 1) input1 = outIt->second;
            }
        }
    }

    MeshData result;

    switch (node.type) {
        case ShipNodeType::HullSpline: {
            int segments = SafeStoiGraph(node.GetProperty("segments"), 8);
            result = GenerateHullFromControlPoints(m_controlPoints, segments);
            break;
        }
        case ShipNodeType::TurretMount: {
            // Generate all turret mount meshes merged together
            MeshData merged;
            for (const auto& hp : m_turretHardpoints) {
                MeshData mount = GenerateTurretMount(hp.position, hp.direction, hp.size);
                uint32_t offset = static_cast<uint32_t>(merged.VertexCount());
                merged.vertices.insert(merged.vertices.end(),
                                       mount.vertices.begin(), mount.vertices.end());
                merged.normals.insert(merged.normals.end(),
                                      mount.normals.begin(), mount.normals.end());
                for (uint32_t idx : mount.indices) {
                    merged.indices.push_back(idx + offset);
                }
            }
            result = merged;
            break;
        }
        case ShipNodeType::LightFixture: {
            MeshData merged;
            for (const auto& light : m_lights) {
                MeshData fixture = GenerateLightFixture(light.position, light.range, light.interior);
                uint32_t offset = static_cast<uint32_t>(merged.VertexCount());
                merged.vertices.insert(merged.vertices.end(),
                                       fixture.vertices.begin(), fixture.vertices.end());
                merged.normals.insert(merged.normals.end(),
                                      fixture.normals.begin(), fixture.normals.end());
                for (uint32_t idx : fixture.indices) {
                    merged.indices.push_back(idx + offset);
                }
            }
            result = merged;
            break;
        }
        case ShipNodeType::InteriorVolume: {
            MeshData merged;
            for (const auto& section : m_interiors) {
                MeshData vol = GenerateInteriorVolume(section.boundsMin, section.boundsMax);
                uint32_t offset = static_cast<uint32_t>(merged.VertexCount());
                merged.vertices.insert(merged.vertices.end(),
                                       vol.vertices.begin(), vol.vertices.end());
                merged.normals.insert(merged.normals.end(),
                                      vol.normals.begin(), vol.normals.end());
                for (uint32_t idx : vol.indices) {
                    merged.indices.push_back(idx + offset);
                }
            }
            result = merged;
            break;
        }
        case ShipNodeType::VisualAttachment: {
            MeshData merged;
            for (const auto& feat : m_visualFeatures) {
                MeshData fm = GenerateVisualFeature(feat);
                uint32_t offset = static_cast<uint32_t>(merged.VertexCount());
                merged.vertices.insert(merged.vertices.end(),
                                       fm.vertices.begin(), fm.vertices.end());
                merged.normals.insert(merged.normals.end(),
                                      fm.normals.begin(), fm.normals.end());
                for (uint32_t idx : fm.indices) {
                    merged.indices.push_back(idx + offset);
                }
            }
            result = merged;
            break;
        }
        case ShipNodeType::Merge: {
            result = input0;
            uint32_t offset = static_cast<uint32_t>(result.VertexCount());
            result.vertices.insert(result.vertices.end(),
                                   input1.vertices.begin(), input1.vertices.end());
            result.normals.insert(result.normals.end(),
                                  input1.normals.begin(), input1.normals.end());
            for (uint32_t idx : input1.indices) {
                result.indices.push_back(idx + offset);
            }
            break;
        }
        case ShipNodeType::Output: {
            result = input0;
            break;
        }
    }

    uint64_t key = (static_cast<uint64_t>(node.id) << 16) | 0;
    m_outputs[key] = std::move(result);
}

bool ShipHullGraph::Execute() {
    if (!m_compiled) return false;

    m_outputs.clear();

    for (uint32_t id : m_executionOrder) {
        auto it = m_nodes.find(id);
        if (it == m_nodes.end()) return false;
        ExecuteNode(it->second);
    }

    return true;
}

bool ShipHullGraph::IsCompiled() const {
    return m_compiled;
}

size_t ShipHullGraph::NodeCount() const {
    return m_nodes.size();
}

const MeshData* ShipHullGraph::GetOutput() const {
    for (const auto& [id, node] : m_nodes) {
        if (node.type == ShipNodeType::Output) {
            uint64_t key = (static_cast<uint64_t>(id) << 16) | 0;
            auto it = m_outputs.find(key);
            if (it != m_outputs.end()) return &it->second;
        }
    }
    return nullptr;
}

// ---------- Control Points ----------

uint32_t ShipHullGraph::AddControlPoint(const HullControlPoint& cp) {
    HullControlPoint newCp = cp;
    newCp.id = m_nextElementID++;
    m_controlPoints.push_back(newCp);
    m_compiled = false;
    return newCp.id;
}

void ShipHullGraph::MoveControlPoint(uint32_t id, const Vec3& newPos) {
    for (auto& cp : m_controlPoints) {
        if (cp.id == id) {
            cp.position = newPos;
            m_compiled = false;
            return;
        }
    }
}

void ShipHullGraph::SetControlPointRadius(uint32_t id, float radius) {
    for (auto& cp : m_controlPoints) {
        if (cp.id == id) {
            cp.radius = radius;
            m_compiled = false;
            return;
        }
    }
}

void ShipHullGraph::RemoveControlPoint(uint32_t id) {
    m_controlPoints.erase(
        std::remove_if(m_controlPoints.begin(), m_controlPoints.end(),
            [id](const HullControlPoint& cp) { return cp.id == id; }),
        m_controlPoints.end()
    );
    m_compiled = false;
}

const std::vector<HullControlPoint>& ShipHullGraph::ControlPoints() const {
    return m_controlPoints;
}

const HullControlPoint* ShipHullGraph::GetControlPoint(uint32_t id) const {
    for (const auto& cp : m_controlPoints) {
        if (cp.id == id) return &cp;
    }
    return nullptr;
}

// ---------- Turret Hardpoints ----------

uint32_t ShipHullGraph::AddTurretHardpoint(const TurretHardpoint& hp) {
    TurretHardpoint newHp = hp;
    newHp.id = m_nextElementID++;
    m_turretHardpoints.push_back(newHp);
    m_compiled = false;
    return newHp.id;
}

void ShipHullGraph::RemoveTurretHardpoint(uint32_t id) {
    m_turretHardpoints.erase(
        std::remove_if(m_turretHardpoints.begin(), m_turretHardpoints.end(),
            [id](const TurretHardpoint& hp) { return hp.id == id; }),
        m_turretHardpoints.end()
    );
    m_compiled = false;
}

void ShipHullGraph::MoveTurretHardpoint(uint32_t id, const Vec3& newPos) {
    for (auto& hp : m_turretHardpoints) {
        if (hp.id == id) {
            hp.position = newPos;
            m_compiled = false;
            return;
        }
    }
}

const std::vector<TurretHardpoint>& ShipHullGraph::TurretHardpoints() const {
    return m_turretHardpoints;
}

// ---------- Lights ----------

uint32_t ShipHullGraph::AddLight(const ShipLight& light) {
    ShipLight newLight = light;
    newLight.id = m_nextElementID++;
    m_lights.push_back(newLight);
    m_compiled = false;
    return newLight.id;
}

void ShipHullGraph::RemoveLight(uint32_t id) {
    m_lights.erase(
        std::remove_if(m_lights.begin(), m_lights.end(),
            [id](const ShipLight& l) { return l.id == id; }),
        m_lights.end()
    );
    m_compiled = false;
}

void ShipHullGraph::MoveLight(uint32_t id, const Vec3& newPos) {
    for (auto& l : m_lights) {
        if (l.id == id) {
            l.position = newPos;
            m_compiled = false;
            return;
        }
    }
}

const std::vector<ShipLight>& ShipHullGraph::Lights() const {
    return m_lights;
}

// ---------- Interior Sections ----------

uint32_t ShipHullGraph::AddInterior(const InteriorSection& section) {
    InteriorSection newSec = section;
    newSec.id = m_nextElementID++;
    m_interiors.push_back(newSec);
    m_compiled = false;
    return newSec.id;
}

void ShipHullGraph::RemoveInterior(uint32_t id) {
    m_interiors.erase(
        std::remove_if(m_interiors.begin(), m_interiors.end(),
            [id](const InteriorSection& s) { return s.id == id; }),
        m_interiors.end()
    );
    m_compiled = false;
}

const std::vector<InteriorSection>& ShipHullGraph::Interiors() const {
    return m_interiors;
}

// ---------- Visual Features ----------

uint32_t ShipHullGraph::AddVisualFeature(const VisualFeature& feature) {
    VisualFeature newF = feature;
    newF.id = m_nextElementID++;
    m_visualFeatures.push_back(newF);
    m_compiled = false;
    return newF.id;
}

void ShipHullGraph::RemoveVisualFeature(uint32_t id) {
    m_visualFeatures.erase(
        std::remove_if(m_visualFeatures.begin(), m_visualFeatures.end(),
            [id](const VisualFeature& f) { return f.id == id; }),
        m_visualFeatures.end()
    );
    m_compiled = false;
}

const std::vector<VisualFeature>& ShipHullGraph::VisualFeatures() const {
    return m_visualFeatures;
}

// ---------- Seed-based generation ----------

void ShipHullGraph::GenerateFromSeed(uint64_t seed, const std::string& shipClass) {
    m_controlPoints = GenerateDefaultControlPoints(seed, shipClass);
    m_turretHardpoints = GenerateDefaultHardpoints(seed, shipClass, 3, 1);
    m_interiors = GenerateDefaultInteriors(seed, shipClass);
    m_lights = GenerateDefaultLights(seed, shipClass);

    // Add a default visual feature
    VisualFeature fin;
    fin.type = "fin";
    fin.position = {0.0f, 1.5f, 0.0f};
    fin.scale = {0.3f, 0.8f, 1.0f};
    fin.rotation = 0.0f;
    m_visualFeatures.push_back(fin);

    // Ensure m_nextElementID is higher than any existing element ID
    uint32_t maxId = 0;
    for (const auto& cp : m_controlPoints) { if (cp.id > maxId) maxId = cp.id; }
    for (const auto& hp : m_turretHardpoints) { if (hp.id > maxId) maxId = hp.id; }
    for (const auto& l : m_lights) { if (l.id > maxId) maxId = l.id; }
    for (const auto& s : m_interiors) { if (s.id > maxId) maxId = s.id; }
    m_nextElementID = maxId + 1;
    m_visualFeatures.back().id = m_nextElementID++;

    m_compiled = false;
}

// ---------- PCG Learning ----------

void ShipHullGraph::RecordEdit(const PCGEditRecord& record) {
    m_editHistory.push_back(record);
}

const std::vector<PCGEditRecord>& ShipHullGraph::EditHistory() const {
    return m_editHistory;
}

void ShipHullGraph::ApplyLearningProfile(const PCGLearningProfile& profile) {
    // Adjust current hull based on learned averages.
    // Scale control point count towards average if we have data.
    if (profile.sampleCount > 0 && !m_controlPoints.empty()) {
        // Adjust radii to trend towards average hull width
        float currentMaxR = 0.0f;
        for (const auto& cp : m_controlPoints) {
            if (cp.radius > currentMaxR) currentMaxR = cp.radius;
        }
        if (currentMaxR > 0.0f && profile.avgHullWidth > 0.0f) {
            float scale = (profile.avgHullWidth / 2.0f) / currentMaxR;
            // Blend 30% towards the learned profile
            float blendScale = 1.0f + (scale - 1.0f) * 0.3f;
            for (auto& cp : m_controlPoints) {
                cp.radius *= blendScale;
            }
        }
    }
}

ShipHullData ShipHullGraph::ExportHullData() const {
    ShipHullData data;
    data.controlPoints = m_controlPoints;
    data.turretHardpoints = m_turretHardpoints;
    data.lights = m_lights;
    data.interiors = m_interiors;
    data.visualFeatures = m_visualFeatures;

    const MeshData* output = GetOutput();
    if (output) {
        data.generatedMesh = *output;
    }

    return data;
}

}
