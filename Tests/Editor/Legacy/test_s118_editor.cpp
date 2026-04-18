// S118 editor tests: CloudSyncEditor, OnlineServicesPanel, LiveOpsEditor
#include <catch2/catch_test_macros.hpp>
#include "NF/Editor/Editor.h"
#include "NF/Editor/LiveOpsEditor.h"
#include "NF/Editor/CloudSyncEditor.h"

using namespace NF;

// ── CloudSyncEditor ───────────────────────────────────────────────────────────

TEST_CASE("CloudSyncState names", "[Editor][S118]") {
    REQUIRE(std::string(cloudSyncStateName(CloudSyncState::Idle))        == "Idle");
    REQUIRE(std::string(cloudSyncStateName(CloudSyncState::Uploading))   == "Uploading");
    REQUIRE(std::string(cloudSyncStateName(CloudSyncState::Downloading)) == "Downloading");
    REQUIRE(std::string(cloudSyncStateName(CloudSyncState::Synced))      == "Synced");
    REQUIRE(std::string(cloudSyncStateName(CloudSyncState::Conflict))    == "Conflict");
    REQUIRE(std::string(cloudSyncStateName(CloudSyncState::Error))       == "Error");
    REQUIRE(std::string(cloudSyncStateName(CloudSyncState::Offline))     == "Offline");
}

TEST_CASE("CloudSyncConflictPolicy names", "[Editor][S118]") {
    REQUIRE(std::string(cloudSyncConflictPolicyName(CloudSyncConflictPolicy::KeepLocal))  == "KeepLocal");
    REQUIRE(std::string(cloudSyncConflictPolicyName(CloudSyncConflictPolicy::KeepRemote)) == "KeepRemote");
    REQUIRE(std::string(cloudSyncConflictPolicyName(CloudSyncConflictPolicy::KeepNewest)) == "KeepNewest");
    REQUIRE(std::string(cloudSyncConflictPolicyName(CloudSyncConflictPolicy::KeepOldest)) == "KeepOldest");
    REQUIRE(std::string(cloudSyncConflictPolicyName(CloudSyncConflictPolicy::Manual))     == "Manual");
}

TEST_CASE("CloudSyncDataType names", "[Editor][S118]") {
    REQUIRE(std::string(cloudSyncDataTypeName(CloudSyncDataType::SaveGame))     == "SaveGame");
    REQUIRE(std::string(cloudSyncDataTypeName(CloudSyncDataType::Settings))     == "Settings");
    REQUIRE(std::string(cloudSyncDataTypeName(CloudSyncDataType::Achievements)) == "Achievements");
    REQUIRE(std::string(cloudSyncDataTypeName(CloudSyncDataType::Statistics))   == "Statistics");
    REQUIRE(std::string(cloudSyncDataTypeName(CloudSyncDataType::Profile))      == "Profile");
    REQUIRE(std::string(cloudSyncDataTypeName(CloudSyncDataType::Custom))       == "Custom");
}

TEST_CASE("CloudSyncSlot defaults", "[Editor][S118]") {
    CloudSyncSlot sl(1, "save_slot_1", CloudSyncDataType::SaveGame);
    REQUIRE(sl.id()             == 1u);
    REQUIRE(sl.name()           == "save_slot_1");
    REQUIRE(sl.dataType()       == CloudSyncDataType::SaveGame);
    REQUIRE(sl.state()          == CloudSyncState::Idle);
    REQUIRE(sl.conflictPolicy() == CloudSyncConflictPolicy::KeepNewest);
    REQUIRE(sl.isEnabled());
    REQUIRE(sl.sizeBytes()      == 0u);
}

TEST_CASE("CloudSyncSlot mutation", "[Editor][S118]") {
    CloudSyncSlot sl(2, "settings_slot", CloudSyncDataType::Settings);
    sl.setState(CloudSyncState::Synced);
    sl.setConflictPolicy(CloudSyncConflictPolicy::Manual);
    sl.setIsEnabled(false);
    sl.setSizeBytes(4096u);
    REQUIRE(sl.state()          == CloudSyncState::Synced);
    REQUIRE(sl.conflictPolicy() == CloudSyncConflictPolicy::Manual);
    REQUIRE(!sl.isEnabled());
    REQUIRE(sl.sizeBytes()      == 4096u);
}

