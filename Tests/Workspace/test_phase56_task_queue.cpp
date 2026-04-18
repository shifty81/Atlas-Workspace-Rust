// Tests/Workspace/test_phase56_task_queue.cpp
// Phase 56 — WorkspaceTaskQueue: TaskPriority, TaskState, TaskDescriptor,
//             TaskResult, TaskEntry, TaskQueue
#include <catch2/catch_test_macros.hpp>
#include "NF/Workspace/WorkspaceTaskQueue.h"

// ═════════════════════════════════════════════════════════════════
// TaskPriority / TaskState
// ═════════════════════════════════════════════════════════════════

TEST_CASE("TaskPriority: name helpers", "[taskqueue][priority]") {
    REQUIRE(std::string(NF::taskPriorityName(NF::TaskPriority::Low))      == "Low");
    REQUIRE(std::string(NF::taskPriorityName(NF::TaskPriority::Normal))   == "Normal");
    REQUIRE(std::string(NF::taskPriorityName(NF::TaskPriority::High))     == "High");
    REQUIRE(std::string(NF::taskPriorityName(NF::TaskPriority::Critical)) == "Critical");
}

TEST_CASE("TaskState: name helpers", "[taskqueue][state]") {
    REQUIRE(std::string(NF::taskStateName(NF::TaskState::Pending))   == "Pending");
    REQUIRE(std::string(NF::taskStateName(NF::TaskState::Running))   == "Running");
    REQUIRE(std::string(NF::taskStateName(NF::TaskState::Completed)) == "Completed");
    REQUIRE(std::string(NF::taskStateName(NF::TaskState::Failed))    == "Failed");
    REQUIRE(std::string(NF::taskStateName(NF::TaskState::Cancelled)) == "Cancelled");
}

// ═════════════════════════════════════════════════════════════════
// TaskResult
// ═════════════════════════════════════════════════════════════════

TEST_CASE("TaskResult: ok factory", "[taskqueue][result]") {
    auto r = NF::TaskResult::ok(42);
    REQUIRE(r.succeeded);
    REQUIRE(r.durationMs == 42);
    REQUIRE(r.errorMessage.empty());
}

TEST_CASE("TaskResult: fail factory", "[taskqueue][result]") {
    auto r = NF::TaskResult::fail("oops", 10);
    REQUIRE_FALSE(r.succeeded);
    REQUIRE(r.errorMessage == "oops");
    REQUIRE(r.durationMs == 10);
}

// ═════════════════════════════════════════════════════════════════
// TaskDescriptor
// ═════════════════════════════════════════════════════════════════

static NF::TaskDescriptor makeDesc(const std::string& id, const std::string& label,
                                   NF::TaskPriority pri = NF::TaskPriority::Normal,
                                   bool handlerResult = true) {
    NF::TaskDescriptor desc;
    desc.id       = id;
    desc.label    = label;
    desc.priority = pri;
    desc.handler  = [handlerResult](std::function<void(int)>) { return handlerResult; };
    return desc;
}

TEST_CASE("TaskDescriptor: valid with all fields", "[taskqueue][descriptor]") {
    auto d = makeDesc("t1", "Task One");
    REQUIRE(d.isValid());
}

TEST_CASE("TaskDescriptor: invalid without id", "[taskqueue][descriptor]") {
    auto d = makeDesc("", "Task");
    REQUIRE_FALSE(d.isValid());
}

TEST_CASE("TaskDescriptor: invalid without handler", "[taskqueue][descriptor]") {
    NF::TaskDescriptor d;
    d.id    = "t1";
    d.label = "Test";
    REQUIRE_FALSE(d.isValid());
}

// ═════════════════════════════════════════════════════════════════
// TaskEntry
// ═════════════════════════════════════════════════════════════════

TEST_CASE("TaskEntry: initial state is not terminal", "[taskqueue][entry]") {
    NF::TaskEntry entry;
    REQUIRE_FALSE(entry.isTerminal());
    REQUIRE(entry.state == NF::TaskState::Pending);
}

TEST_CASE("TaskEntry: lifecycle start->complete", "[taskqueue][entry]") {
    NF::TaskEntry entry;
    REQUIRE(entry.start());
    REQUIRE(entry.state == NF::TaskState::Running);
    REQUIRE(entry.complete(50));
    REQUIRE(entry.state == NF::TaskState::Completed);
    REQUIRE(entry.isTerminal());
    REQUIRE(entry.progress == 100);
    REQUIRE(entry.result.succeeded);
}

TEST_CASE("TaskEntry: lifecycle start->fail", "[taskqueue][entry]") {
    NF::TaskEntry entry;
    entry.start();
    REQUIRE(entry.fail("broken", 10));
    REQUIRE(entry.state == NF::TaskState::Failed);
    REQUIRE(entry.isTerminal());
    REQUIRE(entry.result.errorMessage == "broken");
}

TEST_CASE("TaskEntry: cancel from pending", "[taskqueue][entry]") {
    NF::TaskEntry entry;
    REQUIRE(entry.cancel());
    REQUIRE(entry.state == NF::TaskState::Cancelled);
    REQUIRE(entry.isTerminal());
}

