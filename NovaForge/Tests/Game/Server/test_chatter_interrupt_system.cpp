// Tests for: ChatterInterruptSystem
#include "test_log.h"
#include "components/fleet_components.h"
#include "ecs/system.h"
#include "systems/chatter_interrupt_system.h"

using namespace atlas;

static void testChatterInterruptInit() {
    std::cout << "\n=== ChatterInterrupt: Init ===" << std::endl;
    ecs::World world;
    systems::ChatterInterruptSystem sys(&world);
    world.createEntity("e1");
    assertTrue(sys.initialize("e1"), "Init succeeds");
    assertTrue(!sys.isSpeaking("e1"), "Not speaking initially");
    assertTrue(sys.getActiveLineText("e1").empty(), "No active text initially");
    assertTrue(sys.getActiveLineId("e1").empty(), "No active id initially");
    assertTrue(approxEqual(sys.getActivePriority("e1"), 0.0f), "Zero priority initially");
    assertTrue(sys.isActiveInterruptible("e1"), "Default interruptible flag is true");
    assertTrue(!sys.wasInterrupted("e1"), "Not interrupted initially");
    assertTrue(sys.getQueueDepth("e1") == 0, "Queue empty initially");
    assertTrue(sys.getTotalLinesQueued("e1") == 0, "Zero queued total");
    assertTrue(sys.getTotalLinesSpoken("e1") == 0, "Zero spoken total");
    assertTrue(sys.getTotalInterrupts("e1") == 0, "Zero interrupts");
    assertTrue(sys.getMaxQueueSize("e1") == 5, "Default max queue size 5");
    assertTrue(!sys.initialize("nonexistent"), "Init fails on missing entity");
}

static void testChatterInterruptQueueLine() {
    std::cout << "\n=== ChatterInterrupt: QueueLine ===" << std::endl;
    ecs::World world;
    systems::ChatterInterruptSystem sys(&world);
    world.createEntity("e1");
    sys.initialize("e1");

    // First line starts immediately
    assertTrue(sys.queueLine("e1", "l1", "Hello captain", 1.0f, 3.0f, true, "Idle"),
               "First line queued");
    assertTrue(sys.isSpeaking("e1"), "Speaking after first queue");
    assertTrue(sys.getActiveLineId("e1") == "l1", "Active line is l1");
    assertTrue(sys.getActiveLineText("e1") == "Hello captain", "Active text correct");
    assertTrue(approxEqual(sys.getActivePriority("e1"), 1.0f), "Active priority 1.0");
    assertTrue(sys.getQueueDepth("e1") == 0, "Queue depth 0 after first line");
    assertTrue(sys.getTotalLinesQueued("e1") == 1, "Total queued is 1");

    // Second line goes to queue
    assertTrue(sys.queueLine("e1", "l2", "Second line", 0.5f, 2.0f, true, "Idle"),
               "Second line queued");
    assertTrue(sys.getQueueDepth("e1") == 1, "Queue depth 1 after second");
    assertTrue(sys.hasLineInQueue("e1", "l2"), "l2 in queue");
    assertTrue(sys.getTotalLinesQueued("e1") == 2, "Total queued is 2");

    // Empty line_id rejected
    assertTrue(!sys.queueLine("e1", "", "text", 1.0f, 3.0f, true, "Idle"),
               "Empty line_id rejected");

    // Zero duration rejected
    assertTrue(!sys.queueLine("e1", "l3", "text", 1.0f, 0.0f, true, "Idle"),
               "Zero duration rejected");

    // Negative duration rejected
    assertTrue(!sys.queueLine("e1", "l3", "text", 1.0f, -1.0f, true, "Idle"),
               "Negative duration rejected");

    // Missing entity rejected
    assertTrue(!sys.queueLine("missing", "l3", "text", 1.0f, 3.0f, true, "Idle"),
               "Missing entity rejected");
}

static void testChatterInterruptQueueOrder() {
    std::cout << "\n=== ChatterInterrupt: QueueOrder ===" << std::endl;
    ecs::World world;
    systems::ChatterInterruptSystem sys(&world);
    world.createEntity("e1");
    sys.initialize("e1");

    // Occupy active slot
    sys.queueLine("e1", "active", "Active", 0.5f, 5.0f, true, "Idle");
    assertTrue(sys.isSpeaking("e1"), "Speaking");

    // Queue three lines with different priorities
    sys.queueLine("e1", "low",  "Low priority",    0.1f, 2.0f, true, "Idle");
    sys.queueLine("e1", "high", "High priority",   5.0f, 2.0f, true, "Combat");
    sys.queueLine("e1", "mid",  "Medium priority", 2.0f, 2.0f, true, "Warp");

    assertTrue(sys.getQueueDepth("e1") == 3, "Queue depth 3");

    // Finish active line — high priority should become active next
    sys.finishCurrentLine("e1");
    assertTrue(sys.getActiveLineId("e1") == "high", "High priority pops first");
    assertTrue(sys.getQueueDepth("e1") == 2, "Queue depth 2 after pop");

    sys.finishCurrentLine("e1");
    assertTrue(sys.getActiveLineId("e1") == "mid", "Mid priority pops second");

    sys.finishCurrentLine("e1");
    assertTrue(sys.getActiveLineId("e1") == "low", "Low priority pops last");
}

