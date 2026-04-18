#pragma once
#include "../ui/EditorPanel.h"
#include "../../engine/procedural/ShipHullGraph.h"
#include "../../engine/procedural/ShipHullNodes.h"
#include "../../engine/ui/UIDrawList.h"
#include <string>
#include <vector>
#include <sstream>

namespace atlas::editor {

/// Identifies which element type is currently selected in the ship editor.
enum class ShipEditorSelection : uint8_t {
    None,
    ControlPoint,
    TurretHardpoint,
    Light,
    Interior,
    VisualFeature
};

/// Active editing tool in the ship editor.
enum class ShipEditorTool : uint8_t {
    Select,
    MovePoint,
    AddControlPoint,
    AddTurretHardpoint,
    AddLight,
    AddInterior,
    AddVisualFeature,
    Delete
};

/// Ship viewport display mode.
enum class ShipViewMode : uint8_t {
    Exterior,    // Full hull view (default)
    Interior,    // Interior/FPS walkthrough
    Cockpit,     // FPS cockpit preview
    Wireframe,   // Wireframe overlay
    Systems      // Ship systems diagram
};

/// Ship editor panel — the main workspace for building ships.
/// Keeps the left sidebar (asset browser), replaces the rest of the editor
/// with a ship-focused viewport, component palette, properties inspector,
/// and ship systems diagram.
class ShipEditorPanel : public EditorPanel {
public:
    const char* Name() const override { return "Ship Editor"; }
    void Draw() override;

    // --- Ship lifecycle ---
    void NewShip(const std::string& shipClass, uint64_t seed);
    void LoadShipData(const procedural::ShipHullData& data);
    bool HasShip() const { return m_hasShip; }
    procedural::ShipHullData ExportShip() const;

    // --- Graph operations ---
    bool CompileAndGenerate();
    bool IsCompiled() const { return m_graph.IsCompiled(); }
    const procedural::MeshData* GetGeneratedMesh() const { return m_graph.GetOutput(); }

    // --- Selection ---
    void Select(ShipEditorSelection type, uint32_t id);
    void ClearSelection();
    ShipEditorSelection SelectionType() const { return m_selType; }
    uint32_t SelectionId() const { return m_selId; }

    // --- Tools ---
    void SetTool(ShipEditorTool tool) { m_tool = tool; }
    ShipEditorTool CurrentTool() const { return m_tool; }

    // --- View mode ---
    void SetViewMode(ShipViewMode mode) { m_viewMode = mode; }
    ShipViewMode ViewMode() const { return m_viewMode; }

    // --- Control point editing (click-drag) ---
    void MoveControlPoint(uint32_t id, float x, float y, float z);
    void SetControlPointRadius(uint32_t id, float radius);
    uint32_t AddControlPoint(float x, float y, float z, float radius);
    void RemoveControlPoint(uint32_t id);

    // --- Turret hardpoint editing ---
    uint32_t AddTurretHardpoint(float x, float y, float z,
                                const std::string& size, const std::string& weaponType);
    void MoveTurretHardpoint(uint32_t id, float x, float y, float z);
    void RemoveTurretHardpoint(uint32_t id);

    // --- Light editing ---
    uint32_t AddLight(float x, float y, float z, float r, float g, float b,
                      float intensity, float range, bool interior);
    void MoveLight(uint32_t id, float x, float y, float z);
    void RemoveLight(uint32_t id);

    // --- Interior editing ---
    uint32_t AddInterior(const std::string& name,
                         float minX, float minY, float minZ,
                         float maxX, float maxY, float maxZ,
                         bool hasCockpit);
    void RemoveInterior(uint32_t id);

    // --- Visual feature editing ---
    uint32_t AddVisualFeature(const std::string& type,
                              float x, float y, float z,
                              float sx, float sy, float sz);
    void RemoveVisualFeature(uint32_t id);

    // --- PCG learning ---
    void RecordEdit(const std::string& editType, uint32_t targetId,
                    float oldX, float oldY, float oldZ,
                    float newX, float newY, float newZ);
    const std::vector<procedural::PCGEditRecord>& EditHistory() const;
    void ApplyLearningProfile(const procedural::PCGLearningProfile& profile);

    // --- Ship info ---
    std::string ShipClass() const { return m_shipClass; }
    std::string ShipId() const { return m_shipId; }
    void SetShipId(const std::string& id) { m_shipId = id; }
    void SetShipName(const std::string& name) { m_shipName = name; }
    std::string ShipName() const { return m_shipName; }

    // --- Accessors ---
    const procedural::ShipHullGraph& Graph() const { return m_graph; }
    size_t ControlPointCount() const { return m_graph.ControlPoints().size(); }
    size_t TurretHardpointCount() const { return m_graph.TurretHardpoints().size(); }
    size_t LightCount() const { return m_graph.Lights().size(); }
    size_t InteriorCount() const { return m_graph.Interiors().size(); }
    size_t VisualFeatureCount() const { return m_graph.VisualFeatures().size(); }

    bool IsDirty() const { return m_dirty; }
    void MarkClean() { m_dirty = false; }

    bool HasCockpit() const;

    std::string Summary() const;

    const atlas::ui::UIDrawList& GetDrawList() const { return m_drawList; }

private:
    procedural::ShipHullGraph m_graph;
    bool m_hasShip = false;
    bool m_dirty = false;
    std::string m_shipId;
    std::string m_shipName;
    std::string m_shipClass;
    uint64_t m_seed = 0;

    ShipEditorSelection m_selType = ShipEditorSelection::None;
    uint32_t m_selId = 0;
    ShipEditorTool m_tool = ShipEditorTool::Select;
    ShipViewMode m_viewMode = ShipViewMode::Exterior;

    atlas::ui::UIDrawList m_drawList;

    void SetupDefaultGraph();
    void DrawToolbar(int32_t& y);
    void DrawViewport(int32_t y, int32_t height);
    void DrawProperties(int32_t y);
    void DrawComponentPalette(int32_t y);
    void DrawShipSystems(int32_t y);
};

} // namespace atlas::editor
