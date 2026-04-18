#ifndef NOVAFORGE_SYSTEMS_MAIL_SYSTEM_H
#define NOVAFORGE_SYSTEMS_MAIL_SYSTEM_H

#include "ecs/single_component_system.h"
#include "components/social_components.h"
#include <string>

namespace atlas {
namespace systems {

class MailSystem
    : public ecs::SingleComponentSystem<components::MailboxState> {
public:
    explicit MailSystem(ecs::World* world);
    ~MailSystem() override = default;

    std::string getName() const override { return "MailSystem"; }

    // --- Lifecycle ---
    bool initialize(const std::string& entity_id);

    // --- Mail management ---
    bool sendMail(const std::string& entity_id,
                  const std::string& mail_id,
                  const std::string& sender_id,
                  const std::string& sender_name,
                  const std::string& subject,
                  const std::string& body);
    bool deleteMail(const std::string& entity_id,
                    const std::string& mail_id);
    bool clearAll(const std::string& entity_id);

    // --- Mail state ---
    bool markRead(const std::string& entity_id,
                  const std::string& mail_id);
    bool markAllRead(const std::string& entity_id);
    bool flagMail(const std::string& entity_id,
                  const std::string& mail_id);
    bool unflagMail(const std::string& entity_id,
                    const std::string& mail_id);

    // --- Configuration ---
    bool setOwner(const std::string& entity_id,
                  const std::string& owner_id);
    bool setMaxMails(const std::string& entity_id, int max_mails);

    // --- Label management ---
    bool addLabel(const std::string& entity_id,
                  const std::string& mail_id,
                  const std::string& label);
    bool removeLabel(const std::string& entity_id,
                     const std::string& mail_id,
                     const std::string& label);

    // --- Queries ---
    int         getMailCount(const std::string& entity_id) const;
    int         getUnreadCount(const std::string& entity_id) const;
    int         getFlaggedCount(const std::string& entity_id) const;
    bool        hasMail(const std::string& entity_id,
                        const std::string& mail_id) const;
    bool        isRead(const std::string& entity_id,
                       const std::string& mail_id) const;
    bool        isFlagged(const std::string& entity_id,
                          const std::string& mail_id) const;
    std::string getOwner(const std::string& entity_id) const;
    std::string getSubject(const std::string& entity_id,
                           const std::string& mail_id) const;
    std::string getSender(const std::string& entity_id,
                          const std::string& mail_id) const;
    int         getTotalReceived(const std::string& entity_id) const;
    int         getTotalDeleted(const std::string& entity_id) const;
    int         getLabelCount(const std::string& entity_id,
                              const std::string& mail_id) const;
    int         getMailCountByLabel(const std::string& entity_id,
                                    const std::string& label) const;

protected:
    void updateComponent(ecs::Entity& entity,
                         components::MailboxState& comp,
                         float delta_time) override;
};

} // namespace systems
} // namespace atlas

#endif // NOVAFORGE_SYSTEMS_MAIL_SYSTEM_H
