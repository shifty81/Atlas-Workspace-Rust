#include "systems/mail_system.h"
#include "ecs/world.h"
#include "ecs/entity.h"

namespace atlas {
namespace systems {

MailSystem::MailSystem(ecs::World* world)
    : SingleComponentSystem(world) {}

void MailSystem::updateComponent(
        ecs::Entity& /*entity*/,
        components::MailboxState& comp,
        float delta_time) {
    if (!comp.active) return;
    comp.elapsed += delta_time;
}

bool MailSystem::initialize(const std::string& entity_id) {
    auto* entity = world_->getEntity(entity_id);
    if (!entity) return false;
    auto comp = std::make_unique<components::MailboxState>();
    entity->addComponent(std::move(comp));
    return true;
}

// --- Mail management ---

bool MailSystem::sendMail(const std::string& entity_id,
                          const std::string& mail_id,
                          const std::string& sender_id,
                          const std::string& sender_name,
                          const std::string& subject,
                          const std::string& body) {
    auto* comp = getComponentFor(entity_id);
    if (!comp) return false;
    if (mail_id.empty()) return false;
    if (subject.empty()) return false;

    // Duplicate prevention
    for (const auto& m : comp->mails) {
        if (m.mail_id == mail_id) return false;
    }

    // Auto-purge oldest at capacity
    while (static_cast<int>(comp->mails.size()) >= comp->max_mails) {
        comp->mails.erase(comp->mails.begin());
    }

    components::MailboxState::Mail mail;
    mail.mail_id     = mail_id;
    mail.sender_id   = sender_id;
    mail.sender_name = sender_name;
    mail.subject     = subject;
    mail.body        = body;
    mail.timestamp   = comp->elapsed;
    comp->mails.push_back(mail);
    ++comp->total_received;
    return true;
}

bool MailSystem::deleteMail(const std::string& entity_id,
                            const std::string& mail_id) {
    auto* comp = getComponentFor(entity_id);
    if (!comp) return false;
    for (auto it = comp->mails.begin(); it != comp->mails.end(); ++it) {
        if (it->mail_id == mail_id) {
            comp->mails.erase(it);
            ++comp->total_deleted;
            return true;
        }
    }
    return false;
}

bool MailSystem::clearAll(const std::string& entity_id) {
    auto* comp = getComponentFor(entity_id);
    if (!comp) return false;
    comp->total_deleted += static_cast<int>(comp->mails.size());
    comp->mails.clear();
    return true;
}

// --- Mail state ---

bool MailSystem::markRead(const std::string& entity_id,
                          const std::string& mail_id) {
    auto* comp = getComponentFor(entity_id);
    if (!comp) return false;
    for (auto& m : comp->mails) {
        if (m.mail_id == mail_id) {
            m.is_read = true;
            return true;
        }
    }
    return false;
}

bool MailSystem::markAllRead(const std::string& entity_id) {
    auto* comp = getComponentFor(entity_id);
    if (!comp) return false;
    for (auto& m : comp->mails) {
        m.is_read = true;
    }
    return true;
}

bool MailSystem::flagMail(const std::string& entity_id,
                          const std::string& mail_id) {
    auto* comp = getComponentFor(entity_id);
    if (!comp) return false;
    for (auto& m : comp->mails) {
        if (m.mail_id == mail_id) {
            m.is_flagged = true;
            return true;
        }
    }
    return false;
}

bool MailSystem::unflagMail(const std::string& entity_id,
                            const std::string& mail_id) {
    auto* comp = getComponentFor(entity_id);
    if (!comp) return false;
    for (auto& m : comp->mails) {
        if (m.mail_id == mail_id) {
            m.is_flagged = false;
            return true;
        }
    }
    return false;
}

// --- Configuration ---

bool MailSystem::setOwner(const std::string& entity_id,
                          const std::string& owner_id) {
    auto* comp = getComponentFor(entity_id);
    if (!comp) return false;
    comp->owner_id = owner_id;
    return true;
}

bool MailSystem::setMaxMails(const std::string& entity_id, int max_mails) {
    auto* comp = getComponentFor(entity_id);
    if (!comp) return false;
    if (max_mails <= 0) return false;
    comp->max_mails = max_mails;
    return true;
}

// --- Label management ---

bool MailSystem::addLabel(const std::string& entity_id,
                          const std::string& mail_id,
                          const std::string& label) {
    auto* comp = getComponentFor(entity_id);
    if (!comp) return false;
    if (label.empty()) return false;
    for (auto& m : comp->mails) {
        if (m.mail_id == mail_id) {
            // Duplicate label prevention
            for (const auto& l : m.labels) {
                if (l == label) return false;
            }
            m.labels.push_back(label);
            return true;
        }
    }
    return false;
}

bool MailSystem::removeLabel(const std::string& entity_id,
                             const std::string& mail_id,
                             const std::string& label) {
    auto* comp = getComponentFor(entity_id);
    if (!comp) return false;
    for (auto& m : comp->mails) {
        if (m.mail_id == mail_id) {
            for (auto it = m.labels.begin(); it != m.labels.end(); ++it) {
                if (*it == label) {
                    m.labels.erase(it);
                    return true;
                }
            }
            return false;
        }
    }
    return false;
}

// --- Queries ---

int MailSystem::getMailCount(const std::string& entity_id) const {
    auto* comp = getComponentFor(entity_id);
    if (!comp) return 0;
    return static_cast<int>(comp->mails.size());
}

int MailSystem::getUnreadCount(const std::string& entity_id) const {
    auto* comp = getComponentFor(entity_id);
    if (!comp) return 0;
    int count = 0;
    for (const auto& m : comp->mails) {
        if (!m.is_read) ++count;
    }
    return count;
}

int MailSystem::getFlaggedCount(const std::string& entity_id) const {
    auto* comp = getComponentFor(entity_id);
    if (!comp) return 0;
    int count = 0;
    for (const auto& m : comp->mails) {
        if (m.is_flagged) ++count;
    }
    return count;
}

bool MailSystem::hasMail(const std::string& entity_id,
                         const std::string& mail_id) const {
    auto* comp = getComponentFor(entity_id);
    if (!comp) return false;
    for (const auto& m : comp->mails) {
        if (m.mail_id == mail_id) return true;
    }
    return false;
}

bool MailSystem::isRead(const std::string& entity_id,
                        const std::string& mail_id) const {
    auto* comp = getComponentFor(entity_id);
    if (!comp) return false;
    for (const auto& m : comp->mails) {
        if (m.mail_id == mail_id) return m.is_read;
    }
    return false;
}

bool MailSystem::isFlagged(const std::string& entity_id,
                           const std::string& mail_id) const {
    auto* comp = getComponentFor(entity_id);
    if (!comp) return false;
    for (const auto& m : comp->mails) {
        if (m.mail_id == mail_id) return m.is_flagged;
    }
    return false;
}

std::string MailSystem::getOwner(const std::string& entity_id) const {
    auto* comp = getComponentFor(entity_id);
    if (!comp) return "";
    return comp->owner_id;
}

std::string MailSystem::getSubject(const std::string& entity_id,
                                   const std::string& mail_id) const {
    auto* comp = getComponentFor(entity_id);
    if (!comp) return "";
    for (const auto& m : comp->mails) {
        if (m.mail_id == mail_id) return m.subject;
    }
    return "";
}

std::string MailSystem::getSender(const std::string& entity_id,
                                  const std::string& mail_id) const {
    auto* comp = getComponentFor(entity_id);
    if (!comp) return "";
    for (const auto& m : comp->mails) {
        if (m.mail_id == mail_id) return m.sender_name;
    }
    return "";
}

int MailSystem::getTotalReceived(const std::string& entity_id) const {
    auto* comp = getComponentFor(entity_id);
    if (!comp) return 0;
    return comp->total_received;
}

int MailSystem::getTotalDeleted(const std::string& entity_id) const {
    auto* comp = getComponentFor(entity_id);
    if (!comp) return 0;
    return comp->total_deleted;
}

int MailSystem::getLabelCount(const std::string& entity_id,
                              const std::string& mail_id) const {
    auto* comp = getComponentFor(entity_id);
    if (!comp) return 0;
    for (const auto& m : comp->mails) {
        if (m.mail_id == mail_id) return static_cast<int>(m.labels.size());
    }
    return 0;
}

int MailSystem::getMailCountByLabel(const std::string& entity_id,
                                    const std::string& label) const {
    auto* comp = getComponentFor(entity_id);
    if (!comp) return 0;
    int count = 0;
    for (const auto& m : comp->mails) {
        for (const auto& l : m.labels) {
            if (l == label) { ++count; break; }
        }
    }
    return count;
}

} // namespace systems
} // namespace atlas
