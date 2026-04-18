// S107 editor tests: LevelDesignToolkit, RoomEditor, DungeonGenerator
#include <catch2/catch_test_macros.hpp>
#include "NF/Editor/Editor.h"
#include "NF/Editor/DungeonGenerator.h"
#include "NF/Editor/RoomEditor.h"
#include "NF/Editor/LevelDesignToolkit.h"

using namespace NF;

// ── LevelDesignToolkit ───────────────────────────────────────────────────────

TEST_CASE("LevelDesignTool names", "[Editor][S107]") {
    REQUIRE(std::string(levelDesignToolName(LevelDesignTool::Select))    == "Select");
    REQUIRE(std::string(levelDesignToolName(LevelDesignTool::Place))     == "Place");
    REQUIRE(std::string(levelDesignToolName(LevelDesignTool::Erase))     == "Erase");
    REQUIRE(std::string(levelDesignToolName(LevelDesignTool::Paint))     == "Paint");
    REQUIRE(std::string(levelDesignToolName(LevelDesignTool::Fill))      == "Fill");
    REQUIRE(std::string(levelDesignToolName(LevelDesignTool::Transform)) == "Transform");
    REQUIRE(std::string(levelDesignToolName(LevelDesignTool::Snap))      == "Snap");
    REQUIRE(std::string(levelDesignToolName(LevelDesignTool::Align))     == "Align");
}

TEST_CASE("LevelObjectCategory names", "[Editor][S107]") {
    REQUIRE(std::string(levelObjectCategoryName(LevelObjectCategory::Structure))  == "Structure");
    REQUIRE(std::string(levelObjectCategoryName(LevelObjectCategory::Prop))       == "Prop");
    REQUIRE(std::string(levelObjectCategoryName(LevelObjectCategory::Light))      == "Light");
    REQUIRE(std::string(levelObjectCategoryName(LevelObjectCategory::Trigger))    == "Trigger");
    REQUIRE(std::string(levelObjectCategoryName(LevelObjectCategory::Spawn))      == "Spawn");
    REQUIRE(std::string(levelObjectCategoryName(LevelObjectCategory::Pickup))     == "Pickup");
    REQUIRE(std::string(levelObjectCategoryName(LevelObjectCategory::Hazard))     == "Hazard");
    REQUIRE(std::string(levelObjectCategoryName(LevelObjectCategory::Decoration)) == "Decoration");
}

TEST_CASE("LevelValidationResult names", "[Editor][S107]") {
    REQUIRE(std::string(levelValidationResultName(LevelValidationResult::Valid))               == "Valid");
    REQUIRE(std::string(levelValidationResultName(LevelValidationResult::MissingSpawn))        == "MissingSpawn");
    REQUIRE(std::string(levelValidationResultName(LevelValidationResult::OverlappingGeometry)) == "OverlappingGeometry");
    REQUIRE(std::string(levelValidationResultName(LevelValidationResult::InvalidNavMesh))      == "InvalidNavMesh");
    REQUIRE(std::string(levelValidationResultName(LevelValidationResult::UnreachableArea))     == "UnreachableArea");
    REQUIRE(std::string(levelValidationResultName(LevelValidationResult::ExceededBudget))      == "ExceededBudget");
}

TEST_CASE("LevelObject defaults", "[Editor][S107]") {
    LevelObject obj(1, "wall_01", LevelObjectCategory::Structure);
    REQUIRE(obj.id()         == 1u);
    REQUIRE(obj.name()       == "wall_01");
    REQUIRE(obj.category()   == LevelObjectCategory::Structure);
    REQUIRE(!obj.isLocked());
    REQUIRE(obj.isVisible());
    REQUIRE(!obj.isSelected());
    REQUIRE(obj.layer()      == 0u);
}

TEST_CASE("LevelObject mutation", "[Editor][S107]") {
    LevelObject obj(2, "chest_01", LevelObjectCategory::Pickup);
    obj.setLocked(true);
    obj.setVisible(false);
    obj.setSelected(true);
    obj.setLayer(3);
    REQUIRE(obj.isLocked());
    REQUIRE(!obj.isVisible());
    REQUIRE(obj.isSelected());
    REQUIRE(obj.layer() == 3u);
}

