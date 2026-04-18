// S128 editor tests: CloudStorageEditor, SaveDataEditor, SyncConflictEditor
#include <catch2/catch_test_macros.hpp>
#include "NF/Editor/Editor.h"
#include "NF/Editor/SyncConflictEditor.h"
#include "NF/Editor/SaveDataEditor.h"
#include "NF/Editor/CloudStorageEditor.h"

using namespace NF;

// ── CloudStorageEditor ────────────────────────────────────────────────────────

TEST_CASE("StorageTier names", "[Editor][S128]") {
    REQUIRE(std::string(storageTierName(StorageTier::Hot))     == "Hot");
    REQUIRE(std::string(storageTierName(StorageTier::Warm))    == "Warm");
    REQUIRE(std::string(storageTierName(StorageTier::Cold))    == "Cold");
    REQUIRE(std::string(storageTierName(StorageTier::Archive)) == "Archive");
    REQUIRE(std::string(storageTierName(StorageTier::Custom))  == "Custom");
}

TEST_CASE("StorageAccess names", "[Editor][S128]") {
    REQUIRE(std::string(storageAccessName(StorageAccess::Public))        == "Public");
    REQUIRE(std::string(storageAccessName(StorageAccess::Private))       == "Private");
    REQUIRE(std::string(storageAccessName(StorageAccess::Authenticated)) == "Authenticated");
    REQUIRE(std::string(storageAccessName(StorageAccess::Restricted))    == "Restricted");
}

TEST_CASE("StorageBucket defaults", "[Editor][S128]") {
    StorageBucket b(1, "assets_bucket", StorageTier::Hot, StorageAccess::Public);
    REQUIRE(b.id()          == 1u);
    REQUIRE(b.name()        == "assets_bucket");
    REQUIRE(b.tier()        == StorageTier::Hot);
    REQUIRE(b.access()      == StorageAccess::Public);
    REQUIRE(b.capacityMB()  == 1024u);
    REQUIRE(!b.isVersioned());
    REQUIRE(b.isEnabled());
}

TEST_CASE("StorageBucket mutation", "[Editor][S128]") {
    StorageBucket b(2, "archive_bucket", StorageTier::Archive, StorageAccess::Private);
    b.setCapacityMB(2048u);
    b.setIsVersioned(true);
    b.setIsEnabled(false);
    REQUIRE(b.capacityMB()  == 2048u);
    REQUIRE(b.isVersioned());
    REQUIRE(!b.isEnabled());
}

TEST_CASE("CloudStorageEditor defaults", "[Editor][S128]") {
    CloudStorageEditor ed;
    REQUIRE(!ed.isShowDisabled());
    REQUIRE(!ed.isGroupByTier());
    REQUIRE(ed.defaultCapacityMB() == 512u);
    REQUIRE(ed.bucketCount()       == 0u);
}

TEST_CASE("CloudStorageEditor add/remove buckets", "[Editor][S128]") {
    CloudStorageEditor ed;
    REQUIRE(ed.addBucket(StorageBucket(1, "b_a", StorageTier::Hot,  StorageAccess::Public)));
    REQUIRE(ed.addBucket(StorageBucket(2, "b_b", StorageTier::Warm, StorageAccess::Private)));
    REQUIRE(ed.addBucket(StorageBucket(3, "b_c", StorageTier::Cold, StorageAccess::Restricted)));
    REQUIRE(!ed.addBucket(StorageBucket(1, "b_a", StorageTier::Hot, StorageAccess::Public)));
    REQUIRE(ed.bucketCount() == 3u);
    REQUIRE(ed.removeBucket(2));
    REQUIRE(ed.bucketCount() == 2u);
    REQUIRE(!ed.removeBucket(99));
}

