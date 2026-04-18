#pragma once
// NF::Editor — Editor undo system + commands
#include "NF/Core/Core.h"
#include "NF/Engine/Engine.h"
#include "NF/Renderer/Renderer.h"
#include "NF/UI/UI.h"
#include "NF/GraphVM/GraphVM.h"
#include "NF/Input/Input.h"
#include "NF/UI/UIWidgets.h"
#include <filesystem>
#include <fstream>
#include <set>
#include <map>
#include <deque>
#include <unordered_map>
#include "NF/Editor/Scene/EntityPlacement.h"
#include "NF/Editor/Scene/VoxelPaint.h"
#include "NF/Editor/Legacy/PCGTuning.h"

namespace NF {

class PlaceEntityCommand : public ICommand {
public:
    PlaceEntityCommand(EntityPlacementTool& tool, PlacedEntity entity)
        : m_tool(tool), m_entity(std::move(entity)) {}

    void execute() override { m_tool.addEntity(m_entity); }
    void undo() override    { m_tool.removeEntity(m_entity.entityId); }
    [[nodiscard]] std::string description() const override {
        return "Place Entity " + m_entity.templateName;
    }

private:
    EntityPlacementTool& m_tool;
    PlacedEntity m_entity;
};

class RemoveEntityCommand : public ICommand {
public:
    RemoveEntityCommand(EntityPlacementTool& tool, EntityID id)
        : m_tool(tool), m_id(id) {
        for (auto& e : m_tool.placedEntities()) {
            if (e.entityId == id) { m_entity = e; break; }
        }
    }

    void execute() override { m_tool.removeEntity(m_id); }
    void undo() override    { m_tool.addEntity(m_entity); }
    [[nodiscard]] std::string description() const override {
        return "Remove Entity " + std::to_string(m_id);
    }

private:
    EntityPlacementTool& m_tool;
    EntityID m_id;
    PlacedEntity m_entity;
};

class PaintStrokeCommand : public ICommand {
public:
    PaintStrokeCommand(VoxelPaintTool& tool, PaintStroke stroke)
        : m_tool(tool), m_stroke(std::move(stroke)) {}

    void execute() override { m_tool.addStroke(m_stroke); }
    void undo() override    { m_tool.removeLastStroke(); }
    [[nodiscard]] std::string description() const override {
        return "Paint Stroke (" + std::to_string(m_stroke.positions.size()) + " voxels)";
    }

private:
    VoxelPaintTool& m_tool;
    PaintStroke m_stroke;
};

class PCGParamChangeCommand : public ICommand {
public:
    PCGParamChangeCommand(PCGTuningPanel& panel, NoiseParams oldParams, NoiseParams newParams)
        : m_panel(panel), m_old(std::move(oldParams)), m_new(std::move(newParams)) {}

    void execute() override { m_panel.setNoiseParams(m_new); }
    void undo() override    { m_panel.setNoiseParams(m_old); }
    [[nodiscard]] std::string description() const override { return "Change PCG Parameters"; }

private:
    PCGTuningPanel& m_panel;
    NoiseParams m_old;
    NoiseParams m_new;
};

class EditorUndoSystem {
public:
    explicit EditorUndoSystem(CommandStack& stack) : m_stack(stack) {}

    void executePlaceEntity(EntityPlacementTool& tool, const PlacedEntity& entity) {
        m_stack.execute(std::make_unique<PlaceEntityCommand>(tool, entity));
    }

    void executeRemoveEntity(EntityPlacementTool& tool, EntityID id) {
        m_stack.execute(std::make_unique<RemoveEntityCommand>(tool, id));
    }

    void executePaintStroke(VoxelPaintTool& tool, const PaintStroke& stroke) {
        m_stack.execute(std::make_unique<PaintStrokeCommand>(tool, stroke));
    }

    void executePCGChange(PCGTuningPanel& panel, const NoiseParams& oldP, const NoiseParams& newP) {
        m_stack.execute(std::make_unique<PCGParamChangeCommand>(panel, oldP, newP));
    }

    bool undo()  { return m_stack.undo(); }
    bool redo()  { return m_stack.redo(); }
    [[nodiscard]] bool canUndo() const { return m_stack.canUndo(); }
    [[nodiscard]] bool canRedo() const { return m_stack.canRedo(); }
    [[nodiscard]] size_t undoCount() const { return m_stack.undoCount(); }
    [[nodiscard]] size_t redoCount() const { return m_stack.redoCount(); }

private:
    CommandStack& m_stack;
};

// ── World Preview Service ───────────────────────────────────────


} // namespace NF