static void testChatterInterruptQueueCapacity() {
    std::cout << "\n=== ChatterInterrupt: QueueCapacity ===" << std::endl;
    ecs::World world;
    systems::ChatterInterruptSystem sys(&world);
    world.createEntity("e1");
    sys.initialize("e1");

    sys.setMaxQueueSize("e1", 3);
    assertTrue(sys.getMaxQueueSize("e1") == 3, "Max queue size is 3");

    // Occupy active slot
    sys.queueLine("e1", "active", "Active", 0.0f, 10.0f, true, "Idle");

    // Fill queue to capacity
    sys.queueLine("e1", "q1", "Q1", 1.0f, 2.0f, true, "Idle");
    sys.queueLine("e1", "q2", "Q2", 2.0f, 2.0f, true, "Idle");
    sys.queueLine("e1", "q3", "Q3", 3.0f, 2.0f, true, "Idle");
    assertTrue(sys.getQueueDepth("e1") == 3, "Queue full at 3");

    // Adding a higher-priority line should displace the lowest (q1)
    sys.queueLine("e1", "q4", "Q4", 4.0f, 2.0f, true, "Idle");
    assertTrue(sys.getQueueDepth("e1") == 3, "Queue still capped at 3");
    assertTrue(!sys.hasLineInQueue("e1", "q1"), "q1 (lowest) was dropped");
    assertTrue(sys.hasLineInQueue("e1", "q4"), "q4 (highest) kept");

    // Adding a lower-priority line than the tail should still cap
    sys.queueLine("e1", "q5", "Q5", 0.5f, 2.0f, true, "Idle");
    assertTrue(sys.getQueueDepth("e1") == 3, "Queue still capped at 3 after low add");
}

static void testChatterInterruptTimerAdvancement() {
    std::cout << "\n=== ChatterInterrupt: TimerAdvancement ===" << std::endl;
    ecs::World world;
    systems::ChatterInterruptSystem sys(&world);
    world.createEntity("e1");
    sys.initialize("e1");

    sys.queueLine("e1", "l1", "Line1", 1.0f, 3.0f, true, "Idle");
    sys.queueLine("e1", "l2", "Line2", 0.5f, 2.0f, true, "Idle");

    assertTrue(sys.isSpeaking("e1"), "Speaking l1");
    assertTrue(sys.getTotalLinesSpoken("e1") == 0, "Not spoken yet");

    // Partial advance — line still active
    sys.update(2.0f);
    assertTrue(sys.isSpeaking("e1"), "Still speaking after 2s of 3s");
    assertTrue(sys.getActiveLineId("e1") == "l1", "Still l1 active");
    assertTrue(sys.getTotalLinesSpoken("e1") == 0, "Still not spoken");

    // Advance past duration — line finishes, next pops
    sys.update(1.5f);
    assertTrue(sys.isSpeaking("e1"), "Now speaking l2");
    assertTrue(sys.getActiveLineId("e1") == "l2", "l2 is now active");
    assertTrue(sys.getTotalLinesSpoken("e1") == 1, "1 line spoken");

    // Advance past l2 duration
    sys.update(3.0f);
    assertTrue(!sys.isSpeaking("e1"), "Silent after l2 finishes");
    assertTrue(sys.getTotalLinesSpoken("e1") == 2, "2 lines spoken total");
    assertTrue(sys.getQueueDepth("e1") == 0, "Queue empty");
}