TEST_CASE("CloudStorageEditor counts and find", "[Editor][S128]") {
    CloudStorageEditor ed;
    StorageBucket b1(1, "b_a", StorageTier::Hot,  StorageAccess::Public);
    StorageBucket b2(2, "b_b", StorageTier::Hot,  StorageAccess::Private);
    StorageBucket b3(3, "b_c", StorageTier::Warm, StorageAccess::Authenticated);
    StorageBucket b4(4, "b_d", StorageTier::Cold, StorageAccess::Public); b4.setIsEnabled(false);
    ed.addBucket(b1); ed.addBucket(b2); ed.addBucket(b3); ed.addBucket(b4);
    REQUIRE(ed.countByTier(StorageTier::Hot)               == 2u);
    REQUIRE(ed.countByTier(StorageTier::Warm)              == 1u);
    REQUIRE(ed.countByTier(StorageTier::Archive)           == 0u);
    REQUIRE(ed.countByAccess(StorageAccess::Public)        == 2u);
    REQUIRE(ed.countByAccess(StorageAccess::Private)       == 1u);
    REQUIRE(ed.countEnabled()                              == 3u);
    auto* found = ed.findBucket(3);
    REQUIRE(found != nullptr);
    REQUIRE(found->tier() == StorageTier::Warm);
    REQUIRE(ed.findBucket(99) == nullptr);
}

TEST_CASE("CloudStorageEditor settings mutation", "[Editor][S128]") {
    CloudStorageEditor ed;
    ed.setIsShowDisabled(true);
    ed.setIsGroupByTier(true);
    ed.setDefaultCapacityMB(2048u);
    REQUIRE(ed.isShowDisabled());
    REQUIRE(ed.isGroupByTier());
    REQUIRE(ed.defaultCapacityMB() == 2048u);
}

// ── SaveDataEditor ────────────────────────────────────────────────────────────

TEST_CASE("SaveDataSlot names", "[Editor][S128]") {
    REQUIRE(std::string(saveDataSlotName(SaveDataSlot::Auto))       == "Auto");
    REQUIRE(std::string(saveDataSlotName(SaveDataSlot::Manual))     == "Manual");
    REQUIRE(std::string(saveDataSlotName(SaveDataSlot::Checkpoint)) == "Checkpoint");
    REQUIRE(std::string(saveDataSlotName(SaveDataSlot::Cloud))      == "Cloud");
    REQUIRE(std::string(saveDataSlotName(SaveDataSlot::Local))      == "Local");
}

TEST_CASE("SaveDataState names", "[Editor][S128]") {
    REQUIRE(std::string(saveDataStateName(SaveDataState::Empty))     == "Empty");
    REQUIRE(std::string(saveDataStateName(SaveDataState::Valid))     == "Valid");
    REQUIRE(std::string(saveDataStateName(SaveDataState::Corrupt))   == "Corrupt");
    REQUIRE(std::string(saveDataStateName(SaveDataState::Migrating)) == "Migrating");
    REQUIRE(std::string(saveDataStateName(SaveDataState::Locked))    == "Locked");
}

TEST_CASE("SaveDataEntry defaults", "[Editor][S128]") {
    SaveDataEntry e(1, "slot_auto", SaveDataSlot::Auto);
    REQUIRE(e.id()           == 1u);
    REQUIRE(e.name()         == "slot_auto");
    REQUIRE(e.slot()         == SaveDataSlot::Auto);
    REQUIRE(e.state()        == SaveDataState::Empty);
    REQUIRE(e.sizeKB()         == 0u);
    REQUIRE(e.isCompressed());
    REQUIRE(e.isEnabled());
}

TEST_CASE("SaveDataEntry mutation", "[Editor][S128]") {
    SaveDataEntry e(2, "slot_manual", SaveDataSlot::Manual);
    e.setState(SaveDataState::Valid);
    e.setSizeKB(512u);
    e.setIsCompressed(false);
    e.setIsEnabled(false);
    REQUIRE(e.state()        == SaveDataState::Valid);
    REQUIRE(e.sizeKB()         == 512u);
    REQUIRE(!e.isCompressed());
    REQUIRE(!e.isEnabled());
}

TEST_CASE("SaveDataEditor defaults", "[Editor][S128]") {
    SaveDataEditor ed;
    REQUIRE(ed.isShowEmpty());
    REQUIRE(!ed.isGroupBySlot());
    REQUIRE(ed.maxSlotsPerUser() == 10u);
    REQUIRE(ed.saveDataCount()   == 0u);
}