TEST_CASE("LevelDesignToolkit defaults", "[Editor][S107]") {
    LevelDesignToolkit tk;
    REQUIRE(tk.activeTool()      == LevelDesignTool::Select);
    REQUIRE(tk.isSnapEnabled());
    REQUIRE(tk.gridSize()        == 1.0f);
    REQUIRE(tk.lastValidation()  == LevelValidationResult::Valid);
    REQUIRE(tk.objectCount()     == 0u);
    REQUIRE(tk.selectedCount()   == 0u);
}

TEST_CASE("LevelDesignToolkit add/remove objects", "[Editor][S107]") {
    LevelDesignToolkit tk;
    REQUIRE(tk.addObject(LevelObject(1, "wall",  LevelObjectCategory::Structure)));
    REQUIRE(tk.addObject(LevelObject(2, "lamp",  LevelObjectCategory::Light)));
    REQUIRE(tk.addObject(LevelObject(3, "spawn", LevelObjectCategory::Spawn)));
    REQUIRE(!tk.addObject(LevelObject(1, "wall", LevelObjectCategory::Structure)));
    REQUIRE(tk.objectCount() == 3u);
    REQUIRE(tk.removeObject(2));
    REQUIRE(tk.objectCount() == 2u);
    REQUIRE(!tk.removeObject(99));
}

TEST_CASE("LevelDesignToolkit counts and find", "[Editor][S107]") {
    LevelDesignToolkit tk;
    LevelObject o1(1, "wall",   LevelObjectCategory::Structure); o1.setSelected(true);
    LevelObject o2(2, "prop",   LevelObjectCategory::Prop);      o2.setSelected(true);
    LevelObject o3(3, "light",  LevelObjectCategory::Light);
    LevelObject o4(4, "spawn",  LevelObjectCategory::Spawn);
    tk.addObject(o1); tk.addObject(o2); tk.addObject(o3); tk.addObject(o4);
    REQUIRE(tk.selectedCount()                                    == 2u);
    REQUIRE(tk.countByCategory(LevelObjectCategory::Structure)    == 1u);
    REQUIRE(tk.countByCategory(LevelObjectCategory::Prop)         == 1u);
    REQUIRE(tk.countByCategory(LevelObjectCategory::Pickup)       == 0u);
    auto* found = tk.findObject(3);
    REQUIRE(found != nullptr);
    REQUIRE(found->category() == LevelObjectCategory::Light);
    REQUIRE(tk.findObject(99) == nullptr);
}

TEST_CASE("LevelDesignToolkit mutation", "[Editor][S107]") {
    LevelDesignToolkit tk;
    tk.setActiveTool(LevelDesignTool::Paint);
    tk.setSnapEnabled(false);
    tk.setGridSize(0.5f);
    tk.setValidationResult(LevelValidationResult::MissingSpawn);
    REQUIRE(tk.activeTool()     == LevelDesignTool::Paint);
    REQUIRE(!tk.isSnapEnabled());
    REQUIRE(tk.gridSize()       == 0.5f);
    REQUIRE(tk.lastValidation() == LevelValidationResult::MissingSpawn);
}

// ── RoomEditor ───────────────────────────────────────────────────────────────

TEST_CASE("RoomShape names", "[Editor][S107]") {
    REQUIRE(std::string(roomShapeName(RoomShape::Rectangular)) == "Rectangular");
    REQUIRE(std::string(roomShapeName(RoomShape::LShaped))     == "LShaped");
    REQUIRE(std::string(roomShapeName(RoomShape::TShaped))     == "TShaped");
    REQUIRE(std::string(roomShapeName(RoomShape::Circular))    == "Circular");
    REQUIRE(std::string(roomShapeName(RoomShape::Irregular))   == "Irregular");
    REQUIRE(std::string(roomShapeName(RoomShape::Hexagonal))   == "Hexagonal");
}