static void testChatterInterruptInterruptLine() {
    std::cout << "\n=== ChatterInterrupt: InterruptLine ===" << std::endl;
    ecs::World world;
    systems::ChatterInterruptSystem sys(&world);
    world.createEntity("e1");
    sys.initialize("e1");

    // Start speaking a low-priority interruptible line
    sys.queueLine("e1", "idle", "Idle chatter", 1.0f, 10.0f, true, "Idle");
    assertTrue(sys.isSpeaking("e1"), "Speaking idle");

    // Interrupt with higher priority
    assertTrue(sys.interruptWith("e1", "combat", "Enemy detected!", 5.0f, 3.0f, "Combat"),
               "Interrupt succeeds");
    assertTrue(sys.wasInterrupted("e1"), "Was interrupted flag set");
    assertTrue(sys.getActiveLineId("e1") == "combat", "New line is active");
    assertTrue(approxEqual(sys.getActivePriority("e1"), 5.0f), "New priority 5.0");
    assertTrue(sys.getTotalInterrupts("e1") == 1, "1 interrupt counted");

    // Try to interrupt with lower or equal priority — should fail
    assertTrue(!sys.interruptWith("e1", "low", "Low priority", 2.0f, 3.0f, "Idle"),
               "Lower priority interrupt fails");
    assertTrue(!sys.interruptWith("e1", "same", "Same priority", 5.0f, 3.0f, "Idle"),
               "Equal priority interrupt fails");
    assertTrue(sys.getActiveLineId("e1") == "combat", "Still combat active");

    // Non-interruptible line cannot be interrupted
    sys.finishCurrentLine("e1");
    sys.queueLine("e1", "locked", "Cannot stop", 3.0f, 10.0f, false, "Warp");
    assertTrue(!sys.isActiveInterruptible("e1"), "Active line not interruptible");
    assertTrue(!sys.interruptWith("e1", "high", "High", 10.0f, 3.0f, "Combat"),
               "Non-interruptible line blocks interrupt");

    // interruptWith on non-speaking entity fails
    sys.finishCurrentLine("e1");
    sys.update(20.0f); // exhaust any remaining
    assertTrue(!sys.interruptWith("e1", "x", "x", 9.0f, 3.0f, "Idle"),
               "interruptWith when silent fails");

    // empty line_id rejected
    sys.queueLine("e1", "new", "new", 1.0f, 5.0f, true, "Idle");
    assertTrue(!sys.interruptWith("e1", "", "x", 9.0f, 3.0f, "Idle"),
               "interruptWith empty line_id rejected");

    // zero duration rejected
    assertTrue(!sys.interruptWith("e1", "dur0", "x", 9.0f, 0.0f, "Idle"),
               "interruptWith zero duration rejected");
}

static void testChatterInterruptFinishCurrentLine() {
    std::cout << "\n=== ChatterInterrupt: FinishCurrentLine ===" << std::endl;
    ecs::World world;
    systems::ChatterInterruptSystem sys(&world);
    world.createEntity("e1");
    sys.initialize("e1");

    // finishCurrentLine when silent returns false
    assertTrue(!sys.finishCurrentLine("e1"), "finishCurrentLine when silent fails");

    sys.queueLine("e1", "l1", "L1", 1.0f, 10.0f, true, "Idle");
    sys.queueLine("e1", "l2", "L2", 0.5f, 5.0f, true, "Idle");

    assertTrue(sys.finishCurrentLine("e1"), "finishCurrentLine succeeds");
    assertTrue(sys.getTotalLinesSpoken("e1") == 1, "1 line spoken");
    assertTrue(sys.getActiveLineId("e1") == "l2", "l2 now active");
    assertTrue(sys.isSpeaking("e1"), "Still speaking");

    assertTrue(sys.finishCurrentLine("e1"), "finishCurrentLine l2 succeeds");
    assertTrue(sys.getTotalLinesSpoken("e1") == 2, "2 lines spoken");
    assertTrue(!sys.isSpeaking("e1"), "Silent after all lines done");

    // finishCurrentLine on missing entity
    assertTrue(!sys.finishCurrentLine("missing"), "finishCurrentLine missing entity");
}

static void testChatterInterruptClearQueue() {
    std::cout << "\n=== ChatterInterrupt: ClearQueue ===" << std::endl;
    ecs::World world;
    systems::ChatterInterruptSystem sys(&world);
    world.createEntity("e1");
    sys.initialize("e1");

    sys.queueLine("e1", "active", "Active", 1.0f, 10.0f, true, "Idle");
    sys.queueLine("e1", "q1", "Q1", 0.5f, 3.0f, true, "Idle");
    sys.queueLine("e1", "q2", "Q2", 0.3f, 3.0f, true, "Idle");
    assertTrue(sys.getQueueDepth("e1") == 2, "Queue has 2 pending");

    assertTrue(sys.clearQueue("e1"), "Clear queue succeeds");
    assertTrue(sys.getQueueDepth("e1") == 0, "Queue empty after clear");
    // Active line is not affected
    assertTrue(sys.isSpeaking("e1"), "Still speaking active line");
    assertTrue(sys.getActiveLineId("e1") == "active", "Active line preserved");

    assertTrue(!sys.clearQueue("missing"), "clearQueue missing entity fails");
}