TEST_CASE("SaveDataEditor add/remove entries", "[Editor][S128]") {
    SaveDataEditor ed;
    REQUIRE(ed.addSaveData(SaveDataEntry(1, "s_a", SaveDataSlot::Auto)));
    REQUIRE(ed.addSaveData(SaveDataEntry(2, "s_b", SaveDataSlot::Manual)));
    REQUIRE(ed.addSaveData(SaveDataEntry(3, "s_c", SaveDataSlot::Cloud)));
    REQUIRE(!ed.addSaveData(SaveDataEntry(1, "s_a", SaveDataSlot::Auto)));
    REQUIRE(ed.saveDataCount() == 3u);
    REQUIRE(ed.removeSaveData(2));
    REQUIRE(ed.saveDataCount() == 2u);
    REQUIRE(!ed.removeSaveData(99));
}

TEST_CASE("SaveDataEditor counts and find", "[Editor][S128]") {
    SaveDataEditor ed;
    SaveDataEntry e1(1, "s_a", SaveDataSlot::Auto);
    SaveDataEntry e2(2, "s_b", SaveDataSlot::Auto);    e2.setState(SaveDataState::Valid);
    SaveDataEntry e3(3, "s_c", SaveDataSlot::Manual);  e3.setState(SaveDataState::Corrupt);
    SaveDataEntry e4(4, "s_d", SaveDataSlot::Cloud);   e4.setState(SaveDataState::Locked); e4.setIsEnabled(false);
    ed.addSaveData(e1); ed.addSaveData(e2); ed.addSaveData(e3); ed.addSaveData(e4);
    REQUIRE(ed.countBySlot(SaveDataSlot::Auto)         == 2u);
    REQUIRE(ed.countBySlot(SaveDataSlot::Manual)       == 1u);
    REQUIRE(ed.countBySlot(SaveDataSlot::Checkpoint)   == 0u);
    REQUIRE(ed.countByState(SaveDataState::Empty)      == 1u);
    REQUIRE(ed.countByState(SaveDataState::Valid)      == 1u);
    REQUIRE(ed.countEnabled()                          == 3u);
    auto* found = ed.findSaveData(3);
    REQUIRE(found != nullptr);
    REQUIRE(found->slot() == SaveDataSlot::Manual);
    REQUIRE(ed.findSaveData(99) == nullptr);
}

TEST_CASE("SaveDataEditor settings mutation", "[Editor][S128]") {
    SaveDataEditor ed;
    ed.setIsShowEmpty(false);
    ed.setIsGroupBySlot(true);
    ed.setMaxSlotsPerUser(20u);
    REQUIRE(!ed.isShowEmpty());
    REQUIRE(ed.isGroupBySlot());
    REQUIRE(ed.maxSlotsPerUser() == 20u);
}

// ── SyncConflictEditor ────────────────────────────────────────────────────────

TEST_CASE("ConflictResolution names", "[Editor][S128]") {
    REQUIRE(std::string(conflictResolutionName(ConflictResolution::LocalWins))  == "LocalWins");
    REQUIRE(std::string(conflictResolutionName(ConflictResolution::RemoteWins)) == "RemoteWins");
    REQUIRE(std::string(conflictResolutionName(ConflictResolution::Merge))      == "Merge");
    REQUIRE(std::string(conflictResolutionName(ConflictResolution::Manual))     == "Manual");
    REQUIRE(std::string(conflictResolutionName(ConflictResolution::LastWrite))  == "LastWrite");
}

TEST_CASE("ConflictSeverity names", "[Editor][S128]") {
    REQUIRE(std::string(conflictSeverityName(ConflictSeverity::Minor))    == "Minor");
    REQUIRE(std::string(conflictSeverityName(ConflictSeverity::Moderate)) == "Moderate");
    REQUIRE(std::string(conflictSeverityName(ConflictSeverity::Major))    == "Major");
    REQUIRE(std::string(conflictSeverityName(ConflictSeverity::Critical)) == "Critical");
}