TEST_CASE("DoorType names", "[Editor][S107]") {
    REQUIRE(std::string(doorTypeName(DoorType::Open))        == "Open");
    REQUIRE(std::string(doorTypeName(DoorType::Door))        == "Door");
    REQUIRE(std::string(doorTypeName(DoorType::LockedDoor))  == "LockedDoor");
    REQUIRE(std::string(doorTypeName(DoorType::SecretDoor))  == "SecretDoor");
    REQUIRE(std::string(doorTypeName(DoorType::Archway))     == "Archway");
    REQUIRE(std::string(doorTypeName(DoorType::Portcullis))  == "Portcullis");
}

TEST_CASE("RoomTagType names", "[Editor][S107]") {
    REQUIRE(std::string(roomTagTypeName(RoomTagType::Combat))     == "Combat");
    REQUIRE(std::string(roomTagTypeName(RoomTagType::Puzzle))     == "Puzzle");
    REQUIRE(std::string(roomTagTypeName(RoomTagType::Loot))       == "Loot");
    REQUIRE(std::string(roomTagTypeName(RoomTagType::Boss))       == "Boss");
    REQUIRE(std::string(roomTagTypeName(RoomTagType::Safe))       == "Safe");
    REQUIRE(std::string(roomTagTypeName(RoomTagType::Transition)) == "Transition");
    REQUIRE(std::string(roomTagTypeName(RoomTagType::Hub))        == "Hub");
    REQUIRE(std::string(roomTagTypeName(RoomTagType::Secret))     == "Secret");
}

TEST_CASE("RoomConnection defaults", "[Editor][S107]") {
    RoomConnection conn(42, DoorType::LockedDoor);
    REQUIRE(conn.targetRoomId() == 42u);
    REQUIRE(conn.doorType()     == DoorType::LockedDoor);
    REQUIRE(!conn.isLocked());
    REQUIRE(conn.keyId()        == 0u);
}

TEST_CASE("RoomConnection mutation", "[Editor][S107]") {
    RoomConnection conn(7, DoorType::Door);
    conn.setLocked(true);
    conn.setKeyId(101);
    REQUIRE(conn.isLocked());
    REQUIRE(conn.keyId() == 101u);
}

TEST_CASE("Room defaults", "[Editor][S107]") {
    Room room(1, "Entrance", RoomShape::Rectangular);
    REQUIRE(room.id()              == 1u);
    REQUIRE(room.name()            == "Entrance");
    REQUIRE(room.shape()           == RoomShape::Rectangular);
    REQUIRE(room.width()           == 10.0f);
    REQUIRE(room.height()          == 4.0f);
    REQUIRE(room.tagCount()        == 0u);
    REQUIRE(room.connectionCount() == 0u);
}

TEST_CASE("Room tags and connections", "[Editor][S107]") {
    Room room(2, "Boss Chamber", RoomShape::Circular);
    room.setWidth(20.0f);
    room.setHeight(8.0f);
    room.addTag(RoomTagType::Boss);
    room.addTag(RoomTagType::Combat);
    REQUIRE(room.tagCount()       == 2u);
    REQUIRE(room.hasTag(RoomTagType::Boss));
    REQUIRE(room.hasTag(RoomTagType::Combat));
    REQUIRE(!room.hasTag(RoomTagType::Safe));
    room.removeTag(RoomTagType::Combat);
    REQUIRE(room.tagCount()       == 1u);
    room.addConnection(RoomConnection(1, DoorType::Door));
    room.addConnection(RoomConnection(3, DoorType::LockedDoor));
    REQUIRE(room.connectionCount()== 2u);
}

TEST_CASE("RoomEditor defaults", "[Editor][S107]") {
    RoomEditor ed;
    REQUIRE(ed.roomCount()         == 0u);
    REQUIRE(ed.activeRoomId()      == 0u);
    REQUIRE(ed.isShowGrid());
    REQUIRE(ed.isShowConnections());
}