static void testChatterInterruptRemoveLine() {
    std::cout << "\n=== ChatterInterrupt: RemoveLine ===" << std::endl;
    ecs::World world;
    systems::ChatterInterruptSystem sys(&world);
    world.createEntity("e1");
    sys.initialize("e1");

    sys.queueLine("e1", "active", "Active", 1.0f, 10.0f, true, "Idle");
    sys.queueLine("e1", "q1", "Q1", 0.5f, 3.0f, true, "Idle");
    sys.queueLine("e1", "q2", "Q2", 0.3f, 3.0f, true, "Idle");

    assertTrue(sys.hasLineInQueue("e1", "q1"), "q1 in queue");
    assertTrue(sys.removeLine("e1", "q1"), "Remove q1 succeeds");
    assertTrue(!sys.hasLineInQueue("e1", "q1"), "q1 removed from queue");
    assertTrue(sys.getQueueDepth("e1") == 1, "Queue depth 1 after remove");
    assertTrue(sys.hasLineInQueue("e1", "q2"), "q2 still in queue");

    // Remove non-existent
    assertTrue(!sys.removeLine("e1", "nothere"), "Remove missing line fails");

    // Remove on missing entity
    assertTrue(!sys.removeLine("missing", "q2"), "Remove on missing entity fails");
}

static void testChatterInterruptMissingEntity() {
    std::cout << "\n=== ChatterInterrupt: MissingEntity ===" << std::endl;
    ecs::World world;
    systems::ChatterInterruptSystem sys(&world);

    assertTrue(!sys.isSpeaking("missing"), "isSpeaking missing returns false");
    assertTrue(sys.getActiveLineText("missing").empty(), "getActiveLineText missing returns empty");
    assertTrue(sys.getActiveLineId("missing").empty(), "getActiveLineId missing returns empty");
    assertTrue(approxEqual(sys.getActivePriority("missing"), 0.0f), "getActivePriority missing returns 0");
    assertTrue(!sys.isActiveInterruptible("missing"), "isActiveInterruptible missing returns false");
    assertTrue(!sys.wasInterrupted("missing"), "wasInterrupted missing returns false");
    assertTrue(sys.getQueueDepth("missing") == 0, "getQueueDepth missing returns 0");
    assertTrue(!sys.hasLineInQueue("missing", "l1"), "hasLineInQueue missing returns false");
    assertTrue(sys.getTotalLinesQueued("missing") == 0, "getTotalLinesQueued missing returns 0");
    assertTrue(sys.getTotalLinesSpoken("missing") == 0, "getTotalLinesSpoken missing returns 0");
    assertTrue(sys.getTotalInterrupts("missing") == 0, "getTotalInterrupts missing returns 0");
    assertTrue(sys.getMaxQueueSize("missing") == 0, "getMaxQueueSize missing returns 0");
    assertTrue(!sys.clearQueue("missing"), "clearQueue missing returns false");
    assertTrue(!sys.removeLine("missing", "l"), "removeLine missing returns false");
    assertTrue(!sys.finishCurrentLine("missing"), "finishCurrentLine missing returns false");
    assertTrue(!sys.setMaxQueueSize("missing", 3), "setMaxQueueSize missing returns false");
    assertTrue(!sys.queueLine("missing", "l", "t", 1.0f, 1.0f, true, "Idle"),
               "queueLine missing returns false");
    assertTrue(!sys.interruptWith("missing", "l", "t", 1.0f, 1.0f, "Idle"),
               "interruptWith missing returns false");
}

static void testChatterInterruptConfiguration() {
    std::cout << "\n=== ChatterInterrupt: Configuration ===" << std::endl;
    ecs::World world;
    systems::ChatterInterruptSystem sys(&world);
    world.createEntity("e1");
    sys.initialize("e1");

    // setMaxQueueSize valid
    assertTrue(sys.setMaxQueueSize("e1", 1), "setMaxQueueSize 1 succeeds");
    assertTrue(sys.getMaxQueueSize("e1") == 1, "Max is now 1");

    assertTrue(sys.setMaxQueueSize("e1", 10), "setMaxQueueSize 10 succeeds");
    assertTrue(sys.getMaxQueueSize("e1") == 10, "Max is now 10");

    // setMaxQueueSize invalid (< 1)
    assertTrue(!sys.setMaxQueueSize("e1", 0), "setMaxQueueSize 0 fails");
    assertTrue(!sys.setMaxQueueSize("e1", -5), "setMaxQueueSize -5 fails");
    assertTrue(sys.getMaxQueueSize("e1") == 10, "Max unchanged after invalid set");
}

void run_chatter_interrupt_system_tests() {
    testChatterInterruptInit();
    testChatterInterruptQueueLine();
    testChatterInterruptQueueOrder();
    testChatterInterruptQueueCapacity();
    testChatterInterruptTimerAdvancement();
    testChatterInterruptInterruptLine();
    testChatterInterruptFinishCurrentLine();
    testChatterInterruptClearQueue();
    testChatterInterruptRemoveLine();
    testChatterInterruptMissingEntity();
    testChatterInterruptConfiguration();
}
