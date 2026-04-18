// S173 editor tests: CloudStorageEditorV1, CloudSyncEditorV1, LiveOpsEditorV1
#include <catch2/catch_test_macros.hpp>
#include <catch2/catch_approx.hpp>
#include "NF/Editor/Editor.h"
#include "NF/Editor/CloudStorageEditorV1.h"
#include "NF/Editor/CloudSyncEditorV1.h"
#include "NF/Editor/LiveOpsEditorV1.h"

using namespace NF;
using Catch::Approx;

// ── CloudStorageEditorV1 ─────────────────────────────────────────────────────

TEST_CASE("Csev1Bucket validity", "[Editor][S173]") {
    Csev1Bucket b;
    REQUIRE(!b.isValid());
    b.id = 1; b.name = "AssetsBucket";
    REQUIRE(b.isValid());
}

TEST_CASE("CloudStorageEditorV1 addBucket and bucketCount", "[Editor][S173]") {
    CloudStorageEditorV1 cse;
    REQUIRE(cse.bucketCount() == 0);
    Csev1Bucket b; b.id = 1; b.name = "B1";
    REQUIRE(cse.addBucket(b));
    REQUIRE(cse.bucketCount() == 1);
}

TEST_CASE("CloudStorageEditorV1 addBucket invalid fails", "[Editor][S173]") {
    CloudStorageEditorV1 cse;
    REQUIRE(!cse.addBucket(Csev1Bucket{}));
}

TEST_CASE("CloudStorageEditorV1 addBucket duplicate fails", "[Editor][S173]") {
    CloudStorageEditorV1 cse;
    Csev1Bucket b; b.id = 1; b.name = "A";
    cse.addBucket(b);
    REQUIRE(!cse.addBucket(b));
}

TEST_CASE("CloudStorageEditorV1 removeBucket", "[Editor][S173]") {
    CloudStorageEditorV1 cse;
    Csev1Bucket b; b.id = 2; b.name = "B";
    cse.addBucket(b);
    REQUIRE(cse.removeBucket(2));
    REQUIRE(cse.bucketCount() == 0);
    REQUIRE(!cse.removeBucket(2));
}

TEST_CASE("CloudStorageEditorV1 setState availableCount", "[Editor][S173]") {
    CloudStorageEditorV1 cse;
    Csev1Bucket b; b.id = 1; b.name = "A";
    cse.addBucket(b);
    REQUIRE(cse.setState(1, Csev1BucketState::Available));
    REQUIRE(cse.availableCount() == 1);
    REQUIRE(cse.findBucket(1)->isAvailable());
}

TEST_CASE("CloudStorageEditorV1 archivedCount", "[Editor][S173]") {
    CloudStorageEditorV1 cse;
    Csev1Bucket b; b.id = 1; b.name = "A";
    cse.addBucket(b);
    cse.setState(1, Csev1BucketState::Archived);
    REQUIRE(cse.archivedCount() == 1);
    REQUIRE(cse.findBucket(1)->isArchived());
}

TEST_CASE("CloudStorageEditorV1 setAccessLevel countByAccess", "[Editor][S173]") {
    CloudStorageEditorV1 cse;
    Csev1Bucket b1; b1.id = 1; b1.name = "A"; b1.access = Csev1AccessLevel::Private;
    Csev1Bucket b2; b2.id = 2; b2.name = "B"; b2.access = Csev1AccessLevel::Public;
    cse.addBucket(b1); cse.addBucket(b2);
    REQUIRE(cse.countByAccess(Csev1AccessLevel::Private) == 1);
    REQUIRE(cse.countByAccess(Csev1AccessLevel::Public)  == 1);
    REQUIRE(cse.setAccessLevel(1, Csev1AccessLevel::ReadOnly));
    REQUIRE(cse.countByAccess(Csev1AccessLevel::ReadOnly) == 1);
}

TEST_CASE("CloudStorageEditorV1 error state", "[Editor][S173]") {
    CloudStorageEditorV1 cse;
    Csev1Bucket b; b.id = 1; b.name = "A";
    cse.addBucket(b);
    cse.setState(1, Csev1BucketState::Error);
    REQUIRE(cse.findBucket(1)->hasError());
}

