// Tests for: MailSystem
#include "test_log.h"
#include "components/social_components.h"
#include "ecs/system.h"
#include "systems/mail_system.h"

using namespace atlas;

static void testMailInit() {
    std::cout << "\n=== MailSystem: Init ===" << std::endl;
    ecs::World world;
    systems::MailSystem sys(&world);
    world.createEntity("e1");

    assertTrue(sys.initialize("e1"), "Init succeeds");
    assertTrue(!sys.initialize("nonexistent"), "Init fails on missing entity");
    assertTrue(sys.getMailCount("e1") == 0, "No mails initially");
    assertTrue(sys.getUnreadCount("e1") == 0, "No unread initially");
    assertTrue(sys.getFlaggedCount("e1") == 0, "No flagged initially");
    assertTrue(sys.getOwner("e1") == "", "Owner empty initially");
    assertTrue(sys.getTotalReceived("e1") == 0, "Total received 0");
    assertTrue(sys.getTotalDeleted("e1") == 0, "Total deleted 0");
}

static void testMailSend() {
    std::cout << "\n=== MailSystem: Send ===" << std::endl;
    ecs::World world;
    systems::MailSystem sys(&world);
    world.createEntity("e1");
    sys.initialize("e1");

    assertTrue(sys.sendMail("e1", "m1", "sender1", "Alice", "Hello", "Body text"), "Send mail");
    assertTrue(sys.getMailCount("e1") == 1, "1 mail");
    assertTrue(sys.hasMail("e1", "m1"), "Has m1");
    assertTrue(sys.getSubject("e1", "m1") == "Hello", "Subject matches");
    assertTrue(sys.getSender("e1", "m1") == "Alice", "Sender matches");
    assertTrue(sys.getTotalReceived("e1") == 1, "Total received 1");

    // Unread by default
    assertTrue(!sys.isRead("e1", "m1"), "Not read initially");
    assertTrue(sys.getUnreadCount("e1") == 1, "1 unread");

    // Duplicate prevention
    assertTrue(!sys.sendMail("e1", "m1", "s2", "Bob", "Dup", "Body"), "Reject duplicate ID");
    assertTrue(sys.getMailCount("e1") == 1, "Still 1 mail");

    // Empty validations
    assertTrue(!sys.sendMail("e1", "", "s1", "Alice", "Sub", "Body"), "Reject empty mail_id");
    assertTrue(!sys.sendMail("e1", "m2", "s1", "Alice", "", "Body"), "Reject empty subject");

    // Second mail
    assertTrue(sys.sendMail("e1", "m2", "sender2", "Bob", "Hi", "World"), "Send second mail");
    assertTrue(sys.getMailCount("e1") == 2, "2 mails");
    assertTrue(sys.getUnreadCount("e1") == 2, "2 unread");
    assertTrue(sys.getTotalReceived("e1") == 2, "Total received 2");
}

static void testMailReadFlag() {
    std::cout << "\n=== MailSystem: Read & Flag ===" << std::endl;
    ecs::World world;
    systems::MailSystem sys(&world);
    world.createEntity("e1");
    sys.initialize("e1");

    sys.sendMail("e1", "m1", "s1", "Alice", "Sub1", "Body1");
    sys.sendMail("e1", "m2", "s2", "Bob", "Sub2", "Body2");
    sys.sendMail("e1", "m3", "s3", "Carol", "Sub3", "Body3");

    assertTrue(sys.markRead("e1", "m1"), "Mark m1 read");
    assertTrue(sys.isRead("e1", "m1"), "m1 is read");
    assertTrue(sys.getUnreadCount("e1") == 2, "2 unread");

    assertTrue(!sys.markRead("e1", "nonexist"), "Cannot mark nonexistent");

    assertTrue(sys.markAllRead("e1"), "Mark all read");
    assertTrue(sys.getUnreadCount("e1") == 0, "0 unread after mark all");
    assertTrue(sys.isRead("e1", "m2"), "m2 is read");
    assertTrue(sys.isRead("e1", "m3"), "m3 is read");

    // Flagging
    assertTrue(!sys.isFlagged("e1", "m1"), "m1 not flagged initially");
    assertTrue(sys.flagMail("e1", "m1"), "Flag m1");
    assertTrue(sys.isFlagged("e1", "m1"), "m1 is flagged");
    assertTrue(sys.getFlaggedCount("e1") == 1, "1 flagged");

    assertTrue(sys.flagMail("e1", "m2"), "Flag m2");
    assertTrue(sys.getFlaggedCount("e1") == 2, "2 flagged");

    assertTrue(sys.unflagMail("e1", "m1"), "Unflag m1");
    assertTrue(!sys.isFlagged("e1", "m1"), "m1 no longer flagged");
    assertTrue(sys.getFlaggedCount("e1") == 1, "1 flagged");

    assertTrue(!sys.flagMail("e1", "nonexist"), "Cannot flag nonexistent");
    assertTrue(!sys.unflagMail("e1", "nonexist"), "Cannot unflag nonexistent");
}