TEST_CASE("CloudSyncEditor defaults", "[Editor][S118]") {
    CloudSyncEditor ed;
    REQUIRE(ed.isAutoSyncEnabled());
    REQUIRE(ed.syncIntervalSec()        == 300.0f);
    REQUIRE(ed.defaultConflictPolicy()  == CloudSyncConflictPolicy::KeepNewest);
    REQUIRE(ed.slotCount()              == 0u);
}

TEST_CASE("CloudSyncEditor add/remove slots", "[Editor][S118]") {
    CloudSyncEditor ed;
    REQUIRE(ed.addSlot(CloudSyncSlot(1, "save_a",    CloudSyncDataType::SaveGame)));
    REQUIRE(ed.addSlot(CloudSyncSlot(2, "settings_a",CloudSyncDataType::Settings)));
    REQUIRE(ed.addSlot(CloudSyncSlot(3, "ach_a",     CloudSyncDataType::Achievements)));
    REQUIRE(!ed.addSlot(CloudSyncSlot(1, "save_a",   CloudSyncDataType::SaveGame)));
    REQUIRE(ed.slotCount() == 3u);
    REQUIRE(ed.removeSlot(2));
    REQUIRE(ed.slotCount() == 2u);
    REQUIRE(!ed.removeSlot(99));
}

TEST_CASE("CloudSyncEditor counts and find", "[Editor][S118]") {
    CloudSyncEditor ed;
    CloudSyncSlot s1(1, "save_a",  CloudSyncDataType::SaveGame);
    CloudSyncSlot s2(2, "save_b",  CloudSyncDataType::SaveGame);   s2.setState(CloudSyncState::Conflict);
    CloudSyncSlot s3(3, "settings",CloudSyncDataType::Settings);   s3.setIsEnabled(false);
    CloudSyncSlot s4(4, "profile", CloudSyncDataType::Profile);    s4.setState(CloudSyncState::Conflict); s4.setIsEnabled(false);
    ed.addSlot(s1); ed.addSlot(s2); ed.addSlot(s3); ed.addSlot(s4);
    REQUIRE(ed.countByDataType(CloudSyncDataType::SaveGame)  == 2u);
    REQUIRE(ed.countByDataType(CloudSyncDataType::Settings)  == 1u);
    REQUIRE(ed.countByDataType(CloudSyncDataType::Custom)    == 0u);
    REQUIRE(ed.countByState(CloudSyncState::Idle)            == 2u);
    REQUIRE(ed.countByState(CloudSyncState::Conflict)        == 2u);
    REQUIRE(ed.countEnabled()                                == 2u);
    auto* found = ed.findSlot(3);
    REQUIRE(found != nullptr);
    REQUIRE(found->dataType() == CloudSyncDataType::Settings);
    REQUIRE(ed.findSlot(99) == nullptr);
}

TEST_CASE("CloudSyncEditor settings mutation", "[Editor][S118]") {
    CloudSyncEditor ed;
    ed.setAutoSyncEnabled(false);
    ed.setSyncIntervalSec(60.0f);
    ed.setDefaultConflictPolicy(CloudSyncConflictPolicy::KeepLocal);
    REQUIRE(!ed.isAutoSyncEnabled());
    REQUIRE(ed.syncIntervalSec()       == 60.0f);
    REQUIRE(ed.defaultConflictPolicy() == CloudSyncConflictPolicy::KeepLocal);
}

// ── OnlineServicesPanel ───────────────────────────────────────────────────────

TEST_CASE("OnlineServiceType names", "[Editor][S118]") {
    REQUIRE(std::string(onlineServiceTypeName(OnlineServiceType::Authentication))   == "Authentication");
    REQUIRE(std::string(onlineServiceTypeName(OnlineServiceType::Leaderboard))      == "Leaderboard");
    REQUIRE(std::string(onlineServiceTypeName(OnlineServiceType::Achievements))     == "Achievements");
    REQUIRE(std::string(onlineServiceTypeName(OnlineServiceType::Friends))          == "Friends");
    REQUIRE(std::string(onlineServiceTypeName(OnlineServiceType::Matchmaking))      == "Matchmaking");
    REQUIRE(std::string(onlineServiceTypeName(OnlineServiceType::CloudStorage))     == "CloudStorage");
    REQUIRE(std::string(onlineServiceTypeName(OnlineServiceType::PushNotification)) == "PushNotification");
    REQUIRE(std::string(onlineServiceTypeName(OnlineServiceType::InAppPurchase))    == "InAppPurchase");
    REQUIRE(std::string(onlineServiceTypeName(OnlineServiceType::Analytics))        == "Analytics");
}