TEST_CASE("RoomEditor add/remove rooms", "[Editor][S107]") {
    RoomEditor ed;
    REQUIRE(ed.addRoom(Room(1, "Hall",   RoomShape::Rectangular)));
    REQUIRE(ed.addRoom(Room(2, "Boss",   RoomShape::Circular)));
    REQUIRE(ed.addRoom(Room(3, "Secret", RoomShape::LShaped)));
    REQUIRE(!ed.addRoom(Room(1, "Hall",  RoomShape::Rectangular)));
    REQUIRE(ed.roomCount() == 3u);
    REQUIRE(ed.removeRoom(2));
    REQUIRE(ed.roomCount() == 2u);
    REQUIRE(!ed.removeRoom(99));
}

TEST_CASE("RoomEditor counts and find", "[Editor][S107]") {
    RoomEditor ed;
    Room r1(1, "Entry",  RoomShape::Rectangular);
    Room r2(2, "Hall",   RoomShape::Rectangular);
    Room r3(3, "Boss",   RoomShape::Circular);    r3.addTag(RoomTagType::Boss);
    Room r4(4, "Secret", RoomShape::LShaped);     r4.addTag(RoomTagType::Secret);
    ed.addRoom(r1); ed.addRoom(r2); ed.addRoom(r3); ed.addRoom(r4);
    REQUIRE(ed.countByShape(RoomShape::Rectangular) == 2u);
    REQUIRE(ed.countByShape(RoomShape::Circular)    == 1u);
    REQUIRE(ed.countWithTag(RoomTagType::Boss)      == 1u);
    REQUIRE(ed.countWithTag(RoomTagType::Secret)    == 1u);
    REQUIRE(ed.countWithTag(RoomTagType::Hub)       == 0u);
    auto* found = ed.findRoom(3);
    REQUIRE(found != nullptr);
    REQUIRE(found->shape() == RoomShape::Circular);
    REQUIRE(ed.findRoom(99) == nullptr);
}

TEST_CASE("RoomEditor mutation", "[Editor][S107]") {
    RoomEditor ed;
    ed.setActiveRoomId(5);
    ed.setShowGrid(false);
    ed.setShowConnections(false);
    REQUIRE(ed.activeRoomId()     == 5u);
    REQUIRE(!ed.isShowGrid());
    REQUIRE(!ed.isShowConnections());
}

// ── DungeonGenerator ─────────────────────────────────────────────────────────

TEST_CASE("DungeonGenerationAlgorithm names", "[Editor][S107]") {
    REQUIRE(std::string(dungeonGenerationAlgorithmName(DungeonGenerationAlgorithm::BSP))                  == "BSP");
    REQUIRE(std::string(dungeonGenerationAlgorithmName(DungeonGenerationAlgorithm::CellularAutomata))     == "CellularAutomata");
    REQUIRE(std::string(dungeonGenerationAlgorithmName(DungeonGenerationAlgorithm::RandomWalk))           == "RandomWalk");
    REQUIRE(std::string(dungeonGenerationAlgorithmName(DungeonGenerationAlgorithm::Prefab))               == "Prefab");
    REQUIRE(std::string(dungeonGenerationAlgorithmName(DungeonGenerationAlgorithm::Voronoi))              == "Voronoi");
    REQUIRE(std::string(dungeonGenerationAlgorithmName(DungeonGenerationAlgorithm::WaveFunctionCollapse)) == "WaveFunctionCollapse");
}

TEST_CASE("DungeonTheme names", "[Editor][S107]") {
    REQUIRE(std::string(dungeonThemeName(DungeonTheme::Cave))    == "Cave");
    REQUIRE(std::string(dungeonThemeName(DungeonTheme::Castle))  == "Castle");
    REQUIRE(std::string(dungeonThemeName(DungeonTheme::Ruins))   == "Ruins");
    REQUIRE(std::string(dungeonThemeName(DungeonTheme::Sewer))   == "Sewer");
    REQUIRE(std::string(dungeonThemeName(DungeonTheme::Forest))  == "Forest");
    REQUIRE(std::string(dungeonThemeName(DungeonTheme::Temple))  == "Temple");
    REQUIRE(std::string(dungeonThemeName(DungeonTheme::Tech))    == "Tech");
    REQUIRE(std::string(dungeonThemeName(DungeonTheme::Cosmic))  == "Cosmic");
}

