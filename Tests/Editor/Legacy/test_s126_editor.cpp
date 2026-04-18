// S126 editor tests: NotificationCenterEditor, AlertRuleEditor, BroadcastEditor
#include <catch2/catch_test_macros.hpp>
#include "NF/Editor/Editor.h"
#include "NF/Editor/BroadcastEditor.h"
#include "NF/Editor/AlertRuleEditor.h"
#include "NF/Editor/NotificationCenterEditor.h"

using namespace NF;

// ── NotificationCenterEditor ──────────────────────────────────────────────────

TEST_CASE("NotifChannel names", "[Editor][S126]") {
    REQUIRE(std::string(notifChannelName(NotifChannel::InEditor)) == "InEditor");
    REQUIRE(std::string(notifChannelName(NotifChannel::Email))    == "Email");
    REQUIRE(std::string(notifChannelName(NotifChannel::Push))     == "Push");
    REQUIRE(std::string(notifChannelName(NotifChannel::Webhook))  == "Webhook");
    REQUIRE(std::string(notifChannelName(NotifChannel::Slack))    == "Slack");
    REQUIRE(std::string(notifChannelName(NotifChannel::Custom))   == "Custom");
}

TEST_CASE("NotifPriority names", "[Editor][S126]") {
    REQUIRE(std::string(notifPriorityName(NotifPriority::Low))      == "Low");
    REQUIRE(std::string(notifPriorityName(NotifPriority::Normal))   == "Normal");
    REQUIRE(std::string(notifPriorityName(NotifPriority::High))     == "High");
    REQUIRE(std::string(notifPriorityName(NotifPriority::Critical)) == "Critical");
}

TEST_CASE("NotifEntry defaults", "[Editor][S126]") {
    NotifEntry n(1, "Build finished", NotifChannel::InEditor, NotifPriority::Normal);
    REQUIRE(n.id()       == 1u);
    REQUIRE(n.title()    == "Build finished");
    REQUIRE(n.channel()  == NotifChannel::InEditor);
    REQUIRE(n.priority() == NotifPriority::Normal);
    REQUIRE(!n.isRead());
    REQUIRE(!n.isMuted());
    REQUIRE(n.isEnabled());
}

TEST_CASE("NotifEntry mutation", "[Editor][S126]") {
    NotifEntry n(2, "Critical error", NotifChannel::Slack, NotifPriority::Critical);
    n.setIsRead(true);
    n.setIsMuted(true);
    n.setIsEnabled(false);
    REQUIRE(n.isRead());
    REQUIRE(n.isMuted());
    REQUIRE(!n.isEnabled());
}

TEST_CASE("NotificationCenterEditor defaults", "[Editor][S126]") {
    NotificationCenterEditor ed;
    REQUIRE(!ed.isShowMuted());
    REQUIRE(!ed.isGroupByChannel());
    REQUIRE(ed.maxNotifications() == 200u);
    REQUIRE(ed.notifCount()       == 0u);
}

TEST_CASE("NotificationCenterEditor add/remove notifs", "[Editor][S126]") {
    NotificationCenterEditor ed;
    REQUIRE(ed.addNotif(NotifEntry(1, "n_a", NotifChannel::InEditor, NotifPriority::Low)));
    REQUIRE(ed.addNotif(NotifEntry(2, "n_b", NotifChannel::Email,    NotifPriority::Normal)));
    REQUIRE(ed.addNotif(NotifEntry(3, "n_c", NotifChannel::Slack,    NotifPriority::High)));
    REQUIRE(!ed.addNotif(NotifEntry(1, "n_a", NotifChannel::InEditor, NotifPriority::Low)));
    REQUIRE(ed.notifCount() == 3u);
    REQUIRE(ed.removeNotif(2));
    REQUIRE(ed.notifCount() == 2u);
    REQUIRE(!ed.removeNotif(99));
}

