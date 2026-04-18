# Notification System

## Overview
The workspace notification system provides severity-based alerts for build
status, AI suggestions, system warnings, and user actions.

## Existing Implementation
- `NF::NotificationQueue` — toast notification queue in EditorApp
- `NF::UI::AtlasUI::NotificationHost` — service for displaying notifications
- `NF::UI::AtlasUI::NotificationCard` — widget for notification display

## Severity Levels

| Level | Color | Duration | Action |
|-------|-------|----------|--------|
| Info | Blue | 3s auto-dismiss | None |
| Success | Green | 3s auto-dismiss | None |
| Warning | Yellow | 10s, manual dismiss | Optional action button |
| Error | Red | Persistent until dismissed | View details, escalate to AI |
| Critical | Red pulsing | Persistent, modal | Must acknowledge |

## Missing Contracts
- Severity routing rules (which events trigger which level)
- Notification deduplication (same error doesn't spam)
- Notification history (scrollback log)
- Action buttons (fix, ignore, details, escalate)
- Sound/visual attention hooks
- AtlasAI escalation path (error → AI analysis)

## Workflow Rules
1. Build failure → Error notification → "View Log" + "Ask AI" buttons
2. Test failure → Warning notification → "View Details" button
3. Asset import → Info notification → auto-dismiss
4. AI suggestion → Info notification → "Apply" + "Dismiss" buttons
5. Critical system error → Critical notification → must acknowledge
