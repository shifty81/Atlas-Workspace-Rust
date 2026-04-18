#include <catch2/catch_test_macros.hpp>
#include "NF/Editor/Editor.h"
#include "NF/Editor/EventGraph.h"
#include "NF/Editor/CurveLibrary.h"
#include "NF/Editor/DataTableEditor.h"

using namespace NF;

// ── S94: DataTableEditor + CurveLibrary + EventGraph ─────────────

// ── DataTableEditor ──────────────────────────────────────────────

TEST_CASE("DataTableColumnType names are correct", "[Editor][S94]") {
    REQUIRE(std::string(dataTableColumnTypeName(DataTableColumnType::Bool))   == "Bool");
    REQUIRE(std::string(dataTableColumnTypeName(DataTableColumnType::Int))    == "Int");
    REQUIRE(std::string(dataTableColumnTypeName(DataTableColumnType::Float))  == "Float");
    REQUIRE(std::string(dataTableColumnTypeName(DataTableColumnType::String)) == "String");
    REQUIRE(std::string(dataTableColumnTypeName(DataTableColumnType::Enum))   == "Enum");
    REQUIRE(std::string(dataTableColumnTypeName(DataTableColumnType::Asset))  == "Asset");
    REQUIRE(std::string(dataTableColumnTypeName(DataTableColumnType::Curve))  == "Curve");
}

TEST_CASE("DataTableImportFormat names are correct", "[Editor][S94]") {
    REQUIRE(std::string(dataTableImportFormatName(DataTableImportFormat::CSV))          == "CSV");
    REQUIRE(std::string(dataTableImportFormatName(DataTableImportFormat::JSON))         == "JSON");
    REQUIRE(std::string(dataTableImportFormatName(DataTableImportFormat::XML))          == "XML");
    REQUIRE(std::string(dataTableImportFormatName(DataTableImportFormat::Binary))       == "Binary");
    REQUIRE(std::string(dataTableImportFormatName(DataTableImportFormat::GoogleSheets)) == "GoogleSheets");
}

TEST_CASE("DataTableEditState names are correct", "[Editor][S94]") {
    REQUIRE(std::string(dataTableEditStateName(DataTableEditState::Clean))     == "Clean");
    REQUIRE(std::string(dataTableEditStateName(DataTableEditState::Modified))  == "Modified");
    REQUIRE(std::string(dataTableEditStateName(DataTableEditState::Importing)) == "Importing");
    REQUIRE(std::string(dataTableEditStateName(DataTableEditState::Exporting)) == "Exporting");
    REQUIRE(std::string(dataTableEditStateName(DataTableEditState::Error))     == "Error");
}

TEST_CASE("DataTableColumn stores properties", "[Editor][S94]") {
    DataTableColumn col("Damage", DataTableColumnType::Float);
    col.setRequired(true);
    col.setDefaultValue("0.0");
    col.setWidth(120);
    REQUIRE(col.name()         == "Damage");
    REQUIRE(col.type()         == DataTableColumnType::Float);
    REQUIRE(col.isRequired());
    REQUIRE(col.defaultValue() == "0.0");
    REQUIRE(col.width()        == 120);
    REQUIRE(col.isVisible());
}

TEST_CASE("DataTableEditor add remove findColumn", "[Editor][S94]") {
    DataTableEditor editor;
    DataTableColumn c1("Name",   DataTableColumnType::String);
    DataTableColumn c2("Health", DataTableColumnType::Int);
    REQUIRE(editor.addColumn(c1));
    REQUIRE(editor.addColumn(c2));
    REQUIRE(editor.columnCount() == 2);
    REQUIRE(editor.findColumn("Health") != nullptr);
    editor.removeColumn("Health");
    REQUIRE(editor.columnCount() == 1);
}

TEST_CASE("DataTableEditor rejects duplicate column name", "[Editor][S94]") {
    DataTableEditor editor;
    DataTableColumn c("Speed", DataTableColumnType::Float);
    editor.addColumn(c);
    REQUIRE_FALSE(editor.addColumn(c));
}