TEST_CASE("NotificationCenterEditor counts and find", "[Editor][S126]") {
    NotificationCenterEditor ed;
    NotifEntry n1(1, "n_a", NotifChannel::InEditor, NotifPriority::Low);
    NotifEntry n2(2, "n_b", NotifChannel::InEditor, NotifPriority::Normal); n2.setIsRead(true);
    NotifEntry n3(3, "n_c", NotifChannel::Email,    NotifPriority::High);
    NotifEntry n4(4, "n_d", NotifChannel::Slack,    NotifPriority::Critical); n4.setIsRead(true);
    ed.addNotif(n1); ed.addNotif(n2); ed.addNotif(n3); ed.addNotif(n4);
    REQUIRE(ed.countByChannel(NotifChannel::InEditor)   == 2u);
    REQUIRE(ed.countByChannel(NotifChannel::Email)      == 1u);
    REQUIRE(ed.countByChannel(NotifChannel::Push)       == 0u);
    REQUIRE(ed.countByPriority(NotifPriority::Low)      == 1u);
    REQUIRE(ed.countByPriority(NotifPriority::Critical) == 1u);
    REQUIRE(ed.countUnread()                            == 2u);
    auto* found = ed.findNotif(3);
    REQUIRE(found != nullptr);
    REQUIRE(found->channel() == NotifChannel::Email);
    REQUIRE(ed.findNotif(99) == nullptr);
}

TEST_CASE("NotificationCenterEditor settings mutation", "[Editor][S126]") {
    NotificationCenterEditor ed;
    ed.setIsShowMuted(true);
    ed.setIsGroupByChannel(true);
    ed.setMaxNotifications(500u);
    REQUIRE(ed.isShowMuted());
    REQUIRE(ed.isGroupByChannel());
    REQUIRE(ed.maxNotifications() == 500u);
}

// ── AlertRuleEditor ───────────────────────────────────────────────────────────

TEST_CASE("AlertCondition names", "[Editor][S126]") {
    REQUIRE(std::string(alertConditionName(AlertCondition::ThresholdExceeded)) == "ThresholdExceeded");
    REQUIRE(std::string(alertConditionName(AlertCondition::PatternMatch))      == "PatternMatch");
    REQUIRE(std::string(alertConditionName(AlertCondition::RateChange))        == "RateChange");
    REQUIRE(std::string(alertConditionName(AlertCondition::Absence))           == "Absence");
    REQUIRE(std::string(alertConditionName(AlertCondition::Compound))          == "Compound");
}

TEST_CASE("AlertSeverity names", "[Editor][S126]") {
    REQUIRE(std::string(alertSeverityName(AlertSeverity::Info))     == "Info");
    REQUIRE(std::string(alertSeverityName(AlertSeverity::Warning))  == "Warning");
    REQUIRE(std::string(alertSeverityName(AlertSeverity::Error))    == "Error");
    REQUIRE(std::string(alertSeverityName(AlertSeverity::Critical)) == "Critical");
}

TEST_CASE("AlertRule defaults", "[Editor][S126]") {
    AlertRule r(1, "cpu_threshold", AlertCondition::ThresholdExceeded, AlertSeverity::Warning);
    REQUIRE(r.id()          == 1u);
    REQUIRE(r.name()        == "cpu_threshold");
    REQUIRE(r.condition()   == AlertCondition::ThresholdExceeded);
    REQUIRE(r.severity()    == AlertSeverity::Warning);
    REQUIRE(r.isActive());
    REQUIRE(r.cooldownSec() == 60.0f);
    REQUIRE(r.isEnabled());
}

TEST_CASE("AlertRule mutation", "[Editor][S126]") {
    AlertRule r(2, "pattern_alert", AlertCondition::PatternMatch, AlertSeverity::Critical);
    r.setIsActive(false);
    r.setCooldownSec(120.0f);
    r.setIsEnabled(false);
    REQUIRE(!r.isActive());
    REQUIRE(r.cooldownSec() == 120.0f);
    REQUIRE(!r.isEnabled());
}

TEST_CASE("AlertRuleEditor defaults", "[Editor][S126]") {
    AlertRuleEditor ed;
    REQUIRE(ed.isShowInactive());
    REQUIRE(!ed.isGroupBySeverity());
    REQUIRE(ed.globalCooldownSec() == 30.0f);
    REQUIRE(ed.ruleCount()         == 0u);
}