static void testMailDelete() {
    std::cout << "\n=== MailSystem: Delete ===" << std::endl;
    ecs::World world;
    systems::MailSystem sys(&world);
    world.createEntity("e1");
    sys.initialize("e1");

    sys.sendMail("e1", "m1", "s1", "Alice", "Sub1", "B1");
    sys.sendMail("e1", "m2", "s2", "Bob", "Sub2", "B2");

    assertTrue(sys.deleteMail("e1", "m1"), "Delete m1");
    assertTrue(sys.getMailCount("e1") == 1, "1 mail left");
    assertTrue(!sys.hasMail("e1", "m1"), "m1 gone");
    assertTrue(sys.getTotalDeleted("e1") == 1, "Total deleted 1");

    assertTrue(!sys.deleteMail("e1", "m1"), "Cannot delete again");
    assertTrue(!sys.deleteMail("e1", "nonexist"), "Cannot delete nonexistent");
}

static void testMailClearAll() {
    std::cout << "\n=== MailSystem: Clear All ===" << std::endl;
    ecs::World world;
    systems::MailSystem sys(&world);
    world.createEntity("e1");
    sys.initialize("e1");

    sys.sendMail("e1", "m1", "s1", "A", "S1", "B1");
    sys.sendMail("e1", "m2", "s2", "B", "S2", "B2");
    sys.sendMail("e1", "m3", "s3", "C", "S3", "B3");

    assertTrue(sys.clearAll("e1"), "Clear all");
    assertTrue(sys.getMailCount("e1") == 0, "0 mails after clear");
    assertTrue(sys.getTotalDeleted("e1") == 3, "Total deleted 3");
    assertTrue(sys.getTotalReceived("e1") == 3, "Total received preserved");
}

static void testMailConfiguration() {
    std::cout << "\n=== MailSystem: Configuration ===" << std::endl;
    ecs::World world;
    systems::MailSystem sys(&world);
    world.createEntity("e1");
    sys.initialize("e1");

    assertTrue(sys.setOwner("e1", "pilot_001"), "Set owner");
    assertTrue(sys.getOwner("e1") == "pilot_001", "Owner matches");

    assertTrue(sys.setMaxMails("e1", 5), "Set max mails");
    assertTrue(!sys.setMaxMails("e1", 0), "Reject zero max");
    assertTrue(!sys.setMaxMails("e1", -1), "Reject negative max");
}

static void testMailAutoPurge() {
    std::cout << "\n=== MailSystem: Auto Purge ===" << std::endl;
    ecs::World world;
    systems::MailSystem sys(&world);
    world.createEntity("e1");
    sys.initialize("e1");

    sys.setMaxMails("e1", 3);

    sys.sendMail("e1", "m1", "s1", "A", "S1", "B1");
    sys.sendMail("e1", "m2", "s2", "B", "S2", "B2");
    sys.sendMail("e1", "m3", "s3", "C", "S3", "B3");
    assertTrue(sys.getMailCount("e1") == 3, "3 mails at capacity");

    // Adding one more should auto-purge the oldest
    assertTrue(sys.sendMail("e1", "m4", "s4", "D", "S4", "B4"), "Send m4 (over capacity)");
    assertTrue(sys.getMailCount("e1") == 3, "Still 3 after auto-purge");
    assertTrue(!sys.hasMail("e1", "m1"), "m1 purged (oldest)");
    assertTrue(sys.hasMail("e1", "m4"), "m4 exists");
    assertTrue(sys.getTotalReceived("e1") == 4, "Total received 4");
}