TEST_CASE("TaskEntry: cancel from running", "[taskqueue][entry]") {
    NF::TaskEntry entry;
    entry.start();
    REQUIRE(entry.cancel());
    REQUIRE(entry.state == NF::TaskState::Cancelled);
}

TEST_CASE("TaskEntry: cannot cancel completed", "[taskqueue][entry]") {
    NF::TaskEntry entry;
    entry.start();
    entry.complete();
    REQUIRE_FALSE(entry.cancel());
}

TEST_CASE("TaskEntry: cannot start twice", "[taskqueue][entry]") {
    NF::TaskEntry entry;
    entry.start();
    REQUIRE_FALSE(entry.start());
}

TEST_CASE("TaskEntry: setProgress clamps", "[taskqueue][entry]") {
    NF::TaskEntry entry;
    entry.start();
    entry.setProgress(150);
    REQUIRE(entry.progress == 100);
    entry.setProgress(-5);
    REQUIRE(entry.progress == 0);
    entry.setProgress(42);
    REQUIRE(entry.progress == 42);
}

TEST_CASE("TaskEntry: setProgress ignored when not running", "[taskqueue][entry]") {
    NF::TaskEntry entry;
    entry.setProgress(50);
    REQUIRE(entry.progress == 0);
}

// ═════════════════════════════════════════════════════════════════
// TaskQueue
// ═════════════════════════════════════════════════════════════════

TEST_CASE("TaskQueue: default empty", "[taskqueue][queue]") {
    NF::TaskQueue queue;
    REQUIRE(queue.empty());
    REQUIRE(queue.totalEntries() == 0);
}

TEST_CASE("TaskQueue: enqueue", "[taskqueue][queue]") {
    NF::TaskQueue queue;
    REQUIRE(queue.enqueue(makeDesc("t1", "Task 1")));
    REQUIRE(queue.totalEntries() == 1);
    REQUIRE(queue.hasEntry("t1"));
}

TEST_CASE("TaskQueue: enqueue rejects invalid", "[taskqueue][queue]") {
    NF::TaskQueue queue;
    NF::TaskDescriptor bad;
    REQUIRE_FALSE(queue.enqueue(bad));
}

TEST_CASE("TaskQueue: enqueue rejects duplicate", "[taskqueue][queue]") {
    NF::TaskQueue queue;
    queue.enqueue(makeDesc("t1", "Task 1"));
    REQUIRE_FALSE(queue.enqueue(makeDesc("t1", "Task 1 dup")));
}

TEST_CASE("TaskQueue: cancel pending", "[taskqueue][queue]") {
    NF::TaskQueue queue;
    queue.enqueue(makeDesc("t1", "Task 1"));
    REQUIRE(queue.cancel("t1"));
    REQUIRE(queue.findEntry("t1")->state == NF::TaskState::Cancelled);
}

TEST_CASE("TaskQueue: cancel unknown", "[taskqueue][queue]") {
    NF::TaskQueue queue;
    REQUIRE_FALSE(queue.cancel("nope"));
}

TEST_CASE("TaskQueue: tick dispatches and completes", "[taskqueue][queue]") {
    NF::TaskQueue queue;
    queue.enqueue(makeDesc("t1", "Task 1"));
    int dispatched = queue.tick();
    REQUIRE(dispatched == 1);
    REQUIRE(queue.findEntry("t1")->state == NF::TaskState::Completed);
}

TEST_CASE("TaskQueue: tick dispatches highest priority first", "[taskqueue][queue]") {
    NF::TaskQueue queue;
    std::vector<std::string> order;
    auto makeTracked = [&](const std::string& id, NF::TaskPriority pri) {
        NF::TaskDescriptor d;
        d.id       = id;
        d.label    = id;
        d.priority = pri;
        d.handler  = [&order, id](std::function<void(int)>) {
            order.push_back(id);
            return true;
        };
        return d;
    };

    queue.enqueue(makeTracked("low",  NF::TaskPriority::Low));
    queue.enqueue(makeTracked("high", NF::TaskPriority::High));
    queue.enqueue(makeTracked("crit", NF::TaskPriority::Critical));
    queue.enqueue(makeTracked("norm", NF::TaskPriority::Normal));
    queue.tick();

    REQUIRE(order.size() == 4);
    REQUIRE(order[0] == "crit");
    REQUIRE(order[1] == "high");
    REQUIRE(order[2] == "norm");
    REQUIRE(order[3] == "low");
}

TEST_CASE("TaskQueue: tick handles handler failure", "[taskqueue][queue]") {
    NF::TaskQueue queue;
    queue.enqueue(makeDesc("t1", "FailTask", NF::TaskPriority::Normal, false));
    queue.tick();
    REQUIRE(queue.findEntry("t1")->state == NF::TaskState::Failed);
}

