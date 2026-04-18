#include "ShipEditorPanel.h"
#include <cstdio>

namespace atlas::editor {

// --- Ship lifecycle ---

void ShipEditorPanel::NewShip(const std::string& shipClass, uint64_t seed) {
    m_graph = procedural::ShipHullGraph();
    m_shipClass = shipClass;
    m_seed = seed;
    m_shipId = "";
    m_shipName = "";

    m_graph.GenerateFromSeed(seed, shipClass);
    SetupDefaultGraph();

    m_hasShip = true;
    m_dirty = true;
    ClearSelection();
}

void ShipEditorPanel::LoadShipData(const procedural::ShipHullData& data) {
    m_graph = procedural::ShipHullGraph();
    m_shipId = data.shipId;
    m_shipName = data.shipName;
    m_shipClass = data.shipClass;
    m_seed = data.generationSeed;

    for (const auto& cp : data.controlPoints) {
        m_graph.AddControlPoint(cp);
    }
    for (const auto& hp : data.turretHardpoints) {
        m_graph.AddTurretHardpoint(hp);
    }
    for (const auto& l : data.lights) {
        m_graph.AddLight(l);
    }
    for (const auto& s : data.interiors) {
        m_graph.AddInterior(s);
    }
    for (const auto& f : data.visualFeatures) {
        m_graph.AddVisualFeature(f);
    }

    SetupDefaultGraph();
    m_hasShip = true;
    m_dirty = false;
    ClearSelection();
}

procedural::ShipHullData ShipEditorPanel::ExportShip() const {
    auto data = m_graph.ExportHullData();
    data.shipId = m_shipId;
    data.shipName = m_shipName;
    data.shipClass = m_shipClass;
    data.generationSeed = m_seed;
    return data;
}

void ShipEditorPanel::SetupDefaultGraph() {
    // Build the standard ship graph pipeline:
    // HullSpline --\
    // TurretMount --+-- Merge --> Merge --> Merge --> Merge --> Output
    // LightFixture -/      |        |         |
    // InteriorVolume ------/        |         |
    // VisualAttachment -------------/         |

    auto hull = m_graph.AddNode(procedural::ShipNodeType::HullSpline);
    m_graph.SetNodeProperty(hull, "segments", "8");

    auto turrets = m_graph.AddNode(procedural::ShipNodeType::TurretMount);
    auto lights = m_graph.AddNode(procedural::ShipNodeType::LightFixture);
    auto interiors = m_graph.AddNode(procedural::ShipNodeType::InteriorVolume);
    auto visuals = m_graph.AddNode(procedural::ShipNodeType::VisualAttachment);

    auto merge1 = m_graph.AddNode(procedural::ShipNodeType::Merge);
    auto merge2 = m_graph.AddNode(procedural::ShipNodeType::Merge);
    auto merge3 = m_graph.AddNode(procedural::ShipNodeType::Merge);
    auto merge4 = m_graph.AddNode(procedural::ShipNodeType::Merge);
    auto output = m_graph.AddNode(procedural::ShipNodeType::Output);

    m_graph.AddEdge({hull, 0, merge1, 0});
    m_graph.AddEdge({turrets, 0, merge1, 1});
    m_graph.AddEdge({merge1, 0, merge2, 0});
    m_graph.AddEdge({lights, 0, merge2, 1});
    m_graph.AddEdge({merge2, 0, merge3, 0});
    m_graph.AddEdge({interiors, 0, merge3, 1});
    m_graph.AddEdge({merge3, 0, merge4, 0});
    m_graph.AddEdge({visuals, 0, merge4, 1});
    m_graph.AddEdge({merge4, 0, output, 0});
}

bool ShipEditorPanel::CompileAndGenerate() {
    if (!m_graph.Compile()) return false;
    return m_graph.Execute();
}

// --- Selection ---

void ShipEditorPanel::Select(ShipEditorSelection type, uint32_t id) {
    m_selType = type;
    m_selId = id;
}

void ShipEditorPanel::ClearSelection() {
    m_selType = ShipEditorSelection::None;
    m_selId = 0;
}

// --- Control point editing ---

void ShipEditorPanel::MoveControlPoint(uint32_t id, float x, float y, float z) {
    auto oldCp = m_graph.GetControlPoint(id);
    if (oldCp) {
        RecordEdit("move_control_point", id,
                   oldCp->position.x, oldCp->position.y, oldCp->position.z,
                   x, y, z);
    }
    m_graph.MoveControlPoint(id, {x, y, z});
    m_dirty = true;
}

void ShipEditorPanel::SetControlPointRadius(uint32_t id, float radius) {
    m_graph.SetControlPointRadius(id, radius);
    m_dirty = true;
}

uint32_t ShipEditorPanel::AddControlPoint(float x, float y, float z, float radius) {
    procedural::HullControlPoint cp;
    cp.position = {x, y, z};
    cp.radius = radius;
    cp.weight = 1.0f;
    uint32_t id = m_graph.AddControlPoint(cp);
    m_dirty = true;
    return id;
}

void ShipEditorPanel::RemoveControlPoint(uint32_t id) {
    m_graph.RemoveControlPoint(id);
    if (m_selType == ShipEditorSelection::ControlPoint && m_selId == id) {
        ClearSelection();
    }
    m_dirty = true;
}

// --- Turret hardpoint editing ---

uint32_t ShipEditorPanel::AddTurretHardpoint(float x, float y, float z,
                                              const std::string& size,
                                              const std::string& weaponType) {
    procedural::TurretHardpoint hp;
    hp.position = {x, y, z};
    hp.direction = {0.0f, 1.0f, 0.0f};
    hp.size = size;
    hp.weaponType = weaponType;
    uint32_t id = m_graph.AddTurretHardpoint(hp);
    m_dirty = true;
    return id;
}

void ShipEditorPanel::MoveTurretHardpoint(uint32_t id, float x, float y, float z) {
    m_graph.MoveTurretHardpoint(id, {x, y, z});
    m_dirty = true;
}

void ShipEditorPanel::RemoveTurretHardpoint(uint32_t id) {
    m_graph.RemoveTurretHardpoint(id);
    if (m_selType == ShipEditorSelection::TurretHardpoint && m_selId == id) {
        ClearSelection();
    }
    m_dirty = true;
}

// --- Light editing ---

uint32_t ShipEditorPanel::AddLight(float x, float y, float z,
                                    float r, float g, float b,
                                    float intensity, float range, bool interior) {
    procedural::ShipLight light;
    light.position = {x, y, z};
    light.color = {r, g, b};
    light.intensity = intensity;
    light.range = range;
    light.type = "point";
    light.interior = interior;
    uint32_t id = m_graph.AddLight(light);
    m_dirty = true;
    return id;
}

void ShipEditorPanel::MoveLight(uint32_t id, float x, float y, float z) {
    m_graph.MoveLight(id, {x, y, z});
    m_dirty = true;
}

void ShipEditorPanel::RemoveLight(uint32_t id) {
    m_graph.RemoveLight(id);
    if (m_selType == ShipEditorSelection::Light && m_selId == id) {
        ClearSelection();
    }
    m_dirty = true;
}

// --- Interior editing ---

uint32_t ShipEditorPanel::AddInterior(const std::string& name,
                                       float minX, float minY, float minZ,
                                       float maxX, float maxY, float maxZ,
                                       bool hasCockpit) {
    procedural::InteriorSection sec;
    sec.name = name;
    sec.boundsMin = {minX, minY, minZ};
    sec.boundsMax = {maxX, maxY, maxZ};
    sec.hasCockpit = hasCockpit;
    uint32_t id = m_graph.AddInterior(sec);
    m_dirty = true;
    return id;
}

void ShipEditorPanel::RemoveInterior(uint32_t id) {
    m_graph.RemoveInterior(id);
    if (m_selType == ShipEditorSelection::Interior && m_selId == id) {
        ClearSelection();
    }
    m_dirty = true;
}

// --- Visual feature editing ---

uint32_t ShipEditorPanel::AddVisualFeature(const std::string& type,
                                            float x, float y, float z,
                                            float sx, float sy, float sz) {
    procedural::VisualFeature feat;
    feat.type = type;
    feat.position = {x, y, z};
    feat.scale = {sx, sy, sz};
    feat.rotation = 0.0f;
    uint32_t id = m_graph.AddVisualFeature(feat);
    m_dirty = true;
    return id;
}

void ShipEditorPanel::RemoveVisualFeature(uint32_t id) {
    m_graph.RemoveVisualFeature(id);
    if (m_selType == ShipEditorSelection::VisualFeature && m_selId == id) {
        ClearSelection();
    }
    m_dirty = true;
}

// --- PCG learning ---

void ShipEditorPanel::RecordEdit(const std::string& editType, uint32_t targetId,
                                  float oldX, float oldY, float oldZ,
                                  float newX, float newY, float newZ) {
    procedural::PCGEditRecord record;
    record.editType = editType;
    record.shipClass = m_shipClass;
    record.targetId = targetId;
    record.oldValue = {oldX, oldY, oldZ};
    record.newValue = {newX, newY, newZ};
    m_graph.RecordEdit(record);
}

const std::vector<procedural::PCGEditRecord>& ShipEditorPanel::EditHistory() const {
    return m_graph.EditHistory();
}

void ShipEditorPanel::ApplyLearningProfile(const procedural::PCGLearningProfile& profile) {
    m_graph.ApplyLearningProfile(profile);
    m_dirty = true;
}

// --- Queries ---

bool ShipEditorPanel::HasCockpit() const {
    for (const auto& sec : m_graph.Interiors()) {
        if (sec.hasCockpit) return true;
    }
    return false;
}

std::string ShipEditorPanel::Summary() const {
    if (!m_hasShip) return "No ship loaded";
    std::ostringstream oss;
    oss << m_shipClass;
    if (!m_shipName.empty()) oss << " '" << m_shipName << "'";
    oss << " — " << ControlPointCount() << " hull pts"
        << ", " << TurretHardpointCount() << " turrets"
        << ", " << LightCount() << " lights"
        << ", " << InteriorCount() << " rooms"
        << ", " << VisualFeatureCount() << " features";
    if (m_dirty) oss << " (modified)";
    return oss.str();
}

// --- Draw ---

void ShipEditorPanel::Draw() {
    m_drawList.Clear();

    // Full editor background
    m_drawList.DrawRect({0, 0, 1020, 690}, {30, 31, 34, 255});

    int32_t y = 0;

    // Ship Editor toolbar
    DrawToolbar(y);

    if (!m_hasShip) {
        m_drawList.DrawText({10, y + 40, 800, 20},
            "No ship loaded. Use File > New Ship or load a NovaForge project.",
            {160, 160, 160, 255});
        return;
    }

    // Main viewport area (takes most of the space)
    int32_t viewportHeight = 420;
    DrawViewport(y, viewportHeight);
    y += viewportHeight;

    // Bottom section: Properties | Component Palette | Ship Systems
    int32_t bottomY = y;
    DrawProperties(bottomY);
    DrawComponentPalette(bottomY);
    DrawShipSystems(bottomY);
}

void ShipEditorPanel::DrawToolbar(int32_t& y) {
    // Toolbar background
    m_drawList.DrawRect({0, 0, 1020, 32}, {43, 43, 43, 255});
    m_drawList.DrawRect({0, 32, 1020, 1}, {70, 73, 75, 255});

    // Ship info
    char buf[256];
    if (m_hasShip) {
        std::snprintf(buf, sizeof(buf), "Ship: %s [%s] — Seed: %llu",
                      m_shipName.empty() ? "(unnamed)" : m_shipName.c_str(),
                      m_shipClass.c_str(),
                      static_cast<unsigned long long>(m_seed));
        m_drawList.DrawText({8, 6, 400, 20}, buf, {220, 220, 220, 255});
    }

    // Tool buttons
    const char* toolLabels[] = {"Select", "Move", "+Point", "+Turret", "+Light", "+Room", "+Feature", "Delete"};
    int32_t tx = 420;
    for (int i = 0; i < 8; ++i) {
        bool active = (static_cast<int>(m_tool) == i);
        auto bg = active ? atlas::ui::UIColor{66, 135, 245, 255}
                         : atlas::ui::UIColor{55, 58, 62, 255};
        m_drawList.DrawRect({tx, 4, 68, 24}, bg);
        m_drawList.DrawBorder({tx, 4, 68, 24}, 1, {80, 83, 88, 255});
        m_drawList.DrawText({tx + 4, 8, 60, 16}, toolLabels[i], {220, 220, 220, 255});
        tx += 72;
    }

    // View mode tabs
    const char* viewLabels[] = {"Exterior", "Interior", "Cockpit", "Wireframe", "Systems"};
    tx = 8;
    int32_t tabY = 34;
    m_drawList.DrawRect({0, 33, 1020, 26}, {38, 38, 40, 255});
    for (int i = 0; i < 5; ++i) {
        bool active = (static_cast<int>(m_viewMode) == i);
        auto bg = active ? atlas::ui::UIColor{50, 120, 220, 255}
                         : atlas::ui::UIColor{45, 45, 48, 255};
        m_drawList.DrawRect({tx, tabY, 80, 22}, bg);
        m_drawList.DrawText({tx + 4, tabY + 3, 72, 16}, viewLabels[i],
                           active ? atlas::ui::UIColor{255, 255, 255, 255}
                                  : atlas::ui::UIColor{180, 180, 180, 255});
        tx += 84;
    }

    y = 60;
}

void ShipEditorPanel::DrawViewport(int32_t y, int32_t height) {
    // Viewport background
    m_drawList.DrawRect({0, y, 1020, height}, {25, 25, 28, 255});
    m_drawList.DrawBorder({0, y, 1020, height}, 1, {60, 60, 60, 255});

    // Grid
    int32_t centerY = y + height / 2;
    int32_t centerX = 510;
    m_drawList.DrawRect({0, centerY, 1020, 1}, {40, 40, 40, 255});
    m_drawList.DrawRect({centerX, y, 1, height}, {40, 40, 40, 255});

    // Draw control points as draggable handles
    const auto& cps = m_graph.ControlPoints();
    for (const auto& cp : cps) {
        // Project 3D to 2D viewport (simple orthographic for now)
        int32_t px = centerX + static_cast<int32_t>(cp.position.x * 20.0f);
        int32_t py = centerY - static_cast<int32_t>(cp.position.y * 20.0f);
        int32_t size = static_cast<int32_t>(cp.radius * 10.0f);
        if (size < 4) size = 4;

        bool selected = (m_selType == ShipEditorSelection::ControlPoint && m_selId == cp.id);
        auto color = selected ? atlas::ui::UIColor{255, 200, 50, 255}
                              : atlas::ui::UIColor{100, 200, 255, 255};

        m_drawList.DrawRect({px - size / 2, py - size / 2, size, size}, color);
        m_drawList.DrawBorder({px - size / 2, py - size / 2, size, size}, 1,
                             {255, 255, 255, 200});
    }

    // Draw turret hardpoints
    for (const auto& hp : m_graph.TurretHardpoints()) {
        int32_t px = centerX + static_cast<int32_t>(hp.position.x * 20.0f);
        int32_t py = centerY - static_cast<int32_t>(hp.position.y * 20.0f);

        bool selected = (m_selType == ShipEditorSelection::TurretHardpoint && m_selId == hp.id);
        auto color = selected ? atlas::ui::UIColor{255, 100, 100, 255}
                              : atlas::ui::UIColor{255, 50, 50, 200};

        // Triangle marker for turret
        m_drawList.DrawRect({px - 4, py - 6, 8, 12}, color);
        m_drawList.DrawBorder({px - 4, py - 6, 8, 12}, 1, {255, 200, 200, 255});
    }

    // Draw lights
    for (const auto& l : m_graph.Lights()) {
        int32_t px = centerX + static_cast<int32_t>(l.position.x * 20.0f);
        int32_t py = centerY - static_cast<int32_t>(l.position.y * 20.0f);

        bool selected = (m_selType == ShipEditorSelection::Light && m_selId == l.id);
        uint8_t cr = static_cast<uint8_t>(l.color.x * 255);
        uint8_t cg = static_cast<uint8_t>(l.color.y * 255);
        uint8_t cb = static_cast<uint8_t>(l.color.z * 255);
        auto color = selected ? atlas::ui::UIColor{255, 255, 100, 255}
                              : atlas::ui::UIColor{cr, cg, cb, 200};

        // Diamond shape for light
        m_drawList.DrawRect({px - 3, py - 3, 6, 6}, color);

        // Range indicator
        if (selected) {
            int32_t rangeR = static_cast<int32_t>(l.range * 2.0f);
            m_drawList.DrawBorder({px - rangeR, py - rangeR, rangeR * 2, rangeR * 2}, 1,
                                 {cr, cg, cb, 80});
        }
    }

    // Draw interior sections
    for (const auto& sec : m_graph.Interiors()) {
        int32_t x0 = centerX + static_cast<int32_t>(sec.boundsMin.x * 20.0f);
        int32_t y0 = centerY - static_cast<int32_t>(sec.boundsMax.y * 20.0f);
        int32_t w = static_cast<int32_t>((sec.boundsMax.x - sec.boundsMin.x) * 20.0f);
        int32_t h = static_cast<int32_t>((sec.boundsMax.y - sec.boundsMin.y) * 20.0f);

        bool selected = (m_selType == ShipEditorSelection::Interior && m_selId == sec.id);
        auto borderColor = selected ? atlas::ui::UIColor{100, 255, 100, 255}
                                    : atlas::ui::UIColor{60, 180, 60, 120};
        auto fillColor = sec.hasCockpit
            ? atlas::ui::UIColor{60, 60, 180, 40}
            : atlas::ui::UIColor{60, 120, 60, 30};

        m_drawList.DrawRect({x0, y0, w, h}, fillColor);
        m_drawList.DrawBorder({x0, y0, w, h}, 1, borderColor);

        // Label
        m_drawList.DrawText({x0 + 2, y0 + 2, w - 4, 12}, sec.name,
                           {200, 255, 200, 200});
        if (sec.hasCockpit) {
            m_drawList.DrawText({x0 + 2, y0 + 14, w - 4, 12}, "[COCKPIT]",
                               {150, 150, 255, 200});
        }
    }

    // Status info in viewport
    char buf[128];
    std::snprintf(buf, sizeof(buf), "Hull Points: %zu  Turrets: %zu  Lights: %zu  Rooms: %zu  Features: %zu",
                  ControlPointCount(), TurretHardpointCount(), LightCount(),
                  InteriorCount(), VisualFeatureCount());
    m_drawList.DrawText({8, y + 4, 600, 16}, buf, {180, 180, 180, 200});

    // View mode label
    const char* modeLabels[] = {"EXTERIOR", "INTERIOR", "COCKPIT", "WIREFRAME", "SYSTEMS"};
    m_drawList.DrawText({8, y + height - 20, 200, 16},
                       modeLabels[static_cast<int>(m_viewMode)],
                       {100, 200, 255, 200});
}

void ShipEditorPanel::DrawProperties(int32_t y) {
    // Properties panel (left third of bottom area)
    int32_t pw = 340;
    int32_t ph = 200;
    m_drawList.DrawRect({0, y, pw, ph}, {38, 38, 40, 255});
    m_drawList.DrawRect({0, y, pw, 24}, {50, 53, 55, 255});
    m_drawList.DrawText({8, y + 4, pw - 16, 16}, "Properties", {220, 220, 220, 255});

    int32_t py = y + 28;
    char buf[128];

    if (m_selType == ShipEditorSelection::None) {
        m_drawList.DrawText({8, py, pw - 16, 16}, "No selection", {120, 120, 120, 255});
    } else if (m_selType == ShipEditorSelection::ControlPoint) {
        auto* cp = m_graph.GetControlPoint(m_selId);
        if (cp) {
            std::snprintf(buf, sizeof(buf), "Control Point #%u", cp->id);
            m_drawList.DrawText({8, py, pw - 16, 16}, buf, {200, 200, 200, 255});
            py += 20;
            std::snprintf(buf, sizeof(buf), "Pos: (%.2f, %.2f, %.2f)",
                          cp->position.x, cp->position.y, cp->position.z);
            m_drawList.DrawText({8, py, pw - 16, 16}, buf, {180, 180, 180, 255});
            py += 20;
            std::snprintf(buf, sizeof(buf), "Radius: %.2f  Weight: %.2f",
                          cp->radius, cp->weight);
            m_drawList.DrawText({8, py, pw - 16, 16}, buf, {180, 180, 180, 255});
        }
    } else if (m_selType == ShipEditorSelection::TurretHardpoint) {
        std::snprintf(buf, sizeof(buf), "Turret Hardpoint #%u", m_selId);
        m_drawList.DrawText({8, py, pw - 16, 16}, buf, {200, 200, 200, 255});
    } else if (m_selType == ShipEditorSelection::Light) {
        std::snprintf(buf, sizeof(buf), "Light #%u", m_selId);
        m_drawList.DrawText({8, py, pw - 16, 16}, buf, {200, 200, 200, 255});
    } else if (m_selType == ShipEditorSelection::Interior) {
        std::snprintf(buf, sizeof(buf), "Interior Section #%u", m_selId);
        m_drawList.DrawText({8, py, pw - 16, 16}, buf, {200, 200, 200, 255});
    } else if (m_selType == ShipEditorSelection::VisualFeature) {
        std::snprintf(buf, sizeof(buf), "Visual Feature #%u", m_selId);
        m_drawList.DrawText({8, py, pw - 16, 16}, buf, {200, 200, 200, 255});
    }
}

void ShipEditorPanel::DrawComponentPalette(int32_t y) {
    // Component palette (middle third of bottom area)
    int32_t px = 340;
    int32_t pw = 340;
    int32_t ph = 200;
    m_drawList.DrawRect({px, y, pw, ph}, {35, 35, 38, 255});
    m_drawList.DrawRect({px, y, pw, 24}, {50, 53, 55, 255});
    m_drawList.DrawText({px + 8, y + 4, pw - 16, 16}, "Component Palette", {220, 220, 220, 255});

    // Component categories
    const char* categories[] = {
        "Hull Points", "Turret Mounts", "Launchers",
        "Lights (Interior)", "Lights (Exterior)",
        "Cockpit/Bridge", "Engine Room", "Cargo Bay", "Crew Quarters",
        "Fin", "Antenna", "Vent", "Intake", "Wing", "Nacelle"
    };
    int32_t cy = y + 28;
    for (int i = 0; i < 15 && cy + 16 < y + ph; ++i) {
        m_drawList.DrawText({px + 8, cy, pw - 16, 14}, categories[i],
                           {180, 180, 180, 255});
        cy += 16;
    }
}

void ShipEditorPanel::DrawShipSystems(int32_t y) {
    // Ship systems diagram (right third of bottom area)
    int32_t px = 680;
    int32_t pw = 340;
    int32_t ph = 200;
    m_drawList.DrawRect({px, y, pw, ph}, {32, 32, 36, 255});
    m_drawList.DrawRect({px, y, pw, 24}, {50, 53, 55, 255});
    m_drawList.DrawText({px + 8, y + 4, pw - 16, 16}, "Ship Systems", {220, 220, 220, 255});

    int32_t sy = y + 28;
    char buf[128];

    // Show ship stats derived from components
    std::snprintf(buf, sizeof(buf), "Hull Points: %zu", ControlPointCount());
    m_drawList.DrawText({px + 8, sy, pw - 16, 16}, buf, {180, 180, 180, 255});
    sy += 18;

    std::snprintf(buf, sizeof(buf), "Turret Slots: %zu", TurretHardpointCount());
    m_drawList.DrawText({px + 8, sy, pw - 16, 16}, buf, {180, 180, 180, 255});
    sy += 18;

    // Count interior/exterior lights
    size_t intLights = 0, extLights = 0;
    for (const auto& l : m_graph.Lights()) {
        if (l.interior) intLights++; else extLights++;
    }
    std::snprintf(buf, sizeof(buf), "Interior Lights: %zu  Nav Lights: %zu", intLights, extLights);
    m_drawList.DrawText({px + 8, sy, pw - 16, 16}, buf, {180, 180, 180, 255});
    sy += 18;

    std::snprintf(buf, sizeof(buf), "Interior Sections: %zu", InteriorCount());
    m_drawList.DrawText({px + 8, sy, pw - 16, 16}, buf, {180, 180, 180, 255});
    sy += 18;

    bool hasCockpit = HasCockpit();
    m_drawList.DrawText({px + 8, sy, pw - 16, 16},
                       hasCockpit ? "FPS Cockpit: YES" : "FPS Cockpit: NO",
                       hasCockpit ? atlas::ui::UIColor{80, 200, 120, 255}
                                  : atlas::ui::UIColor{230, 80, 80, 255});
    sy += 18;

    std::snprintf(buf, sizeof(buf), "Visual Features: %zu", VisualFeatureCount());
    m_drawList.DrawText({px + 8, sy, pw - 16, 16}, buf, {180, 180, 180, 255});
    sy += 18;

    // PCG learning stats
    std::snprintf(buf, sizeof(buf), "PCG Edits Recorded: %zu", EditHistory().size());
    m_drawList.DrawText({px + 8, sy, pw - 16, 16}, buf, {140, 140, 200, 255});
}

} // namespace atlas::editor