TEST_CASE("SyncConflict defaults", "[Editor][S128]") {
    SyncConflict c(1, "save_conflict", ConflictResolution::Merge, ConflictSeverity::Minor);
    REQUIRE(c.id()             == 1u);
    REQUIRE(c.name()           == "save_conflict");
    REQUIRE(c.resolution()     == ConflictResolution::Merge);
    REQUIRE(c.severity()       == ConflictSeverity::Minor);
    REQUIRE(!c.isResolved());
    REQUIRE(c.conflictTimeMs() == 0u);
    REQUIRE(c.isEnabled());
}

TEST_CASE("SyncConflict mutation", "[Editor][S128]") {
    SyncConflict c(2, "critical_conflict", ConflictResolution::Manual, ConflictSeverity::Critical);
    c.setIsResolved(true);
    c.setConflictTimeMs(500u);
    c.setIsEnabled(false);
    REQUIRE(c.isResolved());
    REQUIRE(c.conflictTimeMs() == 500u);
    REQUIRE(!c.isEnabled());
}

TEST_CASE("SyncConflictEditor defaults", "[Editor][S128]") {
    SyncConflictEditor ed;
    REQUIRE(!ed.isShowResolved());
    REQUIRE(!ed.isGroupBySeverity());
    REQUIRE(ed.autoResolvMinor());
    REQUIRE(ed.conflictCount() == 0u);
}

TEST_CASE("SyncConflictEditor add/remove conflicts", "[Editor][S128]") {
    SyncConflictEditor ed;
    REQUIRE(ed.addConflict(SyncConflict(1, "c_a", ConflictResolution::LocalWins,  ConflictSeverity::Minor)));
    REQUIRE(ed.addConflict(SyncConflict(2, "c_b", ConflictResolution::RemoteWins, ConflictSeverity::Moderate)));
    REQUIRE(ed.addConflict(SyncConflict(3, "c_c", ConflictResolution::Merge,      ConflictSeverity::Major)));
    REQUIRE(!ed.addConflict(SyncConflict(1, "c_a", ConflictResolution::LocalWins, ConflictSeverity::Minor)));
    REQUIRE(ed.conflictCount() == 3u);
    REQUIRE(ed.removeConflict(2));
    REQUIRE(ed.conflictCount() == 2u);
    REQUIRE(!ed.removeConflict(99));
}

TEST_CASE("SyncConflictEditor counts and find", "[Editor][S128]") {
    SyncConflictEditor ed;
    SyncConflict c1(1, "c_a", ConflictResolution::Merge,     ConflictSeverity::Minor);
    SyncConflict c2(2, "c_b", ConflictResolution::Merge,     ConflictSeverity::Moderate); c2.setIsResolved(true);
    SyncConflict c3(3, "c_c", ConflictResolution::Manual,    ConflictSeverity::Major);
    SyncConflict c4(4, "c_d", ConflictResolution::LastWrite, ConflictSeverity::Critical); c4.setIsResolved(true);
    ed.addConflict(c1); ed.addConflict(c2); ed.addConflict(c3); ed.addConflict(c4);
    REQUIRE(ed.countByResolution(ConflictResolution::Merge)      == 2u);
    REQUIRE(ed.countByResolution(ConflictResolution::Manual)     == 1u);
    REQUIRE(ed.countByResolution(ConflictResolution::LocalWins)  == 0u);
    REQUIRE(ed.countBySeverity(ConflictSeverity::Minor)          == 1u);
    REQUIRE(ed.countBySeverity(ConflictSeverity::Critical)       == 1u);
    REQUIRE(ed.countResolved()                                   == 2u);
    auto* found = ed.findConflict(3);
    REQUIRE(found != nullptr);
    REQUIRE(found->resolution() == ConflictResolution::Manual);
    REQUIRE(ed.findConflict(99) == nullptr);
}

TEST_CASE("SyncConflictEditor settings mutation", "[Editor][S128]") {
    SyncConflictEditor ed;
    ed.setIsShowResolved(true);
    ed.setIsGroupBySeverity(true);
    ed.setAutoResolvMinor(false);
    REQUIRE(ed.isShowResolved());
    REQUIRE(ed.isGroupBySeverity());
    REQUIRE(!ed.autoResolvMinor());
}