static void testMailLabels() {
    std::cout << "\n=== MailSystem: Labels ===" << std::endl;
    ecs::World world;
    systems::MailSystem sys(&world);
    world.createEntity("e1");
    sys.initialize("e1");

    sys.sendMail("e1", "m1", "s1", "Alice", "Sub", "Body");
    sys.sendMail("e1", "m2", "s2", "Bob", "Sub2", "Body2");

    assertTrue(sys.addLabel("e1", "m1", "important"), "Add label");
    assertTrue(sys.getLabelCount("e1", "m1") == 1, "1 label on m1");
    assertTrue(sys.getMailCountByLabel("e1", "important") == 1, "1 mail with important");

    assertTrue(sys.addLabel("e1", "m1", "work"), "Add second label");
    assertTrue(sys.getLabelCount("e1", "m1") == 2, "2 labels on m1");
    assertTrue(!sys.addLabel("e1", "m1", "important"), "Reject duplicate label");

    assertTrue(sys.addLabel("e1", "m2", "important"), "Add label to m2");
    assertTrue(sys.getMailCountByLabel("e1", "important") == 2, "2 mails with important");

    assertTrue(sys.removeLabel("e1", "m1", "work"), "Remove label");
    assertTrue(sys.getLabelCount("e1", "m1") == 1, "1 label on m1 after remove");
    assertTrue(!sys.removeLabel("e1", "m1", "work"), "Cannot remove again");
    assertTrue(!sys.removeLabel("e1", "m1", "nonexist"), "Cannot remove nonexistent label");

    // Invalid operations
    assertTrue(!sys.addLabel("e1", "m1", ""), "Reject empty label");
    assertTrue(!sys.addLabel("e1", "nonexist", "label"), "Reject label on nonexistent mail");
    assertTrue(!sys.removeLabel("e1", "nonexist", "label"), "Reject remove on nonexistent mail");
}

static void testMailUpdate() {
    std::cout << "\n=== MailSystem: Update ===" << std::endl;
    ecs::World world;
    systems::MailSystem sys(&world);
    world.createEntity("e1");
    sys.initialize("e1");

    sys.sendMail("e1", "m1", "s1", "Alice", "Sub", "Body");
    sys.update(1.0f);
    // Update should not alter mail count
    assertTrue(sys.getMailCount("e1") == 1, "Mail count unchanged after update");
}

static void testMailMissing() {
    std::cout << "\n=== MailSystem: Missing Entity ===" << std::endl;
    ecs::World world;
    systems::MailSystem sys(&world);

    // Mutating methods return false
    assertTrue(!sys.initialize("none"), "Init fails");
    assertTrue(!sys.sendMail("none", "m1", "s1", "A", "S", "B"), "sendMail fails");
    assertTrue(!sys.deleteMail("none", "m1"), "deleteMail fails");
    assertTrue(!sys.clearAll("none"), "clearAll fails");
    assertTrue(!sys.markRead("none", "m1"), "markRead fails");
    assertTrue(!sys.markAllRead("none"), "markAllRead fails");
    assertTrue(!sys.flagMail("none", "m1"), "flagMail fails");
    assertTrue(!sys.unflagMail("none", "m1"), "unflagMail fails");
    assertTrue(!sys.setOwner("none", "o1"), "setOwner fails");
    assertTrue(!sys.setMaxMails("none", 10), "setMaxMails fails");
    assertTrue(!sys.addLabel("none", "m1", "l"), "addLabel fails");
    assertTrue(!sys.removeLabel("none", "m1", "l"), "removeLabel fails");

    // Queries return defaults
    assertTrue(sys.getMailCount("none") == 0, "getMailCount default");
    assertTrue(sys.getUnreadCount("none") == 0, "getUnreadCount default");
    assertTrue(sys.getFlaggedCount("none") == 0, "getFlaggedCount default");
    assertTrue(!sys.hasMail("none", "m1"), "hasMail default");
    assertTrue(!sys.isRead("none", "m1"), "isRead default");
    assertTrue(!sys.isFlagged("none", "m1"), "isFlagged default");
    assertTrue(sys.getOwner("none") == "", "getOwner default");
    assertTrue(sys.getSubject("none", "m1") == "", "getSubject default");
    assertTrue(sys.getSender("none", "m1") == "", "getSender default");
    assertTrue(sys.getTotalReceived("none") == 0, "getTotalReceived default");
    assertTrue(sys.getTotalDeleted("none") == 0, "getTotalDeleted default");
    assertTrue(sys.getLabelCount("none", "m1") == 0, "getLabelCount default");
    assertTrue(sys.getMailCountByLabel("none", "l") == 0, "getMailCountByLabel default");
}

void run_mail_system_tests() {
    testMailInit();
    testMailSend();
    testMailReadFlag();
    testMailDelete();
    testMailClearAll();
    testMailConfiguration();
    testMailAutoPurge();
    testMailLabels();
    testMailUpdate();
    testMailMissing();
}