TEST_CASE("DataTableEditor state and rowCount", "[Editor][S94]") {
    DataTableEditor editor;
    editor.setTableName("Weapons");
    editor.setRowCount(100);
    editor.setState(DataTableEditState::Modified);
    editor.setImportFormat(DataTableImportFormat::JSON);
    REQUIRE(editor.tableName()    == "Weapons");
    REQUIRE(editor.rowCount()     == 100);
    REQUIRE(editor.isModified());
    REQUIRE(editor.importFormat() == DataTableImportFormat::JSON);
}

TEST_CASE("DataTableEditor visibleColumnCount and requiredColumnCount", "[Editor][S94]") {
    DataTableEditor editor;
    DataTableColumn c1("A", DataTableColumnType::Int);    c1.setRequired(true);
    DataTableColumn c2("B", DataTableColumnType::Float);  c2.setVisible(false);
    DataTableColumn c3("C", DataTableColumnType::String); c3.setRequired(true);
    editor.addColumn(c1); editor.addColumn(c2); editor.addColumn(c3);
    REQUIRE(editor.visibleColumnCount()  == 2);
    REQUIRE(editor.requiredColumnCount() == 2);
}

TEST_CASE("DataTableEditor MAX_COLUMNS and MAX_ROWS", "[Editor][S94]") {
    REQUIRE(DataTableEditor::MAX_COLUMNS == 64);
    REQUIRE(DataTableEditor::MAX_ROWS    == 65536);
}

// ── CurveLibrary ─────────────────────────────────────────────────

TEST_CASE("CurveLibraryCategory names are correct", "[Editor][S94]") {
    REQUIRE(std::string(curveLibraryCategoryName(CurveLibraryCategory::Animation)) == "Animation");
    REQUIRE(std::string(curveLibraryCategoryName(CurveLibraryCategory::Material))  == "Material");
    REQUIRE(std::string(curveLibraryCategoryName(CurveLibraryCategory::Gameplay))  == "Gameplay");
    REQUIRE(std::string(curveLibraryCategoryName(CurveLibraryCategory::Audio))     == "Audio");
    REQUIRE(std::string(curveLibraryCategoryName(CurveLibraryCategory::UI))        == "UI");
    REQUIRE(std::string(curveLibraryCategoryName(CurveLibraryCategory::Custom))    == "Custom");
}

TEST_CASE("CurveLibraryInterp names are correct", "[Editor][S94]") {
    REQUIRE(std::string(curveLibraryInterpName(CurveLibraryInterp::Linear))    == "Linear");
    REQUIRE(std::string(curveLibraryInterpName(CurveLibraryInterp::Cubic))     == "Cubic");
    REQUIRE(std::string(curveLibraryInterpName(CurveLibraryInterp::Step))      == "Step");
    REQUIRE(std::string(curveLibraryInterpName(CurveLibraryInterp::EaseIn))    == "EaseIn");
    REQUIRE(std::string(curveLibraryInterpName(CurveLibraryInterp::EaseOut))   == "EaseOut");
    REQUIRE(std::string(curveLibraryInterpName(CurveLibraryInterp::EaseInOut)) == "EaseInOut");
}

TEST_CASE("CurveLibraryScope names are correct", "[Editor][S94]") {
    REQUIRE(std::string(curveLibraryScopeName(CurveLibraryScope::Project)) == "Project");
    REQUIRE(std::string(curveLibraryScopeName(CurveLibraryScope::Global))  == "Global");
    REQUIRE(std::string(curveLibraryScopeName(CurveLibraryScope::Package)) == "Package");
    REQUIRE(std::string(curveLibraryScopeName(CurveLibraryScope::Local))   == "Local");
}

TEST_CASE("CurveAssetEntry stores properties", "[Editor][S94]") {
    CurveAssetEntry curve("EaseInQuad", CurveLibraryCategory::UI, CurveLibraryInterp::EaseIn);
    curve.setScope(CurveLibraryScope::Global);
    curve.setKeyCount(4);
    curve.setLooped(false);
    curve.setReadOnly(true);
    REQUIRE(curve.name()      == "EaseInQuad");
    REQUIRE(curve.category()  == CurveLibraryCategory::UI);
    REQUIRE(curve.interp()    == CurveLibraryInterp::EaseIn);
    REQUIRE(curve.scope()     == CurveLibraryScope::Global);
    REQUIRE(curve.keyCount()  == 4);
    REQUIRE(curve.isReadOnly());
    REQUIRE_FALSE(curve.isLooped());
}