TEST_CASE("OnlineServiceStatus names", "[Editor][S118]") {
    REQUIRE(std::string(onlineServiceStatusName(OnlineServiceStatus::Unknown))      == "Unknown");
    REQUIRE(std::string(onlineServiceStatusName(OnlineServiceStatus::Connecting))   == "Connecting");
    REQUIRE(std::string(onlineServiceStatusName(OnlineServiceStatus::Connected))    == "Connected");
    REQUIRE(std::string(onlineServiceStatusName(OnlineServiceStatus::Degraded))     == "Degraded");
    REQUIRE(std::string(onlineServiceStatusName(OnlineServiceStatus::Disconnected)) == "Disconnected");
    REQUIRE(std::string(onlineServiceStatusName(OnlineServiceStatus::Maintenance))  == "Maintenance");
}

TEST_CASE("OnlineServiceEnvironment names", "[Editor][S118]") {
    REQUIRE(std::string(onlineServiceEnvironmentName(OnlineServiceEnvironment::Development)) == "Development");
    REQUIRE(std::string(onlineServiceEnvironmentName(OnlineServiceEnvironment::Staging))     == "Staging");
    REQUIRE(std::string(onlineServiceEnvironmentName(OnlineServiceEnvironment::Production))  == "Production");
    REQUIRE(std::string(onlineServiceEnvironmentName(OnlineServiceEnvironment::Sandbox))     == "Sandbox");
}

TEST_CASE("OnlineService defaults", "[Editor][S118]") {
    OnlineService svc(1, "auth_service", OnlineServiceType::Authentication);
    REQUIRE(svc.id()          == 1u);
    REQUIRE(svc.name()        == "auth_service");
    REQUIRE(svc.type()        == OnlineServiceType::Authentication);
    REQUIRE(svc.status()      == OnlineServiceStatus::Unknown);
    REQUIRE(svc.environment() == OnlineServiceEnvironment::Development);
    REQUIRE(svc.isEnabled());
    REQUIRE(svc.latencyMs()   == 0.0f);
}

TEST_CASE("OnlineService mutation", "[Editor][S118]") {
    OnlineService svc(2, "leaderboard_service", OnlineServiceType::Leaderboard);
    svc.setStatus(OnlineServiceStatus::Connected);
    svc.setEnvironment(OnlineServiceEnvironment::Production);
    svc.setIsEnabled(false);
    svc.setLatencyMs(42.5f);
    REQUIRE(svc.status()      == OnlineServiceStatus::Connected);
    REQUIRE(svc.environment() == OnlineServiceEnvironment::Production);
    REQUIRE(!svc.isEnabled());
    REQUIRE(svc.latencyMs()   == 42.5f);
}

TEST_CASE("OnlineServicesPanel defaults", "[Editor][S118]") {
    OnlineServicesPanel panel;
    REQUIRE(panel.activeEnvironment() == OnlineServiceEnvironment::Development);
    REQUIRE(panel.isShowOffline());
    REQUIRE(panel.isAutoReconnect());
    REQUIRE(panel.serviceCount()      == 0u);
}

TEST_CASE("OnlineServicesPanel add/remove services", "[Editor][S118]") {
    OnlineServicesPanel panel;
    REQUIRE(panel.addService(OnlineService(1, "auth",  OnlineServiceType::Authentication)));
    REQUIRE(panel.addService(OnlineService(2, "lboard",OnlineServiceType::Leaderboard)));
    REQUIRE(panel.addService(OnlineService(3, "match", OnlineServiceType::Matchmaking)));
    REQUIRE(!panel.addService(OnlineService(1, "auth", OnlineServiceType::Authentication)));
    REQUIRE(panel.serviceCount() == 3u);
    REQUIRE(panel.removeService(2));
    REQUIRE(panel.serviceCount() == 2u);
    REQUIRE(!panel.removeService(99));
}