TEST_CASE("Csev1Bucket addAsset", "[Editor][S173]") {
    Csev1Bucket b; b.id = 1; b.name = "A";
    Csev1AssetRef ref; ref.assetId = "tex001"; ref.remotePath = "cloud/tex001.png";
    REQUIRE(b.addAsset(ref));
    REQUIRE(b.assets.size() == 1);
    REQUIRE(!b.addAsset(ref)); // duplicate
}

TEST_CASE("Csev1AssetRef validity", "[Editor][S173]") {
    Csev1AssetRef ref;
    REQUIRE(!ref.isValid());
    ref.assetId = "a"; ref.remotePath = "b";
    REQUIRE(ref.isValid());
}

TEST_CASE("csev1BucketStateName covers all values", "[Editor][S173]") {
    REQUIRE(std::string(csev1BucketStateName(Csev1BucketState::Unknown))   == "Unknown");
    REQUIRE(std::string(csev1BucketStateName(Csev1BucketState::Error))     == "Error");
}

TEST_CASE("csev1AccessLevelName covers all values", "[Editor][S173]") {
    REQUIRE(std::string(csev1AccessLevelName(Csev1AccessLevel::Private))  == "Private");
    REQUIRE(std::string(csev1AccessLevelName(Csev1AccessLevel::ReadOnly)) == "ReadOnly");
}

TEST_CASE("CloudStorageEditorV1 findBucket returns nullptr when missing", "[Editor][S173]") {
    CloudStorageEditorV1 cse;
    REQUIRE(cse.findBucket(99) == nullptr);
}

TEST_CASE("CloudStorageEditorV1 onChange callback", "[Editor][S173]") {
    CloudStorageEditorV1 cse;
    uint64_t notified = 0;
    cse.setOnChange([&](uint64_t id) { notified = id; });
    Csev1Bucket b; b.id = 3; b.name = "C";
    cse.addBucket(b);
    cse.setState(3, Csev1BucketState::Available);
    REQUIRE(notified == 3);
}

// ── CloudSyncEditorV1 ────────────────────────────────────────────────────────

TEST_CASE("Csynv1SyncSession validity", "[Editor][S173]") {
    Csynv1SyncSession s;
    REQUIRE(!s.isValid());
    s.id = 1; s.name = "MainSync";
    REQUIRE(s.isValid());
}

TEST_CASE("CloudSyncEditorV1 addSession and sessionCount", "[Editor][S173]") {
    CloudSyncEditorV1 csy;
    REQUIRE(csy.sessionCount() == 0);
    Csynv1SyncSession s; s.id = 1; s.name = "S1";
    REQUIRE(csy.addSession(s));
    REQUIRE(csy.sessionCount() == 1);
}

TEST_CASE("CloudSyncEditorV1 addSession invalid fails", "[Editor][S173]") {
    CloudSyncEditorV1 csy;
    REQUIRE(!csy.addSession(Csynv1SyncSession{}));
}

TEST_CASE("CloudSyncEditorV1 addSession duplicate fails", "[Editor][S173]") {
    CloudSyncEditorV1 csy;
    Csynv1SyncSession s; s.id = 1; s.name = "A";
    csy.addSession(s);
    REQUIRE(!csy.addSession(s));
}

TEST_CASE("CloudSyncEditorV1 removeSession", "[Editor][S173]") {
    CloudSyncEditorV1 csy;
    Csynv1SyncSession s; s.id = 2; s.name = "B";
    csy.addSession(s);
    REQUIRE(csy.removeSession(2));
    REQUIRE(csy.sessionCount() == 0);
    REQUIRE(!csy.removeSession(2));
}

TEST_CASE("CloudSyncEditorV1 setState pushing", "[Editor][S173]") {
    CloudSyncEditorV1 csy;
    Csynv1SyncSession s; s.id = 1; s.name = "A";
    csy.addSession(s);
    REQUIRE(csy.setState(1, Csynv1SyncState::Pushing));
    REQUIRE(csy.findSession(1)->isPushing());
    REQUIRE(csy.countByState(Csynv1SyncState::Pushing) == 1);
}

