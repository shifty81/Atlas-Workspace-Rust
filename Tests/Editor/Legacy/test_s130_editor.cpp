// S130 editor tests: EventBusEditor, MessageQueueEditor, PubSubEditor
#include <catch2/catch_test_macros.hpp>
#include "NF/Editor/Editor.h"
#include "NF/Editor/PubSubEditor.h"
#include "NF/Editor/MessageQueueEditor.h"
#include "NF/Editor/EventBusEditor.h"

using namespace NF;

// ── EventBusEditor ────────────────────────────────────────────────────────────

TEST_CASE("EventBusPriority names", "[Editor][S130]") {
    REQUIRE(std::string(eventBusPriorityName(EventBusPriority::Low))      == "Low");
    REQUIRE(std::string(eventBusPriorityName(EventBusPriority::Normal))   == "Normal");
    REQUIRE(std::string(eventBusPriorityName(EventBusPriority::High))     == "High");
    REQUIRE(std::string(eventBusPriorityName(EventBusPriority::Critical)) == "Critical");
    REQUIRE(std::string(eventBusPriorityName(EventBusPriority::Realtime)) == "Realtime");
}

TEST_CASE("EventBusScope names", "[Editor][S130]") {
    REQUIRE(std::string(eventBusScopeName(EventBusScope::Local))   == "Local");
    REQUIRE(std::string(eventBusScopeName(EventBusScope::Scene))   == "Scene");
    REQUIRE(std::string(eventBusScopeName(EventBusScope::Global))  == "Global");
    REQUIRE(std::string(eventBusScopeName(EventBusScope::Network)) == "Network");
}

TEST_CASE("EventBusChannel defaults", "[Editor][S130]") {
    EventBusChannel c(1, "game_events", EventBusPriority::Normal, EventBusScope::Scene);
    REQUIRE(c.id()           == 1u);
    REQUIRE(c.name()         == "game_events");
    REQUIRE(c.priority()     == EventBusPriority::Normal);
    REQUIRE(c.scope()        == EventBusScope::Scene);
    REQUIRE(c.maxListeners() == 64u);
    REQUIRE(!c.isBuffered());
    REQUIRE(c.isEnabled());
}

TEST_CASE("EventBusChannel mutation", "[Editor][S130]") {
    EventBusChannel c(2, "critical_bus", EventBusPriority::Critical, EventBusScope::Global);
    c.setMaxListeners(128u);
    c.setIsBuffered(true);
    c.setIsEnabled(false);
    REQUIRE(c.maxListeners() == 128u);
    REQUIRE(c.isBuffered());
    REQUIRE(!c.isEnabled());
}

TEST_CASE("EventBusEditor defaults", "[Editor][S130]") {
    EventBusEditor ed;
    REQUIRE(!ed.isShowDisabled());
    REQUIRE(!ed.isGroupByScope());
    REQUIRE(ed.defaultMaxListeners() == 32u);
    REQUIRE(ed.channelCount()        == 0u);
}

TEST_CASE("EventBusEditor add/remove channels", "[Editor][S130]") {
    EventBusEditor ed;
    REQUIRE(ed.addChannel(EventBusChannel(1, "c_a", EventBusPriority::Low,      EventBusScope::Local)));
    REQUIRE(ed.addChannel(EventBusChannel(2, "c_b", EventBusPriority::Normal,   EventBusScope::Scene)));
    REQUIRE(ed.addChannel(EventBusChannel(3, "c_c", EventBusPriority::Critical, EventBusScope::Global)));
    REQUIRE(!ed.addChannel(EventBusChannel(1, "c_a", EventBusPriority::Low,     EventBusScope::Local)));
    REQUIRE(ed.channelCount() == 3u);
    REQUIRE(ed.removeChannel(2));
    REQUIRE(ed.channelCount() == 2u);
    REQUIRE(!ed.removeChannel(99));
}