TEST_CASE("DungeonGenerationState names", "[Editor][S107]") {
    REQUIRE(std::string(dungeonGenerationStateName(DungeonGenerationState::Idle))       == "Idle");
    REQUIRE(std::string(dungeonGenerationStateName(DungeonGenerationState::Generating)) == "Generating");
    REQUIRE(std::string(dungeonGenerationStateName(DungeonGenerationState::Validating)) == "Validating");
    REQUIRE(std::string(dungeonGenerationStateName(DungeonGenerationState::Complete))   == "Complete");
    REQUIRE(std::string(dungeonGenerationStateName(DungeonGenerationState::Failed))     == "Failed");
}

TEST_CASE("DungeonGeneratorParams defaults", "[Editor][S107]") {
    DungeonGeneratorParams params;
    REQUIRE(params.seed()          == 0u);
    REQUIRE(params.width()         == 64u);
    REQUIRE(params.height()        == 64u);
    REQUIRE(params.minRooms()      == 5u);
    REQUIRE(params.maxRooms()      == 20u);
    REQUIRE(params.corridorWidth() == 2u);
    REQUIRE(params.density()       == 0.5f);
}

TEST_CASE("DungeonGeneratorParams mutation", "[Editor][S107]") {
    DungeonGeneratorParams params;
    params.setSeed(12345);
    params.setWidth(128);
    params.setHeight(128);
    params.setMinRooms(10);
    params.setMaxRooms(40);
    params.setCorridorWidth(3);
    params.setDensity(0.7f);
    REQUIRE(params.seed()          == 12345u);
    REQUIRE(params.width()         == 128u);
    REQUIRE(params.height()        == 128u);
    REQUIRE(params.minRooms()      == 10u);
    REQUIRE(params.maxRooms()      == 40u);
    REQUIRE(params.corridorWidth() == 3u);
    REQUIRE(params.density()       == 0.7f);
}

TEST_CASE("DungeonGenerator defaults", "[Editor][S107]") {
    DungeonGenerator gen;
    REQUIRE(gen.algorithm()          == DungeonGenerationAlgorithm::BSP);
    REQUIRE(gen.theme()              == DungeonTheme::Cave);
    REQUIRE(gen.state()              == DungeonGenerationState::Idle);
    REQUIRE(gen.generatedRoomCount() == 0u);
    REQUIRE(gen.isEnsureConnected());
    REQUIRE(gen.isPlaceBossRoom());
    REQUIRE(!gen.isComplete());
    REQUIRE(!gen.isFailed());
}

TEST_CASE("DungeonGenerator mutation", "[Editor][S107]") {
    DungeonGenerator gen;
    gen.setAlgorithm(DungeonGenerationAlgorithm::Voronoi);
    gen.setTheme(DungeonTheme::Temple);
    gen.setState(DungeonGenerationState::Complete);
    gen.setGeneratedRoomCount(15);
    gen.setEnsureConnected(false);
    gen.setPlaceBossRoom(false);
    REQUIRE(gen.algorithm()          == DungeonGenerationAlgorithm::Voronoi);
    REQUIRE(gen.theme()              == DungeonTheme::Temple);
    REQUIRE(gen.state()              == DungeonGenerationState::Complete);
    REQUIRE(gen.generatedRoomCount() == 15u);
    REQUIRE(!gen.isEnsureConnected());
    REQUIRE(!gen.isPlaceBossRoom());
    REQUIRE(gen.isComplete());
    REQUIRE(!gen.isFailed());
}

TEST_CASE("DungeonGenerator failed state", "[Editor][S107]") {
    DungeonGenerator gen;
    gen.setState(DungeonGenerationState::Failed);
    REQUIRE(gen.isFailed());
    REQUIRE(!gen.isComplete());
}

TEST_CASE("DungeonGenerator params roundtrip", "[Editor][S107]") {
    DungeonGenerator gen;
    DungeonGeneratorParams params;
    params.setSeed(99999);
    params.setWidth(256);
    params.setMaxRooms(50);
    gen.setParams(params);
    REQUIRE(gen.params().seed()     == 99999u);
    REQUIRE(gen.params().width()    == 256u);
    REQUIRE(gen.params().maxRooms() == 50u);
}