TEST_CASE("OnlineServicesPanel counts and find", "[Editor][S118]") {
    OnlineServicesPanel panel;
    OnlineService s1(1, "auth_a",  OnlineServiceType::Authentication);
    OnlineService s2(2, "auth_b",  OnlineServiceType::Authentication);  s2.setStatus(OnlineServiceStatus::Degraded);
    OnlineService s3(3, "lboard",  OnlineServiceType::Leaderboard);     s3.setIsEnabled(false);
    OnlineService s4(4, "match",   OnlineServiceType::Matchmaking);     s4.setStatus(OnlineServiceStatus::Degraded); s4.setIsEnabled(false);
    panel.addService(s1); panel.addService(s2); panel.addService(s3); panel.addService(s4);
    REQUIRE(panel.countByType(OnlineServiceType::Authentication) == 2u);
    REQUIRE(panel.countByType(OnlineServiceType::Leaderboard)    == 1u);
    REQUIRE(panel.countByType(OnlineServiceType::Friends)        == 0u);
    REQUIRE(panel.countByStatus(OnlineServiceStatus::Unknown)    == 2u);
    REQUIRE(panel.countByStatus(OnlineServiceStatus::Degraded)   == 2u);
    REQUIRE(panel.countEnabled()                                 == 2u);
    auto* found = panel.findService(3);
    REQUIRE(found != nullptr);
    REQUIRE(found->type() == OnlineServiceType::Leaderboard);
    REQUIRE(panel.findService(99) == nullptr);
}

TEST_CASE("OnlineServicesPanel settings mutation", "[Editor][S118]") {
    OnlineServicesPanel panel;
    panel.setActiveEnvironment(OnlineServiceEnvironment::Production);
    panel.setShowOffline(false);
    panel.setAutoReconnect(false);
    REQUIRE(panel.activeEnvironment() == OnlineServiceEnvironment::Production);
    REQUIRE(!panel.isShowOffline());
    REQUIRE(!panel.isAutoReconnect());
}

// ── LiveOpsEditor ─────────────────────────────────────────────────────────────

TEST_CASE("LiveOpsEventType names", "[Editor][S118]") {
    REQUIRE(std::string(liveOpsEventTypeName(LiveOpsEventType::LimitedTimeOffer)) == "LimitedTimeOffer");
    REQUIRE(std::string(liveOpsEventTypeName(LiveOpsEventType::SeasonalEvent))    == "SeasonalEvent");
    REQUIRE(std::string(liveOpsEventTypeName(LiveOpsEventType::DailyChallenge))   == "DailyChallenge");
    REQUIRE(std::string(liveOpsEventTypeName(LiveOpsEventType::WeeklyChallenge))  == "WeeklyChallenge");
    REQUIRE(std::string(liveOpsEventTypeName(LiveOpsEventType::BonusXP))          == "BonusXP");
    REQUIRE(std::string(liveOpsEventTypeName(LiveOpsEventType::DoubleRewards))    == "DoubleRewards");
    REQUIRE(std::string(liveOpsEventTypeName(LiveOpsEventType::FlashSale))        == "FlashSale");
    REQUIRE(std::string(liveOpsEventTypeName(LiveOpsEventType::CommunityGoal))    == "CommunityGoal");
}

TEST_CASE("LiveOpsEventState names", "[Editor][S118]") {
    REQUIRE(std::string(liveOpsEventStateName(LiveOpsEventState::Draft))     == "Draft");
    REQUIRE(std::string(liveOpsEventStateName(LiveOpsEventState::Scheduled)) == "Scheduled");
    REQUIRE(std::string(liveOpsEventStateName(LiveOpsEventState::Active))    == "Active");
    REQUIRE(std::string(liveOpsEventStateName(LiveOpsEventState::Paused))    == "Paused");
    REQUIRE(std::string(liveOpsEventStateName(LiveOpsEventState::Completed)) == "Completed");
    REQUIRE(std::string(liveOpsEventStateName(LiveOpsEventState::Cancelled)) == "Cancelled");
}

TEST_CASE("LiveOpsTargetAudience names", "[Editor][S118]") {
    REQUIRE(std::string(liveOpsTargetAudienceName(LiveOpsTargetAudience::All))        == "All");
    REQUIRE(std::string(liveOpsTargetAudienceName(LiveOpsTargetAudience::NewPlayers)) == "NewPlayers");
    REQUIRE(std::string(liveOpsTargetAudienceName(LiveOpsTargetAudience::Veterans))   == "Veterans");
    REQUIRE(std::string(liveOpsTargetAudienceName(LiveOpsTargetAudience::Churned))    == "Churned");
    REQUIRE(std::string(liveOpsTargetAudienceName(LiveOpsTargetAudience::Whales))     == "Whales");
    REQUIRE(std::string(liveOpsTargetAudienceName(LiveOpsTargetAudience::Regional))   == "Regional");
    REQUIRE(std::string(liveOpsTargetAudienceName(LiveOpsTargetAudience::Custom))     == "Custom");
}