TEST_CASE("EventBusEditor counts and find", "[Editor][S130]") {
    EventBusEditor ed;
    EventBusChannel c1(1, "c_a", EventBusPriority::Normal,   EventBusScope::Scene);
    EventBusChannel c2(2, "c_b", EventBusPriority::Normal,   EventBusScope::Local);
    EventBusChannel c3(3, "c_c", EventBusPriority::High,     EventBusScope::Global);
    EventBusChannel c4(4, "c_d", EventBusPriority::Realtime, EventBusScope::Network); c4.setIsEnabled(false);
    ed.addChannel(c1); ed.addChannel(c2); ed.addChannel(c3); ed.addChannel(c4);
    REQUIRE(ed.countByPriority(EventBusPriority::Normal)   == 2u);
    REQUIRE(ed.countByPriority(EventBusPriority::High)     == 1u);
    REQUIRE(ed.countByPriority(EventBusPriority::Low)      == 0u);
    REQUIRE(ed.countByScope(EventBusScope::Scene)          == 1u);
    REQUIRE(ed.countByScope(EventBusScope::Local)          == 1u);
    REQUIRE(ed.countEnabled()                              == 3u);
    auto* found = ed.findChannel(3);
    REQUIRE(found != nullptr);
    REQUIRE(found->priority() == EventBusPriority::High);
    REQUIRE(ed.findChannel(99) == nullptr);
}

TEST_CASE("EventBusEditor settings mutation", "[Editor][S130]") {
    EventBusEditor ed;
    ed.setIsShowDisabled(true);
    ed.setIsGroupByScope(true);
    ed.setDefaultMaxListeners(128u);
    REQUIRE(ed.isShowDisabled());
    REQUIRE(ed.isGroupByScope());
    REQUIRE(ed.defaultMaxListeners() == 128u);
}

// ── MessageQueueEditor ────────────────────────────────────────────────────────

TEST_CASE("MsgQueuePolicy names", "[Editor][S130]") {
    REQUIRE(std::string(msgQueuePolicyName(MsgQueuePolicy::FIFO))       == "FIFO");
    REQUIRE(std::string(msgQueuePolicyName(MsgQueuePolicy::LIFO))       == "LIFO");
    REQUIRE(std::string(msgQueuePolicyName(MsgQueuePolicy::Priority))   == "Priority");
    REQUIRE(std::string(msgQueuePolicyName(MsgQueuePolicy::RoundRobin)) == "RoundRobin");
    REQUIRE(std::string(msgQueuePolicyName(MsgQueuePolicy::Custom))     == "Custom");
}

TEST_CASE("MsgQueueState names", "[Editor][S130]") {
    REQUIRE(std::string(msgQueueStateName(MsgQueueState::Idle))     == "Idle");
    REQUIRE(std::string(msgQueueStateName(MsgQueueState::Active))   == "Active");
    REQUIRE(std::string(msgQueueStateName(MsgQueueState::Paused))   == "Paused");
    REQUIRE(std::string(msgQueueStateName(MsgQueueState::Draining)) == "Draining");
    REQUIRE(std::string(msgQueueStateName(MsgQueueState::Error))    == "Error");
}

TEST_CASE("MessageQueue defaults", "[Editor][S130]") {
    MessageQueue q(1, "main_queue", MsgQueuePolicy::FIFO);
    REQUIRE(q.id()        == 1u);
    REQUIRE(q.name()      == "main_queue");
    REQUIRE(q.policy()    == MsgQueuePolicy::FIFO);
    REQUIRE(q.state()     == MsgQueueState::Idle);
    REQUIRE(q.maxDepth()  == 256u);
    REQUIRE(!q.isDeduped());
    REQUIRE(q.isEnabled());
}

TEST_CASE("MessageQueue mutation", "[Editor][S130]") {
    MessageQueue q(2, "priority_queue", MsgQueuePolicy::Priority);
    q.setState(MsgQueueState::Active);
    q.setMaxDepth(512u);
    q.setIsDeduped(true);
    q.setIsEnabled(false);
    REQUIRE(q.state()     == MsgQueueState::Active);
    REQUIRE(q.maxDepth()  == 512u);
    REQUIRE(q.isDeduped());
    REQUIRE(!q.isEnabled());
}

TEST_CASE("MessageQueueEditor defaults", "[Editor][S130]") {
    MessageQueueEditor ed;
    REQUIRE(!ed.isShowDisabled());
    REQUIRE(!ed.isGroupByPolicy());
    REQUIRE(ed.defaultMaxDepth() == 128u);
    REQUIRE(ed.queueCount()      == 0u);
}