TEST_CASE("CloudSyncEditorV1 setState pulling", "[Editor][S173]") {
    CloudSyncEditorV1 csy;
    Csynv1SyncSession s; s.id = 1; s.name = "A";
    csy.addSession(s);
    csy.setState(1, Csynv1SyncState::Pulling);
    REQUIRE(csy.findSession(1)->isPulling());
}

TEST_CASE("CloudSyncEditorV1 conflictCount", "[Editor][S173]") {
    CloudSyncEditorV1 csy;
    Csynv1SyncSession s; s.id = 1; s.name = "A";
    csy.addSession(s);
    csy.setState(1, Csynv1SyncState::Conflict);
    REQUIRE(csy.conflictCount() == 1);
    REQUIRE(csy.findSession(1)->hasConflict());
}

TEST_CASE("CloudSyncEditorV1 resolveConflict", "[Editor][S173]") {
    CloudSyncEditorV1 csy;
    Csynv1SyncSession s; s.id = 1; s.name = "A";
    Csynv1ConflictRecord rec; rec.filePath = "Assets/Texture.png";
    s.addConflict(rec);
    csy.addSession(s);
    REQUIRE(csy.resolveConflict(1, "Assets/Texture.png", Csynv1ConflictResolution::Local));
    REQUIRE(csy.findSession(1)->conflicts[0].isResolved());
}

TEST_CASE("Csynv1ConflictRecord validity and resolution", "[Editor][S173]") {
    Csynv1ConflictRecord rec;
    REQUIRE(!rec.isValid());
    rec.filePath = "file.cpp";
    REQUIRE(rec.isValid());
    REQUIRE(!rec.isResolved());
    rec.resolution = Csynv1ConflictResolution::Merge;
    REQUIRE(rec.isResolved());
}

TEST_CASE("csynv1SyncStateName covers all values", "[Editor][S173]") {
    REQUIRE(std::string(csynv1SyncStateName(Csynv1SyncState::Idle))     == "Idle");
    REQUIRE(std::string(csynv1SyncStateName(Csynv1SyncState::Complete)) == "Complete");
}

TEST_CASE("csynv1ConflictResolutionName covers all values", "[Editor][S173]") {
    REQUIRE(std::string(csynv1ConflictResolutionName(Csynv1ConflictResolution::None))  == "None");
    REQUIRE(std::string(csynv1ConflictResolutionName(Csynv1ConflictResolution::Skip))  == "Skip");
}

TEST_CASE("CloudSyncEditorV1 onStateChange callback", "[Editor][S173]") {
    CloudSyncEditorV1 csy;
    uint64_t notified = 0;
    csy.setOnStateChange([&](uint64_t id) { notified = id; });
    Csynv1SyncSession s; s.id = 8; s.name = "H";
    csy.addSession(s);
    csy.setState(8, Csynv1SyncState::Complete);
    REQUIRE(notified == 8);
}

// ── LiveOpsEditorV1 ──────────────────────────────────────────────────────────

TEST_CASE("Lopsv1Campaign validity", "[Editor][S173]") {
    Lopsv1Campaign c;
    REQUIRE(!c.isValid());
    c.id = 1; c.name = "SummerEvent";
    REQUIRE(c.isValid());
}

TEST_CASE("LiveOpsEditorV1 addCampaign and campaignCount", "[Editor][S173]") {
    LiveOpsEditorV1 lo;
    REQUIRE(lo.campaignCount() == 0);
    Lopsv1Campaign c; c.id = 1; c.name = "C1";
    REQUIRE(lo.addCampaign(c));
    REQUIRE(lo.campaignCount() == 1);
}

TEST_CASE("LiveOpsEditorV1 addCampaign invalid fails", "[Editor][S173]") {
    LiveOpsEditorV1 lo;
    REQUIRE(!lo.addCampaign(Lopsv1Campaign{}));
}

TEST_CASE("LiveOpsEditorV1 addCampaign duplicate fails", "[Editor][S173]") {
    LiveOpsEditorV1 lo;
    Lopsv1Campaign c; c.id = 1; c.name = "A";
    lo.addCampaign(c);
    REQUIRE(!lo.addCampaign(c));
}