TEST_CASE("CurveLibrary add selectCurve remove", "[Editor][S94]") {
    CurveLibrary lib;
    CurveAssetEntry c1("A", CurveLibraryCategory::Animation, CurveLibraryInterp::Cubic);
    CurveAssetEntry c2("B", CurveLibraryCategory::Audio,     CurveLibraryInterp::Linear);
    REQUIRE(lib.addCurve(c1));
    REQUIRE(lib.addCurve(c2));
    REQUIRE(lib.curveCount() == 2);
    REQUIRE(lib.selectCurve("A"));
    REQUIRE(lib.selectedCurve() == "A");
    lib.removeCurve("A");
    REQUIRE(lib.selectedCurve().empty());
}

TEST_CASE("CurveLibrary rejects duplicate name", "[Editor][S94]") {
    CurveLibrary lib;
    CurveAssetEntry c("Bounce", CurveLibraryCategory::Gameplay, CurveLibraryInterp::Cubic);
    lib.addCurve(c);
    REQUIRE_FALSE(lib.addCurve(c));
}

TEST_CASE("CurveLibrary countByCategory, countByInterp, loopedCount, readOnlyCount", "[Editor][S94]") {
    CurveLibrary lib;
    CurveAssetEntry c1("A", CurveLibraryCategory::Animation, CurveLibraryInterp::Cubic);
    c1.setLooped(true); c1.setReadOnly(true);
    CurveAssetEntry c2("B", CurveLibraryCategory::Animation, CurveLibraryInterp::Linear);
    c2.setLooped(false); c2.setReadOnly(false);
    CurveAssetEntry c3("C", CurveLibraryCategory::Audio,     CurveLibraryInterp::Cubic);
    c3.setLooped(true);  c3.setReadOnly(false);
    lib.addCurve(c1); lib.addCurve(c2); lib.addCurve(c3);
    REQUIRE(lib.countByCategory(CurveLibraryCategory::Animation) == 2);
    REQUIRE(lib.countByInterp(CurveLibraryInterp::Cubic)         == 2);
    REQUIRE(lib.loopedCount()   == 2);
    REQUIRE(lib.readOnlyCount() == 1);
}

TEST_CASE("CurveLibrary MAX_CURVES is 1024", "[Editor][S94]") {
    REQUIRE(CurveLibrary::MAX_CURVES == 1024);
}

// ── EventGraph ───────────────────────────────────────────────────

TEST_CASE("EventGraphNodeKind names are correct", "[Editor][S94]") {
    REQUIRE(std::string(eventGraphNodeKindName(EventGraphNodeKind::Event))    == "Event");
    REQUIRE(std::string(eventGraphNodeKindName(EventGraphNodeKind::Action))   == "Action");
    REQUIRE(std::string(eventGraphNodeKindName(EventGraphNodeKind::Branch))   == "Branch");
    REQUIRE(std::string(eventGraphNodeKindName(EventGraphNodeKind::Loop))     == "Loop");
    REQUIRE(std::string(eventGraphNodeKindName(EventGraphNodeKind::Variable)) == "Variable");
    REQUIRE(std::string(eventGraphNodeKindName(EventGraphNodeKind::Function)) == "Function");
    REQUIRE(std::string(eventGraphNodeKindName(EventGraphNodeKind::Comment))  == "Comment");
    REQUIRE(std::string(eventGraphNodeKindName(EventGraphNodeKind::Macro))    == "Macro");
}