TEST_CASE("MessageQueueEditor add/remove queues", "[Editor][S130]") {
    MessageQueueEditor ed;
    REQUIRE(ed.addQueue(MessageQueue(1, "q_a", MsgQueuePolicy::FIFO)));
    REQUIRE(ed.addQueue(MessageQueue(2, "q_b", MsgQueuePolicy::LIFO)));
    REQUIRE(ed.addQueue(MessageQueue(3, "q_c", MsgQueuePolicy::Priority)));
    REQUIRE(!ed.addQueue(MessageQueue(1, "q_a", MsgQueuePolicy::FIFO)));
    REQUIRE(ed.queueCount() == 3u);
    REQUIRE(ed.removeQueue(2));
    REQUIRE(ed.queueCount() == 2u);
    REQUIRE(!ed.removeQueue(99));
}

TEST_CASE("MessageQueueEditor counts and find", "[Editor][S130]") {
    MessageQueueEditor ed;
    MessageQueue q1(1, "q_a", MsgQueuePolicy::FIFO);
    MessageQueue q2(2, "q_b", MsgQueuePolicy::FIFO);   q2.setState(MsgQueueState::Active);
    MessageQueue q3(3, "q_c", MsgQueuePolicy::LIFO);   q3.setState(MsgQueueState::Paused);
    MessageQueue q4(4, "q_d", MsgQueuePolicy::Custom); q4.setState(MsgQueueState::Error); q4.setIsEnabled(false);
    ed.addQueue(q1); ed.addQueue(q2); ed.addQueue(q3); ed.addQueue(q4);
    REQUIRE(ed.countByPolicy(MsgQueuePolicy::FIFO)        == 2u);
    REQUIRE(ed.countByPolicy(MsgQueuePolicy::LIFO)        == 1u);
    REQUIRE(ed.countByPolicy(MsgQueuePolicy::RoundRobin)  == 0u);
    REQUIRE(ed.countByState(MsgQueueState::Idle)          == 1u);
    REQUIRE(ed.countByState(MsgQueueState::Active)        == 1u);
    REQUIRE(ed.countEnabled()                             == 3u);
    auto* found = ed.findQueue(3);
    REQUIRE(found != nullptr);
    REQUIRE(found->policy() == MsgQueuePolicy::LIFO);
    REQUIRE(ed.findQueue(99) == nullptr);
}

TEST_CASE("MessageQueueEditor settings mutation", "[Editor][S130]") {
    MessageQueueEditor ed;
    ed.setIsShowDisabled(true);
    ed.setIsGroupByPolicy(true);
    ed.setDefaultMaxDepth(512u);
    REQUIRE(ed.isShowDisabled());
    REQUIRE(ed.isGroupByPolicy());
    REQUIRE(ed.defaultMaxDepth() == 512u);
}

// ── PubSubEditor ──────────────────────────────────────────────────────────────

TEST_CASE("PubSubDelivery names", "[Editor][S130]") {
    REQUIRE(std::string(pubSubDeliveryName(PubSubDelivery::AtMostOnce))  == "AtMostOnce");
    REQUIRE(std::string(pubSubDeliveryName(PubSubDelivery::AtLeastOnce)) == "AtLeastOnce");
    REQUIRE(std::string(pubSubDeliveryName(PubSubDelivery::ExactlyOnce)) == "ExactlyOnce");
    REQUIRE(std::string(pubSubDeliveryName(PubSubDelivery::BestEffort))  == "BestEffort");
}

TEST_CASE("PubSubRetention names", "[Editor][S130]") {
    REQUIRE(std::string(pubSubRetentionName(PubSubRetention::None))       == "None");
    REQUIRE(std::string(pubSubRetentionName(PubSubRetention::Session))    == "Session");
    REQUIRE(std::string(pubSubRetentionName(PubSubRetention::Persistent)) == "Persistent");
    REQUIRE(std::string(pubSubRetentionName(PubSubRetention::Infinite))   == "Infinite");
}