TEST_CASE("TaskQueue: handler sets progress", "[taskqueue][queue]") {
    NF::TaskQueue queue;
    NF::TaskDescriptor desc;
    desc.id    = "t1";
    desc.label = "Progress";
    desc.handler = [](std::function<void(int)> setProgress) {
        setProgress(50);
        return true;
    };
    queue.enqueue(desc);
    queue.tick();
    // Completed task has progress 100
    REQUIRE(queue.findEntry("t1")->progress == 100);
}

TEST_CASE("TaskQueue: countByState", "[taskqueue][queue]") {
    NF::TaskQueue queue;
    queue.enqueue(makeDesc("t1", "A"));
    queue.enqueue(makeDesc("t2", "B"));
    REQUIRE(queue.countByState(NF::TaskState::Pending) == 2);
    queue.tick();
    REQUIRE(queue.countByState(NF::TaskState::Completed) == 2);
    REQUIRE(queue.countByState(NF::TaskState::Pending) == 0);
}

TEST_CASE("TaskQueue: countByPriority", "[taskqueue][queue]") {
    NF::TaskQueue queue;
    queue.enqueue(makeDesc("t1", "A", NF::TaskPriority::High));
    queue.enqueue(makeDesc("t2", "B", NF::TaskPriority::High));
    queue.enqueue(makeDesc("t3", "C", NF::TaskPriority::Low));
    REQUIRE(queue.countByPriority(NF::TaskPriority::High) == 2);
    REQUIRE(queue.countByPriority(NF::TaskPriority::Low) == 1);
}

TEST_CASE("TaskQueue: pendingTasks view", "[taskqueue][queue]") {
    NF::TaskQueue queue;
    queue.enqueue(makeDesc("t1", "A"));
    auto pending = queue.pendingTasks();
    REQUIRE(pending.size() == 1);
    REQUIRE(pending[0]->descriptor.id == "t1");
}

TEST_CASE("TaskQueue: clearCompleted", "[taskqueue][queue]") {
    NF::TaskQueue queue;
    queue.enqueue(makeDesc("t1", "A"));
    queue.enqueue(makeDesc("t2", "B"));
    queue.tick();
    int removed = queue.clearCompleted();
    REQUIRE(removed == 2);
    REQUIRE(queue.empty());
}

TEST_CASE("TaskQueue: observer notified on tick", "[taskqueue][queue]") {
    NF::TaskQueue queue;
    int notified = 0;
    queue.addObserver([&](const NF::TaskEntry&) { ++notified; });
    queue.enqueue(makeDesc("t1", "A"));
    queue.tick();
    REQUIRE(notified == 1);
}

TEST_CASE("TaskQueue: observer notified on cancel", "[taskqueue][queue]") {
    NF::TaskQueue queue;
    int notified = 0;
    queue.addObserver([&](const NF::TaskEntry&) { ++notified; });
    queue.enqueue(makeDesc("t1", "A"));
    queue.cancel("t1");
    REQUIRE(notified == 1);
}

TEST_CASE("TaskQueue: clearObservers", "[taskqueue][queue]") {
    NF::TaskQueue queue;
    int count = 0;
    queue.addObserver([&](const NF::TaskEntry&) { ++count; });
    queue.clearObservers();
    queue.enqueue(makeDesc("t1", "A"));
    queue.tick();
    REQUIRE(count == 0);
}

TEST_CASE("TaskQueue: clear removes everything", "[taskqueue][queue]") {
    NF::TaskQueue queue;
    queue.enqueue(makeDesc("t1", "A"));
    queue.clear();
    REQUIRE(queue.empty());
}

// ═════════════════════════════════════════════════════════════════
// Integration
// ═════════════════════════════════════════════════════════════════

TEST_CASE("Integration: enqueue, tick, cancel workflow", "[taskqueue][integration]") {
    NF::TaskQueue queue;
    queue.enqueue(makeDesc("build", "Build", NF::TaskPriority::High));
    queue.enqueue(makeDesc("lint",  "Lint",  NF::TaskPriority::Normal));
    queue.enqueue(makeDesc("test",  "Test",  NF::TaskPriority::Low));

    // Cancel lint before dispatch
    queue.cancel("lint");
    REQUIRE(queue.findEntry("lint")->state == NF::TaskState::Cancelled);

    // Tick dispatches remaining
    queue.tick();
    REQUIRE(queue.findEntry("build")->state == NF::TaskState::Completed);
    REQUIRE(queue.findEntry("test")->state  == NF::TaskState::Completed);

    // Clear completed
    int removed = queue.clearCompleted();
    REQUIRE(removed == 3); // completed + cancelled are terminal
}

TEST_CASE("Integration: observer tracks all lifecycle events", "[taskqueue][integration]") {
    NF::TaskQueue queue;
    std::vector<std::string> events;
    queue.addObserver([&](const NF::TaskEntry& e) {
        events.push_back(e.descriptor.id + ":" + NF::taskStateName(e.state));
    });

    queue.enqueue(makeDesc("a", "A"));
    queue.enqueue(makeDesc("b", "B"));
    queue.cancel("b");
    queue.tick();

    REQUIRE(events.size() == 2);
    REQUIRE(events[0] == "b:Cancelled");
    REQUIRE(events[1] == "a:Completed");
}