TEST_CASE("LiveOpsEvent defaults", "[Editor][S118]") {
    LiveOpsEvent ev(1, "summer_sale", LiveOpsEventType::LimitedTimeOffer);
    REQUIRE(ev.id()             == 1u);
    REQUIRE(ev.name()           == "summer_sale");
    REQUIRE(ev.type()           == LiveOpsEventType::LimitedTimeOffer);
    REQUIRE(ev.state()          == LiveOpsEventState::Draft);
    REQUIRE(ev.targetAudience() == LiveOpsTargetAudience::All);
    REQUIRE(ev.durationHours()  == 24.0f);
    REQUIRE(!ev.isFeatured());
}

TEST_CASE("LiveOpsEvent mutation", "[Editor][S118]") {
    LiveOpsEvent ev(2, "winter_event", LiveOpsEventType::SeasonalEvent);
    ev.setState(LiveOpsEventState::Active);
    ev.setTargetAudience(LiveOpsTargetAudience::Veterans);
    ev.setDurationHours(168.0f);
    ev.setIsFeatured(true);
    REQUIRE(ev.state()          == LiveOpsEventState::Active);
    REQUIRE(ev.targetAudience() == LiveOpsTargetAudience::Veterans);
    REQUIRE(ev.durationHours()  == 168.0f);
    REQUIRE(ev.isFeatured());
}

TEST_CASE("LiveOpsEditor defaults", "[Editor][S118]") {
    LiveOpsEditor ed;
    REQUIRE(!ed.isShowInactive());
    REQUIRE(!ed.isPreviewMode());
    REQUIRE(ed.defaultDurationHours() == 24.0f);
    REQUIRE(ed.eventCount()           == 0u);
}

TEST_CASE("LiveOpsEditor add/remove events", "[Editor][S118]") {
    LiveOpsEditor ed;
    REQUIRE(ed.addEvent(LiveOpsEvent(1, "sale_a",    LiveOpsEventType::LimitedTimeOffer)));
    REQUIRE(ed.addEvent(LiveOpsEvent(2, "seasonal_a",LiveOpsEventType::SeasonalEvent)));
    REQUIRE(ed.addEvent(LiveOpsEvent(3, "daily_a",   LiveOpsEventType::DailyChallenge)));
    REQUIRE(!ed.addEvent(LiveOpsEvent(1, "sale_a",   LiveOpsEventType::LimitedTimeOffer)));
    REQUIRE(ed.eventCount() == 3u);
    REQUIRE(ed.removeEvent(2));
    REQUIRE(ed.eventCount() == 2u);
    REQUIRE(!ed.removeEvent(99));
}

TEST_CASE("LiveOpsEditor counts and find", "[Editor][S118]") {
    LiveOpsEditor ed;
    LiveOpsEvent e1(1, "sale_a",    LiveOpsEventType::LimitedTimeOffer);
    LiveOpsEvent e2(2, "sale_b",    LiveOpsEventType::LimitedTimeOffer);  e2.setState(LiveOpsEventState::Active);
    LiveOpsEvent e3(3, "seasonal",  LiveOpsEventType::SeasonalEvent);     e3.setIsFeatured(true);
    LiveOpsEvent e4(4, "daily",     LiveOpsEventType::DailyChallenge);    e4.setState(LiveOpsEventState::Active); e4.setIsFeatured(true);
    ed.addEvent(e1); ed.addEvent(e2); ed.addEvent(e3); ed.addEvent(e4);
    REQUIRE(ed.countByType(LiveOpsEventType::LimitedTimeOffer) == 2u);
    REQUIRE(ed.countByType(LiveOpsEventType::SeasonalEvent)    == 1u);
    REQUIRE(ed.countByType(LiveOpsEventType::BonusXP)          == 0u);
    REQUIRE(ed.countByState(LiveOpsEventState::Draft)          == 2u);
    REQUIRE(ed.countByState(LiveOpsEventState::Active)         == 2u);
    REQUIRE(ed.countFeatured()                                 == 2u);
    auto* found = ed.findEvent(3);
    REQUIRE(found != nullptr);
    REQUIRE(found->type() == LiveOpsEventType::SeasonalEvent);
    REQUIRE(ed.findEvent(99) == nullptr);
}

TEST_CASE("LiveOpsEditor settings mutation", "[Editor][S118]") {
    LiveOpsEditor ed;
    ed.setShowInactive(true);
    ed.setPreviewMode(true);
    ed.setDefaultDurationHours(48.0f);
    REQUIRE(ed.isShowInactive());
    REQUIRE(ed.isPreviewMode());
    REQUIRE(ed.defaultDurationHours() == 48.0f);
}