TEST_CASE("PubSubTopic defaults", "[Editor][S130]") {
    PubSubTopic t(1, "game_state", PubSubDelivery::AtLeastOnce, PubSubRetention::Session);
    REQUIRE(t.id()             == 1u);
    REQUIRE(t.name()           == "game_state");
    REQUIRE(t.delivery()       == PubSubDelivery::AtLeastOnce);
    REQUIRE(t.retention()      == PubSubRetention::Session);
    REQUIRE(t.maxSubscribers() == 128u);
    REQUIRE(!t.isCached());
    REQUIRE(t.isEnabled());
}

TEST_CASE("PubSubTopic mutation", "[Editor][S130]") {
    PubSubTopic t(2, "persistent_topic", PubSubDelivery::ExactlyOnce, PubSubRetention::Persistent);
    t.setMaxSubscribers(256u);
    t.setIsCached(true);
    t.setIsEnabled(false);
    REQUIRE(t.maxSubscribers() == 256u);
    REQUIRE(t.isCached());
    REQUIRE(!t.isEnabled());
}

TEST_CASE("PubSubEditor defaults", "[Editor][S130]") {
    PubSubEditor ed;
    REQUIRE(!ed.isShowDisabled());
    REQUIRE(!ed.isGroupByDelivery());
    REQUIRE(ed.defaultMaxSubscribers() == 64u);
    REQUIRE(ed.topicCount()            == 0u);
}

TEST_CASE("PubSubEditor add/remove topics", "[Editor][S130]") {
    PubSubEditor ed;
    REQUIRE(ed.addTopic(PubSubTopic(1, "t_a", PubSubDelivery::AtMostOnce,  PubSubRetention::None)));
    REQUIRE(ed.addTopic(PubSubTopic(2, "t_b", PubSubDelivery::AtLeastOnce, PubSubRetention::Session)));
    REQUIRE(ed.addTopic(PubSubTopic(3, "t_c", PubSubDelivery::ExactlyOnce, PubSubRetention::Persistent)));
    REQUIRE(!ed.addTopic(PubSubTopic(1, "t_a", PubSubDelivery::AtMostOnce, PubSubRetention::None)));
    REQUIRE(ed.topicCount() == 3u);
    REQUIRE(ed.removeTopic(2));
    REQUIRE(ed.topicCount() == 2u);
    REQUIRE(!ed.removeTopic(99));
}

TEST_CASE("PubSubEditor counts and find", "[Editor][S130]") {
    PubSubEditor ed;
    PubSubTopic t1(1, "t_a", PubSubDelivery::AtLeastOnce, PubSubRetention::Session);
    PubSubTopic t2(2, "t_b", PubSubDelivery::AtLeastOnce, PubSubRetention::None);
    PubSubTopic t3(3, "t_c", PubSubDelivery::ExactlyOnce, PubSubRetention::Persistent);
    PubSubTopic t4(4, "t_d", PubSubDelivery::BestEffort,  PubSubRetention::Infinite); t4.setIsEnabled(false);
    ed.addTopic(t1); ed.addTopic(t2); ed.addTopic(t3); ed.addTopic(t4);
    REQUIRE(ed.countByDelivery(PubSubDelivery::AtLeastOnce) == 2u);
    REQUIRE(ed.countByDelivery(PubSubDelivery::ExactlyOnce) == 1u);
    REQUIRE(ed.countByDelivery(PubSubDelivery::AtMostOnce)  == 0u);
    REQUIRE(ed.countByRetention(PubSubRetention::Session)   == 1u);
    REQUIRE(ed.countByRetention(PubSubRetention::None)      == 1u);
    REQUIRE(ed.countEnabled()                               == 3u);
    auto* found = ed.findTopic(3);
    REQUIRE(found != nullptr);
    REQUIRE(found->delivery() == PubSubDelivery::ExactlyOnce);
    REQUIRE(ed.findTopic(99) == nullptr);
}

TEST_CASE("PubSubEditor settings mutation", "[Editor][S130]") {
    PubSubEditor ed;
    ed.setIsShowDisabled(true);
    ed.setIsGroupByDelivery(true);
    ed.setDefaultMaxSubscribers(256u);
    REQUIRE(ed.isShowDisabled());
    REQUIRE(ed.isGroupByDelivery());
    REQUIRE(ed.defaultMaxSubscribers() == 256u);
}