TEST_CASE("AlertRuleEditor add/remove rules", "[Editor][S126]") {
    AlertRuleEditor ed;
    REQUIRE(ed.addRule(AlertRule(1, "r_a", AlertCondition::ThresholdExceeded, AlertSeverity::Info)));
    REQUIRE(ed.addRule(AlertRule(2, "r_b", AlertCondition::PatternMatch,      AlertSeverity::Warning)));
    REQUIRE(ed.addRule(AlertRule(3, "r_c", AlertCondition::RateChange,        AlertSeverity::Error)));
    REQUIRE(!ed.addRule(AlertRule(1, "r_a", AlertCondition::ThresholdExceeded, AlertSeverity::Info)));
    REQUIRE(ed.ruleCount() == 3u);
    REQUIRE(ed.removeRule(2));
    REQUIRE(ed.ruleCount() == 2u);
    REQUIRE(!ed.removeRule(99));
}

TEST_CASE("AlertRuleEditor counts and find", "[Editor][S126]") {
    AlertRuleEditor ed;
    AlertRule r1(1, "r_a", AlertCondition::ThresholdExceeded, AlertSeverity::Info);
    AlertRule r2(2, "r_b", AlertCondition::ThresholdExceeded, AlertSeverity::Warning); r2.setIsActive(false);
    AlertRule r3(3, "r_c", AlertCondition::PatternMatch,      AlertSeverity::Error);
    AlertRule r4(4, "r_d", AlertCondition::RateChange,        AlertSeverity::Critical); r4.setIsActive(false);
    ed.addRule(r1); ed.addRule(r2); ed.addRule(r3); ed.addRule(r4);
    REQUIRE(ed.countByCondition(AlertCondition::ThresholdExceeded) == 2u);
    REQUIRE(ed.countByCondition(AlertCondition::PatternMatch)      == 1u);
    REQUIRE(ed.countByCondition(AlertCondition::Absence)           == 0u);
    REQUIRE(ed.countBySeverity(AlertSeverity::Info)                == 1u);
    REQUIRE(ed.countBySeverity(AlertSeverity::Warning)             == 1u);
    REQUIRE(ed.countActive()                                       == 2u);
    auto* found = ed.findRule(3);
    REQUIRE(found != nullptr);
    REQUIRE(found->condition() == AlertCondition::PatternMatch);
    REQUIRE(ed.findRule(99) == nullptr);
}

TEST_CASE("AlertRuleEditor settings mutation", "[Editor][S126]") {
    AlertRuleEditor ed;
    ed.setIsShowInactive(false);
    ed.setIsGroupBySeverity(true);
    ed.setGlobalCooldownSec(90.0f);
    REQUIRE(!ed.isShowInactive());
    REQUIRE(ed.isGroupBySeverity());
    REQUIRE(ed.globalCooldownSec() == 90.0f);
}

// ── BroadcastEditor ───────────────────────────────────────────────────────────

TEST_CASE("BroadcastType names", "[Editor][S126]") {
    REQUIRE(std::string(broadcastTypeName(BroadcastType::System))      == "System");
    REQUIRE(std::string(broadcastTypeName(BroadcastType::Event))       == "Event");
    REQUIRE(std::string(broadcastTypeName(BroadcastType::News))        == "News");
    REQUIRE(std::string(broadcastTypeName(BroadcastType::Maintenance)) == "Maintenance");
    REQUIRE(std::string(broadcastTypeName(BroadcastType::Custom))      == "Custom");
}

TEST_CASE("BroadcastStatus names", "[Editor][S126]") {
    REQUIRE(std::string(broadcastStatusName(BroadcastStatus::Draft))     == "Draft");
    REQUIRE(std::string(broadcastStatusName(BroadcastStatus::Scheduled)) == "Scheduled");
    REQUIRE(std::string(broadcastStatusName(BroadcastStatus::Live))      == "Live");
    REQUIRE(std::string(broadcastStatusName(BroadcastStatus::Completed)) == "Completed");
    REQUIRE(std::string(broadcastStatusName(BroadcastStatus::Cancelled)) == "Cancelled");
}