TEST_CASE("EventGraphPinType names are correct", "[Editor][S94]") {
    REQUIRE(std::string(eventGraphPinTypeName(EventGraphPinType::Exec))   == "Exec");
    REQUIRE(std::string(eventGraphPinTypeName(EventGraphPinType::Bool))   == "Bool");
    REQUIRE(std::string(eventGraphPinTypeName(EventGraphPinType::Int))    == "Int");
    REQUIRE(std::string(eventGraphPinTypeName(EventGraphPinType::Float))  == "Float");
    REQUIRE(std::string(eventGraphPinTypeName(EventGraphPinType::String)) == "String");
    REQUIRE(std::string(eventGraphPinTypeName(EventGraphPinType::Object)) == "Object");
    REQUIRE(std::string(eventGraphPinTypeName(EventGraphPinType::Struct)) == "Struct");
    REQUIRE(std::string(eventGraphPinTypeName(EventGraphPinType::Array))  == "Array");
}

TEST_CASE("EventGraphCompileState names are correct", "[Editor][S94]") {
    REQUIRE(std::string(eventGraphCompileStateName(EventGraphCompileState::Unknown))   == "Unknown");
    REQUIRE(std::string(eventGraphCompileStateName(EventGraphCompileState::Clean))     == "Clean");
    REQUIRE(std::string(eventGraphCompileStateName(EventGraphCompileState::Warning))   == "Warning");
    REQUIRE(std::string(eventGraphCompileStateName(EventGraphCompileState::Error))     == "Error");
    REQUIRE(std::string(eventGraphCompileStateName(EventGraphCompileState::Compiling)) == "Compiling");
}

TEST_CASE("EventGraphNode stores properties", "[Editor][S94]") {
    EventGraphNode node("OnBeginPlay", EventGraphNodeKind::Event);
    node.setComment("Entry point");
    node.setBreakpoint(true);
    node.setPinCount(2);
    REQUIRE(node.name()          == "OnBeginPlay");
    REQUIRE(node.kind()          == EventGraphNodeKind::Event);
    REQUIRE(node.comment()       == "Entry point");
    REQUIRE(node.hasBreakpoint());
    REQUIRE(node.pinCount()      == 2);
    REQUIRE(node.isEnabled());
}

TEST_CASE("EventGraph add selectNode remove", "[Editor][S94]") {
    EventGraph graph;
    EventGraphNode n1("OnBeginPlay", EventGraphNodeKind::Event);
    EventGraphNode n2("PrintString", EventGraphNodeKind::Action);
    REQUIRE(graph.addNode(n1));
    REQUIRE(graph.addNode(n2));
    REQUIRE(graph.nodeCount() == 2);
    REQUIRE(graph.selectNode("OnBeginPlay"));
    REQUIRE(graph.selectedNode() == "OnBeginPlay");
    graph.removeNode("OnBeginPlay");
    REQUIRE(graph.selectedNode().empty());
}

TEST_CASE("EventGraph rejects duplicate node name", "[Editor][S94]") {
    EventGraph graph;
    EventGraphNode n("Move", EventGraphNodeKind::Action);
    graph.addNode(n);
    REQUIRE_FALSE(graph.addNode(n));
}

TEST_CASE("EventGraph compile state and isClean", "[Editor][S94]") {
    EventGraph graph;
    graph.setGraphName("Main");
    graph.setCompileState(EventGraphCompileState::Clean);
    REQUIRE(graph.graphName()     == "Main");
    REQUIRE(graph.isClean());
}

TEST_CASE("EventGraph countByKind, breakpointCount, disabledCount", "[Editor][S94]") {
    EventGraph graph;
    EventGraphNode n1("A", EventGraphNodeKind::Event);  n1.setBreakpoint(true);
    EventGraphNode n2("B", EventGraphNodeKind::Action); n2.setEnabled(false);
    EventGraphNode n3("C", EventGraphNodeKind::Action); n3.setBreakpoint(true);
    graph.addNode(n1); graph.addNode(n2); graph.addNode(n3);
    REQUIRE(graph.countByKind(EventGraphNodeKind::Action) == 2);
    REQUIRE(graph.breakpointCount() == 2);
    REQUIRE(graph.disabledCount()   == 1);
}

TEST_CASE("EventGraph MAX_NODES is 512", "[Editor][S94]") {
    REQUIRE(EventGraph::MAX_NODES == 512);
}