TEST_CASE("LiveOpsEditorV1 removeCampaign", "[Editor][S173]") {
    LiveOpsEditorV1 lo;
    Lopsv1Campaign c; c.id = 2; c.name = "B";
    lo.addCampaign(c);
    REQUIRE(lo.removeCampaign(2));
    REQUIRE(lo.campaignCount() == 0);
    REQUIRE(!lo.removeCampaign(2));
}

TEST_CASE("LiveOpsEditorV1 setState activeCount", "[Editor][S173]") {
    LiveOpsEditorV1 lo;
    Lopsv1Campaign c; c.id = 1; c.name = "A";
    lo.addCampaign(c);
    REQUIRE(lo.setState(1, Lopsv1CampaignState::Active));
    REQUIRE(lo.activeCount() == 1);
    REQUIRE(lo.findCampaign(1)->isActive());
}

TEST_CASE("LiveOpsEditorV1 archivedCount", "[Editor][S173]") {
    LiveOpsEditorV1 lo;
    Lopsv1Campaign c; c.id = 1; c.name = "A";
    lo.addCampaign(c);
    lo.setState(1, Lopsv1CampaignState::Archived);
    REQUIRE(lo.archivedCount() == 1);
    REQUIRE(lo.findCampaign(1)->isArchived());
}

TEST_CASE("LiveOpsEditorV1 countByType", "[Editor][S173]") {
    LiveOpsEditorV1 lo;
    Lopsv1Campaign c1; c1.id = 1; c1.name = "A"; c1.type = Lopsv1EventType::Push;
    Lopsv1Campaign c2; c2.id = 2; c2.name = "B"; c2.type = Lopsv1EventType::Email;
    lo.addCampaign(c1); lo.addCampaign(c2);
    REQUIRE(lo.countByType(Lopsv1EventType::Push)  == 1);
    REQUIRE(lo.countByType(Lopsv1EventType::Email) == 1);
}

TEST_CASE("Lopsv1Campaign addEvent", "[Editor][S173]") {
    Lopsv1Campaign c; c.id = 1; c.name = "A";
    Lopsv1Event ev; ev.name = "PushNotif"; ev.type = Lopsv1EventType::Push;
    REQUIRE(c.addEvent(ev));
    REQUIRE(c.events.size() == 1);
    REQUIRE(!c.addEvent(ev)); // duplicate
}

TEST_CASE("Lopsv1Campaign paused state", "[Editor][S173]") {
    LiveOpsEditorV1 lo;
    Lopsv1Campaign c; c.id = 1; c.name = "A";
    lo.addCampaign(c);
    lo.setState(1, Lopsv1CampaignState::Paused);
    REQUIRE(lo.findCampaign(1)->isPaused());
}

TEST_CASE("lopsv1CampaignStateName covers all values", "[Editor][S173]") {
    REQUIRE(std::string(lopsv1CampaignStateName(Lopsv1CampaignState::Draft))    == "Draft");
    REQUIRE(std::string(lopsv1CampaignStateName(Lopsv1CampaignState::Archived)) == "Archived");
}

TEST_CASE("lopsv1EventTypeName covers all values", "[Editor][S173]") {
    REQUIRE(std::string(lopsv1EventTypeName(Lopsv1EventType::Push))      == "Push");
    REQUIRE(std::string(lopsv1EventTypeName(Lopsv1EventType::Challenge)) == "Challenge");
}

TEST_CASE("LiveOpsEditorV1 findCampaign returns nullptr when missing", "[Editor][S173]") {
    LiveOpsEditorV1 lo;
    REQUIRE(lo.findCampaign(99) == nullptr);
}

TEST_CASE("LiveOpsEditorV1 onChange callback", "[Editor][S173]") {
    LiveOpsEditorV1 lo;
    uint64_t notified = 0;
    lo.setOnChange([&](uint64_t id) { notified = id; });
    Lopsv1Campaign c; c.id = 4; c.name = "D";
    lo.addCampaign(c);
    lo.setState(4, Lopsv1CampaignState::Active);
    REQUIRE(notified == 4);
}