TEST_CASE("BroadcastMessage defaults", "[Editor][S126]") {
    BroadcastMessage msg(1, "server_maintenance", BroadcastType::Maintenance);
    REQUIRE(msg.id()           == 1u);
    REQUIRE(msg.title()        == "server_maintenance");
    REQUIRE(msg.type()         == BroadcastType::Maintenance);
    REQUIRE(msg.status()       == BroadcastStatus::Draft);
    REQUIRE(msg.audienceSize() == 0u);
    REQUIRE(!msg.isUrgent());
    REQUIRE(msg.isEnabled());
}

TEST_CASE("BroadcastMessage mutation", "[Editor][S126]") {
    BroadcastMessage msg(2, "big_event", BroadcastType::Event);
    msg.setStatus(BroadcastStatus::Live);
    msg.setAudienceSize(500000u);
    msg.setIsUrgent(true);
    msg.setIsEnabled(false);
    REQUIRE(msg.status()       == BroadcastStatus::Live);
    REQUIRE(msg.audienceSize() == 500000u);
    REQUIRE(msg.isUrgent());
    REQUIRE(!msg.isEnabled());
}

TEST_CASE("BroadcastEditor defaults", "[Editor][S126]") {
    BroadcastEditor ed;
    REQUIRE(!ed.isShowCompleted());
    REQUIRE(!ed.isGroupByType());
    REQUIRE(ed.maxAudienceSize() == 1000000u);
    REQUIRE(ed.messageCount()    == 0u);
}

TEST_CASE("BroadcastEditor add/remove messages", "[Editor][S126]") {
    BroadcastEditor ed;
    REQUIRE(ed.addMessage(BroadcastMessage(1, "m_a", BroadcastType::System)));
    REQUIRE(ed.addMessage(BroadcastMessage(2, "m_b", BroadcastType::Event)));
    REQUIRE(ed.addMessage(BroadcastMessage(3, "m_c", BroadcastType::News)));
    REQUIRE(!ed.addMessage(BroadcastMessage(1, "m_a", BroadcastType::System)));
    REQUIRE(ed.messageCount() == 3u);
    REQUIRE(ed.removeMessage(2));
    REQUIRE(ed.messageCount() == 2u);
    REQUIRE(!ed.removeMessage(99));
}

TEST_CASE("BroadcastEditor counts and find", "[Editor][S126]") {
    BroadcastEditor ed;
    BroadcastMessage m1(1, "m_a", BroadcastType::System);
    BroadcastMessage m2(2, "m_b", BroadcastType::System);      m2.setStatus(BroadcastStatus::Live);
    BroadcastMessage m3(3, "m_c", BroadcastType::Event);       m3.setIsUrgent(true);
    BroadcastMessage m4(4, "m_d", BroadcastType::Maintenance); m4.setStatus(BroadcastStatus::Live); m4.setIsUrgent(true);
    ed.addMessage(m1); ed.addMessage(m2); ed.addMessage(m3); ed.addMessage(m4);
    REQUIRE(ed.countByType(BroadcastType::System)           == 2u);
    REQUIRE(ed.countByType(BroadcastType::Event)            == 1u);
    REQUIRE(ed.countByType(BroadcastType::News)             == 0u);
    REQUIRE(ed.countByStatus(BroadcastStatus::Draft)        == 2u);
    REQUIRE(ed.countByStatus(BroadcastStatus::Live)         == 2u);
    REQUIRE(ed.countUrgent()                                == 2u);
    auto* found = ed.findMessage(3);
    REQUIRE(found != nullptr);
    REQUIRE(found->type() == BroadcastType::Event);
    REQUIRE(ed.findMessage(99) == nullptr);
}

TEST_CASE("BroadcastEditor settings mutation", "[Editor][S126]") {
    BroadcastEditor ed;
    ed.setIsShowCompleted(true);
    ed.setIsGroupByType(true);
    ed.setMaxAudienceSize(2000000u);
    REQUIRE(ed.isShowCompleted());
    REQUIRE(ed.isGroupByType());
    REQUIRE(ed.maxAudienceSize() == 2000000u);
}
